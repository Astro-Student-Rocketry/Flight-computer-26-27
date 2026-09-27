#pragma once
#include "IStorage.h"
#include "VehicleState.h"

class DataLogger {
public:
    explicit DataLogger(IStorage& storage);

    void update(const VehicleState& s);
    void flush();

private:
    IStorage& storage_;
};
