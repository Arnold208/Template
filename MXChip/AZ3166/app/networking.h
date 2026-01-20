#ifndef NETWORKING_H
#define NETWORKING_H

#include "nx_api.h"

// Initialize WiFi and Networking Stack
UINT network_init(char* ssid, char* password, UINT security_mode);

// Connect to WiFi and acquire IP
UINT network_connect(void);

#endif // NETWORKING_H
