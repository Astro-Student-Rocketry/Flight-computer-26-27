#include "StateEstimator.h"

StateEstimator::StateEstimator(IBarometer& baro, IImu& imu) : baro_(baro), imu_(imu) {}

void StateEstimator::update(float dt) {
    (void)dt;
    // TODO: les sensorer og oppdater state_
}

const VehicleState& StateEstimator::state() const { return state_; }

bool StateEstimator::isConverged() const {
    // TODO
    return false;
}
