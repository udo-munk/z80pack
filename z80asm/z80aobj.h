/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2024 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80AOUT_INC
#define Z80AOUT_INC

#include "z80a.h"

/*
 *	definition of object file formats
 */
#define OBJ_BIN		0	/* binary file */
#define OBJ_MOS		1	/* Mostek binary file */
#define OBJ_HEX		2	/* Intel HEX file */
#define OBJ_CARY	3	/* C initialized array */

/*
 *	definition of address output modes for pseudo ops
 */
#define A_STD		0	/* address from <addr> */
#define A_EQU		1	/* address from <a_addr>, '=' */
#define A_SET		2	/* address from <a_addr>, '#' */
#define A_DS		3	/* address from <a_addr>, no data */
#define A_NONE		4	/* no address */

extern WORD a_addr;
extern int  a_mode;
extern WORD load_addr;
extern WORD start_addr;

extern void asmerr(int i);

extern void lst_line(char *l, WORD addr, WORD op_cnt, int expn_flag);
extern void lst_mac(int sort_mode);
extern void lst_sym(int sort_mode);

extern void obj_header(void);
extern void obj_end(void);
extern void obj_org(WORD addr);
extern void obj_writeb(WORD op_cnt);
extern void obj_fill(WORD count);
extern void obj_fill_value(WORD count, WORD value);

#endif /* !Z80AOUT_INC */
