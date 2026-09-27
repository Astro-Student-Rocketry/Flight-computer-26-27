#pragma once
#include "VehicleState.h"

class ApogeeDetector {
public:
    bool evaluate(const VehicleState& s);
};
