#pragma once

#include <stdint.h>

namespace ssd1306 {

template<uint8_t plevel>
struct contrast{
    constexpr static uint8_t level{plevel};
    constexpr static int size{2};
};

enum class turn{ on, off };

struct turn_on{
    constexpr static uint8_t code{0xaf};
    constexpr static int size{1};
};

struct turn_off{
    constexpr static uint8_t code{0xae};
    constexpr static int size{1};
};

}
