#include "FlightStateMachine.h"

void FlightStateMachine::update(const VehicleState& s) {
    (void)s;
    // TODO: faseoverganger
}

Phase FlightStateMachine::phase() const { return phase_; }

void FlightStateMachine::transitionTo(Phase p) { phase_ = p; }
