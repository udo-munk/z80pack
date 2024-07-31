/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2024 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80AOBJ_INC
#define Z80AOBJ_INC

#include <stdio.h>

#include "z80asm.h"

/*
 *	definition of object file formats
 */
#define OBJ_BIN		0	/* binary file */
#define OBJ_MOS		1	/* Mostek binary file */
#define OBJ_HEX		2	/* Intel HEX file */
#define OBJ_CARY	3	/* C initialized array */

extern void obj_set_options(int fmt, int hexl, int caryl, int nofill);
extern const char *obj_file_ext(void);
extern void obj_open_file(const char *fn);
extern void obj_close_file(void);
extern void obj_header(const char *fn);
extern void obj_end(void);
extern void obj_start_addr(WORD addr);
extern void obj_load_addr(WORD addr);
extern void obj_org(WORD addr);
extern void obj_writeb(const BYTE *ops, WORD op_cnt);
extern void obj_fill(WORD count);
extern void obj_fill_value(WORD count, WORD value);

#endif /* !Z80AOBJ_INC */
