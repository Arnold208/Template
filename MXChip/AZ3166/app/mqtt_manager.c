/**
 ******************************************************************************
 * @file    mqtt_manager.c
 * @author  Microsoft Corporation & Contributors
 * @brief   Simplified MQTT Manager Implementation
 ******************************************************************************
 */

#include "mqtt_manager.h"
#include "app_config.h"
#include "board_init.h"
#include "networking.h"
#include "nxd_dns.h"
#include "nxd_mqtt_client.h"
#include "screen.h"
#include "wiced_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MQTT_THREAD_PRIORITY 2

// Connection status
static bool is_connected = false;

static NXD_MQTT_CLIENT mqtt_client;
static UCHAR mqtt_stack[4096];
static char last_received_topic[128];
static char last_received_message[256];
static int publish_count = 0;
static int receive_count = 0;

// Forward declaration
static void mqtt_message_callback(NXD_MQTT_CLIENT* client_ptr, UINT message_count);

bool MQTT_Init(void)
{
    UINT status;

    // Clear screen for initialization status
    screen_print("Initializing...", L0);
    screen_print("Network Stack", L1);

    printf("Initializing Network...\r\n");

    // 1. Initialize Network (NetX + WiFi)
    // Pass SSID, Password, Mode (Assuming WPA2)
    status = network_init((CHAR*)WIFI_SSID, (CHAR*)WIFI_PASSWORD, WPA2_PSK_AES);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: network_init failed (0x%08X)\r\n", status);
        screen_print("Net Init Fail", L1);
        return false;
    }

    // 2. Connect (WiFi Join + DHCP)
    screen_print("Connecting WiFi", L0);
    screen_print((char*)WIFI_SSID, L1);

    printf("Connecting to WiFi: %s\r\n", WIFI_SSID);
    status = network_connect();
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to connect to WiFi (0x%08X)\r\n", status);
        screen_print("WiFi Fail", L1);
        return false;
    }
    printf("WiFi Connected! IP Address obtained.\r\n");
    screen_print("WiFi Connected", L1);

    // 5. MQTT Client Setup
    status = nxd_mqtt_client_create(&mqtt_client,
        "MXChipMQTT",
        MQTT_CLIENT_ID,
        strlen(MQTT_CLIENT_ID),
        &nx_ip,
        &nx_pool[0], // Fixed: nx_pool is an extern struct, not array
        mqtt_stack,
        sizeof(mqtt_stack),
        MQTT_THREAD_PRIORITY,
        NX_NULL,
        0);

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("ERROR: MQTT Create failed (0x%08X)\r\n", status);
        screen_print("MQTT Create Fail", L2);
        return false;
    }

    // 6. Resolve Broker IP
    // 6. Resolve Broker IP
    NXD_ADDRESS server_ip;
    server_ip.nxd_ip_version = NX_IP_VERSION_V4;

#if MQTT_USE_HOSTNAME
    printf("Resolving Hostname: %s\r\n", MQTT_BROKER_HOSTNAME);
    screen_print("Resolving DNS", L2);
    screen_print(MQTT_BROKER_HOSTNAME, L3);

    // DNS Resolution
    ULONG ip_resolved;
    status = nx_dns_host_by_name_get(
        &nx_dns_client, (UCHAR*)MQTT_BROKER_HOSTNAME, &ip_resolved, 5 * TX_TIMER_TICKS_PER_SECOND);

    if (status != NX_SUCCESS)
    {
        printf("ERROR: DNS Resolution failed (0x%08X)\r\n", status);
        screen_print("DNS Fail", L2);
        return false;
    }

    server_ip.nxd_ip_address.v4 = ip_resolved;
    printf("Resolved IP: %lu.%lu.%lu.%lu\r\n",
        (ip_resolved >> 24) & 0xFF,
        (ip_resolved >> 16) & 0xFF,
        (ip_resolved >> 8) & 0xFF,
        (ip_resolved & 0xFF));

#else
    int ip0, ip1, ip2, ip3;
    if (sscanf(MQTT_BROKER_IP_STRING, "%d.%d.%d.%d", &ip0, &ip1, &ip2, &ip3) == 4)
    {
        server_ip.nxd_ip_address.v4 = IP_ADDRESS(ip0, ip1, ip2, ip3);
        printf("Connecting to Broker: %d.%d.%d.%d\r\n", ip0, ip1, ip2, ip3);
        screen_print("Connecting...", L2);
        char broker_disp[20];
        snprintf(broker_disp, sizeof(broker_disp), "%d.%d.%d.%d", ip0, ip1, ip2, ip3);
        screen_print(broker_disp, L3);
    }
    else
    {
        printf("ERROR: Invalid IP format in app_config.h\r\n");
        return false;
    }
#endif

    status = nxd_mqtt_client_connect(&mqtt_client, &server_ip, MQTT_BROKER_PORT, 60, NX_TRUE, NX_WAIT_FOREVER);

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("ERROR: MQTT Connect failed (0x%08X)\r\n", status);
        nxd_mqtt_client_delete(&mqtt_client);
        screen_print("MQTT Conn Fail", L2);
        return false;
    }

    printf("MQTT Connected!\r\n");
    screen_print("MQTT Connected", L2);
    is_connected = true;

    // Set callback
    nxd_mqtt_client_receive_notify_set(&mqtt_client, mqtt_message_callback);

    // Auto-subscribe
    MQTT_Subscribe(MQTT_SUB_TOPIC);

    return true;
}

bool MQTT_Publish(const char* topic, const char* message)
{
    if (!is_connected)
        return false;

    printf("Publishing to '%s': %s\r\n", topic, message);
    UINT status = nxd_mqtt_client_publish(
        &mqtt_client, (CHAR*)topic, strlen(topic), (CHAR*)message, strlen(message), NX_FALSE, 0, NX_WAIT_FOREVER);

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("ERROR: Publish failed (0x%08X)\r\n", status);
        return false;
    }
    publish_count++;
    return true;
}

bool MQTT_Subscribe(const char* topic)
{
    if (!is_connected)
        return false;

    printf("Subscribing to '%s'...\r\n", topic);
    UINT status = nxd_mqtt_client_subscribe(&mqtt_client, (CHAR*)topic, strlen(topic), 0);

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("ERROR: Subscribe failed (0x%08X)\r\n", status);
        return false;
    }
    printf("Subscribed!\r\n");
    return true;
}

void MQTT_Check_Message(void)
{
    if (!is_connected)
        return;
}

const char* MQTT_Get_Last_Message(void)
{
    return last_received_message;
}

const char* MQTT_Get_Last_Topic(void)
{
    return last_received_topic;
}

static void mqtt_message_callback(NXD_MQTT_CLIENT* client_ptr, UINT message_count)
{
    (void)message_count;
    UINT topic_length, message_length;
    UCHAR topic_buffer[128];
    UCHAR message_buffer[256];

    UINT status = nxd_mqtt_client_message_get(client_ptr,
        topic_buffer,
        sizeof(topic_buffer),
        &topic_length,
        message_buffer,
        sizeof(message_buffer),
        &message_length);

    if (status == NXD_MQTT_SUCCESS)
    {
        if (topic_length < sizeof(last_received_topic))
        {
            memcpy(last_received_topic, topic_buffer, topic_length);
            last_received_topic[topic_length] = '\0';
        }

        if (message_length < sizeof(last_received_message))
        {
            memcpy(last_received_message, message_buffer, message_length);
            last_received_message[message_length] = '\0';
        }

        printf("\r\n[MQTT] Received on '%s': %s\r\n", last_received_topic, last_received_message);
        receive_count++;

        // --- JSON Parsing for LED Control ---
        // Look for "led": "ON" or "led": "OFF"
        // This is a simple substring check. Real JSON parsing is better but heavyweight.
        if (strstr((char*)message_buffer, "\"led\": \"ON\"") != NULL ||
            strstr((char*)message_buffer, "\"led\":\"ON\"") != NULL)
        {
            printf("[Command] LED ON request detected.\r\n");
            USER_LED_ON();
        }
        else if (strstr((char*)message_buffer, "\"led\": \"OFF\"") != NULL ||
                 strstr((char*)message_buffer, "\"led\":\"OFF\"") != NULL)
        {
            printf("[Command] LED OFF request detected.\r\n");
            USER_LED_OFF();
        }
    }
}

int MQTT_Get_Publish_Count(void)
{
    return publish_count;
}

int MQTT_Get_Receive_Count(void)
{
    return receive_count;
}
