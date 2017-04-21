# Serialjoy

Note that this program is still on an early stage, there is a **lot** more to do. The only supported gamepad is the SEGA Genesis controller, currently (although it's simple to adapt to other controllers).

## What is this?

Serialjoy is a generic and hobbist-friendly adapter of joysticks/gamepads on Linux using serial ports. That means you can program any sort of microcontroller or similar devices, for example an Arduino, to interface with a game controller (even one that you made! That's why it's generic), then send the data via a serial port and they will be able to emulate joystick buttons, keyboard keys, analog axes & more on your computer. This way you can cut a lot of time and complexity from using the USB protocol for such devices (that's why it's hobbist-friendly), and it's quite more flexible.

## How it works?

This project is composed of two main parts that interact: an *adapter* and a *device*:

The *adapter* is the physical layer that interfaces with a game controller and sends the data to a serial port, which can be an USB-Serial adapter or a real RS232 port (with the voltage levels corrected).

The *device* is the software that your computer will be running when the *adapter* is connected. It will translate the data received on the serial port to a virtual joystick using `uinput`, allowing you to generate real input events with your physical controller.

## To do

- Todo list