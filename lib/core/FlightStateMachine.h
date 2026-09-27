#pragma once
#include "ApogeeDetector.h"
#include "LandingDetector.h"
#include "LiftoffDetector.h"
#include "Phase.h"
#include "VehicleState.h"

class FlightStateMachine {
public:
    void update(const VehicleState& s);
    Phase phase() const;

private:
    void transitionTo(Phase p);

    Phase phase_ = Phase::Pad;
    LiftoffDetector liftoff_;
    ApogeeDetector apogee_;
    LandingDetector landing_;
};
