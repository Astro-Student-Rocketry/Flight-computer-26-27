#include <unity.h>

#include "../fakes/FlightSim.h"
#include "../fakes/FakeRadio.h"
#include "../fakes/FakeStorage.h"

void setUp() {}
void tearDown() {}

struct Rig {
    FakeBarometer baro;
    FakeImu imu;
    FakeGps gps;
    FakePressureSensor chamber, tank;
    FakeTemperatureSensor temp;
    FakeValveSensor valve;
    FlightSim sim{baro, imu, gps, chamber, tank, temp, valve};
};

void test_sensor_fault_injection() {
    FakeBarometer baro;
    float p = 0, t = 0;
    TEST_ASSERT_TRUE(baro.read(p, t));
    baro.readOk = false;
    TEST_ASSERT_FALSE(baro.read(p, t));
    TEST_ASSERT_EQUAL(2, baro.readCount);
}

void test_storage_records_bytes() {
    FakeStorage s;
    const uint8_t a[] = {1, 2, 3};
    TEST_ASSERT_TRUE(s.write(a, 3));
    s.writeOk = false;
    TEST_ASSERT_FALSE(s.write(a, 3));
    TEST_ASSERT_EQUAL(3, s.bytesWritten);
    TEST_ASSERT_EQUAL(3, s.buffer.size());
}

void test_radio_records_packets() {
    FakeRadio r;
    const uint8_t a[] = {9, 8};
    r.send(a, 2);
    TEST_ASSERT_EQUAL(1, r.packets.size());
    TEST_ASSERT_EQUAL(8, r.packets[0][1]);
}

void test_sim_reaches_every_stage_in_order() {
    Rig r;
    using S = FlightSim::Stage;
    for (S s : {S::Fill, S::Filled, S::Boost, S::Coast, S::Descent, S::Landed})
        TEST_ASSERT_TRUE(r.sim.runUntil(s));
    TEST_ASSERT_TRUE(r.sim.apogee() > 500.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, r.sim.altitude());
}

void test_sim_fill_raises_tank_pressure_and_opens_valve() {
    Rig r;
    r.sim.runUntil(FlightSim::Stage::Fill);
    for (int i = 0; i < 1000; i++) r.sim.step(0.01f);
    TEST_ASSERT_TRUE(r.tank.pressure > 101325.0f);
    TEST_ASSERT_TRUE(r.valve.open);
}

void test_sim_imu_shows_boost_acceleration() {
    Rig r;
    r.sim.runUntil(FlightSim::Stage::Boost);
    r.sim.step(0.01f);
    TEST_ASSERT_TRUE(r.imu.sample.az > 80.0f);
    r.sim.runUntil(FlightSim::Stage::Coast);
    r.sim.step(0.01f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, r.imu.sample.az);
}

void test_sim_pressure_falls_with_altitude() {
    Rig r;
    r.sim.runUntil(FlightSim::Stage::Coast);
    r.sim.step(0.01f);
    TEST_ASSERT_TRUE(r.baro.pressure < 101325.0f);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_sensor_fault_injection);
    RUN_TEST(test_storage_records_bytes);
    RUN_TEST(test_radio_records_packets);
    RUN_TEST(test_sim_reaches_every_stage_in_order);
    RUN_TEST(test_sim_fill_raises_tank_pressure_and_opens_valve);
    RUN_TEST(test_sim_imu_shows_boost_acceleration);
    RUN_TEST(test_sim_pressure_falls_with_altitude);
    return UNITY_END();
}
