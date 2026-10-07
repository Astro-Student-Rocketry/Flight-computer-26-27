#include <unity.h>
#include <math.h>

#include "FillDetector.h"
#include "MotorMonitor.h"
#include "../fakes/FakePressureSensor.h"
#include "../fakes/FakeTemperatureSensor.h"
#include "../fakes/FakeValveSensor.h"

void setUp() {}
void tearDown() {}

struct MotorRig {
    FakePressureSensor chamber, tank;
    FakeTemperatureSensor temp;
    FakeValveSensor valve;
    MotorMonitor mm{chamber, tank, temp, valve};
};

void test_motor_monitor_reads_all_sensors() {
    MotorRig r;
    r.chamber.pressure = 3.0e6f;
    r.tank.pressure = 4.0e6f;
    r.temp.temp = 80.0f;
    r.valve.open = true;
    r.mm.update(123456);
    const MotorState& s = r.mm.state();
    TEST_ASSERT_EQUAL_FLOAT(3.0e6f, s.chamberPressurePa);
    TEST_ASSERT_EQUAL_FLOAT(4.0e6f, s.tankPressurePa);
    TEST_ASSERT_EQUAL_FLOAT(80.0f, s.temperatureC);
    TEST_ASSERT_TRUE(s.valveOpen);
    TEST_ASSERT_TRUE(s.timestampUs == 123456u);
    TEST_ASSERT_FALSE(r.mm.hasFault());
}

void test_motor_monitor_keeps_last_good_value_on_failure() {
    MotorRig r;
    r.tank.pressure = 4.0e6f;
    r.mm.update(1000);
    r.tank.readOk = false;
    r.tank.pressure = 0.0f;
    r.mm.update(2000);
    TEST_ASSERT_EQUAL_FLOAT(4.0e6f, r.mm.state().tankPressurePa);  // ikke 0
    TEST_ASSERT_TRUE(r.mm.state().timestampUs == 2000u);
}

void test_motor_monitor_fault_needs_consecutive_failures() {
    MotorRig r;
    r.tank.readOk = false;
    r.mm.update(1000);
    TEST_ASSERT_FALSE(r.mm.hasFault());  // én feil er ikke nok
    r.tank.readOk = true;
    r.mm.update(2000);
    r.tank.readOk = false;
    r.mm.update(3000);
    r.mm.update(4000);
    TEST_ASSERT_FALSE(r.mm.hasFault());  // telleren ble nullstilt av den gode lesningen
    r.mm.update(5000);
    TEST_ASSERT_TRUE(r.mm.hasFault());
}

void test_motor_monitor_fault_clears_when_sensor_recovers() {
    MotorRig r;
    r.temp.readOk = false;
    for (uint32_t i = 1; i <= 5; i++) r.mm.update(i * 1000);
    TEST_ASSERT_TRUE(r.mm.hasFault());
    r.temp.readOk = true;
    r.mm.update(10000);
    TEST_ASSERT_FALSE(r.mm.hasFault());
}

void test_motor_monitor_fault_mask_names_the_sensor() {
    MotorRig r;
    r.valve.readOk = false;
    for (uint32_t i = 1; i <= 5; i++) r.mm.update(i * 1000);
    TEST_ASSERT_TRUE(r.mm.faultMask() == MotorMonitor::kValveFault);
}

void test_motor_monitor_rejects_invalid_values() {
    MotorRig r;
    r.tank.pressure = 4.0e6f;
    r.mm.update(1000);
    r.tank.pressure = NAN;
    r.mm.update(2000);
    TEST_ASSERT_EQUAL_FLOAT(4.0e6f, r.mm.state().tankPressurePa);
    r.tank.pressure = -5.0f;  // negativt absoluttrykk er umulig
    r.mm.update(3000);
    TEST_ASSERT_EQUAL_FLOAT(4.0e6f, r.mm.state().tankPressurePa);
    r.mm.update(4000);
    TEST_ASSERT_TRUE(r.mm.hasFault());
}

void test_motor_monitor_feeds_fill_detector() {
    MotorRig r;
    FillDetector d;
    r.tank.pressure = 1.0e6f;
    bool filling = false;
    for (uint32_t i = 1; i <= 10; i++) {
        r.mm.update(i * 10000);
        filling |= d.isFilling(r.mm.state());
    }
    TEST_ASSERT_TRUE(filling);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_motor_monitor_reads_all_sensors);
    RUN_TEST(test_motor_monitor_keeps_last_good_value_on_failure);
    RUN_TEST(test_motor_monitor_fault_needs_consecutive_failures);
    RUN_TEST(test_motor_monitor_fault_clears_when_sensor_recovers);
    RUN_TEST(test_motor_monitor_fault_mask_names_the_sensor);
    RUN_TEST(test_motor_monitor_rejects_invalid_values);
    RUN_TEST(test_motor_monitor_feeds_fill_detector);
    return UNITY_END();
}
