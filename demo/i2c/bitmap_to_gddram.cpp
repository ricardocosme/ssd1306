#include <avr/io.hpp>
#include <avr/pgmspace.h>
#include <ssd1306.hpp>

using namespace avr::io;
using namespace ssd1306;

/** This demos is a "hello world" that setups a display with 128x64
    dots with some basic commands and after that erases the content of
    the whole screen to print the string 'hi'.
*/

static const uint8_t cmds[] [[gnu::__progmem__]] = {
    /** Commands to inform the driver what is the physical
        configuration of the display. Note that your display can be
        different, take a look at the secton 10.1.18 of the datasheet
        with the result on the screen is weird. 
    */
    0xC8, /** COM Output Scan Direction*/ 
    0xA1, /** Segment Re-map */
    
    0x20, 0, /** Horizontal Addressing Mode*/ 
    0x22, 0, 7, /** Set page address: 0 to 7*/  
    0x21, 0, 127, /** Set column address: 0 to 127*/
    
    0x8D, 0x14, /** Enable Charge Pump*/
    
    0xAF, /** Turn on the display */
};

constexpr uint8_t transform(uint8_t col, uint8_t b, uint8_t pos) {
    if(pos > col) b >>= pos - col;
    else if (pos < col) b <<= col - pos;
    b &= 1<<(7 - pos);
    return b;
}

struct bitmap_90_clockwise { uint8_t data[8]; };

/** Transform a bitmap to a sequence of bytes to be sent to the
    GDDRAM. Basically the bitmap is rotated 90 degress clockwise.
 */
constexpr bitmap_90_clockwise toGDDRAM(const uint8_t(&bitmap)[8]) {
    bitmap_90_clockwise ret{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    for(auto col{0}; col < 8; ++col)
        for(auto row{0}; row < 8; ++row) 
            ret.data[col] |= transform(col, bitmap[row], 7 - row);
    return ret;
}
    
constexpr static const uint8_t bitmap_h[] = {
    0b10000000, 
    0b10000000,
    0b10111000,
    0b11000100,
    0b10000100,
    0b10000100,
    0b10000100,
    0b10000100,
};

constexpr static const uint8_t bitmap_i[] = {
    0b00010000,
    0b00000000,
    0b00110000,
    0b00010000,
    0b00010000,
    0b00010000,
    0b00010000,
    0b00011000,
};

static const bitmap_90_clockwise letter_h [[gnu::__progmem__]] = toGDDRAM(bitmap_h);
/** result of toGDDRAM(bitmap_h)
    0b11111111,
    0b00001000,
    0b00000100,
    0b00000100,
    0b11111000,
    0b00000000,
    0b00000000,

    the bitmap of the letter 'h' was rotated at 
    90 degress clockwise.
 */

static const bitmap_90_clockwise letter_i [[gnu::__progmem__]] = toGDDRAM(bitmap_i);
/** result of toGDDRAM(bitmap_i)
    0b00000000,
    0b00000000,
    0b00000100,
    0b11111101,
    0b10000000,
    0b00000000,
    0b00000000,
    0b00000000,

    the bitmap of the letter 'i' was rotated at 
    90 degress clockwise.
 */

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

    /** print 'hi' at page 0 and column 0 */
    dev.start_data();
    
    //send letter 'h'
    for(uint8_t i{0}; i < 8; ++i)
        dev.send_byte(pgm_read_byte(&letter_h.data[i]));

    //send letter 'i'
    for(uint8_t i{0}; i < 8; ++i)
        dev.send_byte(pgm_read_byte(&letter_i.data[i]));

    dev.stop_condition();
    
    while(true);
}
