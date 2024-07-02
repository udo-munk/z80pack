/**
 * cromemco-88ccc.h
 * 
 * Emulation of the Cromemco 88 CCC - Cyclops Camera Controller
 *
 * Copyright (C) 2018 by David McNaughton
 * 
 * History:
 * 14-AUG-2018	1.0	Initial Release
 * 04-NOV-2019		remove fake DMA bus request
 */

#ifndef CROMEMCO_88CCC_INC
#define CROMEMCO_88CCC_INC

#include "sim.h"
#include "simdefs.h"

extern void cromemco_88ccc_ctrl_a_out(BYTE data);
extern void cromemco_88ccc_ctrl_b_out(BYTE data);
extern void cromemco_88ccc_ctrl_c_out(BYTE data);
extern BYTE cromemco_88ccc_ctrl_a_in(void);

#endif /* !CROMEMCO_88CCC_INC */
