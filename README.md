This is a fork of https://bitbucket.org/Mongo/canfestival-3-asc which is again a fork of the original CanFestival-3 project http://dev.automforge.net/CanFestival-3

This is CanFestival-3 + the best fixes from canfestival-3-asc + optimizations for small memory devices.

The major improvement this fork brings to the table is the ability to place static data in flash instead of RAM to reduce memory consumption on devices with limited RAM (example PIC or AVR microcontrollers). As an example, the memory consumption of the AVR/Slave example is reduced by ~1KB.

Highlights of changes in this fork:
- Fixed compile warnings in AVR example
- Fixed MSG_WAR/MSG_ERROR debug output for AVR
- Improved example buildscripts to automatically rebuild object dictionaries when gen_cfile.py or objdictgen.py changes.
- Allow static dictionary index tables to be stored in flash instead of RAM to save RAM on small systems with limited RAM (for example AVR). 
- Allow constant data in object dictionary to be stored in flash instead of RAM to save RAM on small systems with limited RAM (for example AVR).
- Hide internal structure definitions from public API

Comments/feedback:
github < a t > ljessen < d o t > dk

