#include <avr/io.hpp>
#include <avr/pgmspace.h>
#include <ssd1306.hpp>
#define F_CPU 1.2e6
#include <util/delay.h>
using namespace avr::io;
using namespace ssd1306;

/** This demo setups a display with 128x64 dots with some basic
    commands and after that erases the content of the whole screen to
    print a square of 8x8 pixels.
*/

static const uint8_t cmds[] [[gnu::__progmem__]] = {
    /** Commands to inform the driver what is the physical
        configuration of the display. Note that your display can be
        different, take a look at the secton 10.1.18 of the datasheet
        with the result on the screen is weird. 
    */
    0xC8, /** COM Output Scan Direction*/ 
    0xA1, /** Segment Re-map */
    
    0x20, 1, /** Vertical Addressing Mode*/ 
    0x22, 0, 7, /** Set page address: 0 to 7*/  
    0x21, 0, 127, /** Set column address: 0 to 127*/
    
    0x8D, 0x14, /** Enable Charge Pump*/
    
    0xAF, /** Turn on the display */
};

constexpr static auto top = 1<<0;
constexpr static auto left_top = 1<<1;
constexpr static auto right_top = 1<<2;
constexpr static auto middle = 1<<3;
constexpr static auto left_bottom = 1<<4;
constexpr static auto right_bottom = 1<<5;
constexpr static auto bottom = 1<<6;

template<typename I2C>
void send_digit(I2C& i2c, uint8_t segments, uint8_t size) {
    for(uint8_t col{0}; col < 17; ++col) {
        if(col < 2) {
            for(uint8_t i{}; i < size; ++i) {
                if(segments & left_top) {
                    i2c.send_byte(0xff);
                } else i2c.send_byte(0x00);
            }
            for(uint8_t i{}; i < size; ++i) {
                if(segments & left_bottom) {
                    i2c.send_byte(0xff);
                } else i2c.send_byte(0x00);
            }
        } else if(col > 2 && col < 14) {
            if(segments & top) {
                i2c.send_byte(0x03);
            } else {
                i2c.send_byte(0x00);
            } 
            if(segments & middle) {
                i2c.send_byte(0x80);
                i2c.send_byte(0x01);
            } else {
                i2c.send_byte(0x00); 
                i2c.send_byte(0x00);
            }
            if(segments & bottom) i2c.send_byte(0xc0);
            else i2c.send_byte(0x00);
        } else if(col > 14 && col < 17) {
            for(uint8_t i{}; i < size; ++i) {
                if(segments & right_top) {
                    i2c.send_byte(0xff);
                } else i2c.send_byte(0x00);
            }
            for(uint8_t i{}; i < size; ++i) {
                if(segments & right_bottom) {
                    i2c.send_byte(0xff);
                } else i2c.send_byte(0x00);
            }
        }
    }
    for(uint8_t i{}; i < 3 * 4; ++i) {
        i2c.send_byte(0x00);
    }
}

template<typename I2C> 
inline void send_digit(I2C& dev, uint8_t i) {
    uint8_t segments;
    if(i == 0)
        segments = top | left_top | left_bottom | right_top | right_bottom
            | bottom;
    else if(i == 1)
        segments = right_top| right_bottom;
    else if(i == 2)
        segments = top | right_top | middle | left_bottom | bottom;
    else if(i == 3)
        segments = top | right_top | middle | right_bottom | bottom;
    else if(i == 4)
        segments = left_top | middle | right_top | right_bottom;
    else if(i == 5)
        segments = top | left_top | middle | right_bottom | bottom;
    else if(i == 6)
        segments = left_top | middle | left_bottom | bottom | right_bottom;
    else if(i == 7)
        segments = top | right_top | right_bottom;
    else if(i == 8)
        segments = left_top | top | right_top | middle | left_bottom
            | right_bottom | bottom;
    else if(i == 9)
        segments = left_top | top | right_top | middle | right_bottom | bottom;
    send_digit(dev, segments, 2);
}

template<typename I2C> 
inline void send_int(I2C& dev, uint8_t i) {
    if(i < 10) {
        send_digit(dev, i);
        return;
    }
    send_int(dev, i / 10);
    send_int(dev, i % 10);
}

int main() {
    ssd1306::i2c dev{pb0, pb2};

    /** setup the display */
    dev.start_commands();
    for(uint8_t i{0}; i < sizeof(cmds); ++i)
        dev.send_byte(pgm_read_byte(&cmds[i]));
    dev.stop_condition();

    /** clear the whole screen */
    dev.start_data();
    for(uint16_t i{0}; i < 128 * 8; ++i)
        dev.send_byte(0x00);
    dev.stop_condition();

    dev.start_commands();
    dev.send_byte(0x22);
    dev.send_byte(2);
    dev.send_byte(5);
    dev.stop_condition();
    
    for(uint8_t i{}; i < 255; ++i) {
        dev.start_commands();
        dev.send_byte(0x21);
        dev.send_byte(0);
        dev.send_byte(127);
        dev.stop_condition();
        
        dev.start_data();
        send_int(dev, i);
        dev.stop_condition();
        _delay_ms(500);
    }
    
    while(true);
}
