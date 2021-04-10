#pragma once

#include "ssd1306/commands.hpp"
#include "ssd1306/detail/merge_cmds.hpp"
#include "ssd1306/i2c.hpp"
#include "ssd1306/send_commands.hpp"
#include "ssd1306/send_seven_segment.hpp"
#include "ssd1306/set_page_column.hpp"

#include <stdint.h>

namespace ssd1306 {

template<typename T>
struct repeat{ T value; };

template<typename Sda, typename Scl>
class display {
    template<uint8_t w, uint8_t h>
    void out_impl(seven_segment seg)
    { send_digit_segmented<w,h>(_i2c, seg.segments); }

    template<uint8_t w, uint8_t h>
    void out_impl(uint8_t v)
    { send_int<w, h>(_i2c, v); }

    template<uint8_t w, uint8_t h>
    uint8_t out_impl(uint32_t n) {
        auto whole = n / 100;
        auto decimal = n % 100;

        auto d = decimal / 10;
        auto r = decimal % 10;
        if(r >= 5) {
            if(d == 9) {
                ++whole;
                d = 0;
            } else ++d;
        }
        auto n_digits = send_int<w, h>(_i2c, whole);
        static const uint8_t dot[] = {
            0x00, 0x00, 0x00, 0xf0,
            0x00, 0x00, 0x00, 0xf0,
            0x00, 0x00, 0x00, 0xf0,
            0x00, 0x00, 0x00, 0xf0,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            };
        for(uint8_t i{}; i < sizeof(dot); ++i)
            _i2c.send_byte(dot[i]);
        
        return n_digits + send_int<w, h>(_i2c, d);
    }
    
    template<uint8_t w, uint8_t h, int N>
    void out_impl(const uint8_t (&bytes)[N]) {
        for(uint8_t i{}; i < N; ++i)
            _i2c.send_byte(bytes[i]);
    }
public:
    using i2c_t = ::ssd1306::i2c<Sda, Scl>;
private:
    i2c_t _i2c;
public:
    display() = default;
    
    template<typename... Cmds>
    display(Sda sda, Scl scl, Cmds... cmds)
        : _i2c(sda, scl)
    {
        constexpr static uint8_t data[] = {
            0xC8, /** COM Output Scan Direction*/ 
            0xA1, /** Segment Re-map */

            0x20, 1, /** Vertical Addressing Mode*/ 
            0x22, 0, 7, /** Set page address: 0 to 7*/  
            0x21, 0, 127, /** Set column address: 0 to 127*/
    
            0x8D, 0x14, /** Enable Charge Pump*/
        };
        
        constexpr static auto data2 = detail::merge_cmds(i2c_t{}, data, cmds...);
        send_commands(_i2c, data2.data);
        
        /** clear the whole screen */
        out(0x00, repeat<uint16_t>{128 * 8});
    }

    template<uint8_t w = 0, uint8_t h = 0>
    uint8_t out(page pg, column col, int32_t n) {
        set(_i2c, pg, col);
        _i2c.start_data();
        uint32_t un;
        if(n < 0) {
            send_digit_segmented<w, h>(_i2c, segments::hyphen.segments);
            un = n * -1;
        } else un = n;
        auto n_digits = out_impl<w, h>(un);
        _i2c.stop_condition();
        return n_digits;
    }
    
    template<uint8_t w = 0, uint8_t h = 0>
    void out(uint8_t n) {
        _i2c.start_data();
        out_impl<w, h>(n);
        _i2c.stop_condition();
    }
    
    template<uint8_t w = 0, uint8_t h = 0, typename... Segs>
    void out(column col, Segs... segs) {
        set(_i2c, col);
        _i2c.start_data();
        (out_impl<w, h>(segs), ...);
        _i2c.stop_condition();
    }

    template<uint8_t w = 0, uint8_t h = 0, typename... Segs>
    void out(page pg, column col, Segs... segs) {
        set(_i2c, pg, col);
        _i2c.start_data();
        (out_impl<w, h>(segs), ...);
        _i2c.stop_condition();
    }

    template<uint8_t w = 0, uint8_t h = 0, int N>
    void out(const uint8_t (&bytes)[N]) {
        _i2c.start_data();
        out_impl<w, h>(bytes);
        _i2c.stop_condition();
    }

    
    template<int N>
    void out(page pg, const uint8_t (&bytes)[N]) {
        set(_i2c, pg);
        out(bytes);
    }
    
    template<int N>
    void out(column col, const uint8_t (&bytes)[N]) {
        set(_i2c, col);
        out(bytes);
    }

    template<int N>
    void out(page pg, column col, const uint8_t (&bytes)[N]) {
        set(_i2c, pg, col);
        out(bytes);
    }

    template<typename T>
    void out(uint8_t byte, const repeat<T>& rep) {
        _i2c.start_data();
        for(T i{}; i < rep.value; ++i)
            _i2c.send_byte(byte);
        _i2c.stop_condition();
    }

    template<typename T>
    void out(page pg, uint8_t byte, const repeat<T>& rep) {
        set(_i2c, pg);
        out(byte, rep);
    }

    template<typename T>
    void out(column col, uint8_t byte, const repeat<T>& rep) {
        set(_i2c, col);
        out(byte, rep);
    }
    
    template<typename T>
    void out(page pg, column col, uint8_t byte, const repeat<T>& rep) {
        set(_i2c, pg, col);
        out(byte, rep);
    }
};

}
