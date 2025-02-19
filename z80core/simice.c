/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

/*
 *	This module is an ICE type user interface to debug Z80/8080 programs
 *	on a host system. It emulates, as acurate as possible and practical,
 *	the Mostek ICE we were using in the 80th and 90th.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sim.h"
#include "simdefs.h"
#include "simglb.h"
#include "simcore.h"
#include "simmem.h"
#include "simdis.h"
#include "simport.h"
#include "simice.h"

#ifndef BAREMETAL
#include <signal.h>
#include <sys/time.h>
#include "simfun.h"
#include "simint.h"
#endif

#ifdef WANT_ICE

/*
 *	Variables for history memory
 */
#ifdef HISIZE
history_t his[HISIZE];		/* memory to hold trace information */
int h_next;			/* index into trace memory */
bool h_flag;			/* flag for trace memory overrun */
#endif

/*
 *	Variables for breakpoint memory
 */
#ifdef SBSIZE
softbreak_t soft[SBSIZE];	/* memory to hold breakpoint information */
#endif

/*
 *	Variables for runtime measurement
 */
#ifdef WANT_TIM
Tstates_t t_states_s;		/* T states marker at start of measurement */
Tstates_t t_states_e;		/* T states marker at end of measurement */
bool t_flag;			/* flag, true = on, false = off */
WORD t_start = 65535;		/* start address for measurement */
WORD t_end = 65535;		/* end address for measurement */
#endif

/*
 *	Variables for hardware breakpoint
 */
#ifdef WANT_HB
bool hb_flag;			/* hardware breakpoint enabled flag */
WORD hb_addr;			/* address of hardware breakpoint */
int hb_mode;			/* access mode of hardware breakpoint */
int hb_trig;			/* hardware breakpoint triggered flag */
#endif

static void do_step(void);
static void do_trace(char *s);
static void do_go(char *s);
static void install_softbp(void);
static void uninstall_softbp(void);
static bool handle_break(void);
static void do_dump(char *s);
static void do_list(char *s);
static void do_modify(char *s);
static void do_fill(char *s);
static void do_move(char *s);
static void do_port(char *s);
static void do_reg(char *s);
static void print_head(void);
static void print_reg(void);
static void do_break(char *s);
static void do_hist(char *s);
static void do_count(char *s);
#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
static void do_switch(char *s);
#endif
static void do_uflag(void);
static void do_iflag(void);
static void do_show(void);
static void do_help(void);

#ifndef BAREMETAL
static void do_clock(void);
static void timeout(int sig);
static void do_load(char *s);
static void do_unix(char *s);
#endif

static char arg[LENCMD];
static WORD wrk_addr;

void (*ice_before_go)(void);
void (*ice_after_go)(void);
void (*ice_cust_cmd)(char *cmd, WORD *wrk_addr);
void (*ice_cust_help)(void);

/*
 *	The function "ice_cmd_loop()" is the dialog user interface, called
 *	from the simulation when desired with go_mode set to 0, or, when
 *	no other user interface is needed, directly after any necessary
 *	initialization work at program start with go_mode set to 1 (or 2
 *	with timing output).
 *
 *	There are also two function pointers "ice_before_go" and
 *	"ice_after_go" which can be set to a void function(void) to be
 *	called before and after the go command. For example, mosteksim
 *	uses "set_unix_terminal" and "reset_unix_terminal".
 *
 *	Additionally you can add custom commands by setting "ice_cust_cmd"
 *	to a void function(char *, WORD *) which gets called with the
 *	command line and a pointer to the current working address as
 *	parameters. "ice_cust_help" can be set to a void function(void)
 *	to display help for custom commands.
 */
void ice_cmd_loop(int go_mode)
{
	register bool eoj = true;
	static char cmd[LENCMD];

	if (!go_mode) {
		report_cpu_error();
		print_head();
		print_reg();
		(void) disass(PC);
	}
	wrk_addr = PC;

	while (eoj) {
		if (go_mode) {
			strcpy(cmd, (go_mode > 1) ? "g *" : "g");
			go_mode = 0;
		} else {
			printf(">>> ");
			fflush(stdout);
			if (!get_cmdline(cmd, LENCMD)) {
				eoj = false;
				continue;
			}
		}
		switch (tolower((unsigned char) *cmd)) {
		case '\n':
		case '\0':
			do_step();
			break;
		case 't':
			do_trace(cmd + 1);
			break;
		case 'g':
			do_go(cmd + 1);
			break;
		case 'd':
			do_dump(cmd + 1);
			break;
		case 'l':
			do_list(cmd + 1);
			break;
		case 'm':
			do_modify(cmd + 1);
			break;
		case 'f':
			do_fill(cmd + 1);
			break;
		case 'v':
			do_move(cmd + 1);
			break;
		case 'x':
			do_reg(cmd + 1);
			break;
		case 'p':
			do_port(cmd + 1);
			break;
		case 'b':
			do_break(cmd + 1);
			break;
		case 'h':
			do_hist(cmd + 1);
			break;
		case 'z':
			do_count(cmd + 1);
			break;
#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
		case '8':
			do_switch(cmd + 1);
			break;
#endif
		case 'u':
			do_uflag();
			break;
		case 'i':
			do_iflag();
			break;
		case 's':
			do_show();
			break;
		case '?':
			do_help();
			break;
#ifndef BAREMETAL
		case 'c':
			do_clock();
			break;
		case 'r':
			do_load(cmd + 1);
			break;
		case '!':
			do_unix(cmd + 1);
			break;
#endif
		case 'q':
			eoj = false;
			break;
		default:
			if (ice_cust_cmd)
				(*ice_cust_cmd)(cmd, &wrk_addr);
			else
				puts("what??");
			break;
		}
	}
}

/*
 *	Execute a single step
 */
static void do_step(void)
{
	install_softbp();
	step_cpu();
	if (cpu_error == OPHALT)
		(void) handle_break();
	uninstall_softbp();
	report_cpu_error();
	print_head();
	print_reg();
	(void) disass(PC);
	wrk_addr = PC;
}

/*
 *	Execute several steps with trace output
 */
static void do_trace(char *s)
{
	register int count, i;

	while (isspace((unsigned char) *s))
		s++;
	if (*s == '\0')
		count = 20;
	else
		count = atoi(s);
	print_head();
	print_reg();
	install_softbp();
	for (i = 0; i < count; i++) {
		step_cpu();
		print_reg();
		if (cpu_error && (cpu_error != OPHALT || handle_break()))
			break;
	}
	uninstall_softbp();
	report_cpu_error();
	wrk_addr = PC;
}

/*
 *	Run the CPU emulation endless
 */
static void do_go(char *s)
{
	int timeit = 0;
	uint64_t start_cpu_time, stop_cpu_time;
	uint64_t start_io_time, stop_io_time;
	uint64_t start_wait_time, stop_wait_time;
	Tstates_t T0;
	unsigned freq;

	while (isspace((unsigned char) *s))
		s++;
	if (*s == '*') {
		timeit = 1;
		s++;
	}
	if (isxdigit((unsigned char) *s))
		PC = strtol(s, NULL, 16);
	if (ice_before_go)
		(*ice_before_go)();
	install_softbp();
	T0 = T;
	start_cpu_time = cpu_time;
	start_io_time = total_io_time;
	start_wait_time = total_wait_time;
	while (true) {
		run_cpu();
		if (cpu_error && (cpu_error != OPHALT || handle_break()))
			break;
	}
	stop_cpu_time = cpu_time;
	stop_io_time = total_io_time;
	stop_wait_time = total_wait_time;
	uninstall_softbp();
	if (ice_after_go)
		(*ice_after_go)();
	report_cpu_error();
	if (timeit) {
		freq = (unsigned) (((T - T0) * 100) /
				   (stop_cpu_time - start_cpu_time));
		printf("I/O ran for %" PRIu64 " ms, ",
		       (stop_io_time - start_io_time) / 1000);
		printf("waited for %" PRIu64 " ms\n",
		       (stop_wait_time - start_wait_time) / 1000);
		printf("CPU executed %" PRIu64 " t-states in %" PRIu64 " ms\n",
		       T - T0, (stop_cpu_time - start_cpu_time) / 1000);
		printf("clock frequency = %u.%02u MHz\n",
		       freq / 100, freq % 100);
	}
	print_head();
	print_reg();
	wrk_addr = PC;
}

/*
 *	Install software breakpoints (HALT opcode)
 */
static void install_softbp(void)
{
#ifdef SBSIZE
	register int i;

	for (i = 0; i < SBSIZE; i++)
		if (soft[i].sb_pass) {
			soft[i].sb_oldopc = getmem(soft[i].sb_addr);
			putmem(soft[i].sb_addr, 0x76); /* HALT */
		}
#endif
}

/*
 *	Uninstall software breakpoints (HALT opcode)
 */
static void uninstall_softbp(void)
{
#ifdef SBSIZE
	register int i;

	for (i = 0; i < SBSIZE; i++)
		if (soft[i].sb_pass)
			putmem(soft[i].sb_addr, soft[i].sb_oldopc);
#endif
}

/*
 *	Handling of software (HALT opcode) / hardware breakpoints:
 *
 *	Output:	false breakpoint hit, pass counter not reached (continue)
 *		true breakpoint or other HALT opcode reached (stop)
 */
static bool handle_break(void)
{
#ifdef SBSIZE
	register int i;
#endif

#ifdef WANT_HB
	if (hb_flag && hb_trig) {
		printf("Hardware breakpoint hit by ");
		if (hb_trig == HB_READ)
			printf("read");
		else if (hb_trig == HB_WRITE)
			printf("write");
		else
			printf("execute");
		printf(" access to %04x\n", hb_addr);
		hb_trig = 0;
		cpu_error = NONE;
		return true;
	}
#endif
#ifdef SBSIZE
	for (i = 0; i < SBSIZE; i++)	/* search for breakpoint */
		if (soft[i].sb_pass && soft[i].sb_addr == PC - 1)
			break;
	if (i == SBSIZE)		/* no breakpoint found */
		return true;
#ifdef HISIZE
	if (h_next)			/* correct history */
		h_next--;
	else
		h_next = HISIZE - 1;
#endif
	PC--;				/* substitute HALT opcode by */
	putmem(PC, soft[i].sb_oldopc);	/* original opcode */
	step_cpu();			/* and execute it */
	putmem(soft[i].sb_addr, 0x76);	/* restore HALT opcode again */
	soft[i].sb_passcount++;		/* increment pass counter */
	if (soft[i].sb_passcount != soft[i].sb_pass)
		return false;		/* pass not reached, continue */
	printf("Software breakpoint hit at %04x\n", soft[i].sb_addr);
	soft[i].sb_passcount = 0;	/* reset pass counter */
	return true;			/* pass reached, stop */
#else /* !SBSIZE */
	return true;
#endif /* !SBSIZE */
}

/*
 *	Memory dump
 */
static void do_dump(char *s)
{
	register int i, j;
	int n = 16;
	BYTE c;

	while (isspace((unsigned char) *s))
		s++;
	if (isxdigit((unsigned char) *s))
		wrk_addr = strtol(s, &s, 16) & ~0xf;
	while (isspace((unsigned char) *s))
		s++;
	if (*s == ',') {
		s++;
		while (isspace((unsigned char) *s))
			s++;
		if (isxdigit((unsigned char) *s)) {
			n = ((strtol(s, NULL, 16) & ~0xf) - wrk_addr) / 16 + 1;
			if (n <= 0)
				n = 1;
		}
	}
	printf("Addr   ");
	for (i = 0; i < 16; i++)
		printf("%02x ", i);
	puts(" ASCII");
	for (i = 0; i < n; i++) {
		printf("%04x - ", (unsigned int) wrk_addr);
		for (j = 0; j < 16; j++)
			printf("%02x ", getmem(wrk_addr++));
		putchar('\t');
		for (j = -16; j < 0; j++) {
			c = getmem(wrk_addr + j);
			printf("%c", isprint((unsigned char) c) ? c : '.');
		}
		putchar('\n');
	}
}

/*
 *	Disassemble
 */
static void do_list(char *s)
{
	register int i;
	register WORD a;

	while (isspace((unsigned char) *s))
		s++;
	if (isxdigit((unsigned char) *s))
		wrk_addr = strtol(s, &s, 16);
	while (isspace((unsigned char) *s))
		s++;
	if (*s == ',') {
		s++;
		while (isspace((unsigned char) *s))
			s++;
		if (isxdigit((unsigned char) *s)) {
			a = strtol(s, NULL, 16);
			if (a < wrk_addr)
				a = wrk_addr;
			while (wrk_addr <= a) {
				printf("%04x - ", (unsigned int) wrk_addr);
				wrk_addr += disass(wrk_addr);
			}
			return;
		}
	}
	for (i = 0; i < 10; i++) {
		printf("%04x - ", (unsigned int) wrk_addr);
		wrk_addr += disass(wrk_addr);
	}
}

/*
 *	Memory modify
 */
static void do_modify(char *s)
{
	while (isspace((unsigned char) *s))
		s++;
	if (isxdigit((unsigned char) *s))
		wrk_addr = strtol(s, NULL, 16);
	while (true) {
		printf("%04x = %02x : ", (unsigned int) wrk_addr,
		       getmem(wrk_addr));
		if (!get_cmdline(arg, LENCMD) || arg[0] == '\0')
			break;
		s = arg;
		while (isspace((unsigned char) *s))
			s++;
		if (*s == '\0') {
			wrk_addr++;
			continue;
		}
		if (isxdigit((unsigned char) *s))
			putmem(wrk_addr++, strtol(s, NULL, 16));
		else
			break;
	}
}

/*
 *	Memory fill
 */
static void do_fill(char *s)
{
	register WORD a;
	register int i;
	register BYTE val;

	while (isspace((unsigned char) *s))
		s++;
	if (!isxdigit((unsigned char) *s)) {
		puts("address missing");
		return;
	}
	a = strtol(s, &s, 16);
	while (isspace((unsigned char) *s))
		s++;
	if (*s == ',') {
		s++;
		while (isspace((unsigned char) *s))
			s++;
		if (!isxdigit((unsigned char) *s)) {
			puts("count missing");
			return;
		}
	} else {
		puts("count missing");
		return;
	}
	i = strtol(s, &s, 16);
	while (isspace((unsigned char) *s))
		s++;
	if (*s == ',') {
		s++;
		while (isspace((unsigned char) *s))
			s++;
		if (!isxdigit((unsigned char) *s)) {
			puts("value missing");
			return;
		}
	} else {
		puts("value missing");
		return;
	}
	val = strtol(s, NULL, 16);
	while (i--)
		putmem(a++, val);
}

/*
 *	Memory move
 */
static void do_move(char *s)
{
	register WORD a1, a2;
	register int count;

	while (isspace((unsigned char) *s))
		s++;
	if (!isxdigit((unsigned char) *s)) {
		puts("from missing");
		return;
	}
	a1 = strtol(s, &s, 16);
	while (isspace((unsigned char) *s))
		s++;
	if (*s == ',') {
		s++;
		while (isspace((unsigned char) *s))
			s++;
		if (!isxdigit((unsigned char) *s)) {
			puts("to missing");
			return;
		}
	} else {
		puts("to missing");
		return;
	}
	a2 = strtol(s, &s, 16);
	while (isspace((unsigned char) *s))
		s++;
	if (*s == ',') {
		s++;
		while (isspace((unsigned char) *s))
			s++;
		if (!isxdigit((unsigned char) *s)) {
			puts("count missing");
			return;
		}
	} else {
		puts("count missing");
		return;
	}
	count = strtol(s, NULL, 16);
	while (count--)
		putmem(a2++, getmem(a1++));
}

/*
 *	Port modify
 */
static void do_port(char *s)
{
	register BYTE port, data;

	while (isspace((unsigned char) *s))
		s++;
	if (!isxdigit((unsigned char) *s)) {
		puts("port missing");
		return;
	}
	port = strtol(s, NULL, 16);
	cpu_error = NONE;
	data = io_in(port, 0);
	report_cpu_error();
	printf("%02x = %02x : ", port, data);
	if (get_cmdline(arg, LENCMD)) {
		s = arg;
		while (isspace((unsigned char) *s))
			s++;
		if (isxdigit((unsigned char) *s)) {
			data = (BYTE) strtol(s, NULL, 16);
			cpu_error = NONE;
			io_out(port, 0, data);
			report_cpu_error();
		}
	}
}

/*
 *	definition of register types
 */
#define R_8	1		/* 8-bit register */
#define R_88	2		/* 8-bit register pair */
#define R_16	3		/* 16-bit register */
#define R_R	4		/* R register */
#define R_F	5		/* F or F' register */
#define R_M	6		/* status register flag mask */

/*
 *	register definitions table (must be sorted by name length)
 */
typedef struct reg_def {
	const char *name;	/* register name */
	char len;		/* register name length */
	const char *prt;	/* printable register name */
	char z80;		/* Z80 only flag */
	char type;		/* register type */
	union {
		BYTE *r8;	/* 8-bit register pointer */
		struct {	/* 8-bit register pair pointers */
			BYTE *r8h;
			BYTE *r8l;
		};
		WORD *r16;	/* 16-bit register pointer */
		int *rf;	/* F or F' register pointer */
		BYTE rm;	/* status register flag mask */
	};
} reg_def_t;

static reg_def_t const regs[] = {
#ifndef EXCLUDE_Z80
	{ "bc'", 3, "BC'", 1, R_88, .r8h = &B_, .r8l = &C_ },
	{ "de'", 3, "DE'", 1, R_88, .r8h = &D_, .r8l = &E_ },
	{ "hl'", 3, "HL'", 1, R_88, .r8h = &H_, .r8l = &L_ },
#endif
	{ "pc",  2, "PC",  0, R_16, .r16 = &PC },
	{ "bc",  2, "BC",  0, R_88, .r8h = &B, .r8l = &C },
	{ "de",  2, "DE",  0, R_88, .r8h = &D, .r8l = &E },
	{ "hl",  2, "HL",  0, R_88, .r8h = &H, .r8l = &L },
#ifndef EXCLUDE_Z80
	{ "ix",  2, "IX",  1, R_16, .r16 = &IX },
	{ "iy",  2, "IY",  1, R_16, .r16 = &IY },
#endif
	{ "sp",  2, "SP",  0, R_16, .r16 = &SP },
	{ "fs",  2, "S",   0, R_M,  .rm = S_FLAG },
	{ "fz",  2, "Z",   0, R_M,  .rm = Z_FLAG },
	{ "fh",  2, "H",   0, R_M,  .rm = H_FLAG },
	{ "fp",  2, "P",   0, R_M,  .rm = P_FLAG },
#ifndef EXCLUDE_Z80
	{ "fn",  2, "N",   1, R_M,  .rm = N_FLAG },
#endif
	{ "fc",  2, "C",   0, R_M,  .rm = C_FLAG },
#ifndef EXCLUDE_Z80
	{ "a'",  2, "A'",  1, R_8,  .r8 = &A_ },
#if !defined(ALT_I8080) && !defined(ALT_Z80)
	{ "f'",  2, "F'",  1, R_F,  .rf = &F_ },
#else
	{ "f'",  2, "F'",  1, R_8,  .r8 = &F_ },
#endif
	{ "b'",  2, "B'",  1, R_8,  .r8 = &B_ },
	{ "c'",  2, "C'",  1, R_8,  .r8 = &C_ },
	{ "d'",  2, "D'",  1, R_8,  .r8 = &D_ },
	{ "e'",  2, "E'",  1, R_8,  .r8 = &E_ },
	{ "h'",  2, "H'",  1, R_8,  .r8 = &H_ },
	{ "l'",  2, "L'",  1, R_8,  .r8 = &L_ },
	{ "i",   1, "I",   1, R_8,  .r8 = &I },
	{ "r",   1, "R",   1, R_R,  .r8h = &R_, .r8l = &R },
#endif
	{ "a",   1, "A",   0, R_8,  .r8 = &A },
#if !defined(ALT_I8080) && !defined(ALT_Z80)
	{ "f",   1, "F",   0, R_F,  .rf = &F },
#else
	{ "f",   1, "F",   0, R_8,  .r8 = &F },
#endif
	{ "b",   1, "B",   0, R_8,  .r8 = &B },
	{ "c",   1, "C",   0, R_8,  .r8 = &C },
	{ "d",   1, "D",   0, R_8,  .r8 = &D },
	{ "e",   1, "E",   0, R_8,  .r8 = &E },
	{ "h",   1, "H",   0, R_8,  .r8 = &H },
	{ "l",   1, "L",   0, R_8,  .r8 = &L }
};
static int nregs = sizeof(regs) / sizeof(reg_def_t);

/*
 *	Register modify
 */
static void do_reg(char *s)
{
	register int i;
	register const reg_def_t *p;
	WORD w;

	while (isspace((unsigned char) *s))
		s++;
	if (*s != '\0') {
		for (i = 0, p = regs; i < nregs; i++, p++) {
#ifndef EXCLUDE_Z80
			if (p->z80 && cpu != Z80)
				continue;
#endif
			if (strncmp(s, p->name, p->len) == 0)
				break;
		}
		if (i < nregs) {
			switch (p->type) {
			case R_8:
				printf("%s = %02x : ", p->prt, *(p->r8));
				break;
			case R_88:
				printf("%s = %04x : ", p->prt,
				       (*(p->r8h) << 8) + *(p->r8l));
				break;
			case R_16:
				printf("%s = %04x : ", p->prt, *(p->r16));
				break;
			case R_R:
				printf("%s = %02x : ", p->prt,
				       (*(p->r8h) & 0x80) |
				       (*(p->r8l) & 0x7f));
				break;
			case R_F:
				printf("%s = %02x : ", p->prt, *(p->rf));
				break;
			case R_M:
				printf("%s-FLAG = %c : ", p->prt,
				       (F & p->rm) ? '1' : '0');
				break;
			default:
				break;
			}
			if (get_cmdline(arg, LENCMD)) {
				s = arg;
				while (isspace((unsigned char) *s))
					s++;
				if (isxdigit((unsigned char) *s)) {
					w = strtol(s, NULL, 16);
					switch (p->type) {
					case R_8:
						*(p->r8) = w & 0xff;
						break;
					case R_88:
						*(p->r8h) = (w >> 8) & 0xff;
						*(p->r8l) = w & 0xff;
						break;
					case R_16:
						*(p->r16) = w;
						break;
					case R_R:
						*(p->r8h) = *(p->r8l) =
							w & 0xff;
						break;
					case R_F:
						*(p->rf) = w & 0xff;
						break;
					case R_M:
						F = w ? (F | p->rm)
						      : (F & ~p->rm);
						break;
					default:
						break;
					}
				}
			}
		} else
			printf("unknown register %s", s);
	}
	print_head();
	print_reg();
}

/*
 *	Output header for the CPU registers
 */
static void print_head(void)
{
#ifndef EXCLUDE_Z80
	if (cpu == Z80)
		printf("\nPC   A  SZHPNC I  R  IFF BC   DE   HL   "
		       "A'F' B'C' D'E' H'L' IX   IY   SP\n");
#endif
#ifndef EXCLUDE_I8080
	if (cpu == I8080)
		puts("\nPC   A  SZHPC BC   DE   HL   SP");
#endif
}

/*
 *	Output all CPU registers
 */
static void print_reg(void)
{
	printf("%04x %02x ", PC, A);
	printf("%c", F & S_FLAG ? '1' : '0');
	printf("%c", F & Z_FLAG ? '1' : '0');
	printf("%c", F & H_FLAG ? '1' : '0');
	printf("%c", F & P_FLAG ? '1' : '0');
#ifndef EXCLUDE_Z80
	if (cpu == Z80) {
		printf("%c", F & N_FLAG ? '1' : '0');
		printf("%c", F & C_FLAG ? '1' : '0');
		printf(" %02x ", I);
		printf("%02x ", (R_ & 0x80) | (R & 0x7f));
		printf("%c", IFF & 1 ? '1' : '0');
		printf("%c", IFF & 2 ? '1' : '0');
		printf("  %02x%02x %02x%02x %02x%02x %02x%02x "
		       "%02x%02x %02x%02x %02x%02x %04x %04x %04x\n",
		       B, C, D, E, H, L, A_, F_,
		       B_, C_, D_, E_, H_, L_, IX, IY, SP);
	}
#endif
#ifndef EXCLUDE_I8080
	if (cpu == I8080) {
		printf("%c", F & C_FLAG ? '1' : '0');
		printf(" %02x%02x %02x%02x %02x%02x %04x\n",
		       B, C, D, E, H, L, SP);
	}
#endif
}

/*
 *	Software breakpoints
 */
static void do_break(char *s)
{
#if defined(SBSIZE) || defined(WANT_HB)
	WORD a;
	int n;
#ifdef SBSIZE
	register int i;
	int hdr_flag;
#endif
#endif

	if (*s == 'h') {
#ifndef WANT_HB
		puts("Sorry, no hardware breakpoint available");
		puts("Please recompile with WANT_HB defined in sim.h");
#else /* WANT_HB */
		s++;
		if (*s == '\n' || *s == '\0') {
			if (hb_flag) {
				printf("Hardware breakpoint set with ");
				n = 0;
				if (hb_mode & HB_READ) {
					printf("read");
					n = 1;
				}
				if (hb_mode & HB_WRITE) {
					if (n)
						putchar('/');
					printf("write");
					n = 1;
				}
				if (hb_mode & HB_EXEC) {
					if (n)
						putchar('/');
					printf("execute");
				}
				printf(" access trigger to %04x\n", hb_addr);
			} else
				puts("No hardware breakpoint set");
			return;
		}
		if (tolower((unsigned char) *s) == 'c') {
			hb_flag = false;
			return;
		}
		while (isspace((unsigned char) *s))
			s++;
		if (!isxdigit((unsigned char) *s)) {
			puts("address missing");
			return;
		}
		a = strtol(s, &s, 16);
		while (isspace((unsigned char) *s))
			s++;
		if (*s == ',') {
			s++;
			while (isspace((unsigned char) *s))
				s++;
			n = 0;
			if (tolower((unsigned char) *s) == 'r') {
				n |= HB_READ;
				s++;
			}
			if (tolower((unsigned char) *s) == 'w') {
				n |= HB_WRITE;
				s++;
			}
			if (tolower((unsigned char) *s) == 'x')
				n |= HB_EXEC;
		} else
			n = HB_READ | HB_WRITE | HB_EXEC;
#ifdef SBSIZE
		if (n & HB_EXEC) {
			for (i = 0; i < SBSIZE; i++)
				if (soft[i].sb_pass && soft[i].sb_addr == a) {
					puts("Software breakpoint set "
					     "at same execute access address");
					return;
				}
		}
#endif
		hb_addr = a;
		hb_mode = n;
		hb_flag = true;
#endif /* WANT_HB */
		return;
	}
#ifndef SBSIZE
	puts("Sorry, no software breakpoints available");
	puts("Please recompile with SBSIZE defined in sim.h");
#else /* SBSIZE */
	if (*s == '\n' || *s == '\0') {
		hdr_flag = 0;
		for (i = 0; i < SBSIZE; i++)
			if (soft[i].sb_pass) {
				if (!hdr_flag) {
					puts("Addr Pass  Counter");
					hdr_flag = 1;
				}
				printf("%04x %05d %05d\n",
				       soft[i].sb_addr, soft[i].sb_pass,
				       soft[i].sb_passcount);
			}
		if (!hdr_flag)
			puts("No software breakpoints set");
		return;
	}
	if (tolower((unsigned char) *s) == 'c') {
		s++;
		while (isspace((unsigned char) *s))
			s++;
		if (*s == '\0') {
			memset((char *) soft, 0, sizeof(softbreak_t) * SBSIZE);
			return;
		}
		if (!isxdigit((unsigned char) *s)) {
			puts("address missing");
			return;
		}
		a = strtol(s, NULL, 16);
		for (i = 0; i < SBSIZE; i++) {
			if (soft[i].sb_pass && soft[i].sb_addr == a)
				break;
		}
		if (i == SBSIZE)
			printf("No software breakpoint at address %04x\n", a);
		else
			memset((char *) &soft[i], 0, sizeof(softbreak_t));
		return;
	}
	while (isspace((unsigned char) *s))
		s++;
	if (!isxdigit((unsigned char) *s)) {
		puts("address missing");
		return;
	}
	a = strtol(s, &s, 16);
	/* look for existing breakpoint */
	for (i = 0; i < SBSIZE; i++) {
		if (soft[i].sb_pass && soft[i].sb_addr == a)
			break;
	}
	/* if not found, look for free one */
	if (i == SBSIZE) {
		for (i = 0; i < SBSIZE; i++)
			if (!soft[i].sb_pass)
				break;
	}
	if (i == SBSIZE)
		puts("All software breakpoints in use");
	else {
		if (!soft[i].sb_pass) {
			/* new breakpoint */
#ifdef WANT_HB
			if (hb_flag && (hb_mode & HB_EXEC) && hb_addr == a) {
				puts("Hardware execute access breakpoint set "
				     "at same address");
				return;
			}
#endif
			soft[i].sb_addr = a;
		}
		while (isspace((unsigned char) *s))
			s++;
		if (*s == ',') {
			s++;
			while (isspace((unsigned char) *s))
				s++;
			if (!isdigit((unsigned char) *s))
				n = 1;
			else {
				n = atoi(s);
				if (n == 0)
					n = 1;
			}
		} else
			n = 1;
		soft[i].sb_pass = n;
		soft[i].sb_passcount = 0;
	}
#endif /* SBSIZE */
}

/*
 *	History
 */
static void do_hist(char *s)
{
#ifndef HISIZE
	UNUSED(s);

	puts("Sorry, no history available");
	puts("Please recompile with HISIZE defined in sim.h");
#else /* HISIZE */
	int i, l, b, e, sa;

	if (tolower((unsigned char) *s) == 'c') {
		memset((char *) his, 0, sizeof(history_t) * HISIZE);
		h_next = 0;
		h_flag = false;
		return;
	}
	if (h_next == 0 && !h_flag) {
		puts("History memory is empty");
		return;
	}
	e = h_next;
	b = h_flag ? h_next + 1 : 0;
	l = 0;
	while (isspace((unsigned char) *s))
		s++;
	if (isxdigit((unsigned char) *s))
		sa = strtol(s, NULL, 16);
	else
		sa = -1;
	for (i = b; i != e; i++) {
		if (i == HISIZE)
			i = 0;
		if (sa != -1) {
			if (his[i].h_addr < sa)
				continue;
			else
				sa = -1;
		}
#ifndef EXCLUDE_Z80
		if (his[i].h_cpu == Z80) {
			printf("%04x AF=%04x BC=%04x DE=%04x HL=%04x "
			       "IX=%04x IY=%04x SP=%04x\n",
			       his[i].h_addr, his[i].h_af, his[i].h_bc,
			       his[i].h_de, his[i].h_hl, his[i].h_ix,
			       his[i].h_iy, his[i].h_sp);
		}
#endif
#ifndef EXCLUDE_I8080
		if (his[i].h_cpu == I8080) {
			printf("%04x AF=%04x BC=%04x DE=%04x HL=%04x "
			       "SP=%04x\n",
			       his[i].h_addr, his[i].h_af, his[i].h_bc,
			       his[i].h_de, his[i].h_hl, his[i].h_sp);
		}
#endif
		if (++l < 20)
			continue;
		l = 0;
		fputs("q = quit, else continue: ", stdout);
		fflush(stdout);
		if (get_cmdline(arg, LENCMD) &&
		    (arg[0] == '\0' || tolower((unsigned char) arg[0]) == 'q'))
			break;
	}
#endif /* HISIZE */
}

/*
 *	Runtime measurement by counting the executed T states
 */
static void do_count(char *s)
{
#ifndef WANT_TIM
	UNUSED(s);

	puts("Sorry, no t-state count available");
	puts("Please recompile with WANT_TIM defined in sim.h");
#else
	WORD start;

	while (isspace((unsigned char) *s))
		s++;
	if (*s == '\0') {
		puts("start  stop  status  T-states");
		printf("%04x   %04x    %s   %" PRIu64 "\n",
		       t_start, t_end,
		       t_flag ? "on " : "off", t_states_e - t_states_s);
		return;
	}
	if (!isxdigit((unsigned char) *s)) {
		puts("start missing");
		return;
	}
	start = strtol(s, &s, 16);
	while (isspace((unsigned char) *s))
		s++;
	if (*s == ',') {
		s++;
		while (isspace((unsigned char) *s))
			s++;
		if (!isxdigit((unsigned char) *s)) {
			puts("stop missing");
			return;
		}
	} else {
		puts("stop missing");
		return;
	}
	t_start = start;
	t_end = strtol(s, NULL, 16);
	t_states_s = t_states_e = T;
	t_flag = false;
#endif
}

#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
/*
 *	Switch between CPU modes
 */
static void do_switch(char *s)
{
	int old_cpu = cpu;

	while (isspace((unsigned char) *s))
		s++;
	if (*s == '\0') {
		if (cpu == Z80) {
			switch_cpu(I8080);
			puts("Switched to 8080 mode");
		} else {
			switch_cpu(Z80);
			puts("Switched to Z80 mode");
		}
	} else if (tolower((unsigned char) *s) == 'z') {
		if (cpu == Z80)
			puts("Already in Z80 mode");
		else {
			switch_cpu(Z80);
			puts("Switched to Z80 mode");
		}
	} else if (*s == '8') {
		if (cpu == I8080)
			puts("Already in 8080 mode");
		else {
			switch_cpu(I8080);
			puts("Switched to 8080 mode");
		}
	} else
		puts("Unsupported CPU mode");
	if (old_cpu != cpu) {
		print_head();
		print_reg();
	}
}
#endif /* !EXCLUDE_I8080 && !EXCLUDE_Z80 */

/*
 *	Toggle trap on undocumented op-codes
 */
static void do_uflag(void)
{
#ifdef UNDOC_INST
	u_flag = !u_flag;
	printf("Undocumented op-codes are now %s\n",
	       u_flag ? "trapped" : "executed");
#else
	puts("Undocumented op-codes are always trapped");
#endif
}

/*
 *	Toggle trap on undefined ports I/O
 */
static void do_iflag(void)
{
	i_flag = !i_flag;
	printf("Undefined ports I/O is now %s\n",
	       i_flag ? "trapped" : "allowed");
}

/*
 *	Output information about compiling options
 */
static void do_show(void)
{
	register int i;

#ifndef EXCLUDE_Z80
	if (cpu == Z80)
		printf("z80");
#endif
#ifndef EXCLUDE_I8080
	if (cpu == I8080)
		printf("8080");
#endif
	printf("sim Release: %s\n", RELEASE);
#ifdef HISIZE
	printf("No. of entries in history memory: %d\n", HISIZE);
#else
	puts("History not available");
#endif
#ifdef SBSIZE
	printf("No. of software breakpoints: %d\n", SBSIZE);
#else
	puts("Software breakpoints not available");
#endif
#ifdef WANT_HB
	i = 1;
#else
	i = 0;
#endif
	printf("Hardware breakpoint %savailable\n", i ? "" : "not ");
#ifdef UNDOC_INST
	printf("Undocumented op-codes are %s\n",
	       u_flag ? "trapped" : "executed");
#else
	puts("Undocumented op-codes are always trapped");
#endif
	printf("Undefined ports I/O is %s\n", i_flag ? "trapped" : "allowed");
#ifdef WANT_TIM
	i = 1;
#else
	i = 0;
#endif
	printf("T-State counting %spossible\n", i ? "" : "not ");
}

/*
 *	Output help text
 */
static void do_help(void)
{
	puts("d [from][,to]             dump memory");
	puts("l [from][,to]             list memory");
	puts("m [address]               modify memory");
	puts("f address,count,value     fill memory");
	puts("v from,to,count           move memory");
	puts("p address                 show/modify port");
	puts("g [*][address]            run program (* = with timing)");
	puts("t [count]                 trace program");
	puts("return                    single step program");
	puts("x [register]              show/modify register");
	puts("x f<flag>                 modify flag");
	puts("b address[,pass]          set software breakpoint");
	puts("b                         show software breakpoints");
	puts("bc [address]              clear software breakpoint(s)");
	puts("bh address[,accmode]      set hardware breakpoint");
	puts("bh                        show hardware breakpoint");
	puts("bhc                       clear hardware breakpoint");
	puts("h [address]               show history");
	puts("hc                        clear history");
	puts("z start,stop              set trigger addr for t-state count");
	puts("z                         show t-state count");
	puts("u                         toggle trap on undocumented op-codes");
	puts("i                         toggle trap on undefined ports I/O");
	puts("s                         show settings");
#if !defined (EXCLUDE_I8080) && !defined(EXCLUDE_Z80)
	puts("8                         toggle between Z80 & 8080 mode");
	puts("8 [z|8]                   switch to Z80 or 8080 mode");
#endif
#ifndef BAREMETAL
	puts("c                         measure clock frequency");
	puts("r filename[,address]      read object into memory");
	puts("! command                 execute external command");
#endif
	if (ice_cust_help)
		(*ice_cust_help)();
	puts("q                         quit");
}

#ifndef BAREMETAL

/*
 *	Calculate the clock frequency of the emulated CPU:
 *	into memory locations 0000H to 0002H the following
 *	code will be stored:
 *		LOOP: JP LOOP
 *	It uses 10 T states for each execution. A 3 second
 *	timer is started and then the CPU. For every JP the
 *	T states counter is incremented by 10 and after
 *	the timer is down and stops the emulation, the clock
 *	speed of the CPU in MHz is calculated with:
 *		f = (T - T0) / 3000000
 */
static void do_clock(void)
{
	BYTE save[3];
	WORD save_PC;
	Tstates_t T0;
	const char *s = NULL;
	struct sigaction newact;
	struct itimerval tim;
	unsigned freq;
#ifdef WANT_HB
	bool save_hb_flag;

	save_hb_flag = hb_flag;
	hb_flag = false;
#endif
	save[0] = getmem(0x0000);	/* save memory locations */
	save[1] = getmem(0x0001);	/* 0000H - 0002H */
	save[2] = getmem(0x0002);
	putmem(0x0000, 0xc3);		/* store opcode JP 0000H at address */
	putmem(0x0001, 0x00);		/* 0000H */
	putmem(0x0002, 0x00);
	save_PC = PC;			/* save PC */
	PC = 0;				/* set PC to this code */
	T0 = T;				/* remember start clock counter */
	newact.sa_handler = timeout;	/* set timer interrupt handler */
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGALRM, &newact, NULL);
	tim.it_value.tv_sec = 3;	/* start 3 second timer */
	tim.it_value.tv_usec = 0;
	tim.it_interval.tv_sec = 0;
	tim.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &tim, NULL);
	run_cpu();			/* start CPU */
	newact.sa_handler = SIG_DFL;	/* reset timer interrupt handler */
	sigaction(SIGALRM, &newact, NULL);
	PC = save_PC;			/* restore PC */
	putmem(0x0000, save[0]);	/* restore memory locations */
	putmem(0x0001, save[1]);	/* 0000H - 0002H */
	putmem(0x0002, save[2]);
#ifdef WANT_HB
	hb_flag = save_hb_flag;
#endif
#ifndef EXCLUDE_Z80
	if (cpu == Z80)
		s = "JP";
#endif
#ifndef EXCLUDE_I8080
	if (cpu == I8080)
		s = "JMP";
#endif
	if (cpu_error == NONE) {
		freq = (unsigned) ((T - T0) / 30000);
		printf("CPU executed %" PRIu64 " %s instructions "
		       "in 3 seconds\n", (T - T0) / 10, s);
		printf("clock frequency = %5u.%02u MHz\n",
		       freq / 100, freq % 100);
	} else
		puts("Interrupted by user");
}

/*
 *	This function is the signal handler for the timer interrupt.
 *	The CPU emulation is stopped here.
 */
static void timeout(int sig)
{
	UNUSED(sig);

	cpu_state = ST_STOPPED;
}

/*
 *	Load a file into the memory of the emulated CPU
 */
static void do_load(char *s)
{
	static char fn[MAX_LFN];
	char *pfn = fn;

	while (isspace((unsigned char) *s))
		s++;
	while (*s != ',' && *s != '\n' && *s != '\0')
		*pfn++ = *s++;
	*pfn = '\0';
	if (*s == ',') {
		s++;
		while (isspace((unsigned char) *s))
			s++;
		if (isxdigit((unsigned char) *s)) {
			if (load_file(fn, strtol(s, NULL, 16), -1))
				wrk_addr = PC;
			return;
		}
	}
	if (load_file(fn, 0, 0))
		wrk_addr = PC;
}

/*
 *	Call system function from simulator
 */
static void do_unix(char *s)
{
	int_off();
	if (system(s) == -1)
		perror("external command");
	int_on();
}

#endif /* !BAREMETAL */

#endif /* WANT_ICE */
