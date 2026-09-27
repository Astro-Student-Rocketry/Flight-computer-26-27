#pragma once
#include "IBarometer.h"
#include "IImu.h"
#include "VehicleState.h"

class StateEstimator {
public:
    StateEstimator(IBarometer& baro, IImu& imu);

    void update(float dt);
    const VehicleState& state() const;
    bool isConverged() const;

private:
    IBarometer& baro_;
    IImu& imu_;
    VehicleState state_;
    float p0_ = 101325.0f;  // bakketrykk
};
