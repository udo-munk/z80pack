/*
 * cromemco-wdi.h
 *
 * Copyright (C) 2022 by David McNaughton
 *
 * Cromemco WDI-II Hard disk controller interface
 *
 * History:
 * 23-JUL-2022    1.0     Initial Release 
 *
 */

void wdi_init(void);

BYTE cromemco_wdi_pio0a_data_in(void);
BYTE cromemco_wdi_pio0b_data_in(void);
BYTE cromemco_wdi_pio0a_cmd_in(void);
BYTE cromemco_wdi_pio0b_cmd_in(void);
BYTE cromemco_wdi_pio1a_data_in(void);
BYTE cromemco_wdi_pio1b_data_in(void);
BYTE cromemco_wdi_pio1a_cmd_in(void);
BYTE cromemco_wdi_pio1b_cmd_in(void);
BYTE cromemco_wdi_dma0_in(void);
BYTE cromemco_wdi_dma1_in(void);
BYTE cromemco_wdi_dma2_in(void);
BYTE cromemco_wdi_dma3_in(void);
BYTE cromemco_wdi_ctc0_in(void);
BYTE cromemco_wdi_ctc1_in(void);
BYTE cromemco_wdi_ctc2_in(void);
BYTE cromemco_wdi_ctc3_in(void);

void cromemco_wdi_pio0a_data_out(BYTE);
void cromemco_wdi_pio0b_data_out(BYTE);
void cromemco_wdi_pio0a_cmd_out(BYTE);
void cromemco_wdi_pio0b_cmd_out(BYTE);
void cromemco_wdi_pio1a_data_out(BYTE);
void cromemco_wdi_pio1b_data_out(BYTE);
void cromemco_wdi_pio1a_cmd_out(BYTE);
void cromemco_wdi_pio1b_cmd_out(BYTE);
void cromemco_wdi_dma0_out(BYTE);
void cromemco_wdi_dma1_out(BYTE);
void cromemco_wdi_dma2_out(BYTE);
void cromemco_wdi_dma3_out(BYTE);
void cromemco_wdi_ctc0_out(BYTE);
void cromemco_wdi_ctc1_out(BYTE);
void cromemco_wdi_ctc2_out(BYTE);
void cromemco_wdi_ctc3_out(BYTE);
