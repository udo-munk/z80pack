/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2021 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Emulation of an Intel Intellec iSBC 201 single density disk controller
 *
 * History:
 * 09-JUN-2024 first version
 */

#ifndef MDS_ISBC201_INC
#define MDS_ISBC201_INC

extern BYTE isbc201_status_in(void), isbc201_res_type_in(void);
extern BYTE isbc201_res_byte_in(void);

extern void isbc201_iopbl_out(BYTE data), isbc201_iopbh_out(BYTE data);
extern void isbc201_reset_out(BYTE data);

extern void isbc201_disk_check(void);
extern void isbc201_reset(void);

#endif /* !MDS_ISBC201_INC */
