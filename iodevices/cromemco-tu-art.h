/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2015 by Udo Munk
 *
 * Emulation of a Cromemco TU-ART S100 board
 *
 * History:
 *    DEC-14 first version
 *    JAN-15 better subdue of non printable characters in output
 * 02-FEB-15 implemented the timers and interrupt flag for TBE
 * 05-FEB-15 implemented interrupt flag for RDA
 * 14-FEB-15 improvements, so that the Cromix tty driver works
 * 10-MAR-15 lpt's implemented for CP/M, CDOS and Cromix
 * 23-MAR-15 drop only null's
 * 26-MAR-15 tty's implemented for CDOS and Cromix
 */

extern BYTE cromemco_tuart_0a_status_in(void);
extern void cromemco_tuart_0a_baud_out(BYTE);

extern BYTE cromemco_tuart_0a_data_in(void);
extern void cromemco_tuart_0a_data_out(BYTE);

extern void cromemco_tuart_0a_command_out(BYTE);

extern BYTE cromemco_tuart_0a_interrupt_in(void);
extern void cromemco_tuart_0a_interrupt_out(BYTE);

extern BYTE cromemco_tuart_0a_parallel_in(void);
extern void cromemco_tuart_0a_parallel_out(BYTE);

extern void cromemco_tuart_0a_timer1_out(BYTE);
extern void cromemco_tuart_0a_timer2_out(BYTE);
extern void cromemco_tuart_0a_timer3_out(BYTE);
extern void cromemco_tuart_0a_timer4_out(BYTE);
extern void cromemco_tuart_0a_timer5_out(BYTE);

extern int uart0a_int_mask, uart0a_int, uart0a_int_pending, uart0a_rst7;
extern int uart0a_timer1, uart0a_timer2, uart0a_timer3, uart0a_timer4, uart0a_timer5;
extern int uart0a_tbe, uart0a_rda;

/* <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><> */

extern BYTE cromemco_tuart_1a_status_in(void);
extern void cromemco_tuart_1a_baud_out(BYTE);

extern BYTE cromemco_tuart_1a_data_in(void);
extern void cromemco_tuart_1a_data_out(BYTE);

extern void cromemco_tuart_1a_command_out(BYTE);

extern BYTE cromemco_tuart_1a_interrupt_in(void);
extern void cromemco_tuart_1a_interrupt_out(BYTE);

extern BYTE cromemco_tuart_1a_parallel_in(void);
extern void cromemco_tuart_1a_parallel_out(BYTE);

extern int uart1a_int_mask, uart1a_int, uart1a_int_pending;
extern int uart1a_sense, uart1a_lpt_busy;
extern int uart1a_tbe, uart1a_rda;

/* <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><> */

extern BYTE cromemco_tuart_1b_status_in(void);
extern void cromemco_tuart_1b_baud_out(BYTE);

extern BYTE cromemco_tuart_1b_data_in(void);
extern void cromemco_tuart_1b_data_out(BYTE);

extern void cromemco_tuart_1b_command_out(BYTE);

extern BYTE cromemco_tuart_1b_interrupt_in(void);
extern void cromemco_tuart_1b_interrupt_out(BYTE);

extern BYTE cromemco_tuart_1b_parallel_in(void);
extern void cromemco_tuart_1b_parallel_out(BYTE);

extern int uart1b_int_mask, uart1b_int, uart1b_int_pending;
extern int uart1b_sense, uart1b_lpt_busy;
extern int uart1b_tbe, uart1b_rda;
