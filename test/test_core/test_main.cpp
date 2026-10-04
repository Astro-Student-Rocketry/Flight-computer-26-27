#include <unity.h>

#include "../fakes/FakeBarometer.h"
#include "../fakes/FakeGps.h"
#include "../fakes/FakeImu.h"
#include "../fakes/FakePressureSensor.h"
#include "../fakes/FakeRadio.h"
#include "../fakes/FakeStorage.h"
#include "../fakes/FakeTemperatureSensor.h"
#include "../fakes/FakeValveSensor.h"
#include "../fakes/FlightSim.h"
#include "FillDetector.h"
#include "VentDetector.h"
#include "FlightComputer.h"

void setUp() {}
void tearDown() {}

void test_flight_computer_constructs() {
    FakeBarometer baro;
    FakeImu imu;
    FakeGps gps;
    FakeStorage storage;
    FakeRadio radio;
    FakePressureSensor chamberPressure;
    FakePressureSensor tankPressure;
    FakeTemperatureSensor motorTemp;
    FakeValveSensor valve;
    FlightComputer fc(baro, imu, gps, storage, radio, chamberPressure, tankPressure, motorTemp, valve);
    fc.tick(0.01f);
    TEST_PASS();
}


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

MotorState motorAt(uint32_t us, float tankPa) {
    MotorState m;
    m.timestampUs = us;
    m.tankPressurePa = tankPa;
    return m;
}

void test_fill_not_detected_at_rest() {
    SimRig r;
    FillDetector d;
    while (r.sim.stage() == FlightSim::Stage::Pad) TEST_ASSERT_FALSE(d.isFilling(r.step()));
}

void test_fill_detected_during_fill() {
    SimRig r;
    FillDetector d;
    bool seen = false;
    while (r.sim.stage() != FlightSim::Stage::Filled) seen |= d.isFilling(r.step());
    TEST_ASSERT_TRUE(seen);
}

void test_fill_ignores_single_spike() {
    FillDetector d;
    TEST_ASSERT_FALSE(d.isFilling(motorAt(0, 101325.0f)));
    TEST_ASSERT_FALSE(d.isFilling(motorAt(10000, 3.0e6f)));  // én støymåling
    TEST_ASSERT_FALSE(d.isFilling(motorAt(20000, 101325.0f)));
    for (uint32_t i = 3; i < 30; i++) TEST_ASSERT_FALSE(d.isFilling(motorAt(i * 10000, 101325.0f)));
}

void test_full_only_after_stable_at_target() {
    SimRig r;
    FillDetector d;
    // Ikke full mens tanken fortsatt fylles.
    while (r.sim.stage() != FlightSim::Stage::Filled) {
        MotorState m = r.step();
        d.isFilling(m);
        TEST_ASSERT_FALSE(d.isFull(m));
    }
    // Ikke full rett etter at fyllingen stoppet, men full etter noen sekunder.
    bool full = false;
    int steps = 0;
    while (!full && steps < 1000) {
        full = d.isFull(r.step());
        steps++;
    }
    TEST_ASSERT_TRUE(full);
    TEST_ASSERT_TRUE(steps > 100);  // minst 1 s stabilt
}

void test_full_resets_when_pressure_drops() {
    FillDetector d;
    uint32_t t = 0;
    for (int i = 0; i < 250; i++, t += 10000) d.isFull(motorAt(t, 5.0e6f));  // 2,5 s stabilt
    d.isFull(motorAt(t, 2.0e6f));                                            // lekkasje
    t += 10000;
    for (int i = 0; i < 250; i++, t += 10000) TEST_ASSERT_FALSE(d.isFull(motorAt(t, 5.0e6f)));
}

void test_full_not_reached_when_stable_below_target() {
    FillDetector d;
    for (uint32_t i = 0; i < 2000; i++) TEST_ASSERT_FALSE(d.isFull(motorAt(i * 10000, 2.0e6f)));
}

void test_fill_reset_clears_state() {
    FillDetector d;
    for (uint32_t i = 0; i < 400; i++) d.isFull(motorAt(i * 10000, 5.0e6f));
    d.reset();
    TEST_ASSERT_FALSE(d.isFull(motorAt(5000000, 5.0e6f)));
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

void test_vent_not_detected_when_flat() {
    VentDetector d;
    uint32_t t = 0;
    float pa = 101325.0f;
    TEST_ASSERT_FALSE(ramp([&](const MotorState& m) { return d.isVenting(m); }, t, pa, 0, 10));
    pa = 5.0e6f;
    TEST_ASSERT_FALSE(ramp([&](const MotorState& m) { return d.isVenting(m); }, t, pa, 0, 10));
}

void test_vent_not_detected_while_filling() {
    VentDetector d;
    uint32_t t = 0;
    float pa = 101325.0f;
    TEST_ASSERT_FALSE(ramp([&](const MotorState& m) { return d.isVenting(m); }, t, pa, 2.5e5f, 20));
}

void test_vent_detected_on_steady_fall() {
    VentDetector d;
    uint32_t t = 0;
    float pa = 5.0e6f;
    ramp([&](const MotorState& m) { return d.isVenting(m); }, t, pa, 0, 5);
    TEST_ASSERT_TRUE(ramp([&](const MotorState& m) { return d.isVenting(m); }, t, pa, -1.0e6f, 3));
}

void test_vent_ignores_single_drop() {
    VentDetector d;
    uint32_t t = 0;
    for (int i = 0; i < 100; i++, t += 10000) TEST_ASSERT_FALSE(d.isVenting(motorAt(t, 5.0e6f)));
    TEST_ASSERT_FALSE(d.isVenting(motorAt(t, 3.0e6f)));  // én støymåling
    t += 10000;
    for (int i = 0; i < 100; i++, t += 10000) TEST_ASSERT_FALSE(d.isVenting(motorAt(t, 5.0e6f)));
}

void test_vent_ignores_slow_leak() {
    VentDetector d;
    uint32_t t = 0;
    float pa = 5.0e6f;
    // 0,1 bar/s i to minutter: over 10 bar tapt, men altfor sakte til å være lufting.
    TEST_ASSERT_FALSE(ramp([&](const MotorState& m) { return d.isVenting(m); }, t, pa, -1.0e4f, 120));
}

void test_vent_rise_resets_the_fall() {
    VentDetector d;
    uint32_t t = 0;
    float pa = 5.0e6f;
    auto f = [&](const MotorState& m) { return d.isVenting(m); };
    // To fall på 3 bar hver, avbrutt av en stigning. Ingen av dem er stor nok alene.
    TEST_ASSERT_FALSE(ramp(f, t, pa, -1.0e6f, 0.3f));
    TEST_ASSERT_FALSE(ramp(f, t, pa, 1.0e6f, 0.2f));
    TEST_ASSERT_FALSE(ramp(f, t, pa, -1.0e6f, 0.3f));
}

void test_vent_empty_only_after_staying_near_ambient() {
    VentDetector d;
    uint32_t t = 0;
    float pa = 5.0e6f;
    auto f = [&](const MotorState& m) { return d.isEmpty(m); };
    TEST_ASSERT_FALSE(ramp(f, t, pa, -1.0e6f, 4.9f));  // fortsatt over ambient
    TEST_ASSERT_TRUE(ramp(f, t, pa, 0, 3));            // blir liggende nær ambient
}

void test_vent_empty_ignores_brief_dip() {
    VentDetector d;
    uint32_t t = 0;
    TEST_ASSERT_FALSE(d.isEmpty(motorAt(t, 3.0e6f)));
    for (int i = 0; i < 20; i++, t += 10000) TEST_ASSERT_FALSE(d.isEmpty(motorAt(t, 101325.0f)));  // 0,2 s
    for (int i = 0; i < 300; i++, t += 10000) TEST_ASSERT_FALSE(d.isEmpty(motorAt(t, 3.0e6f)));
}

void test_vent_reset_clears_state() {
    VentDetector d;
    for (uint32_t i = 0; i < 400; i++) d.isEmpty(motorAt(i * 10000, 101325.0f));
    d.reset();
    TEST_ASSERT_FALSE(d.isEmpty(motorAt(5000000, 101325.0f)));
}

// TODO: tester for StateEstimator, MotorMonitor, detektorene og FlightStateMachine

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_flight_computer_constructs);
    RUN_TEST(test_fill_not_detected_at_rest);
    RUN_TEST(test_fill_detected_during_fill);
    RUN_TEST(test_fill_ignores_single_spike);
    RUN_TEST(test_full_only_after_stable_at_target);
    RUN_TEST(test_full_resets_when_pressure_drops);
    RUN_TEST(test_full_not_reached_when_stable_below_target);
    RUN_TEST(test_fill_reset_clears_state);
    RUN_TEST(test_vent_not_detected_when_flat);
    RUN_TEST(test_vent_not_detected_while_filling);
    RUN_TEST(test_vent_detected_on_steady_fall);
    RUN_TEST(test_vent_ignores_single_drop);
    RUN_TEST(test_vent_ignores_slow_leak);
    RUN_TEST(test_vent_rise_resets_the_fall);
    RUN_TEST(test_vent_empty_only_after_staying_near_ambient);
    RUN_TEST(test_vent_empty_ignores_brief_dip);
    RUN_TEST(test_vent_reset_clears_state);
    return UNITY_END();
}
