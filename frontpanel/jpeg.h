// jpeg.h

#ifndef _JPEG_DEFS
#define _JPEG_DEFS

#include "jpeglib.h"

EXTERN(unsigned char *) read_jpeg JPP((char *fname, int *width, int *height,
				       int *num_components));

#endif /* !_JPEG_DEFS */
