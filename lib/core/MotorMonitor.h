#pragma once
#include <stdint.h>

#include "IPressureSensor.h"
#include "ITemperatureSensor.h"
#include "IValveSensor.h"
#include "MotorState.h"

// Leser motorsensorene. Styrer ingenting på motoren.
// Ved sensorfeil beholdes siste gode verdi. Feilen meldes med hasFault() og faultMask().
class MotorMonitor {
public:
    // Bits i faultMask().
    static constexpr uint8_t kChamberFault = 1 << 0;
    static constexpr uint8_t kTankFault = 1 << 1;
    static constexpr uint8_t kTempFault = 1 << 2;
    static constexpr uint8_t kValveFault = 1 << 3;

    // Antall mislykkede lesninger på rad før en sensor regnes som feil.
    static constexpr int kFaultThreshold = 3;

    MotorMonitor(IPressureSensor& chamber, IPressureSensor& tank, ITemperatureSensor& temp,
                 IValveSensor& valve);

    // nowUs: tid i mikrosekunder, gitt utenfra siden core/ ikke har noen klokke.
    void update(uint32_t nowUs);
    const MotorState& state() const;

    bool hasFault() const { return faultMask() != 0; }
    uint8_t faultMask() const;

private:
    // Oppdaterer feilteller for sensor i. Returnerer ok.
    bool record(int i, bool ok);

    IPressureSensor& chamber_;
    IPressureSensor& tank_;
    ITemperatureSensor& temp_;
    IValveSensor& valve_;
    MotorState state_;
    int failures_[4] = {0, 0, 0, 0};
};
