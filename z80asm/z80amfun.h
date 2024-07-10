/*
 *	Z80/8080-Macro-Assembler - Intel-like macro implementation
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80AMFUN_INC
#define Z80AMFUN_INC

#include "z80a.h"
#include "z80aopc.h"

extern char *mac_first(int sort_mode, int *rp);
extern char *mac_next(int *rp);

extern void mac_start_pass(void);
extern void mac_end_pass(void);

extern void mac_add_line(struct opc *op, char *line);
extern char *mac_expand(void);
extern int mac_lookup(char *opcode);
extern void mac_call(void);

extern WORD op_endm(BYTE dummy1, BYTE dummy2);
extern WORD op_exitm(BYTE dummy1, BYTE dummy2);
extern WORD op_mcond(BYTE op_code, BYTE dummy);
extern WORD op_irp(BYTE op_code, BYTE dummy);
extern WORD op_local(BYTE dummy1, BYTE dummy2);
extern WORD op_macro(BYTE dummy1, BYTE dummy2);
extern WORD op_rept(BYTE dummy1, BYTE dummy2);

#endif /* !Z80AMFUN_INC */
