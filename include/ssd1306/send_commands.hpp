#pragma once

#include "ssd1306/i2c.hpp"

#include <stdint.h>

namespace ssd1306 {

template<typename I2C, typename T, int N>
void send_commands(I2C&& dev, const T(&cmds)[N]) {
    dev.start_commands();
    for(uint8_t i{0}; i < N; ++i)
        dev.send_byte(cmds[i]);
    dev.stop_condition();
}

}
