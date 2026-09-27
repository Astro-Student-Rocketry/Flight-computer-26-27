#pragma once
#include "IStorage.h"
#include "MotorState.h"
#include "VehicleState.h"

class DataLogger {
public:
    explicit DataLogger(IStorage& storage);

    void update(const VehicleState& s, const MotorState& m);
    void flush();

private:
    IStorage& storage_;
};
