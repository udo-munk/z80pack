/**
 * generic-at-modem.h
 * 
 * Emulation of generic 'AT' modem over TCP/IP sockets (telnet)
 *
 * Copyright (C) 2019 by David McNaughton
 * 
 * History:
 * 12-SEP-19    1.0     Initial Release
 * 29-SEP-19    1.1     Added Answer modes and registers
 * 06-OCT-19	1.2	Implemented telnet protocol
 */

extern int modem_device_alive(int);
extern int modem_device_poll(int);
extern int modem_device_get(int);
extern void modem_device_send(int, char);

#define DEV_SIO2B 0
