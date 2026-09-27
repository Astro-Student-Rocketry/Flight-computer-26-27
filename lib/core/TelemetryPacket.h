#pragma once
#include <stdint.h>

struct TelemetryPacket {
    uint8_t version = 1;
    uint8_t phase = 0;
    uint16_t anomalies = 0;
    float altitudeM = 0;
    float chamberPressurePa = 0;
    float tankPressurePa = 0;
    double latDeg = 0;
    double lonDeg = 0;
};
