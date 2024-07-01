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

#ifndef ALTAIR_88_SIO_INC
#define ALTAIR_88_SIO_INC

extern int sio0_upper_case;	/* SIO 0 translate input to upper case */
extern int sio0_strip_parity;	/* SIO 0 strip parity from output */
extern int sio0_drop_nulls;	/* SIO 0 drop nulls after CR/LF */
extern int sio0_revision;	/* SIO 0 Rev 0 or Rev 1 */
extern int sio0_baud_rate;	/* SIO 0 baud rate */

extern int sio3_baud_rate;	/* SIO 3 baud rate */

extern BYTE altair_sio0_status_in(void);
extern void altair_sio0_status_out(BYTE data);
extern BYTE altair_sio0_data_in(void);
extern void altair_sio0_data_out(BYTE data);

extern BYTE altair_sio3_status_in(void);
extern void altair_sio3_status_out(BYTE data);
extern BYTE altair_sio3_data_in(void);
extern void altair_sio3_data_out(BYTE data);

#endif /* !ALTAIR_88_SIO_INC */
