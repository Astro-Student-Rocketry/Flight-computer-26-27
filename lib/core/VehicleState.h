#pragma once
#include <stdint.h>

struct VehicleState {
    uint32_t timestampUs = 0;
    float altitudeM = 0;
    float velocityMs = 0;
    float accelMs2 = 0;
    double latDeg = 0;
    double lonDeg = 0;
    bool gpsValid = false;
};
