#pragma once
#include "MotorState.h"

class FillDetector {
public:
    bool isFilling(const MotorState& m);
    bool isFull(const MotorState& m);
};
