/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#ifndef SIMCORE_INC
#define SIMCORE_INC

#include "sim.h"
#include "simdefs.h"

extern void init_cpu(void);
extern void reset_cpu(void);
#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
extern void switch_cpu(int new_cpu);
#endif
extern void run_cpu(void);
extern void step_cpu(void);

extern void report_cpu_error(void);
extern void report_cpu_stats(void);

extern BYTE io_in(BYTE addrl, BYTE addrh);
extern void io_out(BYTE addrl, BYTE addrh, BYTE data);

extern void start_bus_request(BusDMA_t mode,
			      Tstates_t (*bus_master)(BYTE bus_ack));
extern void end_bus_request(void);

#endif /* !SIMCORE_INC */
