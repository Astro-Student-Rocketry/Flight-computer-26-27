#include "FillDetector.h"

#include <math.h>

bool FillDetector::isFilling(const MotorState& m) {
    if (m.tankPressurePa > cfg_.ambientPa + cfg_.fillMarginPa) {
        if (fillingCount_ < cfg_.confirmSamples) fillingCount_++;
    } else {
        fillingCount_ = 0;
    }
    return fillingCount_ >= cfg_.confirmSamples;
}

bool FillDetector::isFull(const MotorState& m) {
    if (m.tankPressurePa < cfg_.targetPa) {
        stableActive_ = false;
        return false;
    }
    // Starter på nytt hver gang trykket flytter seg ut av båndet.
    if (!stableActive_ || fabsf(m.tankPressurePa - stableRefPa_) > cfg_.stableBandPa) {
        stableActive_ = true;
        stableRefPa_ = m.tankPressurePa;
        stableSinceUs_ = m.timestampUs;
        return false;
    }
    // Differansen i uint32_t tåler at mikrosekundtelleren går rundt.
    float stableS = static_cast<float>(m.timestampUs - stableSinceUs_) * 1e-6f;
    return stableS >= cfg_.stableSeconds;
}

void FillDetector::reset() {
    fillingCount_ = 0;
    stableActive_ = false;
}
