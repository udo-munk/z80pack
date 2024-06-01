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
#define ISBC202_RT_IOERR	0x00	/* I/O complete error bits */
#define ISBC202_RT_DSKRD	0x02	/* Diskette ready status */

					/* I/O complete error bits */
#define ISBC202_IO_DELR		0x01	/* Deleted record */
#define ISBC202_IO_CRC		0x02	/* CRC error */
#define ISBC202_IO_SEEK		0x04	/* Seek error */
#define ISBC202_IO_ADDR		0x08	/* Address error */
#define ISBC202_IO_DOURUN	0x10	/* Data overrun/underrun */
#define ISBC202_IO_WPROT	0x20	/* Write protect */
#define ISBC202_IO_WERR		0x40	/* Write error */
#define ISBC202_IO_NRDY		0x80	/* Not ready */

					/* Diskette ready status bits */
#define ISBC202_DR_UNIT2	0x10	/* Unit 2 ready */
#define ISBC202_DR_UNIT3	0x20	/* Unit 3 ready */
#define ISBC202_DR_UNIT0	0x40	/* Unit 0 ready */
#define ISBC202_DR_UNIT1	0x80	/* Unit 1 ready */

					/* Channel word definitions */
#define ISBC202_CW_DWLEN	0x08	/* Data word length (0 = 8, 1 = 16) */
#define ISBC202_CW_ICMSK	0x30	/* Interrupt control mask */
#define ISBC202_CW_IEN		0x00	/* Interrupt on completion/error */
#define ISBC202_CW_IDIS		0x10	/* All I/O interrupts disabled */
#define ISBC202_CW_RFS		0x40	/* Random format sequence */

					/* Diskette instruction */
#define ISBC202_I_OPMSK		0x07	/* Op code mask */
#define ISBC202_I_DWLEN		0x08	/* Data word length (0 = 8, 1 = 16) */
#define ISBC202_I_USMSK		0x30	/* Unit select mask */
#define ISBC202_I_UNIT0		0x00	/* Drive 0 */
#define ISBC202_I_UNIT1		0x10	/* Drive 1 */
#define ISBC202_I_UNIT2		0x20	/* Drive 2 */
#define ISBC202_I_UNIT3		0x30	/* Drive 3 */

					/* Diskette operations */
#define ISBC202_OP_NOP		0x00	/* No operation */
#define ISBC202_OP_SEEK		0x01	/* Seek */
#define ISBC202_OP_FORMAT	0x02	/* Format track */
#define ISBC202_OP_RECAL	0x03	/* Recalibrate */
#define ISBC202_OP_READ		0x04	/* Read data */
#define ISBC202_OP_VCRC		0x05	/* Verify CRC */
#define ISBC202_OP_WRITE	0x06	/* Write data */
#define ISBC202_OP_WRDEL	0x07	/* Write 'deleted' data */

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
