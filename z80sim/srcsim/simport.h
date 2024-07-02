/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 Udo Munk
 * Copyright (C) 2021 David McNaughton
 */

#ifndef SIMPORT_INC
#define SIMPORT_INC

#include <stdint.h>

#include "sim.h"

extern void sleep_for_us(long time);
extern void sleep_for_ms(int time);
extern uint64_t get_clock_us(void);
#ifdef WANT_ICE
extern int get_cmdline(char *buf, int len);
#endif

#endif /* !SIMPORT_INC */
