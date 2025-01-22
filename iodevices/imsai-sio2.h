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

#ifndef IMSAI_SIO2_INC
#define IMSAI_SIO2_INC

#include "sim.h"
#include "simdefs.h"

extern bool sio1a_upper_case;	/* SIO 1 A translate input to upper case */
extern bool sio1a_strip_parity;	/* SIO 1 A strip parity from output */
extern bool sio1a_drop_nulls;	/* SIO 1 A drop nulls after CR/LF */
extern int sio1a_baud_rate;	/* SIO 1 A simulated baud rate */

extern bool sio1b_upper_case;	/* SIO 1 B translate input to upper case */
extern bool sio1b_strip_parity;	/* SIO 1 B strip parity from output */
extern bool sio1b_drop_nulls;	/* SIO 1 B drop nulls after CR/LF */
extern int sio1b_baud_rate;	/* SIO 1 B simulated baud rate */

extern bool sio2a_upper_case;	/* SIO 2 A translate input to upper case */
extern bool sio2a_strip_parity;	/* SIO 2 A strip parity from output */
extern bool sio2a_drop_nulls;	/* SIO 2 A drop nulls after CR/LF */
extern int sio2a_baud_rate;	/* SIO 2 A simulated baud rate */

extern bool sio2b_upper_case;	/* SIO 2 B translate input to upper case */
extern bool sio2b_strip_parity;	/* SIO 2 B strip parity from output */
extern bool sio2b_drop_nulls;	/* SIO 2 B drop nulls after CR/LF */
extern int sio2b_baud_rate;	/* SIO 2 B simulated baud rate */

extern void imsai_sio_reset(void);

extern BYTE imsai_sio_nofun_in(void);
extern void imsai_sio_nofun_out(BYTE data);

extern BYTE imsai_sio1_ctl_in(void), imsai_sio2_ctl_in(void);
extern void imsai_sio1_ctl_out(BYTE data), imsai_sio2_ctl_out(BYTE data);

extern BYTE imsai_sio1a_status_in(void);
extern void imsai_sio1a_status_out(BYTE data);
extern BYTE imsai_sio1a_data_in(void);
extern void imsai_sio1a_data_out(BYTE data);

extern BYTE imsai_sio1b_status_in(void);
extern void imsai_sio1b_status_out(BYTE data);
extern BYTE imsai_sio1b_data_in(void);
extern void imsai_sio1b_data_out(BYTE data);

extern BYTE imsai_sio2a_status_in(void);
extern void imsai_sio2a_status_out(BYTE data);
extern BYTE imsai_sio2a_data_in(void);
extern void imsai_sio2a_data_out(BYTE data);

#ifdef HAS_MODEM
extern BYTE imsai_sio2b_status_in(void);
extern void imsai_sio2b_status_out(BYTE data);
extern BYTE imsai_sio2b_data_in(void);
extern void imsai_sio2b_data_out(BYTE data);
#endif

#endif /* !IMSAI_SIO2_INC */
