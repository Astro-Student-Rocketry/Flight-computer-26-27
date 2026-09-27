#include "DataLogger.h"

DataLogger::DataLogger(IStorage& storage) : storage_(storage) {}

void DataLogger::update(const VehicleState& s, const MotorState& m) {
    (void)s;
    (void)m;
    // TODO
}

void DataLogger::flush() {
    // TODO
}
