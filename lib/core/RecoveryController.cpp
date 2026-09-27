#include "RecoveryController.h"

RecoveryController::RecoveryController(HealthMonitor& health, IPyro& pyro)
    : health_(health), pyro_(pyro) {}

void RecoveryController::update(Phase phase, const VehicleState& s) {
    (void)phase;
    (void)s;
    // TODO: arm og fyr drogue/main
}

void RecoveryController::arm() {
    // TODO: sjekk health_ før arming
}

bool RecoveryController::isArmed() const { return armed_; }

void RecoveryController::fire(PyroChannel ch) {
    (void)ch;
    // TODO
}
