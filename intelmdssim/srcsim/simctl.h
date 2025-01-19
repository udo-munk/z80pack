/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2008-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#ifndef SIMCTL_INC
#define SIMCTL_INC

#include <stdbool.h>

extern BYTE boot_switch;

extern void mon(void);

extern bool wait_step(void);
extern void wait_int_step(void);

#endif /* !SIMCTL_INC */
