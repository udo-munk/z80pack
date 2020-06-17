/**
 * cromemco-d+7a.h
 * 
 * Emulation of the Cromemco D+7A I/O 
 *
 * Copyright (C) 2020 by David McNaughton
 * 
 * History:
 * 14-JAN-20    1.0     Initial Release
 */

extern void cromemco_d7a_init(void);

extern void cromemco_d7a_D_out(BYTE);
extern void cromemco_d7a_A1_out(BYTE);
extern void cromemco_d7a_A2_out(BYTE);
extern void cromemco_d7a_A3_out(BYTE);
extern void cromemco_d7a_A4_out(BYTE);
extern void cromemco_d7a_A5_out(BYTE);
extern void cromemco_d7a_A6_out(BYTE);
extern void cromemco_d7a_A7_out(BYTE);

extern BYTE cromemco_d7a_D_in(void);
extern BYTE cromemco_d7a_A1_in(void);
extern BYTE cromemco_d7a_A2_in(void);
extern BYTE cromemco_d7a_A3_in(void);
extern BYTE cromemco_d7a_A4_in(void);
extern BYTE cromemco_d7a_A5_in(void);
extern BYTE cromemco_d7a_A6_in(void);
extern BYTE cromemco_d7a_A7_in(void);
