/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80ANUM_INC
#define Z80ANUM_INC

#include "z80asm.h"

extern BYTE ctype[256];

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
 *	macros for character classification and conversion
 */
#define IS_FSYM(c)	(ctype[(BYTE) (c)] & C_FSYM)
#define IS_SYM(c)	(ctype[(BYTE) (c)] & C_SYM)
#define IS_DIG(c)	(ctype[(BYTE) (c)] & C_DIG)
#define IS_XDIG(c)	(ctype[(BYTE) (c)] & C_XDIG)
#define IS_SPC(c)	(ctype[(BYTE) (c)] & C_SPC)
/* don't use parameters with side-effects with this! */
#define TO_UPP(c)	((ctype[(BYTE) (c)] & C_LOW) ? ((c) - 32) : (c))

extern void init_ctype(void);

extern void set_radix(int r);
extern int get_radix(void);
extern WORD eval(char *s);
extern BYTE chk_byte(WORD w);
extern BYTE chk_sbyte(WORD w);

#endif /* !Z80ANUM_INC */
