/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2015-2018 by Udo Munk
 *
 * This module contains functions to implement networking connections.
 *
 * History:
 * 26-MAR-2015 first version finished
 * 22-MAR-2017 implemented UNIX domain sockets and tested with Altair SIO/2SIO
 * 22-APR-2018 implemented TCP socket polling
 * 14-JUL-2018 use logging
 */

#ifndef UNIX_NETWORK_INC
#define UNIX_NETWORK_INC

#define TELNET_TIMEOUT 800	/* telnet negotiation timeout in milliseconds */

/* structure for UNIX socket connections */
struct unix_connectors {
	int ss;		/* server socket descriptor */
	int ssc;	/* connected server socket descriptor */
};

/* structure for TCP/IP socket connections */
struct net_connectors {
	int ss;		/* server socket descriptor */
	int ssc;	/* connected server socket descriptor */
	int port;	/* TCP/IP port for server socket */
	int telnet;	/* telnet protocol flag for TCP/IP server socket */
};

extern void init_unix_server_socket(struct unix_connectors *p, const char *fn);

extern void init_tcp_server_socket(struct net_connectors *p);
extern void sigio_tcp_server_socket(int sig);

extern void telnet_negotiation(int fd);

#endif /* !UNIX_NETWORK_INC */
