/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 2019 by Mike Douglas
 *
 * This module of the simulator contains the I/O simulation
 * for a Mostek AID-80F or SYS-80FT
 *
 * History:
 * 15-SEP-2019 (Mike Douglas) created from iosim.c for the Altair
 * 28-SEP-2019 (Udo Munk) use logging
 * 08-OCT-2019 (Mike Douglas) added OUT 161 trap to simbdos.c for host file I/O
 * 27-MAY-2024 moved io_in & io_out to simcore
 */

#include "sim.h"
#include "simdefs.h"
#include "simio.h"

#include "mostek-cpu.h"
#include "mostek-fdc.h"
#include "simbdos.h"

#if 0
/* #define LOG_LOCAL_LEVEL LOG_DEBUG */
#include "log.h"
static const char *TAG = "I/O";
#endif

/*
 *	Forward declarations for I/O functions
 */
static BYTE io_no_card_in(void);
static void io_no_card_out(BYTE data);

/*
 *	This array contains function pointers for every
 *	input I/O port (0 - 255), to do the required I/O.
 */
BYTE (*const port_in[256])(void) = {
	[208] = io_no_card_in,		/* (d0) PIO1 Data A */
	[209] = io_no_card_in,		/* (d1) PIO1 Control A */
	[210] = io_no_card_in,		/* (d2) PIO1 Data B */
	[211] = io_no_card_in,		/* (d3) PIO1 Control B */
	[212] = io_no_card_in,		/* (d4) PIO2 Data A */
	[213] = io_no_card_in,		/* (d5) PIO2 Control A */
	[214] = io_no_card_in,		/* (d6) PIO2 Data B */
	[215] = io_no_card_in,		/* (d7) PIO2 Control B */
	[216] = io_no_card_in,		/* (d8) CTC 0 (baud rate) */
	[217] = io_no_card_in,		/* (d9) CTC 1 */
	[218] = io_no_card_in,		/* (da) CTC 2 */
	[219] = io_no_card_in,		/* (db) CTC 3 */
	[220] = sio_data_in,		/* (dc) SIO Data */
	[221] = sio_status_in,		/* (dd) SIO Status */
	[222] = sio_handshake_in,	/* (de) Sys Control (handshake lines in) */
	[223] = io_no_card_in,		/* (df) Debug Control */
	[226] = fdcBoard_stat_in,	/* (e2) FDC board status */
	[227] = fdcBoard_ctl_in,	/* (e3) FDC board control */
	[228] = fdc1771_stat_in,	/* (e4) WD1771 Command/Status */
	[229] = fdc1771_track_in,	/* (e5) WD1771 Track */
	[230] = fdc1771_sec_in,		/* (e6) WD1771 Sector */
	[231] = fdc1771_data_in		/* (e7) WD1771 Data */
};

/*
 *	This array contains function pointers for every
 *	output I/O port (0 - 255), to do the required I/O.
 */
void (*const port_out[256])(BYTE data) = {
	[161] = host_bdos_out,		/* host file I/O hook */
	[208] = io_no_card_out,		/* (d0) PIO1 Data A */
	[209] = io_no_card_out,		/* (d1) PIO1 Control A */
	[210] = io_no_card_out,		/* (d2) PIO1 Data B */
	[211] = io_no_card_out,		/* (d3) PIO1 Control B */
	[212] = io_no_card_out,		/* (d4) PIO2 Data A */
	[213] = io_no_card_out,		/* (d5) PIO2 Control A */
	[214] = io_no_card_out,		/* (d6) PIO2 Data B */
	[215] = io_no_card_out,		/* (d7) PIO2 Control B */
	[216] = io_no_card_out,		/* (d8) CTC 0 (baud rate) */
	[217] = io_no_card_out,		/* (d9) CTC 1 */
	[218] = io_no_card_out,		/* (da) CTC 2 */
	[219] = io_no_card_out,		/* (db) CTC 3 */
	[220] = sio_data_out,		/* (dc) SIO Data */
	[221] = sio_control_out,	/* (dd) SIO Control */
	[222] = sio_handshake_out,	/* (de) Sys Control (handshake lines out) */
	[223] = io_no_card_out,		/* (df) Debug Control */
	[226] = io_no_card_out,		/* (e2) FDC board status */
	[227] = fdcBoard_ctl_out,	/* (e3) FDC board control */
	[228] = fdc1771_cmd_out,	/* (e4) WD1771 Command/Status */
	[229] = fdc1771_track_out,	/* (e5) WD1771 Track */
	[230] = fdc1771_sec_out,	/* (e6) WD1771 Sector */
	[231] = fdc1771_data_out	/* (e7) WD1771 Data */
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 */
void init_io(void)
{
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 */
void exit_io(void)
{
}

/*
 *	No card I/O input trap function
 *	Used for input ports where I/O cards might be
 *	installed, but haven't.
 */
static BYTE io_no_card_in(void)
{
	return (BYTE) IO_DATA_UNUSED;
}

/*
 *	No card I/O output trap function
 *	Used for output ports where I/O cards might be
 *	installed, but haven't.
 */
static void io_no_card_out(BYTE data)
{
	UNUSED(data);
}
