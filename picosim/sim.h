/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2024 by Udo Munk
 *
 * This is the configuration for a Raspberry Pico (W) board
 */

#define DEF_CPU Z80     /* default CPU (Z80 or I8080) */
#define CPU_SPEED 0     /* default CPU speed 0=unlimited */
#define Z80_UNDOC       /* compile undocumented Z80 instructions */

#include "simcore.h"
