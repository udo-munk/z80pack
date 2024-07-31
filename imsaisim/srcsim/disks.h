#ifndef DISKS_INC
#define DISKS_INC

#include "imsai-fif.h"

#ifndef DISKMAP
#define DISKMAP     "disk.map"
#endif

#define LAST_DISK   'D'
#define _MAX_DISK   (LAST_DISK - '@')

#define DISKNAME(A) disks[A]

#endif
