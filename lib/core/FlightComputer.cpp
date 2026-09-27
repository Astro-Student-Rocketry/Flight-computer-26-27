#include "FlightComputer.h"

FlightComputer::FlightComputer(IBarometer& baro, IImu& imu, IStorage& storage, IRadio& radio,
                               IPyro& pyro)
    : baro_(baro),
      imu_(imu),
      storage_(storage),
      radio_(radio),
      pyro_(pyro),
      estimator_(baro, imu),
      recovery_(health_, pyro),
      logger_(storage),
      link_(radio) {}

bool FlightComputer::begin() {
    // TODO: begin() på alle drivere
    return false;
}

void FlightComputer::tick(float dt) {
    (void)dt;
    // TODO: health -> estimator -> fsm -> recovery -> logger -> telemetri
}
