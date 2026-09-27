#include "TelemetryEncoder.h"

void TelemetryEncoder::update(const VehicleState& s, const MotorState& m, Phase phase,
                              uint16_t anomalies) {
    (void)s;
    (void)m;
    (void)phase;
    (void)anomalies;
    // TODO
}

bool TelemetryEncoder::takePacket(TelemetryPacket& out) {
    (void)out;
    // TODO
    return false;
}
