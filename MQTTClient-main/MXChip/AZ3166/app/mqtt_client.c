#include <stdio.h>
#include <string.h>
#include "tx_api.h"
#include "nxd_mqtt_client.h"
#include "wwd_networking.h"
#include "mqtt_client.h"
#include "cloud_config.h"  // Ensure this is included for MQTT_CLIENT_ID

static NXD_MQTT_CLIENT mqtt_client;
static UCHAR mqtt_stack[MQTT_STACK_SIZE];
static char received_message[256];  // Store received message

// Function to initialize MQTT
void mqtt_init()
{
    printf("Initializing MQTT client...\n");
}

// Function to connect to MQTT broker
void mqtt_connect()
{
    UINT status;
    NXD_ADDRESS broker_address;

    // Set MQTT broker IP
    broker_address.nxd_ip_version = NX_IP_VERSION_V4;
    broker_address.nxd_ip_address.v4 = IP_ADDRESS(18, 134, 118, 11);

    // Create MQTT client
    status = nxd_mqtt_client_create(
        &mqtt_client, 
        MQTT_CLIENT_ID, 
        MQTT_CLIENT_ID, strlen(MQTT_CLIENT_ID),  
        &nx_ip, &nx_pool[0], 
        mqtt_stack, MQTT_STACK_SIZE, 
        MQTT_THREAD_PRIORITY, 
        NX_NULL, 0  
    );

    if (status != NX_SUCCESS)
    {
        printf("ERROR: MQTT client creation failed (0x%08x)\n", status);
        return;
    }

    // Connect to broker
    status = nxd_mqtt_client_connect(&mqtt_client, &broker_address, 
                                     MQTT_BROKER_PORT, 60, NX_TRUE, NX_WAIT_FOREVER);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: MQTT connection failed (0x%08x)\n", status);
        return;
    }

    printf("MQTT connected successfully!\n");
}

// Function to publish a message to a given topic
void mqtt_publish(const char *topic, const char *msg)
{
    UINT status = nxd_mqtt_client_publish(&mqtt_client, 
                                          (CHAR *)topic, strlen(topic),  // Cast `topic` to `CHAR *`
                                          (CHAR *)msg, strlen(msg),      // Cast `msg` to `CHAR *`
                                          NX_FALSE, 0, NX_WAIT_FOREVER);

    if (status != NX_SUCCESS)
    {
        printf("ERROR: MQTT publish failed (0x%08x)\n", status);
    }
    else
    {
        printf("MQTT message published to %s: %s\n", topic, msg);
    }
}


// Callback function to handle received messages
void mqtt_callback(NXD_MQTT_CLIENT *client, UINT num_messages)
{
    UINT topic_length, message_length;
    CHAR topic_buffer[100];

    // Read the received message
    nxd_mqtt_client_message_get(client, (UCHAR*)topic_buffer, sizeof(topic_buffer), &topic_length,
                                (UCHAR*)received_message, sizeof(received_message), &message_length);

    // Ensure strings are null-terminated
    topic_buffer[topic_length] = '\0';
    received_message[message_length] = '\0';

    printf("Received message on topic %s: %s\n", topic_buffer, received_message);
}

// Function to subscribe to a given topic and return the received message
char* mqtt_subscribe(const char *topic)
{
    UINT status = nxd_mqtt_client_subscribe(&mqtt_client, 
                                            (CHAR *)topic, strlen(topic), 0); // Cast `topic` to `CHAR *`

    if (status != NX_SUCCESS)
    {
        printf("ERROR: MQTT subscribe failed (0x%08x)\n", status);
        return NULL;
    }
    else
    {
        printf("Subscribed to topic: %s\n", topic);
        nxd_mqtt_client_receive_notify_set(&mqtt_client, mqtt_callback);
        return received_message;  // Return received message after callback
    }
}

