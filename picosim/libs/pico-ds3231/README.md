# pico-ds3231

A C library for using the DS3231 real-time clock with the Raspberry Pi
Pico.

The DS3231 ([product website][analog]) is a real-time clock (RTC) with
integrated crystal oscillator. It also features two 2 alarms and a
battery-backup input.

There are breakout boards that use this RTC, notably Adafruit's DS3231
Precision RTC breakout board ([cat.num. 3013][ada3013]).

[analog]: https://www.analog.com/en/products/ds3231.html
[ada3013]: https://www.adafruit.com/product/3013


## Pin description

| DS3231 Pin | Description
|------------|------------
| 32 kHz     | Oscillator output. Leave unconnected if not used
| Vin        | Power supply, requires decoupling capacitor (0.1 to 1 uF)
| ~INT/SQW   | Square wave or interrupt output. Leave unconnected if not used
| ~RST       | Active-low reset
| GND        | Ground
| VBAT       | Backup power supply input. If not used connect to ground
| SDA        | Serial data input/output
| SCL        | Serial data clock


## Wiring

The DS3231 RTC operates with an I2C interface. Thus, all it requires is
connections to SDA, SCL, Vin, and GND pins in the Pico, e.g. 

| RTC | Pico
|-----|---------
| Vin | 3V3(OUT)
| SDA | GP4
| SCL | GP5
| GDN | GND


## Using the library

See examples.

### To do

These are additional functions that are available in the DS3231 but have
not yet been implemented in this library:

* Alarms/alarm interrupts
* Enabling/setting freq of square-wave output
 