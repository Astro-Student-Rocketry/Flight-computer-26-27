#include "MotorMonitor.h"

MotorMonitor::MotorMonitor(IPressureSensor& chamber, IPressureSensor& tank,
                           ITemperatureSensor& temp, IValveSensor& valve)
    : chamber_(chamber), tank_(tank), temp_(temp), valve_(valve) {}

void MotorMonitor::update() {
    // TODO: les sensorene og oppdater state_
}

const MotorState& MotorMonitor::state() const { return state_; }
