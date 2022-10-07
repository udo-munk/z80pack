/*
 *	Z80 - Assembler
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
 *	28-OCT-2017 added variable symbol lenght and other improvements
 *	15-MAY-2018 mark unreferenced symbols in listing
 *	30-JUL-2021 fix verbose option
 *	28-JAN-2022 added syntax check for OUT (n),A
 *	24-SEP-2022 added undocumented Z80 instructions and 8080 mode (TE)
 *	04-OCT-2022 new expression parser (TE)
 */

/*
 *	main module, handles the options and runs 2 passes over the sources
 */

#include <stdlib.h>
#include <stdio.h>
#ifdef _POSIX_C_SOURCE
#include <unistd.h>
#endif
#include <string.h>
#include <ctype.h>
#include "z80a.h"
#include "z80aglb.h"

void init(void), options(int, char *[]);
void usage(void), fatal(int, char *);
void pass1(void), p1_file(char *);
void pass2(void), p2_file(char *);
int p1_line(void), p2_line(void);
void open_o_files(char *), get_fn(char *, char *, char *);
char *get_label(char *, char *);
char *get_opcode(char *, char *);
char *get_arg(char *, char *);

/* z80aout.c */
extern void asmerr(int);
extern void lst_line(int, int);
extern void lst_sym(void);
extern void lst_sort_sym(int);
extern void obj_header(void);
extern void obj_end(void);
extern void obj_writeb(int);

/* z80anum.c */
int is_sym_char(char);

/* z80atab.c */
extern struct opc *search_op(char *);
extern int put_sym(char *, int);
extern void put_label(void);
extern int copy_sym(void);
extern void n_sort_sym(int);
extern void a_sort_sym(int);

static char *errmsg[] = {		/* error messages for fatal() */
	"out of memory: %s",		/* 0 */
	"usage: z80asm -f[b|m|h] -s[n|a] -e<num> -h<num> -x -8 -u -v\n\
              -ofile -l[file] -dsymbol ... file ...", /* 1 */
	"Assembly halted",		/* 2 */
	"can't open file %s",		/* 3 */
	"internal error: %s",		/* 4 */
	"invalid hex record length: %s"	/* 5 */
};

int main(int argc, char *argv[])
{
	int len;

	init();
	options(argc, argv);
	printf("Z80 - Assembler Release %s, %s\n", REL, COPYR);
	pass1();
	pass2();
	if (list_flag) {
		switch (sym_flag) {
		case 0:		/* no symbol table */
			break;
		case 1:		/* unsorted symbol table */
			lst_sym();
			break;
		case 2:		/* symbol table sorted by name */
			len = copy_sym();
			n_sort_sym(len);
			lst_sort_sym(len);
			break;
		case 3:		/* symbol table sorted by address */
			len = copy_sym();
			a_sort_sym(len);
			lst_sort_sym(len);
			break;
		default:
			break;
		}
		fclose(lstfp);
	}
	return(errors);
}

/*
 *	initialization
 */
void init(void)
{
	errfp = stdout;
}

/*
 *	process options
 */
void options(int argc, char *argv[])
{
	register char *s, *t;
	register int i;

	while (--argc > 0 && (*++argv)[0] == '-')
		for (s = argv[0] + 1; *s != '\0'; s++)
			switch (*s) {
			case 'o':
				if (*++s == '\0') {
					puts("name missing in option -o");
					usage();
				}
				if (out_form == OUTHEX)
					get_fn(objfn, s, OBJEXTHEX);
				else
					get_fn(objfn, s, OBJEXTBIN);
				s += (strlen(s) - 1);
				break;
			case 'l':
				if (*(s + 1) != '\0') {
					get_fn(lstfn, ++s, LSTEXT);
					s += (strlen(s) - 1);
				}
				list_flag = 1;
				break;
			case 's':
				if (*(s + 1) == '\0')
					sym_flag = 1;
				else if (*(s + 1) == 'n')
					sym_flag = 2;
				else if (*(s + 1) == 'a')
					sym_flag = 3;
				else {
					printf("unknown option -%s\n", s);
					usage();
				}
				s += (strlen(s) - 1);
				break;
			case 'x':
				nofill_flag = 1;
				break;
			case 'f':
				if (*(s + 1) == 'b')
					out_form = OUTBIN;
				else if (*(s + 1) == 'm')
					out_form = OUTMOS;
				else if (*(s + 1) == 'h')
					out_form = OUTHEX;
				else {
					printf("unknown option -%s\n", s);
					usage();
				}
				s += (strlen(s) - 1);
				break;
			case 'd':
				if (*++s == '\0') {
					puts("name missing in option -d");
					usage();
				}
				t = tmp;
				while (*s)
					*t++ = toupper((unsigned char) *s++);
				s--;
				*t = '\0';
				if (put_sym(tmp, 0))
					fatal(F_OUTMEM, "symbols");
				break;
			case '8':
				opset = OPSET_8080;
				break;
			case 'u':
				undoc_flag = 1;
				break;
			case 'v':
				ver_flag = 1;
				break;
			case 'e':
				if (*++s == '\0') {
					puts("symbol length missing in option -e");
					usage();
				}
				symlen = atoi(s);
				s += (strlen(s) - 1);
				break;
			case 'h':
				if (*++s == '\0') {
					puts("hex record length missing in option -h");
					usage();
				}
				hexlen = atoi(s);
				if (hexlen < 1 || hexlen > MAXHEX)
					fatal(F_HEXLEN, s);
				s += (strlen(s) - 1);
				break;
			default :
				printf("unknown option %c\n", *s);
				usage();
			}
	if (nofill_flag && !((out_form == OUTBIN) || (out_form == OUTMOS))) {
		puts("option -x only allowed with binary output");
		usage();
	}
	i = 0;
	while ((argc--) && (i < MAXFN)) {
		if ((infiles[i] = malloc(LENFN + 1)) == NULL)
			fatal(F_OUTMEM, "filenames");
		get_fn(infiles[i], *argv++, SRCEXT);
		i++;
	}
	if (i == 0) {
		printf("no input file\n");
		usage();
	}
}

/*
 *	error in options, print usage
 */
void usage(void)
{
	fatal(F_USAGE, NULL);
}

/*
 *	print error messages and abort
 */
void fatal(int i, char *arg)
{
	printf(errmsg[i], arg);
	putchar('\n');
	exit(1);
}

/*
 *	Pass 1:
 *	  - process all source files
 */
void pass1(void)
{
	register int fi;

	pass = 1;
	radix = 10;
	rpc = pc = 0;
	phs_flag = 0;
	fi = 0;
	if (ver_flag)
		puts("Pass 1");
	open_o_files(infiles[fi]);
	while (infiles[fi] != NULL) {
		if (ver_flag)
			printf("   Read    %s\n", infiles[fi]);
		p1_file(infiles[fi]);
		fi++;
	}
	if (errors) {
		fclose(objfp);
		unlink(objfn);
		printf("%d error(s)\n", errors);
		fatal(F_HALT, NULL);
	}
}

/*
 *	Pass 1:
 *	  - process one source file
 *
 *	Input: name of source file
 */
void p1_file(char *fn)
{
	c_line = 0;
	srcfn = fn;
	if ((srcfp = fopen(fn, READA)) == NULL)
		fatal(F_FOPEN, fn);
	while (p1_line())
		;
	fclose(srcfp);
	if (iflevel)
		asmerr(E_MISEIF);
}

/*
 *	Pass 1:
 *	  - process one line of source
 *
 *	Output: 1 line processed
 *		0 EOF
 */
int p1_line(void)
{
	register char *p;
	register int i;
	register struct opc *op;

	if ((p = fgets(line, MAXLINE, srcfp)) == NULL)
		return(0);
	c_line++;
	p = get_label(label, p);
	p = get_opcode(opcode, p);
	p = get_arg(operand, p);
	if (*opcode) {
		if ((op = search_op(opcode)) != NULL) {
			if (gencode && *label && (op->op_type != OP_SET))
				put_label();
			if (gencode || (op->op_type == OP_COND)) {
				i = (*op->op_fun)(op->op_c1, op->op_c2);
				pc += i;
				rpc += i;
				if (op->op_type == OP_END)
					return(0);
			}
		} else
			asmerr(E_ILLOPC);
	} else if (gencode && *label)
		put_label();
	return(1);
}

/*
 *	Pass 2:
 *	  - process all source files
 */
void pass2(void)
{
	register int fi;

	pass = 2;
	radix = 10;
	rpc = pc = 0;
	phs_flag = 0;
	fi = 0;
	if (ver_flag)
		puts("Pass 2");
	obj_header();
	while (infiles[fi] != NULL) {
		if (ver_flag)
			printf("   Read    %s\n", infiles[fi]);
		p2_file(infiles[fi]);
		fi++;
	}
	obj_end();
	fclose(objfp);
	printf("%d error(s)\n", errors);
}

/*
 *	Pass 2:
 *	  - process one source file
 *
 *	Input: name of source file
 */
void p2_file(char *fn)
{
	c_line = 0;
	srcfn = fn;
	if ((srcfp = fopen(fn, READA)) == NULL)
		fatal(F_FOPEN, fn);
	while (p2_line())
		;
	fclose(srcfp);
}

/*
 *	Pass 2:
 *	  - process one line of source
 *
 *	Output: 1 line processed
 *		0 EOF
 */
int p2_line(void)
{
	register char *p;
	register int op_count;
	register struct opc *op;

	if ((p = fgets(line, MAXLINE, srcfp)) == NULL)
		return(0);
	c_line++;
	s_line++;
	p = get_label(label, p);
	p = get_opcode(opcode, p);
	p = get_arg(operand, p);
	if (*opcode) {
		op = search_op(opcode);
		if (gencode || (op->op_type == OP_COND)) {
			op_count = (*op->op_fun)(op->op_c1, op->op_c2);
			lst_line(pc, op_count);
			obj_writeb(op_count);
			pc += op_count;
			rpc += op_count;
			if (op->op_type == OP_END)
				return(0);
		} else {
			sd_flag = 2;
			lst_line(0, 0);
		}
	} else {
		sd_flag = 2;
		lst_line(0, 0);
	}
	return(1);
}

/*
 *	open output files:
 *	input is filename of source file
 *	list and object filenames are build from source filename if
 *	not given by options
 */
void open_o_files(char *source)
{
	register char *p;

	if (*objfn == '\0')
		strcpy(objfn, source);
	if ((p = strrchr(objfn, '.')) != NULL) {
		if (out_form == OUTHEX)
			strcpy(p, OBJEXTHEX);
		else
			strcpy(p, OBJEXTBIN);
	} else {
		if (out_form == OUTHEX)
			strcat(objfn, OBJEXTHEX);
		else
			strcat(objfn, OBJEXTBIN);
	}

	if (out_form == OUTHEX)
		objfp = fopen(objfn, WRITEA);
	else
		objfp = fopen(objfn, WRITEB);
	if (objfp == NULL)
		fatal(F_FOPEN, objfn);
	if (list_flag) {
		if (*lstfn == '\0')
			strcpy(lstfn, source);
		if ((p = strrchr(lstfn, '.')) != NULL)
			strcpy(p, LSTEXT);
		else
			strcat(lstfn, LSTEXT);
		if ((lstfp = fopen(lstfn, WRITEA)) == NULL)
			fatal(F_FOPEN, lstfn);
		errfp = lstfp;
	}
}

/*
 *	create a filename in "dest" from "src" and "ext"
 */
void get_fn(char *dest, char *src, char *ext)
{
	register int i;
	register char *sp, *dp;

	i = 0;
	sp = src;
	dp = dest;
	while ((i++ < LENFN) && (*sp != '\0'))
		*dp++ = *sp++;
	*dp = '\0';
	if ((strrchr(dest,'.') == NULL) &&
	    (strlen(dest) <= (LENFN - strlen(ext))))
		strcat(dest, ext);
}

/*
 *	get labels, constants and variables from source line
 *	convert names to upper case and truncate length of name
 */
char *get_label(char *s, char *l)
{
	register int i;

	i = 0;
	if (*l == LINCOM)
		goto comment;
	while (!isspace((unsigned char) *l) && *l != COMMENT && *l != LABSEP
	       && i < symlen) {
		*s++ = toupper((unsigned char) *l++);
		i++;
	}
comment:
	*s = '\0';
	return(l);
}

/*
 *	get opcode into s from source line l
 *	converts to upper case
 */
char *get_opcode(char *s, char *l)
{
	if (*l == LINCOM)
		goto comment;
	while (!isspace((unsigned char) *l) && *l != COMMENT && *l != LABSEP)
		l++;
	if (*l == LABSEP)
		l++;
	while (*l == ' ' || *l == '\t')
		l++;
	while (!isspace((unsigned char) *l) && *l != COMMENT)
		*s++ = toupper((unsigned char) *l++);
comment:
	*s = '\0';
	return(l);
}

/*
 *	get operand into s from source line l
 *	converts to upper case
 *	delimited strings are copied without changes
 */
char *get_arg(char *s, char *l)
{
	register char *s0;
	register char c;

	s0 = s;
	if (*l == LINCOM)
		goto comment;
	while (*l == ' ' || *l == '\t')
		l++;
	while (*l != '\n' && *l != COMMENT) {
		if (isspace((unsigned char) *l)) {
			l++;
			while (*l == ' ' || *l == '\t')
				l++;
			if ((s > s0) && is_sym_char(*(s - 1))
				     && is_sym_char(*l))
				/* add space between symbols/numbers */
				*s++ = ' ';
			continue;
		}
		if ((*l != STRDEL) && (*l != STRDEL2)) {
			*s++ = toupper((unsigned char) *l++);
			continue;
		}
		c = *l;
		*s++ = *l++;
		if (s - s0 == 6 && strncmp(s0, "AF,AF'", 6) == 0)
			continue;
		while (1) {
			if (*l == c) {
				/* check for double delim */
				if (*(l + 1) != c)
					break;
				else
					*s++ = *l++;
			}
			if (*l == '\n' || *l == '\0')
				goto comment;
			*s++ = *l++;
		}
		*s++ = *l++;
	}
comment:
	*s = '\0';
	return(l);
}
