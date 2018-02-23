/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2015-2017 by Udo Munk
 *
 * This module contains functions to implement networking connections.
 * The TCP/IP sockets need to support asynchron I/O, the UNIX domain
 * sockets are used with polling.
 *
 * History:
 * 26-MAR-15 first version finished
 * 22-MAR-17 implemented UNIX domain sockets and tested with Altair SIO/2SIO
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
#include "unix_network.h"
#include "sim.h"

void telnet_negotiation(int);

/*
 * initialise a server UNIX domain socket
 */
void init_unix_server_socket(struct unix_connectors *p, char *fn)
{
	struct sockaddr_un sun;
	struct stat sbuf;
	static char *path = "/tmp/.z80pack/";
	static char socket_path[sizeof(sun.sun_path)];

	/* check if /tmp/.z80pack exists */
	if (stat(path, &sbuf) != 0)
		mkdir(path, 0777);   /* no, create it */

	/* create socket path and unlike file */
	strcpy(socket_path, path);
	strcat(socket_path, fn);
	unlink(socket_path);

	/* create the socket, bind it and listen */
	memset((void *)&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strncpy(sun.sun_path, socket_path, sizeof(sun.sun_path) - 1);
	if ((p->ss = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("create server socket");
		exit(1);
	}
	if (bind(p->ss, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
		perror("bind server socket");
		exit(1);
	}
	if (listen(p->ss, 0) == -1) {
		perror("listen on server socket");
		exit(1);
	}
}

/*
 * initialise a server TCP/IP socket
 */
void init_tcp_server_socket(struct net_connectors *p)
{
	struct sockaddr_in sin;
	int on = 1;
	int n;

	/* if the TCP/IP port is not configured we're done here */
	if (p->port == 0)
		return;

	/* create TCP/IP socket */
	if ((p->ss = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("create server socket");
		exit(1);
	}

	/* configure socket for async I/O */
	if (setsockopt(p->ss, SOL_SOCKET, SO_REUSEADDR, (void *) &on,
	    sizeof(on)) == -1) {
		perror("setsockopt SO_REUSEADDR on server socket");
		exit(1);
	}
	fcntl(p->ss, F_SETOWN, getpid());
	n = fcntl(p->ss, F_GETFL, 0);
	if (fcntl(p->ss, F_SETFL, n | FASYNC) == -1) {
		perror("fcntl FASYNC on server socket");
		exit(1);
	}

	/* bind socket and listen on it */
	memset((void *) &sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(p->port);
	if (bind(p->ss, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		perror("bind server socket");
		exit(1);
	}
	if (listen(p->ss, 0) == -1) {
		perror("listen on server socket");
		exit(1);
	}

	printf("telnet console listening on port %d\n", p->port);
}

/*
 * SIGIO interrupt handler for TCP/IP server sockets
 */
void sigio_tcp_server_socket(int sig)
{
	register int i;
	struct pollfd p[NUMNSOC];
	struct sockaddr_in fsin;
	socklen_t alen;
	int go_away;
	int on = 1;

	sig = sig;	/* to avoid compiler warning */

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
				perror("accept on server socket");
				ncons[i].ssc = 0;
			}

			if (setsockopt(ncons[i].ssc, IPPROTO_TCP, TCP_NODELAY,
			    (void *) &on, sizeof(on)) == -1) {
				perror("setsockopt TCP_NODELAY on server socket");
			}

			if (ncons[i].telnet) {
				telnet_negotiation(ncons[i].ssc);
			}
		}
	}
}

/*
 *	telnet option negotiation on TCP/IP server sockets
 */
void telnet_negotiation(int fd)
{
	static char will_echo[3] = {255, 251, 1};
	static char char_mode[3] = {255, 251, 3};
	struct pollfd p[1];
	BYTE c[3];

	/* send the telnet options we need */
	write(fd, &char_mode, 3);
	write(fd, &will_echo, 3);

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
		read(fd, &c, 3);
		//printf("telnet: %d %d %d\r\n", c[0], c[1], c[2]);
		if (c[2] == 1 || c[2] == 3)
			continue;	/* ignore answers to our requests */
		if (c[1] == 251)	/* and reject other options */
			c[1] = 254;
		else if (c[1] == 253)
			c[1] = 252;
		write(fd, &c, 3);
	}
}
