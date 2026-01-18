/**
 ******************************************************************************
 * @file    app_config.h
 * @author  Microsoft Corporation & Contributors
 * @brief   User overrides for WiFi and MQTT configuration.
 ******************************************************************************
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// --- WiFi Settings ---
// REPLACE WITH YOUR WIFI CREDENTIALS
#define WIFI_SSID       ""
#define WIFI_PASSWORD   ""

// Set to 1 to use Hostname, 0 to use IP Address
#define MQTT_USE_HOSTNAME  0

// Option A: IP Address String (broker.hivemq.com)
#define MQTT_BROKER_IP_STRING "52.58.30.91"

// Option B: Hostname (e.g., "test.mosquitto.org")
#define MQTT_BROKER_HOSTNAME "broker.hivemq.com"

#define MQTT_BROKER_PORT  1883

#define MQTT_CLIENT_ID    "MXChip_Sensor_Node"
#define MQTT_PUB_TOPIC    "mxchip/data"     // Default publish topic
#define MQTT_SUB_TOPIC    "mxchip/commands" // Default subscribe topic

#define MQTT_TOPIC_BUTTON_A "mxchip/buttonA"
#define MQTT_TOPIC_BUTTON_B "mxchip/buttonB"

#endif // APP_CONFIG_H
