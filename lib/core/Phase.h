#pragma once
#include <stdint.h>

enum class Phase : uint8_t {
    Idle,     // tom tank, trygg
    Filling,  // oksidator fylles
    Filled,   // klar, venter på oppskyting
    Venting,  // tømmer/lufter tanken (avbrutt fylling)
    Liftoff,
    Boost,    // motoren brenner
    Coast,    // burnout
    Descent,
    Landed
};
