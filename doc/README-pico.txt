Let's first have a brief look at the hardware, I've setup the dev prototyp
on breadboard, which is a good thing as you will see in a moment ;-)

picoboard1:
Shows my initial setup, just a Raspberry Pi Pico WH on a breadboard, a simple
switch with 10 K pull down resistor that causes an interrupt and stops
the emulated CPU. There is one additional LED which you can ignore, I use
it to simulate a Pico to see, if I got the configurations for the different
boards correct. The UART lines are connected to a Raspberry debug probe,
I got that for the case that I really have to poke arround in the thing
with gdb. Not needed so far, should be fine to use a simple TTL to USB
adapter.

picoboard2:
Here you see that I got an Adafruit MicroSD drive already in place, which
is not wired yet. This is because I noticed that I used one of the SPI
pins of the primary SPI interface for the break switch. Read the datasheet
more carefully next time, but thanks to breadboard this is easy to fix, I
connected the break switch to a different port.
The MicroSD drive is a 3.3V one and it supports SPI and SDIO, I'll first
use SPI, that should be fast enough for what I want.
