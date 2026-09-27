#include "FlightStateMachine.h"

void FlightStateMachine::update(const VehicleState& s, const MotorState& m) {
    (void)s;
    (void)m;
    // TODO: faseoverganger
    // Idle -> Filling -> Filled -> Liftoff -> Boost -> Coast -> Descent -> Landed
    // Filling/Filled -> Venting -> Idle
    // Reserve-tidsgrenser og avvik: se README.
}

Phase FlightStateMachine::phase() const { return phase_; }

uint16_t FlightStateMachine::anomalies() const { return anomalies_; }

void FlightStateMachine::transitionTo(Phase p, uint32_t nowUs) {
    phase_ = p;
    phaseStartUs_ = nowUs;
}

void FlightStateMachine::raiseAnomaly(Anomaly a) {
    anomalies_ |= static_cast<uint16_t>(a);
}

float FlightStateMachine::timeInPhaseS(uint32_t nowUs) const {
    return (nowUs - phaseStartUs_) * 1e-6f;
}
