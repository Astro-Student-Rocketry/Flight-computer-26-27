#include "ApogeeDetector.h"

#include <math.h>

bool ApogeeDetector::evaluate(const VehicleState& s) {
    // NaN-høyde skal verken bli ny topp eller telle som fall.
    const bool altValid = !isnan(s.altitudeM);
    if (altValid && (!haveMax_ || s.altitudeM > maxAltM_)) {
        haveMax_ = true;
        maxAltM_ = s.altitudeM;
    }

    // NaN-fart er aldri under 0, så den nullstiller telleren.
    if (s.velocityMs < 0.0f) {
        if (count_ < cfg_.confirmSamples) count_++;
    } else {
        count_ = 0;
    }

    const bool dropped = altValid && haveMax_ && s.altitudeM <= maxAltM_ - cfg_.minDropM;
    return count_ >= cfg_.confirmSamples && dropped;
}

void ApogeeDetector::reset() {
    count_ = 0;
    haveMax_ = false;
}
