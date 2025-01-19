/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2015-2020 by Udo Munk
 *
 * This module contains functions to implement networking connections.
 *
 * History:
 * 26-MAR-2015 first version finished
 * 22-MAR-2017 implemented UNIX domain sockets and tested with Altair SIO/2SIO
 * 22-APR-2018 implemented TCP socket polling
 * 14-JUL-2018 use logging
 * 16-JUL-2020 fix bug/warning detected by gcc 9
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "sim.h"
#include "simdefs.h"
#include "simio.h"

#include "unix_network.h"

/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"
static const char *TAG = "net";

/*
 * initialize a server UNIX domain socket
 */
void init_unix_server_socket(unix_connector_t *p, const char *fn)
{
	struct sockaddr_un sun;
	struct stat sbuf;
	static const char *path = "/tmp/.z80pack/";
	static char socket_path[sizeof(sun.sun_path)];

	/* check if /tmp/.z80pack exists */
	if (stat(path, &sbuf) != 0)
		mkdir(path, 0777);   /* no, create it */

	/* create socket path and unlink file */
	strcpy(socket_path, path);
	strcat(socket_path, fn);
	unlink(socket_path);

	/* create the socket, bind it and listen */
	memset((void *) &sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strncpy(sun.sun_path, socket_path, sizeof(sun.sun_path));
	if ((p->ss = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		LOGE(TAG, "can't create server socket");
		exit(EXIT_FAILURE);
	}
	if (bind(p->ss, (struct sockaddr *) &sun, sizeof(sun)) == -1) {
		LOGE(TAG, "can't bind server socket");
		exit(EXIT_FAILURE);
	}
	if (listen(p->ss, 0) == -1) {
		LOGE(TAG, "can't listen on server socket");
		exit(EXIT_FAILURE);
	}
}

/*
 * initialize a server TCP/IP socket
 */
void init_tcp_server_socket(net_connector_t *p)
{
	struct sockaddr_in sin;
	int on = 1;
#ifdef TCPASYNC
	int n;
#endif

	/* if the TCP/IP port is not configured we're done here */
	if (p->port == 0)
		return;

	/* create TCP/IP socket */
	if ((p->ss = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		LOGE(TAG, "can't create server socket");
		exit(EXIT_FAILURE);
	}

	/* set socket options */
	if (setsockopt(p->ss, SOL_SOCKET, SO_REUSEADDR, (void *) &on,
		       sizeof(on)) == -1) {
		LOGE(TAG, "can't setsockopt SO_REUSEADDR on server socket");
		exit(EXIT_FAILURE);
	}

#ifdef TCPASYNC
	/* configure socket for async I/O */
	fcntl(p->ss, F_SETOWN, getpid());
	n = fcntl(p->ss, F_GETFL, 0);
	if (fcntl(p->ss, F_SETFL, n | O_ASYNC) == -1) {
		LOGE(TAG, "can't fcntl O_ASYNC on server socket");
		exit(EXIT_FAILURE);
	}
#endif

	/* bind socket and listen on it */
	memset((void *) &sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(p->port);
	if (bind(p->ss, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		LOGE(TAG, "can't bind server socket");
		exit(EXIT_FAILURE);
	}
	if (listen(p->ss, 0) == -1) {
		LOGE(TAG, "can't listen on server socket");
		exit(EXIT_FAILURE);
	}

	LOG(TAG, "telnet console listening on port %d\r\n", p->port);
}

#if NUMNSOC > 0
/*
 * SIGIO interrupt handler for TCP/IP server sockets
 * if SIGIO not working (Cygwin e.g.) call from appropriate I/O thread
 */
void sigio_tcp_server_socket(int sig)
{
	register int i;
	struct pollfd p[NUMNSOC];
	struct sockaddr_in fsin;
	socklen_t alen;
	int go_away;
	int on = 1;

	UNUSED(sig);

	for (i = 0; i < NUMNSOC; i++) {
		p[i].fd = ncons[i].ss;
		p[i].events = POLLIN;
		p[i].revents = 0;
	}

	poll(p, NUMNSOC, 0);

	for (i = 0; i < NUMNSOC; i++) {
		if ((ncons[i].ss != 0) && (p[i].revents)) {
			alen = sizeof(fsin);

			if (ncons[i].ssc != 0) {
				go_away = accept(ncons[i].ss,
						 (struct sockaddr *) &fsin,
						 &alen);
				close(go_away);
				return;
			}

			if ((ncons[i].ssc = accept(ncons[i].ss,
						   (struct sockaddr *) &fsin,
						   &alen)) == -1) {
				LOGW(TAG, "can't accept on server socket");
				ncons[i].ssc = 0;
			}

			if (setsockopt(ncons[i].ssc, IPPROTO_TCP, TCP_NODELAY,
				       (void *) &on, sizeof(on)) == -1) {
				LOGW(TAG, "can't set sockopt TCP_NODELAY on server socket");
			}

			if (ncons[i].telnet) {
				telnet_negotiation(ncons[i].ssc);
			}
		}
	}
}
#endif

/*
 *	telnet option negotiation on TCP/IP server sockets
 */
void telnet_negotiation(int fd)
{
	static unsigned char will_echo[3] = {255, 251, 1};
	static unsigned char char_mode[3] = {255, 251, 3};
	struct pollfd p[1];
	BYTE c[3];

	/* send the telnet options we need */
	if (write(fd, &char_mode, 3) != 3)
		LOGE(TAG, "can't send char_mode telnet option");
	if (write(fd, &will_echo, 3) != 3)
		LOGE(TAG, "can't send will_echo telnet option");

	/* and reject all others offered */
	p[0].fd = fd;
	p[0].events = POLLIN;
	while (1) {
		/* wait for input */
		p[0].revents = 0;
		poll(p, 1, TELNET_TIMEOUT);

		/* done if no more input */
		if (! p[0].revents)
			break;

		/* else read the option */
		if (read(fd, &c, 3) != 3)
			LOGE(TAG, "can't read telnet option");
		LOGD(TAG, "telnet: %d %d %d", c[0], c[1], c[2]);
		if (c[2] == 1 || c[2] == 3)
			continue;	/* ignore answers to our requests */
		if (c[1] == 251)	/* and reject other options */
			c[1] = 254;
		else if (c[1] == 253)
			c[1] = 252;
		if (write(fd, &c, 3) != 3)
			LOGE(TAG, "can't write telnet option");
	}
}
