#include <stdio.h>
#include "tx_api.h"
#include "board_init.h"
#include "cmsis_utils.h"
#include "sntp_client.h"
#include "wwd_networking.h"
#include "mqtt_client.h"
#include "screen.h"

#define ECLIPSETX_THREAD_STACK_SIZE 4096
#define ECLIPSETX_THREAD_PRIORITY   4

TX_THREAD eclipsetx_thread;
ULONG eclipsetx_thread_stack[ECLIPSETX_THREAD_STACK_SIZE / sizeof(ULONG)];

static void eclipsetx_thread_entry(ULONG parameter)
{
    UINT status;

    printf("Starting Eclipse ThreadX thread\n\n");

    // Initialize the network
    if ((status = wwd_network_init(WIFI_SSID, WIFI_PASSWORD, WIFI_MODE)))
    {
        printf("ERROR: Failed to initialize the network (0x%08x)\n", status);
        return;
    }

    // Connect to the Wi-Fi
    if ((status = wwd_network_connect()))
    {
        printf("ERROR: Failed to connect to the network (0x%08x)\n", status);
        return;
    }

    // Initialize and connect to MQTT
    mqtt_init();
    mqtt_connect();
    screen_print("  MQTT",L0);

    screen_print(" Connected",L1);


    // Main loop: check buttons and publish JSON messages
    while (1)
    {
        if (BUTTON_A_IS_PRESSED)
        {
            printf("Button A Pressed: Hello My Friend \n");
            mqtt_publish("Arnold", "Hi Lesley");
            screen_print("ON",L1);
            tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND); // Prevent rapid re-triggering
        }

        if (BUTTON_B_IS_PRESSED)
        {
            printf("Button B Pressed: Turning socket OFF\n");
            mqtt_publish("office/smart_extension", "{\"socket1\": \"OFF\"}");
            screen_print("OFF",L1);

            tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND); // Prevent rapid re-triggering
        }

        printf(mqtt_subscribe("message"));
        // Sleep to reduce CPU usage
        tx_thread_sleep(100);
    }
}

// Define application entry point
void tx_application_define(void* first_unused_memory)
{
    systick_interval_set(TX_TIMER_TICKS_PER_SECOND);

    // Create ThreadX thread
    UINT status = tx_thread_create(&eclipsetx_thread,
        "Eclipse ThreadX Thread",
        eclipsetx_thread_entry,
        0,
        eclipsetx_thread_stack,
        ECLIPSETX_THREAD_STACK_SIZE,
        ECLIPSETX_THREAD_PRIORITY,
        ECLIPSETX_THREAD_PRIORITY,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);

    if (status != TX_SUCCESS)
    {
        printf("ERROR: Eclipse ThreadX thread creation failed\n");
    }
}

// Main function
int main(void)
{
    // Initialize the board
    board_init();
    tx_kernel_enter(); // Start ThreadX
    return 0;
}
