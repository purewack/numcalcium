# Numcalcium *[Archive]*

## ***Note - Software Archive for A0 board***

## Electrical Engineer's Friend :)
USB Calculator and numpad with build in tools to help you create and debug circuits easier.

# Hardware 
The board contains an <u>STM32F103C8</u> 64k program space and 20k SRAM.
An included battery charge circuit allows the unit to operate in a standalone manner.
The boards hosts a Monochrome 128x64 LCD, Rotary Encoder, 20 Mechanical switches, USB port, and IO pins from the microcontroller.
### Warnings
<b>The exposed pins are unprotected and are wired directly to the chip, take care with the voltages you apply to them and do not exceed maximum current capabilities of the chip</b>

## Dependencies
Using a modified STM32 library for the arduino core, one is able to build and flash the unit with the latest build,
<br>
<s>or use the latest prebuilt binary file and upload utility</s> <i><b>- coming soon</b></i>

### Submodules
<i>numcalcium-base</i> - Base drivers for developement : [https://github.com/purewack/numcalcium-base]<br>
<i>libIntDSP</i> - Audio library for integer DSP : [https://github.com/purewack/libintdsp]<br>
<i>libDArray</i> - lightweight dynamic (and static) arrays library : [https://github.com/purewack/libdarray]<br>

### Drivers
This software uses <u><a href="https://github.com/purewack/numcalcium-base/blob/docs/API.md">numcalcium-base</a></u> to ease the interfacing of the exposed peripherals like buttons or LCD.
This is a minimal implementation package and contains only functions which were required to finish this project.

# Programs

### Keyboard

### Calculator

### MIDI

### Comms

### Signals
### GPIO

### Scope