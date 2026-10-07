#include "MotorMonitor.h"

#include <math.h>

MotorMonitor::MotorMonitor(IPressureSensor& chamber, IPressureSensor& tank,
                           ITemperatureSensor& temp, IValveSensor& valve)
    : chamber_(chamber), tank_(tank), temp_(temp), valve_(valve) {}

bool MotorMonitor::record(int i, bool ok) {
    failures_[i] = ok ? 0 : failures_[i] + 1;
    return ok;
}

void MotorMonitor::update(uint32_t nowUs) {
    state_.timestampUs = nowUs;

    float chamber = 0, tank = 0, temp = 0;
    bool open = false;

    // Et absoluttrykk under 0 eller en NaN er umulig, og regnes som en feil lesning.
    if (record(0, chamber_.read(chamber) && isfinite(chamber) && chamber >= 0))
        state_.chamberPressurePa = chamber;
    if (record(1, tank_.read(tank) && isfinite(tank) && tank >= 0)) state_.tankPressurePa = tank;
    if (record(2, temp_.read(temp) && isfinite(temp))) state_.temperatureC = temp;
    if (record(3, valve_.read(open))) state_.valveOpen = open;
}

const MotorState& MotorMonitor::state() const { return state_; }

uint8_t MotorMonitor::faultMask() const {
    uint8_t mask = 0;
    for (int i = 0; i < 4; i++)
        if (failures_[i] >= kFaultThreshold) mask |= static_cast<uint8_t>(1 << i);
    return mask;
}
