/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80ATAB_INC
#define Z80ATAB_INC

#include "z80asm.h"

/*
 *	structure symbol table entries
 */
struct sym {
	char *sym_name;		/* symbol name */
	WORD sym_val;		/* symbol value */
	int sym_refflg;		/* symbol reference flag */
	struct sym *sym_next;	/* next entry */
};

extern struct sym *look_sym(const char *sym_name);
extern struct sym *get_sym(const char *sym_name);
extern WORD sym_lastval(void);

extern void new_sym(const char *sym_name, WORD sym_val);
extern void put_sym(const char *sym_name, WORD sym_val);
extern void put_label(const char *label, WORD addr, int pass);

extern int get_symmax(void);
extern struct sym *first_sym(int sort_mode);
extern struct sym *next_sym(void);

#endif /* !Z80ATAB_INC */
