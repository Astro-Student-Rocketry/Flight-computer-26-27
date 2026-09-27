#pragma once
#include "IImu.h"

class FakeImu : public IImu {
public:
    bool begin() override { return true; }
    bool read(ImuSample& out) override {
        out = sample;
        return true;
    }

    ImuSample sample;
};
