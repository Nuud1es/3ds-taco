#ifndef WIFI_H
#define WIFI_H

#include <3ds.h>

typedef struct {
    char ssid[64];
    char password[64];
    char ipAddress[16];
    int port;
} WiFiConfig;

bool loadWiFiConfig(WiFiConfig* config);
bool saveWiFiConfig(WiFiConfig* config);
int connectToOBD(WiFiConfig* config);

#endif
