#pragma once
#include "ITemperatureSensor.h"

class FakeTemperatureSensor : public ITemperatureSensor {
public:
    bool begin() override { return beginOk; }
    bool read(float& tempC) override {
        readCount++;
        if (!readOk) return false;
        tempC = temp;
        return true;
    }

    float temp = 15.0f;
    bool beginOk = true;
    bool readOk = true;
    int readCount = 0;
};
