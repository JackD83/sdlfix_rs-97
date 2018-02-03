                                                    sdlfix_rs-97
                                                    ------------


sdlfix_rs-97 is a small library that is designed to fix the resolution of sdl-based applications on 
the RS97 handheld gaming device. 
The RS97 has the odd resolution of 320x480 px while it is a 4:3 screen. This library just doubles the lines of a 320x240 rendered input, which 
makes the visual impression correct.

Usage
-----

The library is used with the LD_PRELOAD command.

If, for example you executable is 

DinguxCommander.dge

write a new text file with "dge" or "sh" extension

with the following content:

#!/bin/sh
LD_PRELOAD=./sdlfix.so ./DinguxCommander_.dge

Make sure that you use an editor capable of linux type new lines (I prefere Visual Studio Code). You need "LF" in the bottomn right, not "CRLF"

There are two requirements:
- The application has to use SDL for drawing to the screen. Most of them do, some don't
- The application has to be running in the first place. If its running and has distorted screen, give this fix a try

Source of the sdlfix is here: https://github.com/JackD83/sdlfix_rs-97