#pragma once
#include <stdint.h>

#include "IGps.h"
#include "IImu.h"

// Rådata fra sensorene, logges ved siden av estimatene for sammenligning med Simulink.
struct RawSensorData {
    uint32_t timestampUs = 0;
    float pressurePa = 0;
    float tempC = 0;
    ImuSample imu;
    GpsFix gps;
};
