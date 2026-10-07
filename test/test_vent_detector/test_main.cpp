#include <unity.h>

#include "VentDetector.h"
#include "../helpers/TestHelpers.h"

void setUp() {}
void tearDown() {}

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

int main() {
    UNITY_BEGIN();
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
