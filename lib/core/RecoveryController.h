#pragma once
#include "HealthMonitor.h"
#include "IPyro.h"
#include "Phase.h"
#include "VehicleState.h"

class RecoveryController {
public:
    RecoveryController(HealthMonitor& health, IPyro& pyro);

    void update(Phase phase, const VehicleState& s);
    void arm();
    bool isArmed() const;

private:
    void fire(PyroChannel ch);

    HealthMonitor& health_;
    IPyro& pyro_;
    bool armed_ = false;
    bool drogueFired_ = false;
    bool mainFired_ = false;
};
