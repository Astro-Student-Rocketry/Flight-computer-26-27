#pragma once
#include "IImu.h"

class FakeImu : public IImu {
public:
    bool begin() override { return beginOk; }
    bool read(ImuSample& out) override {
        readCount++;
        if (!readOk) return false;
        out = sample;
        return true;
    }

    ImuSample sample;
    bool beginOk = true;
    bool readOk = true;
    int readCount = 0;
};
