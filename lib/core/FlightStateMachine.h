#pragma once
#include "ApogeeDetector.h"
#include "FillDetector.h"
#include "LandingDetector.h"
#include "LiftoffDetector.h"
#include "MotorState.h"
#include "Phase.h"
#include "VehicleState.h"
#include "VentDetector.h"

class FlightStateMachine {
public:
    void update(const VehicleState& s, const MotorState& m);
    Phase phase() const;

private:
    void transitionTo(Phase p);

    Phase phase_ = Phase::Idle;
    FillDetector fill_;
    VentDetector vent_;
    LiftoffDetector liftoff_;
    ApogeeDetector apogee_;
    LandingDetector landing_;
};
