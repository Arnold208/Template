# MXChip AZ3166 Smart Sensor Node

Welcome to the **MXChip AZ3166 Smart Sensor Node** project. This comprehensive firmware implementation transforms the MXChip IoT DevKit into a robust, cloud-connected sensor monitoring station. It is designed to demonstrate best practices in embedded systems development, distinctively featuring modular architecture, real-time multitasking, and interactive user interfaces.

## Project Overview

This project serves as a production-grade template for building connected devices using the **Azure RTOS (NetX Duo & ThreadX)** stack. Beyond simple demonstrations, it provides a stable foundation for capturing environmental data (Temperature, Humidity, Pressure, Motion) and transmitting it to any MQTT-compatible cloud broker using a flexible, generic data payload system.

## Key Features

### 1. Advanced Sensor Integration
- **Real-Time Monitoring**: Continuous polling of on-board sensors including HTS221 (Temperature/Humidity), LPS22HB (Pressure), and LSM6DSL (Accelerometer/Gyroscope).
- **Data Abstraction**: A clean `simple_sensor` layer abstracts hardware complexity, providing easy-to-use API calls for application logic.

### 2. Interactive User Interface
- **OLED Display**: A rich, paged interface allows users to cycle through different sensor views using the on-board Buttons (A and B).
- **Status Feedback**: Clear visual indicators for network connectivity and system status.
- **MQTT Stats Screen**: Dedicated screen showing Connection Status, Publish Counts (P), and Receive Counts (S).

### 3. Generic Cloud Connectivity
- **Robust MQTT Client**: A custom-built, generic MQTT manager capable of establishing reliable connections to any standard MQTT broker.
- **Configurable Connection**: Logic to support both direct IP addresses and Hostname resolution (DNS) for the broker.
- **Universal Payload Support**: Sreams Sensor Data as **JSON** objects.
- **Bi-directional Communication**: The device not only publishes telemetry but also subscribes to topics, enabling remote command execution.
- **Remote LED Control**: Control the User LED remotely via MQTT JSON commands.

## Hardware Requirements

- **MXChip IoT DevKit (AZ3166)**: An all-in-one Arduino-compatible board with rich peripherals.
- Micro-USB Cable for power and debugging.

## Getting Started

### Prerequisites
1.  **Visual Studio Code**: Ensure you have the latest version installed.
2.  **MXChip Toolchain**: Install the `MXCHIP AZ1366 Toolbox` extension for VSCode.
3.  **Drivers**: Use the `Mxchip Install Drivers` command in VSCode to set up CMake and Ninja.

### Configuration
Before building, you must configure your network and broker settings. Open `app/app_config.h` and update the following credentials:

```c
// WiFi Settings
#define WIFI_SSID       "Your_WiFi_Name"
#define WIFI_PASSWORD   "Your_WiFi_Password"

// MQTT Selection (1 = Hostname, 0 = IP)
#define MQTT_USE_HOSTNAME  1

// Option A: IP Address String
#define MQTT_BROKER_IP_STRING "52.58.30.91"

// Option B: Hostname (e.g., "broker.hivemq.com")
#define MQTT_BROKER_HOSTNAME "broker.hivemq.com"
```

### Building and Flashing
1.  Open the project directory in VSCode.
2.  Run the build command:
    ```bash
    cmake --build build
    ```
3.  Flash by copying the bin file to the device drive (e.g., `H:`):
    ```bash
    copy build/app/mxchip_azure_iot.bin H:/
    ```

## Usage Guide

-   **Navigation**: Press **Button A** to cycle forward through sensor screens (Temperature -> Humidity -> Motion -> Magnetometer -> **MQTT Stats**). Press **Button B** to cycle backward.
-   **Cloud Data**: The device automatically publishes JSON telemetry to the configured `MQTT_PUB_TOPIC` every 5 seconds.
    *   *Sample Payload:* `{"device": "AZ3166_Device", "temp": 24.50, "status": "active"}`
-   **Remote Control**: Publish to `mxchip/commands` to control the LED:
    *   Turn ON: `{"led": "ON"}`
    *   Turn OFF: `{"led": "OFF"}`
-   **Button Events**: The device publishes button press events:
    *   Button A -> `mxchip/buttonA` -> `{"buttonA": "PRESSED"}`
    *   Button B -> `mxchip/buttonB` -> `{"buttonB": "PRESSED"}`

## Credits and Attribution

We would like to extend our sincere gratitude to the original creators and maintainers of the underlying technologies that make this project possible:

*   **Microsoft Corporation**: For the excellent [Azure RTOS](https://github.com/azure-rtos) (ThreadX, NetX Duo) and the [BSP for the AZ3166](https://github.com/mxchip/MXChip-IoT-DevKit).
*   **Afrilogic Solutions**: For providing the educational workshop templates.
*   **STMicroelectronics**: For the high-quality [MEMS sensors and drivers](https://github.com/STMicroelectronics) used in the DevKit.

---
*Developed with precision and care for the Embedded IoT Community.*
