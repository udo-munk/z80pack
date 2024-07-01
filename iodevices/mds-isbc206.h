/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2021 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Emulation of an Intel Intellec iSBC 206 hard disk controller
 *
 * History:
 * 08-JUN-2024 first version
 */

#ifndef MDS_ISBC206_INC
#define MDS_ISBC206_INC

extern BYTE isbc206_status_in(void), isbc206_res_type_in(void);
extern BYTE isbc206_res_byte_in(void);

extern void isbc206_iopbl_out(BYTE data), isbc206_iopbh_out(BYTE data);
extern void isbc206_reset_out(BYTE data);

extern void isbc206_disk_check(void);
extern void isbc206_reset(void);

#endif /* !MDS_ISBC206_INC */
