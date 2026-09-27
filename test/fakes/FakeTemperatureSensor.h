#pragma once
#include "ITemperatureSensor.h"

class FakeTemperatureSensor : public ITemperatureSensor {
public:
    bool begin() override { return true; }
    bool read(float& tempC) override {
        tempC = temp;
        return true;
    }

    float temp = 15.0f;
};
