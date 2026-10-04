#pragma once
#include "IPressureSensor.h"

class FakePressureSensor : public IPressureSensor {
public:
    bool begin() override { return beginOk; }
    bool read(float& pressurePa) override {
        readCount++;
        if (!readOk) return false;
        pressurePa = pressure;
        return true;
    }

    float pressure = 0.0f;
    bool beginOk = true;
    bool readOk = true;
    int readCount = 0;
};
