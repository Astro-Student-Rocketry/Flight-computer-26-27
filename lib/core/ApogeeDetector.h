#pragma once
#include "VehicleState.h"

// Oppdager apogee: farten er negativ i flere prøver på rad, og høyden har falt et stykke fra
// toppen. Begge må være sanne, så en feil i bare fart eller bare høyde ikke utløser apogee.
// Kalles én gang per tick i Coast. Tersklene er foreløpige og skal justeres mot Simulink-modellen.
// TODO: se over koden. Se punktet om ApogeeDetector i sjekklisten i README.
class ApogeeDetector {
public:
    struct Config {
        float minDropM = 2.0f;  // høyden må ha falt minst så mye under høyeste målte høyde
        int confirmSamples = 5; // prøver på rad med negativ fart
    };

    ApogeeDetector() = default;
    explicit ApogeeDetector(const Config& cfg) : cfg_(cfg) {}

    bool evaluate(const VehicleState& s);
    // Nullstiller telleren og høyeste høyde. Kalles når fasen byttes.
    void reset();

    // Høyeste høyde siden reset(). Gyldig når evaluate() har fått minst én gyldig høyde.
    float maxAltitudeM() const { return maxAltM_; }

private:
    Config cfg_;
    int count_ = 0;
    bool haveMax_ = false;
    float maxAltM_ = 0;
};
