/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80APFUN_INC
#define Z80APFUN_INC

#include "z80a.h"

extern WORD op_instrset(BYTE op_code, BYTE dummy);
extern WORD op_org(BYTE op_code, BYTE dummy);
extern WORD op_radix(BYTE dummy1, BYTE dummy2);
extern WORD op_equ(BYTE dummy1, BYTE dummy2);
extern WORD op_dl(BYTE dummy1, BYTE dummy2);
extern WORD op_ds(BYTE dummy1, BYTE dummy2);
extern WORD op_db(BYTE op_code, BYTE dummy);
extern WORD op_dw(BYTE dummy1, BYTE dummy2);
extern WORD op_misc(BYTE op_code, BYTE dummy);
extern WORD op_cond(BYTE op_code, BYTE dummy);
extern WORD op_glob(BYTE op_code, BYTE dummy);
extern WORD op_end(BYTE dummy1, BYTE dummy2);

#endif /* !Z80APFUN_INC */
