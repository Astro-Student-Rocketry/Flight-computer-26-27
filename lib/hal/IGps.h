#pragma once
#include <stdint.h>

struct GpsFix {
    double latDeg = 0;
    double lonDeg = 0;
    float altitudeM = 0;
    uint8_t satellites = 0;
    bool valid = false;
};

class IGps {
public:
    virtual ~IGps() = default;
    virtual bool begin() = 0;
    // Returnerer true når en ny posisjon er lest.
    virtual bool read(GpsFix& out) = 0;
};
