#pragma once
#include "IPyro.h"

class PyroDriver : public IPyro {
public:
    bool begin() override;
    void fire(PyroChannel ch) override;
};
