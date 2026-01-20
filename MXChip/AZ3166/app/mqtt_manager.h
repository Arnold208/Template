#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdbool.h>

// Initialize MQTT (and Network/TLS internally)
bool MQTT_Init(void);

// Check for incoming messages
void MQTT_Check_Message(void);

// Publish a message
bool MQTT_Publish(const char* topic, const char* message);

// Get the last received message
const char* MQTT_Get_Last_Message(void);

#endif // MQTT_MANAGER_H
