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

TX_THREAD publisher_thread;
ULONG publisher_thread_stack[ECLIPSETX_THREAD_STACK_SIZE / sizeof(ULONG)];

static void publisher_thread_entry(ULONG parameter)
{
    UINT status;

    printf("Starting Publisher Thread\n\n");

    // Initialize the network
    if ((status = wwd_network_init(WIFI_SSID, WIFI_PASSWORD, WIFI_MODE)))
    {
        printf("ERROR: Network initialization failed (0x%08x)\n", status);
        return;
    }

    // Connect to Wi-Fi
    if ((status = wwd_network_connect()))
    {
        printf("ERROR: Wi-Fi connection failed (0x%08x)\n", status);
        return;
    }

    // Initialize and connect to MQTT
    mqtt_init();
    mqtt_connect();  // Assume this connects and returns void
    screen_print("MQTT OK!", L0);
    printf("MQTT Connected\n");

    while (1)
    {
        // Check for Button A: Turn green LED on and publish the command
        if (BUTTON_A_IS_PRESSED)
        {
            printf("Button A Pressed: Turning GREEN ON and publishing command\n");
            RGB_LED_SET_G(255);  // Turn green LED on
            mqtt_publish("fun/led", "green_on");
            screen_print("Sent: GREEN ON", L1);
            tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND); // Debounce delay
        }

        // Check for Button B: Turn green LED off and publish the command
        if (BUTTON_B_IS_PRESSED)
        {
            printf("Button B Pressed: Turning GREEN OFF and publishing command\n");
            RGB_LED_SET_G(0);  // Turn green LED off
            mqtt_publish("fun/led", "green_off");
            screen_print("Sent: GREEN OFF", L1);
            tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND); // Debounce delay
        }

        // Small delay to reduce CPU usage
        tx_thread_sleep(100);
    }
}

void tx_application_define(void* first_unused_memory)
{
    systick_interval_set(TX_TIMER_TICKS_PER_SECOND);

    UINT status = tx_thread_create(&publisher_thread,
                                   "Publisher Thread",
                                   publisher_thread_entry,
                                   0,
                                   publisher_thread_stack,
                                   ECLIPSETX_THREAD_STACK_SIZE,
                                   ECLIPSETX_THREAD_PRIORITY,
                                   ECLIPSETX_THREAD_PRIORITY,
                                   TX_NO_TIME_SLICE,
                                   TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        printf("ERROR: Publisher thread creation failed\n");
    }
}

int main(void)
{
    board_init();
    tx_kernel_enter();
    return 0;
}
