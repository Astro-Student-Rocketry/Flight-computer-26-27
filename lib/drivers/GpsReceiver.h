#pragma once
#include "IGps.h"

class GpsReceiver : public IGps {
public:
    bool begin() override;
    bool read(GpsFix& out) override;
};
