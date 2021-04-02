#pragma once

#include "ssd1306/detail/global.hpp"

#include <avr/io.hpp>

namespace ssd1306 {

namespace sa0 {
struct on_t{ static constexpr uint8_t bv{1<<1}; };
SSD1306_INLINE_GLOBAL(on)
struct off_t{ static constexpr uint8_t bv{0x00}; };
SSD1306_INLINE_GLOBAL(off)
}//namespace sa0

enum class dc { data, command };
enum class co { on, off };

/** I2C communication interface 

    Sda: pin that represents the bus data signal SDA.
    Scl: pin that represents the bus clock signal SCL.
    SA0: slave address bit. The default value is zero(off).

    This abstraction follows the specification from the section '8.1.5
    MCU I2C Interface' of datasheet.

    There are five low level methods that allows any operation on the
    device: start_condition(), send_slave_addr(), send_ctrl_byte(),
    send_byte() and stop_condition().
*/
template<typename Sda, typename Scl, typename SA0 = sa0::off_t>
struct i2c {
    static constexpr uint8_t addr() { return 0b01111000 | SA0::bv; }
    
    i2c() = default;
    
    explicit i2c(Sda sda, Scl scl, SA0 = SA0{})
    { avr::io::out(sda, scl); }

    /** The first operation to send a command or a data byte to GDDRAM
        is to initiate the communication by a start condition. The
        start condition is established by pulling the SDA from HIGH to
        LOW while the SCL stays HIGH.

        precondition: SDA and SCL are high by pull-up resistors. 
    */
    static void start_condition() { Sda::low(); }

    /** Send the slave address of the device. 

        The method sents a byte 0b01111000 or 0b01111010. The bit 1
        from left to right is the SA0, and the bit 0 has always the
        value 0 to indicate that the write mode is being used.

        precondition: a start condition should be sent before this
        operation.
    */
    static void send_slave_addr() { send_byte(addr()); }

    /** Send a control byte.

        The control byte informs if a command or a data to the RAM
        will be sent. The control byte has the following form:
        
        |co|dc|0|0|0|0|0|0|

        'co' means 'Continuation Bit' and it informs if the next bytes
        to be sent are only data bytes. The value 0 means that all the
        next bytes are data bytes and the value 1 indicates that pairs
        of (control_byte, data) are will be sent. The last option
        allows commands and data to GDDRAM to be sent inside one pair
        of start and stop condition, for example, the following is
        possible with 'co == 1':
        
        start_condition();
        send_slave_addr();
        send_ctrl_byte(dc::command, co::on);
        send_byte(0x22);
        send_ctrl_byte(dc::command, co::on);
        send_byte(4);
        send_ctrl_byte(dc::command, co::on);
        send_byte(7);
        send_ctrl_byte(dc::data, co::on);
        send_byte(0xff);
        stop_condition();

        Note that the code above is sending one command 0x22 with two
        arguments and after that a data 0xff to RAM using only one
        {start,stop}condition pair. It's important to keep in mind
        that this approach must considered with caution because one
        extra byte(the control byte) is sent to each command or data
        byte.

        'dc' is 0 when a command should be sent and 1 when a data to
        GDDRAM should be sent.

        precondition: the slave address should be sent before this operation.
    */
    static void send_ctrl_byte(dc mode, co co_bit = co::off) {
        uint8_t byte{0x00};
        if(co_bit == co::on) byte |= (1<<7);
        if(mode == dc::data) byte |= (1<<6);
        send_byte(byte);
    }
    
    /** Send one byte.

        The byte can represent a command, an argument to a command or
        a data to be sent to RAM.

        If it is a data to RAM then each bit represents a dot(or a
        pixel) and the MSB is on the bottom of the column and the LSB
        on the top.

              SEGY(ColumnY)
        PAGEX |1| LSB
              |1|
              |1|
              |1|
              |0|
              |0|
              |0|
              |0| MSB

        The page number(X) ou the column number(Y) should be defined
        through one or more commands.

        precondition: a control byte should be sent before this call. 
    */
    static void send_byte(uint8_t byte) {
        Scl::low();
        for(uint8_t i{8}; i > 0; --i) {
            Sda::low();
            if(byte & 0x80) Sda::high();
            byte <<= 1;
            Scl::pulse();
        }
        //acknowledge bit
        Scl::pulse();
    }
    
    /** The operation should be finished by a stop condition. The stop
        condition is established by pulling the SDA from low to high
        while the SCL stays high.
    */
    static void stop_condition() {
        //The stop condition is established by pulling the SDA from low to
        //high while the SCL stays high.
        Scl::high();
        Sda::high();
    }

    /** Inform the device that a sequence of commands will be sent. */
    static void start_commands() {
        start_condition();
        send_slave_addr();
        send_ctrl_byte(dc::command);
    }

    /** RAII to start_commands/stop_condition */
    struct scoped_commands_t {
        scoped_commands_t() { i2c::start_commands(); }
        ~scoped_commands_t() { i2c::stop_condition(); }
    };    
    static scoped_commands_t scoped_commands() { return{}; }
    
    /** Inform the device that a sequence of data will be sent. */
    static void start_data() {
        start_condition();
        send_slave_addr();
        send_ctrl_byte(dc::data);
    }
    
    /** RAII to start_data/stop_condition */
    struct scoped_data_t {
        scoped_data_t() { i2c::start_data(); }
        ~scoped_data_t() { i2c::stop_condition(); }
    };    
    static scoped_data_t scoped_data() { return{}; }
};

}
