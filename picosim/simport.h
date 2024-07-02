/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 Thomas Eberhardt
 */

#ifndef SIMPORT_INC
#define SIMPORT_INC

#include <stdint.h>
#include "pico/time.h"

static inline void sleep_for_us(long time) { sleep_us(time); }
static inline void sleep_for_ms(int time) { sleep_ms(time); }

static inline uint64_t get_clock_us(void)
{
	return to_us_since_boot(get_absolute_time());
}

extern int get_cmdline(char *buf, int len);

#endif /* !SIMPORT_INC */
