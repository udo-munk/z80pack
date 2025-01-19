/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2019 by Udo Munk
 *
 * This module contains a complex I/O-simulation for running
 * CP/M, MP/M, UCSD p-System, Fuzix ...
 *
 * Please note this doesn't emulate any hardware which
 * ever existed, we've got all virtual circuits in here!
 *
 * History:
 * 28-SEP-1987 Development on TARGON/35 with AT&T Unix System V.3
 * 19-MAY-1989 Additions for CP/M 3.0 and MP/M
 * 23-DEC-1990 Ported to COHERENT 3.0
 * 10-JUN-1992 Some optimization done
 * 25-JUN-1992 Flush output of stdout only at every OUT to port 0
 * 25-JUN-1992 Comments in english and ported to COHERENT 4.0
 * 05-OCT-2006 modified to compile on modern POSIX OS's
 * 18-NOV-2006 added a second harddisk
 * 08-DEC-2006 modified MMU so that segment size can be configured
 * 10-DEC-2006 started adding serial port for a passive TCP/IP socket
 * 14-DEC-2006 started adding serial port for a client TCP/IP socket
 * 25-DEC-2006 CPU speed option and 100 ticks interrupt
 * 19-FEB-2007 improved networking
 * 22-JUL-2007 added second FDC sector port for implementing large harddisks
 * 30-OCT-2007 printer port returns EOF on input
 * 03-SEP-2007 improved the clock chip for support of other OS's
 * 19-SEP-2007 delay circuit modified to delay x * 10ms
 * 05-DEC-2007 fixed socket shutdown issues for NetBSD
 * 06-DEC-2007 added hardware control port to reset I/O, CPU and reboot and such
 * 07-DEC-2007 conditional compile pipes for aux device, problems with Cygwin
 * 17-DEC-2007 conditional compile async TCP/IP server, problems with Cygwin
 * 03-FEB-2008 added hardware control port to reset CPU, MMU and abort sim
 * 07-APR-2008 added port to set/get CPU speed
 * 13-AUG-2008 work on console I/O busy waiting detection
 * 24-AUG-2008 changed terminal line discipline to not add CR if LF send
 * xx-OCT-2008 some improvements here and there
 * xx-JAN-2014 some improvements here and there
 * 02-MAR-2014 source cleanup and improvements
 * 03-MAI-2014 improved network code, telnet negotiation rewritten
 * 16-JUL-2014 unused I/O ports need to return FF, see survey.mac
 * 17-SEP-2014 FDC error 8 for DMA overrun, as reported by Alan Cox
 * 17-SEP-2014 fixed bug in MMU bank select, as reported by Alan Cox
 * 31-JAN-2015 took over some improvements made for the Z-1 emulation
 * 28-FEB-2015 cleanup for 1.25 release
 * 09-MAR-2016 moved pipes to /tmp/.z80pack
 * 14-MAR-2016 renamed the used disk images to drivex.dsk
 * 12-MAY-2016 find disk images at -d <path>, ./disks and DISKSDIR
 * 22-JUL-2016 added support for read only disks
 * 21-DEC-2016 moved MMU out to the new memory interface module
 * 20-MAR-2017 renamed pipes for auxin/auxout
 * 29-JUN-2017 system reset overworked
 * 24-APR-2018 cleanup
 * 17-MAY-2018 improved hardware control
 * 10-JUN-2018 modified boot code for early loading of files
 * 30-MAR-2019 added two more 4MB HD's
 * 08-OCT-2019 (Mike Douglas) added OUT 161 trap to simbdos.c for host file I/O
 * 24-OCT-2019 move RTC to I/O module for usage by any machine
 * 27-MAY-2024 moved io_in & io_out to simcore
 */

/*
 *	This module contains the I/O handlers for a simulation
 *	of the hardware required for a CP/M / MP/M system.
 *
 *	Used I/O ports:
 *
 *	 0 - console status
 *	 1 - console data
 *
 *	 2 - printer status
 *	 3 - printer data
 *
 *	 4 - auxiliary status
 *	 5 - auxiliary data
 *
 *	10 - FDC drive
 *	11 - FDC track
 *	12 - FDC sector (low)
 *	13 - FDC command
 *	14 - FDC status
 *
 *	15 - DMA destination address low
 *	16 - DMA destination address high
 *
 *	17 - FDC sector high
 *
 *	20 - MMU initialization
 *	21 - MMU bank select
 *	22 - MMU select segment size (in pages a 256 bytes)
 *	23 - MMU write protect/unprotect common memory segment
 *
 *	25 - clock command
 *	26 - clock data
 *	27 - 10ms timer causing maskable interrupt
 *	28 - x * 10ms delay circuit for busy waiting loops
 *	30 - CPU speed low
 *	31 - CPU speed high
 *
 *	40 - passive socket #1 status
 *	41 - passive socket #1 data
 *	42 - passive socket #2 status
 *	43 - passive socket #2 data
 *	44 - passive socket #3 status
 *	45 - passive socket #3 data
 *	46 - passive socket #4 status
 *	47 - passive socket #4 data
 *
 *	50 - client socket #1 status
 *	51 - client socket #1 data
 *
 *	160 - hardware control
 */

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/poll.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcore.h"
#include "simmem.h"
#include "simctl.h"
#include "simport.h"
#include "simio.h"

#include "rtc80.h"
#include "simbdos.h"

#ifdef NETWORKING
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#endif

/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"
static const char *TAG = "IO";

#define BUFSIZE 256		/* max line length of command buffer */
#define MAX_BUSY_COUNT 10	/* max counter to detect I/O busy waiting
				   on the console status port */

static BYTE drive;		/* current drive A..P (0..15) */
static BYTE track;		/* current track (0..255) */
static unsigned int sector;	/* current sector (0..65535) */
static BYTE status;		/* status of last I/O operation on FDC */
static BYTE dmadl;		/* current DMA address destination low */
static BYTE dmadh;		/* current DMA address destination high */
static BYTE timer;		/* 10ms timer */
static int drivea;		/* fd for file "drivea.dsk" */
static int driveb;		/* fd for file "driveb.dsk" */
static int drivec;		/* fd for file "drivec.dsk" */
static int drived;		/* fd for file "drived.dsk" */
static int drivee;		/* fd for file "drivee.dsk" */
static int drivef;		/* fd for file "drivef.dsk" */
static int driveg;		/* fd for file "driveg.dsk" */
static int driveh;		/* fd for file "driveh.dsk" */
static int drivei;		/* fd for file "drivei.dsk" */
static int drivej;		/* fd for file "drivej.dsk" */
static int drivek;		/* fd for file "drivek.dsk" */
static int drivel;		/* fd for file "drivel.dsk" */
static int drivem;		/* fd for file "drivem.dsk" */
static int driven;		/* fd for file "driven.dsk" */
static int driveo;		/* fd for file "driveo.dsk" */
static int drivep;		/* fd for file "drivep.dsk" */
static int printer;		/* fd for file "printer.txt" */
static char fn[MAX_LFN];	/* path/filename for disk images */
static int speed;		/* to reset CPU speed */
static BYTE hwctl_lock = 0xff;	/* lock status hardware control port */

#ifdef PIPES
static int auxin;		/* fd for pipe "auxin" */
static int auxout;		/* fd for pipe "auxout" */
static int aux_in_eof;		/* status of pipe "auxin" (<>0 means EOF) */
static int pid_rec;		/* PID of the receiving process for auxiliary */
#else
static int aux_in;		/* fd for file "auxiliaryin.txt" */
static int aux_in_lf;		/* linefeed flag for aux_in */
static int aux_out;		/* fd for file "auxiliaryout.txt" */
#endif

#ifdef NETWORKING

#define TELNET_TIMEOUT 800	/* telnet negotiation timeout in milliseconds */

static int ss[NUMSOC];		/* server socket descriptors */
static int ssc[NUMSOC];		/* connected server socket descriptors */
static int ss_port[NUMSOC];	/* TCP/IP port for server sockets */
static int ss_telnet[NUMSOC];	/* telnet protocol flag for server sockets */
static int cs;			/* client socket #1 descriptor */
static int cs_port;		/* TCP/IP port for cs */
static char cs_host[BUFSIZE];	/* hostname for cs */

#ifdef CNETDEBUG
static int cdirection = -1; /* protocol direction, 0 = send, 1 = receive */
#endif

#ifdef SNETDEBUG
static int sdirection = -1; /* protocol direction, 0 = send 1 = receive */
#endif

#endif /* NETWORKING */

dskdef_t disks[16] = {
	{ "drivea.dsk", &drivea, 77, 26 },
	{ "driveb.dsk", &driveb, 77, 26 },
	{ "drivec.dsk", &drivec, 77, 26 },
	{ "drived.dsk", &drived, 77, 26 },
	{ "drivee.dsk", &drivee,  0,  0 },
	{ "drivef.dsk", &drivef,  0,  0 },
	{ "driveg.dsk", &driveg,  0,  0 },
	{ "driveh.dsk", &driveh,  0,  0 },
	{ "drivei.dsk", &drivei, 255, 128 },
	{ "drivej.dsk", &drivej, 255, 128 },
	{ "drivek.dsk", &drivek, 255, 128 },
	{ "drivel.dsk", &drivel, 255, 128 },
	{ "drivem.dsk", &drivem,  0,  0 },
	{ "driven.dsk", &driven,  0,  0 },
	{ "driveo.dsk", &driveo,  0,  0 },
	{ "drivep.dsk", &drivep, 256, 16384 }
};

/*
 *	Forward declaration of the I/O handlers for all used ports
 */
static BYTE cond_in(void), cons_in(void);
static void cond_out(BYTE data), cons_out(BYTE data);
static BYTE prtd_in(void), prts_in(void);
static void prtd_out(BYTE data), prts_out(BYTE data);
static BYTE auxd_in(void), auxs_in(void);
static void auxd_out(BYTE data), auxs_out(BYTE data);
static BYTE fdcd_in(void);
static void fdcd_out(BYTE data);
static BYTE fdct_in(void);
static void fdct_out(BYTE data);
static BYTE fdcs_in(void);
static void fdcs_out(BYTE data);
static BYTE fdcsh_in(void);
static void fdcsh_out(BYTE data);
static BYTE fdco_in(void);
static void fdco_out(BYTE data);
static BYTE fdcx_in(void);
static void fdcx_out(BYTE data);
static BYTE dmal_in(void);
static void dmal_out(BYTE data);
static BYTE dmah_in(void);
static void dmah_out(BYTE data);
static BYTE mmui_in(void), mmus_in(void), mmuc_in(void);
static void mmui_out(BYTE data), mmus_out(BYTE data), mmuc_out(BYTE data);
static BYTE mmup_in(void);
static void mmup_out(BYTE data);
static BYTE time_in(void);
static void time_out(BYTE data);
static BYTE delay_in(void);
static void delay_out(BYTE data);
static BYTE hwctl_in(void);
static void hwctl_out(BYTE data);
static BYTE speedl_in(void), speedh_in(void);
static void speedl_out(BYTE data), speedh_out(BYTE data);
static BYTE cond1_in(void), cons1_in(void);
static void cond1_out(BYTE data), cons1_out(BYTE data);
static BYTE cond2_in(void), cons2_in(void);
static void cond2_out(BYTE data), cons2_out(BYTE data);
static BYTE cond3_in(void), cons3_in(void);
static void cond3_out(BYTE data), cons3_out(BYTE data);
static BYTE cond4_in(void), cons4_in(void);
static void cond4_out(BYTE data), cons4_out(BYTE data);
static BYTE netd1_in(void), nets1_in(void);
static void netd1_out(BYTE data), nets1_out(BYTE data);

/*
 *	Forward declaration of support functions
 */
static void int_timer(int sig);

#ifdef NETWORKING
static void net_server_config(void), net_client_config(void);
static void init_server_socket(int n), telnet_negotiation(int fd);
#ifdef TCPASYNC
static void int_io(int sig);
#endif
#endif

/*
 *	This array contains function pointers for every
 *	input port.
 */
in_func_t *const port_in[256] = {
	[  0] = cons_in,
	[  1] = cond_in,
	[  2] = prts_in,
	[  3] = prtd_in,
	[  4] = auxs_in,
	[  5] = auxd_in,
	[ 10] = fdcd_in,
	[ 11] = fdct_in,
	[ 12] = fdcs_in,
	[ 13] = fdco_in,
	[ 14] = fdcx_in,
	[ 15] = dmal_in,
	[ 16] = dmah_in,
	[ 17] = fdcsh_in,
	[ 20] = mmui_in,
	[ 21] = mmus_in,
	[ 22] = mmuc_in,
	[ 23] = mmup_in,
	[ 25] = clkc_in,
	[ 26] = clkd_in,
	[ 27] = time_in,
	[ 28] = delay_in,
	[ 30] = speedl_in,
	[ 31] = speedh_in,
	[ 40] = cons1_in,
	[ 41] = cond1_in,
	[ 42] = cons2_in,
	[ 43] = cond2_in,
	[ 44] = cons3_in,
	[ 45] = cond3_in,
	[ 46] = cons4_in,
	[ 47] = cond4_in,
	[ 50] = nets1_in,
	[ 51] = netd1_in,
	[160] = hwctl_in	/* virtual hardware control */
};

/*
 *	This array contains function pointers for every
 *	output port.
 */
out_func_t *const port_out[256] = {
	[  0] = cons_out,
	[  1] = cond_out,
	[  2] = prts_out,
	[  3] = prtd_out,
	[  4] = auxs_out,
	[  5] = auxd_out,
	[ 10] = fdcd_out,
	[ 11] = fdct_out,
	[ 12] = fdcs_out,
	[ 13] = fdco_out,
	[ 14] = fdcx_out,
	[ 15] = dmal_out,
	[ 16] = dmah_out,
	[ 17] = fdcsh_out,
	[ 20] = mmui_out,
	[ 21] = mmus_out,
	[ 22] = mmuc_out,
	[ 23] = mmup_out,
	[ 25] = clkc_out,
	[ 26] = clkd_out,
	[ 27] = time_out,
	[ 28] = delay_out,
	[ 30] = speedl_out,
	[ 31] = speedh_out,
	[ 40] = cons1_out,
	[ 41] = cond1_out,
	[ 42] = cons2_out,
	[ 43] = cond2_out,
	[ 44] = cons3_out,
	[ 45] = cond3_out,
	[ 46] = cons4_out,
	[ 47] = cond4_out,
	[ 50] = nets1_out,
	[ 51] = netd1_out,
	[160] = hwctl_out,	/* virtual hardware control */
	[161] = host_bdos_out	/* host file I/O hook */
};

/*
 *	This function initializes the I/O handlers:
 *	1. Creates the named pipes under /tmp/.z80pack, if they don't
 *	   exist.
 *	2. Fork the process for receiving from the auxiliary serial port.
 *	3. Open the named pipes "auxin" and "auxout" for simulation
 *	   of the auxiliary serial port.
 *	4. Open the files which emulate the disk drives.
 *	   Errors for opening one of the drives results
 *	   in a NULL pointer for fd in the dskdef structure,
 *	   so that this drive can't be used.
 *	5. Prepare TCP/IP sockets for serial port simulation
 */
void init_io(void)
{
	register int i;
	struct stat sbuf;
#if defined(NETWORKING) && defined(TCPASYNC)
	static struct sigaction newact;
#endif

#ifdef PIPES
	/* check if /tmp/.z80pack exists */
	if (stat("/tmp/.z80pack", &sbuf) != 0)
		mkdir("/tmp/.z80pack", 0777);	/* no, create it */
	/* and then the pipes */
	if (stat("/tmp/.z80pack/cpmsim.auxin", &sbuf) != 0)
		mkfifo("/tmp/.z80pack/cpmsim.auxin", 0666);
	if (stat("/tmp/.z80pack/cpmsim.auxout", &sbuf) != 0)
		mkfifo("/tmp/.z80pack/cpmsim.auxout", 0666);

	pid_rec = fork();
	switch (pid_rec) {
	case -1:
		LOGE(TAG, "can't fork");
		exit(EXIT_FAILURE);
		break;
	case 0:
		if (access("./srctools/cpmrecv", X_OK) == 0)
			execlp("./srctools/cpmrecv", "cpmrecv", "auxiliaryout.txt",
			       (char *) NULL);
		/* should be in path somewhere */
		else
			execlp("cpmrecv", "cpmrecv", "auxiliaryout.txt",
			       (char *) NULL);
		/* if not cry and die */
		LOGE(TAG, "can't exec cpmrecv process, compile/install the tools dude");
		kill(0, SIGQUIT);
		exit(EXIT_FAILURE);
		break;
	}
	if ((auxin = open("/tmp/.z80pack/cpmsim.auxin", O_RDONLY | O_NONBLOCK)) == -1) {
		LOGE(TAG, "can't open pipe auxin");
		exit(EXIT_FAILURE);
	}
	if ((auxout = open("/tmp/.z80pack/cpmsim.auxout", O_WRONLY)) == -1) {
		LOGE(TAG, "can't open pipe auxout");
		exit(EXIT_FAILURE);
	}
#endif

	for (i = 0; i <= 15; i++) {

		/* if option -d is used disks are there */
		if (diskdir != NULL) {
			strcpy(fn, diskd);
		} else {
			/* if not first try ./disks */
			if ((stat("./disks", &sbuf) == 0) &&
			    S_ISDIR(sbuf.st_mode)) {
				strcpy(fn, "./disks");
			} else {
				/* nope, then DISKSDIR as set in Makefile */
				strcpy(fn, DISKSDIR);
			}
		}

		strcat(fn, "/");
		strcat(fn, disks[i].fn);

		if ((*disks[i].fd = open(fn, O_RDWR)) == -1)
			if ((*disks[i].fd = open(fn, O_RDONLY)) == -1)
				disks[i].fd = NULL;
	}

#ifdef NETWORKING
	net_server_config();
	net_client_config();

#ifdef TCPASYNC
	newact.sa_handler = int_io;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGIO, &newact, NULL);
#endif

	for (i = 0; i < NUMSOC; i++)
		init_server_socket(i);
#endif /* NETWORKING */
}

#ifdef NETWORKING
/*
 * initialize a server socket
 */
static void init_server_socket(int n)
{
	struct sockaddr_in sin;
	int on = 1;
#ifdef TCPASYNC
	int i;
#endif

	if (ss_port[n] == 0)
		return;
	if ((ss[n] = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		LOGE(TAG, "can't create server socket");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(ss[n], SOL_SOCKET, SO_REUSEADDR, (void *) &on,
		       sizeof(on)) == -1) {
		LOGE(TAG, "can't setsockopt SO_REUSEADDR on server socket");
		exit(EXIT_FAILURE);
	}
#ifdef TCPASYNC
	fcntl(ss[n], F_SETOWN, getpid());
	i = fcntl(ss[n], F_GETFL, 0);
	if (fcntl(ss[n], F_SETFL, i | O_ASYNC) == -1) {
		LOGE(TAG, "can't fcntl O_ASYNC on server socket");
		exit(EXIT_FAILURE);
	}
#endif
	memset((void *) &sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(ss_port[n]);
	if (bind(ss[n], (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		LOGE(TAG, "can't bind server socket");
		exit(EXIT_FAILURE);
	}
	if (listen(ss[n], 0) == -1) {
		LOGE(TAG, "can't listen on server socket");
		exit(EXIT_FAILURE);
	}
}

/*
 * Read and process network server configuration file
 */
static void net_server_config(void)
{
	register int i;
	FILE *fp;
	char buf[BUFSIZE];
	char *s;
	char fn[MAX_LFN];

	strcpy(fn, confdir);
	strcat(fn, "/net_server.conf");

	if ((fp = fopen(fn, "r")) != NULL) {
		LOG(TAG, "Server network configuration:\r\n");
		while (fgets(buf, BUFSIZE, fp) != NULL) {
			s = buf;
			if ((*s == '\n') || (*s == '#'))
				continue;
			i = atoi(s);
			if ((i < 1) || (i > 4)) {
				LOGW(TAG, "console %d not supported", i);
				continue;
			}
			while ((*s != ' ') && (*s != '\t'))
				s++;
			while ((*s == ' ') || (*s == '\t'))
				s++;
			ss_telnet[i - 1] = atoi(s);
			while ((*s != ' ') && (*s != '\t'))
				s++;
			while ((*s == ' ') || (*s == '\t'))
				s++;
			ss_port[i - 1] = atoi(s);
			LOG(TAG, "console %d listening on port %d, telnet = %s\r\n",
			    i, ss_port[i - 1],
			    ((ss_telnet[i - 1] > 0) ? "on" : "off"));
		}
		fclose(fp);
	}
}

/*
 * Read and process network client configuration file
 */
static void net_client_config(void)
{
	FILE *fp;
	char buf[BUFSIZE];
	char *s, *d;
	char fn[MAX_LFN];

	strcpy(fn, confdir);
	strcat(fn, "/net_client.conf");

	if ((fp = fopen(fn, "r")) != NULL) {
		LOG(TAG, "Client network configuration:\r\n");
		while (fgets(buf, BUFSIZE, fp) != NULL) {
			s = buf;
			if ((*s == '\n') || (*s == '#'))
				continue;
			while ((*s != ' ') && (*s != '\t'))
				s++;
			while ((*s == ' ') || (*s == '\t'))
				s++;
			d = cs_host;
			while ((*s != ' ') && (*s != '\t'))
				*d++ = *s++;
			*d = '\0';
			while ((*s == ' ') || (*s == '\t'))
				s++;
			cs_port = atoi(s);
			LOG(TAG, "Connecting to %s at port %d\r\n", cs_host,
			    cs_port);
		}
		fclose(fp);
	}
}
#endif /* NETWORKING */

/*
 *	This function stops the I/O handlers:
 *
 *	1. The files emulating the disk drives are closed.
 *	2. The file "printer.txt" emulating a printer is closed.
 *	3. The named pipes "auxin" and "auxout" are closed.
 *	4. The receiving process for the aux serial port is stopped.
 *	5. All connected sockets are closed
 */
void exit_io(void)
{
	register int i;

	for (i = 0; i <= 15; i++)
		if (disks[i].fd != NULL)
			close(*disks[i].fd);

	if (printer != 0)
		close(printer);

#ifdef PIPES
	close(auxin);
	close(auxout);
	kill(pid_rec, SIGHUP);
#endif

#ifdef NETWORKING
	for (i = 0; i < NUMSOC; i++)
		if (ssc[i])
			close(ssc[i]);
	if (cs)
		close(cs);
#endif
}

/*
 *	reset the CPU and I/O system
 */
void reset_system(void)
{
	register int i;

	/* reset hardware */
	time_out(0);			/* stop timer */

	for (i = 1; i < MAXSEG; i++) {	/* reset MMU */
		if (memory[i] != NULL) {
			free(memory[i]);
			memory[i] = NULL;
		}
	}
	selbnk = 0;
	segsize = SEGSIZ;

	/* reset CPU */
	reset_cpu();

	/* reboot */
	boot(1);
}

/*
 *	I/O handler for read console 0 status:
 *	0xff : input available
 *	0x00 : no input available
 */
static BYTE cons_in(void)
{
	struct pollfd p[1];

	if (++busy_loop_cnt >= MAX_BUSY_COUNT) {
		sleep_for_ms(1);
		busy_loop_cnt = 0;
	}

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (p[0].revents & POLLIN)
		return (BYTE) 0xff;
	else
		return (BYTE) 0x00;
}

/*
 *	I/O handler for read console 1 status:
 *	bit 0 = 1: input available
 *	bit 1 = 1: output writable
 */
static BYTE cons1_in(void)
{
	BYTE status = 0;
#ifdef NETWORKING
	struct pollfd p[1];

#ifndef TCPASYNC
	socklen_t alen;
	struct sockaddr_in fsin;
	int go_away;
	int on = 1;

	if (ss[0] == 0)
		return status;

	p[0].fd = ss[0];
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);

	if (p[0].revents) {

		alen = sizeof(fsin);

		if (ssc[0] != 0) {
			go_away = accept(ss[0],
					 (struct sockaddr *) &fsin, &alen);
			close(go_away);
			goto ss0_done;
		}

		if ((ssc[0] = accept(ss[0], (struct sockaddr *) &fsin,
				     &alen)) == -1) {
			LOGW(TAG, "can't accept server socket");
			ssc[0] = 0;
		}

		if (setsockopt(ssc[0], IPPROTO_TCP, TCP_NODELAY,
			       (void *) &on, sizeof(on)) == -1) {
			LOGW(TAG, "can't setsockopt TCP_NODELAY on server socket");
		}

		if (ss_telnet[0])
			telnet_negotiation(ssc[0]);
	}
ss0_done:
#endif /* !TCPASYNC */

	if (ssc[0] != 0) {
		p[0].fd = ssc[0];
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLHUP) {
			close(ssc[0]);
			ssc[0] = 0;
			return 0;
		}
		if (p[0].revents & POLLIN)
			status |= 1;
		if (p[0].revents & POLLOUT)
			status |= 2;
	}
#endif /* NETWORKING */
	return status;
}

/*
 *	I/O handler for read console 2 status:
 *	bit 0 = 1: input available
 *	bit 1 = 1: output writable
 */
static BYTE cons2_in(void)
{
	BYTE status = 0;
#ifdef NETWORKING
	struct pollfd p[1];

#ifndef TCPASYNC
	socklen_t alen;
	struct sockaddr_in fsin;
	int go_away;
	int on = 1;

	if (ss[1] == 0)
		return status;

	p[0].fd = ss[1];
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);

	if (p[0].revents) {

		alen = sizeof(fsin);

		if (ssc[1] != 0) {
			go_away = accept(ss[1],
					 (struct sockaddr *) &fsin, &alen);
			close(go_away);
			goto ss1_done;
		}

		if ((ssc[1] = accept(ss[1], (struct sockaddr *) &fsin,
				     &alen)) == -1) {
			LOGW(TAG, "can't accept server socket");
			ssc[1] = 0;
		}

		if (setsockopt(ssc[1], IPPROTO_TCP, TCP_NODELAY,
			       (void *) &on, sizeof(on)) == -1) {
			LOGW(TAG, "can't setsockopt TCP_NODELAY on server socket");
		}

		if (ss_telnet[1])
			telnet_negotiation(ssc[1]);
	}
ss1_done:
#endif /* !TCPASYNC */

	if (ssc[1] != 0) {
		p[0].fd = ssc[1];
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLHUP) {
			close(ssc[1]);
			ssc[1] = 0;
			return 0;
		}
		if (p[0].revents & POLLIN)
			status |= 1;
		if (p[0].revents & POLLOUT)
			status |= 2;
	}
#endif /* NETWORKING */
	return status;
}

/*
 *	I/O handler for read console 3 status:
 *	bit 0 = 1: input available
 *	bit 1 = 1: output writable
 */
static BYTE cons3_in(void)
{
	BYTE status = 0;
#ifdef NETWORKING
	struct pollfd p[1];

#ifndef TCPASYNC
	socklen_t alen;
	struct sockaddr_in fsin;
	int go_away;
	int on = 1;

	if (ss[2] == 0)
		return status;

	p[0].fd = ss[2];
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);

	if (p[0].revents) {

		alen = sizeof(fsin);

		if (ssc[2] != 0) {
			go_away = accept(ss[2],
					 (struct sockaddr *) &fsin, &alen);
			close(go_away);
			goto ss2_done;
		}

		if ((ssc[2] = accept(ss[2], (struct sockaddr *) &fsin,
				     &alen)) == -1) {
			LOGW(TAG, "can't accept server socket");
			ssc[2] = 0;
		}

		if (setsockopt(ssc[2], IPPROTO_TCP, TCP_NODELAY,
			       (void *) &on, sizeof(on)) == -1) {
			LOGW(TAG, "can't setsockopt TCP_NODELAY on server socket");
		}

		if (ss_telnet[2])
			telnet_negotiation(ssc[2]);
	}
ss2_done:
#endif /* !TCPASYNC */

	if (ssc[2] != 0) {
		p[0].fd = ssc[2];
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLHUP) {
			close(ssc[2]);
			ssc[2] = 0;
			return 0;
		}
		if (p[0].revents & POLLIN)
			status |= 1;
		if (p[0].revents & POLLOUT)
			status |= 2;
	}
#endif /* NETWORKING */
	return status;
}

/*
 *	I/O handler for read console 4 status:
 *	bit 0 = 1: input available
 *	bit 1 = 1: output writable
 */
static BYTE cons4_in(void)
{
	BYTE status = 0;
#ifdef NETWORKING
	struct pollfd p[1];

#ifndef TCPASYNC
	socklen_t alen;
	struct sockaddr_in fsin;
	int go_away;
	int on = 1;

	if (ss[3] == 0)
		return status;

	p[0].fd = ss[3];
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);

	if (p[0].revents) {

		alen = sizeof(fsin);

		if (ssc[3] != 0) {
			go_away = accept(ss[3],
					 (struct sockaddr *) &fsin, &alen);
			close(go_away);
			goto ss3_done;
		}

		if ((ssc[3] = accept(ss[3], (struct sockaddr *) &fsin,
				     &alen)) == -1) {
			LOGW(TAG, "can't accept server socket");
			ssc[3] = 0;
		}

		if (setsockopt(ssc[3], IPPROTO_TCP, TCP_NODELAY,
			       (void *) &on, sizeof(on)) == -1) {
			LOGW(TAG, "can't setsockopt TCP_NODELAY on server socket");
		}

		if (ss_telnet[3])
			telnet_negotiation(ssc[3]);
	}
ss3_done:
#endif /* !TCPASYNC */

	if (ssc[3] != 0) {
		p[0].fd = ssc[3];
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLHUP) {
			close(ssc[3]);
			ssc[3] = 0;
			return 0;
		}
		if (p[0].revents & POLLIN)
			status |= 1;
		if (p[0].revents & POLLOUT)
			status |= 2;
	}
#endif /* NETWORKING */
	return status;
}

/*
 *	I/O handler for read client socket 1 status:
 *	bit 0 = 1: input available
 *	bit 1 = 1: output writable
 */
static BYTE nets1_in(void)
{
	BYTE status = 0;
#ifdef NETWORKING
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	struct pollfd p[1];
	int on = 1, s;
	char service[6];

	if ((cs == 0) && (cs_port != 0)) {
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;	/* Allow only IPv4 not IPv6 */
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;
		hints.ai_protocol = 0;		/* Any protocol */
		snprintf(service, sizeof(service), "%d", cs_port);
		if ((s = getaddrinfo(cs_host, service, &hints, &result)) != 0) {
			LOGE(TAG, "getaddrinfo failed: %s", gai_strerror(s));
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
			return (BYTE) 0;
		}

		for (rp = result; rp != NULL; rp = rp->ai_next) {
			if ((cs = socket(rp->ai_family, rp->ai_socktype,
					 rp->ai_protocol)) == -1)
				continue;

			if (connect(cs, rp->ai_addr, rp->ai_addrlen) != -1)
				break;

			close(cs);
		}
		freeaddrinfo(result);

		if (rp == NULL) {
			LOGE(TAG, "can't connect to host %s", cs_host);
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
			return (BYTE) 0;
		}

		if (setsockopt(cs, IPPROTO_TCP, TCP_NODELAY,
			       (void *) &on, sizeof(on)) == -1) {
			LOGW(TAG,
			     "can't setsockopt TCP_NODELAY on client socket");
		}
	}

	if (cs != 0) {
		p[0].fd = cs;
		p[0].events = POLLIN | POLLOUT;
		p[0].revents = 0;
		poll(p, 1, 0);
		if (p[0].revents & POLLHUP) {
			close(cs);
			cs = 0;
			return (BYTE) 0;
		}
		if (p[0].revents & POLLIN)
			status |= 1;
		if (p[0].revents & POLLOUT)
			status |= 2;
	}
#endif /* NETWORKING */
	return status;
}

/*
 *	I/O handler for write console 0 status:
 *	no function
 */
static void cons_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	I/O handler for write console 1 status:
 *	no function
 */
static void cons1_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	I/O handler for write console 2 status:
 *	no function
 */
static void cons2_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	I/O handler for write console 3 status:
 *	no function
 */
static void cons3_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	I/O handler for write console 4 status:
 *	no function
 */
static void cons4_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	I/O handler for write client socket 1 status:
 *	no function
 */
static void nets1_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	I/O handler for read console 0 data
 */
static BYTE cond_in(void)
{
	char c;

	busy_loop_cnt = 0;
	if (read(fileno(stdin), &c, 1) != 1)
		LOGE(TAG, "can't read console 0");
	return (BYTE) c;
}

/*
 *	I/O handler for read console 1 data
 */
static BYTE cond1_in(void)
{
	char c;
#ifdef NETWORKING
	char x;

	if (read(ssc[0], &c, 1) != 1) {
		if ((errno == EAGAIN) || (errno == EINTR)) {
			close(ssc[0]);
			ssc[0] = 0;
			return (BYTE) 0;
		} else {
			LOGE(TAG, "can't read console 1");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
			return (BYTE) 0;
		}
	}
	if (ss_telnet[0] && (c == '\r'))
		if (read(ssc[0], &x, 1) != 1)
			LOGE(TAG, "can't read console 1");
#ifdef SNETDEBUG
	if (sdirection != 1) {
		printf("\n<- ");
		sdirection = 1;
	}
	printf("%02x ", (BYTE) c);
#endif
#else /* !NETWORKING */
	c = 0;
#endif
	return (BYTE) c;
}

/*
 *	I/O handler for read console 2 data
 */
static BYTE cond2_in(void)
{
	char c;
#ifdef NETWORKING
	char x;

	if (read(ssc[1], &c, 1) != 1) {
		if ((errno == EAGAIN) || (errno == EINTR)) {
			close(ssc[1]);
			ssc[1] = 0;
			return (BYTE) 0;
		} else {
			LOGE(TAG, "can't read console 2");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
			return (BYTE) 0;
		}
	}
	if (ss_telnet[1] && (c == '\r'))
		if (read(ssc[1], &x, 1) != 1)
			LOGE(TAG, "can't read console 2");
#ifdef SNETDEBUG
	if (sdirection != 1) {
		printf("\n<- ");
		sdirection = 1;
	}
	printf("%02x ", (BYTE) c);
#endif
#else /* !NETWORKING */
	c = 0;
#endif
	return (BYTE) c;
}

/*
 *	I/O handler for read console 3 data
 */
static BYTE cond3_in(void)
{
	char c;
#ifdef NETWORKING
	char x;

	if (read(ssc[2], &c, 1) != 1) {
		if ((errno == EAGAIN) || (errno == EINTR)) {
			close(ssc[2]);
			ssc[2] = 0;
			return (BYTE) 0;
		} else {
			LOGE(TAG, "can't read console 3");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
			return (BYTE) 0;
		}
	}
	if (ss_telnet[2] && (c == '\r'))
		if (read(ssc[2], &x, 1) != 1)
			LOGE(TAG, "can't read console 3");
#ifdef SNETDEBUG
	if (sdirection != 1) {
		printf("\n<- ");
		sdirection = 1;
	}
	printf("%02x ", (BYTE) c);
#endif
#else /* !NETWORKING */
	c = 0;
#endif
	return (BYTE) c;
}

/*
 *	I/O handler for read console 4 data
 */
static BYTE cond4_in(void)
{
	char c;
#ifdef NETWORKING
	char x;

	if (read(ssc[3], &c, 1) != 1) {
		if ((errno == EAGAIN) || (errno == EINTR)) {
			close(ssc[3]);
			ssc[3] = 0;
			return (BYTE) 0;
		} else {
			LOGE(TAG, "can't read console 4");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
			return (BYTE) 0;
		}
	}
	if (ss_telnet[3] && (c == '\r'))
		if (read(ssc[3], &x, 1) != 1)
			LOGE(TAG, "can't read console 4");
#ifdef SNETDEBUG
	if (sdirection != 1) {
		printf("\n<- ");
		sdirection = 1;
	}
	printf("%02x ", (BYTE) c);
#endif
#else /* !NETWORKING */
	c = 0;
#endif
	return (BYTE) c;
}

/*
 *	I/O handler for read client socket 1 data
 */
static BYTE netd1_in(void)
{
	char c;

#ifdef NETWORKING
	if (read(cs, &c, 1) != 1) {
		LOGE(TAG, "can't read client socket");
		cpu_error = IOERROR;
		cpu_state = ST_STOPPED;
		return (BYTE) 0;
	}
#ifdef CNETDEBUG
	if (cdirection != 1) {
		printf("\n<- ");
		cdirection = 1;
	}
	printf("%02x ", (BYTE) c);
#endif
#else /* !NETWORKING */
	c = 0;
#endif
	return (BYTE) c;
}

/*
 *	I/O handler for write console 0 data:
 *	the output is written to the terminal
 */
static void cond_out(BYTE data)
{
again:
	if (write(fileno(stdout), (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			LOGE(TAG, "can't write console 0");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
		}
	}
}

/*
 *	I/O handler for write console 1 data:
 *	the output is written to the socket
 */
static void cond1_out(BYTE data)
{
#ifdef NETWORKING
#ifdef SNETDEBUG
	if (sdirection != 0) {
		printf("\n-> ");
		sdirection = 0;
	}
	printf("%02x ", (BYTE) data);
#endif
again:
	if (write(ssc[0], (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			LOGE(TAG, "can't write console 1");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
		}
	}
#else /* !NETWORKING */
	UNUSED(data);
#endif
}

/*
 *	I/O handler for write console 2 data:
 *	the output is written to the socket
 */
static void cond2_out(BYTE data)
{
#ifdef NETWORKING
#ifdef SNETDEBUG
	if (sdirection != 0) {
		printf("\n-> ");
		sdirection = 0;
	}
	printf("%02x ", (BYTE) data);
#endif
again:
	if (write(ssc[1], (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			LOGE(TAG, "can't write console 2");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
		}
	}
#else /* !NETWORKING */
	UNUSED(data);
#endif
}

/*
 *	I/O handler for write console 3 data:
 *	the output is written to the socket
 */
static void cond3_out(BYTE data)
{
#ifdef NETWORKING
#ifdef SNETDEBUG
	if (sdirection != 0) {
		printf("\n-> ");
		sdirection = 0;
	}
	printf("%02x ", (BYTE) data);
#endif
again:
	if (write(ssc[2], (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			LOGE(TAG, "can't write console 3");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
		}
	}
#else /* !NETWORKING */
	UNUSED(data);
#endif
}

/*
 *	I/O handler for write console 4 data:
 *	the output is written to the socket
 */
static void cond4_out(BYTE data)
{
#ifdef NETWORKING
#ifdef SNETDEBUG
	if (sdirection != 0) {
		printf("\n-> ");
		sdirection = 0;
	}
	printf("%02x ", (BYTE) data);
#endif
again:
	if (write(ssc[3], (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			LOGE(TAG, "can't write console 4");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
		}
	}
#else /* !NETWORKING */
	UNUSED(data);
#endif
}

/*
 *	I/O handler for write client socket 1 data:
 *	the output is written to the socket
 */
static void netd1_out(BYTE data)
{
#ifdef NETWORKING
#ifdef CNETDEBUG
	if (cdirection != 0) {
		printf("\n-> ");
		cdirection = 0;
	}
	printf("%02x ", (BYTE) data);
#endif
again:
	if (write(cs, (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			LOGE(TAG, "can't write client socket");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
		}
	}
#else /* !NETWORKING */
	UNUSED(data);
#endif
}

/*
 *	I/O handler for read printer status:
 *	the printer is ready all the time
 */
static BYTE prts_in(void)
{
	return (BYTE) 0xff;
}

/*
 *	I/O handler for write printer status:
 *	no function
 */
static void prts_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	I/O handler for read printer data:
 *	always read an EOF from the printer
 */
static BYTE prtd_in(void)
{
	return (BYTE) 0x1a;	/* CP/M EOF */
}

/*
 *	I/O handler for write printer data:
 *	the output is written to file "printer.txt"
 */
static void prtd_out(BYTE data)
{
	if (printer == 0) {
		if ((printer = creat("printer.txt", 0664)) == -1) {
			LOGE(TAG, "can't create printer.txt");
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
			printer = 0;
			return;
		}
	}

	if (data != '\r') {
again:
		if (write(printer, (char *) &data, 1) != 1) {
			if (errno == EINTR) {
				goto again;
			} else {
				LOGE(TAG, "can't write to printer.txt");
				cpu_error = IOERROR;
				cpu_state = ST_STOPPED;
			}
		}
	}
}

/*
 *	I/O handler for read aux status:
 *	return EOF status of the aux device
 */
static BYTE auxs_in(void)
{
#ifdef PIPES
	return (BYTE) aux_in_eof;
#else
	return (BYTE) 0xff;
#endif
}

/*
 *	I/O handler for write aux status:
 *	change EOF status of the aux device
 */
static void auxs_out(BYTE data)
{
#ifdef PIPES
	aux_in_eof = data;
#else
	UNUSED(data);
#endif
}

/*
 *	I/O handler for read aux data:
 *	read next byte from pipe "auxin" or from file "auxiliaryin.txt"
 */
static BYTE auxd_in(void)
{
	char c;

#ifdef PIPES
	if (read(auxin, &c, 1) == 1)
		return (BYTE) c;
	else {
		aux_in_eof = 0xff;
		return (BYTE) 0x1a;	/* CP/M EOF */
	}
#else
	if (aux_in == 0) {
		if ((aux_in = open("auxiliaryin.txt", O_RDONLY)) == -1) {
			LOGE(TAG, "can't open auxiliaryin.txt");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			return (BYTE) 0;
		}
	}

	if (aux_in_lf) {
		aux_in_lf = 0;
		return (BYTE) '\n';
	}

	if (read(aux_in, &c, 1) != 1) {
		close(aux_in);
		aux_in = 0;
		return (BYTE) 0x1a;
	}

	if (c == '\n') {
		aux_in_lf = 1;
		return (BYTE) '\r';
	}

	return (BYTE) c;
#endif
}

/*
 *	I/O handler for write aux data:
 *	write output to pipe "auxout" or to file "auxiliaryout.txt"
 */
static void auxd_out(BYTE data)
{
#ifdef PIPES
	if ((data == 0) || (data == 0x1a))
		return;

	if (data != '\r')
		if (write(auxout, (char *) &data, 1) != 1)
			LOGE(TAG, "can't write to auxout pipe");
#else
	if (data == 0)
		return;

	if (aux_out == 0) {
		if ((aux_out = creat("auxiliaryout.txt", 0644)) == -1) {
			LOGE(TAG, "can't open auxiliaryout.txt");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
			return;
		}
	}

	if (data == 0x1a) {
		close(aux_out);
		aux_out = 0;
		return;
	}

	if (data != '\r')
		if (write(aux_out, (char *) &data, 1) != 1)
			LOGE(TAG, "can't write to auxiliaryout.txt");
#endif
}

/*
 *	I/O handler for read FDC drive:
 *	return the current drive
 */
static BYTE fdcd_in(void)
{
	return (BYTE) drive;
}

/*
 *	I/O handler for write FDC drive:
 *	set the current drive
 */
static void fdcd_out(BYTE data)
{
	drive = data;
}

/*
 *	I/O handler for read FDC track:
 *	return the current track
 */
static BYTE fdct_in(void)
{
	return (BYTE) track;
}

/*
 *	I/O handler for write FDC track:
 *	set the current track
 */
static void fdct_out(BYTE data)
{
	track = data;
}

/*
 *	I/O handler for read FDC sector low
 *	return low byte of the current sector
 */
static BYTE fdcs_in(void)
{
	return (BYTE) sector & 0xff;
}

/*
 *	I/O handler for write FDC sector low
 *	set low byte of the current sector
 */
static void fdcs_out(BYTE data)
{
	sector = (sector & 0xff00) + data;
}

/*
 *	I/O handler for read FDC sector high
 *	return high byte of the current sector
 */
static BYTE fdcsh_in(void)
{
	return (BYTE) (sector >> 8);
}

/*
 *	I/O handler for write FDC sector high
 *	set high byte of the current sector
 */
static void fdcsh_out(BYTE data)
{
	sector = (sector & 0xff) + (data << 8);
}

/*
 *	I/O handler for read FDC command:
 *	always returns 0
 */
static BYTE fdco_in(void)
{
	return (BYTE) 0;
}

/*
 *	I/O handler for write FDC command:
 *	transfer one sector in the wanted direction,
 *	0 = read, 1 = write
 *
 *	The status byte of the FDC is set as follows:
 *	  0 - ok
 *	  1 - invalid drive
 *	  2 - invalid track
 *	  3 - invalid sector
 *	  4 - seek error
 *	  5 - read error
 *	  6 - write error
 *	  7 - invalid command to FDC
 */
static void fdco_out(BYTE data)
{
	register int i;
	off_t pos;
	static char buf[128];

	if (disks[drive].fd == NULL) {
		status = 1;
		return;
	}
	if (track > disks[drive].tracks) {
		status = 2;
		return;
	}
	if (sector > disks[drive].sectors) {
		status = 3;
		return;
	}
	pos = (((off_t) track) * ((off_t) disks[drive].sectors) + sector - 1) << 7;
	if (lseek(*disks[drive].fd, pos, SEEK_SET) == -1L) {
		status = 4;
		return;
	}
	switch (data) {
	case 0:	/* read */
		if (read(*disks[drive].fd, buf, 128) != 128)
			status = 5;
		else {
			for (i = 0; i < 128; i++)
				dma_write((dmadh << 8) + dmadl + i, buf[i]);
			status = 0;
		}
		break;
	case 1:	/* write */
		for (i = 0; i < 128; i++)
			buf[i] = dma_read((dmadh << 8) + dmadl + i);
		if (write(*disks[drive].fd, buf, 128) != 128)
			status = 6;
		else
			status = 0;
		break;
	default:		/* invalid command */
		status = 7;
		break;
	}
}

/*
 *	I/O handler for read FDC status:
 *	returns status of last FDC operation,
 *	0 = ok, else some error
 */
static BYTE fdcx_in(void)
{
	return (BYTE) status;
}

/*
 *	I/O handler for write FDC status:
 *	no function
 */
static void fdcx_out(BYTE data)
{
	UNUSED(data);
}

/*
 *	I/O handler for read lower byte of DMA address:
 *	return lower byte of current DMA address
 */
static BYTE dmal_in(void)
{
	return (BYTE) dmadl;
}

/*
 *	I/O handler for write lower byte of DMA address:
 *	set lower byte of DMA address
 */
static void dmal_out(BYTE data)
{
	dmadl = data;
}

/*
 *	I/O handler for read higher byte of DMA address:
 *	return higher byte of current DMA address
 */
static BYTE dmah_in(void)
{
	return (BYTE) dmadh;
}

/*
 *	I/O handler for write higher byte of DMA address:
 *	set higher byte of the DMA address
 */
static void dmah_out(BYTE data)
{
	dmadh = data;
}

/*
 *	I/O handler for read MMU initialization:
 *	return number of initialized MMU banks
 */
static BYTE mmui_in(void)
{
	return (BYTE) maxbnk;
}

/*
 *	I/O handler for write MMU initialization:
 *	for the FIRST call the memory for the wanted number of banks
 *	is allocated and pointers to the memory is stored in the MMU array
 *
 *	The number of banks is the total, including bank 0 which already
 *	is allocated
 */
static void mmui_out(BYTE data)
{
	register int i;

	/* do nothing if MMU initialized already */
	if (memory[1] != NULL)
		return;

	if (data > MAXSEG) {
		LOGE(TAG, "Try to init %d banks, available %d banks",
		     data, MAXSEG);
		cpu_error = IOERROR;
		cpu_state = ST_STOPPED;
		return;
	}

	for (i = 1; i < data; i++) {
		if ((memory[i] = (BYTE *) malloc(segsize)) == NULL) {
			LOGE(TAG, "can't allocate memory for bank %d", i);
			cpu_error = IOERROR;
			cpu_state = ST_STOPPED;
			return;
		}
	}

	maxbnk = data;
}

/*
 *	I/O handler for read MMU bank select:
 *	return current selected MMU bank
 */
static BYTE mmus_in(void)
{
	return (BYTE) selbnk;
}

/*
 *	I/O handler for write MMU bank select:
 *	make the new bank the current one
 */
static void mmus_out(BYTE data)
{
	if (data > maxbnk - 1) {
		LOGE(TAG, "%04x: try to select unallocated bank %d", PC, data);
		cpu_error = IOERROR;
		cpu_state = ST_STOPPED;
		return;
	}
	selbnk = data;
}

/*
 *	I/O handler for read MMU segment size configuration:
 *	returns size of the bank segments in pages a 256 bytes
 */
static BYTE mmuc_in(void)
{
	return (BYTE) (segsize >> 8);
}

/*
 *	I/O handler for write MMU segment size configuration:
 *	set the size of the bank segments in pages a 256 bytes
 *	must be done before any banks are allocated
 */
static void mmuc_out(BYTE data)
{
	if (memory[1] != NULL) {
		LOGE(TAG, "Not possible to resize already allocated segments");
		cpu_error = IOERROR;
		cpu_state = ST_STOPPED;
		return;
	}
	segsize = data << 8;
}

/*
 *	I/O handler to read status of MMU protect/unprotect common segment
 */
static BYTE mmup_in(void)
{
	return wp_common;
}

/*
 *	I/O handler for MMU protect/unprotect common segment
 */
static void mmup_out(BYTE data)
{
	wp_common = data;
}

/*
 *	I/O handler for write timer
 *	start or stop the 10ms interrupt timer
 */
static void time_out(BYTE data)
{
	static struct itimerval tim;
	static struct sigaction newact;

	if (data == 1) {
		timer = 1;
		newact.sa_handler = int_timer;
		sigemptyset(&newact.sa_mask);
		newact.sa_flags = 0;
		sigaction(SIGALRM, &newact, NULL);
		tim.it_value.tv_sec = 0;
		tim.it_value.tv_usec = 10000;
		tim.it_interval.tv_sec = 0;
		tim.it_interval.tv_usec = 10000;
		setitimer(ITIMER_REAL, &tim, NULL);
	} else {
		timer = 0;
		newact.sa_handler = SIG_IGN;
		sigemptyset(&newact.sa_mask);
		newact.sa_flags = 0;
		sigaction(SIGALRM, &newact, NULL);
		tim.it_value.tv_sec = 0;
		tim.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &tim, NULL);
	}
}

/*
 *	I/O handler for read timer
 *	return current status of 10ms interrupt timer,
 *	1 = enabled, 0 = disabled
 */
static BYTE time_in(void)
{
	return timer;
}

/*
 *	I/O handler for write delay
 *	delay CPU for data * 10ms
 */
static void delay_out(BYTE data)
{
	sleep_for_ms(data * 10);

#ifdef CNETDEBUG
	printf(". ");
#endif
}

/*
 *	I/O handler for read delay
 *	returns 0
 */
static BYTE delay_in(void)
{
	return (BYTE) 0;
}

/*
 *	Port is locked until magic number 0xaa is received!
 *
 *	I/O handler for write hardware control after unlocking:
 *
 *	bit 4 = 1	switch CPU model to 8080
 *	bit 5 = 1	switch CPU model to Z80
 *	bit 6 = 1	reset CPU, MMU and reboot
 *	bit 7 = 1	halt emulation via I/O
 */
static void hwctl_out(BYTE data)
{
	/* if port is locked do nothing */
	if (hwctl_lock && (data != 0xaa))
		return;

	/* unlock port ? */
	if (hwctl_lock && (data == 0xaa)) {
		hwctl_lock = 0;
		return;
	}

	/* process output to unlocked port */
	/* but first lock port again */
	hwctl_lock = 0xff;

	if (data & 128) {	/* halt system */
		cpu_error = IOHALT;
		cpu_state = ST_STOPPED;
		return;
	}

	if (data & 64) {	/* reset system */
		reset_system();
		return;
	}

#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
	if (data & 32) {	/* switch cpu model to Z80 */
		switch_cpu(Z80);
		return;
	}

	if (data & 16) {	/* switch cpu model to 8080 */
		switch_cpu(I8080);
		return;
	}
#endif
}

/*
 *	I/O handler for read hardware control
 *	returns lock status of the port
 */
static BYTE hwctl_in(void)
{
	return hwctl_lock;
}

/*
 *	I/O handler for write CPU speed low
 */
static void speedl_out(BYTE data)
{
	speed = data;
}

/*
 *	I/O handler for read CPU speed low
 */
static BYTE speedl_in(void)
{
	return f_flag & 0xff;
}

/*
 *	I/O handler for write CPU speed high
 */
static void speedh_out(BYTE data)
{
	speed += data << 8;
	tmax = speed * 10000;
	f_flag = speed;
}

/*
 *	I/O handler for read CPU speed high
 */
static BYTE speedh_in(void)
{
	return f_flag >> 8;
}

/*
 *	timer interrupt causes maskable CPU interrupt
 */
static void int_timer(int sig)
{
	UNUSED(sig);

	int_int = true;
	int_data = 0xff;	/* RST 38H for IM 0, 0FFH for IM 2 */
}

#if defined(NETWORKING) && defined(TCPASYNC)
/*
 *	SIGIO interrupt handler
 */
static void int_io(int sig)
{
	register int i;
	struct sockaddr_in fsin;
	socklen_t alen;
	struct pollfd p[NUMSOC];
	int go_away;
	int on = 1;

	UNUSED(sig);

	for (i = 0; i < NUMSOC; i++) {
		p[i].fd = ss[i];
		p[i].events = POLLIN;
		p[i].revents = 0;
	}

	poll(p, NUMSOC, 0);

	for (i = 0; i < NUMSOC; i++) {
		if ((ss[i] != 0) && (p[i].revents)) {
			alen = sizeof(fsin);

			if (ssc[i] != 0) {
				go_away = accept(ss[i],
						 (struct sockaddr *) &fsin,
						 &alen);
				close(go_away);
				return;
			}

			if ((ssc[i] = accept(ss[i], (struct sockaddr *) &fsin,
					     &alen)) == -1) {
				LOGW(TAG, "can't accept on server socket");
				ssc[i] = 0;
			}

			if (setsockopt(ssc[i], IPPROTO_TCP, TCP_NODELAY,
				       (void *) &on, sizeof(on)) == -1) {
				LOGW(TAG, "can't setsockopt TCP_NODELAY on server socket");
			}

			if (ss_telnet[i])
				telnet_negotiation(ssc[i]);
		}
	}
}
#endif /* NETWORKING && TCPASYNC */

#ifdef NETWORKING
/*
 *	do the telnet option negotiation
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
#endif /* NETWORKING */
