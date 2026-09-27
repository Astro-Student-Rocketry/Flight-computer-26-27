#include "TelemetryLink.h"

TelemetryLink::TelemetryLink(IRadio& radio) : radio_(radio) {}

bool TelemetryLink::send(const TelemetryPacket& p) {
    (void)p;
    // TODO
    return false;
}
