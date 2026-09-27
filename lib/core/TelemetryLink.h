#pragma once
#include "IRadio.h"
#include "TelemetryPacket.h"

class TelemetryLink {
public:
    explicit TelemetryLink(IRadio& radio);

    bool send(const TelemetryPacket& p);

private:
    IRadio& radio_;
};
