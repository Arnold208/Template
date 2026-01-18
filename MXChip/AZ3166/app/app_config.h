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

// MQTT Broker Hostname (Replace with your Event Grid Namespace MQTT hostname)
// Example: your-namespace.westeurope-1.ts.eventgrid.azure.net
#define MQTT_BROKER_HOSTNAME "your-eventgrid-namespace.ts.eventgrid.azure.net"

// Set to 1 for Secure MQTT (TLS/8883), 0 for TCP/1883
#define MQTT_SECURE_CONNECTION 1

#if MQTT_SECURE_CONNECTION
    #define MQTT_BROKER_PORT  8883
#else
    #define MQTT_BROKER_PORT  1883
#endif

#define MQTT_CLIENT_ID    "mxchip-test-eventgrid"
#define MQTT_PUB_TOPIC    "message/fromnanoframework"  
#define MQTT_SUB_TOPIC    "message/device/#"          

#define MQTT_TOPIC_BUTTON_A "message/buttonA"
#define MQTT_TOPIC_BUTTON_B "message/buttonB"

#endif // APP_CONFIG_H
