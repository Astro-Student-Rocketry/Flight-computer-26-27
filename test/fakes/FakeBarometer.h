#pragma once
#include "IBarometer.h"

class FakeBarometer : public IBarometer {
public:
    bool begin() override { return beginOk; }
    bool read(float& pressurePa, float& tempC) override {
        readCount++;
        if (!readOk) return false;
        pressurePa = pressure;
        tempC = temp;
        return true;
    }

    float pressure = 101325.0f;
    float temp = 15.0f;
    bool beginOk = true;
    bool readOk = true;  // sett til false for å simulere sensorfeil
    int readCount = 0;
};
