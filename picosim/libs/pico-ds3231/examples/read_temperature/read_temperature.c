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
This example shows how to read the temperature register in the DS3231
*/

// Modify these definitions as required, to match your wiring
#define DS3231_I2C_PORT i2c1
#define DS3231_I2C_SDA_PIN 2
#define DS3231_I2C_SCL_PIN 3

int main() {
    stdio_init_all();

    // Variable for the temperature value.
    float temperature;

    // Create a real-time clock structure and initiate this.
    struct ds3231_rtc rtc;
    ds3231_init(DS3231_I2C_PORT, DS3231_I2C_SDA_PIN,
                DS3231_I2C_SCL_PIN, &rtc);

    // A datetime structure holds date and time values.
    ds3231_datetime_t dt;

    // This is a character array that will be used for storing a string
    // representation of the date and time.
    uint8_t dt_str[25];

    // Sleep for a few seconds while the user opens the terminal.
    sleep_ms(3000);
  
    while (1) {
        // Read the date and time from the DS3231 RTC.
        ds3231_get_datetime(&dt, &rtc);

        // Convert the dt structure to a string and print this.
        ds3231_ctime(dt_str, sizeof(dt_str), &dt);
        puts(dt_str);

        // Read the temperature register and print
        ds3231_get_temperature(&temperature, &rtc);
        printf("Temperature: %0.2f C\n", temperature);
        
        // Sleep for a while. Note that the temperature register is only
        // updated every 64 seconds. Thus, reading this value more often
        // than this does not make much sense.
        sleep_ms(65000);
    }
    return 0;
}
