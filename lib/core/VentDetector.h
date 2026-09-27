#pragma once
#include "MotorState.h"

class VentDetector {
public:
    bool isVenting(const MotorState& m);
    bool isEmpty(const MotorState& m);
};
