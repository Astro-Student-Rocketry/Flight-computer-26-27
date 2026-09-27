#pragma once
#include "VehicleState.h"

class LiftoffDetector {
public:
    bool evaluate(const VehicleState& s);
};
