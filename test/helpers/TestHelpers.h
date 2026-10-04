#pragma once
// Felles hjelpere for testene. Hver test_*-mappe er et eget program, så alt her er header-only.
#include <stdint.h>

#include "../fakes/FlightSim.h"
#include "MotorState.h"
#include "VehicleState.h"

// Setter opp fakene og simulatoren, og lager MotorState som MotorMonitor skal gjøre senere.
struct SimRig {
    FakeBarometer baro;
    FakeImu imu;
    FakeGps gps;
    FakePressureSensor chamber, tank;
    FakeTemperatureSensor temp;
    FakeValveSensor valve;
    FlightSim sim{baro, imu, gps, chamber, tank, temp, valve};
    uint32_t nowUs = 0;

    MotorState step(float dt = 0.01f) {
        sim.step(dt);
        nowUs += static_cast<uint32_t>(dt * 1e6f);
        MotorState m;
        m.timestampUs = nowUs;
        m.tankPressurePa = tank.pressure;
        m.chamberPressurePa = chamber.pressure;
        m.temperatureC = temp.temp;
        m.valveOpen = valve.open;
        return m;
    }
};

inline MotorState motorAt(uint32_t us, float tankPa) {
    MotorState m;
    m.timestampUs = us;
    m.tankPressurePa = tankPa;
    return m;
}

// Spiller av et lineært trykkforløp med litt deterministisk støy (+-0,1 bar).
// Returnerer true hvis fn ga true på noen prøve. t og pa oppdateres til sluttverdiene.
template <typename Fn>
bool ramp(Fn fn, uint32_t& tUs, float& pa, float ratePaPerS, float seconds, float dt = 0.01f) {
    bool seen = false;
    int n = static_cast<int>(seconds / dt);
    for (int i = 0; i < n; i++) {
        pa += ratePaPerS * dt;
        tUs += static_cast<uint32_t>(dt * 1e6f);
        float noise = (i % 3 - 1) * 1.0e4f;
        seen |= fn(motorAt(tUs, pa + noise));
    }
    return seen;
}

inline VehicleState vehicleAt(uint32_t us, float accelMs2) {
    VehicleState s;
    s.timestampUs = us;
    s.accelMs2 = accelMs2;
    return s;
}

// Lager VehicleState fra simulatoren slik StateEstimator skal gjøre senere:
// IMU-en måler spesifikk kraft, så g trekkes fra for å få kinematisk akselerasjon.
inline VehicleState vehicleFrom(SimRig& r) {
    VehicleState s = vehicleAt(r.nowUs, r.imu.sample.az - 9.81f);
    s.altitudeM = r.sim.altitude();
    s.velocityMs = r.sim.velocity();
    return s;
}
