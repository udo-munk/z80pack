/*
 * Copyright (c) 2023 Antonio González
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "ds3231.h"


/* Set the time on the DS3231.

This helps set the time in the DS3231 real-time clock. If the device has
a battery backup then it should only be necessary to do this once.

The steps involved are:

(1) After connecting the clock tot the Pico, update the relevant I2C
definitions below.

(2) Enter date and time values below, see the ds3231_datetime_t
structure. Set a time a few minutes in the future.

(3) Compile this code and upload to the Pico.

(4) Open a terminal (e.g. `picocom /dev/ttyACM0`)

(5) Look at a watch or clock: as soon as the time matches the one
entered below, press `enter`. At that point the date/time will be
uploaded to the DS3231 and this will start ticking.
*/

#define DS3231_I2C_PORT i2c1
#define DS3231_I2C_SDA_PIN 2
#define DS3231_I2C_SCL_PIN 3

#define ASCII_ENTER 0x0D


int main() {
    stdio_init_all();

    struct ds3231_rtc rtc;

    ds3231_init(DS3231_I2C_PORT, DS3231_I2C_SDA_PIN,
                DS3231_I2C_SCL_PIN, &rtc);
    
    // Set a date and time here to a few minutes the future.
    ds3231_datetime_t dt = {
        .year = 2023, // Year (2000-2099)
        .month = 12, // Month (1-12, 1=January)
        .day = 11, // Day (1-31)
        .hour = 16, // Hour, 24-h format (0-23)
        .minutes = 03, // Minutes (0-59)
        .seconds = 00, // Seconds (0-59)
        .dotw = 1 // Day of the week (1-7, 1=Monday)
    };

    char c;    
    uint8_t dt_str[25];
    
    // Wait for a few seconds for the user to open a terminal, then
    // wait for the user to press enter.
    sleep_ms(3000);
    ds3231_ctime(dt_str, sizeof(dt_str), &dt);
    printf("Press enter at %s\n");
    while(1) {
        c = getchar();
        if (c == ASCII_ENTER) {
            // Set the date and time as soon as enter is pressed.
            ds3231_set_datetime(&dt, &rtc);
            puts("Time set.");
            break;
        }
    }

    while (1) {
        ds3231_get_datetime(&dt, &rtc);
        ds3231_ctime(dt_str, sizeof(dt_str), &dt);
        puts(dt_str);
        sleep_ms(10000);
    }
    return 0;
}