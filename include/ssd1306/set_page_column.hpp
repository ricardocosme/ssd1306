#pragma once

#include "ssd1306/i2c.hpp"

#include <stdint.h>

namespace ssd1306 {

struct page {
    uint8_t start{0}, end{7};
};

struct column {
    uint8_t start{0}, end{127};
};

template<typename I2C>
[[gnu::always_inline]] inline
void set(I2C&& i2c, page pg, column col) {
    i2c.start_commands();
    i2c.send_byte(0x22);
    i2c.send_byte(pg.start);
    i2c.send_byte(pg.end);
    i2c.send_byte(0x21);
    i2c.send_byte(col.start);
    i2c.send_byte(col.end);
    i2c.stop_condition();
}

template<typename I2C>
[[gnu::always_inline]] inline
void set(I2C&& i2c, column col) {
    i2c.start_commands();
    i2c.send_byte(0x21);
    i2c.send_byte(col.start);
    i2c.send_byte(col.end);
    i2c.stop_condition();
}

template<typename I2C>
[[gnu::always_inline]] inline
void set(I2C&& i2c, page pg) {
    i2c.start_commands();
    i2c.send_byte(0x22);
    i2c.send_byte(pg.start);
    i2c.send_byte(pg.end);
    i2c.stop_condition();
}

}
