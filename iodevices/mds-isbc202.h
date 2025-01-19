/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2021 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Emulation of an Intel Intellec iSBC 202 double density disk controller
 *
 * History:
 * 04-JUN-2024 first version
 */

#ifndef MDS_ISBC202_INC
#define MDS_ISBC202_INC

#include "sim.h"
#include "simdefs.h"

extern BYTE isbc202_status_in(void), isbc202_res_type_in(void);
extern BYTE isbc202_res_byte_in(void);

extern void isbc202_iopbl_out(BYTE data), isbc202_iopbh_out(BYTE data);
extern void isbc202_reset_out(BYTE data);

extern void isbc202_disk_check(void);
extern void isbc202_reset(void);

#endif /* !MDS_ISBC202_INC */
