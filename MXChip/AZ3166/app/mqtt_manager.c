#include "mqtt_manager.h"
#include "app_config.h"
#include "board_init.h"
#include "nxd_mqtt_client.h"
#include "screen.h"
#include <stdio.h>
#include <string.h>

#if MQTT_SECURE_CONNECTION
#include "nx_secure_tls_api.h"
#include "user_credentials.h"

static NX_SECURE_X509_CERT device_certificate;
static NX_SECURE_X509_CERT root_ca_certificate;
extern const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;
#endif

#include "nxd_dns.h"

// --- Externs ---
extern NX_IP nx_ip;
extern NX_PACKET_POOL nx_pool[2];
extern NX_DNS nx_dns_client;

// --- MQTT Client State ---
static NXD_MQTT_CLIENT az_mqtt_client;
static NXD_ADDRESS server_ip;
static bool is_connected = false;

// TLS buffers
#if MQTT_SECURE_CONNECTION
static CHAR tls_metadata_buffer[8192];
static UCHAR tls_packet_buffer[4096 * 4];
#endif

// MQTT Stack
static ULONG mqtt_client_stack[2048 / sizeof(ULONG)];

static uint32_t publish_count          = 0;
static uint32_t receive_count          = 0;
static char last_received_topic[128]   = {0};
static char last_received_message[256] = {0};

// Forward declaration
static void mqtt_message_callback(NXD_MQTT_CLIENT* client_ptr, UINT message_count);

#if MQTT_SECURE_CONNECTION
// TLS Setup Callback
static UINT tls_setup_callback(NXD_MQTT_CLIENT* client_ptr,
    NX_SECURE_TLS_SESSION* tls_session,
    NX_SECURE_X509_CERT* cert_ptr,
    NX_SECURE_X509_CERT* trusted_cert_ptr)
{
    UINT status;

    // Initialize and create TLS session.
    printf("Creating TLS Session...\r\n");
    status = nx_secure_tls_session_create(
        tls_session, &nx_crypto_tls_ciphers, tls_metadata_buffer, sizeof(tls_metadata_buffer));

    if (status != NX_SUCCESS)
    {
        printf("ERROR: TLS Session Create failed (0x%08X)\r\n", status);
        return status;
    }

    // Allocate space for packet reassembly.
    status = nx_secure_tls_session_packet_buffer_set(tls_session, tls_packet_buffer, sizeof(tls_packet_buffer));

    if (status != NX_SUCCESS)
    {
        printf("ERROR: TLS Packet Buffer Set failed (0x%08X)\r\n", status);
        return status;
    }

    // 2. Initialize Device Certificate (with Private Key)
    status = nx_secure_x509_certificate_initialize(&device_certificate,
        (UCHAR*)device_cert_der,
        (USHORT)device_cert_len,
        NX_NULL,
        0,
        (UCHAR*)device_key_der,
        (USHORT)device_key_len,
        NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Device Cert Init failed (0x%08X)\r\n", status);
        return status;
    }

    // 4. Initialize Root CA
    status = nx_secure_x509_certificate_initialize(&root_ca_certificate,
        (UCHAR*)root_ca_der,
        (USHORT)root_ca_len,
        NX_NULL,
        0,
        NULL,
        0,
        NX_SECURE_X509_KEY_TYPE_NONE);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Root CA Init failed (0x%08X)\r\n", status);
        return status;
    }

    // Add device certificate
    status = nx_secure_tls_local_certificate_add(tls_session, &device_certificate);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to add device certificate (0x%08X)\r\n", status);
        return status;
    }

    // Add root CA
    status = nx_secure_tls_trusted_certificate_add(tls_session, &root_ca_certificate);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to add root CA (0x%08X)\r\n", status);
        return status;
    }

    return NX_SUCCESS;
}
#endif

bool MQTT_Init(void)
{
    UINT status;

    // 1. WiFi is already initialized in main.c or by calling network_init

    // 2. Create MQTT Client
    status = nxd_mqtt_client_create(&az_mqtt_client,
        "AZ_MQTT_Client",
        MQTT_CLIENT_ID,
        strlen(MQTT_CLIENT_ID),
        &nx_ip,
        &nx_pool[0], // TX Pool
        (VOID*)mqtt_client_stack,
        sizeof(mqtt_client_stack),
        4, // Priority
        NX_NULL,
        0);

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("ERROR: MQTT Client creation failed (0x%08X)\r\n", status);
        return false;
    }

    // 3. Resolve Broker IP
    printf("Resolving %s...\r\n", MQTT_BROKER_HOSTNAME);
    status = nxd_dns_host_by_name_get(
        &nx_dns_client, (UCHAR*)MQTT_BROKER_HOSTNAME, &server_ip, NX_WAIT_FOREVER, NX_DNS_RR_TYPE_A);

    if (status != NX_SUCCESS)
    {
        printf("ERROR: IP Resolution failed (0x%08X)\r\n", status);
        return false;
    }

    // 4. Connect
#if MQTT_SECURE_CONNECTION
    printf("Connecting to Secure MQTT Broker %s:%d...\r\n", MQTT_BROKER_HOSTNAME, MQTT_BROKER_PORT);
    screen_print("TLS Connecting...", (int)L2);

    status = nxd_mqtt_client_secure_connect(
        &az_mqtt_client, &server_ip, MQTT_BROKER_PORT, tls_setup_callback, 60, NX_TRUE, NX_WAIT_FOREVER);
#else
    printf("Connecting to MQTT Broker %s:%d...\r\n", MQTT_BROKER_HOSTNAME, MQTT_BROKER_PORT);
    screen_print("Connecting...", (int)L2);
    status = nxd_mqtt_client_connect(&az_mqtt_client, &server_ip, MQTT_BROKER_PORT, 60, NX_TRUE, NX_WAIT_FOREVER);
#endif

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("ERROR: MQTT Connect failed (0x%08X)\r\n", status);
        nxd_mqtt_client_delete(&az_mqtt_client);
        screen_print("MQTT Conn Fail", (int)L2);
        return false;
    }

    printf("MQTT Connected!\r\n");
    screen_print("MQTT Connected", (int)L2);
    is_connected = true;

    // Set callback
    nxd_mqtt_client_receive_notify_set(&az_mqtt_client, mqtt_message_callback);

    // Auto-subscribe
    MQTT_Subscribe(MQTT_SUB_TOPIC);

    return true;
}

bool MQTT_Publish(const char* topic, const char* message)
{
    if (!is_connected)
        return false;

    printf("Publishing to '%s': %s\r\n", topic, message);
    UINT status = nxd_mqtt_client_publish(&az_mqtt_client,
        (CHAR*)topic,
        (UINT)strlen(topic),
        (CHAR*)message,
        (UINT)strlen(message),
        NX_FALSE,
        0,
        NX_WAIT_FOREVER);

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
    UINT status = nxd_mqtt_client_subscribe(&az_mqtt_client, (CHAR*)topic, (UINT)strlen(topic), 0);

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
    // NetX Duo handles messages via callback
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

        // LED Control commands
        if (strstr((char*)message_buffer, "\"led\": \"ON\"") != NULL ||
            strstr((char*)message_buffer, "\"led\":\"ON\"") != NULL)
        {
            printf("[Command] LED ON\r\n");
            USER_LED_ON();
        }
        else if (strstr((char*)message_buffer, "\"led\": \"OFF\"") != NULL ||
                 strstr((char*)message_buffer, "\"led\":\"OFF\"") != NULL)
        {
            printf("[Command] LED OFF\r\n");
            USER_LED_OFF();
        }
    }
}

int MQTT_Get_Publish_Count(void)
{
    return (int)publish_count;
}

int MQTT_Get_Receive_Count(void)
{
    return (int)receive_count;
}
