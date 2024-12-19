/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2024 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	module for output functions to object files
 */

#include <stddef.h>
#include <stdio.h>

#include "z80asm.h"
#include "z80aobj.h"

static void pos_fill_bin(void);

static void pos_fill_cary(void);
static void cary_byte(BYTE b);

static void eof_hex(WORD addr);
static void flush_hex(void);
static void hex_record(BYTE rec_type);
static char *btoh(BYTE b, char *p);
static BYTE chksum(BYTE rec_type);

#ifndef SEEK_SET
#define SEEK_SET	0
#endif
#ifndef SEEK_END
#define SEEK_END	2
#endif

/*
 *	Intel HEX record types
 */
#define	HEX_DATA	0
#define HEX_EOF		1

static int  obj_fmt;			/* format of object file */
static const char *objfn;		/* object filename */
static FILE *objfp;			/* file pointer for object code */
static WORD load_addr;			/* load address of program */
static WORD start_addr;			/* execution start addr of program */
static WORD curr_addr;			/* current logical file address */
static WORD eof_addr;			/* address at binary/C end of file */
static WORD hex_addr;			/* current address in HEX record */
static WORD hex_cnt;			/* number of bytes in HEX buffer */
static long code_start;			/* file position where code begins */
static int  neof_flag;			/* not at EOF flag */
static int  nl_size;			/* size of newline in text files */
static int  hexlen;			/* HEX record length */
static int  carylen;			/* C array bytes per line */
static int  nofill_flag;		/* don't fill up object code flag */

static BYTE hex_buf[MAXHEX];		/* buffer for one HEX record */
static char hex_out[MAXHEX * 2 + 13];	/* ASCII buffer for one HEX record */

static const struct {
	const char *ext;
	const char *mode;
} obj_str[] = {
	{ OBJEXTBIN, WRITEB },	/* OBJ_BIN */
	{ OBJEXTBIN, WRITEB },	/* OBJ_MOS */
	{ OBJEXTHEX, WRITEA },	/* OBJ_HEX */
	{ OBJEXTCARY, WRITEA }	/* OBJ_CARY */
};

/*
 *	set object file options
 */
void obj_set_options(int fmt, int hexl, int caryl, int nofill)
{
	obj_fmt = fmt;
	hexlen = hexl;
	carylen = caryl;
	nofill_flag = nofill;
}

/*
 *	return object file extension
 */
const char *obj_file_ext(void)
{
	return obj_str[obj_fmt].ext;
}

/*
 *	set object file
 */
void obj_open_file(const char *fn)
{
	objfn = fn;
	objfp = fopen(objfn, obj_str[obj_fmt].mode);
	if (objfp == NULL)
		fatal(F_FOPEN, objfn);
}

/*
 *	close object file, optionally delete it
 */
void obj_close_file(void)
{
	if (objfp != NULL)
		fclose(objfp);
}

/*
 *	write header record into object file
 */
void obj_header(const char *fn)
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
		if (fprintf(objfp, "// build from source file %s", fn) < 0 ||
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
 *	write end record into object file, optionally
 *	without filling up the file
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
 *	set execution start address of program
 */
void obj_start_addr(WORD addr)
{
	start_addr = addr;
}

/*
 *	set load address of program on first call
 */
void obj_load_addr(WORD addr)
{
	static int load_flag;

	if (!load_flag) {
		load_addr = addr;
		load_flag = TRUE;
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
void obj_writeb(const BYTE *ops, WORD op_cnt)
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
static void pos_fill_bin(void)
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
static void pos_fill_cary(void)
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
static void cary_byte(BYTE b)
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
static void eof_hex(WORD addr)
{
	hex_cnt = 0;
	hex_addr = addr;
	hex_record(HEX_EOF);
}

/*
 *	create a HEX data record in ASCII and write into object file
 */
static void flush_hex(void)
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
static void hex_record(BYTE rec_type)
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
static char *btoh(BYTE b, char *p)
{
	register char c;

	c = b >> 4;
	*p++ = c + (c < 10 ? '0' : 'A' - 10);
	c = b & 0xf;
	*p++ = c + (c < 10 ? '0' : 'A' - 10);
	return p;
}

/*
 *	compute checksum for Intel HEX record
 */
static BYTE chksum(BYTE rec_type)
{
	register int i;
	register BYTE sum;

	sum = hex_cnt;
	sum += hex_addr >> 8;
	sum += hex_addr & 0xff;
	sum += rec_type;
	for (i = 0; i < hex_cnt; i++)
		sum += hex_buf[i];
	return -sum;
}
