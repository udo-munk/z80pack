/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80ASM_INC
#define Z80ASM_INC

/*
 *	OS dependent definitions
 */
#define READA		"r"	/* file open mode read ASCII */
#define WRITEA		"w"	/* file open mode write ASCII */
#define WRITEB		"wb"	/* file open mode write binary */
#define PATHSEP		'/'	/* directory separator in paths */

/*
 *	various constants
 */
#define COPYR		"Copyright (C) 1987-2024 by Udo Munk" \
			" & 2022-2024 by Thomas Eberhardt"
#define RELEASE		"2.1"

#define SRCEXT		".asm"	/* filename extension source */
#define OBJEXTBIN	".bin"	/* filename extension object */
#define OBJEXTHEX	".hex"	/* filename extension HEX */
#define OBJEXTCARY	".c"	/* filename extension C initialized array */
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
#define CARYLEN		12	/* default number of bytes per C array line */
#define CARYSPC		12	/* max number of bytes per C array line that
				   are followed by a space character */

/*
 *	definition of fatal errors
 */
#define F_OUTMEM	0	/* out of memory */
#define F_USAGE		1	/* usage: .... */
#define F_HALT		2	/* assembly halted */
#define F_FOPEN		3	/* can't open file */
#define F_OBJFILE	4	/* writing object file */
#define F_INTERN	5	/* internal error */
#define F_PAGLEN	6	/* page length out of range */
#define F_SYMLEN	7	/* symbol length out of range */
#define F_CARYLEN	8	/* C array bytes per line out of range */
#define F_HEXLEN	9	/* HEX record length out of range */

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
#define E_MISIFF	9	/* missing IF at ELSE or ENDIF */
#define E_IFNEST	10	/* IF nested too deep */
#define E_MISEIF	11	/* missing ENDIF */
#define E_INCNST	12	/* INCLUDE nested too deep */
#define E_PHSNST	13	/* invalid .PHASE nesting */
#define E_ORGPHS	14	/* invalid ORG in .PHASE section */
#define E_MISPHS	15	/* missing .PHASE at .DEPHASE */
#define E_DIVBY0	16	/* division by zero */
#define E_INVEXP	17	/* invalid expression */
#define E_INVLBL	18	/* invalid label */
#define E_MISDPH	19	/* missing .DEPHASE */
#define E_NIMDEF	20	/* not in macro definition */
#define E_MISEMA	21	/* missing ENDM */
#define E_NIMEXP	22	/* not in macro expansion */
#define E_MACNST	23	/* macro expansion nested too deep */
#define E_OUTLCL	24	/* too many local labels */
#define E_LBLDIF	25	/* label address differs between passes */
#define E_MACOVF	26	/* macro buffer overflow */

/*
 *	definition of macro list options
 */
#define	M_OPS		0	/* only list macro expansions producing ops */
#define M_ALL		1	/* list all macro expansions */
#define M_NONE		2	/* list no macro expansions */

/*
 *	definition of set_pc() options
 */
#define PC_ORG		0	/* set both program counters */
#define PC_PHASE	1	/* set logical program counter */
#define PC_DEPHASE	2	/* reset logical to real program counter */

#ifndef FALSE
#define FALSE		0
#endif
#ifndef TRUE
#define TRUE		1
#endif

/*
 *	macro for declaring unused function parameters
 */
#define UNUSED(x)	(void) (x)

/*
 *	attribute for declaring that a function doesn't return
 */
#if __GNUC__ > 2 || defined(__clang__)
#define NORETURN	__attribute__ ((noreturn))
#else
#define NORETURN
#endif

/*
 *	types for working with 8 and 16-bit quantities
 */
typedef unsigned char BYTE;
typedef unsigned short WORD;

extern void NORETURN fatal(int err, const char *arg);
extern void asmerr(int err);

extern char *strsave(const char *s);
extern char *next_arg(char *p, int *str_flag);

extern int undoc_allowed(void);
extern int get_symlen(void);
extern const char *get_label(void);
extern WORD get_pc(void);
extern void set_pc(int opt, WORD addr);
extern void set_list_active(int flag);
extern void set_mac_list_opt(int opt);
extern void set_nofalselist(int flag);

#endif /* !Z80ASM_INC */
