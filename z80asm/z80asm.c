/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	main module, handles the options and runs 2 passes over the sources
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _POSIX_C_SOURCE
#include <unistd.h>
#endif

#include "z80asm.h"
#include "z80alst.h"
#include "z80amfun.h"
#include "z80anum.h"
#include "z80aobj.h"
#include "z80aopc.h"
#include "z80apfun.h"
#include "z80atab.h"

static void init(void);
static void options(int argc, char *argv[]);
static void usage(void);
static void do_pass(int p);
static int process_line(char *line);
static void process_file(char *fn);
static void process_include(char *line, char *operand, int expn_flag);
static char *get_fn(char *src, const char *ext, int replace);
static char *get_symbol(char *s, char *line, int lbl_flag);
static void get_operand(char *s, char *line, int nopre_flag);

static const char *fatalmsg[] = {	/* error messages for fatal() */
	"out of memory: %s",		/* 0 */
	("\nz80asm version %s\n"
	 "usage: z80asm -8 -u -v -U -e<num> -f{b|m|h|c} -x "
	 "-h<num> -c<num> -m -T -p<num>\n"
	 "              -s[n|a] -o<file> -l[<file>] "
	 "-d<symbol>[=<expr>] ... <file> ..."), /* 1 */
	"Assembly halted",		/* 2 */
	"can't open file %s",		/* 3 */
	"error writing object file %s",	/* 4 */
	"internal error: %s",		/* 5 */
	"invalid page length: %s",	/* 6 */
	"invalid symbol length: %s",	/* 7 */
	"invalid C bytes per line: %s",	/* 8 */
	"invalid HEX record length: %s"	/* 9 */
};

static const char *errmsg[] = {		/* error messages for asmerr() */
	"no error",			/* 0 */
	"undefined symbol",		/* 1 */
	"invalid opcode",		/* 2 */
	"invalid operand",		/* 3 */
	"missing operand",		/* 4 */
	"multiple defined symbol",	/* 5 */
	"value out of range",		/* 6 */
	"missing right parenthesis",	/* 7 */
	"missing string delimiter",	/* 8 */
	"missing IF at ELSE of ENDIF",	/* 9 */
	"IF nested too deep",		/* 10 */
	"missing ENDIF",		/* 11 */
	"INCLUDE nested too deep",	/* 12 */
	"invalid .PHASE nesting",	/* 13 */
	"invalid ORG in .PHASE section", /* 14 */
	"missing .PHASE at .DEPHASE",	/* 15 */
	"division by zero",		/* 16 */
	"invalid expression",		/* 17 */
	"invalid label",		/* 18 */
	"missing .DEPHASE",		/* 19 */
	"not in macro definition",	/* 20 */
	"missing ENDM",			/* 21 */
	"not in macro expansion",	/* 22 */
	"macro expansion nested too deep", /* 23 */
	"too many local labels",	/* 24 */
	"label address differs between passes", /* 25 */
	"macro buffer overflow"		/* 26 */
};

static BYTE ops[OPCARRAY];		/* buffer for generated object code */
static char **infiles;			/* source filenames */
static char *srcfn;			/* filename of current source file */
static char *objfn;			/* object filename */
static char *lstfn;			/* listing filename */
static char line[MAXLINE + 2];		/* buffer for one line of source */
static char label[MAXLINE + 1];		/* buffer for label */
static char opcode[MAXLINE + 1];	/* buffer for opcode */
static char operand[MAXLINE + 1];	/* buffer for working with operand */
static int  list_flag;			/* flag for option -l */
static int  undoc_flag;			/* flag for option -u */
static int  verb_flag;			/* flag for option -v */
static int  upcase_flag;		/* flag for option -U */
static int  mac_list_opt;		/* value of option -m */
static int  list_active;		/* list output active flag */
static int  nofalselist;		/* false conditional listing flag */
static int  symlen;			/* significant characters in symbols */
static int  nfiles;			/* number of input files */
static int  pass;			/* processed pass */
static int  errors;			/* error counter */
static int  errnum;			/* error number in pass 2 */
static WORD rpc;			/* real program counter */
static WORD pc;				/* logical program counter, normally */
					/* equal to rpc, except when inside */
					/* a .PHASE section */
static FILE *srcfp;			/* file pointer for current source */
static FILE *errfp;			/* file pointer for error output */
static unsigned long c_line;		/* current line # in current source */

int main(int argc, char *argv[])
{
	init();
	options(argc, argv);
	printf("Z80/8080-Macro-Assembler  Release %s\n%s\n", RELEASE, COPYR);
	do_pass(1);
	do_pass(2);
	if (list_flag) {
		lst_mac();
		lst_sym();
		lst_close_file();
	}
	return errors;
}

/*
 *	initialization
 */
static void init(void)
{
	init_ctype();
	errfp = stdout;
}

/*
 *	process options
 */
static void options(int argc, char *argv[])
{
	register char *s, *t;
	register char **p;
	WORD val;
	int i8080_flag, obj_fmt, hexlen, carylen, nofill_flag;
	int ppl, nodate_flag, sym_opt;

	/* set default options */
	symlen      = SYMLEN;
	i8080_flag  = FALSE;
	obj_fmt     = OBJ_HEX;
	hexlen      = MAXHEX;
	carylen     = CARYLEN;
	nofill_flag = FALSE;
	ppl         = PLENGTH;
	nodate_flag = FALSE;
	sym_opt     = SYM_NONE;

	while (--argc > 0 && (*++argv)[0] == '-')
		for (s = argv[0] + 1; *s != '\0'; s++)
			switch (*s) {
			case 'o':
				if (*++s == '\0') {
					puts("name missing in option -o");
					usage();
				}
				objfn = get_fn(s, obj_file_ext(), FALSE);
				s += (strlen(s) - 1);
				break;
			case 'l':
				if (*(s + 1) != '\0') {
					lstfn = get_fn(++s, LSTEXT, FALSE);
					s += (strlen(s) - 1);
				}
				list_flag = TRUE;
				break;
			case 'T':
				nodate_flag = TRUE;
				break;
			case 's':
				if (*(s + 1) == '\0')
					sym_opt = SYM_UNSORT;
				else if (*(s + 1) == 'n')
					sym_opt = SYM_SORTN;
				else if (*(s + 1) == 'a')
					sym_opt = SYM_SORTA;
				else {
					printf("unknown option -%s\n", s);
					usage();
				}
				s += (strlen(s) - 1);
				break;
			case 'x':
				nofill_flag = TRUE;
				break;
			case 'f':
				if (*(s + 1) == 'b')
					obj_fmt = OBJ_BIN;
				else if (*(s + 1) == 'm')
					obj_fmt = OBJ_MOS;
				else if (*(s + 1) == 'h')
					obj_fmt = OBJ_HEX;
				else if (*(s + 1) == 'c')
					obj_fmt = OBJ_CARY;
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
				t = get_symbol(label, s, FALSE);
				if (t == s) {
					puts("invalid name in option -d");
					usage();
				}
				if (*t == '=') {
					get_operand(operand, ++t, FALSE);
					set_radix(10);
					val = eval(operand);
					if (errors)
						usage();
					t += strlen(t);
				} else
					val = 0;
				put_sym(label, val);
				s = t - 1;
				break;
			case '8':
				i8080_flag = TRUE;
				break;
			case 'u':
				undoc_flag = TRUE;
				break;
			case 'v':
				verb_flag = TRUE;
				break;
			case 'm':
				if (mac_list_opt == M_OPS)
					mac_list_opt = M_ALL;
				else
					mac_list_opt = M_NONE;
				break;
			case 'U':
				upcase_flag = TRUE;
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
			case 'c':
				if (*++s == '\0') {
					puts("length missing in option -c");
					usage();
				}
				carylen = atoi(s);
				if (carylen < 1 || carylen > 16)
					fatal(F_CARYLEN, s);
				s += (strlen(s) - 1);
				break;
			default:
				printf("unknown option %c\n", *s);
				usage();
				break;
			}
	if (argc == 0) {
		puts("no input file");
		usage();
	}

	nfiles = argc;
	/* some gcc versions go bonkers here, since the (int) */
	if ((infiles = (char **) malloc((int) (sizeof(char *) *
					       nfiles))) == NULL)
		fatal(F_OUTMEM, "input file names");
	for (p = infiles; argc--; p++)
		*p = get_fn(*argv++, SRCEXT, FALSE);

	obj_set_options(obj_fmt, hexlen, carylen, nofill_flag);
	if (objfn == NULL)
		objfn = get_fn(*infiles, obj_file_ext(), TRUE);

	if (list_flag) {
		lst_set_options(ppl, nodate_flag, sym_opt);
		if (lstfn == NULL)
			lstfn = get_fn(*infiles, LSTEXT, TRUE);
	}

	instrset(i8080_flag ? INSTR_8080 : INSTR_Z80);
}

/*
 *	error in options, print usage
 */
static void usage(void)
{
	fatal(F_USAGE, RELEASE);
}

/*
 *	print error message and abort
 */
void NORETURN fatal(int err, const char *arg)
{
	printf(fatalmsg[err], arg);
	putchar('\n');
	obj_close_file();
	if (objfn != NULL)
		unlink(objfn);
	exit(EXIT_FAILURE);
}

/*
 *	print error message to error output and increase error counter
 */
void asmerr(int err)
{
	if (pass == 0) {
		fputs("error in option -d: ", errfp);
		fputs(errmsg[err], errfp);
		fputc('\n', errfp);
	} else if (pass == 1) {
		fprintf(errfp, "Error in file: %s  Line: %ld\n",
			srcfn, c_line);
		fputs(line, errfp);
		fputc('\n', errfp);
		fprintf(errfp, "=> %s\n", errmsg[err]);
	} else
		errnum = err;
	errors++;
}

/*
 *	process all source files
 */
static void do_pass(int p)
{
	register int i;
	register char **ip;

	pass = p;
	set_radix(10);
	rpc = pc = 0;
	list_active = list_flag;
	mac_start_pass(pass);
	if (verb_flag)
		printf("Pass %d\n", pass);
	if (pass == 1) {			/* PASS 1 */
		obj_open_file(objfn);
		if (list_flag)
			errfp = lst_open_file(lstfn);
	} else					/* PASS 2 */
		obj_header(srcfn);
	for (i = 0, ip = infiles; i < nfiles; i++, ip++) {
		if (verb_flag)
			printf("   Read    %s\n", *ip);
		process_file(*ip);
	}
	mac_end_pass(pass);
	if (pass == 1) {			/* PASS 1 */
		if (errors > 0) {
			printf("%d error(s)\n", errors);
			fatal(F_HALT, NULL);
		}
	} else {				/* PASS 2 */
		obj_end();
		obj_close_file();
		printf("%d error(s)\n", errors);
	}
}

/*
 *	process source file fn
 */
static void process_file(char *fn)
{
	register char *s;
	register int i;
	register char *l;

	c_line = 0;
	srcfn = fn;
	lst_set_srcfn(fn);
	if ((srcfp = fopen(fn, READA)) == NULL)
		fatal(F_FOPEN, fn);
	do {
		l = NULL;
		while (mac_get_exp_nest() > 0
		       && (l = mac_expand(line)) == NULL)
			;
		if (l == NULL) {
			if ((l = fgets(line, MAXLINE + 2, srcfp)) == NULL)
				break;
			i = strlen(line) - 1;
			if (line[i] == '\n')
				line[i] = '\0';
			else if (i == MAXLINE) {
				line[i] = '\0';
				while ((i = fgetc(srcfp)) != EOF && i != '\n')
					;
			}
			if (upcase_flag)
				for (s = l; *s; s++)
					*s = TO_UPP(*s);
		}
	} while (process_line(l));
	fclose(srcfp);
	if (in_phase_section())
		asmerr(E_MISDPH);
	if (in_cond_section())
		asmerr(E_MISEIF);
	if (mac_get_def_nest() > 0)
		asmerr(E_MISEMA);
}

/*
 *	process one line of source from line
 *	returns FALSE when END encountered, otherwise TRUE
 */
static int process_line(char *line)
{
	register opc_t *op;
	register WORD op_count;
	char *p;
	int gencode;		/* we are not in a false conditional section */
	int genc_lbl_flag;	/* we are not in a false conditional section,
				   and there is a label present */
	int expn_flag;		/* we are in a macro expansion */
	int new_gencode, lflag, a_mode;
	WORD a_addr;

	expn_flag = (mac_get_exp_nest() > 0);
	if (!expn_flag)
		c_line++;
	a_mode = A_STD;
	errnum = E_OK;
	op = NULL;
	op_count = 0;
	gencode = in_true_section();

	if (*line == LINCOM || (*line == LINOPT && !IS_SYM(*(line + 1)))) {
		/* a line comment, nothing to do */
		a_mode = A_NONE;
	} else {
		p = get_symbol(label, line, TRUE);
		p = get_symbol(opcode, p, FALSE);
		genc_lbl_flag = (gencode && label[0] != '\0');

		if (mac_get_def_nest() > 0) {
			/* inside a macro definition, add line to macro */
			a_mode = A_NONE;
			if (opcode[0] != '\0')
				op = search_op(opcode);
			mac_add_line(op, line);
		} else if (opcode[0] == '\0') {
			/* line without an op-code */
			if (genc_lbl_flag)
				put_label(label, pc, pass);
			else
				a_mode = A_NONE;
		} else if (mac_lookup(opcode)) {
			/* a macro call, start macro expansion */
			if (gencode) {
				if (genc_lbl_flag)
					put_label(label, pc, pass);
				get_operand(operand, p, TRUE);
				mac_call(operand);
			} else
				a_mode = A_NONE;
		} else if ((op = search_op(opcode)) != NULL) {
			/* normal line with op-code */
			if (genc_lbl_flag) {
				/* if op-code doesn't allow label, error out */
				if (op->op_flags & OP_NOLBL)
					asmerr(E_INVLBL);
				/* else, if op-code is not of symbol defining
				   class, this is a true label */
				else if (!(op->op_flags & OP_SET))
					put_label(label, pc, pass);
			}
			get_operand(operand, p, op->op_flags & OP_NOPRE);
			/* if an operand is present and the op-code doesn't
			   have one, error out */
			if (operand[0] != '\0' && operand[0] != COMMENT
					       && (op->op_flags & OP_NOOPR))
				asmerr(E_INVOPE);
			/* else, if we are not in a false conditional section,
			   or the op-code is a conditional, that could change
			   this, call its processing function */
			else if (gencode || (op->op_flags & OP_COND)) {
				if (op->op_func == NULL) {
					/* INCLUDE or MACLIB */
					process_include(line, operand,
							expn_flag);
				} else {
					op_count =
						(*op->op_func)(pass, op->op_c1,
							       op->op_c2,
							       operand, ops);
					a_mode = op->op_a_mode;
					/* if there is a true label present,
					   and the op-code's listing address
					   mode is "no address", change it */
					if (genc_lbl_flag
					    && !(op->op_flags & OP_SET)
					    && a_mode == A_NONE)
						a_mode = A_STD;
				}
			} else {
				/* we are in a false conditional section */
				a_mode = A_NONE;
			}
		} else if (gencode) {
			/* an invalid op-code */
			asmerr(E_INVOPC);
			a_mode = A_NONE;
		}
	}

	new_gencode = in_true_section();

	if (pass == 2) {
		/* output object code when not in a false conditional section,
		   except for DS, which did this already */
		if (new_gencode && (op == NULL || !(op->op_flags & OP_DS)))
			obj_writeb(ops, op_count);

		/* determine if line should be listed, default TRUE */
		lflag = TRUE;

		/* INCLUDE/MACLIB does list output by itself */
		if (op != NULL && op->op_func == NULL)
			lflag = FALSE;

		/* if this is a macro expansion, list according to the macro
		   list options, but always list when an error occured */
		if (errnum == E_OK && expn_flag) {
			if (mac_list_opt == M_NONE)
				lflag = FALSE;
			else if (mac_list_opt == M_OPS
				 && (op_count == 0 && a_mode != A_EQU
						   && a_mode != A_DS))
				lflag = FALSE;
		}

		/* don't list, if we are in a false conditional section and
		   "don't list false sections" is on, but list the conditional
		   that made this into a false section */
		if (nofalselist && !gencode && !new_gencode)
			lflag = FALSE;

		if (lflag) {
			/* list the line, EQU and SET use the set symbol
			   as the address for the address column */
			if (a_mode == A_EQU || a_mode == A_SET)
				a_addr = sym_lastval();
			else
				a_addr = pc;
			lst_line(line, list_active, expn_flag, a_mode, a_addr,
				 ops, op_count, c_line,
				 errnum == E_OK ? NULL : errmsg[errnum]);
		}
	}

	/* if we generated object code, advance the PC */
	if (new_gencode) {
		pc += op_count;
		rpc += op_count;
		return op == NULL || !(op->op_flags & OP_END);
	} else
		return TRUE;
}

/*
 *	process INCLUDE and MACLIB
 */
static void process_include(char *line, char *operand, int expn_flag)
{
	register char *p;
	unsigned long inc_line;
	char *inc_fn, *fn;
	FILE *inc_fp;
	static int incnest;

	if (incnest >= INCNEST) {
		asmerr(E_INCNST);
		return;
	}
	inc_line = c_line;
	inc_fn = srcfn;
	inc_fp = srcfp;
	incnest++;
	p = operand;
	while (!IS_SPC(*p) && *p != COMMENT && *p != '\0')
		p++;
	*p = '\0';
	fn = strsave(operand);
	if (pass == 2)
		lst_line(line, list_active, expn_flag, A_NONE, 0, NULL, 0,
			 c_line, NULL);
	if (verb_flag)
		printf("   Include %s\n", fn);
	process_file(fn);
	free(fn);
	incnest--;
	c_line = inc_line;
	srcfn = inc_fn;
	srcfp = inc_fp;
	if (verb_flag)
		printf("   Resume  %s\n", srcfn);
	if (list_active && pass == 2)
		lst_eject(TRUE);
}

/*
 *	return a filename created from "src" and "ext"
 *	append "ext" if "src" has no extension
 *	replace existing extension with "ext" if "replace" is TRUE
 */
static char *get_fn(char *src, const char *ext, int replace)
{
	register char *sp, *dp;
	register char *ep;
	int n, m;

	if ((sp = strrchr(src, PATHSEP)) == NULL)
		sp = src;
	else
		sp++;
	if ((ep = strrchr(sp, '.')) == NULL)
		n = strlen(src) + strlen(ext);
	else
		n = (m = ep - src) + (replace ? strlen(ext) : strlen(ep));
	if ((dp = (char *) malloc(n + 1)) == NULL)
		fatal(F_OUTMEM, "file name");
	if (ep == NULL) {
		strcpy(dp, src);
		strcat(dp, ext);
	} else {
		strncpy(dp, src, m);
		strcpy(dp + m, (replace ? ext : ep));
	}
	return dp;
}

/*
 *	save string into allocated memory
 */
char *strsave(const char *s)
{
	register char *p;

	if ((p = (char *) malloc(strlen(s) + 1)) == NULL)
		fatal(F_OUTMEM, "strsave");
	return strcpy(p, s);
}

/*
 *	get label or opcode from source line
 *	if lbl_flag is FALSE skip front white space
 *	if lbl_flag is TRUE skip LABSEP at end of symbol
 *	convert names to upper case and truncate length of name
 */
static char *get_symbol(char *s, char *line, int lbl_flag)
{
	register int i;

	if (!lbl_flag)
		while (IS_SPC(*line))
			line++;
	if (IS_FSYM(*line)) {
		*s++ = TO_UPP(*line);
		line++;
		i = 1;
		while (IS_SYM(*line)) {
			if (i++ < symlen)
				*s++ = TO_UPP(*line);
			line++;
		}
		if (lbl_flag && *line == LABSEP)
			line++;
	}
	*s = '\0';
	return line;
}

/*
 *	get operand into s from source line
 *	if nopre_flag is FALSE converts to upper case, and
 *	removes all unnecessary white space and comment
 *	delimited strings are copied without changes
 *	if nopre_flag is TRUE removes only leading white space
 */
static void get_operand(char *s, char *line, int nopre_flag)
{
	register char *s0;
	register char c;

	while (IS_SPC(*line))
		line++;
	if (nopre_flag) {
		strcpy(s, line);
		return;
	}
	s0 = s;
	while (*line != '\0' && *line != COMMENT) {
		if (IS_SPC(*line)) {
			line++;
			while (IS_SPC(*line))
				line++;
			/* leave one space between symbols */
			if (s > s0 && IS_SYM(*(s - 1)) && IS_SYM(*line))
				*s++ = ' ';
		} else if (*line == STRDEL || *line == STRDEL2) {
			*s++ = c = *line++;
			if (s - s0 == 6 && strncmp(s0, "AF,AF'", 6) == 0)
				continue;
			while (TRUE) {
				if (*line == '\0') {
					/* undelimited string */
					*s = '\0';
					return;
				} else if (*line == c) {
					/* double delim? */
					if (*(line + 1) != c)
						break;
					else
						*s++ = *line++;
				}
				*s++ = *line++;
			}
			*s++ = *line++;
		} else {
			*s++ = TO_UPP(*line);
			line++;
		}
	}
	*s = '\0';
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
		return p;
	} else
		return NULL;
}

/*
 *	return undocumented instructions allowed flag
 */
int undoc_allowed(void)
{
	return undoc_flag;
}

/*
 *	return significant characters in symbols
 */
int get_symlen(void)
{
	return symlen;
}

/*
 *	get current source line label
 */
const char *get_label(void)
{
	return label;
}

/*
 *	get program counter
 */
WORD get_pc(void)
{
	return pc;
}

/*
 *	set program counter
 */
void set_pc(int opt, WORD addr)
{
	switch (opt) {
	case PC_ORG:
		pc = rpc = addr;
		break;
	case PC_PHASE:
		pc = addr;
		break;
	case PC_DEPHASE:
		pc = rpc;
		break;
	default:
		fatal(F_INTERN, "invalid option for function set_pc");
		break;
	}
}

/*
 *	set list output active flag
 */
void set_list_active(int flag)
{
	if (list_flag)
		list_active = flag;
}

/*
 *	set macro list options
 */
void set_mac_list_opt(int opt)
{
	switch (opt) {
	case M_OPS:
	case M_ALL:
	case M_NONE:
		mac_list_opt = opt;
		break;
	default:
		fatal(F_INTERN,
		      "invalid option for function set_mac_list_opt");
		break;
	}
}

/*
 *	set no list false conditional section flag
 */
void set_nofalselist(int flag)
{
	nofalselist = flag;
}
