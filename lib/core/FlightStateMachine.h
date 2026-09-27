#pragma once
#include <stdint.h>

#include "Anomaly.h"
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
    // Bitflagg av Anomaly som har skjedd så langt.
    uint16_t anomalies() const;

private:
    void transitionTo(Phase p, uint32_t nowUs);
    void raiseAnomaly(Anomaly a);
    float timeInPhaseS(uint32_t nowUs) const;

    Phase phase_ = Phase::Idle;
    uint32_t phaseStartUs_ = 0;
    uint16_t anomalies_ = 0;
    FillDetector fill_;
    VentDetector vent_;
    LiftoffDetector liftoff_;
    ApogeeDetector apogee_;
    LandingDetector landing_;
};
