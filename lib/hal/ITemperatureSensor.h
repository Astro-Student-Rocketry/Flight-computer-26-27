#pragma once

class ITemperatureSensor {
public:
    virtual ~ITemperatureSensor() = default;
    virtual bool begin() = 0;
    virtual bool read(float& tempC) = 0;
};
