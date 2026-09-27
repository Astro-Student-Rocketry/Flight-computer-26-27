#include "FlightStateMachine.h"

void FlightStateMachine::update(const VehicleState& s, const MotorState& m) {
    (void)s;
    (void)m;
    // TODO: faseoverganger
    // Idle -> Filling -> Filled -> Liftoff -> Boost -> Coast -> Descent -> Landed
    // Filling/Filled -> Venting -> Idle
}

Phase FlightStateMachine::phase() const { return phase_; }

void FlightStateMachine::transitionTo(Phase p) { phase_ = p; }
