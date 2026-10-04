#pragma once
#include "VehicleState.h"

// Oppdager liftoff ut fra akselerasjonen. Kalles én gang per tick, men bare i Filled-fasen,
// så støt på rampa under fylling ikke kan utløse den.
// accelMs2 er kinematisk akselerasjon oppover (tyngdekraften trukket fra, 0 i ro).
// Tersklene er foreløpige og skal justeres mot Simulink-modellen og motordataene.
class LiftoffDetector {
public:
    struct Config {
        float thresholdMs2 = 30.0f;  // ca. 3 g. Godt over støt fra håndtering, godt under motorens skyvekraft.
        int confirmSamples = 5;      // prøver på rad over terskelen (50 ms ved 100 Hz)
    };

    LiftoffDetector() = default;
    explicit LiftoffDetector(const Config& cfg) : cfg_(cfg) {}

    bool evaluate(const VehicleState& s);
    // Nullstiller telleren. Kalles når fasen byttes, så gamle målinger ikke teller med.
    void reset();

private:
    Config cfg_;
    int count_ = 0;
};
