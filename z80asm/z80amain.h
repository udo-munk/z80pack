/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80AMAIN_INC
#define Z80AMAIN_INC

#include "z80a.h"

extern void NORETURN fatal(int i, const char *arg);

extern void process_file(char *fn);

extern char *strsave(char *s);

extern char *next_arg(char *p, int *str_flag);

#endif /* !Z80AMAIN_INC */
