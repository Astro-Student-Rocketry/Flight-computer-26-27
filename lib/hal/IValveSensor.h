#pragma once

class IValveSensor {
public:
    virtual ~IValveSensor() = default;
    virtual bool begin() = 0;
    virtual bool read(bool& isOpen) = 0;
};
