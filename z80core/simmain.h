/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 Udo Munk
 * Copyright (C) 2021 David McNaughton
 * Copyright (C) 2022-2024 Thomas Eberhardt
 */

#ifndef SIMMAIN_INC
#define SIMMAIN_INC

#include "sim.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FRONTPANEL
extern int sim_main(int argc, char *argv[]);
#endif

#ifdef __cplusplus
}
#endif

#endif /* !SIMMAIN_INC */
