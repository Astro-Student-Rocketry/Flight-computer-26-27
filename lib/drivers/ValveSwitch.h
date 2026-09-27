#pragma once
#include <stdint.h>

#include "IValveSensor.h"

class ValveSwitch : public IValveSensor {
public:
    explicit ValveSwitch(uint8_t pin);

    bool begin() override;
    bool read(bool& isOpen) override;

private:
    uint8_t pin_;
};
