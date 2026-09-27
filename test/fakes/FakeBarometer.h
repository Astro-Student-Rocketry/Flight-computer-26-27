#pragma once
#include "IBarometer.h"

class FakeBarometer : public IBarometer {
public:
    bool begin() override { return true; }
    bool read(float& pressurePa, float& tempC) override {
        pressurePa = pressure;
        tempC = temp;
        return true;
    }

    float pressure = 101325.0f;
    float temp = 15.0f;
};
