#pragma once
#include <stdint.h>

struct VehicleState {
    uint32_t timestampUs = 0;
    float altitudeM = 0;
    float velocityMs = 0;
    float accelMs2 = 0;
};
