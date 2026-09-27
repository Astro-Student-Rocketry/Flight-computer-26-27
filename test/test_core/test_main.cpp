#include <unity.h>

#include "../fakes/FakeBarometer.h"
#include "../fakes/FakeImu.h"
#include "../fakes/FakePressureSensor.h"
#include "../fakes/FakeRadio.h"
#include "../fakes/FakeStorage.h"
#include "../fakes/FakeTemperatureSensor.h"
#include "../fakes/FakeValveSensor.h"
#include "FlightComputer.h"

void setUp() {}
void tearDown() {}

void test_flight_computer_constructs() {
    FakeBarometer baro;
    FakeImu imu;
    FakeStorage storage;
    FakeRadio radio;
    FakePressureSensor chamberPressure;
    FakePressureSensor tankPressure;
    FakeTemperatureSensor motorTemp;
    FakeValveSensor valve;
    FlightComputer fc(baro, imu, storage, radio, chamberPressure, tankPressure, motorTemp, valve);
    fc.tick(0.01f);
    TEST_PASS();
}

// TODO: tester for StateEstimator, MotorMonitor, detektorene og FlightStateMachine

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_flight_computer_constructs);
    return UNITY_END();
}
