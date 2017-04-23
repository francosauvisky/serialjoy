PGR = main
MCU_TARGET = atmega8

lfuse = E4
hfuse = D9

OPTIMIZE = Os

MODULES = genesis.o genesis2.o uart_helper.o

all: $(PGR).hex $(PGR)_eeprom.hex $(PGR).map

full: all prog pon

$(PGR).hex: $(PGR).elf
	avr-objcopy -j .text -j .data -O ihex $(PGR).elf $(PGR).hex

$(PGR)_eeprom.hex: $(PGR).elf
	avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex $(PGR).elf $(PGR)_eeprom.hex

#$(PGR).lst: $(PGR).elf
#	avr-objdump -h -S $(PGR).elf > $(PGR).lst

$(PGR).elf $(PGR).map: $(PGR).o $(MODULES)
	avr-gcc -g -mmcu=$(MCU_TARGET) -Wl,-Map,$(PGR).map -o $(PGR).elf $(PGR).o $(MODULES)

$(PGR).o: $(PGR).c
	avr-gcc -g -$(OPTIMIZE) -mmcu=$(MCU_TARGET) -c $(PGR).c

$(MODULES): %.o: %.c
	avr-gcc -g -$(OPTIMIZE) -mmcu=$(MCU_TARGET) -c $<

.PHONY: prog_lfuse prog_hfuse pon poff clean tar

prog: $(PGR).hex
	avrdude -c pickit2 -p $(MCU_TARGET) -e -U flash:w:'$(PGR).hex':a

prog_eeprom: $(PGR)_eeprom.hex
	avrdude -c pickit2 -p $(MCU_TARGET) -e -U eeprom:w:'$(PGR)_eeprom.hex':a

prog_lfuse:
	echo -e -n '\x$(lfuse)' > lfuse.txt
	avrdude -c pickit2 -p $(MCU_TARGET) -e -U lfuse:w:lfuse.txt
	rm lfuse.txt

prog_hfuse:
	echo -e -n '\x$(hfuse)' > huse.txt
	avrdude -c pickit2 -p $(MCU_TARGET) -e -U hfuse:w:hfuse.txt
	rm hfuse.txt

pon:
	pk2cmd -P PIC16F630 -T -R

poff:
	pk2cmd -P PIC16F630

clean: 
	rm *.elf *.hex *.map *.o #.lst

tar: $(wildcard *.c) $(wildcard *.h) Makefile
	tar --create --file $(PGR).tar.gz --gzip $^
