/*
 * Copyright (c) 2023 Antonio González
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DS3231_H
#define DS3231_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/** \file ds3231.h
 * \brief Library for using a DS3231 real-time clock with the Raspberry
 * Pi Pico
 *
*/

#define DS3231_I2C_ADDRESS 0x68
#define DS3231_I2C_FREQ 100 * 1000

enum rtc_register {
    DS3231_DATETIME_REG = 0x00u,   
    DS3231_ALARM1_REG = 0x07u,    
    DS3231_ALARM2_REG = 0x0Bu,
    DS3231_CONTROL_REG = 0x0Eu,
    DS3231_STATUS_REG = 0x0Fu,
    DS3231_AGING_OFFSET_REG = 0x10u,
    DS3231_TEMPERATURE_REG = 0x11u
};

/*
 * Even if the RTC deals with 12-h formats, this library uses only 24-h
 * formats to read/write from/to the RTC. Any conversions to 12-h format
 * could be implemented later as separate functions if desired.
 * 
 * This library: assumes century is 2000, hour is alway entered in 24-h format, day 1 is Monday, day 7 is Sunday
 * DS years are 1900 to 2099. Here I assume century is 2000
*/
typedef struct {
    uint8_t hour;    // Hour (0-23)
    uint8_t minutes; // Minutes (0-59)
    uint8_t seconds; // Seconds (0-59)
    uint8_t day;     // Day of the month (1-31)
    uint8_t dotw;    // Day of the week (1-7, 1=Monday)
    uint8_t month;   // Month (1-12, 1 = January)
    uint16_t year;   // Year (2000-2099)
} ds3231_datetime_t;


typedef struct ds3231_rtc {
    i2c_inst_t *i2c_port;
    uint8_t i2c_addr;
    uint8_t i2c_sda_pin;
    uint8_t i2c_scl_pin;
} ds3231_rtc_t;

static const char *DS3231_WDAYS[7] = {"Mon", "Tue", "Wed", "Thu", "Fri",
                                      "Sat", "Sun"};

static const char *DS3231_MONTHS[12] = {"Jan", "Feb", "Mar", "Apr",
                                        "May", "Jun", "Jul", "Aug",
                                        "Sep", "Oct", "Nov", "Dec"};

/*! \brief Initialise a DS321 real-time clock
 *
 * \param i2c_port The I2C instance, i2c0 or i2c1
 * \param i2c_addr 7-bit address of the DS3231 device
 * \param rtc Pointer to a ds3231_rtc_t structure
*/
void ds3231_init(i2c_inst_t *i2c_port, uint8_t i2c_sda_pin,
                 uint8_t i2c_scl_pin, ds3231_rtc_t *rtc);

/*! 
*/
void ds3231_set_datetime(ds3231_datetime_t *dt, ds3231_rtc_t *rtc);

void ds3231_get_datetime(ds3231_datetime_t *dt, ds3231_rtc_t *rtc);

/*! \brief Format a ds3231_datetime_t structure into a ISO 8601 string
 * 
 * Date/time string in the format YYYY-MM-DDTHH:MM:SS
 *
 * \param buf Character buffer to accept generated string
 * \param buf_size The size of the passed-in buffer (at least 19)
 * \param dt The datetime to be converted
*/
 void ds3231_isoformat(char *buf, uint8_t buf_size,
                       const ds3231_datetime_t *dt);

/*! \brief Format ds3231_datetime_t structure into a date string
 * 
 * \param buf Character buffer to accept generated string
 * \param buf_size The size of the passed-in buffer (at least 10)
 * \param dt The datetime to be converted
*/
void ds3231_str_date(char *buf, uint8_t buf_size,
                     const ds3231_datetime_t *dt);


/*! \brief Format ds3231_datetime_t structure into a time string
 * 
 * \param buf Character buffer to accept generated string
 * \param buf_size The size of the passed-in buffer (at least 8)
 * \param dt The datetime to be converted
*/
void ds3231_str_time(char *buf, uint8_t buf_size,
                     const ds3231_datetime_t *dt);


/*! \brief Format ds3231_datetime_t structure into a ctime string
 * 
 * Convert datetime to a string in the format "Www Mmm dd hh:mm:ss
 * yyyy", where Www is the weekday, Mmm the month in letters, dd the day
 * of the month, hh:mm:ss the time, and yyyy the year.
 * 
 * \param buf Character buffer to accept generated string
 * \param buf_size The size of the passed-in buffer (at least 25)
 * \param dt The datetime to be converted
*/
void ds3231_ctime(char *buf, uint8_t buf_size,
                  const ds3231_datetime_t *dt);

/*! \brief Check if the oscillator has stopped
 * 
 * \param rtc Pointer to the ds3231_rtc_t structure
*/
bool ds3231_oscillator_is_stopped(ds3231_rtc_t *rtc);

/*! \brief Read the temperature
 * 
 * \param val Pointer to the temperature variable
 * \param rtc Pointer to the ds3231_rtc_t structure
 * 
 * Note that the temperature registers are updated every 64 seconds. It
 * thus makes little sense to read the temperature register more often
 * than this.
*/
void ds3231_get_temperature(float *val, ds3231_rtc_t *rtc);

#endif
