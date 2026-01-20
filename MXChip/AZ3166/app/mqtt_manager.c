/**
 ******************************************************************************
 * @file    mqtt_manager.c
 * @author  Microsoft Corporation & Contributors
 * @brief   Secure MQTT Manager for Azure Event Grid V2
 ******************************************************************************
 */

#include "mqtt_manager.h"
#include "app_config.h"
#include "board_init.h"
#include "networking.h"
#include "nx_secure_tls_api.h" // Added for TLS
#include "nxd_dns.h"
#include "nxd_mqtt_client.h"
#include "screen.h"
#include "user_credentials.h" // Credentials
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

// TLS Resources
static NX_SECURE_TLS_SESSION tls_session;
static NX_SECURE_X509_CERT root_ca_cert;
static NX_SECURE_X509_CERT device_cert;
// Packet buffer must be large enough for certs + handshake (16KB safe bet)
static UCHAR tls_packet_buffer[16384];

// Forward declaration
static void mqtt_message_callback(NXD_MQTT_CLIENT* client_ptr, UINT message_count);

// TLS Setup Callback
UINT mqtt_tls_setup(NXD_MQTT_CLIENT* client_ptr,
    NX_SECURE_TLS_SESSION* tls_session_ptr,
    NX_SECURE_X509_CERT* certificate_ptr,
    NX_SECURE_X509_CERT* trusted_certificate_ptr)
{
    UINT status;
    printf("[TLS] Setting up session...\r\n");

    // 1. Initialize Session
    status = nx_secure_tls_session_create(
        tls_session_ptr, &nx_crypto_tls_ciphers, nx_crypto_tls_metadata, sizeof(nx_crypto_tls_metadata));
    if (status != NX_SUCCESS)
        return status;

    // 2. Set Packet Buffer
    status = nx_secure_tls_session_packet_buffer_set(tls_session_ptr, tls_packet_buffer, sizeof(tls_packet_buffer));
    if (status != NX_SUCCESS)
        return status;

    // 3. Initialize & Add Device Certificate (Client Auth)
    // Using EC keys usually for Event Grid, but supports RSA too.
    // Assuming keys in user_credentials.h are EC based on previous user logs.
    // If RSA, use NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER.
    // User placeholder doesn't specify logic, but previous logs said "EC".
    status = nx_secure_x509_certificate_initialize(certificate_ptr,
        (UCHAR*)device_cert_der,
        (USHORT)device_cert_len,
        NX_NULL,
        0,
        (UCHAR*)device_key_der,
        (USHORT)device_key_len,
        NX_SECURE_X509_KEY_TYPE_EC_DER);
    if (status != NX_SUCCESS)
    {
        printf("[TLS] Device Cert Init Failed (0x%X)\r\n", status);
        return status;
    }

    status = nx_secure_tls_local_certificate_add(tls_session_ptr, certificate_ptr);
    if (status != NX_SUCCESS)
        return status;

    // 4. Initialize & Add Root CA (Server Verification)
    status = nx_secure_x509_certificate_initialize(trusted_certificate_ptr,
        (UCHAR*)root_ca_der,
        (USHORT)root_ca_len,
        NX_NULL,
        0,
        NULL,
        0,
        NX_SECURE_X509_KEY_TYPE_NONE);
    if (status != NX_SUCCESS)
    {
        printf("[TLS] Root CA Init Failed (0x%X)\r\n", status);
        return status;
    }

    status = nx_secure_tls_trusted_certificate_add(tls_session_ptr, trusted_certificate_ptr);
    if (status != NX_SUCCESS)
        return status;

    printf("[TLS] Setup Complete.\r\n");
    return NX_SUCCESS;
}

bool MQTT_Init(void)
{
    UINT status;

    screen_print("Init MQTT...", L0);
    printf("Initializing Network...\r\n");

    // 1. Initialize Network
    status = network_init((CHAR*)WIFI_SSID, (CHAR*)WIFI_PASSWORD, WPA2_PSK_AES);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: network_init failed (0x%08X)\r\n", status);
        return false;
    }

    // 2. Connect WiFi
    printf("Connecting to WiFi: %s\r\n", WIFI_SSID);
    status = network_connect();
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to connect to WiFi\r\n");
        return false;
    }
    printf("WiFi Connected!\r\n");

    // 3. Create MQTT Client
    status = nxd_mqtt_client_create(&mqtt_client,
        "MXChipMQTT",
        MQTT_CLIENT_ID,
        strlen(MQTT_CLIENT_ID),
        &nx_ip,
        &nx_pool[0],
        mqtt_stack,
        sizeof(mqtt_stack),
        MQTT_THREAD_PRIORITY,
        NX_NULL,
        0);

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("ERROR: MQTT Create failed (0x%08X)\r\n", status);
        return false;
    }

    // 4. Resolve Hostname
    NXD_ADDRESS server_ip;
    server_ip.nxd_ip_version = NX_IP_VERSION_V4;

    printf("Resolving: %s\r\n", MQTT_BROKER_HOSTNAME);
    ULONG ip_resolved;
    status = nx_dns_host_by_name_get(&nx_dns_client, (UCHAR*)MQTT_BROKER_HOSTNAME, &ip_resolved, 500); // 5s wait

    if (status != NX_SUCCESS)
    {
        printf("ERROR: DNS Resolution failed (0x%08X)\r\n", status);
        printf("Attempting Hardcoded IP check...(Disabled)\r\n");
        return false;
    }
    server_ip.nxd_ip_address.v4 = ip_resolved;
    printf("Resolved IP: %lu.%lu.%lu.%lu\r\n",
        (ip_resolved >> 24) & 0xFF,
        (ip_resolved >> 16) & 0xFF,
        (ip_resolved >> 8) & 0xFF,
        ip_resolved & 0xFF);

    // 5. Connect Securely
    printf("Connecting to MQTT Broker (Secure Port %d)...\r\n", MQTT_BROKER_PORT);
    screen_print("Connecting...", L2);

    // Set Login (User/Pass) for Event Grid
    nxd_mqtt_client_login_set(
        &mqtt_client, MQTT_USERNAME, strlen(MQTT_USERNAME), (CHAR*)MQTT_PASSWORD, strlen(MQTT_PASSWORD));

    // Connect using Secure API
    status = nxd_mqtt_client_secure_connect(
        &mqtt_client, &server_ip, MQTT_BROKER_PORT, mqtt_tls_setup, 60, NX_TRUE, NX_WAIT_FOREVER);

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("ERROR: MQTT Connect failed (0x%08X)\r\n", status);
        screen_print("Connect Fail", L2);
        nxd_mqtt_client_delete(&mqtt_client);
        return false;
    }

    printf("MQTT Connected!\r\n");
    screen_print("Connected!", L2);
    is_connected = true;

    nxd_mqtt_client_receive_notify_set(&mqtt_client, mqtt_message_callback);
    MQTT_Subscribe(MQTT_SUB_TOPIC);

    return true;
}

bool MQTT_Publish(const char* topic, const char* message)
{
    if (!is_connected)
        return false;
    printf("Pub: %s\r\n", topic);
    UINT status = nxd_mqtt_client_publish(
        &mqtt_client, (CHAR*)topic, strlen(topic), (CHAR*)message, strlen(message), NX_FALSE, 0, NX_WAIT_FOREVER);
    if (status == NXD_MQTT_SUCCESS)
        publish_count++;
    return (status == NXD_MQTT_SUCCESS);
}

bool MQTT_Subscribe(const char* topic)
{
    if (!is_connected)
        return false;
    printf("Sub: %s\r\n", topic);
    return (nxd_mqtt_client_subscribe(&mqtt_client, (CHAR*)topic, strlen(topic), 0) == NXD_MQTT_SUCCESS);
}

void MQTT_Check_Message(void)
{
    // Polling handled by callback or thread logic
}

const char* MQTT_Get_Last_Message(void)
{
    return last_received_message;
}
const char* MQTT_Get_Last_Topic(void)
{
    return last_received_topic;
}

int MQTT_Get_Publish_Count(void)
{
    return publish_count;
}
int MQTT_Get_Receive_Count(void)
{
    return receive_count;
}

static void mqtt_message_callback(NXD_MQTT_CLIENT* client_ptr, UINT message_count)
{
    (void)message_count;
    UINT topic_length, message_length;
    UCHAR topic_buffer[128];
    UCHAR message_buffer[256];

    if (nxd_mqtt_client_message_get(client_ptr,
            topic_buffer,
            sizeof(topic_buffer),
            &topic_length,
            message_buffer,
            sizeof(message_buffer),
            &message_length) == NXD_MQTT_SUCCESS)
    {
        if (topic_length < sizeof(last_received_topic))
        {
            memcpy(last_received_topic, topic_buffer, topic_length);
            last_received_topic[topic_length] = 0;
        }
        if (message_length < sizeof(last_received_message))
        {
            memcpy(last_received_message, message_buffer, message_length);
            last_received_message[message_length] = 0;
        }
        printf("[Rx] %s: %s\r\n", last_received_topic, last_received_message);
        receive_count++;

        // Simple LED Control
        if (strstr((char*)message_buffer, "ON"))
            USER_LED_ON();
        if (strstr((char*)message_buffer, "OFF"))
            USER_LED_OFF();
    }
}
