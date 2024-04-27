/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2021 by Udo Munk
 * Copyright (C) 2018-2021 David McNaughton
 *
 * Emulation of IMSAI SIO-2 S100 boards
 *
 * History:
 * 20-OCT-2008 first version finished
 * 19-JUN-2014 added config parameter for dropping nulls after CR/LF
 * 18-JUL-2014 don't block on read from terminal
 * 09-OCT-2014 modified to support SIO 2
 * 23-MAR-2015 drop only null's
 * 22-AUG-2017 reopen tty at EOF from input redirection
 * 03-MAY-2018 improved accuracy
 * 03-JUL-2018 implemented baud rate for terminal SIO
 * 13-JUL-2018 use logging
 * 14-JUL-2018 integrate webfrontend
 * 12-JUL-2019 implemented second SIO
 * 27-JUL-2019 more correct emulation
 * 17-SEP-2019 more consistent SIO naming
 * 23-SEP-2019 added AT-modem
 * 06-OCT-2019 started to implement telnet protocol for modem device
 * 07-OCT-2019 implemented baud rate for modem device
 * 09-OCT-2019 implement telnet binary transfer
 * 12-NOV-2019 implemented SIO control ports
 * 19-JUL-2020 avoid problems with some third party terminal emulations
 * 14-JUL-2021 added all options for SIO 2B
 * 15-JUL-2021 refactor serial keyboard
 * 16-JUL-2021 added all options for SIO 1B
 * 01-AUG-2021 integrated HAL
 */

extern BYTE imsai_sio_nofun_in(void);
extern void imsai_sio_nofun_out(BYTE);

extern BYTE imsai_sio1_ctl_in(void), imsai_sio2_ctl_in(void);
extern void imsai_sio1_ctl_out(BYTE), imsai_sio2_ctl_out(BYTE);

extern BYTE imsai_sio1a_status_in(void);
extern void imsai_sio1a_status_out(BYTE);
extern BYTE imsai_sio1a_data_in(void);
extern void imsai_sio1a_data_out(BYTE);

extern BYTE imsai_sio1b_status_in(void);
extern void imsai_sio1b_status_out(BYTE);
extern BYTE imsai_sio1b_data_in(void);
extern void imsai_sio1b_data_out(BYTE);

extern BYTE imsai_sio2a_status_in(void);
extern void imsai_sio2a_status_out(BYTE);
extern BYTE imsai_sio2a_data_in(void);
extern void imsai_sio2a_data_out(BYTE);

#ifdef HAS_MODEM
extern BYTE imsai_sio2b_status_in(void);
extern void imsai_sio2b_status_out(BYTE);
extern BYTE imsai_sio2b_data_in(void);
extern void imsai_sio2b_data_out(BYTE);
#endif
