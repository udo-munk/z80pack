/**
 * diskmanager.h
 *
 * Copyright (C) 2018 by David McNaughton
 *
 * History:
 * 12-JUL-2018	1.0	Initial Release
 */

#ifndef DISKMANAGER_INC
#define DISKMANAGER_INC

#include "sim.h"
#include "simdefs.h"

#ifdef HAS_NETSERVER
#include "netsrv.h"
#endif

extern void readDiskmap(char *path_name);

#ifdef HAS_NETSERVER
extern int LibraryHandler(HttpdConnection_t *conn, void *unused);
extern int DiskHandler(HttpdConnection_t *conn, void *unused);
#endif

#endif /* !DISKMANAGER_INC */
