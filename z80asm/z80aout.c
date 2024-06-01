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
void fill_bin(void);
void fill_cary(void);
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
	"non-sequential object code",		/* 9 */
	"missing IF at ELSE of ENDIF",		/* 10 */
	"IF nested too deep",			/* 11 */
	"missing ENDIF",			/* 12 */
	"INCLUDE nested too deep",		/* 13 */
	"invalid .PHASE nesting",		/* 14 */
	"invalid ORG in .PHASE block",		/* 15 */
	"missing .PHASE at .DEPHASE",		/* 16 */
	"division by zero",			/* 17 */
	"invalid expression",			/* 18 */
	"object code before ORG",		/* 19 */
	"invalid label",			/* 20 */
	"missing .DEPHASE",			/* 21 */
	"not in macro definition",		/* 22 */
	"missing ENDM",				/* 23 */
	"not in macro expansion",		/* 24 */
	"macro expansion nested too deep",	/* 25 */
	"too many local labels",		/* 26 */
	"label address differs between passes",	/* 27 */
	"macro buffer overflow"			/* 28 */
};

static int nseq_flag;			/* flag for non-sequential ORG */
static WORD curr_addr;			/* current logical file address */
static WORD bin_addr;			/* current address written to file */
static WORD hex_addr;			/* current address in HEX record */
static WORD hex_cnt;			/* number of bytes in HEX buffer */
static WORD cary_cnt;			/* number of bytes in C array */

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
	fprintf(lstfp, "%c%5lu %6lu %s\n", expn_flag ? '+' : ' ',
		c_line, s_line, l);
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
	register int i, j;
	register char *p;
	int rf;

	p_line = i = 0;
	strcpy(title, "Macro table");
	for (p = mac_first(sort_mode, &rf); p != NULL; p = mac_next(&rf)) {
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
		fprintf(lstfp, "%s%c", p, rf ? ' ' : '*');
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
	}
	if (i > 0)
		fputc('\n', lstfp);
}

/*
 *	print symbol table into listfile, sorted as specified in sort_mode
 */
void lst_sym(int sort_mode)
{
	register int i;
	register struct sym *sp;

	p_line = i = 0;
	strcpy(title, "Symbol table");
	for (sp = first_sym(sort_mode); sp != NULL; sp = next_sym()) {
		if (p_line == 0) {
			lst_header();
			if (ppl == 0) {
				fputs(title, lstfp);
				fputc('\n', lstfp);
			}
			fputc('\n', lstfp);
			p_line++;
		}
		fprintf(lstfp, "%*s ", -symmax, sp->sym_name);
		lst_byte(sp->sym_val >> 8);
		lst_byte(sp->sym_val & 0xff);
		fputc(sp->sym_refflg ? ' ' : '*', lstfp);
		i += symmax + 9;
		if (i + symmax + 6 >= 80) {
			fputc('\n', lstfp);
			if (ppl != 0 && ++p_line >= ppl)
				p_line = 0;
			i = 0;
		} else
			fputs("   ", lstfp);
	}
	if (i > 0)
		fputc('\n', lstfp);
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
	switch (obj_fmt) {
	case OBJ_BIN:
		break;
	case OBJ_MOS:
		fputc(0xff, objfp);
		fputc(load_addr & 0xff, objfp);
		fputc(load_addr >> 8, objfp);
		break;
	case OBJ_HEX:
		break;
	case OBJ_CARY:
		fprintf(objfp, "//build from source file %s\n", srcfn);
		fputs("unsigned char code[MEMSIZE] = {", objfp);
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
		if (!nofill_flag && !(load_flag && (bin_addr < load_addr)))
			fill_bin();
		break;
	case OBJ_HEX:
		flush_hex();
		eof_hex(start_addr);
		break;
	case OBJ_CARY:
		if (!nofill_flag && !(load_flag && (bin_addr < load_addr)))
			fill_cary();
		fputs("\n};\n", objfp);
		fprintf(objfp, "unsigned short code_length = %u;\n",
			cary_cnt);
		fprintf(objfp, "unsigned short code_load_addr = 0x%04x;\n",
			load_addr);
		if (start_addr >= load_addr)
			fprintf(objfp, "unsigned short "
				"code_start_addr = 0x%04x;\n", start_addr);
		break;
	}
}

/*
 *	set logical address for object file
 */
void obj_org(WORD addr)
{
	switch (obj_fmt) {
	case OBJ_BIN:
	case OBJ_MOS:
	case OBJ_CARY:
		nseq_flag = (addr < curr_addr);
		if (load_flag && (bin_addr < load_addr))
			bin_addr = addr;
		curr_addr = addr;
		break;
	case OBJ_HEX:
		curr_addr = addr;
		break;
	}
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
	case OBJ_CARY:
		if (nseq_flag)
			asmerr(E_NSQWRT);
		else {
			if (load_flag && (bin_addr < load_addr))
				asmerr(E_BFRORG);
			else {
				if (obj_fmt == OBJ_CARY) {
					fill_cary();
					for (i = 0; i < op_cnt; i++)
						cary_byte(ops[i]);
				} else {
					fill_bin();
					fwrite(ops, 1, op_cnt, objfp);
				}
				bin_addr += op_cnt;
			}
			curr_addr += op_cnt;
		}
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
	}
}

/*
 *	advance logical address of object file by count
 */
void obj_fill(WORD count)
{
	if (count == 0)
		return;
	switch (obj_fmt) {
	case OBJ_BIN:
	case OBJ_MOS:
	case OBJ_CARY:
		if (!nseq_flag)
			curr_addr += count;
		break;
	case OBJ_HEX:
		curr_addr += count;
		break;
	}
}

/*
 *	write <count> bytes <value> into object file
 */
void obj_fill_value(WORD count, WORD value)
{
	register WORD n;

	if (count == 0)
		return;
	n = count;
	switch (obj_fmt) {
	case OBJ_BIN:
	case OBJ_MOS:
	case OBJ_CARY:
		if (nseq_flag)
			asmerr(E_NSQWRT);
		else {
			if (load_flag && (bin_addr < load_addr))
				asmerr(E_BFRORG);
			else {
				if (obj_fmt == OBJ_CARY) {
					fill_cary();
					while (n-- > 0)
						cary_byte(value);
				} else {
					fill_bin();
					while (n-- > 0)
						fputc(value, objfp);
				}
				bin_addr += count;
			}
			curr_addr += count;
		}
		break;
	case OBJ_HEX:
		if (hex_addr + hex_cnt != curr_addr)
			flush_hex();
		while (n-- > 0) {
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
		fputc(0xff, objfp);
		bin_addr++;
	}
}

/*
 *	fill C array object file up to the logical address with 0xff bytes
 */
void fill_cary(void)
{
	while (bin_addr < curr_addr) {
		cary_byte(0xff);
		bin_addr++;
	}
}

/*
 *	Output a C array byte
 */
void cary_byte(BYTE b)
{
	if (cary_cnt == 0)
		fputs("\n\t", objfp);
	else {
		fputc(',', objfp);
		if ((cary_cnt % carylen) == 0)
			fputs("\n\t", objfp);
		else if (carylen <= 12)
			fputc(' ', objfp);
	}
	fprintf(objfp, "0x%02x", b);
	cary_cnt++;
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
	fputs(hex_out, objfp);
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
