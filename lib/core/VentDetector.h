#pragma once
#include <stdint.h>

#include "MotorState.h"

// Oppdager lufting av tanken ut fra tanktrykket. Kalles én gang per tick.
// Tersklene er foreløpige og skal justeres mot motordataene.
class VentDetector {
public:
    struct Config {
        // TODO: sett fra barometeret ved oppstart. Fast verdi gir feil på høyt sted eller ved lavtrykk.
        float ambientPa = 101325.0f;
        float ventDropPa = 5.0e5f;        // lufting: trykket har falt minst så mye fra toppen...
        float ventMinSeconds = 0.5f;      // ...i løpet av minst så lang tid...
        float ventMinRatePaPerS = 5.0e5f; // ...med minst så høy gjennomsnittlig fart (skiller fra lekkasje)
        float riseTolerancePa = 2.0e4f;   // en stigning større enn dette avbryter fallet (støy under dette)
        int confirmSamples = 5;           // prøver på rad uten stigning
        float emptyMarginPa = 1.0e5f;     // tom: tanktrykk under ambient + dette...
        float emptySeconds = 1.0f;        // ...så lenge
    };

    VentDetector() = default;
    explicit VentDetector(const Config& cfg) : cfg_(cfg) {}

    bool isVenting(const MotorState& m);
    bool isEmpty(const MotorState& m);
    // Nullstiller tellere. Kalles når fasen byttes, så gamle målinger ikke teller med.
    void reset();

private:
    Config cfg_;
    // Fallet måles fra siste topp (anker).
    bool haveAnchor_ = false;
    float anchorPa_ = 0;
    uint32_t anchorUs_ = 0;
    float lastPa_ = 0;
    int fallSamples_ = 0;
    // Tom-sjekk.
    bool lowActive_ = false;
    uint32_t lowSinceUs_ = 0;
};
