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
 *	28-OCT-2017 added variable symbol length and other improvements
 *	15-MAY-2018 mark unreferenced symbols in listing
 *	30-JUL-2021 fix verbose option
 *	28-JAN-2022 added syntax check for OUT (n),A
 *	24-SEP-2022 added undocumented Z80 instructions and 8080 mode (TE)
 *	04-OCT-2022 new expression parser (TE)
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
#define COPYR		"Copyright (C) 1987-2022 by Udo Munk"
#define SRCEXT		".asm"	/* filename extension source */
#define OBJEXTBIN	".bin"	/* filename extension object */
#define OBJEXTHEX	".hex"	/* filename extension hex */
#define LSTEXT		".lis"	/* filename extension listing */
#define OUTBIN		1	/* format of object: binary */
#define OUTMOS		2	/*		     Mostek binary */
#define OUTHEX		3	/*		     Intel hex */
#define OUTDEF		OUTHEX	/* default object format */
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
#define IFNEST		5	/* max IF.. nesting depth */
#define HASHSIZE	500	/* max. entries in symbol hash array */
#define OPCARRAY	256	/* size of object buffer */
#define SYMINC		100	/* start size of sorted symbol array */
#define MAXHEX		32	/* max no bytes/hex record */

/*
 *	structure opcode table
 */
struct opc {
	char *op_name;		/* opcode name */
	int (*op_fun) (int, int); /* function pointer code generation */
	unsigned char op_c1;	/* first base opcode */
	unsigned char op_c2;	/* second base opcode */
	unsigned char op_type;	/* opcode type */
};

/*
 *	structure operand table
 */
struct ope {
	char *ope_name;		/* operand name */
	unsigned char ope_sym;	/* operand symbol value */
	unsigned char ope_type; /* operand type */
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
	int  sym_val;		/* symbol value */
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
 *	definition of opcode types
 */
#define OP_STD		0	/* nothing special */
#define OP_UNDOC	1	/* undocumented opcode */
#define OP_COND		2	/* concerns conditional assembly */
#define OP_SET		3	/* assigns value to label */
#define OP_END		4	/* end of source */

/*
 *	definition of operand symbols
 *	definitions for registers A, B, C, D, H, L and (HL)
 *	are defined as the bits used in opcodes and may not
 *	be changed!
 */
#define REGB		0	/* register B */
#define REGC		1	/* register C */
#define REGD		2	/* register D */
#define REGE		3	/* register E */
#define REGH		4	/* register H */
#define REGL		5	/* register L */
#define REGIHL		6	/* register indirect HL */
#define REGM		6	/* register indirect HL (8080) */
#define REGA		7	/* register A */
#define REGI		8	/* register I */
#define REGR		9	/* register R */
#define REGAF		10	/* register pair AF */
#define REGPSW		10	/* register pair AF (8080) */
#define REGBC		11	/* register pair BC */
#define REGDE		12	/* register pair DE */
#define REGHL		13	/* register pair HL */
#define REGIX		14	/* register IX */
#define REGIY		15	/* register IY */
#define REGSP		16	/* register SP */
#define REGIBC		17	/* register indirect BC */
#define REGIDE		18	/* register indirect DE */
#define REGIIX		19	/* register indirect IX */
#define REGIIY		20	/* register indirect IY */
#define REGISP		21	/* register indirect SP */
#define REGIXH		22	/* register IXH */
#define REGIXL		23	/* register IXL */
#define REGIYH		24	/* register IYH */
#define REGIYL		25	/* register IYL */
#define FLGNC		30	/* flag no carry */
#define FLGNZ		31	/* flag not zero */
#define FLGZ		32	/* flag zero */
#define FLGM		33	/* flag minus */
#define FLGP		34	/* flag plus */
#define FLGPE		35	/* flag parity even */
#define FLGPO		36	/* flag parity odd */
#define NOOPERA		98	/* no operand */
#define NOREG		99	/* operand isn't register */

/*
 *	definition of operand types
 */
#define OPE_STD		0	/* nothing special */
#define OPE_UNDOC	1	/* undocumented operand */

/*
 *	definition of operation sets
 */
#define OPSET_PSD	0	/* pseudo ops */
#define OPSET_Z80	1	/* Z80 opcodes */
#define OPSET_8080	2	/* 8080 opcodes */

/*
 *	definition of address/data output modes for pseudo ops
 */
#define AD_STD		0	/* address from <addr>, data from <ops> */
#define AD_ADDR		1	/* address from <ad_addr>, no data */
#define AD_NONE		2	/* no address, no data */
#define AD_SUPPR	3	/* suppress whole line */

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

/*
 *	definition of fatal errors
 */
#define F_OUTMEM	0	/* out of memory */
#define F_USAGE		1	/* usage: .... */
#define F_HALT		2	/* assembly halted */
#define F_FOPEN		3	/* can't open file */
#define F_INTERN	4	/* internal error */
#define F_HEXLEN	5	/* hex record length out of range */

/*
 *	macro for declaring unused function parameters
 */
#define UNUSED(x)	(void)(x)
