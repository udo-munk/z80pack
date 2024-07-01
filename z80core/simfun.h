/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 Udo Munk
 * Copyright (C) 2021 David McNaughton
 */

#ifndef SIMFUN_INC
#define SIMFUN_INC

#include "sim.h"

#ifndef BAREMETAL
extern void sleep_us(long time);
extern void sleep_ms(int time);
#endif

/* must be implemented in bare metal simulator */
extern uint64_t get_clock_us(void);

#if defined(WANT_ICE) || defined(BAREMETAL)
/* must be implemented in bare metal simulator */
extern int get_cmdline(char *buf, int len);
#endif

#ifndef BAREMETAL
extern int load_file(char *fn, WORD start, int size);
#endif

#endif /* !SIMFUN_INC */
