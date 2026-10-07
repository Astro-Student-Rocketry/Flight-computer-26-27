#include "TelemetryLink.h"

TelemetryLink::TelemetryLink(IRadio& radio) : radio_(radio) {}

bool TelemetryLink::send(const TelemetryPacket& p) {
    uint8_t buf[TelemetryPacket::kSize];
    p.encode(buf);
    return radio_.send(buf, sizeof(buf));
}
