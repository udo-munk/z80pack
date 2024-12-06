/*
 * Copyright (c) 2023 Antonio González
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "ds3231.h"

/*
This example illustrates the basic functionality of this library, namely
setting the date and time and then reading these back. See comments
below for more details.
*/

// Modify these definitions as required, to match connections.
#define DS3231_I2C_PORT i2c1
#define DS3231_I2C_SDA_PIN 2
#define DS3231_I2C_SCL_PIN 3

int main() {
    stdio_init_all();

    // Create a real-time clock structure and initiate this.
    struct ds3231_rtc rtc;
    ds3231_init(DS3231_I2C_PORT, DS3231_I2C_SDA_PIN,
                DS3231_I2C_SCL_PIN, &rtc);
    
    // A `ds3231_datetime_t` structure holds date and time values. It is
    // used first to set the initial (user-defined) date/time. After
    // these initial values are set, then this same structure will be
    // updated by the ds3231 functions as needed with the most current
    // time and date values.
    ds3231_datetime_t dt = {
        .year = 2023, // Year (2000-2099)
        .month = 12, // Month (1-12, 1=January, 12=December) 
        .day = 12, // Day (1-31)
        .hour = 14, // Hour, in 24-h format (0-23)
        .minutes = 30, // Minutes (0-59)
        .seconds = 00, // Seconds (0-59)
        .dotw = 2,  // Day of the week (1-7, 1=Monday, 7=Sunday)
    };

    // This will use the above information to set the date and time.
    ds3231_set_datetime(&dt, &rtc);

    // This is a character array that will be used for storing a string
    // representation of the date and time.
    uint8_t dt_str[25];
  
    while (1) {
        // Read the date and time from the DS3231 RTC.
        ds3231_get_datetime(&dt, &rtc);

        // Convert the dt structure to a string and print this.
        ds3231_ctime(dt_str, sizeof(dt_str), &dt);
        puts(dt_str);

        // These related functions are also available; they similarly
        // take a character array and convert time and/or date to
        // string:
        //
        // ds3231_isoformat(dt_str, sizeof(dt_str), &dt);
        // ds3231_str_date(dt_str, sizeof(dt_str), &dt);
        // ds3231_str_time(dt_str, sizeof(dt_str), &dt);
        
        // Sleep for a few seconds.
        sleep_ms(10000);
    }
    return 0;
}
