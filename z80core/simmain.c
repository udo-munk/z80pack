/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 Udo Munk
 * Copyright (C) 2021 David McNaughton
 * Copyright (C) 2022-2024 Thomas Eberhardt
 */

/*
 *	This module contains the 'main()' function of the simulator,
 *	where the options are checked and variables are initialized.
 *	After initialization of the UNIX interrupts ( int_on() )
 *	and initialization of the I/O simulation ( init_io() )
 *	the user interface ( mon() ) is called.
 *
 *	Usable for UNIX workstations, for development and IoT boards
 *	this should be substituted, see picosim for example.
 */

#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcore.h"
#include "simcfg.h"
#include "simctl.h"
#include "simmem.h"
#include "simio.h"
#include "simport.h"
#include "simfun.h"
#include "simint.h"

static void save_core(void);
static int load_core(void);

#ifdef WANT_SDL
int sim_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	register char *s, *p;
	char *pn = basename(argv[0]);
#ifdef CONFDIR
	struct stat sbuf;
#endif
#ifdef HAS_CONFIG
	const char *rom = "-r rompath ";
#else
#ifdef BOOTROM
	const char *rom = "-r ";
#else
	const char *rom = "";
#endif
#endif
#ifdef CPU_SPEED
	f_value = CPU_SPEED;
	tmax = CPU_SPEED * 10000; /* theoretically */
#endif

	while (--argc > 0 && (*++argv)[0] == '-')
		for (s = argv[0] + 1; *s != '\0'; s++)

			switch (*s) {
			case 's':	/* save core and CPU on exit */
				s_flag = true;
				break;

			case 'l':	/* load core and CPU from file */
				l_flag = true;
				break;

			case 'u':	/* trap undocumented ops */
				u_flag = true;
				break;

			case 'i':	/* trap I/O on unused ports */
				i_flag = true;
				break;

			case 'm':	/* initialize memory */
				if (*(s + 1) != '\0') {
					m_value = strtol(s + 1, NULL, 16);
					s += strlen(s + 1);
				} else {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					m_value = strtol(argv[0], NULL, 16);
				}
				break;

			case 'f':	/* set emulation speed */
				if (*(s + 1) != '\0') {
					f_value = atoi(s + 1);
					s += strlen(s + 1);
				} else {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					f_value = atoi(argv[0]);
				}
				tmax = f_value * 10000; /* theoretically */
				break;

			case 'x':	/* get filename with executable */
				x_flag = true;
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
#else /* !HAS_CONFIG */
#ifdef BOOTROM
			case 'r':	/* load default boot ROM */
				r_flag = true;
				x_flag = true;
				strcpy(xfn, BOOTROM);
				break;
#endif
#endif /* !HAS_CONFIG */

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
				c_flag = true;
				s++;
				if (*s == '\0') {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					s = argv[0];
				}
				p = conffn;
				while (*s)
					*p++ = *s++;
				*p = '\0';
				s--;
				break;

#if MAXMEMSECT > 0
			case 'M': /* use memory map section nn */
				if (*(s + 1) != '\0') {
					M_value = atoi(s + 1) - 1;
					s += strlen(s + 1);
				} else {
					if (argc <= 1)
						goto usage;
					argc--;
					argv++;
					M_value = atoi(argv[0]) - 1;
				}
				if (M_value < 0 || M_value > (MAXMEMSECT - 1))
					goto usage;
				break;
#endif

#endif /* HAS_CONFIG */

#ifdef HAS_BANKED_ROM
			case 'R':	/* enable banked ROM for machines
					   that implement it */
				R_flag = true;
				break;
#endif

#ifndef EXCLUDE_I8080
			case '8':	/* emulate Intel 8080 */
				cpu = I8080;
				break;
#endif

#ifndef EXCLUDE_Z80
			case 'z':	/* emulate Zilog Z80 */
				cpu = Z80;
				break;
#endif
#ifdef FRONTPANEL
			case 'F':	/* disable front panel emulation */
				F_flag = false;
				break;
#endif
#ifdef HAS_NETSERVER
			case 'n':	/* enable web-based frontend */
				n_flag = true;
				break;
#endif

			case '?':
			case 'h':
				goto usage;

			default:
				printf("invalid option %c\n", *s);

usage:

				printf("usage:\t%s%s%s -s -l -i -u %s-m val "
				       "-f freq\n\t\t-x filename", pn,
#ifndef EXCLUDE_Z80
				       " -z",
#else
				       "",
#endif
#ifndef EXCLUDE_I8080
				       " -8",
#else
				       "",
#endif
				       rom);
#ifdef HAS_DISKS
				fputs(" -d diskpath", stdout);
#endif
#ifdef HAS_CONFIG
				fputs(" -c filename", stdout);
#if MAXMEMSECT > 0
				fputs(" -M val", stdout);
#endif
#endif
#ifdef HAS_BANKED_ROM
				fputs(" -R", stdout);
#endif
#ifdef FRONTPANEL
				fputs(" -F", stdout);
#endif
#ifdef HAS_NETSERVER
				fputs(" -n", stdout);
#endif
				fputs("\n\n", stdout);
#ifndef EXCLUDE_Z80
				puts("\t-z = emulate Zilog Z80");
#endif
#ifndef EXCLUDE_I8080
				puts("\t-8 = emulate Intel 8080");
#endif
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
#ifdef FRONTPANEL
				puts("\t-F = disable front panel emulation");
#endif
#ifdef HAS_NETSERVER
				puts("\t-n = enable web-based frontend");
#endif
				return EXIT_FAILURE;
			}

	putchar('\n');

#ifndef EXCLUDE_Z80
	if (cpu == Z80) {
puts("#######  #####    ###            #####    ###   #     #");
puts("     #  #     #  #   #          #     #    #    ##   ##");
puts("    #   #     # #     #         #          #    # # # #");
puts("   #     #####  #     #  #####   #####     #    #  #  #");
puts("  #     #     # #     #               #    #    #     #");
puts(" #      #     #  #   #          #     #    #    #     #");
puts("#######  #####    ###            #####    ###   #     #");
	}
#endif
#ifndef EXCLUDE_I8080
	if (cpu == I8080) {
puts(" #####    ###     #####    ###            #####    ###   #     #");
puts("#     #  #   #   #     #  #   #          #     #    #    ##   ##");
puts("#     # #     #  #     # #     #         #          #    # # # #");
puts(" #####  #     #   #####  #     #  #####   #####     #    #  #  #");
puts("#     # #     #  #     # #     #               #    #    #     #");
puts("#     #  #   #   #     #  #   #          #     #    #    #     #");
puts(" #####    ###     #####    ###            #####    ###   #     #");
	}
#endif

	printf("\nRelease %s, %s\n", RELEASE, COPYR);

#ifdef USR_COM
	printf("%s Release %s\n%s\n\n", USR_COM, USR_REL, USR_CPR);
#else
	putchar('\n');
#endif

	if (f_value > 0)
		printf("CPU speed is %d MHz", f_value);
	else
		fputs("CPU speed is unlimited", stdout);

#ifndef UNDOC_INST
	puts(", CPU doesn't execute undocumented instructions");
#else
	if (u_flag)
		puts(", CPU doesn't execute undocumented instructions");
	else
		puts(", CPU executes undocumented instructions");
#endif
	fflush(stdout);

	/* if the machine has configuration files try to find them */
#ifdef CONFDIR
	/* first try ./conf */
	if ((stat("./conf", &sbuf) == 0) && S_ISDIR(sbuf.st_mode)) {
		strcpy(confdir, "./conf");
	} else {
		/* then CONFDIR as set in Makefile */
		strcpy(confdir, CONFDIR);
	}
#endif

#ifdef HAS_CONFIG
	/* if option -r is used ROMS are there */
	if (rompath[0] == '\0') {
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
	srand(get_clock_us());

	config();		/* read system configuration */
	init_cpu();		/* initialize CPU */
	init_memory();		/* initialize memory configuration */

	if (l_flag) {		/* load core */
		if (load_core())
			return EXIT_FAILURE;
	} else if (x_flag) { 	/* OR load memory from file */
		if (load_file(xfn, 0, 0)) /* don't care where it loads */
			return EXIT_FAILURE;
	}

	int_on();		/* initialize UNIX interrupts */
	init_io();		/* initialize I/O devices */

	mon();			/* run system */

	if (s_flag)		/* save core */
		save_core();

	exit_io();		/* stop I/O devices */
	int_off();		/* stop UNIX interrupts */

	return EXIT_SUCCESS;
}

/*
 *	This function saves the CPU and the memory into the
 *	file core.z80 or core.8080
 */
static void save_core(void)
{
	register FILE *fp;
	register int i;
	int fd;
	bool err;
	const char *fname;

#ifndef EXCLUDE_Z80
	if (cpu == Z80)
		fname = "core.z80";
#endif
#ifndef EXCLUDE_I8080
	if (cpu == I8080)
		fname = "core.8080";
#endif
	if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) == -1
	    || (fp = fdopen(fd, "w")) == NULL) {
		if (fd != -1)
			close(fd);
		printf("can't open file %s\n", fname);
		return;
	}

	err = false;
	if (fwrite(&A, sizeof(A), 1, fp) != 1 ||
	    fwrite(&F, sizeof(F), 1, fp) != 1 ||
	    fwrite(&B, sizeof(B), 1, fp) != 1 ||
	    fwrite(&C, sizeof(C), 1, fp) != 1 ||
	    fwrite(&D, sizeof(D), 1, fp) != 1 ||
	    fwrite(&E, sizeof(E), 1, fp) != 1 ||
	    fwrite(&H, sizeof(H), 1, fp) != 1 ||
	    fwrite(&L, sizeof(L), 1, fp) != 1)
		err = true;
#ifndef EXCLUDE_Z80
	if (!err && cpu == Z80 &&
	    (fwrite(&A_, sizeof(A_), 1, fp) != 1 ||
	     fwrite(&F_, sizeof(F_), 1, fp) != 1 ||
	     fwrite(&B_, sizeof(B_), 1, fp) != 1 ||
	     fwrite(&C_, sizeof(C_), 1, fp) != 1 ||
	     fwrite(&D_, sizeof(D_), 1, fp) != 1 ||
	     fwrite(&E_, sizeof(E_), 1, fp) != 1 ||
	     fwrite(&H_, sizeof(H_), 1, fp) != 1 ||
	     fwrite(&L_, sizeof(L_), 1, fp) != 1 ||
	     fwrite(&I, sizeof(I), 1, fp) != 1))
		err = true;
#endif
	if (!err && fwrite(&IFF, sizeof(IFF), 1, fp) != 1)
		err = true;
#ifndef EXCLUDE_Z80
	if (!err && cpu == Z80 &&
	    (fwrite(&R, sizeof(R), 1, fp) != 1 ||
	     fwrite(&R_, sizeof(R_), 1, fp) != 1))
		err = true;
#endif
	if (!err &&
	    (fwrite(&PC, sizeof(PC), 1, fp) != 1 ||
	     fwrite(&SP, sizeof(SP), 1, fp) != 1))
		err = true;
#ifndef EXCLUDE_Z80
	if (!err && cpu == Z80 &&
	    (fwrite(&IX, sizeof(IX), 1, fp) != 1 ||
	     fwrite(&IY, sizeof(IY), 1, fp) != 1))
		err = true;
#endif

	if (!err) {
		for (i = 0; i < 65536; i++)
			if (putc(getmem(i), fp) == EOF) {
				err = true;
				break;
			}
	}

	fclose(fp);

	if (err)
		printf("error writing %s\n", fname);
}

/*
 *	This function loads the CPU and memory from the
 *	file core.z80 or core.8080
 */
static int load_core(void)
{
	register FILE *fp;
	register int i, c;
	bool err;
	const char *fname;

#ifndef EXCLUDE_Z80
	if (cpu == Z80)
		fname = "core.z80";
#endif
#ifndef EXCLUDE_I8080
	if (cpu == I8080)
		fname = "core.8080";
#endif
	if ((fp = fopen(fname, "r")) == NULL) {
		printf("can't open file %s\n", fname);
		return 1;
	}

	err = false;
	if (fread(&A, sizeof(A), 1, fp) != 1 ||
	    fread(&F, sizeof(F), 1, fp) != 1 ||
	    fread(&B, sizeof(B), 1, fp) != 1 ||
	    fread(&C, sizeof(C), 1, fp) != 1 ||
	    fread(&D, sizeof(D), 1, fp) != 1 ||
	    fread(&E, sizeof(E), 1, fp) != 1 ||
	    fread(&H, sizeof(H), 1, fp) != 1 ||
	    fread(&L, sizeof(L), 1, fp) != 1)
		err = true;
#ifndef EXCLUDE_Z80
	if (!err && cpu == Z80 &&
	    (fread(&A_, sizeof(A_), 1, fp) != 1 ||
	     fread(&F_, sizeof(F_), 1, fp) != 1 ||
	     fread(&B_, sizeof(B_), 1, fp) != 1 ||
	     fread(&C_, sizeof(C_), 1, fp) != 1 ||
	     fread(&D_, sizeof(D_), 1, fp) != 1 ||
	     fread(&E_, sizeof(E_), 1, fp) != 1 ||
	     fread(&H_, sizeof(H_), 1, fp) != 1 ||
	     fread(&L_, sizeof(L_), 1, fp) != 1 ||
	     fread(&I, sizeof(I), 1, fp) != 1))
		err = true;
#endif
	if (!err && fread(&IFF, sizeof(IFF), 1, fp) != 1)
		err = true;
#ifndef EXCLUDE_Z80
	if (!err && cpu == Z80 &&
	    (fread(&R, sizeof(R), 1, fp) != 1 ||
	     fread(&R_, sizeof(R_), 1, fp) != 1))
		err = true;
#endif
	if (!err &&
	    (fread(&PC, sizeof(PC), 1, fp) != 1 ||
	     fread(&SP, sizeof(SP), 1, fp) != 1))
		err = true;
#ifndef EXCLUDE_Z80
	if (!err && cpu == Z80 &&
	    (fread(&IX, sizeof(IX), 1, fp) != 1 ||
	     fread(&IY, sizeof(IY), 1, fp) != 1))
		err = true;
#endif

	if (!err) {
		for (i = 0; i < 65536; i++)
			if ((c = getc(fp)) == EOF) {
				err = true;
				break;
			} else
				putmem(i, c);
	}

	fclose(fp);

	if (err) {
		printf("error reading %s\n", fname);
		return 1;
	} else
		return 0;
}
