#pragma once
#include <stddef.h>
#include <stdint.h>

class IRadio {
public:
    virtual ~IRadio() = default;
    virtual bool begin() = 0;
    virtual bool send(const uint8_t* data, size_t len) = 0;
};
