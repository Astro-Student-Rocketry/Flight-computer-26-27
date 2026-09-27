#pragma once
#include <stdint.h>

// Bitflagg. Flere avvik kan være aktive samtidig.
enum class Anomaly : uint16_t {
    None = 0,
    EarlyBurnout = 1 << 0,      // burnout tidligere enn forventet
    ApogeeByTimeout = 1 << 1,   // Coast -> Descent via tidsgrense, ikke detektor
    LandingByTimeout = 1 << 2,  // Descent -> Landed via tidsgrense, ikke detektor
    SensorFault = 1 << 3,
    GpsLost = 1 << 4,
};
