#pragma once
#include "Event.h"
#include "IStorage.h"
#include "MotorState.h"
#include "RawSensorData.h"
#include "VehicleState.h"

class DataLogger {
public:
    explicit DataLogger(IStorage& storage);

    void update(const VehicleState& s, const MotorState& m, const RawSensorData& raw);
    // Hendelsesloggen: faseendringer, avvik og rampeslutt.
    void logEvent(const Event& e);
    void flush();

private:
    IStorage& storage_;
};
