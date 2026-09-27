#include "DataLogger.h"

DataLogger::DataLogger(IStorage& storage) : storage_(storage) {}

void DataLogger::update(const VehicleState& s, const MotorState& m, const RawSensorData& raw) {
    (void)s;
    (void)m;
    (void)raw;
    // TODO
}

void DataLogger::logEvent(const Event& e) {
    (void)e;
    // TODO
}

void DataLogger::flush() {
    // TODO
}
