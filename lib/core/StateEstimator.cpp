#include "StateEstimator.h"

StateEstimator::StateEstimator(IBarometer& baro, IImu& imu, IGps& gps)
    : baro_(baro), imu_(imu), gps_(gps) {}

void StateEstimator::update(float dt) {
    (void)dt;
    // TODO: les sensorer inn i raw_ og oppdater state_
}

const VehicleState& StateEstimator::state() const { return state_; }

const RawSensorData& StateEstimator::raw() const { return raw_; }

bool StateEstimator::isConverged() const {
    // TODO
    return false;
}
