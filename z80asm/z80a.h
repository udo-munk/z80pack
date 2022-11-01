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
 *	OS dependant definitions
 */
#define LENFN		2048	/* max. filename length */
#define READA		"r"	/* file open mode read ascii */
#define WRITEA		"w"	/* file open mode write ascii */
#define WRITEB		"wb"	/* file open mode write binary */
#define PATHSEP		'/'	/* directory separator in paths */

/*
 *	various constants
 */
#define REL		"1.11-dev"
#define COPYR		"Copyright (C) 1987-2022 by Udo Munk" \
			" & 2022 by Thomas Eberhardt"
#define SRCEXT		".asm"	/* filename extension source */
#define OBJEXTBIN	".bin"	/* filename extension object */
#define OBJEXTHEX	".hex"	/* filename extension hex */
#define LSTEXT		".lis"	/* filename extension listing */
#define COMMENT		';'	/* inline comment character */
#define LINCOM		'*'	/* comment line if in column 1 */
#define LABSEP		':'	/* label separator */
#define STRDEL		'\''	/* string delimiter */
#define STRDEL2		'"'	/* the other string delimiter */
#define MAXFN		512	/* max. no. source files */
#define MAXLINE		128	/* max. line length source */
#define PLENGTH		65	/* default lines/page in listing */
#define SYMLEN		8	/* default max. symbol length */
#define INCNEST		5	/* max. INCLUDE nesting depth */
#define IFNEST		20	/* max. IF.. nesting depth */
#define HASHSIZE	500	/* max. entries in symbol hash array */
#define OPCARRAY	256	/* size of object buffer */
#define SYMINC		100	/* start size of sorted symbol array */
#define MAXHEX		32	/* max. no bytes/hex record */

/*
 *	types for working with op-codes, addresses and expressions
 */
typedef unsigned char BYTE;
typedef unsigned short WORD;

/*
 *	structure opcode table
 */
struct opc {
	const char *op_name;	/* opcode name */
	int (*op_fun) (BYTE, BYTE); /* function pointer code gen. */
	BYTE op_c1;		/* first base opcode */
	BYTE op_c2;		/* second base opcode */
	WORD op_flags;		/* opcode flags */
};

/*
 *	structure operand table
 */
struct ope {
	const char *ope_name;	/* operand name */
	BYTE ope_sym;		/* operand symbol value */
	BYTE ope_flags;		/* operand flags */
};

/*
 *	structure operations set
 */
struct opset {
	int no_opcodes;		/* number of opcode entries */
	struct opc *opctab;	/* opcode table */
	int no_operands;	/* number of operand entries */
	struct ope *opetab;	/* operand table */
};

/*
 *	structure symbol table entries
 */
struct sym {
	char *sym_name;		/* symbol name */
	WORD sym_val;		/* symbol value */
	int  sym_refcnt;	/* symbol reference counter */
	struct sym *sym_next;	/* next entry */
};

/*
 *	structure nested INCLUDE's
 */
struct inc {
	unsigned inc_line;	/* line counter for listing */
	char *inc_fn;		/* filename */
	FILE *inc_fp;		/* file pointer */
};

/*
 *	definition of opcode flags
 */
#define OP_UNDOC	0x0001	/* undocumented opcode */
#define OP_COND		0x0002	/* concerns conditional assembly */
#define OP_SET		0x0004	/* assigns value to label */
#define OP_END		0x0008	/* end of source */
#define OP_NOPRE	0x0010	/* no preprocessing of operand */
#define OP_NOLBL	0x0020	/* label not allowed */
#define OP_NOOPR	0x0040	/* doesn't have an operand */
#define OP_INCL		0x0080	/* include (list before executing) */
#define OP_DS		0x0100	/* define space (does own obj_* calls) */
#define OP_MDEF		0x0200	/* macro definition start */
#define OP_MEND		0x0400	/* macro definition end */

/*
 *	definition of operand symbols
 *	these are defined as the bits used in opcodes and
 *	may not be changed!
 */
				/* usable with OPMASK0 and OPMASK3 */
#define REGB		0000	/* register B */
#define REGC		0011	/* register C */
#define REGD		0022	/* register D */
#define REGE		0033	/* register E */
#define REGH		0044	/* register H */
#define REGL		0055	/* register L */
#define REGIHL		0066	/* register indirect HL */
#define REGM		0066	/* register indirect HL (8080) */
#define REGA		0077	/* register A */
#define REGIXH		0244	/* register IXH */
#define REGIXL		0255	/* register IXL */
#define REGIYH		0344	/* register IYH */
#define REGIYL		0355	/* register IYL */
				/* usable with OPMASK3 */
#define REGBC		0001	/* register pair BC */
#define REGDE		0021	/* register pair DE */
#define REGHL		0041	/* register pair HL */
#define REGAF		0061	/* register pair AF */
#define REGPSW		0061	/* register pair AF (8080) */
#define REGSP		0062	/* register SP */
#define REGIBC		0003	/* register indirect BC */
#define REGIDE		0023	/* register indirect DE */
#define REGISP		0063	/* register indirect SP */
#define REGIX		0240	/* register IX */
#define REGIIX		0260	/* register indirect IX */
#define REGIY		0340	/* register IY */
#define REGIIY		0360	/* register indirect IY */
#define REGI		0004	/* register I */
#define REGR		0014	/* register R */
#define FLGNZ		0100	/* flag not zero */
#define FLGZ		0110	/* flag zero */
#define FLGNC		0120	/* flag no carry */
#define FLGC		0130	/* flag carry */
#define FLGPO		0140	/* flag parity odd */
#define FLGPE		0150	/* flag parity even */
#define FLGP		0160	/* flag plus */
#define FLGM		0170	/* flag minus */

#define NOOPERA		0376	/* no operand */
#define NOREG		0377	/* operand isn't register */

#define OPMASK0		0007	/* << 0 bit mask for the registers */
#define OPMASK3		0070	/* << 3 bit mask for the registers/flags */
#define XYMASK		0100	/* bit mask for IX/IY */

/*
 *	definition of operand flags
 */
#define OPE_UNDOC	0x01	/* undocumented operand */

/*
 *	definition of object file formats
 */
#define OBJ_BIN		0	/* binary file */
#define OBJ_MOS		1	/* Mostek binary file */
#define OBJ_HEX		2	/* Intel hex file */

/*
 *	definition of operation sets
 */
#define OPSET_PSD	0	/* pseudo ops */
#define OPSET_Z80	1	/* Z80 opcodes */
#define OPSET_8080	2	/* 8080 opcodes */

/*
 *	definition of address output modes for pseudo ops
 */
#define A_STD		0	/* address from <addr> */
#define A_EQU		1	/* address from <a_addr>, '=' */
#define A_SET		2	/* address from <a_addr>, '#' */
#define A_DS		3	/* address from <a_addr>, no data */
#define A_NONE		4	/* no address */

/*
 *	definition of macro list flag options
 */
#define	M_OPS		0	/* only list macro expansions producing ops */
#define M_ALL		1	/* list all macro expansions */
#define M_NONE		2	/* list no macro expansions */

/*
 *	definition of error numbers for error messages in listfile
 */
#define E_NOERR		0	/* no error (used by eval()) */
#define E_ILLOPC	1	/* illegal opcode */
#define E_ILLOPE	2	/* illegal operand */
#define E_MISOPE	3	/* missing operand */
#define E_MULSYM	4	/* multiple defined symbol */
#define E_UNDSYM	5	/* undefined symbol */
#define E_VALOUT	6	/* value out of bounds */
#define E_MISPAR	7	/* missing right parenthesis */
#define E_MISDEL	8	/* missing string delimiter */
#define E_NSQWRT	9	/* non-sequential code write (binary output) */
#define E_MISIFF	10	/* missing IF at ELSE or ENDIF */
#define E_IFNEST	11	/* too many IF's nested */
#define E_MISEIF	12	/* missing ENDIF */
#define E_INCNEST	13	/* too many INCLUDE's nested */
#define E_PHSNEST	14	/* .PHASE can't be nested */
#define E_ORGPHS	15	/* illegal ORG in .PHASE block */
#define E_MISPHS	16	/* missing .PHASE at .DEPHASE */
#define E_DIVBY0	17	/* division by zero */
#define E_INVEXP	18	/* invalid expression */
#define E_BFRORG	19	/* code before first ORG (binary output) */
#define E_ILLLBL	20	/* illegal label */
#define E_MISDPH	21	/* missing .DEPHASE */
#define E_NIMDEF	22	/* not in macro definition */
#define E_MISEMA	23	/* missing ENDM */
#define E_NIMEXP	24	/* not in macro expansion */
#define E_MACNEST	25	/* macro expansion nested too deep */
#define E_OUTLCL	26	/* too many local labels */

/*
 *	definition of fatal errors
 */
#define F_OUTMEM	0	/* out of memory */
#define F_USAGE		1	/* usage: .... */
#define F_HALT		2	/* assembly halted */
#define F_FOPEN		3	/* can't open file */
#define F_INTERN	4	/* internal error */
#define F_PAGLEN	5	/* page length out of range */
#define F_SYMLEN	6	/* symbol length out of range */
#define F_HEXLEN	7	/* hex record length out of range */

/*
 *	macro for declaring unused function parameters
 */
#define UNUSED(x)	(void)(x)
