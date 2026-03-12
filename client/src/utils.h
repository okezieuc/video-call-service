#pragma once

#include <cstdint>

enum class Tristate : uint8_t {
    Unknown = 0,
    False   = 1,
    True    = 2
};

