#ifndef DISKS_INC
#define DISKS_INC

#include "cromemco-fdc.h"

#ifndef DISKMAP
#define DISKMAP     "disk.map"
#endif

#define LAST_DISK   'D'
#define _MAX_DISK   (LAST_DISK - '@')

extern Diskdef  disks[];

#define DISKNAME(A) disks[A].fn

#endif
