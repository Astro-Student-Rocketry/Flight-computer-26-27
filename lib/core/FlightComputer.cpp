#include "FlightComputer.h"

FlightComputer::FlightComputer(IBarometer& baro, IImu& imu, IGps& gps, IStorage& storage,
                               IRadio& radio,
                               IPressureSensor& chamberPressure, IPressureSensor& tankPressure,
                               ITemperatureSensor& motorTemp, IValveSensor& valve)
    : baro_(baro),
      imu_(imu),
      gps_(gps),
      storage_(storage),
      radio_(radio),
      estimator_(baro, imu, gps),
      motor_(chamberPressure, tankPressure, motorTemp, valve),
      logger_(storage),
      link_(radio) {}

bool FlightComputer::begin() {
    // TODO: begin() på alle drivere
    return false;
}

void FlightComputer::tick(float dt) {
    (void)dt;
    // TODO: health -> estimator -> motor -> fsm -> logger -> telemetri
    // TODO: logg Event ved faseendring (lastPhase_), nye avvik (lastAnomalies_) og rampeslutt
}
