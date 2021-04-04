#include <avr/io.hpp>
#include <avr/pgmspace.h>
#include <ssd1306.hpp>

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
    
    0x20, 0, /** Horizontal Addressing Mode*/ 
    0x22, 0, 7, /** Set page address: 0 to 7*/  
    0x21, 0, 127, /** Set column address: 0 to 127*/
    
    0x8D, 0x14, /** Enable Charge Pump*/
    
    0xAF, /** Turn on the display */
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

    /** send a square of 8x8 pixels to the screen */
    dev.start_data();
    for(uint8_t i{0}; i < 8; ++i)
        dev.send_byte(0xff);
    dev.stop_condition();
    
    while(true);
}
