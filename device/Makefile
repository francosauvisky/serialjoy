all: serialjoy

gamepad_events.o: gamepad_events.c defs.h
	gcc -c gamepad_events.c

simple_serial.o: simple_serial.c defs.h
	gcc -c simple_serial.c 

simple_uinput.o: simple_uinput.c defs.h gamepad_events.o
	gcc -c simple_uinput.c 

serialjoy: main.c gamepad_events.o simple_serial.o simple_uinput.o defs.h
	gcc main.c -o serialjoy gamepad_events.o simple_serial.o simple_uinput.o

clean: 
	mkdir -p .trash
	mv -fb *.o ./.trash

tar: $(wildcard *.c) $(wildcard *.h) Makefile
	tar --create --file serialjoy.tar.gz --gzip $^