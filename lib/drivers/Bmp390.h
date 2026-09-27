#pragma once
#include "IBarometer.h"

class Bmp390 : public IBarometer {
public:
    bool begin() override;
    bool read(float& pressurePa, float& tempC) override;
};
