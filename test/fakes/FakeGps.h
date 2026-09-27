#pragma once
#include "IGps.h"

class FakeGps : public IGps {
public:
    bool begin() override { return true; }
    bool read(GpsFix& out) override {
        out = fix;
        return fix.valid;
    }

    GpsFix fix;
};
