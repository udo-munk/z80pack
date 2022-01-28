/*
 *	Z80 - Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
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
 */

/*
 *	this module contains all global variables other
 *	than CPU specific tables
 */

#include <stdio.h>
#include "z80a.h"

char *infiles[MAXFN],		/* source filenames */
     objfn[LENFN + 1],		/* object filename */
     lstfn[LENFN + 1],		/* listing filename */
     *srcfn,			/* filename of current processed source file */
     line[MAXLINE],		/* buffer for one line source */
     tmp[MAXLINE],		/* temporary buffer */
     label[MAXLINE],		/* buffer for label */
     opcode[MAXLINE],		/* buffer for opcode */
     operand[MAXLINE],		/* buffer for operand */
     ops[OPCARRAY],		/* buffer for generated object code */
     title[MAXLINE];		/* buffer for title of source */

int  list_flag,			/* flag for option -l */
     sym_flag,			/* flag for option -s */
     ver_flag,			/* flag for option -v */
     dump_flag,			/* flag for option -x */
     pc,			/* program counter */
     pass,			/* processed pass */
     iflevel,			/* IF nesting level */
     gencode = 1,		/* flag for conditional object code */
     errors,			/* error counter */
     errnum,			/* error number in pass 2 */
     sd_flag,			/* list flag for PSEUDO opcodes */
				/* = 0: address from <val>, data from <ops> */
				/* = 1: address from <sd_val>, data from <ops>*/
				/* = 2: no address, data from <ops> */
				/* = 3: address from <sd_val>, no data */
				/* = 4: suppress whole line */
     sd_val,			/* output value for PSEUDO opcodes */
     prg_adr,			/* start address of program */
     prg_flag,			/* flag for prg_adr valid */
     out_form = OUTDEF,		/* format of object file */
     symlen = SYMLEN,			/* max. symbol length */
     symsize;			/* size of symarray */

FILE *srcfp,			/* file pointer for current source */
     *objfp,			/* file pointer for object code */
     *lstfp,			/* file pointer for listing */
     *errfp;			/* file pointer for error output */

unsigned
      c_line,			/* current line no. in current source */
      s_line,			/* line no. counter for listing */
      p_line,			/* no. printed lines on page */
      ppl = PLENGTH,		/* page length */
      page;			/* no. of pages for listing */

struct sym
     *symtab[HASHSIZE],		/* symbol table */
     **symarray;		/* sorted symbol table */
