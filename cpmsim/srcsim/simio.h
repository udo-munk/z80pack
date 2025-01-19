/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2019 by Udo Munk
 */

#ifndef SIMIO_INC
#define SIMIO_INC

#include "sim.h"
#include "simdefs.h"

#define IO_DATA_UNUSED	0xff	/* data returned on unused ports */

/*
 *	Structure for the disk images
 */
typedef struct dskdef {
	const char *fn;			/* filename */
	int *fd;			/* file descriptor */
	unsigned int tracks;		/* number of tracks */
	unsigned int sectors;		/* number of sectors */
} dskdef_t;

extern dskdef_t disks[16];

extern in_func_t *const port_in[256];
extern out_func_t *const port_out[256];

extern void init_io(void);
extern void exit_io(void);

#endif /* !SIMIO_INC */
