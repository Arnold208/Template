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

// --- ThreadX Definitions ---
#define APP_THREAD_STACK_SIZE 4096
#define APP_THREAD_PRIORITY   4

TX_THREAD app_thread;
ULONG app_thread_stack[APP_THREAD_STACK_SIZE / sizeof(ULONG)];

// --- Display State ---
volatile int current_screen     = 0; // 0: Logo, 1: Telemetry
volatile uint8_t screen_changed = 0;

// --- Button Flags for MQTT ---
volatile bool button_a_flag = false;
volatile bool button_b_flag = false;

// --- Button Callbacks ---
void button_a_callback()
{
    // Switch to Logo Screen
    current_screen = 0;
    screen_changed = 1;

    // Trigger MQTT Publish
    button_a_flag = true;
}

void button_b_callback()
{
    // Switch to Telemetry Screen
    current_screen = 1;
    screen_changed = 1;

    // Trigger MQTT Publish
    button_b_flag = true;
}

// --- Display Update Logic ---
void update_display()
{
    screen_clear();

    switch (current_screen)
    {
        case 0:
            // Display NEW MQTT Logo from images.c
            screen_draw_bitmap(epd_bitmap_Mqtt);
            break;

        case 1:
        {
            float temp     = Sensor_ReadTemperature();
            float humidity = Sensor_ReadHumidity();
            float pressure = Sensor_ReadPressure();
            Display_Unified_Telemetry(temp, humidity, pressure);
            break;
        }
    }

    screen_changed = 0;
}

// --- Application Thread Entry ---
void app_thread_entry(ULONG parameter)
{
    printf("Starting MXChip Sensor Node...\n");

    // 1. Initialize Cloud Connection
    Display_Startup_Status("Connecting...", "WiFi + MQTT");

    if (MQTT_Init())
    {
        printf("Cloud Online.\n");
        Display_Startup_Status("Connected!", "MQTT Online");
        HAL_Delay(1000);
    }
    else
    {
        printf("Cloud Connection Failed. Running Offline.\n");
        Display_Startup_Status("Offline Mode", "MQTT Failed");
        HAL_Delay(2000);
    }

    // Set initial screen to Logo
    current_screen = 0;
    update_display();

    uint32_t last_tick = 0;

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
            MQTT_Publish(MQTT_TOPIC_BUTTON_A, "{\"event\": \"ButtonA_Pressed\"}");
        }

        if (button_b_flag)
        {
            button_b_flag = false;
            MQTT_Publish(MQTT_TOPIC_BUTTON_B, "{\"event\": \"ButtonB_Pressed\"}");
        }

        // Example: Publish Sensor Data every 5 seconds
        // Use HAL_GetTick or tx_time_get
        if (HAL_GetTick() - last_tick > 5000)
        {
            last_tick = HAL_GetTick();

            char payload[128];
            float temp = Sensor_ReadTemperature();
            snprintf(payload,
                sizeof(payload),
                "{\"device\": \"%s\", \"temp\": %d.%02d, \"status\": \"active\"}",
                MQTT_CLIENT_ID,
                (int)temp,
                abs((int)((temp - (int)temp) * 100)));

            if (MQTT_Publish(MQTT_PUB_TOPIC, payload))
            {
                printf("Data sent to %s\r\n", MQTT_PUB_TOPIC);
            }

            const char* last_cmd = MQTT_Get_Last_Message();
            if (strlen(last_cmd) > 0)
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