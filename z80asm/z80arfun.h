/*
 *	Z80/8080-Macro-Assembler
 *	Copyright (C) 1987-2022 by Udo Munk
 *	Copyright (C) 2022-2024 by Thomas Eberhardt
 */

#ifndef Z80ARFUN_INC
#define Z80ARFUN_INC

#include "z80a.h"

extern WORD op_1b(BYTE b1, BYTE dummy);
extern WORD op_2b(BYTE b1, BYTE b2);
extern WORD op_im(BYTE base_op1, BYTE base_op2);
extern WORD op_pupo(BYTE base_op, BYTE dummy);
extern WORD op_ex(BYTE base_ops, BYTE base_opd);
extern WORD op_rst(BYTE base_op, BYTE dummy);
extern WORD op_ret(BYTE base_op, BYTE base_opc);
extern WORD op_jpcall(BYTE base_op, BYTE base_opc);
extern WORD op_jr(BYTE base_op, BYTE base_opc);
extern WORD op_djnz(BYTE base_op, BYTE dummy);
extern WORD op_ld(BYTE base_op, BYTE dummy);
extern WORD op_add(BYTE base_op, BYTE base_op16);
extern WORD op_sbadc(BYTE base_op, BYTE base_op16);
extern WORD op_decinc(BYTE base_op, BYTE base_op16);
extern WORD op_alu(BYTE base_op, BYTE dummy);
extern WORD op_out(BYTE op_base, BYTE op_basec);
extern WORD op_in(BYTE op_base, BYTE op_basec);
extern WORD op_cbgrp(BYTE base_op, BYTE dummy);

extern WORD op8080_mov(BYTE base_op, BYTE dummy);
extern WORD op8080_alu(BYTE base_op, BYTE dummy);
extern WORD op8080_dcrinr(BYTE base_op, BYTE dummy);
extern WORD op8080_reg16(BYTE base_op, BYTE dummy);
extern WORD op8080_regbd(BYTE base_op, BYTE dummy);
extern WORD op8080_imm(BYTE base_op, BYTE dummy);
extern WORD op8080_rst(BYTE base_op, BYTE dummy);
extern WORD op8080_pupo(BYTE base_op, BYTE dummy);
extern WORD op8080_addr(BYTE base_op, BYTE dummy);
extern WORD op8080_mvi(BYTE base_op, BYTE dummy);
extern WORD op8080_lxi(BYTE base_op, BYTE dummy);

#endif /* !Z80ARFUN_INC */
