#include "DataLogger.h"

DataLogger::DataLogger(IStorage& storage) : storage_(storage) {}

void DataLogger::update(const VehicleState& s) {
    (void)s;
    // TODO
}

void DataLogger::flush() {
    // TODO
}
