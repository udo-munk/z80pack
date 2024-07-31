/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This module performs provides host file I/O using a BDOS
 * like interface.
 *
 * History:
 * 03-OCT-2019 Original
 */

#ifndef SIMBDOS_INC
#define SIMBDOS_INC

#include "sim.h"
#include "simdefs.h"

extern void host_bdos_out(BYTE outByte);

#endif /* !SIMBDOS_INC */
