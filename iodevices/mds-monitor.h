/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Simulation of an Intel Intellec MDS-800 monitor module
 * It includes the monitor ROM, TTY, CRT, PTR, PTP, line printer,
 * and PROM programmer interface.
 *
 * History:
 * 03-JUN-2024 first version
 * 07-JUN-2024 rewrite of the monitor ports and the timing thread
 */

#ifndef MDS_MONITOR_INC
#define MDS_MONITOR_INC

extern BYTE mon_prom_data_in(void), mon_prom_status_in(void);
extern void mon_prom_data_out(BYTE);
extern void mon_prom_high_ctl_out(BYTE), mon_prom_low_out(BYTE);

extern BYTE mon_int_status_in(void);
extern void mon_int_ctl_out(BYTE);

extern void mon_tty_periodic(void);
extern BYTE mon_tty_data_in(void), mon_tty_status_in(void);
extern void mon_tty_data_out(BYTE), mon_tty_ctl_out(BYTE);

extern void mon_crt_periodic(void);
extern BYTE mon_crt_data_in(void), mon_crt_status_in(void);
extern void mon_crt_data_out(BYTE), mon_crt_ctl_out(BYTE);

extern void mon_pt_periodic(void);
extern BYTE mon_ptr_data_in(void), mon_pt_status_in(void);
extern void mon_ptp_data_out(BYTE), mon_pt_ctl_out(BYTE);

extern void mon_lpt_periodic(void);
extern BYTE mon_lpt_status_in(void);
extern void mon_lpt_data_out(BYTE), mon_lpt_ctl_out(BYTE);

extern void mon_reset(void);

#endif /* !MDS_MONITOR_INC */
