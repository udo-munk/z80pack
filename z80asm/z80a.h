/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

/*
 *	OS dependant definitions
 */
#define READA		"r"	/* file open mode read ascii */
#define WRITEA		"w"	/* file open mode write ascii */
#define WRITEB		"wb"	/* file open mode write binary */
#define PATHSEP		'/'	/* directory separator in paths */

/*
 *	various constants
 */
#define COPYR		"Copyright (C) 1987-2024 by Udo Munk" \
			" & 2022-2024 by Thomas Eberhardt"
#define RELEASE		"2.0-dev"

#define SRCEXT		".asm"	/* filename extension source */
#define OBJEXTBIN	".bin"	/* filename extension object */
#define OBJEXTHEX	".hex"	/* filename extension HEX */
#define LSTEXT		".lis"	/* filename extension listing */
#define COMMENT		';'	/* inline comment character */
#define LINCOM		'*'	/* comment line if in column 1 */
#define LINOPT		'$'	/* option line if in column 1 */
#define LABSEP		':'	/* label separator */
#define STRDEL		'\''	/* string delimiter */
#define STRDEL2		'"'	/* the other string delimiter */
#define MAXLINE		128	/* max. line length source */
#define PLENGTH		65	/* default lines/page in listing */
#define SYMLEN		8	/* default max. symbol length */
#define INCNEST		10	/* max. INCLUDE nesting depth */
#define HASHSIZE	503	/* max. entries in symbol hash array */
#define OPCARRAY	128	/* size of object buffer */
#define MAXHEX		32	/* max. no bytes per HEX record */
#define MACNEST		50	/* max. expansion nesting */

#ifndef FALSE
#define FALSE		0
#endif
#ifndef TRUE
#define TRUE		1
#endif

/*
 *	types for working with op-codes, addresses, expressions,
 *	and bit flags and masks
 */
typedef unsigned char BYTE;
typedef unsigned short WORD;

/*
 *	structure opcode table
 */
struct opc {
	const char *op_name;	/* opcode name */
	WORD (*op_fun) (BYTE, BYTE); /* function pointer code gen. */
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
 *	structure symbol table entries
 */
struct sym {
	char *sym_name;		/* symbol name */
	WORD sym_val;		/* symbol value */
	int sym_refflg;		/* symbol reference flag */
	struct sym *sym_next;	/* next entry */
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
#define REGAFA		0062	/* register pair AF' */
#define REGSP		0063	/* register SP */
#define REGIBC		0004	/* register indirect BC */
#define REGIDE		0024	/* register indirect DE */
#define REGISP		0064	/* register indirect SP */
#define REGIX		0240	/* register IX */
#define REGIIX		0260	/* register indirect IX */
#define REGIY		0340	/* register IY */
#define REGIIY		0360	/* register indirect IY */
#define REGI		0005	/* register I */
#define REGR		0015	/* register R */
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
#define OBJ_HEX		2	/* Intel HEX file */

/*
 *	definition of symbol table listing options
 */
#define SYM_NONE	0	/* no symbol table */
#define SYM_UNSORT	1	/* unsorted symbol table */
#define SYM_SORTN	2	/* symbol table sorted by name */
#define SYM_SORTA	3	/* symbol table sorted by address */

/*
 *	definition of instruction sets
 */
#define INSTR_NONE	0	/* not yet initialized */
#define INSTR_Z80 	1	/* Z80 instructions set */
#define INSTR_8080	2	/* 8080 instructions set */

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
 *	definition of character types
 */
#define C_FSYM		0x01	/* first symbol character */
#define C_SYM		0x02	/* symbol character */
#define C_LOW		0x04	/* lower case letter */
#define C_DIG		0x08	/* decimal digit */
#define C_XDIG		0x10	/* hexadecimal digit */
#define C_SPC		0x20	/* white space */

/*
 *	definition of error numbers for error messages in listfile
 *	E_OK and E_UNDSYM have special meaning for eval()
 *	and must not be changed!
 */
#define E_OK		0	/* no error */
#define E_UNDSYM	1	/* undefined symbol */
#define E_INVOPC	2	/* invalid opcode */
#define E_INVOPE	3	/* invalid operand */
#define E_MISOPE	4	/* missing operand */
#define E_MULSYM	5	/* multiple defined symbol */
#define E_VALOUT	6	/* value out of range */
#define E_MISPAR	7	/* missing right parenthesis */
#define E_MISDEL	8	/* missing string delimiter */
#define E_NSQWRT	9	/* non-sequential object code (binary) */
#define E_MISIFF	10	/* missing IF at ELSE or ENDIF */
#define E_IFNEST	11	/* IF nested too deep */
#define E_MISEIF	12	/* missing ENDIF */
#define E_INCNST	13	/* INCLUDE nested too deep */
#define E_PHSNST	14	/* invalid .PHASE nesting */
#define E_ORGPHS	15	/* invalid ORG in .PHASE block */
#define E_MISPHS	16	/* missing .PHASE at .DEPHASE */
#define E_DIVBY0	17	/* division by zero */
#define E_INVEXP	18	/* invalid expression */
#define E_BFRORG	19	/* object code before ORG (binary) */
#define E_INVLBL	20	/* invalid label */
#define E_MISDPH	21	/* missing .DEPHASE */
#define E_NIMDEF	22	/* not in macro definition */
#define E_MISEMA	23	/* missing ENDM */
#define E_NIMEXP	24	/* not in macro expansion */
#define E_MACNST	25	/* macro expansion nested too deep */
#define E_OUTLCL	26	/* too many local labels */
#define E_LBLDIF	27	/* label address differs between passes */
#define E_MACOVF	28	/* macro buffer overflow */

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
#define F_HEXLEN	7	/* HEX record length out of range */

/*
 *	macro for declaring unused function parameters
 */
#define UNUSED(x)	(void)(x)

/*
 *	macros for character classification and conversion
 */
#define IS_FSYM(c)	(ctype[(BYTE) (c)] & C_FSYM)
#define IS_SYM(c)	(ctype[(BYTE) (c)] & C_SYM)
#define IS_DIG(c)	(ctype[(BYTE) (c)] & C_DIG)
#define IS_XDIG(c)	(ctype[(BYTE) (c)] & C_XDIG)
#define IS_SPC(c)	(ctype[(BYTE) (c)] & C_SPC)
/* don't use parameters with side-effects with this! */
#define TO_UPP(c)	((ctype[(BYTE) (c)] & C_LOW) ? ((c) - 32) : (c))
