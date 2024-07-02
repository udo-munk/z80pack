/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1989-2021 by Udo Munk
 * Parts Copyright (C) 2008 by Justin Clancy
 * 8080 disassembler Copyright (C) 2018 by Christophe Staiesse
 * Copyright (c) 2022-2024 by Thomas Eberhardt
 */

#ifndef SIMDIS_INC
#define SIMDIS_INC

#include "sim.h"
#include "simdefs.h"

#ifdef WANT_ICE

extern char Disass_Str[64];
extern char Opcode_Str[64];

extern int disass(WORD addr);

#endif

#endif /* !SIMDIS_INC */
