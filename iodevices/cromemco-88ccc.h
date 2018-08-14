/**
 * cromemco-88ccc.h
 * 
 * Emulation of the Cromemco 88 CCC - Cyclops Camera Controller
 *
 * Copyright (C) 2018 by David McNaughton
 * 
 * History:
 * 14-AUG-18    1.0     Initial Release
 */

extern void cromemco_88ccc_ctrl_a_out(BYTE);
extern void cromemco_88ccc_ctrl_b_out(BYTE);
extern void cromemco_88ccc_ctrl_c_out(BYTE);
extern BYTE cromemco_88ccc_ctrl_a_in(void);