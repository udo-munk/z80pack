/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2024 by Udo Munk
 */

#ifndef SIMCTL_INC
#define SIMCTL_INC

#include "sim.h"
#include "simdefs.h"

extern int boot_switch;			/* boot address for switch */

extern void mon(void);

extern bool wait_step(void);
extern void wait_int_step(void);

#endif /* !SIMCTL_INC */
