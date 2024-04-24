// jpeg 


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "cdjpeg.h"            /* Common decls for cjpeg/djpeg applications */

#define UNUSED(x) (void) (x)

unsigned char *pixels;
int xsize, ysize, ncomps;

/*
 * Since BMP stores scanlines bottom-to-top, we have to invert the image
 * from JPEG's top-to-bottom order.  To do this, we save the outgoing data
 * in a virtual array during put_pixel_row calls, then actually emit the
 * BMP file during finish_output.  The virtual array contains one JSAMPLE per
 * pixel if the output is grayscale or colormapped, three if it is full color.
 */

/* Private version of data destination object */

typedef struct {
  struct djpeg_dest_struct pub;	/* public fields */

  jvirt_sarray_ptr whole_image;	/* needed to reverse row order */
  JDIMENSION data_width;	/* JSAMPLEs per row */
  JDIMENSION row_width;		/* physical width of one row in the BMP file */
  JDIMENSION cur_output_row;	/* next row# to write to virtual array */
} bmp_dest_struct;

typedef bmp_dest_struct * bmp_dest_ptr;


/*
 * Write some pixel data.
 * In this module rows_supplied will always be 1.
 */

METHODDEF(void)
put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		JDIMENSION rows_supplied)
/* This version is for writing 24-bit pixels */
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  JSAMPARRAY image_ptr;
  JSAMPROW inptr, outptr;
  JDIMENSION col;
  /* int pad; */

  UNUSED(rows_supplied);

  /* Access next row in virtual array */
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->whole_image,
     dest->cur_output_row, (JDIMENSION) 1, TRUE);
  dest->cur_output_row++;

  /* Transfer data. 
   */
  inptr = dest->pub.buffer[0];
  outptr = image_ptr[0];
  for (col = cinfo->output_width; col > 0; col--) {
    outptr[0] = *inptr++;	/* can omit GETJSAMPLE() safely */
    outptr[1] = *inptr++;
    outptr[2] = *inptr++;
    outptr += 3;
  }
}

METHODDEF(void)
put_gray_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
	       JDIMENSION rows_supplied)
/* This version is for grayscale OR quantized color output */
{
//printf("put gray rows\n");
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  JSAMPARRAY image_ptr;
  JSAMPROW inptr, outptr;
  JDIMENSION col;
  /* int pad; */

  UNUSED(rows_supplied);

  /* Access next row in virtual array */
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->whole_image,
     dest->cur_output_row, (JDIMENSION) 1, TRUE);
  dest->cur_output_row++;

  /* Transfer data. */
  inptr = dest->pub.buffer[0];
  outptr = image_ptr[0];
  for (col = cinfo->output_width; col > 0; col--) {
    *outptr++ = *inptr++;	/* can omit GETJSAMPLE() safely */
  }

}


/*
 * Startup: normally writes the file header.
 * In this module we may as well postpone everything until finish_output.
 */

METHODDEF(void)
start_output_pixels (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  UNUSED(cinfo);
  UNUSED(dinfo);

//printf("start output\n");
  /* no work here */
}



METHODDEF(void)
finish_output_pixels (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  unsigned char *p;

  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;

  JSAMPARRAY image_ptr;
  JSAMPROW data_ptr;
  JDIMENSION row;
  JDIMENSION col;
  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;

    p = pixels;	// final output
//printf("finish output_pixels row_width=%d\n",dest->row_width);
  /* Write the file body from our virtual array */
  for (row = cinfo->output_height; row > 0; row--) {
    if (progress != NULL) {
      progress->pub.pass_counter = (long) (cinfo->output_height - row);
      progress->pub.pass_limit = (long) cinfo->output_height;
      (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
    }
    image_ptr = (*cinfo->mem->access_virt_sarray)
      ((j_common_ptr) cinfo, dest->whole_image, row-1, (JDIMENSION) 1, FALSE);
    data_ptr = image_ptr[0];
    for (col = dest->row_width; col > 0; col--) 
    {
//      putc(GETJSAMPLE(*data_ptr), outfile);
      *p++ = *data_ptr;
      data_ptr++;
    }
  }
  if (progress != NULL)
    progress->completed_extra_passes++;

}


/*
 * The module selection routine for BMP format output.
 */

GLOBAL(djpeg_dest_ptr)
jinit_write_pixels (j_decompress_ptr cinfo)
{
  bmp_dest_ptr dest;
  JDIMENSION row_width;

  /* Create module interface object, fill in method pointers */
  dest = (bmp_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(bmp_dest_struct));
  dest->pub.start_output = start_output_pixels;
  dest->pub.finish_output = finish_output_pixels;

  if (cinfo->out_color_space == JCS_GRAYSCALE) {
    dest->pub.put_pixel_rows = put_gray_rows;
  } else if (cinfo->out_color_space == JCS_RGB) {
    if (cinfo->quantize_colors)
      dest->pub.put_pixel_rows = put_gray_rows;
    else
      dest->pub.put_pixel_rows = put_pixel_rows;
  } else {
      ERREXIT(cinfo, JERR_BMP_COLORSPACE);
  }

  /* Calculate output image dimensions so we can allocate space */
  jpeg_calc_output_dimensions(cinfo);

  /* Determine width of rows in the BMP file (padded to 4-byte boundary). */
  row_width = cinfo->output_width * cinfo->output_components;
  dest->data_width = row_width;
  dest->row_width = row_width;

  xsize = cinfo->output_width;
  ysize = cinfo->output_height;
  ncomps = cinfo->output_components;

  //allocate space for final pixels
  pixels = new unsigned char[xsize*ysize*ncomps];

  /* Allocate space for inversion array, prepare for write pass */
  dest->whole_image = (*cinfo->mem->request_virt_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
     row_width, cinfo->output_height, (JDIMENSION) 1);
  dest->cur_output_row = 0;
  if (cinfo->progress != NULL) {
    cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
    progress->total_extra_passes++; /* count file input as separate pass */
  }

  /* Create decompressor output buffer. */
  dest->pub.buffer = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, row_width, (JDIMENSION) 1);
  dest->pub.buffer_height = 1;

  return (djpeg_dest_ptr) dest;
}

#define DBG 0

unsigned char 
*read_jpeg(char *fname, int *width, int *height, int *num_components)
{
 FILE *fd;

 struct jpeg_decompress_struct cinfo;
 struct jpeg_error_mgr jerr;
 djpeg_dest_ptr dest_mgr = NULL;

 pixels = NULL;

 int num_scanlines;

 if( (fd = fopen(fname,"rb")) == NULL)
 {
   fprintf(stderr,"read_jpeg: could not open file %s\n",fname);
   return NULL;
 }

#if DBG
fprintf(stderr,"read_jpeg: reading file %s\n",fname);
#endif

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  /* Specify data source for decompression */
  jpeg_stdio_src(&cinfo, fd);
   
  /* Read file header, set default decompression parameters */
  (void) jpeg_read_header(&cinfo, TRUE);

  dest_mgr = jinit_write_pixels(&cinfo);

 // print header information

 
    /* Start decompressor */
  (void) jpeg_start_decompress(&cinfo);
#if DBG
fprintf(stderr,"read_jpeg: started decompressor for  file %s\n",fname);
#endif

  /* Write output file header */
  (*dest_mgr->start_output) (&cinfo, dest_mgr);

   
#if 1
//  (*dest_mgr->start_output) (&cinfo, dest_mgr);

  /* Process data */
  while (cinfo.output_scanline < cinfo.output_height) {
    num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer, dest_mgr->buffer_height);
    (*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
  }
#endif

  (*dest_mgr->finish_output) (&cinfo, dest_mgr);
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);


 *width = xsize;
 *height = ysize;
 *num_components=ncomps;
 return pixels;
}


