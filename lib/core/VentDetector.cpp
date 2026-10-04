#include "VentDetector.h"

bool VentDetector::isVenting(const MotorState& m) {
    const float p = m.tankPressurePa;
    // Så lenge trykket er nær toppen (eller stiger), er fallet ikke begynt. Ankeret følger med,
    // så fallfarten måles fra det trykket forlot toppen, ikke fra hvor lenge det lå flatt.
    // En stigning midt i fallet starter målingen på nytt herfra.
    if (!haveAnchor_ || p >= anchorPa_ - cfg_.riseTolerancePa || p > lastPa_ + cfg_.riseTolerancePa) {
        if (!haveAnchor_ || p > anchorPa_ || p > lastPa_ + cfg_.riseTolerancePa) anchorPa_ = p;
        haveAnchor_ = true;
        anchorUs_ = m.timestampUs;
        fallSamples_ = 0;
        lastPa_ = p;
        return false;
    }
    lastPa_ = p;
    fallSamples_++;

    const float dropPa = anchorPa_ - p;
    // Differansen i uint32_t tåler at mikrosekundtelleren går rundt.
    const float elapsedS = static_cast<float>(m.timestampUs - anchorUs_) * 1e-6f;
    if (fallSamples_ < cfg_.confirmSamples || elapsedS < cfg_.ventMinSeconds) return false;
    return dropPa >= cfg_.ventDropPa && dropPa / elapsedS >= cfg_.ventMinRatePaPerS;
}

bool VentDetector::isEmpty(const MotorState& m) {
    if (m.tankPressurePa >= cfg_.ambientPa + cfg_.emptyMarginPa) {
        lowActive_ = false;
        return false;
    }
    if (!lowActive_) {
        lowActive_ = true;
        lowSinceUs_ = m.timestampUs;
        return false;
    }
    const float lowS = static_cast<float>(m.timestampUs - lowSinceUs_) * 1e-6f;
    return lowS >= cfg_.emptySeconds;
}

void VentDetector::reset() {
    haveAnchor_ = false;
    fallSamples_ = 0;
    lowActive_ = false;
}
