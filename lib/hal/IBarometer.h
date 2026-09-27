#pragma once

class IBarometer {
public:
    virtual ~IBarometer() = default;
    virtual bool begin() = 0;
    virtual bool read(float& pressurePa, float& tempC) = 0;
};
