#pragma once
#include <stdint.h>

enum class Phase : uint8_t { Pad, Boost, Coast, Descent, Landed };
