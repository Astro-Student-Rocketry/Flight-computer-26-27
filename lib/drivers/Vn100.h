#pragma once
#include "IImu.h"

class Vn100 : public IImu {
public:
    bool begin() override;
    bool read(ImuSample& out) override;
};
