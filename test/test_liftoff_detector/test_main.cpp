#include <unity.h>
#include <math.h>

#include "LiftoffDetector.h"
#include "../helpers/TestHelpers.h"

void setUp() {}
void tearDown() {}

void test_liftoff_not_detected_on_pad() {
    SimRig r;
    LiftoffDetector d;
    while (r.sim.stage() != FlightSim::Stage::Boost) {
        r.step();
        TEST_ASSERT_FALSE(d.evaluate(vehicleFrom(r)));
    }
}

void test_liftoff_detected_early_in_boost() {
    SimRig r;
    LiftoffDetector d;
    r.sim.runUntil(FlightSim::Stage::Boost);
    int steps = 0;
    bool liftoff = false;
    while (!liftoff && steps < 100) {
        r.step();
        liftoff = d.evaluate(vehicleFrom(r));
        steps++;
    }
    TEST_ASSERT_TRUE(liftoff);
    TEST_ASSERT_EQUAL_INT(5, steps);         // akkurat confirmSamples prøver
    TEST_ASSERT_TRUE(r.sim.altitude() < r.sim.railLength());  // fortsatt på rampa
}

void test_liftoff_ignores_short_bump() {
    LiftoffDetector d;
    uint32_t t = 0;
    for (int i = 0; i < 4; i++, t += 10000) TEST_ASSERT_FALSE(d.evaluate(vehicleAt(t, 100.0f)));
    TEST_ASSERT_FALSE(d.evaluate(vehicleAt(t, 0.0f)));
    t += 10000;
    for (int i = 0; i < 4; i++, t += 10000) TEST_ASSERT_FALSE(d.evaluate(vehicleAt(t, 100.0f)));
}

void test_liftoff_ignores_sustained_handling() {
    LiftoffDetector d;
    // 2 g i lang tid, f.eks. når raketten løftes eller rampa reises.
    for (uint32_t i = 0; i < 500; i++) TEST_ASSERT_FALSE(d.evaluate(vehicleAt(i * 10000, 20.0f)));
}

void test_liftoff_nan_resets_count() {
    LiftoffDetector d;
    uint32_t t = 0;
    for (int i = 0; i < 4; i++, t += 10000) d.evaluate(vehicleAt(t, 80.0f));
    TEST_ASSERT_FALSE(d.evaluate(vehicleAt(t, NAN)));
    t += 10000;
    for (int i = 0; i < 4; i++, t += 10000) TEST_ASSERT_FALSE(d.evaluate(vehicleAt(t, 80.0f)));
    TEST_ASSERT_TRUE(d.evaluate(vehicleAt(t, 80.0f)));
}

void test_liftoff_stays_true_while_accelerating() {
    LiftoffDetector d;
    for (uint32_t i = 0; i < 5; i++) d.evaluate(vehicleAt(i * 10000, 80.0f));
    for (uint32_t i = 5; i < 100; i++) TEST_ASSERT_TRUE(d.evaluate(vehicleAt(i * 10000, 80.0f)));
}

void test_liftoff_reset_clears_state() {
    LiftoffDetector d;
    for (uint32_t i = 0; i < 4; i++) d.evaluate(vehicleAt(i * 10000, 80.0f));
    d.reset();
    TEST_ASSERT_FALSE(d.evaluate(vehicleAt(40000, 80.0f)));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_liftoff_not_detected_on_pad);
    RUN_TEST(test_liftoff_detected_early_in_boost);
    RUN_TEST(test_liftoff_ignores_short_bump);
    RUN_TEST(test_liftoff_ignores_sustained_handling);
    RUN_TEST(test_liftoff_nan_resets_count);
    RUN_TEST(test_liftoff_stays_true_while_accelerating);
    RUN_TEST(test_liftoff_reset_clears_state);
    return UNITY_END();
}
