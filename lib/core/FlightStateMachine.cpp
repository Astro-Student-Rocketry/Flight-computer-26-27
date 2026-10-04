#include "FlightStateMachine.h"

FlightStateMachine::FlightStateMachine(const Config& cfg)
    : cfg_(cfg), fill_(cfg.fill), vent_(cfg.vent), liftoff_(cfg.liftoff) {}

void FlightStateMachine::update(const VehicleState& s, const MotorState& m) {
    const uint32_t now = s.timestampUs;

    // Detektorene teller prøver på rad, så alle som er aktive i fasen må kalles hver tick.
    // Derfor regnes de ut før de sammenlignes, ikke inni en || som kan hoppe over noen.
    switch (phase_) {
        case Phase::Idle:
            if (fill_.isFilling(m)) transitionTo(Phase::Filling, now);
            break;

        case Phase::Filling: {
            const bool venting = vent_.isVenting(m);
            const bool full = fill_.isFull(m);
            if (venting) transitionTo(Phase::Venting, now);
            else if (full) transitionTo(Phase::Filled, now);
            break;
        }

        case Phase::Filled: {
            // Liftoff kan bare skje herfra (sikkerhetsregel 2). Tanktrykket faller når motoren
            // tenner, så liftoff sjekkes først for at det ikke skal tolkes som lufting.
            const bool liftoff = liftoff_.evaluate(s);
            const bool venting = vent_.isVenting(m);
            if (liftoff) {
                liftoffUs_ = now;
                transitionTo(Phase::Liftoff, now);
            } else if (venting) {
                transitionTo(Phase::Venting, now);
            }
            break;
        }

        case Phase::Venting:
            if (vent_.isEmpty(m)) transitionTo(Phase::Idle, now);
            break;

        // Fra Liftoff og framover går fasen bare framover (sikkerhetsregel 1).
        case Phase::Liftoff:
            if (timeInPhaseS(now) >= cfg_.liftoffToBoostS) transitionTo(Phase::Boost, now);
            break;

        case Phase::Boost:
            if (burnoutDetected(s, m)) {
                if ((now - liftoffUs_) * 1e-6f < cfg_.minBurnS) raiseAnomaly(Anomaly::EarlyBurnout);
                transitionTo(Phase::Coast, now);
            } else if (timeInPhaseS(now) >= cfg_.burnoutTimeoutS) {
                raiseAnomaly(Anomaly::BurnoutByTimeout);
                transitionTo(Phase::Coast, now);
            }
            break;

        case Phase::Coast:
            if (apogee_.evaluate(s)) {
                transitionTo(Phase::Descent, now);
            } else if (timeInPhaseS(now) >= cfg_.apogeeTimeoutS) {
                raiseAnomaly(Anomaly::ApogeeByTimeout);
                transitionTo(Phase::Descent, now);
            }
            break;

        case Phase::Descent:
            if (landing_.evaluate(s)) {
                transitionTo(Phase::Landed, now);
            } else if (timeInPhaseS(now) >= cfg_.landingTimeoutS) {
                raiseAnomaly(Anomaly::LandingByTimeout);
                transitionTo(Phase::Landed, now);
            }
            break;

        case Phase::Landed:
            break;
    }
}

Phase FlightStateMachine::phase() const { return phase_; }

uint16_t FlightStateMachine::anomalies() const { return anomalies_; }

bool FlightStateMachine::burnoutDetected(const VehicleState& s, const MotorState& m) {
    // Kammertrykket er hovedkilden. Akselerasjonen er reserve hvis trykksensoren henger
    // (MotorMonitor beholder siste gode verdi ved feil).
    if (m.chamberPressurePa < cfg_.burnoutChamberPa || s.accelMs2 < cfg_.burnoutAccelMs2) {
        if (burnoutCount_ < cfg_.burnoutConfirmSamples) burnoutCount_++;
    } else {
        burnoutCount_ = 0;
    }
    return burnoutCount_ >= cfg_.burnoutConfirmSamples;
}

void FlightStateMachine::transitionTo(Phase p, uint32_t nowUs) {
    phase_ = p;
    phaseStartUs_ = nowUs;
    // Gamle målinger skal ikke telle med i neste fase.
    fill_.reset();
    vent_.reset();
    liftoff_.reset();
    burnoutCount_ = 0;
}

void FlightStateMachine::raiseAnomaly(Anomaly a) {
    anomalies_ |= static_cast<uint16_t>(a);
}

float FlightStateMachine::timeInPhaseS(uint32_t nowUs) const {
    return (nowUs - phaseStartUs_) * 1e-6f;
}
