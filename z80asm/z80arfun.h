/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80ARFUN_INC
#define Z80ARFUN_INC

#include "z80asm.h"

extern WORD op_1b(int pass, BYTE b1, BYTE dummy, char *operand, BYTE *ops);
extern WORD op_2b(int pass, BYTE b1, BYTE b2, char *operand, BYTE *ops);
extern WORD op_im(int pass, BYTE base_op1, BYTE base_op2, char *operand,
		  BYTE *ops);
extern WORD op_pupo(int pass, BYTE base_op, BYTE dummy, char *operand,
		    BYTE *ops);
extern WORD op_ex(int pass, BYTE base_ops, BYTE base_opd, char *operand,
		  BYTE *ops);
extern WORD op_rst(int pass, BYTE base_op, BYTE dummy, char *operand,
		   BYTE *ops);
extern WORD op_ret(int pass, BYTE base_op, BYTE base_opc, char *operand,
		   BYTE *ops);
extern WORD op_jpcall(int pass, BYTE base_op, BYTE base_opc, char *operand,
		      BYTE *ops);
extern WORD op_jr(int pass, BYTE base_op, BYTE base_opc, char *operand,
		  BYTE *ops);
extern WORD op_djnz(int pass, BYTE base_op, BYTE dummy,char *operand,
		    BYTE *ops);
extern WORD op_ld(int pass, BYTE base_op, BYTE dummy, char *operand,
		  BYTE *ops);
extern WORD op_add(int pass, BYTE base_op, BYTE base_op16,char *operand,
		   BYTE *ops);
extern WORD op_sbadc(int pass, BYTE base_op, BYTE base_op16, char *operand,
		     BYTE *ops);
extern WORD op_decinc(int pass, BYTE base_op, BYTE base_op16, char *operand,
		      BYTE *ops);
extern WORD op_alu(int pass, BYTE base_op, BYTE dummy, char *operand,
		   BYTE *ops);
extern WORD op_out(int pass, BYTE op_base, BYTE op_basec, char *operand,
		   BYTE *ops);
extern WORD op_in(int pass, BYTE op_base, BYTE op_basec, char *operand,
		  BYTE *ops);
extern WORD op_cbgrp(int pass, BYTE base_op, BYTE dummy, char *operand,
		     BYTE *ops);

extern WORD op8080_mov(int pass, BYTE base_op, BYTE dummy, char *operand,
		       BYTE *ops);
extern WORD op8080_alu(int pass, BYTE base_op, BYTE dummy, char *operand,
		       BYTE *ops);
extern WORD op8080_dcrinr(int pass, BYTE base_op, BYTE dummy, char *operand,
			  BYTE *ops);
extern WORD op8080_reg16(int pass, BYTE base_op, BYTE dummy, char *operand,
			 BYTE *ops);
extern WORD op8080_regbd(int pass, BYTE base_op, BYTE dummy, char *operand,
			 BYTE *ops);
extern WORD op8080_imm(int pass, BYTE base_op, BYTE dummy, char *operand,
		       BYTE *ops);
extern WORD op8080_rst(int pass, BYTE base_op, BYTE dummy, char *operand,
		       BYTE *ops);
extern WORD op8080_pupo(int pass, BYTE base_op, BYTE dummy, char *operand,
			BYTE *ops);
extern WORD op8080_addr(int pass, BYTE base_op, BYTE dummy, char *operand,
			BYTE *ops);
extern WORD op8080_mvi(int pass, BYTE base_op, BYTE dummy, char *operand,
		       BYTE *ops);
extern WORD op8080_lxi(int pass, BYTE base_op, BYTE dummy, char *operand,
		       BYTE *ops);

#endif /* !Z80ARFUN_INC */
