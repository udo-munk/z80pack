/**
 * log.h
 * 
 * Copyright (C) 2018 by David McNaughton
 * 
 * NOTICE: This is a deriviative of a work that is can be found at and is itself:
 * https://github.com/espressif/esp-idf/blob/master/components/log/include/esp_log.h
 * Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * History:
 * 12-JUL-18    1.0     Initial Release
 * 
 */
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#define CONFIG_LOG_COLORS
#define LOG_DEFAULT_LEVEL   LOG_INFO

typedef enum {
    LOG_NONE,       /* No log output */
    LOG_ERROR,      /* Critical errors, software module can not recover on its own */
    LOG_WARN,       /* Error conditions from which recovery measures have been taken */
    LOG_INFO,       /* Information messages which describe normal flow of events */
    LOG_DEBUG,      /* Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
    LOG_VERBOSE     /* Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
} log_level_t;

static void _log_write(log_level_t level, const char* tag, const char* format, ...) __attribute__ ((always_inline, format (printf, 3, 4)));

static void _log_write(log_level_t level, const char* tag, const char* format, ...)
{
    (void)(level);
    (void)(tag);

    va_list(args);
    va_start(args, format);
    vprintf(format, args);
}

static inline uint32_t _log_timestamp(void) {
    return clock();
}

#ifdef CONFIG_LOG_COLORS
#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D       LOG_COLOR(LOG_COLOR_CYAN)
#define LOG_COLOR_V
#else //CONFIG_LOG_COLORS
#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR
#endif //CONFIG_LOG_COLORS

#define _LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter #letter " (%d) %s: " format LOG_RESET_COLOR "\r\n"

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL  ((log_level_t) LOG_DEFAULT_LEVEL)
#endif

#define LOG( tag, format, ... )  _log_write(0, NULL, format, ##__VA_ARGS__);
#define LOGE( tag, format, ... )  if (LOG_LOCAL_LEVEL >= LOG_ERROR)   { _log_write(LOG_ERROR,   tag, _LOG_FORMAT(E, format), _log_timestamp(), tag, ##__VA_ARGS__); }
#define LOGW( tag, format, ... )  if (LOG_LOCAL_LEVEL >= LOG_WARN)    { _log_write(LOG_WARN,    tag, _LOG_FORMAT(W, format), _log_timestamp(), tag, ##__VA_ARGS__); }
#define LOGI( tag, format, ... )  if (LOG_LOCAL_LEVEL >= LOG_INFO)    { _log_write(LOG_INFO,    tag, _LOG_FORMAT(I, format), _log_timestamp(), tag, ##__VA_ARGS__); }
#define LOGD( tag, format, ... )  if (LOG_LOCAL_LEVEL >= LOG_DEBUG)   { _log_write(LOG_DEBUG,   tag, _LOG_FORMAT(D, format), _log_timestamp(), tag, ##__VA_ARGS__); }
#define LOGV( tag, format, ... )  if (LOG_LOCAL_LEVEL >= LOG_VERBOSE) { _log_write(LOG_VERBOSE, tag, _LOG_FORMAT(V, format), _log_timestamp(), tag, ##__VA_ARGS__); }