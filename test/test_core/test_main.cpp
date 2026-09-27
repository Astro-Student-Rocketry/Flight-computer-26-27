#include <unity.h>

#include "../fakes/FakeBarometer.h"
#include "../fakes/FakeImu.h"
#include "../fakes/FakePyro.h"
#include "../fakes/FakeRadio.h"
#include "../fakes/FakeStorage.h"
#include "FlightComputer.h"

void setUp() {}
void tearDown() {}

void test_flight_computer_constructs() {
    FakeBarometer baro;
    FakeImu imu;
    FakeStorage storage;
    FakeRadio radio;
    FakePyro pyro;
    FlightComputer fc(baro, imu, storage, radio, pyro);
    fc.tick(0.01f);
    TEST_PASS();
}

// TODO: tester for StateEstimator, detektorene, FlightStateMachine og RecoveryController

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_flight_computer_constructs);
    return UNITY_END();
}
