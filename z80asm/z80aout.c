/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2024 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	module for output functions to list, object and error files
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "z80a.h"
#include "z80aglb.h"

void lst_byte(BYTE);
void pos_fill_bin(void);
void pos_fill_cary(void);
void cary_byte(BYTE);
void eof_hex(WORD);
void flush_hex(void);
void hex_record(BYTE);
BYTE chksum(BYTE);
char *btoh(BYTE, char *);

/* z80amain.c */
extern void fatal(int, const char *) NORETURN;

/* z80amfun.c */
extern char *mac_first(int, int *);
extern char *mac_next(int *);

/* z80atab.c */
extern struct sym *first_sym(int);
extern struct sym *next_sym(void);

/*
 *	Intel HEX record types
 */
#define	HEX_DATA	0
#define HEX_EOF		1

static const char *errmsg[] = {			/* error messages for asmerr */
	"no error",				/* 0 */
	"undefined symbol",			/* 1 */
	"invalid opcode",			/* 2 */
	"invalid operand",			/* 3 */
	"missing operand",			/* 4 */
	"multiple defined symbol",		/* 5 */
	"value out of range",			/* 6 */
	"missing right parenthesis",		/* 7 */
	"missing string delimiter",		/* 8 */
	"missing IF at ELSE of ENDIF",		/* 9 */
	"IF nested too deep",			/* 10 */
	"missing ENDIF",			/* 11 */
	"INCLUDE nested too deep",		/* 12 */
	"invalid .PHASE nesting",		/* 13 */
	"invalid ORG in .PHASE block",		/* 14 */
	"missing .PHASE at .DEPHASE",		/* 15 */
	"division by zero",			/* 16 */
	"invalid expression",			/* 17 */
	"invalid label",			/* 18 */
	"missing .DEPHASE",			/* 19 */
	"not in macro definition",		/* 20 */
	"missing ENDM",				/* 21 */
	"not in macro expansion",		/* 22 */
	"macro expansion nested too deep",	/* 23 */
	"too many local labels",		/* 24 */
	"label address differs between passes",	/* 25 */
	"macro buffer overflow"			/* 26 */
};

static WORD curr_addr;			/* current logical file address */
static WORD eof_addr;			/* address at binary/C end of file */
static WORD hex_addr;			/* current address in HEX record */
static WORD hex_cnt;			/* number of bytes in HEX buffer */
static long code_start;			/* file position where code begins */
static int  neof_flag;			/* not at EOF flag */
static int  nl_size;			/* size of newline in text files */

static BYTE hex_buf[MAXHEX];		/* buffer for one HEX record */
static char hex_out[MAXHEX * 2 + 13];	/* ASCII buffer for one HEX record */

/*
 *	print error message to listfile and increase error counter
 */
void asmerr(int i)
{
	if (pass == 0) {
		fputs("error in option -d: ", errfp);
		fputs(errmsg[i], errfp);
		fputc('\n', errfp);
	} else if (pass == 1) {
		fprintf(errfp, "Error in file: %s  Line: %ld\n",
			srcfn, c_line);
		fputs(line, errfp);
		fputc('\n', errfp);
		fprintf(errfp, "=> %s\n", errmsg[i]);
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
	time_t tloc;

	if (header_done && ppl != 0)
		fputc('\f', lstfp);
	if (!header_done || ppl != 0) {
		fprintf(lstfp, "Z80/8080-Macro-Assembler  Release %s",
			RELEASE);
		if (!nodate_flag) {
			tloc = time(&tloc);
			fprintf(lstfp, "\t%.24s", ctime(&tloc));
		}
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
	header_done = TRUE;
}

/*
 *	print header for source lines
 */
void lst_attl(void)
{
	static int attl_done;

	if (!attl_done || ppl != 0)
		fprintf(lstfp,
			"\nLOC   OBJECT CODE   LINE   STMT SOURCE CODE\n");
	if (ppl != 0)
		p_line += 2;
	attl_done = TRUE;
}

/*
 *	print one line into listfile, if -l option set
 */
void lst_line(char *l, WORD addr, WORD op_cnt, int expn_flag)
{
	register int i, j;
	register const char *a_mark;
	static unsigned long s_line;

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
	a_mark = "  ";
	switch (a_mode) {
	case A_STD:
		a_addr = addr;
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
		lst_byte(a_addr >> 8);
		lst_byte(a_addr & 0xff);
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
	if (*l) {
		fputc(' ', lstfp);
		fputs(l, lstfp);
	}
	fputc('\n', lstfp);
	if (errnum != E_OK) {
		fprintf(errfp, "=> %s\n", errmsg[errnum]);
		errnum = E_OK;
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
 *	print macro table into listfile, sorted as specified in sort_mode
 */
void lst_mac(int sort_mode)
{
	register int col, prev_len;
	register char *p;
	int prev_rf, rf;

	p_line = col = prev_len = 0;
	prev_rf = TRUE;
	strcpy(title, "Macro table");
	for (p = mac_first(sort_mode, &rf); p != NULL; p = mac_next(&rf)) {
		if (col + mac_symmax + 1 >= 80) {
			if (!prev_rf)
				fputc('*', lstfp);
			fputc('\n', lstfp);
			if (ppl != 0 && ++p_line >= ppl)
				p_line = 0;
			col = 0;
		} else if (col > 0) {
			fputc(prev_rf? ' ' : '*', lstfp);
			while (prev_len++ < mac_symmax)
				fputc(' ', lstfp);
			fputs("   ", lstfp);
		}
		if (p_line == 0) {
			lst_header();
			if (ppl == 0) {
				fputs(title, lstfp);
				fputc('\n', lstfp);
			}
			fputc('\n', lstfp);
			p_line++;
		}
		prev_len = strlen(p);
		fprintf(lstfp, "%s", p);
		prev_rf = rf;
		col += mac_symmax + 4;
	}
	if (col > 0) {
		if (!prev_rf)
			fputc('*', lstfp);
		fputc('\n', lstfp);
	}
}

/*
 *	print symbol table into listfile, sorted as specified in sort_mode
 */
void lst_sym(int sort_mode)
{
	register int col;
	register struct sym *sp;
	int prev_rf;

	p_line = col = 0;
	prev_rf = TRUE;
	strcpy(title, "Symbol table");
	for (sp = first_sym(sort_mode); sp != NULL; sp = next_sym()) {
		if (col + symmax + 6 >= 80) {
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
		fprintf(lstfp, "%*s ", -symmax, sp->sym_name);
		lst_byte(sp->sym_val >> 8);
		lst_byte(sp->sym_val & 0xff);
		prev_rf = sp->sym_refflg;
		col += symmax + 9;
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
void lst_byte(BYTE b)
{
	register char c;

	c = b >> 4;
	fputc(c + (c < 10 ? '0' : 'W'), lstfp);
	c = b & 0xf;
	fputc(c + (c < 10 ? '0' : 'W'), lstfp);
}

/*
 *	write header record into object file
 */
void obj_header(void)
{
	long before_nl, after_nl;

	switch (obj_fmt) {
	case OBJ_BIN:
		code_start = 0L;
		eof_addr = load_addr;
		break;
	case OBJ_MOS:
		if (fputc(0xff, objfp) == EOF ||
		    fputc(load_addr & 0xff, objfp) == EOF ||
		    fputc(load_addr >> 8, objfp) == EOF)
			fatal(F_OBJFILE, objfn);
		code_start = 3L;
		eof_addr = load_addr;
		break;
	case OBJ_HEX:
		break;
	case OBJ_CARY:
		if (fprintf(objfp, "// build from source file %s",
			    srcfn) < 0 ||
		    (before_nl = ftell(objfp)) < 0L ||
		    fputc('\n', objfp) == EOF ||
		    (after_nl = ftell(objfp)) < 0L ||
		    fputs("unsigned char code[MEMSIZE] = {", objfp) == EOF ||
		    (code_start = ftell(objfp)) < 0L)
			fatal(F_OBJFILE, objfn);
		nl_size = after_nl - before_nl;
		eof_addr = load_addr;
		break;
	default:
		fatal(F_INTERN, "invalid obj_fmt for function obj_header");
		break;
	}
}

/*
 *	write end record into object file
 */
void obj_end(void)
{
	switch (obj_fmt) {
	case OBJ_BIN:
	case OBJ_MOS:
		if (!nofill_flag && curr_addr > eof_addr)
			pos_fill_bin();
		break;
	case OBJ_HEX:
		flush_hex();
		eof_hex(start_addr);
		break;
	case OBJ_CARY:
		if (!nofill_flag && curr_addr > eof_addr)
			pos_fill_cary();
		else if (neof_flag) {
			if (fseek(objfp, 0L, SEEK_END) < 0)
				fatal(F_OBJFILE, objfn);
		}
		if (fputs("\n};\n", objfp) == EOF ||
		    fprintf(objfp, "unsigned short code_length = %u;\n",
			    eof_addr - load_addr) < 0 ||
		    fprintf(objfp, "unsigned short code_load_addr = 0x%04x;\n",
			    load_addr) < 0)
			fatal(F_OBJFILE, objfn);
		if (start_addr >= load_addr) {
			if (fprintf(objfp, "unsigned short "
				    "code_start_addr = 0x%04x;\n",
				    start_addr) < 0)
				fatal(F_OBJFILE, objfn);
		}
		break;
	default:
		fatal(F_INTERN, "invalid obj_fmt for function obj_end");
		break;
	}
}

/*
 *	set logical address for object file
 */
void obj_org(WORD addr)
{
	curr_addr = addr;
}

/*
 *	write opcodes in ops[] into object file
 */
void obj_writeb(WORD op_cnt)
{
	register int i;

	if (op_cnt == 0)
		return;
	switch (obj_fmt) {
	case OBJ_BIN:
	case OBJ_MOS:
		pos_fill_bin();
		if (fwrite(ops, 1, op_cnt,
			   objfp) != op_cnt)
			fatal(F_OBJFILE, objfn);
		curr_addr += op_cnt;
		if (curr_addr > eof_addr)
			eof_addr = curr_addr;
		break;
	case OBJ_CARY:
		pos_fill_cary();
		for (i = 0; i < op_cnt; i++) {
			cary_byte(ops[i]);
			curr_addr++;
		}
		if (curr_addr > eof_addr)
			eof_addr = curr_addr;
		break;
	case OBJ_HEX:
		if (hex_addr + hex_cnt != curr_addr)
			flush_hex();
		for (i = 0; op_cnt > 0; op_cnt--) {
			if (hex_cnt >= hexlen)
				flush_hex();
			hex_buf[hex_cnt++] = ops[i++];
			curr_addr++;
		}
		break;
	default:
		fatal(F_INTERN, "invalid obj_fmt for function obj_writeb");
		break;
	}
}

/*
 *	advance logical address of object file by count
 */
void obj_fill(WORD count)
{
	curr_addr += count;
}

/*
 *	write <count> bytes <value> into object file
 */
void obj_fill_value(WORD count, WORD value)
{
	if (count == 0)
		return;
	switch (obj_fmt) {
	case OBJ_BIN:
	case OBJ_MOS:
		pos_fill_bin();
		while (count-- > 0) {
			if (fputc(value, objfp) == EOF)
				fatal(F_OBJFILE, objfn);
			curr_addr++;
		}
		if (curr_addr > eof_addr)
			eof_addr = curr_addr;
		break;
	case OBJ_CARY:
		pos_fill_cary();
		while (count-- > 0) {
			cary_byte(value);
			curr_addr++;
		}
		if (curr_addr > eof_addr)
			eof_addr = curr_addr;
		break;
	case OBJ_HEX:
		if (hex_addr + hex_cnt != curr_addr)
			flush_hex();
		while (count-- > 0) {
			if (hex_cnt >= hexlen)
				flush_hex();
			hex_buf[hex_cnt++] = value;
			curr_addr++;
		}
		break;
	default:
		fatal(F_INTERN, "invalid obj_fmt for function obj_fill_value");
		break;
	}
}

/*
 *	fill binary object file up to the logical address with 0xff bytes
 *	or set file position to the logical address if curr_addr < eof_addr
 */
void pos_fill_bin(void)
{
	WORD addr;

	if (curr_addr < eof_addr) {
		addr = curr_addr - load_addr;
		if (fseek(objfp, code_start + addr, SEEK_SET) < 0)
			fatal(F_OBJFILE, objfn);
		neof_flag = TRUE;
	} else {
		if (neof_flag) {
			if (fseek(objfp, 0L, SEEK_END) < 0)
				fatal(F_OBJFILE, objfn);
			neof_flag = FALSE;
		}
		while (eof_addr < curr_addr) {
			if (fputc(0xff, objfp) == EOF)
				fatal(F_OBJFILE, objfn);
			eof_addr++;
		}
	}
}

/*
 *	fill C array object file up to the logical address with 0xff bytes
 *	or set file position to the logical address if curr_addr < eof_addr
 */
void pos_fill_cary(void)
{
	WORD addr;
	long pos;

	if (curr_addr < eof_addr) {
		/* calculate position of byte at curr_addr in C file */
		addr = curr_addr - load_addr;
		pos = code_start;
		if (addr > 0) {
			/* "\n\t" */
			pos += nl_size + 1L;
			/* "0x%02x" */
			pos += 4L * addr;
			/* don't include "," and formatting for addr,
			   they will be printed again */
			addr--;
			/* "," */
			pos += addr;
			if (carylen <= CARYSPC) {
				/* " " */
				pos += addr;
				/* but not for the last byte on a line */
				pos -= addr / carylen;
			}
			/* "\n\t" */
			pos += (nl_size + 1L) * (addr / carylen);
		}
		if (fseek(objfp, pos, SEEK_SET) < 0)
			fatal(F_OBJFILE, objfn);
		neof_flag = TRUE;
	} else {
		if (neof_flag) {
			if (fseek(objfp, 0L, SEEK_END) < 0)
				fatal(F_OBJFILE, objfn);
			neof_flag = FALSE;
		}
		addr = curr_addr;
		curr_addr = eof_addr;
		while (curr_addr < addr) {
			cary_byte(0xff);
			curr_addr++;
		}
		eof_addr = curr_addr;
	}
}

/*
 *	output a C array byte
 */
void cary_byte(BYTE b)
{
	if (curr_addr == load_addr) {
		if (fputs("\n\t", objfp) == EOF)
			fatal(F_OBJFILE, objfn);
	} else {
		if (fputc(',', objfp) == EOF)
			fatal(F_OBJFILE, objfn);
		if (((curr_addr - load_addr) % carylen) == 0) {
			if (fputs("\n\t", objfp) == EOF)
				fatal(F_OBJFILE, objfn);
		} else if (carylen <= CARYSPC) {
			if (fputc(' ', objfp) == EOF)
				fatal(F_OBJFILE, objfn);
		}
	}
	if (fprintf(objfp, "0x%02x", b) < 0)
		fatal(F_OBJFILE, objfn);
}

/*
 *	create a HEX end-of-file record in ASCII and write into object file
 */
void eof_hex(WORD addr)
{
	hex_cnt = 0;
	hex_addr = addr;
	hex_record(HEX_EOF);
}

/*
 *	create a HEX data record in ASCII and write into object file
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
 *	write a HEX record in ASCII and write into object file
 */
void hex_record(BYTE rec_type)
{
	register int i;
	register char *p;

	p = hex_out;
	*p++ = ':';
	p = btoh(hex_cnt, p);
	p = btoh(hex_addr >> 8, p);
	p = btoh(hex_addr & 0xff, p);
	p = btoh(rec_type, p);
	for (i = 0; i < hex_cnt; i++)
		p = btoh(hex_buf[i], p);
	p = btoh(chksum(rec_type), p);
	*p++ = '\n';
	*p = '\0';
	if (fputs(hex_out, objfp) == EOF)
		fatal(F_OBJFILE, objfn);
}

/*
 *	convert BYTE into ASCII hex and copy to string at p
 *	returns p increased by 2
 */
char *btoh(BYTE b, char *p)
{
	register char c;

	c = b >> 4;
	*p++ = c + (c < 10 ? '0' : '7');
	c = b & 0xf;
	*p++ = c + (c < 10 ? '0' : '7');
	return (p);
}

/*
 *	compute checksum for Intel HEX record
 */
BYTE chksum(BYTE rec_type)
{
	register int i;
	register BYTE sum;

	sum = hex_cnt;
	sum += hex_addr >> 8;
	sum += hex_addr & 0xff;
	sum += rec_type;
	for (i = 0; i < hex_cnt; i++)
		sum += hex_buf[i];
	return (-sum);
}
