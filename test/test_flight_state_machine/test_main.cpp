#include <unity.h>

#include "FlightStateMachine.h"
#include "../helpers/TestHelpers.h"

void setUp() {}
void tearDown() {}

const float kAmbientPa = 101325.0f;
const float kTankFullPa = 5.0e6f;
const float kChamberBurnPa = 3.0e6f;

bool hasAnomaly(const FlightStateMachine& fsm, Anomaly a) {
    return (fsm.anomalies() & static_cast<uint16_t>(a)) != 0;
}

// Kjører tilstandsmaskinen med håndlagde målinger, 100 Hz.
struct Driver {
    FlightStateMachine fsm;
    uint32_t t = 0;

    Driver() = default;
    explicit Driver(const FlightStateMachine::Config& cfg) : fsm(cfg) {}

    void tick(float accel, float tankPa, float chamberPa = kAmbientPa, float vel = 0) {
        t += 10000;
        VehicleState s = vehicleAt(t, accel);
        s.velocityMs = vel;
        MotorState m = motorAt(t, tankPa);
        m.chamberPressurePa = chamberPa;
        fsm.update(s, m);
    }

    void run(float seconds, float accel, float tankPa, float chamberPa = kAmbientPa, float vel = 0) {
        for (int i = 0; i < static_cast<int>(seconds * 100.0f + 0.5f); i++) tick(accel, tankPa, chamberPa, vel);
    }

    // Lufter tanken med 10 bar/s ned til omgivelsestrykk.
    void ventToAmbient(float fromPa) {
        for (float pa = fromPa; pa > kAmbientPa; pa -= 1.0e4f) tick(0, pa);
    }

    void toFilled() {
        run(4.0f, 0, kTankFullPa);
        TEST_ASSERT_TRUE(fsm.phase() == Phase::Filled);
    }

    void toBoost() {
        toFilled();
        run(0.3f, 80.0f, kTankFullPa, kChamberBurnPa);
        TEST_ASSERT_TRUE(fsm.phase() == Phase::Boost);
    }

    void toCoast() {
        toBoost();
        run(2.5f, 80.0f, kTankFullPa, kChamberBurnPa);
        run(0.1f, -10.0f, kAmbientPa, kAmbientPa, 100.0f);
        TEST_ASSERT_TRUE(fsm.phase() == Phase::Coast);
    }
};

void test_fsm_starts_idle() {
    FlightStateMachine fsm;
    TEST_ASSERT_TRUE(fsm.phase() == Phase::Idle);
    TEST_ASSERT_EQUAL_INT(0, fsm.anomalies());
}

void test_fsm_full_flight_goes_through_every_phase_in_order() {
    SimRig r;
    FlightStateMachine fsm;
    Phase seen[16];
    int n = 0;
    seen[n++] = fsm.phase();
    for (int i = 0; i < 100000 && fsm.phase() != Phase::Landed; i++) {
        MotorState m = r.step();
        fsm.update(vehicleFrom(r), m);
        if (fsm.phase() != seen[n - 1] && n < 16) seen[n++] = fsm.phase();
    }
    const Phase expected[] = {Phase::Idle,  Phase::Filling, Phase::Filled,  Phase::Liftoff,
                              Phase::Boost, Phase::Coast,   Phase::Descent, Phase::Landed};
    TEST_ASSERT_EQUAL_INT(8, n);
    for (int i = 0; i < 8; i++) TEST_ASSERT_TRUE(seen[i] == expected[i]);
    TEST_ASSERT_FALSE(hasAnomaly(fsm, Anomaly::EarlyBurnout));
    TEST_ASSERT_FALSE(hasAnomaly(fsm, Anomaly::BurnoutByTimeout));
}

void test_fsm_liftoff_ignored_before_filled() {
    Driver d;
    d.run(1.0f, 100.0f, kAmbientPa);  // støt i Idle
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Idle);
    d.run(1.0f, 100.0f, 2.0e6f);      // støt under fylling
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Filling);
}

void test_fsm_vent_from_filling_returns_to_idle() {
    Driver d;
    d.run(1.0f, 0, 3.0e6f);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Filling);
    d.ventToAmbient(3.0e6f);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Venting);
    d.run(2.0f, 0, kAmbientPa);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Idle);
}

void test_fsm_vent_from_filled_returns_to_idle() {
    Driver d;
    d.toFilled();
    d.ventToAmbient(kTankFullPa);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Venting);
    d.run(2.0f, 0, kAmbientPa);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Idle);
}

void test_fsm_can_refill_after_vent() {
    Driver d;
    d.toFilled();
    d.ventToAmbient(kTankFullPa);
    d.run(2.0f, 0, kAmbientPa);
    d.toFilled();
}

void test_fsm_no_way_back_after_liftoff() {
    FlightStateMachine::Config cfg;
    cfg.apogeeTimeoutS = 1.0f;
    cfg.landingTimeoutS = 1.0f;
    Driver d(cfg);
    d.toFilled();
    d.run(0.1f, 80.0f, kTankFullPa, kChamberBurnPa);
    // Tanken tømmes raskt og ligger på omgivelsestrykk: ser ut som lufting, men fasen skal bare gå framover.
    Phase last = d.fsm.phase();
    for (float pa = kTankFullPa; pa > kAmbientPa; pa -= 1.0e5f) d.tick(-10.0f, pa);
    for (int i = 0; i < 500; i++) {
        d.tick(0, kAmbientPa);
        TEST_ASSERT_TRUE(static_cast<int>(d.fsm.phase()) >= static_cast<int>(last));
        last = d.fsm.phase();
    }
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Landed);
}

void test_fsm_burnout_by_chamber_pressure() {
    Driver d;
    d.toBoost();
    d.run(2.5f, 80.0f, kTankFullPa, kChamberBurnPa);
    d.run(0.1f, 5.0f, kTankFullPa, kAmbientPa);  // akselerasjonen fortsatt positiv
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Coast);
    TEST_ASSERT_EQUAL_INT(0, d.fsm.anomalies());
}

void test_fsm_burnout_by_accel_when_chamber_sensor_stuck() {
    Driver d;
    d.toBoost();
    d.run(2.5f, 80.0f, kTankFullPa, kChamberBurnPa);
    d.run(0.1f, -10.0f, kTankFullPa, kChamberBurnPa);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Coast);
}

void test_fsm_burnout_ignores_single_sample() {
    Driver d;
    d.toBoost();
    d.tick(-10.0f, kTankFullPa, kAmbientPa);
    d.run(1.0f, 80.0f, kTankFullPa, kChamberBurnPa);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Boost);
}

void test_fsm_early_burnout_is_flagged() {
    Driver d;
    d.toBoost();
    d.run(0.5f, 80.0f, kTankFullPa, kChamberBurnPa);
    d.run(0.1f, -10.0f, kAmbientPa, kAmbientPa);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Coast);
    TEST_ASSERT_TRUE(hasAnomaly(d.fsm, Anomaly::EarlyBurnout));
}

void test_fsm_burnout_by_timeout() {
    Driver d;
    d.toBoost();
    d.run(5.9f, 80.0f, kTankFullPa, kChamberBurnPa);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Boost);
    d.run(0.2f, 80.0f, kTankFullPa, kChamberBurnPa);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Coast);
    TEST_ASSERT_TRUE(hasAnomaly(d.fsm, Anomaly::BurnoutByTimeout));
    TEST_ASSERT_FALSE(hasAnomaly(d.fsm, Anomaly::EarlyBurnout));
}

void test_fsm_apogee_by_timeout() {
    FlightStateMachine::Config cfg;
    cfg.apogeeTimeoutS = 5.0f;
    Driver d(cfg);
    d.toCoast();
    d.run(4.8f, -10.0f, kAmbientPa, kAmbientPa, 50.0f);  // fortsatt på vei opp
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Coast);
    d.run(0.2f, -10.0f, kAmbientPa, kAmbientPa, 50.0f);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Descent);
    TEST_ASSERT_TRUE(hasAnomaly(d.fsm, Anomaly::ApogeeByTimeout));
}

void test_fsm_landing_by_timeout() {
    FlightStateMachine::Config cfg;
    cfg.apogeeTimeoutS = 1.0f;
    cfg.landingTimeoutS = 10.0f;
    Driver d(cfg);
    d.toCoast();
    d.run(1.0f, -10.0f, kAmbientPa, kAmbientPa, 50.0f);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Descent);
    d.run(9.8f, 0, kAmbientPa, kAmbientPa, -20.0f);  // fortsatt i fallskjerm
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Descent);
    d.run(0.2f, 0, kAmbientPa, kAmbientPa, -20.0f);
    TEST_ASSERT_TRUE(d.fsm.phase() == Phase::Landed);
    TEST_ASSERT_TRUE(hasAnomaly(d.fsm, Anomaly::LandingByTimeout));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_fsm_starts_idle);
    RUN_TEST(test_fsm_full_flight_goes_through_every_phase_in_order);
    RUN_TEST(test_fsm_liftoff_ignored_before_filled);
    RUN_TEST(test_fsm_vent_from_filling_returns_to_idle);
    RUN_TEST(test_fsm_vent_from_filled_returns_to_idle);
    RUN_TEST(test_fsm_can_refill_after_vent);
    RUN_TEST(test_fsm_no_way_back_after_liftoff);
    RUN_TEST(test_fsm_burnout_by_chamber_pressure);
    RUN_TEST(test_fsm_burnout_by_accel_when_chamber_sensor_stuck);
    RUN_TEST(test_fsm_burnout_ignores_single_sample);
    RUN_TEST(test_fsm_early_burnout_is_flagged);
    RUN_TEST(test_fsm_burnout_by_timeout);
    RUN_TEST(test_fsm_apogee_by_timeout);
    RUN_TEST(test_fsm_landing_by_timeout);
    return UNITY_END();
}
