#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "obd.h"

void initOBDData(OBDData* data) {
    memset(data, 0, sizeof(OBDData));
    data->valid = false;
}

void initOBDConnection(int sockfd) {
    char response[256];

    // Reset ELM327
    send(sockfd, "ATZ\r", 4, 0);
    usleep(1000000); // Wait 1 second for reset
    recv(sockfd, response, sizeof(response), 0);

    // Disable echo
    send(sockfd, "ATE0\r", 5, 0);
    usleep(100000);
    recv(sockfd, response, sizeof(response), 0);

    // Set protocol to automatic
    send(sockfd, "ATSP0\r", 6, 0);
    usleep(100000);
    recv(sockfd, response, sizeof(response), 0);
}

int queryOBDPID(int sockfd, u8 pid, char* response, int maxLen) {
    char cmd[16];
    snprintf(cmd, sizeof(cmd), "01%02X\r", pid);

    int sent = send(sockfd, cmd, strlen(cmd), 0);
    if (sent <= 0) return -1;

    usleep(50000); // Wait 50ms for response

    int received = recv(sockfd, response, maxLen - 1, MSG_DONTWAIT);
    if (received > 0) {
        response[received] = '\0';
        return received;
    }

    return -1;
}

int parseOBDResponse(const char* response, u8 expectedPID) {
    // Look for "41 XX" pattern (response to mode 01 query)
    char pidStr[3];
    snprintf(pidStr, sizeof(pidStr), "%02X", expectedPID);

    char* pidPos = strstr(response, "41");
    if (!pidPos) return -1;

    // Skip to data bytes (after "41 XX ")
    pidPos = strstr(pidPos, pidStr);
    if (!pidPos) return -1;

    pidPos += 3; // Skip PID and space

    // Parse hex values
    int value = 0;
    sscanf(pidPos, "%02X", &value);

    return value;
}

void updateOBDData(int sockfd, OBDData* data) {
    char response[256];
    int value;

    // Query RPM
    if (queryOBDPID(sockfd, OBD_PID_RPM, response, sizeof(response)) > 0) {
        int a = parseOBDResponse(response, OBD_PID_RPM);
        if (a >= 0) {
            // Skip to second byte
            char* pidPos = strstr(response, "41");
            if (pidPos) {
                pidPos = strstr(pidPos, "0C");
                if (pidPos) {
                    int byteA, byteB;
                    sscanf(pidPos + 3, "%02X %02X", &byteA, &byteB);
                    data->rpm = (byteA * 256 + byteB) / 4;

                    // Track maximum RPM for this trip
                    if (data->rpm > data->maxRPM) {
                        data->maxRPM = data->rpm;
                    }
                }
            }
        }
    }

    // Query Speed (MPH)
    if (queryOBDPID(sockfd, OBD_PID_SPEED, response, sizeof(response)) > 0) {
        value = parseOBDResponse(response, OBD_PID_SPEED);
        if (value >= 0) {
            data->speed = (int)(value * 0.621371); // Convert km/h to mph

            // Track maximum speed for this trip
            if (data->speed > data->maxSpeed) {
                data->maxSpeed = data->speed;
            }
        }
    }

    // Query Throttle Position
    if (queryOBDPID(sockfd, OBD_PID_THROTTLE, response, sizeof(response)) > 0) {
        value = parseOBDResponse(response, OBD_PID_THROTTLE);
        if (value >= 0) {
            data->throttle = (value * 100) / 255;
        }
    }

    // Query Coolant Temperature
    if (queryOBDPID(sockfd, OBD_PID_COOLANT_TEMP, response, sizeof(response)) > 0) {
        value = parseOBDResponse(response, OBD_PID_COOLANT_TEMP);
        if (value >= 0) {
            data->coolantTemp = (value - 40) * 9 / 5 + 32; // Convert to Fahrenheit
        }
    }

    // Query Engine Load
    if (queryOBDPID(sockfd, OBD_PID_ENGINE_LOAD, response, sizeof(response)) > 0) {
        value = parseOBDResponse(response, OBD_PID_ENGINE_LOAD);
        if (value >= 0) {
            data->engineLoad = (value * 100) / 255;
        }
    }

    // Query Intake Air Temperature
    if (queryOBDPID(sockfd, OBD_PID_INTAKE_TEMP, response, sizeof(response)) > 0) {
        value = parseOBDResponse(response, OBD_PID_INTAKE_TEMP);
        if (value >= 0) {
            data->intakeTemp = (value - 40) * 9 / 5 + 32; // Convert to Fahrenheit
        }
    }

    data->valid = true;
}

// Smooth interpolation function (exponential moving average)
static float lerp(float current, float target, float smoothing) {
    return current + (target - current) * smoothing;
}

void interpolateOBDData(OBDData* data, float deltaTime) {
    // Smoothing factor (higher = faster response, lower = smoother)
    // Adjusted based on typical 60fps frame rate
    float smoothingFactor = 8.0f * deltaTime; // Reaches ~95% of target in ~0.5 seconds

    // Clamp smoothing to prevent overshooting on frame drops
    if (smoothingFactor > 1.0f) smoothingFactor = 1.0f;

    // Interpolate critical gauges (faster for responsiveness)
    float fastSmoothing = 12.0f * deltaTime;
    if (fastSmoothing > 1.0f) fastSmoothing = 1.0f;

    data->displaySpeed = lerp(data->displaySpeed, (float)data->speed, fastSmoothing);
    data->displayRPM = lerp(data->displayRPM, (float)data->rpm, fastSmoothing);

    // Interpolate other values (slower for stability)
    data->displayThrottle = lerp(data->displayThrottle, (float)data->throttle, smoothingFactor);
    data->displayEngineLoad = lerp(data->displayEngineLoad, (float)data->engineLoad, smoothingFactor);

    // Temperature values interpolate very slowly (they change slowly in reality)
    float slowSmoothing = 4.0f * deltaTime;
    if (slowSmoothing > 1.0f) slowSmoothing = 1.0f;

    data->displayCoolantTemp = lerp(data->displayCoolantTemp, (float)data->coolantTemp, slowSmoothing);
    data->displayIntakeTemp = lerp(data->displayIntakeTemp, (float)data->intakeTemp, slowSmoothing);
}
