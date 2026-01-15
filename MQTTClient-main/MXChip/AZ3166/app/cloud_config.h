/* 
 * Copyright (c) Microsoft
 * Copyright (c) 2024 Eclipse Foundation
 * 
 *  This program and the accompanying materials are made available 
 *  under the terms of the MIT license which is available at
 *  https://opensource.org/license/mit.
 * 
 *  SPDX-License-Identifier: MIT
 * 
 *  Contributors: 
 *     Microsoft         - Initial version
 *     Frédéric Desbiens - 2024 version.
 */

#ifndef _CLOUD_CONFIG_H
#define _CLOUD_CONFIG_H

typedef enum
{
    None         = 0,
    WEP          = 1,
    WPA_PSK_TKIP = 2,
    WPA2_PSK_AES = 3
} WiFi_Mode;

// MQTT Configuration
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "mxchip_client"
#define MQTT_BROKER_IP  "18.134.118.11"   
 
// ----------------------------------------------------------------------------
// WiFi connection config
// ----------------------------------------------------------------------------
#define HOSTNAME      "eclipse-threadx"  //Change to unique hostname.
#define WIFI_SSID     "SENSE-IOT-NETWORK"
#define WIFI_PASSWORD "AfriSense1255" 
#define WIFI_MODE     WPA2_PSK_AES

// ----------------------------------------------------------------------------
// MQTT Config
// ----------------------------------------------------------------------------


#endif // _CLOUD_CONFIG_H
