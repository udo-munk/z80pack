/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2016-2017 by Udo Munk
 *
 * Partial emulation of an Altair 88-SIO Rev. 0/1 for terminal I/O,
 * and 88-SIO Rev. 1 for tape I/O
 *
 * History:
 * 12-JUL-16 first version
 * 02-SEP-16 reopen tty at EOF from input redirection
 * 24-FEB-17 improved tty reopen
 * 24-MAR-17 added configuration
 * 27-MAR-17 added SIO 3 for tape connected to UNIX domain socket
 * 23-OCT-17 improved UNIX domain socket connections
 */

extern BYTE altair_sio0_status_in(void);
extern void altair_sio0_status_out(BYTE);
extern BYTE altair_sio0_data_in(void);
extern void altair_sio0_data_out(BYTE);

extern BYTE altair_sio3_status_in(void);
extern void altair_sio3_status_out(BYTE);
extern BYTE altair_sio3_data_in(void);
extern void altair_sio3_data_out(BYTE);
