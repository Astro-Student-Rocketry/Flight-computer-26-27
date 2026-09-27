#pragma once
#include "IPressureSensor.h"
#include "ITemperatureSensor.h"
#include "IValveSensor.h"
#include "MotorState.h"

// Leser motorsensorene. Styrer ingenting på motoren.
class MotorMonitor {
public:
    MotorMonitor(IPressureSensor& chamber, IPressureSensor& tank, ITemperatureSensor& temp,
                 IValveSensor& valve);

    void update();
    const MotorState& state() const;

private:
    IPressureSensor& chamber_;
    IPressureSensor& tank_;
    ITemperatureSensor& temp_;
    IValveSensor& valve_;
    MotorState state_;
};
