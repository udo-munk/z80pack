/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This module contains the user interface, a full qualified ICE,
 * for the Z80-CPU simulation.
 *
 * History:
 * 15-SEP-19 (Mike Douglas) Created from simctl.c in the z80sim
 *		source directory. Added unix_terminal code to emulate 
 * 		console I/O for the Mostek AID-80F and SYS-80FT computers
 * 27-SEP-19 (Udo Munk) fix double loading of ROM
 * 30-SEP-19 (Mike Douglas) accept also upper case
 */

/*
 *	This module is an ICE type user interface to debug Z80 programs
 *	on a host system.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <memory.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "memory.h"
#include "../../iodevices/mostek-fdc.h"
#include "../../iodevices/unix_terminal.h"

extern void cpu_z80(void), cpu_8080(void);
extern void disass(unsigned char **, int);
extern int exatoi(char *);
extern int getkey(void);
extern void int_on(void), int_off(void);
extern int load_file(char *, BYTE, WORD);

static void do_step(void);
static void do_trace(char *);
static void do_go(char *);
static int handel_break(void);
static void do_dump(char *);
static void do_list(char *);
static void do_modify(char *);
static void do_fill(char *);
static void do_move(char *);
static void do_port(char *);
static void do_reg(char *);
static void print_head(void);
static void print_reg(void);
static void do_break(char *);
static void do_hist(char *);
static void do_count(char *);
static void do_clock(void);
static void timeout(int);
static void do_show(void);
static void do_unix(char *);
static void do_help(void);
static void cpu_err_msg(void);

extern BYTE *wrk_ram;
extern struct termios old_term;

/*
 *	The function "mon()" is the dialog user interface, called
 *	from the simulation just after program start.
 */
void mon(void)
{
	register int eoj = 1;
	static char cmd[LENCMD];

	tcgetattr(0, &old_term);

	fdc_reset();
	if (x_flag)
		do_go("");

	while (eoj) {
		next:
		printf(">>> ");
		fflush(stdout);
		if (fgets(cmd, LENCMD, stdin) == NULL) {
			putchar('\n');
			goto next;
		}
		switch (tolower((unsigned char) *cmd)) {
		case '\n':
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
		case 'c':
			do_clock();
			break;
		case 's':
			do_show();
			break;
		case '?':
			do_help();
			break;
		case 'r':
			load_file(cmd + 1, 0, 0);
			break;
		case '!':
			do_unix(cmd + 1);
			break;
		case 'q':
			eoj = 0;
			break;
		default:
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
	BYTE *p;

	cpu_state = SINGLE_STEP;
	cpu_error = NONE;
	switch(cpu) {
	case Z80:
		cpu_z80();
		break;
	case I8080:
		cpu_8080();
		break;
	}
	if (cpu_error == OPHALT)
		handel_break();
	cpu_err_msg();
	print_head();
	print_reg();
	p = mem_base() + PC;
	disass(&p, PC);
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
	cpu_state = SINGLE_STEP;
	cpu_error = NONE;
	print_head();
	print_reg();
	for (i = 0; i < count; i++) {
		switch(cpu) {
		case Z80:
			cpu_z80();
			break;
		case I8080:
			cpu_8080();
			break;
		}
		print_reg();
		if (cpu_error) {
			if (cpu_error == OPHALT) {
				if (!handel_break()) {
					break;
				}
			} else
				break;
		}
	}
	cpu_err_msg();
}

/*
 *	Run the CPU emulation endless
 */
static void do_go(char *s)
{
	printf("Type CTRL-\\ to halt emulation\n\n");
	set_unix_terminal();

	while (isspace((unsigned char) *s))
		s++;
	if (isxdigit((unsigned char) *s))
		PC = exatoi(s);
	cont:
	cpu_state = CONTIN_RUN;
	cpu_error = NONE;
	switch(cpu) {
	case Z80:
		cpu_z80();
		break;
	case I8080:
		cpu_8080();
		break;
	}
	if (cpu_error == OPHALT)
		if (handel_break())
			if (!cpu_error)
				goto cont;

	/* reset terminal */
	reset_unix_terminal();

	cpu_err_msg();
	print_head();
	print_reg();
}

/*
 *	Handling of software breakpoints (HALT opcode):
 *
 *	Output:	0 breakpoint or other HALT opcode reached (stop)
 *		1 breakpoint reached, passcounter not reached (continue)
 */
static int handel_break(void)
{
#ifdef SBSIZE
	register int i;
	int break_address;

	for (i = 0; i < SBSIZE; i++)	/* search for breakpoint */
		if (soft[i].sb_adr == PC - 1)
			goto was_softbreak;
	return(0);
	was_softbreak:
#ifdef HISIZE
	h_next--;			/* correct history */
	if (h_next < 0)
		h_next = 0;
#endif
	break_address = PC - 1;		/* store adr of breakpoint */
	cpu_error = NONE;		/* HALT was a breakpoint */
	PC--;				/* substitute HALT opcode by */
	*(mem_base() + PC) = soft[i].sb_oldopc;	/* original opcode */
	cpu_state = SINGLE_STEP;	/* and execute it */
	switch(cpu) {
	case Z80:
		cpu_z80();
		break;
	case I8080:
		cpu_8080();
		break;
	}
	*(mem_base() + soft[i].sb_adr) = 0x76; /* restore HALT opcode again */
	soft[i].sb_passcount++;		/* increment passcounter */
	if (soft[i].sb_passcount != soft[i].sb_pass)
		return(1);		/* pass not reached, continue */
	printf("Software breakpoint %d reached at %04x\n", i, break_address);
	soft[i].sb_passcount = 0;	/* reset passcounter */
	return(0);			/* pass reached, stop */
#else
	return(0);
#endif
}

/*
 *	Memory dump
 */
static void do_dump(char *s)
{
	register int i, j;
	BYTE c;

	while (isspace((unsigned char) *s))
		s++;
	if (isxdigit((unsigned char) *s))
		wrk_ram = mem_base() + exatoi(s) - exatoi(s) % 16;
	printf("Adr    ");
	for (i = 0; i < 16; i++)
		printf("%02x ", i);
	puts(" ASCII");
	for (i = 0; i < 16; i++) {
		printf("%04x - ", (unsigned int)(wrk_ram - mem_base()));
		for (j = 0; j < 16; j++) {
			printf("%02x ", *wrk_ram);
			wrk_ram++;
			if (wrk_ram > mem_base() + 65535)
				wrk_ram = mem_base();
		}
		putchar('\t');
		for (j = -16; j < 0; j++)
			printf("%c", ((c = *(wrk_ram + j)) >= ' ' && c <= 0x7f)
			       ? c : '.');
		putchar('\n');
	}
}

/*
 *	Disassemble
 */
static void do_list(char *s)
{
	register int i;

	while (isspace((unsigned char) *s))
		s++;
	if (isxdigit((unsigned char) *s))
		wrk_ram = mem_base() + exatoi(s);
	for (i = 0; i < 10; i++) {
		printf("%04x - ", (unsigned int)(wrk_ram - mem_base()));
		disass(&wrk_ram, wrk_ram - mem_base());
		if (wrk_ram > mem_base() + 65535)
			wrk_ram = mem_base();
	}
}

/*
 *	Memory modify
 */
static void do_modify(char *s)
{
	static char nv[LENCMD];

	while (isspace((unsigned char) *s))
		s++;
	if (isxdigit((unsigned char) *s))
		wrk_ram = mem_base() + exatoi(s);
	for (;;) {
		printf("%04x = %02x : ", (unsigned int)(wrk_ram - mem_base()),
		       *wrk_ram);
		fgets(nv, sizeof(nv), stdin);
		if (nv[0] == '\n') {
			wrk_ram++;
			if (wrk_ram > mem_base() + 65535)
				wrk_ram = mem_base();
			continue;
		}
		if (!isxdigit((unsigned char) nv[0]))
			break;
		*wrk_ram++ = exatoi(nv);
		if (wrk_ram > mem_base() + 65535)
			wrk_ram = mem_base();
	}
}

/*
 *	Memory fill
 */
static void do_fill(char *s)
{
	register BYTE *p;
	register int i;
	register BYTE val;

	while (isspace((unsigned char) *s))
		s++;
	p = mem_base() + exatoi(s);
	while (*s != ',' && *s != '\0')
		s++;
	if (*s) {
		i = exatoi(++s);
	} else {
		puts("count missing");
		return;
	}
	while (*s != ',' && *s != '\0')
		s++;
	if (*s) {
		val = exatoi(++s);
	} else {
		puts("value missing");
		return;
	}
	while (i--) {
		*p++ = val;
		if (p > mem_base() + 65535)
			p = mem_base();
	}
}

/*
 *	Memory move
 */
static void do_move(char *s)
{
	register BYTE *p1, *p2;
	register int count;

	
	while (isspace((unsigned char) *s))
		s++;
	p1 = mem_base() + exatoi(s);
	while (*s != ',' && *s != '\0')
		s++;
	if (*s) {
		p2 = mem_base() + exatoi(++s);
	} else {
		puts("to missing");
		return;
	}
	while (*s != ',' && *s != '\0')
		s++;
	if (*s) {
		count = exatoi(++s);
	} else {
		puts("count missing");
		return;
	}
	while (count--) {
		*p2++ = *p1++;
		if (p1 > mem_base() + 65535)
			p1 = mem_base();
		if (p2 > mem_base() + 65535)
			p2 = mem_base();
	}
}

/*
 *	Port modify
 */
static void do_port(char *s)
{
	register BYTE port;
	static char nv[LENCMD];
	extern BYTE io_out(BYTE, BYTE, BYTE), io_in(BYTE, BYTE);

	while (isspace((unsigned char) *s))
		s++;
	port = exatoi(s);
	printf("%02x = %02x : ", port, io_in(port, 0));
	fgets(nv, sizeof(nv), stdin);
	if (isxdigit((unsigned char) *nv))
		io_out(port, 0, (BYTE) exatoi(nv));
}

/*
 *	Register modify
 */
static void do_reg(char *s)
{
	static char nv[LENCMD];

	while (isspace((unsigned char) *s))
		s++;
	if (*s == '\0') {
		print_head();
		print_reg();
	} else {
		if ((strncmp(s, "bc'", 3) == 0) && (cpu == Z80)) {
			printf("BC' = %04x : ", B_ * 256 + C_);
			fgets(nv, sizeof(nv), stdin);
			B_ = (exatoi(nv) & 0xffff) / 256;
			C_ = (exatoi(nv) & 0xffff) % 256;
		} else if ((strncmp(s, "de'", 3) == 0) && (cpu == Z80)) {
			printf("DE' = %04x : ", D_ * 256 + E_);
			fgets(nv, sizeof(nv), stdin);
			D_ = (exatoi(nv) & 0xffff) / 256;
			E_ = (exatoi(nv) & 0xffff) % 256;
		} else if ((strncmp(s, "hl'", 3) == 0) && (cpu == Z80)) {
			printf("HL' = %04x : ", H_ * 256 + L_);
			fgets(nv, sizeof(nv), stdin);
			H_ = (exatoi(nv) & 0xffff) / 256;
			L_ = (exatoi(nv) & 0xffff) % 256;
		} else if (strncmp(s, "pc", 2) == 0) {
			printf("PC = %04x : ", PC);
			fgets(nv, sizeof(nv), stdin);
			PC = (exatoi(nv) & 0xffff);
		} else if (strncmp(s, "bc", 2) == 0) {
			printf("BC = %04x : ", B * 256 + C);
			fgets(nv, sizeof(nv), stdin);
			B = (exatoi(nv) & 0xffff) / 256;
			C = (exatoi(nv) & 0xffff) % 256;
		} else if (strncmp(s, "de", 2) == 0) {
			printf("DE = %04x : ", D * 256 + E);
			fgets(nv, sizeof(nv), stdin);
			D = (exatoi(nv) & 0xffff) / 256;
			E = (exatoi(nv) & 0xffff) % 256;
		} else if (strncmp(s, "hl", 2) == 0) {
			printf("HL = %04x : ", H * 256 + L);
			fgets(nv, sizeof(nv), stdin);
			H = (exatoi(nv) & 0xffff) / 256;
			L = (exatoi(nv) & 0xffff) % 256;
		} else if ((strncmp(s, "ix", 2) == 0) && (cpu == Z80)) {
			printf("IX = %04x : ", IX);
			fgets(nv, sizeof(nv), stdin);
			IX = exatoi(nv) & 0xffff;
		} else if ((strncmp(s, "iy", 2) == 0) && (cpu == Z80)) {
			printf("IY = %04x : ", IY);
			fgets(nv, sizeof(nv), stdin);
			IY = exatoi(nv) & 0xffff;
		} else if (strncmp(s, "sp", 2) == 0) {
			printf("SP = %04x : ", SP);
			fgets(nv, sizeof(nv), stdin);
			SP = (exatoi(nv) & 0xffff);
		} else if (strncmp(s, "fs", 2) == 0) {
			printf("S-FLAG = %c : ", (F & S_FLAG) ? '1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F | S_FLAG) : (F & ~S_FLAG);
		} else if (strncmp(s, "fz", 2) == 0) {
			printf("Z-FLAG = %c : ", (F & Z_FLAG) ? '1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F | Z_FLAG) : (F & ~Z_FLAG);
		} else if (strncmp(s, "fh", 2) == 0) {
			printf("H-FLAG = %c : ", (F & H_FLAG) ? '1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F | H_FLAG) : (F & ~H_FLAG);
		} else if (strncmp(s, "fp", 2) == 0) {
			printf("P-FLAG = %c : ", (F & P_FLAG) ? '1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F | P_FLAG) : (F & ~P_FLAG);
		} else if ((strncmp(s, "fn", 2) == 0) && (cpu == Z80)) {
			printf("N-FLAG = %c : ", (F & N_FLAG) ? '1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F | N_FLAG) : (F & ~N_FLAG);
		} else if (strncmp(s, "fc", 2) == 0) {
			printf("C-FLAG = %c : ", (F & C_FLAG) ? '1' : '0');
			fgets(nv, sizeof(nv), stdin);
			F = (exatoi(nv)) ? (F | C_FLAG) : (F & ~C_FLAG);
		} else if (strncmp(s, "a'", 2) == 0) {
			printf("A' = %02x : ", A_);
			fgets(nv, sizeof(nv), stdin);
			A_ = exatoi(nv) & 0xff;
		} else if (strncmp(s, "f'", 2) == 0) {
			printf("F' = %02x : ", F_);
			fgets(nv, sizeof(nv), stdin);
			F_ = exatoi(nv) & 0xff;
		} else if ((strncmp(s, "b'", 2) == 0) && (cpu == Z80)) {
			printf("B' = %02x : ", B_);
			fgets(nv, sizeof(nv), stdin);
			B_ = exatoi(nv) & 0xff;
		} else if ((strncmp(s, "c'", 2) == 0) && (cpu == Z80)) {
			printf("C' = %02x : ", C_);
			fgets(nv, sizeof(nv), stdin);
			C_ = exatoi(nv) & 0xff;
		} else if ((strncmp(s, "d'", 2) == 0) && (cpu == Z80)) {
			printf("D' = %02x : ", D_);
			fgets(nv, sizeof(nv), stdin);
			D_ = exatoi(nv) & 0xff;
		} else if ((strncmp(s, "e'", 2) == 0) && (cpu == Z80)) {
			printf("E' = %02x : ", E_);
			fgets(nv, sizeof(nv), stdin);
			E_ = exatoi(nv) & 0xff;
		} else if ((strncmp(s, "h'", 2) == 0) && (cpu == Z80)) {
			printf("H' = %02x : ", H_);
			fgets(nv, sizeof(nv), stdin);
			H_ = exatoi(nv) & 0xff;
		} else if ((strncmp(s, "l'", 2) == 0) && (cpu == Z80)) {
			printf("L' = %02x : ", L_);
			fgets(nv, sizeof(nv), stdin);
			L_ = exatoi(nv) & 0xff;
		} else if ((strncmp(s, "i", 1) == 0) && (cpu == Z80)) {
			printf("I = %02x : ", I);
			fgets(nv, sizeof(nv), stdin);
			I = exatoi(nv) & 0xff;
		} else if (strncmp(s, "a", 1) == 0) {
			printf("A = %02x : ", A);
			fgets(nv, sizeof(nv), stdin);
			A = exatoi(nv) & 0xff;
		} else if (strncmp(s, "f", 1) == 0) {
			printf("F = %02x : ", F);
			fgets(nv, sizeof(nv), stdin);
			F = exatoi(nv) & 0xff;
		} else if (strncmp(s, "b", 1) == 0) {
			printf("B = %02x : ", B);
			fgets(nv, sizeof(nv), stdin);
			B = exatoi(nv) & 0xff;
		} else if (strncmp(s, "c", 1) == 0) {
			printf("C = %02x : ", C);
			fgets(nv, sizeof(nv), stdin);
			C = exatoi(nv) & 0xff;
		} else if (strncmp(s, "d", 1) == 0) {
			printf("D = %02x : ", D);
			fgets(nv, sizeof(nv), stdin);
			D = exatoi(nv) & 0xff;
		} else if (strncmp(s, "e", 1) == 0) {
			printf("E = %02x : ", E);
			fgets(nv, sizeof(nv), stdin);
			E = exatoi(nv) & 0xff;
		} else if (strncmp(s, "h", 1) == 0) {
			printf("H = %02x : ", H);
			fgets(nv, sizeof(nv), stdin);
			H = exatoi(nv) & 0xff;
		} else if (strncmp(s, "l", 1) == 0) {
			printf("L = %02x : ", L);
			fgets(nv, sizeof(nv), stdin);
			L = exatoi(nv) & 0xff;
		} else
			printf("can't change register %s\n", nv);
		print_head();
		print_reg();
	}
}

/*
 *	Output header for the CPU registers
 */
static void print_head(void)
{
	if (cpu == Z80)

	printf("\nPC   A  SZHPNC I  IFF BC   DE   HL   A'F' B'C' D'E' H'L' IX   IY   SP\n");

	else

	printf("\nPC   A  SZHPC BC   DE   HL   SP\n");
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
	if (cpu == Z80)
		printf("%c", F & N_FLAG ? '1' : '0');
	printf("%c", F & C_FLAG ? '1' : '0');
	if (cpu == Z80) {
		printf(" %02x ", I);
		printf("%c", IFF & 1 ? '1' : '0');
		printf("%c", IFF & 2 ? '1' : '0');
	}
	if (cpu == Z80) {
		printf("  %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %04x %04x %04x\n",
		 B, C, D, E, H, L, A_, F_, B_, C_, D_, E_, H_, L_, IX, IY, SP);
	} else {
		printf(" %02x%02x %02x%02x %02x%02x %04x\n",
		 B, C, D, E, H, L, SP);
	}
}

/*
 *	Software breakpoints
 */
static void do_break(char *s)
{
#ifndef SBSIZE
	s = s;	/* to avoid compiler warning */
	puts("Sorry, no breakpoints available");
	puts("Please recompile with SBSIZE defined in sim.h");
#else
	register int i;

	if (*s == '\n') {
		puts("No Addr Pass  Counter");
		for (i = 0; i < SBSIZE; i++)
			if (soft[i].sb_pass)
				printf("%02d %04x %05d %05d\n", i,
				       soft[i].sb_adr,soft[i].sb_pass,
				       soft[i].sb_passcount);
		return;
	}
	if (isxdigit((unsigned char) *s)) {
		i = atoi(s++);
		if (i >= SBSIZE) {
			printf("breakpoint %d not available\n", i);
			return;
		}
	} else {
		i = sb_next++;
		if (sb_next == SBSIZE)
			sb_next = 0;
	}
	while (isspace((unsigned char) *s))
		s++;
	if (*s == 'c') {
		*(mem_base() + soft[i].sb_adr) = soft[i].sb_oldopc;
		memset((char *) &soft[i], 0, sizeof(struct softbreak));
		return;
	}
	if (soft[i].sb_pass)
		*(mem_base() + soft[i].sb_adr) = soft[i].sb_oldopc;
	soft[i].sb_adr = exatoi(s);
	soft[i].sb_oldopc = *(mem_base() + soft[i].sb_adr);
	*(mem_base() + soft[i].sb_adr) = 0x76;
	while (!iscntrl((unsigned char) *s) && !ispunct((unsigned char) *s))
		s++;
	if (*s != ',')
		soft[i].sb_pass = 1;
	else
		soft[i].sb_pass = exatoi(++s);
	soft[i].sb_passcount = 0;
#endif
}

/*
 *	History
 */
static void do_hist(char *s)
{
#ifndef HISIZE
	s = s;	/* to avoid compiler warning */
	puts("Sorry, no history available");
	puts("Please recompile with HISIZE defined in sim.h");
#else
	int i, l, b, e, c, sa;

	while (isspace((unsigned char) *s))
		s++;
	switch (*s) {
	case 'c':
		memset((char *) his, 0, sizeof(struct history) * HISIZE);
		h_next = 0;
		h_flag = 0;
		break;
	default:
		if ((h_next == 0) && (h_flag == 0)) {
			puts("History memory is empty");
			break;
		}
		e = h_next;
		b = (h_flag) ? h_next + 1 : 0;
		l = 0;
		while (isspace((unsigned char) *s))
			s++;
		if (*s)
			sa = exatoi(s);
		else
			sa = -1;
		for (i = b; i != e; i++) {
			if (i == HISIZE)
				i = 0;
			if (sa != -1) {
				if (his[i].h_adr < sa)
					continue;
				else
					sa = -1;
			}
			if (cpu == Z80) {
				printf("%04x AF=%04x BC=%04x DE=%04x HL=%04x IX=%04x IY=%04x SP=%04x\n",
					his[i].h_adr, his[i].h_af, his[i].h_bc,
					his[i].h_de, his[i].h_hl, his[i].h_ix,
					his[i].h_iy, his[i].h_sp);
			} else {
				printf("%04x AF=%04x BC=%04x DE=%04x HL=%04x SP=%04x\n",
					his[i].h_adr, his[i].h_af, his[i].h_bc,
					his[i].h_de, his[i].h_hl, his[i].h_sp);
			}
			l++;
			if (l == 20) {
				l = 0;
				printf("q = quit, else continue: ");
				c = getkey();
				putchar('\n');
				if (toupper((unsigned char) c) == 'Q')
					break;
			}
		}
		break;
	}
#endif
}

/*
 *	Runtime measurement by counting the executed T states
 */
static void do_count(char *s)
{
#ifndef WANT_TIM
	s = s;	/* to avoid compiler warning */
	puts("Sorry, no t-state count available");
	puts("Please recompile with WANT_TIM defined in sim.h");
#else
	while (isspace((unsigned char) *s))
		s++;
	if (*s == '\0') {
		puts("start  stop  status  T-states");
		printf("%04x   %04x    %s   %lu\n",
		       t_start, t_end,
		       t_flag ? "on ": "off", t_states);
	} else {
		t_start = exatoi(s);
		while (*s != ',' && *s != '\0')
			s++;
		if (*s)
			t_end = exatoi(++s);
		t_states = 0L;
		t_flag = 0;
	}
#endif
}

/*
 *	Calculate the clock frequency of the emulated CPU:
 *	into memory locations 0000H to 0002H the following
 *	code will be stored:
 *		LOOP: JP LOOP
 *	It uses 10 T states for each execution. A 3 second
 *	timer is started and then the CPU. For every opcode
 *	fetch the R register is incremented by one and after
 *	the timer is down and stops the emulation, the clock
 *	speed of the CPU is calculated with:
 *		f = R / 300000
 */
static void do_clock(void)
{
	BYTE save[3];
	static struct sigaction newact;
	static struct itimerval tim;

	save[0] = *(mem_base() + 0x0000); /* save memory locations */
	save[1] = *(mem_base() + 0x0001); /* 0000H - 0002H */
	save[2] = *(mem_base() + 0x0002);
	*(mem_base() + 0x0000) = 0xc3;	/* store opcode JP 0000H at address */
	*(mem_base() + 0x0001) = 0x00;	/* 0000H */
	*(mem_base() + 0x0002) = 0x00;
	PC = 0;				/* set PC to this code */
	R = 0L;				/* clear refresh register */
	cpu_state = CONTIN_RUN;		/* initialise CPU */
	cpu_error = NONE;
	newact.sa_handler = timeout;	/* set timer interrupt handler */
	memset((void *) &newact.sa_mask, 0, sizeof(newact.sa_mask));
	newact.sa_flags = 0;
	sigaction(SIGALRM, &newact, NULL);
	tim.it_value.tv_sec = 3;	/* start 3 second timer */
	tim.it_value.tv_usec = 0;
	tim.it_interval.tv_sec = 0;
	tim.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &tim, NULL);
	switch(cpu) {			/* start CPU */
	case Z80:
		cpu_z80();
		break;
	case I8080:
		cpu_8080();
		break;
	}
	newact.sa_handler = SIG_DFL;	/* reset timer interrupt handler */
	sigaction(SIGALRM, &newact, NULL);
	*(mem_base() + 0x0000) = save[0]; /* restore memory locations */
	*(mem_base() + 0x0001) = save[1]; /* 0000H - 0002H */
	*(mem_base() + 0x0002) = save[2];
	if (cpu_error == NONE)
		printf("clock frequency = %5.2f Mhz\n", ((float) R) / 300000.0);
	else
		puts("Interrupted by user");
}

/*
 *	This function is the signal handler for the timer interrupt.
 *	The CPU emulation is stopped here.
 */
static void timeout(int sig)
{
	sig = sig;	/* to avoid compiler warning */

	cpu_state = STOPPED;
}

/*
 *	Output informations about compiling options
 */
static void do_show(void)
{
	register int i;

	printf("Release: %s\n", RELEASE);
#ifdef HISIZE
	i = HISIZE;
#else
	i = 0;
#endif
	printf("No. of entries in history memory: %d\n", i);
#ifdef SBSIZE
	i = SBSIZE;
#else
	i = 0;
#endif
	printf("No. of software breakpoints: %d\n", i);
#ifdef Z80_UNDOC
	i = u_flag;
#else
	i = 1;
#endif
	printf("Undocumented op-codes %sexecuted\n", i ? "not " : "");
#ifdef WANT_TIM
	i = 1;
#else
	i = 0;
#endif
	printf("T-State counting %spossible\n", i ? "" : "im");
}

/*
 *	Call system function from simulator
 */
static void do_unix(char *s)
{
	int_off();
	system(s);
	int_on();
}

/*
 *	Output help text
 */
static void do_help(void)
{
	puts("r filename[,address]      read object into memory");
	puts("d [address]               dump memory");
	puts("l [address]               list memory");
	puts("m [address]               modify memory");
	puts("f address,count,value     fill memory");
	puts("v from,to,count           move memory");
	puts("p address                 show/modify port");
	puts("g [address]               run program");
	puts("t [count]                 trace program");
	puts("return                    single step program");
	puts("x [register]              show/modify register");
	puts("x f<flag>                 modify flag");
	puts("b[no] address[,pass]      set soft breakpoint");
	puts("b                         show soft breakpoints");
	puts("b[no] c                   clear soft breakpoint");
	puts("h [address]               show history");
	puts("h c                       clear history");
	puts("z start,stop              set trigger adr for t-state count");
	puts("z                         show t-state count");
	puts("c                         measure clock frequency");
	puts("s                         show settings");
	puts("! command                 execute UNIX command");
	puts("q                         quit");
}

/*
 *	Error handler after CPU is stopped
 */
static void cpu_err_msg(void)
{
	switch (cpu_error) {
	case NONE:
		break;
	case OPHALT:
		printf("HALT Op-Code reached at %04x\n", PC - 1);
		break;
	case IOTRAPIN:
		printf("I/O input Trap at %04x, port %02x\n", PC, io_port);
		break;
	case IOTRAPOUT:
		printf("I/O output Trap at %04x, port %02x\n", PC, io_port);
		break;
	case IOHALT:
		printf("\nSystem halted, bye.\n");
		break;
	case IOERROR:
		printf("Fatal I/O Error at %04x\n", PC);
		break;
	case OPTRAP1:
		printf("Op-code trap at %04x: %02x\n", PC - 1,
		       *(mem_base() + PC - 1));
		break;
	case OPTRAP2:
		printf("Op-code trap at %04x: %02x %02x\n", PC - 2,
		       *(mem_base() + PC - 2), *(mem_base() + PC - 1));
		break;
	case OPTRAP4:
		printf("Op-code trap at %04x: %02x %02x %02x %02x\n",
		       PC - 4, *(mem_base() + PC - 4), *(mem_base() + PC - 3),
		       *(mem_base() + PC - 2), *(mem_base() + PC - 1));
		break;
	case USERINT:
		puts("User Interrupt");
		break;
	case POWEROFF:
		break;
	default:
		printf("Unknown error %d\n", cpu_error);
		break;
	}
}
