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

static const uint8_t letter_h[] [[gnu::__progmem__]] = {
    /**
       Bitmap of the letter 'h':

       0b10000000, LSB
       0b10000000,
       0b10111000,
       0b11000100,
       0b10000100,
       0b10000100,
       0b10000100,
       0b10000100, MSB

       Each byte below represents one column from left to right. The
       LSB is on the top and the MSB in on the bottom.
    */
    0xff, 0x08, 0x04, 0x04, 0x04, 0xf8, 0x00, 0x00
};

static const uint8_t letter_i[] [[gnu::__progmem__]] = {
    /**
       Bitmap of the letter 'i':

       0b00010000, LSB
       0b00000000,
       0b00110000,
       0b00010000,
       0b00010000,
       0b00010000,
       0b00010000,
       0b00011000, MSB
        
       Each byte below represents one column from left to right. The
       LSB is on the top and the MSB in on the bottom.
    */
    0x00, 0x00, 0x04, 0xfd, 0x80, 0x00, 0x00, 0x00
};

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
        dev.send_byte(pgm_read_byte(&letter_h[i]));
    
    //send letter 'i'
    for(uint8_t i{0}; i < 8; ++i)
        dev.send_byte(pgm_read_byte(&letter_i[i]));
    
    dev.stop_condition();
    
    while(true);
}
