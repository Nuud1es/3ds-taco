#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "wifi.h"

#define CONFIG_FILE "sdmc:/3ds/3ds-taco/config.ini"

bool loadWiFiConfig(WiFiConfig* config) {
    FILE* fp = fopen(CONFIG_FILE, "r");
    if (!fp) return false;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char key[64], value[192];
        if (sscanf(line, "%[^=]=%[^\n]", key, value) == 2) {
            if (strcmp(key, "ssid") == 0) {
                strncpy(config->ssid, value, sizeof(config->ssid) - 1);
            } else if (strcmp(key, "password") == 0) {
                strncpy(config->password, value, sizeof(config->password) - 1);
            } else if (strcmp(key, "ip") == 0) {
                strncpy(config->ipAddress, value, sizeof(config->ipAddress) - 1);
            } else if (strcmp(key, "port") == 0) {
                config->port = atoi(value);
            }
        }
    }

    fclose(fp);
    return true;
}

bool saveWiFiConfig(WiFiConfig* config) {
    // Create directory if it doesn't exist
    mkdir("sdmc:/3ds", 0777);
    mkdir("sdmc:/3ds/3ds-taco", 0777);

    FILE* fp = fopen(CONFIG_FILE, "w");
    if (!fp) return false;

    fprintf(fp, "ssid=%s\n", config->ssid);
    fprintf(fp, "password=%s\n", config->password);
    fprintf(fp, "ip=%s\n", config->ipAddress);
    fprintf(fp, "port=%d\n", config->port);

    fclose(fp);
    return true;
}

int connectToOBD(WiFiConfig* config) {
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1;
    }

    // Set non-blocking mode initially for timeout
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // Setup address structure
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(config->port);

    if (inet_pton(AF_INET, config->ipAddress, &servaddr.sin_addr) <= 0) {
        closesocket(sockfd);
        return -1;
    }

    // Try to connect
    int result = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    if (result < 0) {
        if (errno == EINPROGRESS) {
            // Connection in progress, wait for it
            fd_set writefds;
            struct timeval timeout;

            FD_ZERO(&writefds);
            FD_SET(sockfd, &writefds);

            timeout.tv_sec = 5;
            timeout.tv_usec = 0;

            result = select(sockfd + 1, NULL, &writefds, NULL, &timeout);

            if (result <= 0) {
                closesocket(sockfd);
                return -1;
            }

            // Check if connection succeeded
            int error;
            socklen_t len = sizeof(error);
            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                closesocket(sockfd);
                return -1;
            }
        } else {
            closesocket(sockfd);
            return -1;
        }
    }

    // Set back to blocking mode
    fcntl(sockfd, F_SETFL, 0);

    return sockfd;
}
