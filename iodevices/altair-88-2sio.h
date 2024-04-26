/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2020 by Udo Munk
 *
 * Partial emulation of an Altair 88-2SIO S100 board
 *
 * History:
 * 20-OCT-08 first version finished
 * 31-JAN-14 use correct name from the manual
 * 19-JUN-14 added config parameter for dropping nulls after CR/LF
 * 17-JUL-14 don't block on read from terminal
 * 09-OCT-14 modified to support 2 SIO's
 * 23-MAR-15 drop only null's
 * 02-SEP-16 reopen tty at EOF from input redirection
 * 24-FEB-17 improved tty reopen
 * 22-MAR-17 connected SIO 2 to UNIX domain socket
 * 23-OCT-17 improved UNIX domain socket connections
 * 03-MAY-18 improved accuracy
 * 03-JUL-18 added baud rate to terminal 2SIO
 * 15-JUL-18 use logging
 * 24-NOV-19 configurable baud rate for second channel
 * 19-JUL-20 avoid problems with some third party terminal emulations
 */

extern BYTE altair_sio1_status_in(void);
extern void altair_sio1_status_out(BYTE);
extern BYTE altair_sio1_data_in(void);
extern void altair_sio1_data_out(BYTE);

extern BYTE altair_sio2_status_in(void);
extern void altair_sio2_status_out(BYTE);
extern BYTE altair_sio2_data_in(void);
extern void altair_sio2_data_out(BYTE);
