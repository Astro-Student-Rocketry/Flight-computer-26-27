#pragma once
#include "IBarometer.h"
#include "IGps.h"
#include "IImu.h"
#include "RawSensorData.h"
#include "VehicleState.h"

class StateEstimator {
public:
    StateEstimator(IBarometer& baro, IImu& imu, IGps& gps);

    void update(float dt);
    const VehicleState& state() const;
    const RawSensorData& raw() const;
    bool isConverged() const;

private:
    IBarometer& baro_;
    IImu& imu_;
    IGps& gps_;
    VehicleState state_;
    RawSensorData raw_;
    float p0_ = 101325.0f;  // bakketrykk
};
