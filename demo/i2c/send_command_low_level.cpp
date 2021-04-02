#include <avr/io.hpp>
#include <ssd1306.hpp>

using namespace avr::io;
using namespace ssd1306;

/** This is an educational demo to literally demonstrate what is going
    on when only one command is sent to the display. */
int main() {
    /** Initializes an object that represents the I2C interface with
        an OLED SSD1306 using the pin PB0 as SDA and pin PB2 as
        SCL. The device has an address defined by the bit SA0, the
        default value to this bit is 0(off). Take a look at
        'ssd1306/i2c.hpp' to know more about how to initilize
        'ssd1306::i2c'.
     */
    ssd1306::i2c dev{pb0, pb2};

    /** The first operation to send a command is to initiate the
        communication by a start condition. The start condition is
        established by pulling the SDA from HIGH to LOW while the SCL
        stays HIGH.
    */
    dev.start_condition();

    /** After that the slave address of the device should be sent. The
        method sents a byte 0b01111000 or 0b01111010. The bit 1 from
        left to right is the SA0, and the bit 0 is always 0 to
        indicate that the write mode is being used. 
    */
    dev.send_slave_addr();

    /** Before sending the command it's necessary to inform that a
        command will be sent, this is done by a control byte, that
        informs if the next byte to be sent is a command or data to
        be sent to the GDDRAM. The control byte has the following form:
        
        |co|dc|0|0|0|0|0|0|

        'co' means 'Continuation Bit' and it informs if the next bytes
        to be sent are only data bytes. The value 0 means that all the
        next bytes are data bytes and the value 1 indicates that pairs
        of (control_byte, data) are will be sent. The last option
        allows commands and data to GDDRAM to be sent inside one pair
        of start and stop condition.

        'dc' is 0 when a command should be sent and 1 when a data to
        GDDRAM should be sent.
    */
    dev.send_ctrl_byte(dc::command);
    
    /** Finally the command can be sent. Each command is one byte and
        there may be one or more argumnts(bytes). The command to turn
        on the display(0xAF) doesn't have any argument. 
    */
    dev.send_byte(0xaf);

    /** The operation should be finished by a stop condition. The stop
        condition is established by pulling the SDA from low to high
        while the SCL stays high.
    */
    dev.stop_condition();
}
