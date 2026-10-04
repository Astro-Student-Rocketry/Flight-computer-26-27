#pragma once
#include <stdint.h>

#include "MotorState.h"

// Oppdager fylling og full tank ut fra tanktrykket. Kalles én gang per tick.
// Tersklene er foreløpige og skal justeres mot motordataene.
class FillDetector {
public:
    struct Config {
        // TODO: sett fra barometeret ved oppstart. Fast verdi gir feil på høyt sted eller ved lavtrykk.
        float ambientPa = 101325.0f;
        float fillMarginPa = 2.0e5f;    // tanktrykk over ambient + dette regnes som fylling
        int confirmSamples = 5;         // prøver på rad over terskelen
        // TODO: 45 bar er en antagelse. Bytt til riktig måltrykk for motoren.
        float targetPa = 4.5e6f;        // tanken regnes som full over dette trykket...
        float stableBandPa = 5.0e4f;    // ...når trykket holder seg innenfor dette båndet...
        float stableSeconds = 3.0f;     // ...så lenge
    };

    FillDetector() = default;
    explicit FillDetector(const Config& cfg) : cfg_(cfg) {}

    bool isFilling(const MotorState& m);
    bool isFull(const MotorState& m);
    // Nullstiller tellere. Kalles når fasen byttes, så gamle målinger ikke teller med.
    void reset();

private:
    Config cfg_;
    int fillingCount_ = 0;
    bool stableActive_ = false;
    float stableRefPa_ = 0;
    uint32_t stableSinceUs_ = 0;
};
