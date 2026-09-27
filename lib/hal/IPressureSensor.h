#pragma once

class IPressureSensor {
public:
    virtual ~IPressureSensor() = default;
    virtual bool begin() = 0;
    virtual bool read(float& pressurePa) = 0;
};
