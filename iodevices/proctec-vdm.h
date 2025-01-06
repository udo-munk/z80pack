/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2017-2018 by Udo Munk
 * Copyright (C) 2025 by Thomas Eberhardt
 *
 * Emulation of a Processor Technology VDM-1 S100 board
 *
 * History:
 * 28-FEB-2017 first version, all software tested with working
 * 21-JUN-2017 don't use dma_read(), switches Tarbell ROM off
 * 20-APR-2018 avoid thread deadlock on Windows/Cygwin
 * 15-JUL-2018 use logging
 * 04-JAN-2025 add SDL2 support
 */

#ifndef PROCTEC_VDM_INC
#define PROCTEC_VDM_INC

#include <stdint.h>

#include "sim.h"
#include "simdefs.h"

extern int slf;			/* VDM scanlines factor */
extern uint8_t bg_color[3];	/* VDM background color */
extern uint8_t fg_color[3];	/* VDM foreground color */

extern int proctec_kbd_status;
extern int proctec_kbd_data;

extern void proctec_vdm_off(void);
extern void proctec_vdm_out(BYTE data);

#endif /* !PROTEC_VDM_INC */
