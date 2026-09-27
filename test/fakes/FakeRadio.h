#pragma once
#include "IRadio.h"

class FakeRadio : public IRadio {
public:
    bool begin() override { return true; }
    bool send(const uint8_t* data, size_t len) override {
        (void)data;
        (void)len;
        sendCount++;
        return true;
    }

    int sendCount = 0;
};
