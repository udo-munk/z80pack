/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

/*
 *	Alternative CPU register file.
 *	Allows fast access to full 16-bit, and high and low bytes
 */

#ifndef ALTREGS_INC
#define ALTREGS_INC

#if _BYTE_ORDER == _LITTLE_ENDIAN
typedef struct cpu_reg {
	union {
		struct { BYTE l; BYTE h; };
		WORD w;
	};
} cpu_reg_t;
#elif _BYTE_ORDER == _BIG_ENDIAN
typedef struct cpu_reg {
	union {
		struct { BYTE h; BYTE l; };
		WORD w;
	};
} cpu_reg_t;
#else
#error "Unsupported byte order"
#endif

typedef struct cpu_regs {
	cpu_reg_t af;	/* primary registers */
	cpu_reg_t bc;
	cpu_reg_t de;
	cpu_reg_t hl;
	cpu_reg_t sp;	/* stack pointer */
	cpu_reg_t pc;	/* program counter */
#ifndef EXCLUDE_Z80
	cpu_reg_t af_;	/* Z80 alternate registers */
	cpu_reg_t bc_;
	cpu_reg_t de_;
	cpu_reg_t hl_;
	cpu_reg_t ix;	/* Z80 index registers */
	cpu_reg_t iy;
	BYTE i;			/* Z80 interrupt register */
	BYTE r;			/* Z80 refresh register (7-bit counter) */
	BYTE r_;		/* 8th bit of R (can be loaded with LD R,A) */
#endif
	BYTE iff;		/* interrupt flags */
} cpu_regs_t;

extern cpu_regs_t cpu_regs;

#define AF	cpu_regs.af.w
#define A	cpu_regs.af.h
#define F	cpu_regs.af.l
#define BC	cpu_regs.bc.w
#define B	cpu_regs.bc.h
#define C	cpu_regs.bc.l
#define DE	cpu_regs.de.w
#define D	cpu_regs.de.h
#define E	cpu_regs.de.l
#define HL	cpu_regs.hl.w
#define H	cpu_regs.hl.h
#define L	cpu_regs.hl.l
#define SP	cpu_regs.sp.w
#define SPH	cpu_regs.sp.h
#define SPL	cpu_regs.sp.l
#define PC	cpu_regs.pc.w
#define PCH	cpu_regs.pc.h
#define PCL	cpu_regs.pc.l
#ifndef EXCLUDE_Z80
#define AF_	cpu_regs.af_.w
#define A_	cpu_regs.af_.h
#define F_	cpu_regs.af_.l
#define BC_	cpu_regs.bc_.w
#define B_	cpu_regs.bc_.h
#define C_	cpu_regs.bc_.l
#define DE_	cpu_regs.de_.w
#define D_	cpu_regs.de_.h
#define E_	cpu_regs.de_.l
#define HL_	cpu_regs.hl_.w
#define H_	cpu_regs.hl_.h
#define L_	cpu_regs.hl_.l
#define IX	cpu_regs.ix.w
#define IXH	cpu_regs.ix.h
#define IXL	cpu_regs.ix.l
#define IY	cpu_regs.iy.w
#define IYH	cpu_regs.iy.h
#define IYL	cpu_regs.iy.l
#define I	cpu_regs.i
#define R	cpu_regs.r
#define R_	cpu_regs.r_
#endif
#define IFF	cpu_regs.iff

#endif /* ALTREGS_INC */
