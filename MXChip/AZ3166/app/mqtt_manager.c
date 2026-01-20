#include "mqtt_manager.h"
#include "app_config.h"
#include "networking.h"
#include "nx_secure_tls_api.h"
#include "nxd_mqtt_client.h"
#include "user_credentials.h"


// --- Constants ---
#define MQTT_CLIENT_STACK_SIZE 4096
#define MQTT_CLIENT_PRIORITY   2

// --- Globals ---
static NXD_MQTT_CLIENT mqtt_client;
static ULONG mqtt_client_stack[MQTT_CLIENT_STACK_SIZE / sizeof(ULONG)];

// TLS Session
static NX_SECURE_TLS_SESSION tls_session;
static NX_SECURE_X509_CERT root_ca_cert;
static NX_SECURE_X509_CERT device_cert;
// Packet buffer - User log mentions 16384 bytes
static UCHAR tls_packet_buffer[16384];

// Externs from networking
extern NX_IP nx_ip;
extern NX_PACKET_POOL nx_pool[2]; // 0=TX, 1=RX
extern NX_DNS nx_dns_client;

// Message Callback
static void mqtt_receive_notify(NXD_MQTT_CLIENT* client_ptr, UINT message_count)
{
    // Minimal callback
}

// TLS Setup Callback
UINT mqtt_tls_setup(NXD_MQTT_CLIENT* client_ptr,
    NX_SECURE_TLS_SESSION* tls_session_ptr,
    NX_SECURE_X509_CERT* certificate_ptr,
    NX_SECURE_X509_CERT* trusted_certificate_ptr)
{
    UINT status;

    printf("[TLS] Creating TLS Session (buffer: %d bytes)...\n", sizeof(tls_packet_buffer));

    // Create TLS session
    status = nx_secure_tls_session_create(
        tls_session_ptr, &nx_crypto_tls_ciphers, nx_crypto_tls_metadata, sizeof(nx_crypto_tls_metadata));
    if (status != NX_SUCCESS)
    {
        printf("[TLS] ERROR: Session create failed (0x%08X)\n", status);
        return status;
    }
    printf("[TLS] Session created successfully\n");

    // Set packet buffer
    printf("[TLS] Setting packet buffer (%d bytes)...\n", sizeof(tls_packet_buffer));
    status = nx_secure_tls_session_packet_buffer_set(tls_session_ptr, tls_packet_buffer, sizeof(tls_packet_buffer));
    if (status != NX_SUCCESS)
    {
        printf("[TLS] ERROR: Buffer set failed (0x%08X)\n", status);
        return status;
    }
    printf("[TLS] Packet buffer set successfully\n");

    // 1. Device Cert (Client)
    // User logs say: "[CERT] Initializing device certificate (EC, 496 bytes, key: 121 bytes)..."
    // I MUST use EC key type here.
    printf("[CERT] Initializing device certificate (EC)...\n");
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
        printf("[CERT] ERROR: Device Cert Init Failed (0x%08X)\n", status);
        return status;
    }
    printf("[CERT] Device certificate initialized\n");

    // 2. Add Device Cert
    printf("[CERT] Adding device certificate to TLS session...\n");
    status = nx_secure_tls_local_certificate_add(tls_session_ptr, certificate_ptr);
    if (status != NX_SUCCESS)
    {
        printf("[CERT] ERROR: Add Device Cert Failed (0x%08X)\n", status);
        return status;
    }
    printf("[CERT] Device certificate added\n");

    // 3. Root CA
    printf("[CERT] Initializing Root CA...\n");
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
        printf("[CERT] ERROR: Root CA Init Failed (0x%08X)\n", status);
        return status;
    }
    printf("[CERT] Root CA initialized\n");

    // 4. Add Root CA
    printf("[CERT] Adding Root CA to trusted certificates...\n");
    status = nx_secure_tls_trusted_certificate_add(tls_session_ptr, trusted_certificate_ptr);
    if (status != NX_SUCCESS)
    {
        printf("[CERT] ERROR: Add Root CA Failed (0x%08X)\n", status);
        return status;
    }
    printf("[CERT] Root CA added to trusted certificates\n");

    printf("[TLS] TLS setup completed successfully\n");
    return NX_SUCCESS;
}

bool MQTT_Init()
{
    UINT status;

    // Create Client
    status = nxd_mqtt_client_create(&mqtt_client,
        "MXChip Client",
        &nx_ip,
        &nx_pool[0],
        mqtt_client_stack,
        MQTT_CLIENT_STACK_SIZE,
        MQTT_CLIENT_PRIORITY,
        NX_NULL,
        0);

    // Set Login
    printf("[MQTT] Setting login credentials (user: %s)...\n", MQTT_CLIENT_ID);
    nxd_mqtt_client_login_set(
        &mqtt_client, MQTT_CLIENT_ID, MQTT_CLIENT_ID, NULL); // Auth Name, User, Pass(NULL for mTLS)
    printf("[MQTT] Login credentials set\n");

    // Resolve IP
    NXD_ADDRESS server_ip_addr;
    server_ip_addr.nxd_ip_version = NX_IP_VERSION_V4;

    // Attempt DNS
    /*
    printf("[DNS] Resolving %s...\n", MQTT_BROKER_HOSTNAME);
    status = nxd_dns_host_by_name_get(&nx_dns_client, (UCHAR*)MQTT_BROKER_HOSTNAME, &server_ip_addr, 500); // 5s
    if (status == NX_SUCCESS)
    {
         printf("[DNS] Resolved: %lu.%lu.%lu.%lu\n",
            (server_ip_addr.nxd_ip_address.v4 >> 24),
            (server_ip_addr.nxd_ip_address.v4 >> 16) & 0xFF,
            (server_ip_addr.nxd_ip_address.v4 >> 8) & 0xFF,
            (server_ip_addr.nxd_ip_address.v4) & 0xFF);
    }
    else
    {
         printf("[DNS] Resolution Failed (0x%X). Using Hardcoded IP...\n", status);
         server_ip_addr.nxd_ip_address.v4 = IP_ADDRESS(20, 42, 7, 73);
    }
    */
    // User wants hardcoded IP logic directly as per log: "Using Hardcoded IP..."
    printf("Using Hardcoded IP...\n");
    printf("IP Set to: 20.42.7.73\n");
    server_ip_addr.nxd_ip_version    = NX_IP_VERSION_V4;
    server_ip_addr.nxd_ip_address.v4 = IP_ADDRESS(20, 42, 7, 73);

    // Connect
    printf("Connecting to Secure MQTT Broker %s:%d...\n", MQTT_BROKER_HOSTNAME, MQTT_BROKER_PORT);
    printf("[MQTT] Initiating secure connection (timeout: forever)...\n");

    status = nxd_mqtt_client_secure_connect(&mqtt_client,
        &server_ip_addr,
        MQTT_BROKER_PORT,
        mqtt_tls_setup,
        MQTT_KEEP_ALIVE_VALUE,
        MQTT_CLEAN_SESSION,
        NX_WAIT_FOREVER); // Log says forever

    if (status != NXD_MQTT_SUCCESS)
    {
        printf("[MQTT] ERROR: Connect failed (0x%08X)\n", status);
        printf("[MQTT] Possible causes:\n");
        printf("[MQTT]   - Server rejected client certificate\n");
        printf("[MQTT]   - Invalid username/password\n");
        printf("[MQTT]   - Client ID not authorized\n");
        printf("[MQTT]   - MQTT protocol version mismatch\n");
        return false;
    }

    nxd_mqtt_client_subscribe(&mqtt_client, MQTT_SUB_TOPIC, QOS0);
    return true;
}

bool MQTT_Publish(const char* topic, const char* message)
{
    return (nxd_mqtt_client_publish(
                &mqtt_client, (char*)topic, strlen(topic), (char*)message, strlen(message), 0, QOS0, 100) ==
            NXD_MQTT_SUCCESS);
}

void MQTT_Check_Message()
{
    // Implementation for polling
}

const char* MQTT_Get_Last_Message()
{
    return "";
}
