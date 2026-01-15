<!--
  Copyright (c) 2024 Eclipse Foundation
 
  This program and the accompanying materials are made available 
  under the terms of the MIT license which is available at
  https://opensource.org/license/mit.
 
  SPDX-License-Identifier: MIT
 
  Contributors: 
      Frédéric Desbiens - Initial version.
      Andy Riexinger - Documentation for Mac M1.
      Kimkpe Arnold Sylvian - Updated for MQTT & IoT Use Cases
-->

# **Eclipse ThreadX IoT DevKit - MXChip AZ3166 MQTT Example**

This project demonstrates how to use the **MXChip AZ3166** IoT DevKit board to **connect to an MQTT broker, publish and subscribe to topics, send sensor telemetry, and act as a smart switch**.

This example is based on **Eclipse ThreadX RTOS** and **NetX Duo** for networking. It showcases how to:
- **Connect the MXChip AZ3166 to a Wi-Fi network**
- **Publish messages to an MQTT broker**
- **Subscribe to an MQTT topic and handle received messages**
- **Use the MXChip as a sensor and publish telemetry data (e.g., temperature, humidity, acceleration)**
- **Control devices by acting as a smart switch (e.g., sending ON/OFF commands)**

---

## **📥 Cloning the Repository**
The repository contains **Eclipse ThreadX and NetX Duo** as submodules. Clone it with:

```sh
git clone --recurse-submodules https://github.com/Arnold208/MQTTCLient
git submodule init
git submodule update --recursive
```

---

## **🚀 Project Overview**
### **Supported Features**
This project enables the **MXChip AZ3166** to function as:

1. **IoT Sensor**  
   - Reads sensor data (temperature, humidity, motion) and publishes to an MQTT broker.  

2. **Smart Switch**  
   - Uses **Button A to turn ON** and **Button B to turn OFF** a smart device via MQTT messages.  

3. **MQTT Client**  
   - Publishes and subscribes to MQTT topics dynamically.  

---

## **🔧 Setting Up the MXChip for MQTT**
### **1️⃣ Configure Wi-Fi & MQTT**
Edit `cloud_config.h` to set up **Wi-Fi credentials** and **MQTT broker details**:

```c
#define WIFI_SSID "Your_WiFi_Name"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define WIFI_MODE None  // Set to 'None' for an open network

#define MQTT_BROKER_IP "18.134.118.11"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "mxchip_client"
```

---

## **📡 Connecting to an MQTT Broker**
### **2️⃣ Publishing and Subscribing to Topics**
The **MXChip can publish and subscribe to topics dynamically**.

### **✅ Publish a JSON Message**
```c
mqtt_publish("office/smart_extension", "{\"socket1\": \"ON\"}");
```
This sends:
```json
{"socket1": "ON"}
```

### **✅ Subscribe to a Topic and Handle Data**
```c
char *message = mqtt_subscribe("office/smart_extension");
printf("Received: %s\n", message);
```

---

## **🛰️ Use Cases**
### **🌡️ MXChip as a Sensor (Publishing Telemetry Data)**
Use the MXChip’s **HTS221 sensor** to **publish temperature & humidity data** to an MQTT broker.

```c
char telemetry[100];
float temperature = get_temperature();
float humidity = get_humidity();
snprintf(telemetry, sizeof(telemetry), "{\"temperature\": %.2f, \"humidity\": %.2f}", temperature, humidity);
mqtt_publish("office/sensor_data", telemetry);
```
This sends:
```json
{"temperature": 22.5, "humidity": 45.2}
```

---

### **💡 MXChip as a Smart Switch (Controlling Devices)**
Use **Button A to turn ON** and **Button B to turn OFF** a smart device.

```c
if (BUTTON_A_IS_PRESSED)
{
    mqtt_publish("office/smart_extension", "{\"socket1\": \"ON\"}");
    screen_print("ON", L1);
}

if (BUTTON_B_IS_PRESSED)
{
    mqtt_publish("office/smart_extension", "{\"socket1\": \"OFF\"}");
    screen_print("OFF", L1);
}
```

---

## **📌 Next Steps**
1. **Flash the firmware to the MXChip**  
2. **Test MQTT publish & subscribe**  
3. **Integrate with a smart home system**  
4. **Expand by adding more sensors or IoT controls**  

---

## **📜 Resources**
- [Eclipse ThreadX Docs](https://github.com/eclipse-threadx/rtos-docs)
- [STMicroelectronics STM32F412RG](https://www.st.com/en/microcontrollers-microprocessors/stm32f412rg.html)

---

### **🎯 Have questions? Let us know!**