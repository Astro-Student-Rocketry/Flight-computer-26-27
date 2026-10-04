#include <unity.h>
#include <math.h>

#include "ApogeeDetector.h"
#include "../helpers/TestHelpers.h"

void setUp() {}
void tearDown() {}

VehicleState flightAt(uint32_t us, float altM, float velMs) {
    VehicleState s = vehicleAt(us, -9.81f);
    s.altitudeM = altM;
    s.velocityMs = velMs;
    return s;
}

void test_apogee_not_detected_while_climbing() {
    SimRig r;
    ApogeeDetector d;
    r.sim.runUntil(FlightSim::Stage::Coast);
    while (r.sim.velocity() > 0) {
        r.step();
        TEST_ASSERT_FALSE(d.evaluate(vehicleFrom(r)));
    }
}

void test_apogee_detected_shortly_after_top() {
    SimRig r;
    ApogeeDetector d;
    r.sim.runUntil(FlightSim::Stage::Coast);
    bool apogee = false;
    while (!apogee && r.sim.stage() != FlightSim::Stage::Landed) {
        r.step();
        apogee = d.evaluate(vehicleFrom(r));
    }
    TEST_ASSERT_TRUE(apogee);
    // Under ett sekund etter toppen, og innenfor noen meter av den.
    TEST_ASSERT_TRUE(r.sim.velocity() > -9.81f);
    TEST_ASSERT_TRUE(r.sim.apogee() - r.sim.altitude() < 5.0f);
    TEST_ASSERT_EQUAL_FLOAT(r.sim.apogee(), d.maxAltitudeM());
}

void test_apogee_ignores_negative_velocity_without_drop() {
    ApogeeDetector d;
    uint32_t t = 0;
    // Farten svinger rundt 0 på toppen, men høyden står stille.
    for (int i = 0; i < 100; i++, t += 10000) TEST_ASSERT_FALSE(d.evaluate(flightAt(t, 1000.0f, -1.0f)));
}

void test_apogee_ignores_altitude_drop_without_negative_velocity() {
    ApogeeDetector d;
    uint32_t t = 0;
    d.evaluate(flightAt(t, 1000.0f, 50.0f));
    // Trykkstøt i barometeret gir lav høyde, men farten sier fortsatt oppover.
    for (int i = 0; i < 100; i++) {
        t += 10000;
        TEST_ASSERT_FALSE(d.evaluate(flightAt(t, 900.0f, 50.0f)));
    }
}

void test_apogee_needs_consecutive_negative_samples() {
    ApogeeDetector d;
    uint32_t t = 0;
    d.evaluate(flightAt(t, 1000.0f, 0.0f));
    for (int i = 0; i < 20; i++) {
        t += 10000;
        // Hver femte prøve er positiv, så det blir aldri 5 negative på rad.
        TEST_ASSERT_FALSE(d.evaluate(flightAt(t, 990.0f, i % 5 == 4 ? 1.0f : -10.0f)));
    }
    for (int i = 0; i < 4; i++, t += 10000) TEST_ASSERT_FALSE(d.evaluate(flightAt(t, 990.0f, -10.0f)));
    TEST_ASSERT_TRUE(d.evaluate(flightAt(t, 990.0f, -10.0f)));
}

void test_apogee_nan_altitude_is_ignored() {
    ApogeeDetector d;
    uint32_t t = 0;
    d.evaluate(flightAt(t, 1000.0f, 0.0f));
    for (int i = 0; i < 10; i++) {
        t += 10000;
        TEST_ASSERT_FALSE(d.evaluate(flightAt(t, NAN, -10.0f)));
    }
    TEST_ASSERT_EQUAL_FLOAT(1000.0f, d.maxAltitudeM());
    TEST_ASSERT_TRUE(d.evaluate(flightAt(t + 10000, 990.0f, -10.0f)));
}

void test_apogee_nan_velocity_resets_count() {
    ApogeeDetector d;
    uint32_t t = 0;
    d.evaluate(flightAt(t, 1000.0f, 0.0f));
    for (int i = 0; i < 4; i++) d.evaluate(flightAt(t += 10000, 990.0f, -10.0f));
    TEST_ASSERT_FALSE(d.evaluate(flightAt(t += 10000, 990.0f, NAN)));
    for (int i = 0; i < 4; i++) TEST_ASSERT_FALSE(d.evaluate(flightAt(t += 10000, 990.0f, -10.0f)));
}

void test_apogee_reset_clears_state() {
    ApogeeDetector d;
    uint32_t t = 0;
    d.evaluate(flightAt(t, 1000.0f, 0.0f));
    for (int i = 0; i < 10; i++) d.evaluate(flightAt(t += 10000, 990.0f, -10.0f));
    d.reset();
    TEST_ASSERT_FALSE(d.evaluate(flightAt(t += 10000, 990.0f, -10.0f)));
    TEST_ASSERT_EQUAL_FLOAT(990.0f, d.maxAltitudeM());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_apogee_not_detected_while_climbing);
    RUN_TEST(test_apogee_detected_shortly_after_top);
    RUN_TEST(test_apogee_ignores_negative_velocity_without_drop);
    RUN_TEST(test_apogee_ignores_altitude_drop_without_negative_velocity);
    RUN_TEST(test_apogee_needs_consecutive_negative_samples);
    RUN_TEST(test_apogee_nan_altitude_is_ignored);
    RUN_TEST(test_apogee_nan_velocity_resets_count);
    RUN_TEST(test_apogee_reset_clears_state);
    return UNITY_END();
}
