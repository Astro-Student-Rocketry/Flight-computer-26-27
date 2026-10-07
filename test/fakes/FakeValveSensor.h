#pragma once
#include "IValveSensor.h"

class FakeValveSensor : public IValveSensor {
public:
    bool begin() override { return beginOk; }
    bool read(bool& isOpen) override {
        readCount++;
        if (!readOk) return false;
        isOpen = open;
        return true;
    }

    bool open = false;
    bool beginOk = true;
    bool readOk = true;
    int readCount = 0;
};
