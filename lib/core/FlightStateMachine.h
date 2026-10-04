#pragma once
#include <stdint.h>

#include "Anomaly.h"
#include "ApogeeDetector.h"
#include "FillDetector.h"
#include "LandingDetector.h"
#include "LiftoffDetector.h"
#include "MotorState.h"
#include "Phase.h"
#include "VehicleState.h"
#include "VentDetector.h"

// Bestemmer fasen ut fra VehicleState og MotorState. Kalles én gang per tick.
// Overgangene og sikkerhetsreglene er beskrevet i README.
// Tidene er foreløpige og skal justeres mot Simulink-modellen og motordataene.
class FlightStateMachine {
public:
    struct Config {
        FillDetector::Config fill;
        VentDetector::Config vent;
        LiftoffDetector::Config liftoff;
        float liftoffToBoostS = 0.2f;        // Liftoff -> Boost: så lenge etter bekreftet liftoff
        float burnoutChamberPa = 5.0e5f;     // burnout: kammertrykk under dette...
        float burnoutAccelMs2 = 0.0f;        // ...eller akselerasjon under dette...
        int burnoutConfirmSamples = 5;       // ...i så mange prøver på rad
        float minBurnS = 2.0f;               // burnout tidligere enn dette etter liftoff gir EarlyBurnout
        float burnoutTimeoutS = 6.0f;        // reserve: Boost -> Coast etter så lang tid i Boost
        float apogeeTimeoutS = 40.0f;        // reserve: Coast -> Descent etter så lang tid i Coast
        float landingTimeoutS = 600.0f;      // reserve: Descent -> Landed etter så lang tid i Descent
    };

    FlightStateMachine() : FlightStateMachine(Config()) {}
    explicit FlightStateMachine(const Config& cfg);

    // s og m skal være fra samme tick. Tiden tas fra s.timestampUs.
    void update(const VehicleState& s, const MotorState& m);
    Phase phase() const;
    // Bitflagg av Anomaly som har skjedd så langt.
    uint16_t anomalies() const;

private:
    void transitionTo(Phase p, uint32_t nowUs);
    void raiseAnomaly(Anomaly a);
    float timeInPhaseS(uint32_t nowUs) const;
    bool burnoutDetected(const VehicleState& s, const MotorState& m);

    Config cfg_;
    Phase phase_ = Phase::Idle;
    uint32_t phaseStartUs_ = 0;
    uint32_t liftoffUs_ = 0;
    uint16_t anomalies_ = 0;
    int burnoutCount_ = 0;
    FillDetector fill_;
    VentDetector vent_;
    LiftoffDetector liftoff_;
    ApogeeDetector apogee_;
    LandingDetector landing_;
};
