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

#include "wwd_networking.h"
#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nxd_dhcp_client.h"
#include "nxd_dns.h"
#include "board_init.h"

#include "wiced_sdk.h"

#include "sntp_client.h"

#define NETX_IP_STACK_SIZE   2048
#define NETX_TX_PACKET_COUNT 16
#define NETX_RX_PACKET_COUNT 12
#define NETX_PACKET_SIZE     (WICED_LINK_MTU)
#define NETX_TX_POOL_SIZE    ((NETX_PACKET_SIZE + sizeof(NX_PACKET)) * NETX_TX_PACKET_COUNT)
#define NETX_RX_POOL_SIZE    ((NETX_PACKET_SIZE + sizeof(NX_PACKET)) * NETX_RX_PACKET_COUNT)
#define NETX_ARP_CACHE_SIZE  512
#define NETX_DNS_COUNT       6

#define NETX_IPV4_ADDRESS IP_ADDRESS(0, 0, 0, 0)
#define NETX_IPV4_MASK    IP_ADDRESS(255, 255, 255, 0)

#define DHCP_WAIT_TIME_TICKS (60 * TX_TIMER_TICKS_PER_SECOND)  // Wait 60 seconds for DHCP

#define WIFI_COUNTRY WICED_COUNTRY_WORLD_WIDE_XX

static UCHAR netx_ip_stack[NETX_IP_STACK_SIZE];
static UCHAR netx_tx_pool_stack[NETX_TX_POOL_SIZE];
static UCHAR netx_rx_pool_stack[NETX_RX_POOL_SIZE];
static UCHAR netx_arp_cache_area[NETX_ARP_CACHE_SIZE];

static CHAR* netx_ssid;
static CHAR* netx_password;
static wiced_security_t netx_mode;

static NX_DHCP nx_dhcp_client;

NX_IP nx_ip;
NX_PACKET_POOL nx_pool[2]; // 0=TX, 1=RX.
NX_DNS nx_dns_client;

static void print_address(CHAR* preable, ULONG address)
{
    printf("\t%s: %d.%d.%d.%d\r\n",
        preable,
        (uint8_t)(address >> 24),
        (uint8_t)(address >> 16 & 0xFF),
        (uint8_t)(address >> 8 & 0xFF),
        (uint8_t)(address & 0xFF));
}

/* Join Network.  */
static UINT wifi_init()
{
    wiced_mac_t mac;

    printf("\r\nInitializing WiFi\r\n");

    if (netx_ssid[0] == 0)
    {
        printf("ERROR: wifi_ssid is empty\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    // Set pools for wifi
    if (wwd_buffer_init(nx_pool) != WWD_SUCCESS)
    {
        printf("ERROR: wwd_buffer_init\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    // Set country
    if (wwd_management_wifi_on(WIFI_COUNTRY) != WWD_SUCCESS)
    {
        printf("ERROR: wwd_management_wifi_on\r\n");
        return NX_NOT_SUCCESSFUL;
    }

    wwd_wifi_get_mac_address(&mac, WWD_STA_INTERFACE);
    printf("\tMAC address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
        mac.octet[0],
        mac.octet[1],
        mac.octet[2],
        mac.octet[3],
        mac.octet[4],
        mac.octet[5]);

    printf("SUCCESS: WiFi initialized\r\n");

    return NX_SUCCESS;
}

static UINT dhcp_connect(void)
{
    UINT status;
    ULONG actual_status;
    ULONG ip_address;
    ULONG network_mask;
    ULONG gateway_address;

    printf("\r\nInitializing DHCP...\n");

    // Start DHCP client
    status = nx_dhcp_start(&nx_dhcp_client);
    if (status != NX_SUCCESS)
    {
        printf("ERROR: DHCP client start failed (0x%08x)\n", status);
        return status;
    }

    // Wait for DHCP lease (retry 3 times)
    int retries = 3;
    while (retries--)
    {
        status = nx_ip_status_check(&nx_ip, NX_IP_ADDRESS_RESOLVED, &actual_status, DHCP_WAIT_TIME_TICKS);
        if (status == NX_SUCCESS)
        {
            // Get the assigned IP and Gateway
            nx_ip_address_get(&nx_ip, &ip_address, &network_mask);
            nx_ip_gateway_address_get(&nx_ip, &gateway_address);

            print_address("IP address", ip_address);
            print_address("Mask", network_mask);
            print_address("Gateway", gateway_address);

            printf("SUCCESS: DHCP initialized\n");
            return NX_SUCCESS;
        }

        printf("WARNING: DHCP lease attempt failed, retrying...\n");
    }

    // If DHCP failed, assign a static IP
    printf("ERROR: DHCP failed! Assigning Static IP...\n");

    nx_ip_interface_address_set(&nx_ip, 0, IP_ADDRESS(192, 168, 1, 150), IP_ADDRESS(255, 255, 255, 0));
    nx_ip_gateway_address_set(&nx_ip, IP_ADDRESS(192, 168, 1, 1));

    print_address("Static IP", IP_ADDRESS(192, 168, 1, 150));
    print_address("Gateway", IP_ADDRESS(192, 168, 1, 1));

    return NX_SUCCESS;
}


static UINT dns_connect()
{
    UINT status;
    printf("\nInitializing Static DNS Configuration\n");

    // Manually add Google's public DNS server (8.8.8.8)
    status = nx_dns_server_add(&nx_dns_client, IP_ADDRESS(8, 8, 8, 8));
    if (status != NX_SUCCESS)
    {
        printf("ERROR: nx_dns_server_add (0x%08x)\n", status);
        return status;
    }

    print_address("DNS Server", IP_ADDRESS(8, 8, 8, 8));
    return NX_SUCCESS;
}

UINT wwd_network_init(CHAR* ssid, CHAR* password, WiFi_Mode mode)
{
    UINT status;

    // Stash WiFi credentials
    netx_ssid     = ssid;
    netx_password = password;

    switch (mode)
    {
        case None:
            netx_mode = WICED_SECURITY_OPEN;
            break;
        case WEP:
            netx_mode = WICED_SECURITY_WEP_SHARED;
            break;
        case WPA_PSK_TKIP:
            netx_mode = WICED_SECURITY_WPA_TKIP_PSK;
            break;
        case WPA2_PSK_AES:
            netx_mode = WICED_SECURITY_WPA2_AES_PSK;
            break;
    }

    // Initialize the NetX system.
    nx_system_initialize();

    // Create a packet pool for TX.
    if ((status = nx_packet_pool_create(
             &nx_pool[0], "NetX TX Packet Pool", NETX_PACKET_SIZE, netx_tx_pool_stack, NETX_TX_POOL_SIZE)))
    {
        printf("ERROR: nx_packet_pool_create TX (0x%08x)\r\n", status);
    }

    // Create a packet pool for RX.
    else if ((status = nx_packet_pool_create(
                  &nx_pool[1], "NetX RX Packet Pool", NETX_PACKET_SIZE, netx_rx_pool_stack, NETX_RX_POOL_SIZE)))
    {
        nx_packet_pool_delete(&nx_pool[0]);
        printf("ERROR: nx_packet_pool_create RX (0x%08x)\r\n", status);
    }

    // Initialize Wifi
    else if ((status = wifi_init()))
    {
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("ERROR: wifi_init (0x%08x)\r\n", status);
    }

    // Create an IP instance
    else if ((status = nx_ip_create(&nx_ip,
                  "NetX IP Instance 0",
                  NETX_IPV4_ADDRESS,
                  NETX_IPV4_MASK,
                  &nx_pool[0],
                  wiced_sta_netx_duo_driver_entry,
                  (UCHAR*)netx_ip_stack,
                  NETX_IP_STACK_SIZE,
                  1)))
    {
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("ERROR: nx_ip_create (0x%08x)\r\n", status);
    }

    // Enable ARP and supply ARP cache memory
    else if ((status = nx_arp_enable(&nx_ip, (VOID*)netx_arp_cache_area, NETX_ARP_CACHE_SIZE)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("ERROR: nx_arp_enable (0x%08x)\r\n", status);
    }

    // Enable TCP traffic
    else if ((status = nx_tcp_enable(&nx_ip)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("ERROR: nx_tcp_enable (0x%08x)\r\n", status);
    }

    // Enable UDP traffic
    else if ((status = nx_udp_enable(&nx_ip)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("ERROR: nx_udp_enable (0x%08x)\r\n", status);
    }

    // Enable ICMP traffic
    else if ((status = nx_icmp_enable(&nx_ip)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("ERROR: nx_icmp_enable (0x%08x)\r\n", status);
    }

    // Create the DHCP instance.
    else if ((status = nx_dhcp_create(&nx_dhcp_client, &nx_ip, HOSTNAME)))
    {
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("ERROR: nx_dhcp_create (0x%08x)\r\n", status);
    }

    // Start the DHCP Client.
    else if ((status = nx_dhcp_start(&nx_dhcp_client)))
    {
        nx_dhcp_delete(&nx_dhcp_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("ERROR: nx_dhcp_start (0x%08x)\r\n", status);
    }

    // Create DNS
    else if ((status = nx_dns_create(&nx_dns_client, &nx_ip, (UCHAR*)"DNS Client")))
    {
        nx_dhcp_delete(&nx_dhcp_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("ERROR: nx_dns_create (0x%08x)\r\n", status);
    }

    // Use the packet pool here
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    else if ((status = nx_dns_packet_pool_set(&nx_dns_client, nx_ip.nx_ip_default_packet_pool)))
    {
        nx_dns_delete(&nx_dns_client);
        nx_dhcp_delete(&nx_dhcp_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);
        printf("ERROR: nx_dns_packet_pool_set (%0x08)\r\n", status);
    }
#endif

    // Initialize the SNTP client
    else if ((status = sntp_init()))
    {
        printf("ERROR: Failed to init the SNTP client (0x%08x)\r\n", status);
        nx_dns_delete(&nx_dns_client);
        nx_dhcp_delete(&nx_dhcp_client);
        nx_ip_delete(&nx_ip);
        nx_packet_pool_delete(&nx_pool[0]);
        nx_packet_pool_delete(&nx_pool[1]);        
    }

    // Initialize TLS
    else
    {
        nx_secure_tls_initialize();
    }

    return status;
}

UINT wwd_network_connect()
{
    UINT status;
    // int32_t wifiConnectCounter = 1;
    wiced_ssid_t wiced_ssid = {0};
    wwd_result_t join_result;

    // Check if Wi-Fi is already connected
    if (wwd_wifi_is_ready_to_transceive(WWD_STA_INTERFACE) != WWD_SUCCESS)
    {
        printf("\nConnecting Wi-Fi...\n");

        // Halt any existing connection attempts
        wwd_wifi_join_halt(WICED_TRUE);
        wwd_wifi_leave(WWD_STA_INTERFACE);
        wwd_wifi_join_halt(WICED_FALSE);

        wiced_ssid.length = strlen(netx_ssid);
        memcpy(wiced_ssid.value, netx_ssid, wiced_ssid.length);

        // Try connecting to Wi-Fi (max 5 retries)
        for (int attempt = 1; attempt <= 5; attempt++)
        {
            printf("Attempt %d to connect to SSID '%s'...\n", attempt, netx_ssid);

            // Obtain the IP internal mutex before reconnecting Wi-Fi
            tx_mutex_get(&(nx_ip.nx_ip_protection), TX_WAIT_FOREVER);
            join_result = wwd_wifi_join(&wiced_ssid, netx_mode, (uint8_t*)netx_password, strlen(netx_password), NULL, WWD_STA_INTERFACE);
            tx_mutex_put(&(nx_ip.nx_ip_protection));

            if (join_result == WWD_SUCCESS)
            {
                printf("SUCCESS: Wi-Fi connected\n");
                WIFI_LED_ON();
                break;
            }

            printf("WARNING: Wi-Fi connection failed, retrying...\n");
            tx_thread_sleep(5 * TX_TIMER_TICKS_PER_SECOND);
        }

        // If still not connected, return an error
        if (join_result != WWD_SUCCESS)
        {
            printf("ERROR: Unable to connect to Wi-Fi\n");
            return NX_NOT_SUCCESSFUL;
        }
    }

    // Fetch IP details
    status = dhcp_connect();
    if (status != NX_SUCCESS)
    {
        printf("ERROR: DHCP failed\n");
    }

    // Create DNS client
    status = dns_connect();
    if (status != NX_SUCCESS)
    {
        printf("ERROR: DNS client setup failed\n");
    }

    // Sync SNTP time
    status = sntp_sync();
    if (status != NX_SUCCESS)
    {
        printf("ERROR: Failed to sync SNTP time (0x%08x)\n", status);
    }

    return status;
}

