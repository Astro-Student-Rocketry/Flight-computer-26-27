#pragma once
#include "IGps.h"

class FakeGps : public IGps {
public:
    bool begin() override { return beginOk; }
    bool read(GpsFix& out) override {
        readCount++;
        if (!readOk) return false;
        out = fix;
        return fix.valid;
    }

    GpsFix fix;
    bool beginOk = true;
    bool readOk = true;  // false = GPS svarer ikke. Sett fix.valid = false for "mistet posisjon".
    int readCount = 0;
};
