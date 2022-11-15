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
 *	main module, handles the options and runs 2 passes over the sources
 */

#include <stdlib.h>
#include <stdio.h>
#ifdef _POSIX_C_SOURCE
#include <unistd.h>
#endif
#include <string.h>
#include "z80a.h"
#include "z80aglb.h"

void init(void), options(int, char *[]);
void usage(void), fatal(int, const char *);
void do_pass(int), process_file(char *);
int process_line(char *);
void open_o_files(char *), get_fn(char *, char *, const char *);
char *get_symbol(char *, char *, int);
char *get_operand(char *, char *, int);

/* z80aout.c */
extern void asmerr(int);
extern void lst_line(char *, WORD, WORD, int);
extern void lst_mac(int);
extern void lst_sym(int);
extern void obj_header(void);
extern void obj_end(void);
extern void obj_writeb(WORD);

/* z80mfun.c */
extern void mac_start_pass(void);
extern void mac_end_pass(void);
extern char *mac_expand(void);
extern void mac_add_line(struct opc *, char *);
extern int mac_lookup(char *);
extern void mac_call(void);

/* z80anum.c */
extern void init_ctype(void);

/* z80aopc.c */
extern void instrset(int);
extern struct opc *search_op(char *);

/* z80atab.c */
extern void put_sym(char *, WORD);
extern void put_label(void);

static const char *errmsg[] = {		/* error messages for fatal() */
	"out of memory: %s",		/* 0 */
	("usage: z80asm -f{b|m|h} -s[n|a] -p<num> -e<num> -h<num> -x -8 -u\n"
	 "              -v -m -U -o<file> -l[<file>] "
	 "-d<symbol> ... <file> ..."),	/* 1 */
	"Assembly halted",		/* 2 */
	"can't open file %s",		/* 3 */
	"internal error: %s",		/* 4 */
	"invalid page length: %s",	/* 5 */
	"invalid symbol length: %s",	/* 6 */
	"invalid hex record length: %s"	/* 7 */
};

int main(int argc, char *argv[])
{
	init();
	options(argc, argv);
	instrset(i8080_flag ? INSTR_8080 : INSTR_Z80);
	printf("Z80 - Macro - Assembler Release %s\n%s\n", REL, COPYR);
	do_pass(1);
	do_pass(2);
	if (list_flag) {
		if (sym_flag != SYM_NONE) {
			lst_mac(sym_flag);
			lst_sym(sym_flag);
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
	init_ctype();
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
				if (obj_fmt == OBJ_HEX)
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
					sym_flag = SYM_UNSORT;
				else if (*(s + 1) == 'n')
					sym_flag = SYM_SORTN;
				else if (*(s + 1) == 'a')
					sym_flag = SYM_SORTA;
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
					obj_fmt = OBJ_BIN;
				else if (*(s + 1) == 'm')
					obj_fmt = OBJ_MOS;
				else if (*(s + 1) == 'h')
					obj_fmt = OBJ_HEX;
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
				t = label;
				while (*s) {
					*t++ = TO_UPP(*s);
					s++;
				}
				s--;
				*t = '\0';
				put_sym(label, 0);
				break;
			case '8':
				i8080_flag = 1;
				break;
			case 'u':
				undoc_flag = 1;
				break;
			case 'v':
				ver_flag = 1;
				break;
			case 'm':
				if (mac_list_flag == M_OPS)
					mac_list_flag = M_ALL;
				else
					mac_list_flag = M_NONE;
				break;
			case 'U':
				upcase_flag = 1;
				break;
			case 'p':
				if (*++s == '\0') {
					puts("length missing in option -p");
					usage();
				}
				ppl = atoi(s);
				if (ppl != 0 && (ppl < 6 || ppl > 144))
					fatal(F_PAGLEN, s);
				s += (strlen(s) - 1);
				break;
			case 'e':
				if (*++s == '\0') {
					puts("length missing in option -e");
					usage();
				}
				symlen = atoi(s);
				if (symlen < 6 || symlen > 32)
					fatal(F_SYMLEN, s);
				s += (strlen(s) - 1);
				break;
			case 'h':
				if (*++s == '\0') {
					puts("length missing in option -h");
					usage();
				}
				hexlen = atoi(s);
				if (hexlen < 1 || hexlen > MAXHEX)
					fatal(F_HEXLEN, s);
				s += (strlen(s) - 1);
				break;
			default:
				printf("unknown option %c\n", *s);
				usage();
			}
	i = 0;
	while (argc-- && i < MAXFN) {
		if ((infiles[i] = (char *) malloc(LENFN + 1)) == NULL)
			fatal(F_OUTMEM, "filenames");
		get_fn(infiles[i], *argv++, SRCEXT);
		i++;
	}
	if (i == 0) {
		puts("no input file");
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
void fatal(int i, const char *arg)
{
	printf(errmsg[i], arg);
	putchar('\n');
	exit(1);
}

/*
 *	process all source files
 */
void do_pass(int p)
{
	register int fi;

	pass = p;
	radix = 10;
	rpc = pc = 0;
	fi = 0;
	mac_start_pass();
	if (ver_flag)
		printf("Pass %d\n", pass);
	if (pass == 1)				/* PASS 1 */
		open_o_files(infiles[fi]);
	else					/* PASS 2 */
		obj_header();
	while (infiles[fi] != NULL) {
		if (ver_flag)
			printf("   Read    %s\n", infiles[fi]);
		process_file(infiles[fi]);
		fi++;
	}
	mac_end_pass();
	if (pass == 1) {			/* PASS 1 */
		if (errors > 0) {
			fclose(objfp);
			unlink(objfn);
			printf("%d error(s)\n", errors);
			fatal(F_HALT, NULL);
		}
	} else {				/* PASS 2 */
		obj_end();
		fclose(objfp);
		printf("%d error(s)\n", errors);
	}
}

/*
 *	process source file fn
 */
void process_file(char *fn)
{
	register char *l, *s;

	c_line = 0;
	srcfn = fn;
	if ((srcfp = fopen(fn, READA)) == NULL)
		fatal(F_FOPEN, fn);
	do {
		l = NULL;
		while (mac_exp_nest > 0 && (l = mac_expand()) == NULL)
			;
		if (l == NULL) {
			if ((l = fgets(line, MAXLINE + 2, srcfp)) == NULL)
				break;
			if (upcase_flag)
				for (s = l; *s; s++)
					*s = TO_UPP(*s);
		}
	} while (process_line(l));
	fclose(srcfp);
	if (mac_def_nest > 0)
		asmerr(E_MISEMA);
	if (phs_flag)
		asmerr(E_MISDPH);
	if (iflevel > 0)
		asmerr(E_MISEIF);
}

/*
 *	process one line of source from l
 *	returns 0 when END encountered, otherwise 1
 */
int process_line(char *l)
{
	register char *p;
	register int old_genc, lbl_flag, expn_flag, lflag;
	register WORD op_count;
	register struct opc *op;

	/*
	 *	need expn_flag and lbl_flag, since the conditions
	 *	can change during opcode execution or macro definition
	 */
	expn_flag = (mac_exp_nest > 0);
	if (!expn_flag)
		c_line++;
	a_mode = A_STD;
	op = NULL;
	op_count = 0;
	old_genc = gencode;
	if (*l == LINCOM || (*l == LINOPT && !IS_SYM(*(l + 1))))
		a_mode = A_NONE;
	else {
		p = get_symbol(label, l, 1);
		p = get_symbol(opcode, p, 0);
		lbl_flag = (gencode && label[0] != '\0');
		if (mac_def_nest > 0) {
			if (opcode[0] != '\0')
				op = search_op(opcode);
			mac_add_line(op, l);
		} else if (opcode[0] == '\0') {
			a_mode = A_NONE;
			if (gencode) {
				if (lbl_flag) {
					put_label();
					a_mode = A_STD;
				}
			}
		} else if (mac_lookup(opcode)) {
			if (gencode) {
				if (lbl_flag)
					put_label();
				p = get_operand(operand, p, 1);
				mac_call();
				if (lbl_flag)
					a_mode = A_STD;
			} else
				a_mode = A_NONE;
		} else if ((op = search_op(opcode)) != NULL) {
			if (lbl_flag) {
				if (op->op_flags & OP_NOLBL)
					asmerr(E_INVLBL);
				else if (!(op->op_flags & OP_SET))
					if (gencode)
						put_label();
			}
			p = get_operand(operand, p, op->op_flags & OP_NOPRE);
			if (operand[0] != '\0' && operand[0] != COMMENT
					       && (op->op_flags & OP_NOOPR))
				asmerr(E_INVOPE);
			else if (gencode || (op->op_flags & OP_COND)) {
				if (pass == 2 && (op->op_flags & OP_INCL)) {
					/* list INCLUDE before include file */
					a_mode = A_NONE;
					lst_line(l, 0, 0, expn_flag);
				}
				op_count = (*op->op_fun)(op->op_c1, op->op_c2);
				if (lbl_flag && !(op->op_flags & OP_SET)
					     && a_mode == A_NONE)
					a_mode = A_STD;
			} else
				a_mode = A_NONE;
		} else if (gencode) {
			asmerr(E_INVOPC);
			a_mode = A_NONE;
		}
	}
	if (pass == 2) {
		if (gencode && (op == NULL || !(op->op_flags & OP_DS)))
			obj_writeb(op_count);
		lflag = 1;
		/* already listed INCLUDE */
		if (op != NULL && (op->op_flags & OP_INCL))
			lflag = 0;
		if (errnum == E_OK && expn_flag) {
			if (mac_list_flag == M_NONE)
				lflag = 0;
			else if (mac_list_flag == M_OPS
				 && (op_count == 0 && a_mode != A_EQU
						   && a_mode != A_DS))
				lflag = 0;
		}
		if (nofalselist && !old_genc && !gencode)
			lflag = 0;
		if (lflag)
			lst_line(l, pc, op_count, expn_flag);
	}
	if (gencode) {
		pc += op_count;
		rpc += op_count;
		return(op == NULL || !(op->op_flags & OP_END));
	} else
		return(1);
}

/*
 *	open output files
 *	input is filename of source file
 *	list and object filenames are build from source filename if
 *	not given by options
 */
void open_o_files(char *source)
{
	register char *p;

	if (*objfn == '\0')
		strcpy(objfn, source);
	if ((p = strrchr(objfn, PATHSEP)) != NULL)
		p++;
	else
		p = objfn;
	if ((p = strrchr(p, '.')) != NULL) {
		if (obj_fmt == OBJ_HEX)
			strcpy(p, OBJEXTHEX);
		else
			strcpy(p, OBJEXTBIN);
	} else {
		if (obj_fmt == OBJ_HEX)
			strcat(objfn, OBJEXTHEX);
		else
			strcat(objfn, OBJEXTBIN);
	}
	if (obj_fmt == OBJ_HEX)
		objfp = fopen(objfn, WRITEA);
	else
		objfp = fopen(objfn, WRITEB);
	if (objfp == NULL)
		fatal(F_FOPEN, objfn);
	if (list_flag) {
		if (*lstfn == '\0')
			strcpy(lstfn, source);
		if ((p = strrchr(lstfn, PATHSEP)) != NULL)
			p++;
		else
			p = lstfn;
		if ((p = strrchr(p, '.')) != NULL)
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
void get_fn(char *dest, char *src, const char *ext)
{
	register int i;
	register char *sp, *dp;

	i = 0;
	sp = src;
	dp = dest;
	while (i++ < LENFN && *sp != '\0')
		*dp++ = *sp++;
	*dp = '\0';
	if ((dp = strrchr(dest, PATHSEP)) != NULL)
		dp++;
	else
		dp = dest;
	if (strrchr(dp, '.') == NULL && (strlen(dest) + strlen(ext) <= LENFN))
		strcat(dest, ext);
}

/*
 *	get label or opcode from source line
 *	if lbl_flag is 0 skip front white space
 *	if lbl_flag is 1 skip LABSEP at end of symbol
 *	convert names to upper case and truncate length of name
 */
char *get_symbol(char *s, char *l, int lbl_flag)
{
	register int i;

	if (!lbl_flag)
		while (IS_SPC(*l))
			l++;
	if (IS_FSYM(*l)) {
		*s++ = TO_UPP(*l);
		l++;
		i = 1;
		while (IS_SYM(*l)) {
			if (i++ < symlen)
				*s++ = TO_UPP(*l);
			l++;
		}
		if (lbl_flag && *l == LABSEP)
			l++;
	}
	*s = '\0';
	return(l);
}

/*
 *	get operand into s from source line l
 *	if nopre_flag is 0 converts to upper case, and
 *	removes all unnecessary white space and comment
 *	delimited strings are copied without changes
 *	if nopre_flag is 1 removes only leading white space
 */
char *get_operand(char *s, char *l, int nopre_flag)
{
	register char *s0;
	register char c;

	s0 = s;
	while (IS_SPC(*l))
		l++;
	if (nopre_flag) {
		while (*l != '\n' && *l != '\0')
			*s++ = *l++;
	} else {
		while (*l != '\0' && *l != COMMENT) {
			if (IS_SPC(*l)) {
				l++;
				while (IS_SPC(*l))
					l++;
				/* leave one space between symbols */
				if (s > s0 && IS_SYM(*(s - 1)) && IS_SYM(*l))
					*s++ = ' ';
				continue;
			}
			if (*l != STRDEL && *l != STRDEL2) {
				*s++ = TO_UPP(*l);
				l++;
				continue;
			}
			c = *l;
			*s++ = *l++;
			if (s - s0 == 6 && strncmp(s0, "AF,AF'", 6) == 0)
				continue;
			while (1) {
				if (*l == '\n' || *l == '\0') {
					/* undelimited string */
					*s = '\0';
					return(l);
				}
				if (*l == c) {
					if (*(l + 1) == c) /* double delim? */
						*s++ = *l++;
					else
						break;
				}
				*s++ = *l++;
			}
			*s++ = *l++;
		}
	}
	*s = '\0';
	return(l);
}

/*
 *	jump to next arg in preprocessed operand p
 *	returns next arg and '\0' terminates current arg, or NULL
 *	if str_flag is not NULL stores 1 if arg is string,
 *	-1 if unterminated string otherwise 0,
 *	this is used by op_db() to differentiate between
 *	strings and expressions
 */
char *next_arg(char *p, int *str_flag)
{
	register char c;
	register int sf;

	sf = 1;					/* assume it is a string */
	while (*p != '\0' && *p != ',') {
		c = *p++;
		if (c == STRDEL || c == STRDEL2) {
			while (*p != '\0') {
				if (*p == c) {
					if (*(p + 1) == c) /* double delim? */
						p++;
					else
						break;
				}
				p++;
			}
			if (*p == '\0')	{	/* unterminated string */
				sf = -sf;
				break;
			} else {
				if (sf > 0)	/* when there were only */
					sf++;	/* strings, increment */
				p++;
			}
		} else
			sf = 0;			/* not a string */
	}
	if (str_flag != NULL) {
		if (sf == -1)			/* first string unterminated */
			*str_flag = -1;
		else if (sf == 2)		/* one valid string */
			*str_flag = 1;
		else
			*str_flag = 0;
	}
	if (*p == ',') {
		*p++ = '\0';			/* terminate previous arg */
		return(p);
	} else
		return(NULL);
}
