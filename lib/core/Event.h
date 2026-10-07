#pragma once
#include <stdint.h>

enum class EventType : uint8_t {
    PhaseChange,  // code = ny Phase
    Anomaly,      // code = Anomaly-bit
    RailExit,     // value = hastighet ved rampeslutt (m/s)
};

// En linje i hendelsesloggen.
struct Event {
    uint32_t timestampUs = 0;
    EventType type = EventType::PhaseChange;
    uint16_t code = 0;
    float value = 0;
};
