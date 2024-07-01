/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 * Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef SIMZ80_FDCB_INC
#define SIMZ80_FDCB_INC

#include "sim.h"

#if !defined(EXCLUDE_Z80) && !defined(ALT_Z80)
extern int op_fdcb_handle(void);
#endif

#endif /* !SIMZ80_FDCB_INC */
