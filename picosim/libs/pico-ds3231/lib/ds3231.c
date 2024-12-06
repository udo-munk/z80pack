/*
 * Copyright (c) 2023 Antonio González
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ds3231.h"
 

void ds3231_init(i2c_inst_t *i2c_port, uint8_t i2c_sda_pin,
                 uint8_t i2c_scl_pin, ds3231_rtc_t *rtc) {
    rtc->i2c_port = i2c_port;
    rtc->i2c_addr = DS3231_I2C_ADDRESS;
    rtc->i2c_sda_pin = i2c_sda_pin;
    rtc->i2c_scl_pin = i2c_scl_pin;

    i2c_init(i2c_port, DS3231_I2C_FREQ);
    gpio_set_function(i2c_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(i2c_scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(i2c_sda_pin);
    gpio_pull_up(i2c_scl_pin);
};

void ds3231_set_datetime(ds3231_datetime_t *dt, ds3231_rtc_t *rtc) {
    //uint8_t val;
    uint8_t low, high;
    uint8_t buffer[8];
    buffer[0] = DS3231_DATETIME_REG;

    /* Seconds */
    low = dt->seconds % 10;
    high = (dt->seconds - low) / 10;
    buffer[1] = (high << 4) | low;

    /* Minutes */
    low = dt->minutes % 10;
    high = (dt->minutes - low) / 10;
    buffer[2] = (high << 4) | low;

    /* Hours */
    // Do it all in 24-h mode. Bit 6 set to 0 is 24-h mode so there is
    // no need to do anything about this.
    low = dt->hour % 10;
    high = (dt->hour - low) / 10;
    buffer[3] = (high << 4) | low;

    /* Day of week (1~7) */
    buffer[4] = dt->dotw & 0x07;

    /* Date (day 1~31) */
    low = dt->day % 10;
    high = (dt->day - low) / 10;
    buffer[5] = (high << 4) | low;

    /* Month (1-12) */
    low = dt->month % 10;
    high = (dt->month - low) / 10;
    buffer[6] = (high << 4) | low;

    /* Century (0~1) */
    // Bit 7 in same value as month
    // century = (val & 0x80) >> 7;

    /* Year (0~99) */
    // Takes only last two digits as it assumes year is between 2000 and
    // 2099.
    low = dt->year % 10;
    high = ((dt->year % 100) - low) / 10;
    buffer[7] = (high << 4) | low;

    i2c_write_blocking(rtc->i2c_port, rtc->i2c_addr, buffer, 8,
                       false);

    // After updating the date and time, set the oscillator stop flag
    // (OSF) to 0. Else, it remains at logic 1.
    uint8_t reg = DS3231_STATUS_REG;
    uint8_t status;
    // Read the status register.
    i2c_write_blocking(rtc->i2c_port, rtc->i2c_addr, &reg, 1, true);
    i2c_read_blocking(rtc->i2c_port, rtc->i2c_addr, &status, 1, true);
    // Unset status bit 7.
    status &= ~(1 << 7);
    uint8_t buf[] = {DS3231_STATUS_REG, status};
    i2c_write_blocking(rtc->i2c_port, rtc->i2c_addr, buf, 2, false);
}

void ds3231_get_datetime(ds3231_datetime_t *dt, ds3231_rtc_t *rtc) {
    uint8_t buffer[7];
    uint8_t low, high, val;
    uint8_t reg = DS3231_DATETIME_REG;
    // Go to the start of the date/time register, then read 7 bytes
    i2c_write_blocking(rtc->i2c_port, rtc->i2c_addr, &reg, 1,
                       true);  // `true` to keep control
    i2c_read_blocking(rtc->i2c_port, rtc->i2c_addr, buffer, 7,
                      false);
    
    /* Seconds */
    val = buffer[0];
    dt->seconds = ((val >> 4) * 10) + (val & 0x0f);

    /* Minutes */
    val = buffer[1];
    dt->minutes = ((val >> 4) * 10) + (val & 0x0f);

    /* Hours (1~12 or 0~23) */
    val = buffer[2];
    // Bit 6 is 12-h (1) or 24-h (0) format
    bool mode_is_12_hour = (val >> 6) & 0x1;
    if (mode_is_12_hour) {  // 12-h mode
        // Bit 5 is AM (0) or PM (1)
        // is_pm = (val & 0x20) >> 5;
        // Bit 4 is 10 hour
        high = ((val & 0x10) >> 4) * 10;
        // Bits 3-0 are hour
        low = val & 0x0f;
        dt->hour = high + low;
    } else {  // 24-h mode
        // Bits 5-4 are 20/10 hour
        high = ((val & 0x30) >> 4) * 10;
        // Bits 3-0 are hour
        low = val & 0x0f;
        dt->hour = high + low;
   }

    /* Day of the week (1~7) */
    val = buffer[3];
    // Bits 2-0 are day of the week
    dt->dotw = val & 0x07;
    
    /* Date (day 1~31) */
    val = buffer[4];
    // Bits 5-4 are 10 date
    high = ((val >> 4) * 10);
    // Bits 3-0 are date
    low = val & 0x0f;
    dt->day = high + low;

    /* Month (1-12) */
    val = buffer[5];
    // Bit 4 is 10 month 
    high = ((val & 0x10) >> 4) * 10;
    // Bits 3-0 are month
    low = val & 0x0f;
    dt->month = high + low;
    
    /* Century (0~1) */
    // Bit 7 in same value as month
    // dt->century = (val & 0x80) >> 7;
    
    /* Year (0~99) */
    val = buffer[6];
    // Bits 7-4 are 10 year
    high = ((val & 0xf0) >> 4) * 10;
    // Bits 3-0 are year
    low = val & 0x0f;
    dt->year = 2000 + high + low;
}

void ds3231_isoformat(char *buf, uint8_t buf_size,
                       const ds3231_datetime_t *dt) {
    snprintf(buf, buf_size, "%u-%02u-%02uT%02u:%02u:%02u",
            dt->year, dt->month, dt->day, dt->hour, dt->minutes, dt->seconds);
}

void ds3231_str_date(char *buf, uint8_t buf_size,
                     const ds3231_datetime_t *dt) {
    snprintf(buf, buf_size, "%u-%02u-%02u", dt->year, dt->month,
             dt->day);
}

void ds3231_str_time(char *buf, uint8_t buf_size,
                     const ds3231_datetime_t *dt) {
    snprintf(buf, buf_size, "%02u:%02u:%02u", dt->hour, dt->minutes,
             dt->seconds);
}

void ds3231_ctime(char *buf, uint8_t buf_size,
                  const ds3231_datetime_t *dt) {
    // Day of the week and months start from 1 in the dt structure. We
    // need to subtract 1 from these values to match the correct
    // indices.
    snprintf(buf, buf_size, "%s %s %02u %02u:%02u:%02u %u",
        DS3231_WDAYS[dt->dotw-1], DS3231_MONTHS[dt->month-1], dt->day,
        dt->hour, dt->minutes, dt->seconds, dt->year);
}

bool ds3231_oscillator_is_stopped(ds3231_rtc_t *rtc) {
    uint8_t reg = DS3231_STATUS_REG;
    uint8_t buf;
    // Oscillator Stop Flag: Bit 7 in status register
    i2c_write_blocking(rtc->i2c_port, rtc->i2c_addr, &reg, 1, true);
    i2c_read_blocking(rtc->i2c_port, rtc->i2c_addr, &buf, 1, false);
    return (buf >> 7) & 0b1;
}

void ds3231_get_temperature(float *val, ds3231_rtc_t *rtc) {
    uint8_t buffer[2];
    uint8_t reg = DS3231_TEMPERATURE_REG;
    float frac;
    
    i2c_write_blocking(rtc->i2c_port, rtc->i2c_addr, &reg, 1, true);
    i2c_read_blocking(rtc->i2c_port, rtc->i2c_addr, buffer, 2, false);

    // This quote from the the data sheet (Rev. 10, p.15) explains the
    // lines that follow:
    //
    // Temperature is represented as a 10-bit code with a resolution of
    // 0.25°C and is accessible at location 11h and 12h. The temperature
    // is encoded in two's complement format. The upper 8 bits, the
    // integer portion, are at location 11h and the lower 2 bits, the
    // fractional portion, are in the upper nibble at location 12h.
    
    frac = (float)(buffer[1] >> 6);
    frac *= 0.25;
    
    if ((buffer[0] >> 7) & 1) {
        // If bit 7 is set, the number is negative
        *val = (float)((~buffer[0]) + 1);
    } else {
        // If bit 7 is not set, the number is positive.
        *val = (float)buffer[0];
    }
    *val += frac;
}
