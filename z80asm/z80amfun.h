/*
 *	Z80/8080-Macro-Assembler - Intel-like macro implementation
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80AMFUN_INC
#define Z80AMFUN_INC

#include "z80asm.h"
#include "z80aopc.h"

extern int mac_get_def_nest(void);
extern int mac_get_exp_nest(void);

extern int mac_get_symmax(void);
extern char *mac_first(int sort_mode, int *rp);
extern char *mac_next(int *rp);

extern void mac_start_pass(int pass);
extern void mac_end_pass(int pass);

extern void mac_add_line(opc_t *op, char *line);
extern char *mac_expand(char *line);
extern int mac_lookup(char *sym_name);
extern void mac_call(char *operand);

extern int mac_op_ifb(char *operand, int *false_sect_flagp);
extern int mac_op_ifidn(char *operand, int *false_sect_flagp);

extern WORD op_endm(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		    BYTE *ops);
extern WORD op_exitm(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		     BYTE *ops);
extern WORD op_irp(int pass, BYTE op_code, BYTE dummy, char *operand,
		   BYTE *ops);
extern WORD op_local(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		     BYTE *ops);
extern WORD op_macro(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		     BYTE *ops);
extern WORD op_rept(int pass, BYTE dummy1, BYTE dummy2, char *operand,
		    BYTE *ops);

#endif /* !Z80AMFUN_INC */
