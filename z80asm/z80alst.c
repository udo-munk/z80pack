/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2024 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	module for output functions to list files
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "z80asm.h"
#include "z80amfun.h"
#include "z80atab.h"
#include "z80alst.h"

static void lst_byte(BYTE b);

static FILE *lstfp;			/* file pointer for listing */
static const char *srcfn;		/* current source file name */
static char title[MAXLINE + 1];		/* buffer for title of source */
static int  ppl;			/* page length */
static int  p_line;			/* no. printed lines on page */
static int  page;			/* no. of pages for listing */
static int  nodate_flag;		/* don't print date in header */
static int  sort_mode;			/* symbol table print/sort mode */

/*
 *	set list output options
 */
void lst_set_options(int pagelen, int nodate, int sym_sort)
{
	ppl = pagelen;
	nodate_flag = nodate;
	sort_mode = sym_sort;
}

/*
 *	set/open list output file
 */
FILE *lst_open_file(const char *fn)
{
	lstfp = fopen(fn, WRITEA);
	if (lstfp == NULL)
		fatal(F_FOPEN, fn);
	return lstfp;
}

/*
 *	close list output file
 */
void lst_close_file(void)
{
	if (lstfp != NULL)
		fclose(lstfp);
}

/*
 *	set page length
 */
void lst_set_ppl(int n)
{
	ppl = n;
}

/*
 *	set page title
 */
void lst_set_title(const char *s)
{
	strcpy(title, s);
}

/*
 *	set current source file name
 */
void lst_set_srcfn(const char *fn)
{
	srcfn = fn;
}

/*
 *	eject page before or after next lst_line
 */
void lst_eject(int before)
{
	if (before)
		p_line = ppl + 1;
	else
		p_line = ppl - 1;
}

/*
 *	begin new page in listfile
 */
static void lst_header(void)
{
	static int header_done;
	time_t tloc;

	if (ppl != 0 && header_done)
		fputc('\f', lstfp);
	if (ppl != 0 || !header_done) {
		fprintf(lstfp, "Z80/8080-Macro-Assembler  Release %s",
			RELEASE);
		if (!nodate_flag) {
			tloc = time(&tloc);
			fprintf(lstfp, "\t%.24s", ctime(&tloc));
		}
		header_done = TRUE;
	}
	if (ppl != 0) {
		if (nodate_flag)
			fputs("\t\t\t\t", lstfp);
		fprintf(lstfp, "\tPage %d\n", ++page);
		fprintf(lstfp, "Source file: %s\n", srcfn);
		if (title[0])
			fprintf(lstfp, "Title:       %s", title);
		p_line = 3;
	} else
		p_line = 0;
	fputc('\n', lstfp);
}

/*
 *	print header for source lines
 */
static void lst_attl(void)
{
	static int attl_done;

	if (ppl != 0 || !attl_done) {
		fprintf(lstfp,
			"\nLOC   OBJECT CODE   LINE   STMT SOURCE CODE\n");
		attl_done = TRUE;
	}
	if (ppl != 0)
		p_line += 2;
}

/*
 *	print one line into listfile, if list_active is TRUE
 */
void lst_line(const char *line, int list_active, int expn_flag,
	      int a_mode, WORD addr, const BYTE *ops, WORD op_cnt,
	      unsigned long c_line, const char *errmsg)
{
	register int i, j;
	register const char *a_mark;
	static unsigned long s_line;

	s_line++;
	if (!list_active)
		return;
	if (ppl != 0)
		p_line++;
	if (p_line > ppl || c_line == 1) {
		lst_header();
		lst_attl();
		if (ppl != 0)
			p_line++;
	}
	a_mark = "  ";
	switch (a_mode) {
	case A_STD:
		break;
	case A_EQU:
		a_mark = "= ";
		break;
	case A_SET:
		a_mark = "# ";
		break;
	case A_DS:
		op_cnt = 0;
		break;
	case A_NONE:
		break;
	default:
		fatal(F_INTERN, "invalid a_mode for function lst_line");
		break;
	}
	if (a_mode == A_NONE)
		fputs("    ", lstfp);
	else {
		lst_byte(addr >> 8);
		lst_byte(addr & 0xff);
	}
	fputs("  ", lstfp);
	i = 0;
	for (j = 0; j < 4; j++) {
		if (op_cnt > 0) {
			lst_byte(ops[i++]);
			op_cnt--;
		} else if (j == 0)
			fputs(a_mark, lstfp);
		else
			fputs("  ", lstfp);
		fputc(' ', lstfp);
	}
	fprintf(lstfp, "%c%5lu %6lu", expn_flag ? '+' : ' ',
		c_line, s_line);
	if (*line) {
		fputc(' ', lstfp);
		fputs(line, lstfp);
	}
	fputc('\n', lstfp);
	if (errmsg != NULL) {
		fprintf(lstfp, "=> %s\n", errmsg);
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
		lst_byte(addr >> 8);
		lst_byte(addr & 0xff);
		fputs("  ", lstfp);
		for (j = 0; j < 4; j++) {
			if (op_cnt > 0) {
				lst_byte(ops[i++]);
				op_cnt--;
			} else
				fputs("  ", lstfp);
			fputc(' ', lstfp);
		}
		fprintf(lstfp, "%c%5lu %6lu\n", expn_flag ? '+' : ' ',
			c_line, s_line);
	}
	/* reset p_line after EJECT when ppl is 0 */
	if (p_line < 0)
		p_line = 1;
}

/*
 *	print macro table into listfile, sorted as specified with sort_mode
 */
void lst_mac(void)
{
	register int col, prev_len;
	register char *p;
	int prev_rf, rf;

	if (sort_mode == SYM_NONE)
		return;
	p_line = col = prev_len = 0;
	prev_rf = TRUE;
	strcpy(title, "Macro table");
	for (p = mac_first(sort_mode, &rf); p != NULL; p = mac_next(&rf)) {
		if (col + mac_get_symmax() + 1 >= 80) {
			if (!prev_rf)
				fputc('*', lstfp);
			fputc('\n', lstfp);
			if (ppl != 0 && ++p_line >= ppl)
				p_line = 0;
			col = 0;
		} else if (col > 0) {
			fputc(prev_rf? ' ' : '*', lstfp);
			while (prev_len++ < mac_get_symmax())
				fputc(' ', lstfp);
			fputs("   ", lstfp);
		}
		if (p_line == 0) {
			lst_header();
			if (ppl == 0 && title[0]) {
				fputs(title, lstfp);
				fputc('\n', lstfp);
			}
			fputc('\n', lstfp);
			p_line++;
		}
		prev_len = strlen(p);
		fprintf(lstfp, "%s", p);
		prev_rf = rf;
		col += mac_get_symmax() + 4;
	}
	if (col > 0) {
		if (!prev_rf)
			fputc('*', lstfp);
		fputc('\n', lstfp);
	}
}

/*
 *	print symbol table into listfile, sorted as specified with sort_mode
 */
void lst_sym(void)
{
	register int col;
	register struct sym *sp;
	int prev_rf;

	if (sort_mode == SYM_NONE)
		return;
	p_line = col = 0;
	prev_rf = TRUE;
	strcpy(title, "Symbol table");
	for (sp = first_sym(sort_mode); sp != NULL; sp = next_sym()) {
		if (col + get_symmax() + 6 >= 80) {
			if (!prev_rf)
				fputc('*', lstfp);
			fputc('\n', lstfp);
			if (ppl != 0 && ++p_line >= ppl)
				p_line = 0;
			col = 0;
		} else if (col > 0) {
			fputc(prev_rf ? ' ' : '*', lstfp);
			fputs("   ", lstfp);
		}
		if (p_line == 0) {
			lst_header();
			if (ppl == 0 && title[0]) {
				fputs(title, lstfp);
				fputc('\n', lstfp);
			}
			fputc('\n', lstfp);
			p_line++;
		}
		fprintf(lstfp, "%*s ", -get_symmax(), sp->sym_name);
		lst_byte(sp->sym_val >> 8);
		lst_byte(sp->sym_val & 0xff);
		prev_rf = sp->sym_refflg;
		col += get_symmax() + 9;
	}
	if (col > 0) {
		if (!prev_rf)
			fputc('*', lstfp);
		fputc('\n', lstfp);
	}
}

/*
 *	print BYTE as ASCII hex into listfile
 */
static void lst_byte(BYTE b)
{
	register char c;

	c = b >> 4;
	fputc(c + (c < 10 ? '0' : 'W'), lstfp);
	c = b & 0xf;
	fputc(c + (c < 10 ? '0' : 'W'), lstfp);
}
