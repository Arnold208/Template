#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "nxd_mqtt_client.h"
#include "cloud_config.h"  // ✅ Ensure cloud_config.h is included

// #define MQTT_BROKER_IP       "18.134.118.11"
#define MQTT_BROKER_PORT     1883

#define MQTT_STACK_SIZE 4096
#define MQTT_THREAD_PRIORITY 3

// Function declarations
void mqtt_init();                               // Initializes MQTT client
void mqtt_connect();                            // Connects to MQTT broker
void mqtt_publish(const char *topic, const char *msg);  // Publishes to any topic
char* mqtt_subscribe(const char *topic);        // Subscribes and returns received message
void mqtt_callback(NXD_MQTT_CLIENT *client, UINT num_messages); // Callback for messages

#endif /* MQTT_CLIENT_H */
