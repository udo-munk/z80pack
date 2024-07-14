/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80AOPC_INC
#define Z80AOPC_INC

#include "z80asm.h"

/*
 *	definition of instruction sets
 */
#define INSTR_NONE	0	/* not yet initialized */
#define INSTR_Z80 	1	/* Z80 instructions set */
#define INSTR_8080	2	/* 8080 instructions set */

/*
 *	definition of opcode flags
 */
#define OP_UNDOC	0x0001	/* undocumented opcode */
#define OP_COND		0x0002	/* concerns conditional assembly */
#define OP_SET		0x0004	/* assigns value to label */
#define OP_END		0x0008	/* end of source */
#define OP_NOPRE	0x0010	/* no preprocessing of operand */
#define OP_NOLBL	0x0020	/* label not allowed */
#define OP_NOOPR	0x0040	/* doesn't have an operand */
#define OP_DS		0x0080	/* define space (does own obj_* calls) */
#define OP_MDEF		0x0100	/* macro definition start */
#define OP_MEND		0x0200	/* macro definition end */

/*
 *	definition of operand symbols
 *	these are defined as the bits used in opcodes and
 *	may not be changed!
 */
				/* usable with OPMASK0 and OPMASK3 */
#define REGB		0000	/* register B */
#define REGC		0011	/* register C */
#define REGD		0022	/* register D */
#define REGE		0033	/* register E */
#define REGH		0044	/* register H */
#define REGL		0055	/* register L */
#define REGIHL		0066	/* register indirect HL */
#define REGM		0066	/* register indirect HL (8080) */
#define REGA		0077	/* register A */
#define REGIXH		0244	/* register IXH */
#define REGIXL		0255	/* register IXL */
#define REGIYH		0344	/* register IYH */
#define REGIYL		0355	/* register IYL */
				/* usable with OPMASK3 */
#define REGBC		0001	/* register pair BC */
#define REGDE		0021	/* register pair DE */
#define REGHL		0041	/* register pair HL */
#define REGAF		0061	/* register pair AF */
#define REGPSW		0061	/* register pair AF (8080) */
#define REGAFA		0062	/* register pair AF' */
#define REGSP		0063	/* register SP */
#define REGIBC		0004	/* register indirect BC */
#define REGIDE		0024	/* register indirect DE */
#define REGISP		0064	/* register indirect SP */
#define REGIX		0240	/* register IX */
#define REGIIX		0260	/* register indirect IX */
#define REGIY		0340	/* register IY */
#define REGIIY		0360	/* register indirect IY */
#define REGI		0005	/* register I */
#define REGR		0015	/* register R */
#define FLGNZ		0100	/* flag not zero */
#define FLGZ		0110	/* flag zero */
#define FLGNC		0120	/* flag no carry */
#define FLGC		0130	/* flag carry */
#define FLGPO		0140	/* flag parity odd */
#define FLGPE		0150	/* flag parity even */
#define FLGP		0160	/* flag plus */
#define FLGM		0170	/* flag minus */

#define NOOPERA		0376	/* no operand */
#define NOREG		0377	/* operand isn't register */

#define OPMASK0		0007	/* << 0 bit mask for the registers */
#define OPMASK3		0070	/* << 3 bit mask for the registers/flags */
#define XYMASK		0100	/* bit mask for IX/IY */

/*
 *	definition of operand flags
 */
#define OPE_UNDOC	0x01	/* undocumented operand */

/*
 *	structure opcode table
 */
struct opc {
	const char *op_name;	/* opcode name */
	WORD (*op_func)(int pass, BYTE b1, BYTE b2,
			char *operand, BYTE *ops); /* opcode function */
	BYTE op_c1;		/* first base opcode */
	BYTE op_c2;		/* second base opcode */
	BYTE op_a_mode;		/* listing address mode */
	WORD op_flags;		/* opcode flags */
};

extern void instrset(int is);
extern struct opc *search_op(char *op_name);
extern BYTE get_reg(char *s);

#endif /* !Z80AOPC_INC */
