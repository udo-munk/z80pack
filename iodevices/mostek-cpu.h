/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * Emulation of the console I/O ports on the Mostek SDB-80 CPU board
 *
 * History:
 * 15-SEP-2019 (Mike Douglas) created from altair-88-2sio.h
 */

extern BYTE sio_status_in(void);
extern void sio_control_out(BYTE);
extern BYTE sio_data_in(void);
extern void sio_data_out(BYTE);
extern BYTE sio_handshake_in(void);
extern void sio_handshake_out(BYTE);
