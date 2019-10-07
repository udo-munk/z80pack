/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2019 by Udo Munk
 * Copyright (C) 2018-2019 David McNaughton
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
 * 14-JUL-18 integrate webfrontend
 * 12-JUL-19 implemented second SIO
 * 27-JUL-19 more correct emulation
 * 17-SEP-19 more consistent SIO naming
 * 23-SEP-19 added AT-modem
 * 06-OCT-19 started to implement telnet protocol for modem device
 * 07-OCT-19 implemented baud rate for modem device
 */

extern BYTE imsai_sio_nofun_in(void);
extern void imsai_sio_nofun_out(BYTE);

extern BYTE imsai_sio1a_status_in(void), imsai_sio2a_status_in(void);
extern void imsai_sio1a_status_out(BYTE), imsai_sio2a_status_out(BYTE);
extern BYTE imsai_sio1a_data_in(void), imsai_sio2a_data_in(void);
extern void imsai_sio1a_data_out(BYTE), imsai_sio2a_data_out(BYTE);
#ifdef HAS_MODEM
extern BYTE imsai_sio2b_status_in(void);
extern void imsai_sio2b_status_out(BYTE);
extern BYTE imsai_sio2b_data_in(void);
extern void imsai_sio2b_data_out(BYTE);
#endif
