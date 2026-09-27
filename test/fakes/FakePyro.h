#pragma once
#include "IPyro.h"

class FakePyro : public IPyro {
public:
    bool begin() override { return true; }
    void fire(PyroChannel ch) override {
        lastChannel = ch;
        fireCount++;
    }

    int lastChannel = -1;
    int fireCount = 0;
};
