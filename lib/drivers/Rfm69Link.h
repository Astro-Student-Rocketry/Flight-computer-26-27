#pragma once
#include "IRadio.h"

class Rfm69Link : public IRadio {
public:
    bool begin() override;
    bool send(const uint8_t* data, size_t len) override;
};
