#pragma once
#include <stdint.h>

#include "ITemperatureSensor.h"

class Thermocouple : public ITemperatureSensor {
public:
    explicit Thermocouple(uint8_t csPin);

    bool begin() override;
    bool read(float& tempC) override;

private:
    uint8_t csPin_;
};
