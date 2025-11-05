#ifndef OBD_H
#define OBD_H

#include <3ds.h>

// OBD-II PIDs
#define OBD_PID_RPM 0x0C
#define OBD_PID_SPEED 0x0D
#define OBD_PID_THROTTLE 0x11
#define OBD_PID_COOLANT_TEMP 0x05
#define OBD_PID_INTAKE_TEMP 0x0F
#define OBD_PID_MAF 0x10
#define OBD_PID_FUEL_LEVEL 0x2F
#define OBD_PID_ENGINE_LOAD 0x04

typedef struct {
    int rpm;
    int speed;
    int throttle;
    int coolantTemp;
    int intakeTemp;
    float maf;
    int fuelLevel;
    int engineLoad;
    bool valid;
} OBDData;

void initOBDData(OBDData* data);
void initOBDConnection(int sockfd);
void updateOBDData(int sockfd, OBDData* data);
int queryOBDPID(int sockfd, u8 pid, char* response, int maxLen);
int parseOBDResponse(const char* response, u8 expectedPID);

#endif
