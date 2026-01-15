#include <stdio.h>
#include <string.h>    // for strcmp
#include "tx_api.h"
#include "board_init.h"
#include "cmsis_utils.h"
#include "sntp_client.h"
#include "wwd_networking.h"
#include "mqtt_client.h"
#include "screen.h"

#define ECLIPSETX_THREAD_STACK_SIZE 4096
#define ECLIPSETX_THREAD_PRIORITY   4

TX_THREAD subscriber_thread;
ULONG subscriber_thread_stack[ECLIPSETX_THREAD_STACK_SIZE / sizeof(ULONG)];

static void subscriber_thread_entry(ULONG parameter)
{
    UINT status;
    char *incoming_message;

    printf("Starting Subscriber Thread\n\n");

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
    mqtt_connect();
    screen_print("MQTT OK!", L0);
    printf("MQTT Connected\n");

    while (1)
    {
        // Subscribe for incoming messages on the topic "fun/led"
        incoming_message = mqtt_subscribe("fun/led");
        if (incoming_message != NULL && incoming_message[0] != '\0')
        {
            printf("Received on fun/led: %s\n", incoming_message);
            screen_print(incoming_message, L1);
            if (strcmp(incoming_message, "green_on") == 0)
            {
                // Turn green LED on
                RGB_LED_SET_G(255);
            }
            else if (strcmp(incoming_message, "green_off") == 0)
            {
                // Turn green LED off
                RGB_LED_SET_G(0);
            }
            tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND); // Delay to prevent rapid toggling
        }
        tx_thread_sleep(100);
    }
}

void tx_application_define(void* first_unused_memory)
{
    systick_interval_set(TX_TIMER_TICKS_PER_SECOND);

    UINT status = tx_thread_create(&subscriber_thread,
                                   "Subscriber Thread",
                                   subscriber_thread_entry,
                                   0,
                                   subscriber_thread_stack,
                                   ECLIPSETX_THREAD_STACK_SIZE,
                                   ECLIPSETX_THREAD_PRIORITY,
                                   ECLIPSETX_THREAD_PRIORITY,
                                   TX_NO_TIME_SLICE,
                                   TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        printf("ERROR: Subscriber thread creation failed\n");
    }
}

int main(void)
{
    board_init();
    tx_kernel_enter();
    return 0;
}
