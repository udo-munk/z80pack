/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2022 Udo Munk
 * Copyright (C) 2021 David McNaughton
 * Copyright (C) 2022 Thomas Eberhardt
 *
 * History:
 * 28-SEP-87 Development on TARGON/35 with AT&T Unix System V.3
 * 11-JAN-89 Release 1.1
 * 08-FEB-89 Release 1.2
 * 13-MAR-89 Release 1.3
 * 09-FEB-90 Release 1.4  Ported to TARGON/31 M10/30
 * 20-DEC-90 Release 1.5  Ported to COHERENT 3.0
 * 10-JUN-92 Release 1.6  long casting problem solved with COHERENT 3.2
 *			  and some optimisation
 * 25-JUN-92 Release 1.7  comments in english and ported to COHERENT 4.0
 * 02-OCT-06 Release 1.8  modified to compile on modern POSIX OS's
 * 18-NOV-06 Release 1.9  modified to work with CP/M sources
 * 08-DEC-06 Release 1.10 modified MMU for working with CP/NET
 * 17-DEC-06 Release 1.11 TCP/IP sockets for CP/NET
 * 25-DEC-06 Release 1.12 CPU speed option
 * 19-FEB-07 Release 1.13 various improvements
 * 06-OCT-07 Release 1.14 bug fixes and improvements
 * 06-AUG-08 Release 1.15 many improvements and Windows support via Cygwin
 * 25-AUG-08 Release 1.16 console status I/O loop detection and line discipline
 * 20-OCT-08 Release 1.17 frontpanel integrated and Altair/IMSAI emulations
 * 24-JAN-14 Release 1.18 bug fixes and improvements
 * 02-MAR-14 Release 1.19 source cleanup and improvements
 * 14-MAR-14 Release 1.20 added Tarbell SD FDC and printer port to Altair
 * 29-MAR-14 Release 1.21 many improvements
 * 29-MAY-14 Release 1.22 improved networking and bugfixes
 * 04-JUN-14 Release 1.23 added 8080 emulation
 * 06-SEP-14 Release 1.24 bugfixes and improvements
 * 18-FEB-15 Release 1.25 bugfixes, improvements, added Cromemco Z-1
 * 18-APR-15 Release 1.26 bugfixes and improvements
 * 18-JAN-16 Release 1.27 bugfixes and improvements
 * 05-MAY-16 Release 1.28 improved usability
 * 20-NOV-16 Release 1.29 bugfixes and improvements
 * 15-DEC-16 Release 1.30 improved memory management, machine cycle correct CPUs
 * 28-DEC-16 Release 1.31 improved memory management, reimplemented MMUs
 * 12-JAN-17 Release 1.32 improved configurations, front panel, added IMSAI VIO
 * 07-FEB-17 Release 1.33 bugfixes, improvements, better front panels
 * 16-MAR-17 Release 1.34 improvements, added ProcTec VDM-1
 * 03-AUG-17 Release 1.35 added UNIX sockets, bugfixes, improvements
 * 21-DEC-17 Release 1.36 bugfixes and improvements
 * 06-JAN-21 Release 1.37 bugfixes and improvements
 */

/*
 *	This module contains the 'main()' function of the simulator,
 *	where the options are checked and variables are initialised.
 *	After initialisation of the UNIX interrupts ( int_on() )
 *	and initialisation of the I/O simulation ( init_io() )
 *	the user interface ( mon() ) is called.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif
#include "memory.h"

static void init_cpu(void);
static void save_core(void);
int load_core(void);
extern int load_file(char *, BYTE, WORD);
extern void int_on(void), int_off(void), mon(void);
extern void init_io(void), exit_io(void);
extern int exatoi(char *);

int main(int argc, char *argv[])
{
	register char *s, *p;
	char *pn = basename(argv[0]);
	struct timeval tv;
#ifdef HAS_CONFIG
	struct stat sbuf;
	const char *rom = "-r rompath ";
#else
#ifdef BOOTROM
	const char *rom = "-r ";
#else
	const char *rom = "";
#endif
#endif
#ifdef CPU_SPEED
	f_flag = CPU_SPEED;
	tmax = CPU_SPEED * 10000;
#endif

	while (--argc > 0 && (*++argv)[0] == '-')
		for (s = argv[0] + 1; *s != '\0'; s++)

			switch (*s) {
			case 's':	/* save core and CPU on exit */
				s_flag = 1;
				break;

			case 'l':	/* load core and CPU from file */
				l_flag = 1;
				break;

			case 'u':	/* trap undocumented ops */
				u_flag = 1;
				break;

			case 'i':	/* trap I/O on unused ports */
				i_flag = 1;
				break;

			case 'm':	/* initialise memory */
				if (*(s+1) != '\0') {
					m_flag = exatoi(s+1);
					s += strlen(s+1);
				} else {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					m_flag = exatoi(argv[0]);
				}
				break;

			case 'f':	/* set emulation speed */
				if (*(s+1) != '\0') {
					f_flag = atoi(s+1);
					s += strlen(s+1);
				} else {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					f_flag = atoi(argv[0]);
				}
				tmax = f_flag * 10000;
				break;

			case 'x':	/* get filename with executable */
				x_flag = 1;
				s++;
				if (*s == '\0') {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					s = argv[0];
				}
				p = xfn;
				while (*s)
					*p++ = *s++;
				*p = '\0';
				s--;
				break;

#ifdef HAS_CONFIG
			case 'r':	/* get path for boot ROM images */
				s++;
				if (*s == '\0') {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					s = argv[0];
				}
				p = rompath;
				while (*s)
					*p++ = *s++;
				*p = '\0';
				s--;
				break;
#else
#ifdef BOOTROM
			case 'r':	/* load default boot ROM */
				r_flag = 1;
				x_flag = 1;
				strcpy(xfn, BOOTROM);
#endif
#endif

#ifdef HAS_DISKS
			case 'd':	/* get path for disk images */
				s++;
				if (*s == '\0') {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					s = argv[0];
				}
				p = diskdir = diskd;
				while (*s)
					*p++ = *s++;
				*p = '\0';
				s--;
				break;
#endif

#ifdef HAS_CONFIG
			case 'c':	/* get config file */
				c_flag = 1;
				s++;
				if (*s == '\0') {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					s = argv[0];
				}
				p = &conffn[0];
				while (*s)
					*p++ = *s++;
				*p = '\0';
				s--;
				break;

			case 'M': /* use memory map section nn */
				if (*(s+1) != '\0') {
					M_flag = atoi(s+1) - 1;
					s += strlen(s+1);
				} else {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					M_flag = atoi(argv[0]) - 1;
				}
				if (M_flag < 0 || M_flag > (MAXMEMSECT - 1))
						goto usage;
				break;
#endif

#ifdef HAS_BANKED_ROM
			case 'R':	/* enable banked ROM for machines
					   that implement it */
				R_flag = 1;
				break;
#endif

			case '8':
				cpu = I8080;
				break;

			case 'z':
				cpu = Z80;
				break;

			case '?':
			case 'h':
				goto usage;

			default:
				printf("invalid option %c\n", *s);

usage:

				printf("usage:\t%s -z -8 -s -l -i -u %s-m val "
				       "-f freq -x filename", pn, rom);
#ifdef HAS_DISKS
				printf(" -d diskpath");
#endif
#ifdef HAS_CONFIG
				printf(" -c filename");
#if MAXMEMSECT > 0
				printf(" -M val");
#endif
#endif
#ifdef HAS_BANKED_ROM
				printf(" -R");
#endif
				putchar('\n');
				puts("\t-z = emulate Zilog Z80");
				puts("\t-8 = emulate Intel 8080");
				puts("\t-s = save core and CPU");
				puts("\t-l = load core and CPU");
				puts("\t-i = trap on I/O to unused ports");
				puts("\t-u = trap on undocumented instructions");
#ifdef HAS_CONFIG
				puts("\t-r = use ROM images at rompath");
				puts("\t     default path for ROM images:");
				puts("\t     ./roms");
				printf("\t     %s\n", BOOTROM);
#else
#ifdef BOOTROM
				puts("\t-r = load and execute default ROM");
				printf("\t     %s\n", BOOTROM);
#endif
#endif
				puts("\t-m = init memory with val (00-FF)");
				puts("\t-f = CPU clock frequency freq in MHz");
				puts("\t-x = load and execute filename");
#ifdef HAS_DISKS
				puts("\t-d = use disk images at diskpath");
				puts("\t     default path for disk images:");
				puts("\t     ./disks");
				printf("\t     %s\n", DISKSDIR);
#endif
#ifdef HAS_CONFIG
				puts("\t-c = use config filename");
				puts("\t     default config files:");
				puts("\t     ./conf/system.conf");
				printf("\t     %s/system.conf\n", CONFDIR);
#if MAXMEMSECT > 0
				printf("\t-M = use config file memory "
				       "section val (1-%d)\n", MAXMEMSECT);
#endif
#endif
#ifdef HAS_BANKED_ROM
				puts("\t-R = enable banked ROM");
#endif
				exit(1);
			}

	putchar('\n');

	if (cpu == Z80) {
puts("#######  #####    ###            #####    ###   #     #");
puts("     #  #     #  #   #          #     #    #    ##   ##");
puts("    #   #     # #     #         #          #    # # # #");
puts("   #     #####  #     #  #####   #####     #    #  #  #");
puts("  #     #     # #     #               #    #    #     #");
puts(" #      #     #  #   #          #     #    #    #     #");
puts("#######  #####    ###            #####    ###   #     #");

	} else {

puts(" #####    ###     #####    ###            #####    ###   #     #");
puts("#     #  #   #   #     #  #   #          #     #    #    ##   ##");
puts("#     # #     #  #     # #     #         #          #    # # # #");
puts(" #####  #     #   #####  #     #  #####   #####     #    #  #  #");
puts("#     # #     #  #     # #     #               #    #    #     #");
puts("#     #  #   #   #     #  #   #          #     #    #    #     #");
puts(" #####    ###     #####    ###            #####    ###   #     #");
	}

	printf("\nRelease %s, %s\n", RELEASE, COPYR);

#ifdef USR_COM
	printf("%s Release %s, %s\n\n", USR_COM, USR_REL, USR_CPR);
#else
	putchar('\n');
#endif

	if (f_flag > 0)
		printf("CPU speed is %d MHz", f_flag);
	else
		printf("CPU speed is unlimited");

	if (cpu == Z80) {
#ifndef Z80_UNDOC
		printf(", CPU doesn't execute undocumented instructions\n");
#else
		if (u_flag)
			printf(", CPU doesn't execute undocumented "
			       "instructions\n");
		else
			printf(", CPU executes undocumented instructions\n");
#endif
	} else {
		if (u_flag)
			printf(", CPU doesn't execute undocumented "
			       "instructions\n");
		else
			printf(", CPU executes undocumented instructions\n");
	}

	fflush(stdout);

	/* if the machine has configuration files try to find them */
#ifdef HAS_CONFIG
	/* first try ./conf */
	if ((stat("./conf", &sbuf) == 0) && S_ISDIR(sbuf.st_mode)) {
		strcpy(&confdir[0], "./conf");
	/* then CONFDIR as set in Makefile */
	} else {
		strcpy(&confdir[0], CONFDIR);
	}

	/* if option -r is used ROMS are there */
	if (rompath[0] == 0) {
		/* if not first try ./roms */
		if ((stat("./roms", &sbuf) == 0) && S_ISDIR(sbuf.st_mode)) {
			strcpy(rompath, "./roms");
		/* then BOOTROM as set in Makefile */
		} else {
			strcpy(rompath, BOOTROM);
		}
	}
#endif

	/* seed random generator */
	gettimeofday(&tv, NULL);
	srand(tv.tv_sec);

	config();		/* read system configuration */
	init_cpu();		/* initialise CPU */
	init_memory();	/* initialise memory configuration */

	if (l_flag) {		/* load core */
		if (load_core())
			return(1);
	} else if (x_flag) { 	/* OR load memory from file */
		if (load_file(xfn, 0, 0)) /* don't care where it loads */
			return(1);
	}

	int_on();		/* initialize UNIX interrupts */
	init_io();		/* initialize I/O devices */

	mon();			/* run system */

	if (s_flag)		/* save core */
		save_core();

	exit_io();		/* stop I/O devices */
	int_off();		/* stop UNIX interrupts */

	return(0);
}

/*
 *	Initialize the CPU
 */
static void init_cpu(void)
{
	/* same for i8080 and Z80 */
	PC = 0;
	SP = rand() % 65536;
	A = rand() % 256;
	B = rand() % 256;
	C = rand() % 256;
	D = rand() % 256;
	E = rand() % 256;
	H = rand() % 256;
	L = rand() % 256;
	F = rand() % 256;

	if (cpu == I8080) {	/* i8080 specific */
		F &= ~(N2_FLAG | N1_FLAG);
		F |= N_FLAG;
	} else {		/* Z80 specific */
		I = 0;
		A_ = rand() % 256;
		B_ = rand() % 256;
		C_ = rand() % 256;
		D_ = rand() % 256;
		E_ = rand() % 256;
		H_ = rand() % 256;
		L_ = rand() % 256;
		F_ = rand() % 256;
		IX = rand() % 65536;
		IY = rand() % 65536;
	}
}

/*
 *	Reset the CPU
 */
void reset_cpu(void)
{
	IFF = int_int = int_nmi = int_protection = int_mode = 0;
	int_data = -1;

	PC = 0;

	if (cpu == Z80) {
		I = 0;
		R_ = R = 0L;
	}
}

/*
 *	This function saves the CPU and the memory into the file core.z80
 */
static void save_core(void)
{
	register int i;
	int fd;
	BYTE d;

	if ((fd = open("core.z80", O_WRONLY | O_CREAT, 0600)) == -1) {
		puts("can't open file core.z80");
		return;
	}

	write(fd, (char *) &A, sizeof(A));
	write(fd, (char *) &F, sizeof(F));
	write(fd, (char *) &B, sizeof(B));
	write(fd, (char *) &C, sizeof(C));
	write(fd, (char *) &D, sizeof(D));
	write(fd, (char *) &E, sizeof(E));
	write(fd, (char *) &H, sizeof(H));
	write(fd, (char *) &L, sizeof(L));
	write(fd, (char *) &A_, sizeof(A_));
	write(fd, (char *) &F_, sizeof(F_));
	write(fd, (char *) &B_, sizeof(B_));
	write(fd, (char *) &C_, sizeof(C_));
	write(fd, (char *) &D_, sizeof(D_));
	write(fd, (char *) &E_, sizeof(E_));
	write(fd, (char *) &H_, sizeof(H_));
	write(fd, (char *) &L_, sizeof(L_));
	write(fd, (char *) &I, sizeof(I));
	write(fd, (char *) &IFF, sizeof(IFF));
	write(fd, (char *) &R, sizeof(R));
	write(fd, (char *) &R_, sizeof(R_));
	write(fd, (char *) &PC, sizeof(PC));
	write(fd, (char *) &SP, sizeof(SP));
	write(fd, (char *) &IX, sizeof(IX));
	write(fd, (char *) &IY, sizeof(IY));

	for (i = 0; i < 65536; i++) {
		d = getmem(i);
		write(fd, &d, 1);
	}

	close(fd);
}

/*
 *	This function loads the CPU and memory from the file core.z80
 */
int load_core(void)
{
	register int i;
	int fd;
	BYTE d;

	if ((fd = open("core.z80", O_RDONLY)) == -1) {
		puts("can't open file core.z80");
		return(1);
	}

	read(fd, (char *) &A, sizeof(A));
	read(fd, (char *) &F, sizeof(F));
	read(fd, (char *) &B, sizeof(B));
	read(fd, (char *) &C, sizeof(C));
	read(fd, (char *) &D, sizeof(D));
	read(fd, (char *) &E, sizeof(E));
	read(fd, (char *) &H, sizeof(H));
	read(fd, (char *) &L, sizeof(L));
	read(fd, (char *) &A_, sizeof(A_));
	read(fd, (char *) &F_, sizeof(F_));
	read(fd, (char *) &B_, sizeof(B_));
	read(fd, (char *) &C_, sizeof(C_));
	read(fd, (char *) &D_, sizeof(D_));
	read(fd, (char *) &E_, sizeof(E_));
	read(fd, (char *) &H_, sizeof(H_));
	read(fd, (char *) &L_, sizeof(L_));
	read(fd, (char *) &I, sizeof(I));
	read(fd, (char *) &IFF, sizeof(IFF));
	read(fd, (char *) &R, sizeof(R));
	read(fd, (char *) &R_, sizeof(R_));
	read(fd, (char *) &PC, sizeof(PC));
	read(fd, (char *) &SP, sizeof(SP));
	read(fd, (char *) &IX, sizeof(IX));
	read(fd, (char *) &IY, sizeof(IY));

	for (i = 0; i < 65536; i++) {
		read(fd, &d, 1);
		putmem(i, d);
	}

	close(fd);

	return(0);
}
