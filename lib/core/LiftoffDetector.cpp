#include "LiftoffDetector.h"

bool LiftoffDetector::evaluate(const VehicleState& s) {
    // NaN fra en defekt IMU er aldri over terskelen, så den nullstiller telleren.
    if (s.accelMs2 > cfg_.thresholdMs2) {
        if (count_ < cfg_.confirmSamples) count_++;
    } else {
        count_ = 0;
    }
    return count_ >= cfg_.confirmSamples;
}

void LiftoffDetector::reset() { count_ = 0; }
