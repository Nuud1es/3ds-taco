#include <3ds.h>
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
    consoleInit(GFX_BOTTOM, NULL);
    acInit();

    PrintConsole* bottomScreen = consoleInit(GFX_BOTTOM, NULL);

    // Initialize OBD and WiFi systems
    OBDData obdData;
    WiFiConfig wifiConfig;
    initOBDData(&obdData);

    printf("3DS-TACO v%s\n", VERSION);
    printf("OBD-II WiFi Telemetry\n");
    printf("====================\n\n");

    // Load WiFi configuration
    if (loadWiFiConfig(&wifiConfig)) {
        printf("WiFi config loaded\n");
        printf("SSID: %s\n", wifiConfig.ssid);
    } else {
        printf("Using default config\n");
        strcpy(wifiConfig.ssid, "OBDII");
        strcpy(wifiConfig.password, "");
        wifiConfig.port = 35000;
        strcpy(wifiConfig.ipAddress, "192.168.0.10");
    }

    printf("\nPress START to exit\n");
    printf("Press A to connect to OBD\n");

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
            printf("\nConnecting to OBD adapter...\n");
            sockfd = connectToOBD(&wifiConfig);
            if (sockfd >= 0) {
                connected = true;
                printf("Connected!\n");
                initOBDConnection(sockfd);
            } else {
                printf("Connection failed!\n");
            }
        }

        // Update telemetry data periodically
        if (connected && (osGetTime() - lastUpdate) > 100) {
            updateOBDData(sockfd, &obdData);
            lastUpdate = osGetTime();
        }

        // Render UI
        renderUI(&obdData, connected);

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    // Cleanup
    if (sockfd >= 0) {
        closesocket(sockfd);
    }

    acExit();
    gfxExit();
    return 0;
}
