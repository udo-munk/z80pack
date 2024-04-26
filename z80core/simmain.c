/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 Udo Munk
 * Copyright (C) 2021 David McNaughton
 * Copyright (C) 2022 Thomas Eberhardt
 */

/*
 *	This module contains the 'main()' function of the simulator,
 *	where the options are checked and variables are initialised.
 *	After initialisation of the UNIX interrupts ( int_on() )
 *	and initialisation of the I/O simulation ( init_io() )
 *	the user interface ( mon() ) is called.
 *
 *	Usable for UNIX workstations, for development and IoT boards
 *	this should be substituted, see picosim for example.
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

static void save_core(void);
int load_core(void);
extern void init_cpu(void);
extern int load_file(char *, WORD, int);
extern void int_on(void), int_off(void), mon(void);
extern void init_io(void), exit_io(void);
extern int exatoi(char *);

#ifdef FRONTPANEL
int sim_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
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
				if (*(s + 1) != '\0') {
					m_flag = exatoi(s + 1);
					s += strlen(s + 1);
				} else {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					m_flag = exatoi(argv[0]);
				}
				break;

			case 'f':	/* set emulation speed */
				if (*(s + 1) != '\0') {
					f_flag = atoi(s + 1);
					s += strlen(s + 1);
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
				if (*(s + 1) != '\0') {
					M_flag = atoi(s + 1) - 1;
					s += strlen(s + 1);
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
				puts("\t-u = trap on "
				     "undocumented instructions");
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
				exit(EXIT_FAILURE);
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

#ifndef UNDOC_INST
	printf(", CPU doesn't execute undocumented instructions\n");
#else
	if (u_flag)
		printf(", CPU doesn't execute undocumented instructions\n");
	else
		printf(", CPU executes undocumented instructions\n");
#endif

	fflush(stdout);

	/* if the machine has configuration files try to find them */
#ifdef HAS_CONFIG
	/* first try ./conf */
	if ((stat("./conf", &sbuf) == 0) && S_ISDIR(sbuf.st_mode)) {
		strcpy(&confdir[0], "./conf");
	} else {
		/* then CONFDIR as set in Makefile */
		strcpy(&confdir[0], CONFDIR);
	}

	/* if option -r is used ROMS are there */
	if (rompath[0] == 0) {
		/* if not first try ./roms */
		if ((stat("./roms", &sbuf) == 0) && S_ISDIR(sbuf.st_mode)) {
			strcpy(rompath, "./roms");
		} else {
			/* then BOOTROM as set in Makefile */
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
			return (EXIT_FAILURE);
	} else if (x_flag) { 	/* OR load memory from file */
		if (load_file(xfn, 0, 0)) /* don't care where it loads */
			return (EXIT_FAILURE);
	}

	int_on();		/* initialize UNIX interrupts */
	init_io();		/* initialize I/O devices */

	mon();			/* run system */

	if (s_flag)		/* save core */
		save_core();

	exit_io();		/* stop I/O devices */
	int_off();		/* stop UNIX interrupts */

	return (EXIT_SUCCESS);
}

/*
 *	This function saves the CPU and the memory into the file core.z80
 */
static void save_core(void)
{
	register FILE *fp;
	register int i;
	int fd;

	if ((fd = open("core.z80", O_WRONLY | O_CREAT, 0600)) == -1
	    || (fp = fdopen(fd, "w")) == NULL) {
		if (fd >= 0)
			close(fd);
		puts("can't open file core.z80");
		return;
	}

	fwrite(&A, sizeof(A), 1, fp);
	fwrite(&F, sizeof(F), 1, fp);
	fwrite(&B, sizeof(B), 1, fp);
	fwrite(&C, sizeof(C), 1, fp);
	fwrite(&D, sizeof(D), 1, fp);
	fwrite(&E, sizeof(E), 1, fp);
	fwrite(&H, sizeof(H), 1, fp);
	fwrite(&L, sizeof(L), 1, fp);
	fwrite(&A_, sizeof(A_), 1, fp);
	fwrite(&F_, sizeof(F_), 1, fp);
	fwrite(&B_, sizeof(B_), 1, fp);
	fwrite(&C_, sizeof(C_), 1, fp);
	fwrite(&D_, sizeof(D_), 1, fp);
	fwrite(&E_, sizeof(E_), 1, fp);
	fwrite(&H_, sizeof(H_), 1, fp);
	fwrite(&L_, sizeof(L_), 1, fp);
	fwrite(&I, sizeof(I), 1, fp);
	fwrite(&IFF, sizeof(IFF), 1, fp);
	fwrite(&R, sizeof(R), 1, fp);
	fwrite(&R_, sizeof(R_), 1, fp);
	fwrite(&PC, sizeof(PC), 1, fp);
	fwrite(&SP, sizeof(SP), 1, fp);
	fwrite(&IX, sizeof(IX), 1, fp);
	fwrite(&IY, sizeof(IY), 1, fp);

	for (i = 0; i < 65536; i++)
		fputc(getmem(i), fp);

	if (ferror(fp)) {
		fclose(fp);
		puts("error writing core.z80");
		return;
	}
	fclose(fp);
}

/*
 *	This function loads the CPU and memory from the file core.z80
 */
int load_core(void)
{
	register FILE *fp;
	register int i;

	if ((fp = fopen("core.z80", "r")) == NULL) {
		puts("can't open file core.z80");
		return (1);
	}

	fread(&A, sizeof(A), 1, fp);
	fread(&F, sizeof(F), 1, fp);
	fread(&B, sizeof(B), 1, fp);
	fread(&C, sizeof(C), 1, fp);
	fread(&D, sizeof(D), 1, fp);
	fread(&E, sizeof(E), 1, fp);
	fread(&H, sizeof(H), 1, fp);
	fread(&L, sizeof(L), 1, fp);
	fread(&A_, sizeof(A_), 1, fp);
	fread(&F_, sizeof(F_), 1, fp);
	fread(&B_, sizeof(B_), 1, fp);
	fread(&C_, sizeof(C_), 1, fp);
	fread(&D_, sizeof(D_), 1, fp);
	fread(&E_, sizeof(E_), 1, fp);
	fread(&H_, sizeof(H_), 1, fp);
	fread(&L_, sizeof(L_), 1, fp);
	fread(&I, sizeof(I), 1, fp);
	fread(&IFF, sizeof(IFF), 1, fp);
	fread(&R, sizeof(R), 1, fp);
	fread(&R_, sizeof(R_), 1, fp);
	fread(&PC, sizeof(PC), 1, fp);
	fread(&SP, sizeof(SP), 1, fp);
	fread(&IX, sizeof(IX), 1, fp);
	fread(&IY, sizeof(IY), 1, fp);

	for (i = 0; i < 65536; i++)
		putmem(i, getc(fp));

	if (ferror(fp)) {
		fclose(fp);
		puts("error reading core.z80");
		return (1);
	}
	fclose(fp);
	return (0);
}
