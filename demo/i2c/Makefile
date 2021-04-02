mcu=attiny13a
dev=t13
std=c++17
avr_io_inc=../../../avrIO/include

all: hi.elf send_command_low_level.elf send_data_to_ram_low_level.elf 

%.elf: %.cpp
	avr-g++ -Os -std=$(std) -mmcu=$(mcu) -o $@ $< \
	-I../../include -I$(avr_io_inc)
	avr-size $@

%.s: %.elf
	avr-objdump -d $< > $@
	cat $@

.PHONY: clean flash-%

flash-%: %.elf
	avrdude -p $(dev) -c usbasp -P usb  -U flash:w:$<

clean:
	rm -f *.elf *.s
