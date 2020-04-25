# curing-station
Arduino based UV Curing Station control software

## Background
When 3D printing with UV curable resin the final curing is an important part of the post-printing process, since the UV curing during the printing will only cure the layers enough to make them go into a solid state.

This project is based on a FDM 3D printed frame and some electronics that controls a DC motor for a turntable and some UV LEDs that will do the curing. 
The UI consist of a 1.3" OLED display with 128x64 pixels, using the SH1106 controller, an encoderwheel and two buttons. There is also a beeper for acoustic notification and a hall-effect switch for detecting when the cover is open.

## The design
The mecahnical- and electronics design can be found on Thingiverse:
https://www.thingiverse.com/thing:4295290

I used Atom editor with the PlatformIO plugin for development
https://atom.io/

The following libraries where used:
U8g2

Target:
Arduino Pro Mini ATmega328 (5V, 16 MHz)
