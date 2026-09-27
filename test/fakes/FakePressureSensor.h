#pragma once
#include "IPressureSensor.h"

class FakePressureSensor : public IPressureSensor {
public:
    bool begin() override { return true; }
    bool read(float& pressurePa) override {
        pressurePa = pressure;
        return true;
    }

    float pressure = 0.0f;
};
