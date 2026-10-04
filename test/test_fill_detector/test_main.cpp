#include <unity.h>

#include "FillDetector.h"
#include "../helpers/TestHelpers.h"

void setUp() {}
void tearDown() {}

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

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_fill_not_detected_at_rest);
    RUN_TEST(test_fill_detected_during_fill);
    RUN_TEST(test_fill_ignores_single_spike);
    RUN_TEST(test_full_only_after_stable_at_target);
    RUN_TEST(test_full_resets_when_pressure_drops);
    RUN_TEST(test_full_not_reached_when_stable_below_target);
    RUN_TEST(test_fill_reset_clears_state);
    return UNITY_END();
}
