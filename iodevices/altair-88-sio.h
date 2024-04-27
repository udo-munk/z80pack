/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2016-2020 by Udo Munk
 *
 * Partial emulation of an Altair 88-SIO Rev. 0/1 for terminal I/O,
 * and 88-SIO Rev. 1 for tape I/O
 *
 * History:
 * 12-JUL-2016 first version
 * 02-SEP-2016 reopen tty at EOF from input redirection
 * 24-FEB-2017 improved tty reopen
 * 24-MAR-2017 added configuration
 * 27-MAR-2017 added SIO 3 for tape connected to UNIX domain socket
 * 23-OCT-2017 improved UNIX domain socket connections
 * 03-MAY-2018 improved accuracy
 * 04-JUL-2018 added baud rate to terminal SIO
 * 15-JUL-2018 use logging
 * 24-NOV-2019 configurable baud rate for tape SIO
 * 19-JUL-2020 avoid problems with some third party terminal emulations
 */

extern BYTE altair_sio0_status_in(void);
extern void altair_sio0_status_out(BYTE);
extern BYTE altair_sio0_data_in(void);
extern void altair_sio0_data_out(BYTE);

extern BYTE altair_sio3_status_in(void);
extern void altair_sio3_status_out(BYTE);
extern BYTE altair_sio3_data_in(void);
extern void altair_sio3_data_out(BYTE);
