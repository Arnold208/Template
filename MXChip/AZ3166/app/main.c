/**
 ******************************************************************************
 * @file    main.c
 * @author  Microsoft Corporation & Contributors
 * @version V1.0.0
 * @date    2026-01-15
 * @brief   Simplified main application for MXChip Sensor Display.
 ******************************************************************************
 */

#include "app_config.h"
#include "board_init.h"
#include "screen.h"
#include "sensor_display.h"
#include "simple_sensor.h"
#include "tx_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "images.h"
#include "mqtt_manager.h"
#include "networking.h"

// --- ThreadX Definitions ---
#define APP_THREAD_STACK_SIZE 4096
#define APP_THREAD_PRIORITY   4

TX_THREAD app_thread;
ULONG app_thread_stack[APP_THREAD_STACK_SIZE / sizeof(ULONG)];

// --- Display State ---
volatile int current_screen     = 0;
volatile uint8_t screen_changed = 0;

// --- Button Flags for MQTT ---
volatile bool button_a_flag = false;
volatile bool button_b_flag = false;

// --- Button Callbacks ---
void button_a_callback()
{
    // Switch to Logo Screen
    if (current_screen != 0)
    {
        current_screen = 0;
        screen_changed = 1;
    }
    button_a_flag = true;
}

void button_b_callback()
{
    // Switch to Telemetry Screen
    if (current_screen != 1)
    {
        current_screen = 1;
        screen_changed = 1;
    }
    button_b_flag = true;
}

// --- Display Update Helper ---
void update_display()
{
    screen_clear();

    if (current_screen == 0)
    {
        // Screen 0: Event Grid Logo
        screen_draw_bitmap(0, 0, epd_bitmap_event_grid_header_tall, 128, 64);
    }
    else if (current_screen == 1)
    {
        // Screen 1: Unified Telemetry
        float temp     = Sensor_ReadTemperature();
        float humidity = Sensor_ReadHumidity();
        float pressure = Sensor_ReadPressure();
        Display_Unified_Telemetry(temp, humidity, pressure);
    }

    screen_changed = 0;
}

// --- Application Thread Entry ---
void app_thread_entry(ULONG parameter)
{
    printf("Starting MXChip Azure Event Grid Client...\n");

    // Give hardware time to settle
    tx_thread_sleep(100);

    // 1. Initial Logo
    printf("Display: Showing Logo...\n");
    screen_clear();
    screen_draw_bitmap(0, 0, epd_bitmap_event_grid_header_tall, 128, 64);
    tx_thread_sleep(300); // 3 sec

    // 2. WiFi Connection Sequence
    printf("Display: Initializing WiFi...\n");
    Display_Startup_Status("WiFi Init", WIFI_SSID);

    // Use WPA2_PSK_AES as default
    network_init(WIFI_SSID, WIFI_PASSWORD, WPA2_PSK_AES);

    printf("WiFi: Connecting to %s...\n", WIFI_SSID);
    Display_Startup_Status("Connecting...", WIFI_SSID);

    if (network_connect() == NX_SUCCESS)
    {
        printf("WiFi: Connected! Starting MQTT...\n");
        Display_Startup_Status("Connected!", "Starting MQTT...");
        tx_thread_sleep(100); // 1 sec

        if (MQTT_Init())
        {
            printf("MQTT Online.\n");
            Display_Startup_Status("Online!", "Event Grid Ready");
            tx_thread_sleep(200); // 2 sec
        }
        else
        {
            printf("MQTT Connection Failed.\n");
            Display_Startup_Status("MQTT Failed", "Check Settings");
            tx_thread_sleep(300); // 3 sec
        }
    }
    else
    {
        printf("WiFi Connection Failed.\n");
        Display_Startup_Status("WiFi Failed", "Offline Mode");
        tx_thread_sleep(300); // 3 sec
    }

    // Default to Logo Screen after boot
    current_screen = 0;
    update_display();
    printf("App: Loop Started.\n");

    uint32_t last_tick = 0;

    // Show first screen immediately
    update_display();

    // 2. Data Loop
    while (1)
    {
        // Process MQTT Messages
        MQTT_Check_Message();

        // Check if screen needs update (button pressed)
        if (screen_changed)
        {
            update_display();
        }

        // --- Handle Button Presses (Deferred from ISR) ---
        if (button_a_flag)
        {
            button_a_flag = false;
            MQTT_Publish(MQTT_TOPIC_BUTTON_A, "{\"event\": \"buttonA\", \"state\": \"PRESSED\"}");
        }

        if (button_b_flag)
        {
            button_b_flag = false;
            MQTT_Publish(MQTT_TOPIC_BUTTON_B, "{\"event\": \"buttonB\", \"state\": \"PRESSED\"}");
        }

        // Example: Publish Sensor Data every 5 seconds
        if (HAL_GetTick() - last_tick > 5000)
        {
            last_tick = HAL_GetTick();

            char payload[160];
            float temp     = Sensor_ReadTemperature();
            float humidity = Sensor_ReadHumidity();
            float pressure = Sensor_ReadPressure();

            snprintf(payload,
                sizeof(payload),
                "{\"device\": \"%s\", \"temp\": %d.%02d, \"humidity\": %d.%02d, \"pressure\": %d.%02d}",
                MQTT_CLIENT_ID,
                (int)temp,
                abs((int)((temp - (int)temp) * 100)),
                (int)humidity,
                abs((int)((humidity - (int)humidity) * 100)),
                (int)pressure,
                abs((int)((pressure - (int)pressure) * 100)));

            if (MQTT_Publish(MQTT_PUB_TOPIC, payload))
            {
                printf("Telemetry sent to %s\r\n", MQTT_PUB_TOPIC);
            }

            const char* last_cmd = MQTT_Get_Last_Message();
            if (last_cmd && strlen(last_cmd) > 0)
            {
                screen_print((char*)last_cmd, L1);
            }

            // Periodically refresh display data
            update_display();
        }

        // Sleep to yield control
        tx_thread_sleep(10); // 10 ticks (approx 100ms)
    }
}

// --- Application Define (Called by ThreadX) ---
void tx_application_define(void* first_unused_memory)
{
    UINT status;

    // Enable SysTick for HAL_GetTick if needed, though usually handled by low level init.
    // However, ThreadX timer interrupt drives the scheduler.

    // Create Application Thread
    status = tx_thread_create(&app_thread,
        "App Thread",
        app_thread_entry,
        0,
        app_thread_stack,
        APP_THREAD_STACK_SIZE,
        APP_THREAD_PRIORITY,
        APP_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);

    if (status != TX_SUCCESS)
    {
        printf("ERROR: Thread creation failed (0x%02X)\n", status);
    }
}

// --- Main Entry Point ---
int main(void)
{
    // 1. Initialize Board (Buttons, LEDs, I2C, Serial, etc.)
    board_init();

    // 2. Start ThreadX Kernel
    // This function never returns. It jumps to tx_application_define.
    tx_kernel_enter();

    return 0;
}