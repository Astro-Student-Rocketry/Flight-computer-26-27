#pragma once
#include "Phase.h"
#include "TelemetryPacket.h"
#include "VehicleState.h"

class TelemetryEncoder {
public:
    void update(const VehicleState& s, Phase phase);
    // Returnerer true og fyller ut `out` når en ny pakke er klar.
    bool takePacket(TelemetryPacket& out);
};
