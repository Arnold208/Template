/**
 ******************************************************************************
 * @file    mqtt_manager.h
 * @author  Microsoft Corporation & Contributors
 * @brief   Simplified MQTT Manager for AZ3166
 ******************************************************************************
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  Initializes WiFi, connects to the Internet, and connects to the MQTT broker.
 * @return true if successful, false otherwise.
 */
bool MQTT_Init(void);

// Generic Publish Function
// Returns true if successful, false otherwise
bool MQTT_Publish(const char* topic, const char* message);

// Generic Subscribe Function
// Returns true if successful, false otherwise
bool MQTT_Subscribe(const char* topic);

/**
 * @brief  Checks for incoming MQTT messages and keeps the connection alive.
 *         Should be called periodically in the main loop.
 */
void MQTT_Check_Message(void);

// Optional: Get last received message (simple buffer access)
// In a real app, you might use a callback registration system
const char* MQTT_Get_Last_Message(void);
const char* MQTT_Get_Last_Topic(void);

// MQTT Statistics
int MQTT_Get_Publish_Count(void);
int MQTT_Get_Receive_Count(void);

#endif // MQTT_MANAGER_H
