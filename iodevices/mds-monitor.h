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
 */

#ifndef MDS_MONITOR_INC
#define MDS_MONITOR_INC

				/* Interrupt status and control bits */
#define MDS_MON_ITTYO	0x01	/* Output TTY */
#define MDS_MON_ITTYI	0x02	/* Input TTY */
#define MDS_MON_IPTP	0x04	/* PTP */
#define MDS_MON_IPTR	0x08	/* PTR */
#define MDS_MON_ICRTO	0x10	/* Output CRT */
#define MDS_MON_ICRTI	0x20	/* Input CRT */
#define MDS_MON_ILPT	0x40	/* LPT */
#define MDS_MON_MENB	0x80	/* Enable monitor interrupts */

#define MDS_MON_IRQ	3	/* Monitor module interrupt */

extern BYTE mds_mon_int;

extern BYTE mds_prom_data_in(void), mds_prom_status_in(void);
extern void mds_prom_data_out(BYTE);
extern void mds_prom_high_ctl_out(BYTE), mds_prom_low_out(BYTE);

extern void mds_tty_reset(void);
extern BYTE mds_tty_data_in(void), mds_tty_status_in(void);
extern void mds_tty_data_out(BYTE), mds_tty_ctl_out(BYTE);

extern void mds_crt_reset(void);
extern BYTE mds_crt_data_in(void), mds_crt_status_in(void);
extern void mds_crt_data_out(BYTE), mds_crt_ctl_out(BYTE);

extern BYTE mds_ptr_data_in(void), mds_pt_status_in(void);
extern void mds_ptp_data_out(BYTE), mds_pt_ctl_out(BYTE);

extern BYTE mds_lpt_status_in(void);
extern void mds_lpt_data_out(BYTE), mds_lpt_ctl_out(BYTE);

extern BYTE mds_int_status_in(void);
extern void mds_int_ctl_out(BYTE);

#endif /* !MDS_MONITOR_INC */
