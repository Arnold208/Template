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

// --- Azure Event Grid V2 MQTT Settings ---

// 1. MQTT Broker Hostname (Event Grid Namespace MQTT Hostname)
// Example: "semor-eventgrid.eastus-1.ts.eventgrid.azure.net"
#define MQTT_BROKER_HOSTNAME "your-eventgrid-namespace.region.ts.eventgrid.azure.net"

// 2. MQTT Broker Port (Secure MQTT is 8883)
#define MQTT_BROKER_PORT  8883

// 3. Client ID (Device Name in Event Grid)
// Must match the client certificate Subject Name (CN) or SAN
#define MQTT_CLIENT_ID    "mxchip-sensor-01"

// 4. Username (Required by Event Grid)
// Format varies, but often same as Client ID or specific format for custom authn.
// For mTLS, this is checked against the certificate.
#define MQTT_USERNAME     "mxchip-sensor-01"

// 5. Password (Usually empty for mTLS, required for Token/SAS)
#define MQTT_PASSWORD     ""

// 6. Topics (Event Grid Namespace Topic Structure)
// Publish Topic: devices/{deviceId}/messages/events
#define MQTT_PUB_TOPIC    "devices/mxchip-sensor-01/messages/events"

// Subscribe Topic: devices/{deviceId}/messages/c2d
#define MQTT_SUB_TOPIC    "devices/mxchip-sensor-01/messages/c2d"

// Button Topics
#define MQTT_TOPIC_BUTTON_A "devices/mxchip-sensor-01/messages/buttonA"
#define MQTT_TOPIC_BUTTON_B "devices/mxchip-sensor-01/messages/buttonB"

// --- Secure Connection Flag ---
// Set to 1 to enable TLS/SSL (Required for Event Grid)
#define MQTT_SECURE_CONNECTION 1

#endif // APP_CONFIG_H
