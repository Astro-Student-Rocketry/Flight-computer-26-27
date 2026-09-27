#pragma once
#include <stdint.h>

#include "MotorState.h"
#include "Phase.h"
#include "TelemetryPacket.h"
#include "VehicleState.h"

class TelemetryEncoder {
public:
    void update(const VehicleState& s, const MotorState& m, Phase phase, uint16_t anomalies);
    // Returnerer true og fyller ut `out` når en ny pakke er klar.
    bool takePacket(TelemetryPacket& out);
};
