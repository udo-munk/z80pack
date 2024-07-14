/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80APFUN_INC
#define Z80APFUN_INC

#include "z80asm.h"

#define COND_STATE_SIZE 3	/* size of cond processing state in int's */

extern int in_phase_section(void);

extern int in_true_section(void);
extern int in_cond_section(void);
extern void save_cond_state(int *p);
extern void restore_cond_state(int *p);

extern WORD op_instrset(int pass, BYTE op_code, BYTE dummy, char *operand,
			BYTE *ops);
extern WORD op_org(int pass, BYTE op_code, BYTE dummy, char *operand,
		   BYTE *ops);
extern WORD op_radix(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		     BYTE *ops);
extern WORD op_equ(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		   BYTE *ops);
extern WORD op_dl(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		  BYTE *ops);
extern WORD op_ds(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		  BYTE *ops);
extern WORD op_db(int pass, BYTE op_code, BYTE dummy, char *operand,
		  BYTE *ops);
extern WORD op_dw(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		  BYTE *ops);
extern WORD op_misc(int pass, BYTE op_code, BYTE dummy, char *operand,
		    BYTE *ops);
extern WORD op_cond(int pass, BYTE op_code, BYTE dummy, char *operand,
		    BYTE *ops);
extern WORD op_glob(int pass, BYTE op_code, BYTE dummy, char *operand,
		    BYTE *ops);
extern WORD op_end(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		   BYTE *ops);

#endif /* !Z80APFUN_INC */
