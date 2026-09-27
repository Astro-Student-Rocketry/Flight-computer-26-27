#pragma once
#include <stdint.h>

struct MotorState {
    uint32_t timestampUs = 0;
    float chamberPressurePa = 0;
    float tankPressurePa = 0;
    float temperatureC = 0;
    bool valveOpen = false;
};
