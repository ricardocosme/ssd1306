#pragma once

#include "ssd1306/i2c.hpp"

#include <stdint.h>

namespace ssd1306 {

struct seven_segment { uint8_t segments; };

namespace segment {
constexpr static auto top = 1<<0;
constexpr static auto left_top = 1<<1;
constexpr static auto right_top = 1<<2;
constexpr static auto middle = 1<<3;
constexpr static auto left_bottom = 1<<4;
constexpr static auto right_bottom = 1<<5;
constexpr static auto bottom = 1<<6;
} //namespace segment

namespace segments {
using namespace segment;

//number 0
constexpr static auto _0 = seven_segment
{ top | left_top | left_bottom | right_top | right_bottom | bottom };

//number 1
constexpr static auto _1 = seven_segment
{ right_top | right_bottom };

//number 2
constexpr static auto _2 = seven_segment
{ top | right_top | middle | left_bottom | bottom };

//number 3
constexpr static auto _3 = seven_segment
{ top | right_top | middle | right_bottom | bottom };

//number 4
constexpr static auto _4 = seven_segment
{ left_top | middle | right_top | right_bottom };

//number 5
constexpr static auto _5 = seven_segment
{ top | left_top | middle | right_bottom | bottom };

//number 6
constexpr static auto _6 = seven_segment
{ top | left_top | middle | left_bottom | bottom | right_bottom };

//number 7
constexpr static auto _7 = seven_segment
{ top | right_top | right_bottom };

//number 8
constexpr static auto _8 = seven_segment
{ left_top | top | right_top | middle | left_bottom
  | right_bottom | bottom };

//number 9
constexpr static auto _9 = seven_segment
{ left_top | top | right_top | middle | right_bottom | bottom };

//hyphen '-'
constexpr static auto hyphen = seven_segment{middle};

//letter i
constexpr static auto i = seven_segment{right_bottom};

//letter n
constexpr static auto n = seven_segment
{ left_bottom | middle | right_bottom };

//letter p
constexpr static auto p = seven_segment
{ left_bottom | left_top | top | middle | top };

//letter u
constexpr static auto u = seven_segment
{ left_bottom | bottom | right_bottom };

//letter t
constexpr static auto t = seven_segment
{ left_bottom | left_top | middle | bottom };

//letter o
constexpr static auto o = seven_segment
{ middle | left_bottom | right_bottom | bottom };

} //namespace segments

namespace detail {
template<int pages, typename I2C>
inline void draw_column(I2C&& i2c, uint8_t b) {
    for(uint8_t i{}; i < pages / 2; ++i)
        i2c.send_byte(b);
}
} //namespace detail

template<uint8_t width, uint8_t height, uint8_t spacing = 3, typename I2C>
void send_digit_segmented(I2C&& i2c, uint8_t segments) {
    using namespace segment;
    static_assert(width >= 12 && width <= 64);
    static_assert(height % 16 == 0);
    static_assert(height >= 16 && height <= 64);
    constexpr auto pages = height / 8;
    for(uint8_t col{0}; col < width; ++col) {
        if(col < 2) {
            if(segments & left_top) detail::draw_column<pages>(i2c, 0xff);
            else detail::draw_column<pages>(i2c, 0x00);
            if(segments & left_bottom) detail::draw_column<pages>(i2c, 0xff);
            else detail::draw_column<pages>(i2c, 0x00);
        } else if(col >= 2 && col < (width - 2)) {
            if(pages == 2) {
                if(segments & top) {
                    if(segments & middle) i2c.send_byte(0x83);
                    else i2c.send_byte(0x03);
                } else {
                    if(segments & middle) i2c.send_byte(0x80);
                    else i2c.send_byte(0x00);
                }
                if(segments & bottom) {
                    if(segments & middle) i2c.send_byte(0xc1);
                    else i2c.send_byte(0xc0);
                } else {
                    if(segments & middle) i2c.send_byte(0x01);
                    else i2c.send_byte(0x00);
                }
            } else {
                if(segments & top) i2c.send_byte(0x03);
                else i2c.send_byte(0x00);
                if(segments & middle) {
                    detail::draw_column<pages - 4>(i2c, 0x00);
                    i2c.send_byte(0x80);
                    i2c.send_byte(0x01);
                } else detail::draw_column<pages>(i2c, 0x00);
                detail::draw_column<pages - 4>(i2c, 0x00);
                if(segments & bottom) i2c.send_byte(0xc0);
                else i2c.send_byte(0x00);
            }
        } else if(col >= (width - 2) && col < width) {
            if(segments & right_top) detail::draw_column<pages>(i2c, 0xff);
            else detail::draw_column<pages>(i2c, 0x00);
            if(segments & right_bottom) detail::draw_column<pages>(i2c, 0xff);
            else detail::draw_column<pages>(i2c, 0x00);
        }
    }
    for(uint8_t i{}; i < spacing * pages; ++i)
        i2c.send_byte(0x00);
}

template<uint8_t width, uint8_t height, uint8_t spacing, typename I2C> 
void send_digit(I2C&& dev, uint8_t i) {
    using namespace segment;
    
    seven_segment digit;
    if(i == 0) digit = segments::_0;
    else if(i == 1) digit = segments::_1;
    else if(i == 2) digit = segments::_2;
    else if(i == 3) digit = segments::_3;
    else if(i == 4) digit = segments::_4;
    else if(i == 5) digit = segments::_5;
    else if(i == 6) digit = segments::_6;
    else if(i == 7) digit = segments::_7;
    else if(i == 8) digit = segments::_8;
    else  digit = segments::_9;
    
    send_digit_segmented<width, height, spacing>(dev, digit.segments);
}

template<uint8_t w, uint8_t h, uint8_t spacing = 5, typename I2C> 
uint8_t send_int(I2C&& dev, uint8_t i) {
    if(i < 10) {
        send_digit<w, h, spacing>(dev, i);
        return 1;
    }
    auto d = send_int<w, h, spacing>(dev, uint8_t(i / 10));
    return send_int<w, h, spacing>(dev, uint8_t(i % 10)) + d;
}

template<uint8_t w, uint8_t h, uint8_t spacing = 5, typename I2C> 
uint8_t send_int(I2C&& dev, uint32_t i) {
    if(i < 10) {
        send_digit<w, h, spacing>(dev, i);
        return 1;
    }
    auto d = send_int<w, h, spacing>(dev, i / 10);
    return send_int<w, h, spacing>(dev, i % 10) + d;
}

}
