/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80ATAB_INC
#define Z80ATAB_INC

#include "z80asm.h"

/*
 *	structure type symbol table entries
 */
typedef struct sym {
	char *sym_name;		/* symbol name */
	WORD sym_val;		/* symbol value */
	int sym_refflg;		/* symbol reference flag */
	struct sym *sym_next;	/* next entry */
} sym_t;

extern sym_t *look_sym(const char *sym_name);
extern sym_t *get_sym(const char *sym_name);
extern WORD sym_lastval(void);

extern void new_sym(const char *sym_name, WORD sym_val);
extern void put_sym(const char *sym_name, WORD sym_val);
extern void put_label(const char *label, WORD addr, int pass);

extern int get_symmax(void);
extern sym_t *first_sym(int sort_mode);
extern sym_t *next_sym(void);

#endif /* !Z80ATAB_INC */
