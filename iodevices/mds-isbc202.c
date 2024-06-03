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

				/* result types */
#define RT_IOERR	0x00	/* I/O complete error bits */
#define RT_DSKRD	0x02	/* diskette ready status */

				/* I/O complete error bits */
#define IO_DELR		0x01	/* deleted record */
#define IO_CRC		0x02	/* CRC error */
#define IO_SEEK		0x04	/* seek error */
#define IO_ADDR		0x08	/* address error */
#define IO_DOURUN	0x10	/* data overrun/underrun */
#define IO_WPROT	0x20	/* write protect */
#define IO_WERR		0x40	/* write error */
#define IO_NRDY		0x80	/* not ready */

				/* diskette ready status bits */
#define DR_UNIT2	0x10	/* unit 2 ready */
#define DR_UNIT3	0x20	/* unit 3 ready */
#define DR_UNIT0	0x40	/* unit 0 ready */
#define DR_UNIT1	0x80	/* unit 1 ready */

				/* channel word definitions */
#define CW_DWLEN	0x08	/* data word length (0 = 8, 1 = 16) */
#define CW_ICMSK	0x30	/* interrupt control mask */
#define CW_IEN		0x00	/* interrupt on completion/error */
#define CW_IDIS		0x10	/* all I/O interrupts disabled */
#define CW_RFS		0x40	/* random format sequence */

				/* diskette instruction */
#define DI_OPMSK	0x07	/* op code mask */
#define DI_DWLEN	0x08	/* data word length (0 = 8, 1 = 16) */
#define DI_USMSK	0x30	/* unit select mask */
#define DI_UNIT0	0x00	/* drive 0 */
#define DI_UNIT1	0x10	/* drive 1 */
#define DI_UNIT2	0x20	/* drive 2 */
#define DI_UNIT3	0x30	/* drive 3 */

				/* diskette operations */
#define OP_NOP		0x00	/* no operation */
#define OP_SEEK		0x01	/* seek */
#define OP_FORMAT	0x02	/* format track */
#define OP_RECAL	0x03	/* recalibrate */
#define OP_READ		0x04	/* read data */
#define OP_VCRC		0x05	/* verify CRC */
#define OP_WRITE	0x06	/* write data */
#define OP_WRDEL	0x07	/* write 'deleted' data */

/*
 * Parameter constraints:
 *
 * Number of records 0 - 52 and on same track
 * Track address 0 - 76
 * Sector address 1 - 52
 */

struct iopb {
	BYTE iocw;	/* channel word */
	BYTE ioins;	/* diskette instruction */
	BYTE nsec;	/* number of records */
	BYTE taddr;	/* track address */
	BYTE saddr;	/* sector address */
	BYTE addrl;	/* buffer address (upper) */
	BYTE addrh;	/* buffer address (lower) */
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

void isbc202_reset(void)
{
}
