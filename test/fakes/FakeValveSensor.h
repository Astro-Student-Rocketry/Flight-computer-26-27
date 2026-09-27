#pragma once
#include "IValveSensor.h"

class FakeValveSensor : public IValveSensor {
public:
    bool begin() override { return true; }
    bool read(bool& isOpen) override {
        isOpen = open;
        return true;
    }

    bool open = false;
};
