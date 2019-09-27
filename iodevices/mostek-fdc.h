/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2019 Mike Douglas
 *
 * Emulation of the Mostek FLP-80 Floppy Disk Controller
 *		(a WD1771 based FDC)
 *
 * History:
 * 15-SEP-19 (Mike Douglas) created from tarbell_fdc.h
 */

extern BYTE fdcBoard_stat_in(void), fdcBoard_ctl_in(void);
extern BYTE fdc1771_stat_in(void), fdc1771_track_in(void);
extern BYTE fdc1771_sec_in(void), fdc1771_data_in(void);

extern void fdcBoard_ctl_out(BYTE);
extern void fdc1771_cmd_out(BYTE), fdc1771_track_out(BYTE);
extern void fdc1771_sec_out(BYTE), fdc1771_data_out(BYTE);

extern void fdc_reset(void);
