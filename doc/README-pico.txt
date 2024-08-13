Let's have a brief look at the hardware, I've setup the dev prototyp
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

picoboard3:
And here the MicroSD drive is wired and I can read/write files on the card.
I have connected four of the GND pins of the Pico board with the GND lines
on the breadbaord. The card drives can draw 100 mA, so better provide
sufficient GND. Also please note the two blocking capacitors, these are
to keep radio emissions low.

picoboard4:
I removed the monochrome LED and instead added a breakout board with a
WS2811 RGB LED. With this we can show stati of the machine by using
different colors, show access to the disk drives, or blink it from
the emulated CPU's. These RGB LED's need 5V power, which is available
at pin 40 comming from the USB connection. The data in pin is connected
to GPIO 14 and the 3.3V logic level from the GPIO is fine to program the
LED, a logic level shifter is not needed.

How to build the application for this board is explained in picosim/README.
