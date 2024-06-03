/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2024 by Thomas Eberhardt
 *
 * Simulation of an Intel Intellec iSBC 202 double density disk controller
 *
 * History:
 */

#include <stdint.h>
#include "sim.h"
#include "simglb.h"
#include "memsim.h"
#include "mds-isbc202.h"

				/* Result types */
#define RT_IOERR	0x00	/* I/O complete error bits */
#define RT_DSKRD	0x02	/* Diskette ready status */

				/* I/O complete error bits */
#define IO_DELR		0x01	/* Deleted record */
#define IO_CRC		0x02	/* CRC error */
#define IO_SEEK		0x04	/* Seek error */
#define IO_ADDR		0x08	/* Address error */
#define IO_DOURUN	0x10	/* Data overrun/underrun */
#define IO_WPROT	0x20	/* Write protect */
#define IO_WERR		0x40	/* Write error */
#define IO_NRDY		0x80	/* Not ready */

				/* Diskette ready status bits */
#define DR_UNIT2	0x10	/* Unit 2 ready */
#define DR_UNIT3	0x20	/* Unit 3 ready */
#define DR_UNIT0	0x40	/* Unit 0 ready */
#define DR_UNIT1	0x80	/* Unit 1 ready */

				/* Channel word definitions */
#define CW_DWLEN	0x08	/* Data word length (0 = 8, 1 = 16) */
#define CW_ICMSK	0x30	/* Interrupt control mask */
#define CW_IEN		0x00	/* Interrupt on completion/error */
#define CW_IDIS		0x10	/* All I/O interrupts disabled */
#define CW_RFS		0x40	/* Random format sequence */

				/* Diskette instruction */
#define DI_OPMSK	0x07	/* Op code mask */
#define DI_DWLEN	0x08	/* Data word length (0 = 8, 1 = 16) */
#define DI_USMSK	0x30	/* Unit select mask */
#define DI_UNIT0	0x00	/* Drive 0 */
#define DI_UNIT1	0x10	/* Drive 1 */

#define DI_UNIT3	0x30	/* Drive 3 */

				/* Diskette operations */
#define OP_NOP		0x00	/* No operation */
#define OP_SEEK		0x01	/* Seek */
#define OP_FORMAT	0x02	/* Format track */
#define OP_RECAL	0x03	/* Recalibrate */
#define OP_READ		0x04	/* Read data */
#define OP_VCRC		0x05	/* Verify CRC */
#define OP_WRITE	0x06	/* Write data */
#define OP_WRDEL	0x07	/* Write 'deleted' data */

/*
 * Parameter constraints:
 *
 * Number of records 0 - 52 and on same track
 * Track address 0 - 76
 * Sector address 1 - 52
 */

struct iopb {
	BYTE iocw;	/* Channel word */
	BYTE ioins;	/* Diskette instruction */
	BYTE nsec;	/* Number of records */
	BYTE taddr;	/* Track address */
	BYTE saddr;	/* Sector address */
	BYTE addrl;	/* Buffer address (Upper) */
	BYTE addrh;	/* Buffer address (Lower) */
};

BYTE isbc202_status_in(void)
{
	return (0x00);
}

BYTE isbc202_res_type_in(void)
{
	return (0x00);
}

BYTE isbc202_res_byte_in(void)
{
	return (0x00);
}

void isbc202_iopbl_out(BYTE data)
{
	UNUSED(data);
}

void isbc202_iopbh_out(BYTE data)
{
	UNUSED(data);
}

void isbc202_reset_out(BYTE data)
{
	UNUSED(data);
}
