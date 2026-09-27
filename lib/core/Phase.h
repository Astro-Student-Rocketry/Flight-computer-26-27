#pragma once
#include <stdint.h>

enum class Phase : uint8_t { Pad, Liftoff, Boost, Coast, Descent, Landed };
