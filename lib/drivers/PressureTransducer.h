#pragma once
#include <stdint.h>

#include "IPressureSensor.h"

class PressureTransducer : public IPressureSensor {
public:
    explicit PressureTransducer(uint8_t pin);

    bool begin() override;
    bool read(float& pressurePa) override;

private:
    uint8_t pin_;
};
