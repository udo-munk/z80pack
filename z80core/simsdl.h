/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2025 Thomas Eberhardt
 */

#ifndef SIMSDL_INC
#define SIMSDL_INC

#include <SDL.h>

#include "sim.h"
#include "simdefs.h"

typedef struct win_funcs {
	void (*open)(void);
	void (*close)(void);
	void (*event)(SDL_Event *e);
	void (*draw)(bool tick);
} win_funcs_t;

extern int simsdl_create(win_funcs_t *funcs);
extern void simsdl_destroy(int i);

#endif /* !SIMSDL_INC */
