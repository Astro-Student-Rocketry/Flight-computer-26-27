#include "TelemetryEncoder.h"

void TelemetryEncoder::update(const VehicleState& s, const MotorState& m, Phase phase) {
    (void)s;
    (void)m;
    (void)phase;
    // TODO
}

bool TelemetryEncoder::takePacket(TelemetryPacket& out) {
    (void)out;
    // TODO
    return false;
}
