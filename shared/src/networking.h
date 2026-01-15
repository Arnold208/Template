/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#ifndef _NETWORKING_H
#define _NETWORKING_H

#include "nx_api.h"
#include "nxd_dns.h"

extern NX_IP nx_ip;
extern NX_PACKET_POOL nx_pool[2];
extern NX_DNS nx_dns_client;

typedef enum
{
    None         = 0,
    WEP          = 1,
    WPA_PSK_TKIP = 2,
    WPA2_PSK_AES = 3
} WiFi_Mode;

UINT network_init(CHAR* ssid, CHAR* password, WiFi_Mode mode);
UINT network_connect();

#endif // _NETWORKING_H