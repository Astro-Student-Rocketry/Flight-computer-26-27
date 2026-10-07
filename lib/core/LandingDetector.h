#pragma once
#include "VehicleState.h"

class LandingDetector {
public:
    bool evaluate(const VehicleState& s);
};
