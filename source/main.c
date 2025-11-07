#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "obd.h"
#include "wifi.h"

#define VERSION "1.0.0"

int main(int argc, char **argv)
{
    // Initialize services
    gfxInitDefault();
    acInit();

    // Initialize UI system (Citro2D/3D)
    if (!initUI()) {
        acExit();
        gfxExit();
        return 1;
    }

    // Initialize OBD and WiFi systems
    OBDData obdData;
    WiFiConfig wifiConfig;
    initOBDData(&obdData);

    // Load WiFi configuration
    if (!loadWiFiConfig(&wifiConfig)) {
        // Use default config
        strcpy(wifiConfig.ssid, "OBDII");
        strcpy(wifiConfig.password, "");
        wifiConfig.port = 35000;
        strcpy(wifiConfig.ipAddress, "192.168.0.10");
    }

    bool connected = false;
    int sockfd = -1;
    u64 lastUpdate = 0;

    // Main loop
    while (aptMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START)
            break;

        if (kDown & KEY_A && !connected) {
            sockfd = connectToOBD(&wifiConfig);
            if (sockfd >= 0) {
                connected = true;
                initOBDConnection(sockfd);
            }
        }

        // Update telemetry data periodically
        if (connected && (osGetTime() - lastUpdate) > 100) {
            updateOBDData(sockfd, &obdData);
            lastUpdate = osGetTime();
        }

        // Render UI (handles frame begin/end internally)
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        renderUI(&obdData, connected);
        // C3D_FrameEnd is called inside renderUI
    }

    // Cleanup
    if (sockfd >= 0) {
        closesocket(sockfd);
    }

    exitUI();
    acExit();
    gfxExit();
    return 0;
}
