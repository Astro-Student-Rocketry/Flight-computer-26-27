#pragma once
#include <stdint.h>

using PyroChannel = uint8_t;

class IPyro {
public:
    virtual ~IPyro() = default;
    virtual bool begin() = 0;
    virtual void fire(PyroChannel ch) = 0;
};
