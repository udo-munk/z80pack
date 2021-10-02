#include "imsai-fif.h"

#ifdef HAS_DISKMANAGER

#ifndef DISKMAP
#define DISKMAP     "disk.map"
#endif

#define LAST_DISK   'D'
#define _MAX_DISK   (LAST_DISK - '@')

extern char *disks[];

#define DISKNAME(A) disks[A]

#endif