#pragma once
#include <stdint.h>

struct TelemetryPacket {
    uint8_t version = 1;
    uint8_t phase = 0;
    float altitudeM = 0;
};
