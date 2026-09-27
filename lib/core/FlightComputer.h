#pragma once
#include "DataLogger.h"
#include "FlightStateMachine.h"
#include "HealthMonitor.h"
#include "IBarometer.h"
#include "IImu.h"
#include "IRadio.h"
#include "IStorage.h"
#include "StateEstimator.h"
#include "TelemetryEncoder.h"
#include "TelemetryLink.h"

class FlightComputer {
public:
    FlightComputer(IBarometer& baro, IImu& imu, IStorage& storage, IRadio& radio);

    bool begin();
    void tick(float dt);

private:
    IBarometer& baro_;
    IImu& imu_;
    IStorage& storage_;
    IRadio& radio_;

    // Rekkefølgen er viktig: medlemmene bygges i denne rekkefølgen.
    HealthMonitor health_;
    StateEstimator estimator_;
    FlightStateMachine fsm_;
    DataLogger logger_;
    TelemetryEncoder encoder_;
    TelemetryLink link_;
};
