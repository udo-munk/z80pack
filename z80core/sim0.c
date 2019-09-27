/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2018 by Udo Munk
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
#include <memory.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif
#include "memory.h"

#define BUFSIZE	256		/* buffer size for file I/O */

static void init_cpu(void);
static void save_core(void);
int load_core(void);
int load_file(char *);
static int load_mos(int, char *), load_hex(char *), checksum(char *);
extern void int_on(void), int_off(void), mon(void);
extern void init_io(void), exit_io(void);
extern int exatoi(char *);

BYTE *wrk_ram;			/* work pointer for the memory */

int main(int argc, char *argv[])
{
	register char *s, *p;
	register int i;
	char *pn = basename(argv[0]);
	struct timeval tv;
#ifdef HAS_CONFIG
	struct stat sbuf;
#endif
#ifdef BOOTROM
	char *rom = "-r ";
#else
	char *rom = "";
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

#ifdef BOOTROM
			case 'r':	/* load default boot ROM */
				r_flag = 1;
				x_flag = 1;
				strcpy(xfn, BOOTROM);
				break;
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
				printf("illegal option %c\n", *s);

usage:

#ifdef HAS_DISKS
				printf("usage:\t%s -z -8 -s -l -i -u %s-m val -f freq -x filename -d diskpath\n", pn, rom);
#else
				printf("usage:\t%s -z -8 -s -l -i -u %s-m val -f freq -x filename\n", pn, rom);
#endif
				puts("\t-z = emulate Zilog Z80");
				puts("\t-8 = emulate Intel 8080");
				puts("\t-s = save core and CPU");
				puts("\t-l = load core and CPU");
				puts("\t-i = trap on I/O to unused ports");
				puts("\t-u = trap on undocumented instructions");
#ifdef BOOTROM
				puts("\t-r = load and execute default ROM");
				printf("\t     %s\n", BOOTROM);
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

#ifdef	USR_COM
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
			printf(", CPU doesn't execute undocumented instructions\n");
		else
			printf(", CPU executes undocumented instructions\n");
#endif
	} else {
		if (u_flag)
			printf(", CPU doesn't execute undocumented instructions\n");
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
#endif

	/* seed random generator */
	gettimeofday(&tv, NULL);
	srand(tv.tv_sec);

	config();		/* read system configuration */
	init_memory();		/* initialise memory configuration */
	init_cpu();		/* initialise CPU */
	wrk_ram	= mem_base();	/* set work pointer for memory */

	/* fill memory content with some initial value */
	if (m_flag >= 0) {
		memset((char *) wrk_ram, m_flag, 65536);
	} else {
		for (i = 0; i < 65536; i++)
			*(wrk_ram + i) = (BYTE) (rand() % 256);
	}

	init_rom();		/* initialise ROM's */

	if (l_flag)	{	/* load core */
		if (load_core())
			return(1);
	} else if (x_flag) { 	/* OR load memory from file */
		if (load_file(xfn))
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
		R = 0L;
	}
}

/*
 *	This function saves the CPU and the memory into the file core.z80
 */
static void save_core(void)
{
	int fd;

	if ((fd	= open("core.z80", O_WRONLY | O_CREAT, 0600)) == -1) {
		puts("can't open file core.z80");
		return;
	}
	write(fd, (char	*) &A, sizeof(A));
	write(fd, (char	*) &F, sizeof(F));
	write(fd, (char	*) &B, sizeof(B));
	write(fd, (char	*) &C, sizeof(C));
	write(fd, (char	*) &D, sizeof(D));
	write(fd, (char	*) &E, sizeof(E));
	write(fd, (char	*) &H, sizeof(H));
	write(fd, (char	*) &L, sizeof(L));
	write(fd, (char	*) &A_,	sizeof(A_));
	write(fd, (char	*) &F_,	sizeof(F_));
	write(fd, (char	*) &B_,	sizeof(B_));
	write(fd, (char	*) &C_,	sizeof(C_));
	write(fd, (char	*) &D_,	sizeof(D_));
	write(fd, (char	*) &E_,	sizeof(E_));
	write(fd, (char	*) &H_,	sizeof(H_));
	write(fd, (char	*) &L_,	sizeof(L_));
	write(fd, (char	*) &I, sizeof(I));
	write(fd, (char	*) &IFF, sizeof(IFF));
	write(fd, (char	*) &R, sizeof(R));
	write(fd, (char	*) &PC,	sizeof(PC));
	write(fd, (char	*) &SP, sizeof(SP));
	write(fd, (char	*) &IX,	sizeof(IX));
	write(fd, (char	*) &IY,	sizeof(IY));
	write(fd, (char	*) mem_base(), 65536);
	close(fd);
}

/*
 *	This function loads the CPU and memory from the file core.z80
 */
int load_core(void)
{
	int fd;

	if ((fd	= open("core.z80", O_RDONLY)) == -1) {
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
	read(fd, (char *) &IFF,	sizeof(IFF));
	read(fd, (char *) &R, sizeof(R));
	read(fd, (char *) &PC, sizeof(PC));
	read(fd, (char *) &SP, sizeof(SP));
	read(fd, (char *) &IX, sizeof(IX));
	read(fd, (char *) &IY, sizeof(IY));
	read(fd, (char *) mem_base(), 65536);
	close(fd);

	return(0);
}

/*
 *	Read a file into the memory of the emulated CPU.
 *	The following file formats are supported:
 *
 *		binary images with Mostek header
 *		Intel hex
 */
int load_file(char *s)
{
	char fn[MAX_LFN];
	BYTE fileb[5];
	register char *pfn = fn;
	int fd;

	while (isspace((int)*s))
		s++;
	while (*s != ',' && *s != '\n' && *s != '\0')
		*pfn++ = *s++;
	*pfn = '\0';
	if (strlen(fn) == 0) {
		puts("no input file given");
		return(1);
	}
	if ((fd	= open(fn, O_RDONLY)) == -1) {
		printf("can't open file %s\n", fn);
		return(1);
	}
	if (*s == ',')
		wrk_ram	= mem_base() + exatoi(++s);
	else
		wrk_ram	= NULL;
	read(fd, (char *) fileb, 5); /*	read first 5 bytes of file */
	if (*fileb == (BYTE) 0xff) {	/* Mostek header ? */
		lseek(fd, 0l, SEEK_SET);
		return (load_mos(fd, fn));
	}
	else {
		close(fd);
		return (load_hex(fn));
	}
}

/*
 *	Loader for binary images with Mostek header.
 *	Format of the first 3 bytes:
 *
 *	0xff ll	lh
 *
 *	ll = load address low
 *	lh = load address high
 */
static int load_mos(int fd, char *fn)
{
	BYTE fileb[3];
	unsigned count,	readn;
	int rc = 0;

	read(fd, (char *) fileb, 3);	/* read load address */
	if (wrk_ram == NULL)		/* and set if not given */
		wrk_ram	= mem_base() + (fileb[2] << 8) + fileb[1];
	count = mem_base() + 65535 - wrk_ram;
	if ((readn = read(fd, (char *) wrk_ram, count)) == count) {
		puts("Too much to load, stopped at 0xffff");
		rc = 1;
	}
	close(fd);
	printf("Loader statistics for file %s:\n", fn);
	printf("START : %04X\n", (unsigned int)(wrk_ram - mem_base()));
	printf("END   : %04X\n", (unsigned int)(wrk_ram - mem_base()
				 + readn - 1));
	printf("LOADED: %04X\n\n", readn);
	PC = wrk_ram - mem_base();
	return(rc);
}

/*
 *	Loader for Intel hex
 */
static int load_hex(char *fn)
{
	register int i;
	FILE *fd;
	char buf[BUFSIZE];
	char *s;
	int count = 0;
	int addr = 0;
	int saddr = 0xffff;
	int eaddr = 0;
	int data;

	if ((fd = fopen(fn, "r")) == NULL) {
		printf("can't open file %s\n", fn);
		return(1);
	}

	while (fgets(&buf[0], BUFSIZE, fd) != NULL) {
		s = &buf[0];
		while (isspace((int)*s))
			s++;
		if (*s != ':')
			continue;
		if (checksum(s + 1) != 0) {
			printf("invalid checksum in hex record: %s\n", s);
			return(1);
		}
		s++;
		count = (*s <= '9') ? (*s - '0') << 4 :
				      (*s - 'A' + 10) << 4;
		s++;
		count += (*s <= '9') ? (*s - '0') :
				       (*s - 'A' + 10);
		s++;
		if (count == 0)
			break;
		addr = (*s <= '9') ? (*s - '0') << 4 :
				     (*s - 'A' + 10) << 4;
		s++;
		addr += (*s <= '9') ? (*s - '0') :
				      (*s - 'A' + 10);
		s++;
		addr *= 256;
		addr += (*s <= '9') ? (*s - '0') << 4 :
				      (*s - 'A' + 10) << 4;
		s++;
		addr += (*s <= '9') ? (*s - '0') :
				      (*s - 'A' + 10);
		s++;
		if (addr < saddr)
			saddr = addr;
		if (addr >= eaddr)
			eaddr = addr + count - 1;
		s += 2;
		for (i = 0; i < count; i++) {
			data = (*s <= '9') ? (*s - '0') << 4 :
					     (*s - 'A' + 10) << 4;
			s++;
			data += (*s <= '9') ? (*s - '0') :
					      (*s - 'A' + 10);
			s++;
			MEMORY_WRITE(addr+i) = data;
		}
	}

	fclose(fd);
	count = eaddr - saddr + 1;
	printf("Loader statistics for file %s:\n", fn);
	printf("START : %04XH\n", saddr);
	printf("END   : %04XH\n", eaddr);
	printf("LOADED: %04XH (%d)\n\n", count & 0xffff, count & 0xffff);
	PC = saddr;
	wrk_ram = mem_base() + saddr;

	return(0);
}

/*
 *	Verify checksum of Intel hex records
 */
static int checksum(char *s)
{
	int chk = 0;

	while ((*s != '\r') && (*s != '\n')) {
		chk += (*s <= '9') ?
			(*s - '0') << 4 :
			(*s - 'A' + 10) << 4;
		s++;
		chk += (*s <= '9') ?
			(*s - '0') :
			(*s - 'A' + 10);
		s++;
	}

	if ((chk & 255) == 0)
		return(0);
	else
		return(1);
}
