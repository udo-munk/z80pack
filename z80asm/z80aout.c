/*
 *	Z80 - Macro - Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022 by Thomas Eberhardt
 *
 *	History:
 *	17-SEP-1987 Development under Digital Research CP/M 2.2
 *	28-JUN-1988 Switched to Unix System V.3
 *	21-OCT-2006 changed to ANSI C for modern POSIX OS's
 *	03-FEB-2007 more ANSI C conformance and reduced compiler warnings
 *	18-MAR-2007 use default output file extension dependent on format
 *	04-OCT-2008 fixed comment bug, ';' string argument now working
 *	22-FEB-2014 fixed is...() compiler warnings
 *	13-JAN-2016 fixed buffer overflow, new expression parser from Didier
 *	02-OCT-2017 bug fixes in expression parser from Didier
 *	28-OCT-2017 added variable symbol length and other improvements
 *	15-MAY-2018 mark unreferenced symbols in listing
 *	30-JUL-2021 fix verbose option
 *	28-JAN-2022 added syntax check for OUT (n),A
 *	24-SEP-2022 added undocumented Z80 instructions and 8080 mode (TE)
 *	04-OCT-2022 new expression parser (TE)
 *	25-OCT-2022 Intel-like macros (TE)
 */

/*
 *	module for output functions to list, object and error files
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "z80a.h"
#include "z80aglb.h"

void fill_bin(void);
void eof_hex(void);
void flush_hex(void);
void hex_record(int);
int chksum(int);
void btoh(unsigned char, char **);

/* z80amain.c */
extern void fatal(int, const char *);

/* z80amfun.c */
extern char *mac_lst_first(int, int *);
extern char *mac_lst_next(int, int *);

static const char *errmsg[] = {		/* error messages for asmerr() */
	"no error",			/* 0 */
	"invalid opcode",		/* 1 */
	"invalid operand",		/* 2 */
	"missing operand",		/* 3 */
	"multiple defined symbol",	/* 4 */
	"undefined symbol",		/* 5 */
	"value out of range",		/* 6 */
	"missing right parenthesis",	/* 7 */
	"missing string delimiter",	/* 8 */
	"non-sequential object code",	/* 9 */
	"missing IF",			/* 10 */
	"IF nested too deep",		/* 11 */
	"missing ENDIF",		/* 12 */
	"INCLUDE nested too deep",	/* 13 */
	".PHASE can not be nested",	/* 14 */
	"ORG in .PHASE block",		/* 15 */
	"missing .PHASE",		/* 16 */
	"division by zero",		/* 17 */
	"invalid expression",		/* 18 */
	"object code before ORG",	/* 19 */
	"illegal label",		/* 20 */
	"missing .DEPHASE",		/* 21 */
	"not in macro definition",	/* 22 */
	"missing ENDM",			/* 23 */
	"not in macro expansion",	/* 24 */
	"macro expansion nested too deep", /* 25 */
	"too many local labels"		/* 26 */
};

/*
 *	Intel hex record types
 */
#define	HEX_DATA	0
#define HEX_EOF		1

static int nseq_flag;			/* flag for non-sequential ORG */
static int curr_addr;			/* current logical file address */
static int bin_addr;			/* current address written to file */
static unsigned short hex_addr;		/* current address in hex record */
static int hex_cnt;			/* number of bytes in hex buffer */

static unsigned char hex_buf[MAXHEX];	/* buffer for one hex record */
static char hex_out[MAXHEX*2+20];	/* ASCII buffer for one hex record */

/*
 *	print error message to listfile and increase error counter
 */
void asmerr(int i)
{
	if (pass == 1) {
		fprintf(errfp, "Error in file: %s  Line: %d\n", srcfn, c_line);
		fputs(errmsg[i], errfp);
		fputc('\n', errfp);
	} else
		errnum = i;
	errors++;
}

/*
 *	begin new page in listfile
 */
void lst_header(void)
{
	static int header_done;
	time_t tloc = time(&tloc);

	if (header_done && ppl != 0)
		fputc('\f', lstfp);
	if (!header_done || ppl != 0)
		fprintf(lstfp, "Z80-Macro-Assembler  Release %s\t%.24s",
			REL, ctime(&tloc));
	if (ppl != 0) {
		fprintf(lstfp, "\tPage %d\n", ++page);
		fprintf(lstfp, "Source file: %s\n", srcfn);
		fprintf(lstfp, "Title:       %s", title);
		p_line = 3;
	} else
		p_line = 0;
	fputc('\n', lstfp);
	header_done = 1;
}

/*
 *	print header for source lines
 */
void lst_attl(void)
{
	static int attl_done;

	if (!attl_done || ppl != 0)
		fprintf(lstfp, "\nLOC   OBJECT CODE   LINE   STMT SOURCE CODE\n");
	if (ppl != 0)
		p_line += 2;
	attl_done = 1;
}

/*
 *	print one line into listfile, if -l option set
 */
void lst_line(char *l, int addr, int op_cnt, int expn_flag)
{
	register int i, j;
	register const char *a_mark;
	static int s_line;

	s_line++;
	if (!list_flag)
		return;
	if (ppl != 0)
		p_line++;
	if (p_line > ppl || c_line == 1) {
		lst_header();
		lst_attl();
		if (ppl != 0)
			p_line++;
	}
	a_mark = "   ";
	switch (a_mode) {
	case A_STD:
		fprintf(lstfp, "%04x  ", addr & 0xffff);
		break;
	case A_EQU:
		fprintf(lstfp, "%04x  ", a_addr & 0xffff);
		a_mark = "=  ";
		break;
	case A_SET:
		fprintf(lstfp, "%04x  ", a_addr & 0xffff);
		a_mark = "#  ";
		break;
	case A_DS:
		fprintf(lstfp, "%04x  ", a_addr & 0xffff);
		op_cnt = 0;
		break;
	case A_NONE:
		fputs("      ", lstfp);
		break;
	default:
		fatal(F_INTERN, "invalid a_mode for function lst_line");
		break;
	}
	i = 0;
	for (j = 0; j < 4; j++)
		if (op_cnt-- > 0)
			fprintf(lstfp, "%02x ", ops[i++]);
		else if (j == 0)
			fputs(a_mark, lstfp);
		else
			fputs("   ", lstfp);
	fprintf(lstfp, "%c%5d %6d %s", expn_flag ? '+' : ' ',
		c_line, s_line, l);
	if (errnum) {
		fprintf(errfp, "=> %s\n", errmsg[errnum]);
		errnum = 0;
		if (ppl != 0)
			p_line++;
	}
	while (op_cnt > 0) {
		if (ppl != 0)
			p_line++;
		if (p_line > ppl) {
			lst_header();
			lst_attl();
			if (ppl != 0)
				p_line++;
		}
		s_line++;
		addr += 4;
		fprintf(lstfp, "%04x  ", addr & 0xffff);
		for (j = 0; j < 4; j++)
			if (op_cnt-- > 0)
				fprintf(lstfp, "%02x ", ops[i++]);
			else
				fputs("   ", lstfp);
		fprintf(lstfp, "%c%5d %6d\n", expn_flag ? '+' : ' ',
			c_line, s_line);
	}
	if (p_line < 0)
		p_line = 1;
}

/*
 *	print macro table into listfile
 */
void lst_mac(int sorted)
{
	register int i, j;
	register char *p;
	int refcnt;

	p = mac_lst_first(sorted, &refcnt);
	if (p == NULL)
		return;
	p_line = i = 0;
	strcpy(title,"Macro table");
	while (p != NULL) {
		if (p_line == 0) {
			lst_header();
			if (ppl == 0) {
				fputs(title, lstfp);
				fputc('\n', lstfp);
			}
			fputc('\n', lstfp);
			p_line++;
		}
		j = strlen(p);
		fprintf(lstfp, "%s%c", p, refcnt > 0 ? ' ' : '*');
		while (j++ < mac_symmax)
			fputc(' ', lstfp);
		i += mac_symmax + 4;
		if (i + mac_symmax + 1 >= 80) {
			fputc('\n', lstfp);
			if (ppl != 0 && ++p_line >= ppl)
				p_line = 0;
			i = 0;
		} else
			fputs("   ", lstfp);
		p = mac_lst_next(sorted, &refcnt);
	}
	if (i > 0)
		fputc('\n', lstfp);
}

/*
 *	print symbol table into listfile unsorted
 */
void lst_sym(void)
{
	register int i, j;
	register struct sym *np;

	p_line = j = 0;
	strcpy(title,"Symbol table");
	for (i = 0; i < HASHSIZE; i++) {
		if (symtab[i] != NULL) {
			for (np = symtab[i]; np != NULL; np = np->sym_next) {
				if (p_line == 0) {
					lst_header();
					if (ppl == 0) {
						fputs(title, lstfp);
						fputc('\n', lstfp);
					}
					fputc('\n', lstfp);
					p_line++;
				}
				fprintf(lstfp, "%*s %04x%c", -symmax,
					np->sym_name, np->sym_val & 0xffff,
					np->sym_refcnt > 0 ? ' ' : '*');
				j += symmax + 9;
				if (j + symmax + 6 >= 80) {
					fputc('\n', lstfp);
					if (ppl != 0 && ++p_line >= ppl)
						p_line = 0;
					j = 0;
				} else
					fputs("   ", lstfp);
			}
		}
	}
	if (j > 0)
		fputc('\n', lstfp);
}

/*
 *	print sorted symbol table into listfile
 */
void lst_sort_sym(int len)
{
	register int i, j;

	p_line = i = j = 0;
	strcpy(title, "Symbol table");
	while (i < len) {
		if (p_line == 0) {
			lst_header();
			if (ppl == 0) {
				fputs(title, lstfp);
				fputc('\n', lstfp);
			}
			fputc('\n', lstfp);
			p_line++;
		}
		fprintf(lstfp, "%*s %04x%c", -symmax, symarray[i]->sym_name,
			symarray[i]->sym_val & 0xffff,
			symarray[i]->sym_refcnt > 0 ? ' ' : '*');
		j += symmax + 9;
		if (j + symmax + 6 >= 80) {
			fputc('\n', lstfp);
			if (ppl != 0 && ++p_line >= ppl)
				p_line = 0;
			j = 0;
		} else
			fputs("   ", lstfp);
		i++;
	}
	if (j > 0)
		fputc('\n', lstfp);
}

/*
 *	write header record into object file
 */
void obj_header(void)
{
	switch (out_form) {
	case OUTBIN:
		break;
	case OUTMOS:
		putc(0xff, objfp);
		putc(load_addr & 0xff, objfp);
		putc(load_addr >> 8, objfp);
		break;
	case OUTHEX:
		break;
	}
}

/*
 *	write end record into object file
 */
void obj_end(void)
{
	switch (out_form) {
	case OUTBIN:
	case OUTMOS:
		if (!nofill_flag && !(load_flag && (bin_addr < load_addr)))
			fill_bin();
		break;
	case OUTHEX:
		flush_hex();
		hex_addr = start_addr;
		eof_hex();
		break;
	}
}

/*
 *	set logical address for object file
 */
void obj_org(int addr)
{
	switch (out_form) {
	case OUTBIN:
	case OUTMOS:
		nseq_flag = (addr < curr_addr);
		if (load_flag && (bin_addr < load_addr))
			bin_addr = addr;
		curr_addr = addr;
		break;
	case OUTHEX:
		curr_addr = addr;
		break;
	}
}

/*
 *	write opcodes in ops[] into object file
 */
void obj_writeb(int op_cnt)
{
	register int i;

	if (op_cnt == 0)
		return;
	switch (out_form) {
	case OUTBIN:
	case OUTMOS:
		if (nseq_flag)
			asmerr(E_NSQWRT);
		else {
			if (load_flag && (bin_addr < load_addr))
				asmerr(E_BFRORG);
			else {
				fill_bin();
				fwrite(ops, 1, op_cnt, objfp);
				bin_addr += op_cnt;
			}
			curr_addr += op_cnt;
		}
		break;
	case OUTHEX:
		if (hex_addr + hex_cnt != curr_addr)
			flush_hex();
		for (i = 0; op_cnt; op_cnt--) {
			if (hex_cnt >= hexlen)
				flush_hex();
			hex_buf[hex_cnt++] = ops[i++];
			curr_addr++;
		}
		break;
	}
}

/*
 *	advance logical address of object file by count
 */
void obj_fill(int count)
{
	if (count == 0)
		return;
	switch (out_form) {
	case OUTBIN:
	case OUTMOS:
		if (!nseq_flag)
			curr_addr += count;
		break;
	case OUTHEX:
		curr_addr += count;
		break;
	}
}

/*
 *	write <count> bytes <value> into object file
 */
void obj_fill_value(int count, int value)
{
	register int i;

	if (count == 0)
		return;
	i = count;
	switch (out_form) {
	case OUTBIN:
	case OUTMOS:
		if (nseq_flag)
			asmerr(E_NSQWRT);
		else {
			if (load_flag && (bin_addr < load_addr))
				asmerr(E_BFRORG);
			else {
				fill_bin();
				while (i--)
					putc(value, objfp);
				bin_addr += count;
			}
			curr_addr += count;
		}
		break;
	case OUTHEX:
		if (hex_addr + hex_cnt != curr_addr)
			flush_hex();
		while (i--) {
			if (hex_cnt >= hexlen)
				flush_hex();
			hex_buf[hex_cnt++] = value;
			curr_addr++;
		}
		break;
	}
}

/*
 *	fill binary object file up to the logical address with 0xff bytes
 */
void fill_bin(void)
{
	while (bin_addr < curr_addr) {
		putc(0xff, objfp);
		bin_addr++;
	}
}

/*
 *	create a hex end-of-file record in ASCII and write into object file
 */
void eof_hex(void)
{
	hex_cnt = 0;
	hex_record(HEX_EOF);
}

/*
 *	create a hex data record in ASCII and write into object file
 */
void flush_hex(void)
{
	if (hex_cnt != 0) {
		hex_record(HEX_DATA);
		hex_cnt = 0;
	}
	hex_addr = curr_addr;
}

/*
 *	write a hex record in ASCII and write into object file
 */
void hex_record(int rec_type)
{
	char *p;
	register int i;

	p = hex_out;
	*p++ = ':';
	btoh((unsigned char) hex_cnt, &p);
	btoh((unsigned char) (hex_addr >> 8), &p);
	btoh((unsigned char) (hex_addr & 0xff), &p);
	btoh((unsigned char) rec_type, &p);
	for (i = 0; i < hex_cnt; i++)
		btoh(hex_buf[i], &p);
	btoh((unsigned char) chksum(rec_type), &p);
	*p++ = '\n';
	*p = '\0';
	fwrite(hex_out, 1, strlen(hex_out), objfp);
}

/*
 *	convert unsigned char into ASCII hex and copy to string at p
 *	increase p by 2
 */
void btoh(unsigned char byte, char **p)
{
	register unsigned char c;

	c = byte >> 4;
	*(*p)++ = (c < 10) ? (c + '0') : (c - 10 + 'A');
	c = byte & 0xf;
	*(*p)++ = (c < 10) ? (c + '0') : (c - 10 + 'A');
}

/*
 *	compute checksum for Intel hex record
 */
int chksum(int rec_type)
{
	register int i, j, sum;

	sum = hex_cnt;
	sum += hex_addr >> 8;
	sum += hex_addr & 0xff;
	sum += rec_type;
	for (i = 0; i < hex_cnt; i++) {
		j = hex_buf[i];
		sum += j & 0xff;
	}
	return (0x100 - (sum & 0xff));
}
