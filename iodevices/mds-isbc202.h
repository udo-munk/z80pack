/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Simulation of an Intel Intellec iSBC 202 double density disk controller
 *
 * History:
 */

#ifndef MDS_ISBC202_INC
#define MDS_ISBC202_INC

extern BYTE isbc202_status_in(void), isbc202_res_type_in(void);
extern BYTE isbc202_res_byte_in(void);

extern void isbc202_iopbl_out(BYTE), isbc202_iopbh_out(BYTE);
extern void isbc202_reset_out(BYTE);

#endif /* !MDS_ISBC202_INC */
