# Serialjoy

Note that this program is still on an early stage, there is a **lot** more to do. The only supported gamepad by the adapter is the SEGA Genesis controller (although it's simple to adapt to other/new controllers).

## What is this?

Serialjoy is a generic and hobbist-friendly adapter of joysticks/gamepads on Linux using serial ports. That means you can program any sort of microcontroller or similar devices, for example an Arduino, to interface with a real game controller (even one that you made! That's why it's generic), then send the data via a simple serial port and they will be able to emulate joystick buttons, keyboard keys, analog axes & more on your computer. This way you can cut a lot of time and complexity from using the USB protocol for such devices (that's why it's hobbist-friendly), besides being more flexible.

## How it works?

This project is composed of two main parts that interact: an *adapter* and a *device*:

The *adapter* is the physical layer that interfaces with a game controller and sends the data to a serial port, which can be an USB-Serial adapter or a real RS232 port (with the voltage levels corrected).

The *device* is the software that your computer will be running when the *adapter* is connected. It will quickly translate the data received on the serial port to a virtual joystick using `uinput`, allowing you to generate real input events with your physical controller with almost zero delay.

PS: Although the project is composed of two parts, the name `serialjoy` will always allude to the device software.

### Some implementation details

The device talks to the adapter via a serial port (like `/dev/ttyUSB0` if you're using an USB-RS232 adapter) using only printable characters that represent the state of a button (uppercase is pressed, lowercase is released) or some other data (analog axes, create controller, etc). More details on the communication protocol is described below and is subject to changes.

## Contributing

Feel free to contribute in any way you can. The project is still young, so new ideas are welcome (constructive criticism is also welcome). Writing documentation is a must, as I haven't got the time to do that yet. If you want to become an active developer or have some other question, please contact me at my email: [francosauvisky+serialjoy@gmail.com](mailto:francosauvisky+serialjoy@gmail.com).

Another way to contribute to this project is to donate/lend gamepads so I can program the adapter for them. If you live nearby (Florianópolis, Santa Catarina, Brazil), I can return them afterwards to you. Otherwise, if you can write some code, be welcome to adapt them by yourself (and don't forget to share the code afterwards!).

## Communication Protocol

- Valid characters: Every printable character (from 0x20 up to 0x7E)
- Packets size: Sequences of 1 to 5 chars/bytes.

### Actions

- Simple actions "a":  
a = [a..z]: Buttons presses (up to 26 buttons)  
a = [A..Z]: Buttons relase (up to 26 buttons)

- Complex actions "axx":  
a = [a..zA..Z]: Action code (defined afterwards), for example analog axis  
x = [0x40..0x5F]: Action data (less significant 5 bits of each char) = 10 bits

### Packet formats

- From adapter to device:
1. "!n", where n = [0..9]: Create device n
2. "^n", where n = [0..9]: Destroy device n
3. "a", where a = [a..zA..Z]: Send simple action a to device 0 (LEGACY)
4. "na", where n = [0..9] and a = [a..zA..Z]: Send simple action a to device n
5. "n:axx" where n = [0..9], a = [a..zA..Z] and each x = [0x40..0x5F]: Send complex action axx to device n
6. "s" where s is a null-terminated string: Preprogrammed string (**only** if device asks)

- From device to adapter:
1. "d": Force adapter to (re)create devices
2. "v": Return preprogrammed string

- Handshake protocol
1. "#": OK
2. "%": Not OK
3. "?": Are you there? (return OK)

When adapter sends "!n", device must answer OK when sucessful.
If adapter doesn't responds to "?", then go to legacy mode

## To do

### General

- Create a wiki/add documentation (IMPORTANT!)

### Device

- Use a simpler received data <-> input action dictionary, not a switch statement within a function within a *.c file (maybe with #define or an external configuration file)
- Add analog axes and more buttons compatibility (using data packets)
(the last 3 items can be summarized in: implement better data transmission protocol)
- Use argp or getopt to read the command-line arguments
- Automatic identification of the serial port
- Automatic service (daemon) which runs the program when an adapter is detected and starts at boot/user login.

### Adapter

- Separate adapter from this repository (and rewrite it)
- Simplify the controller drivers (de-hardwire them) [Done!(?)]
- Automatic identification of the controller (s)
- Draw schematics and PCBs.
- Better control of the device: initializing and closing controllers, etc [Partially done!]
- More generic code: using structs for the status, analog axis, etc
- (far) Support for more controller types (NES, SNES, PlayStation, Xbox, etc)
