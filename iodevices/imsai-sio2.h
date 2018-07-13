/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2018 by Udo Munk
 *
 * Emulation of an IMSAI SIO-2 S100 board
 *
 * History:
 * 20-OCT-08 first version finished
 * 19-JUN-14 added config parameter for droping nulls after CR/LF
 * 18-JUL-14 don't block on read from terminal
 * 09-OCT-14 modified to support SIO 2
 * 23-MAR-15 drop only null's
 * 22-AUG-17 reopen tty at EOF from input redirection
 * 03-MAY-18 improved accuracy
 * 03-JUL-18 implemented baud rate for terminal SIO
 * 13-JUL-18 use logging
 */

extern BYTE imsai_sio1_status_in(void);
extern void imsai_sio1_status_out(BYTE);
extern BYTE imsai_sio1_data_in(void);
extern void imsai_sio1_data_out(BYTE);
