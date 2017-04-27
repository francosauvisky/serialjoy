# Serialjoy

Note that this program is still on an early stage, there is a **lot** more to do. The only supported gamepad is the SEGA Genesis controller, currently (although it's simple to adapt to other controllers).

## What is this?

Serialjoy is a generic and hobbist-friendly adapter of joysticks/gamepads on Linux using serial ports. That means you can program any sort of microcontroller or similar devices, for example an Arduino, to interface with a game controller (even one that you made! That's why it's generic), then send the data via a serial port and they will be able to emulate joystick buttons, keyboard keys, analog axes & more on your computer. This way you can cut a lot of time and complexity from using the USB protocol for such devices (that's why it's hobbist-friendly), and it's quite more flexible.

## How it works?

This project is composed of two main parts that interact: an *adapter* and a *device*:

The *adapter* is the physical layer that interfaces with a game controller and sends the data to a serial port, which can be an USB-Serial adapter or a real RS232 port (with the voltage levels corrected).

The *device* is the software that your computer will be running when the *adapter* is connected. It will translate the data received on the serial port to a virtual joystick using `uinput`, allowing you to generate real input events with your physical controller.

## Contributing

Feel free to contribute in any way you can. The project is still young, so new ideas are welcome (constructive criticism is also welcome). Writing documentation is a must, as I haven't got the time to do that yet. If you want to become an active developer or have some other question, please contact me at my email: [francosauvisky+serialjoy@gmail.com](mailto:francosauvisky+serialjoy@gmail.com).

Another way to contribute to this project is to donate/lend gamepads so I can program the adapter for them. If you live nearby (Florianópolis, Santa Catarina, Brazil), I can return them afterwards to you. Otherwise, if you can write some code, be welcome to adapt them by yourself (and don't forget to share the code afterwards!).

## To do

### General

- Create a wiki/add documentation (IMPORTANT!)

### Device

- Add multiple controllers/devices with a single serial port [Done!]
- Use a simpler received data <-> input action dictionary, not a switch statement within a function within a *.c file (maybe with #define or an external configuration file)
- Use commands from the adapter to control the device (for example, to add another controller, or to close the device) [Done!]
- Add analog axes and more buttons compatibility (using data packets)
(the last 3 items can be summarized in: use a better data transmission protocol)
- Use argp or getopt to read the command-line arguments
- Automatic identification of the serial port
- Automatic service (daemon) which runs the program when an adapter is detected and starts at boot/user login.

### Adapter

- Add a second controller [Done!]
- Simplify the controller drivers (de-hardwire them) [Done!]
- Automatic identification of the controller (s)
- Draw schematics and PCBs.
- Better control of the device: initializing and closing controllers, etc [Partially done!]
- More generic code: using structs for the status, analog axis, etc
- (far) Support for more controller types (NES, SNES, PlayStation, Xbox, etc)
