/*____________________________________________________________________
|
| File: pcx.cpp
|
| Description: Functions for reading/writing PCX files.
|
| Functions:    gxReadPCXFile
|                Draw_File
|                 Image_Size
|                 Unpack_PCX_File4
|                  Convert_Scanline_16to256
|                 Unpack_PCX_File8
|                 Unpack_PCX_File24
|                  Unpack_PCX_Line
|                   Get_PCX_Data
|                  Adjust_Scanline_Pixel_Format
|               gxWritePCXFile
|                Pack_PCX_File
|                 Pack_PCX_Line
|               gxGetPCXFileDimensions
|               gxConvertPCXFile
|                Unpack_PCX24_File
|                Unpack_PCX8_File
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include "dp.h"

/*___________________
|
| Type definitions
|__________________*/

typedef struct {
  byte  manufacturer;   /* always 0xA           */
  byte  version;        /* version number       */
  byte  encoding;       /* always 1             */
  byte  bits_per_pixel; /* color bits           */
  short xmin, ymin;     /* image origin         */
  short xmax, ymax;     /* image dimensions     */
  short hres;           /* resolutoin values    */
  short vres;
  byte  palette[48];    /* 16-color palette     */
  byte  reserved;
  byte  color_planes;   /* color planes         */
  short bytes_per_line; /* line buffer size     */
  short palette_type;   /* 1=color, 2=gray palette */
  byte  filler[58];     /* dead space           */
} PCXHeader;

/*___________________
|
| Function prototypes
|__________________*/

static int Draw_File (FILE *fp, int set_palette);
static unsigned Image_Size (gxBound box, int num_bits);

static void Unpack_PCX_File8 (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o
  unsigned  buffer_size,        // size of buffer
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       pic_dx,             // dimensions of pcx image
  int       pic_dy );
static void Unpack_PCX_File4 (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       pic_dx,             // dimensions of pcx image      
  int       pic_dy );
static void Convert_Scanline_16to256 (
  byte     *image16,            // src buffer
  byte     *image256,           // dst buffer
  int       num_bytes,          // # src bytes (per plane) to process
  int       ignore_last );      // # bits in last byte read to skip
static void Unpack_PCX_File24 (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o
  unsigned  buffer_size,        // size of buffer
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       pic_dx,             // dimensions of pcx image
  int       pic_dy );
static void Unpack_PCX_Line (
  FILE     *fp,
  byte     *buffer,
  unsigned  buffer_size,
  byte     *image_data,
  int       num_bytes,
  int      *count,
  int      *next );
static int Get_PCX_Data (FILE *fp, byte *buffer, unsigned buffer_size);
static int Adjust_Scanline_Pixel_Format (
  byte *src_image,            // single scanline image
  byte *dst_image,            // single scanline image
  byte *palette,              // image palette, if image is in 8-bit color format
  int   src_bytes_per_pixel,  // # bytes per pixel in src image
  int   dst_bytes_per_pixel,  // # bytes per pixel in dst image 
  int   num_pixels );         // # pixels represented in image

static void Pack_PCX_File (
  FILE     *fp,                 // pcx file
  byte     *src_image,          // 1 scanline image
  byte     *dst_image,
  int       pic_dy );
static void Pack_PCX_Line (
  FILE     *fp,                 // pcx file
  byte     *image_data,         // 1 scanline image
  int       width );

static void Unpack_PCX24_File (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o
  unsigned  buffer_size,        // size of buffer
  byte     *image,              // 1 scanline image
  int       pic_dx,             // dimensions of pcx image
  int       pic_dy,
  int       palette_spread,
  byte     *uniform_palette );
static void Unpack_PCX8_File (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o
  unsigned  buffer_size,        // size of buffer
  byte     *image,              // 1 scanline image
  int       pic_dx,             // dimensions of pcx image
  int       pic_dy,
  int       palette_spread,
  byte     *uniform_palette,
  byte     *file_palette );     // palette read from file

/*____________________________________________________________________
|
| Function: gxReadPCXFile
|
| Outputs: Draws a 4,8 or 24-bit PCX file on the active page.  Returns
|       true, if successful, else false on any error.  Will optionally,
|       set a new color palette using palette info from PCX file.
|___________________________________________________________________*/

int gxReadPCXFile (char *filename, int set_palette)
{
  FILE *fp;
  int   ok = TRUE;

  fp = fopen (filename, "rb");
  if (fp == NULL) {
    ok = FALSE;
    gxError ("Can't open PCX file.");
  }
  else {
    ok = Draw_File (fp, set_palette);
    fclose (fp);
  }

  return (ok);
}

/*____________________________________________________________________
|
| Function: Draw_File
|
| Outputs: Draws a PCX file on the active page.  Returns true
|       if successful, else false on any error.  Will optionally, set
|       a new color palette using palette info from PCX file.
|___________________________________________________________________*/

static int Draw_File (FILE *fp, int set_palette)
{
  int         i, pic_dx, pic_dy, bytes_read, bytes_per_line, num_bits;
  byte       *buffer = NULL;  // file i/o buffer              
  unsigned    buffer_size;    // size of i/o buffer           
  byte       *palette = NULL; // palette read from file   
  PCXHeader  *pcxhdr;
  byte       *src_image = NULL;
  byte       *dst_image = NULL;
  gxBound     box;
  unsigned    size, *p;
  gxRectangle rect;
  gxState     state;
  unsigned    kilobyte = 1024;
  int         ok = TRUE;

/*____________________________________________________________________
|
| Enable full page operation
|___________________________________________________________________*/

  gxSaveState (&state);
  rect.xleft   = 0;
  rect.ytop    = 0;
  rect.xright  = PAGE_WIDTH-1;
  rect.ybottom = PAGE_HEIGHT-1;
  gxSetWindow (&rect);
  gxSetClip   (&rect);
  gxSetClipping (TRUE);

/*____________________________________________________________________
|
| Allocate up to 100K temporary memory space for file i/o.
|___________________________________________________________________*/

  for (i=100; i>0; i-=20)
    if ((buffer = (byte *) malloc ((unsigned)i*kilobyte)) != NULL) {
      buffer_size = (unsigned)i * kilobyte;
      break;
    }
  if (i == 0) {
    gxError ("Not enough memory to display a PCX file.");
    ok = FALSE;
  }

/*____________________________________________________________________
|
| Read PCX header from file and extract info.  Set palette if needed.
| Finally, call a routine to unpack the pcx file.
|___________________________________________________________________*/

  if (ok) {
    pcxhdr = (PCXHeader *)buffer;
    bytes_read = (int)fread ((void *)pcxhdr, 1, sizeof(PCXHeader), fp);
    if (bytes_read != sizeof(PCXHeader)) {
      ok = FALSE;
      gxError ("Couldn't read PCX header.");
    }
    else {
      /* Is this a 4-bit (16-color) PCX file? */
      if ((pcxhdr->manufacturer   == 0xA) AND
          (pcxhdr->encoding       == 1)   AND
          (pcxhdr->bits_per_pixel == 1)   AND
          (pcxhdr->color_planes   == 4)   AND
          (pcxhdr->bytes_per_line == 80))  
        num_bits = 4;
      /* Is this a 8-bit (256-color) PCX file? */
      else if ((pcxhdr->manufacturer   == 0xA) AND
               (pcxhdr->encoding       == 1)   AND
               (pcxhdr->bits_per_pixel == 8)   AND
               (pcxhdr->color_planes   == 1))
        num_bits = 8;
      /* Is this a 24-bit (16M-color) PCX file? */
      else if ((pcxhdr->manufacturer   == 0xA) AND
               (pcxhdr->encoding       == 1)   AND
               (pcxhdr->bits_per_pixel == 8)   AND
               (pcxhdr->color_planes   == 3))
        num_bits = 24;

      else {
        ok = FALSE;
        gxError ("Bad PCX header found.");
      }

      if (ok) {
        /* Retrieve some variables from header */
        pic_dx = pcxhdr->xmax - pcxhdr->xmin + 1;
        pic_dy = pcxhdr->ymax - pcxhdr->ymin + 1;
        bytes_per_line = pcxhdr->bytes_per_line;
        /* Make sure pic height isn't greater than current page height */
        if (pic_dy > PAGE_HEIGHT)
          pic_dy = PAGE_HEIGHT;

        // Allocate memory for input image scanline
        box.x = 0;
        box.y = 0;
        box.w = pic_dx;
        box.h = 1;
        size = Image_Size (box, num_bits);
        src_image = (byte *) malloc (size);
        // Allocate memory for output image scanline
        size = gxImageSize (box);
        dst_image = (byte *) malloc (size);
        if ((src_image == NULL) OR (dst_image == NULL)) 
          ok = FALSE;

        if (ok) {
          /* Set dimensions of transfer images */
          p = (unsigned *)src_image;
          *p = (unsigned)pic_dx;
          *(p+1) = 1;
          p = (unsigned *)dst_image;
          *p = (unsigned)pic_dx;
          *(p+1) = 1;

          /* Read in palette? */
          if (num_bits == 8) {
            /* Allocate memory for palette */
            palette = (byte *) malloc (NUM_INDEXED_COLORS*3);
            if (palette) {
              if (num_bits == 8) {
                /* Move to the end of the file, then back up 768 bytes */
                fseek (fp, -768L, SEEK_END);
                /* Read in palette data */
                fread ((void *)palette, 1, NUM_INDEXED_COLORS*3, fp);
                /* Set the new palette? */
                if (set_palette)
                  gxSetPalette (palette, NUM_INDEXED_COLORS);
                /* Restore file index */
                fseek (fp, sizeof(PCXHeader), SEEK_SET);
              }
              else if (num_bits == 4) {
                /* Get current palette */
                gxGetPalette (palette, NUM_INDEXED_COLORS);
                /* Set first 16 colors from data in PCX header */
                memcpy ((void *)palette, (void *)(pcxhdr->palette), 16*3);
                /* Set the new palette? */
                if (set_palette)
                  gxSetPalette (palette, NUM_INDEXED_COLORS);
              }
            }
          }
        }
        
        /* Unpack pcx file data */
        if (num_bits == 4)
          Unpack_PCX_File4  (fp, buffer, buffer_size, palette, src_image, dst_image, bytes_per_line*8, pic_dy);
        else if (num_bits == 8)
          Unpack_PCX_File8  (fp, buffer, buffer_size, palette, src_image, dst_image, bytes_per_line, pic_dy);
        else if (num_bits == 24)
          Unpack_PCX_File24 (fp, buffer, buffer_size, src_image, dst_image, bytes_per_line, pic_dy);
      }
    }
  }

/*____________________________________________________________________
|
| Free memory and exit.
|___________________________________________________________________*/

  if (palette)
    free (palette);
  if (buffer)
    free (buffer);
  if (src_image)
    free (src_image);
  if (dst_image)
    free (dst_image);

  gxRestoreState (&state);

  return (ok);
}

/*____________________________________________________________________
|
| Function: Image_Size
|
| Input: Called from gxReadPCXFile(), gxConvertPCXFile()
| Output: Calculates buffer size to store an image with the specified
|   rectangle size and number of bits per pixel.
|___________________________________________________________________*/

static unsigned Image_Size (gxBound box, int num_bits)
{
  int pixel_size;
  unsigned size;

  switch (num_bits) {
    case 4:  // Go ahead and use 1 byte if 4-bit color (more space than needed buts that's ok)
    case 8:  pixel_size = 1;
             break;
    case 24: pixel_size = 3;
             break;
  }
  size = (unsigned)(box.w) * (unsigned)(box.h) * pixel_size + (2*sizeof(unsigned));

  return (size);
}

/*____________________________________________________________________
|
| Function: Unpack_PCX_File4
|
| Inputs: Called from Draw_File()
| Outputs: Unpacks data from a 4-bit pcx file into an image and transfers
|       the image to the page one scanline at a time.  Uses a buffer to
|       speed file input.
|___________________________________________________________________*/

static void Unpack_PCX_File4 (
  FILE     *fp,
  byte     *buffer,             /* buffer for file i/o          */
  unsigned  buffer_size,        /* size of buffer               */
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       pic_dx,             /* dimensions of pcx image      */
  int       pic_dy )
{
  int y, num_bytes, count, next, ignore_last;
  byte *src_image_data, *image16, *dst_image_data;

/*____________________________________________________________________
|
| Create a transfer buffer
|___________________________________________________________________*/

  num_bytes = (pic_dx+7) / 8;
  ignore_last = 7 - ((pic_dx-1) & 7);

  /* Create a 16-color planar image buffer */
  image16 = (byte *) malloc (num_bytes*4);

/*____________________________________________________________________
|
| Unpack the file
|___________________________________________________________________*/

  if (image16) {
    /* Init variables */
    src_image_data = src_image + (2 * sizeof(unsigned));
    dst_image_data = dst_image + (2 * sizeof(unsigned));
    count = Get_PCX_Data (fp, buffer, buffer_size);
    next = 0;

    for (y=0; (y<pic_dy) AND count; y++) {
      /* Unpack data from file into 16-color image */
      Unpack_PCX_Line (fp, buffer, buffer_size, image16, num_bytes*4,
                       &count, &next);
      /* Convert from 16-color image to 256-color image */
      Convert_Scanline_16to256 (image16, src_image_data, num_bytes, ignore_last);
      // Adjust scanline to match screen format
      if (Adjust_Scanline_Pixel_Format (src_image_data, dst_image_data, palette, 1, gx_Pixel_size, pic_dx))
        /* Draw this line on screen */
        gxDrawImage (dst_image, 0, y);
      else
        gxDrawImage (src_image, 0, y);
    }

    /* Free 16-color image buffer */
    free (image16);
  }
}

/*____________________________________________________________________
|
| Function:  Convert_Scanline_16to256
|
| Inputs: Called from Unpack_PCX_File4()
| Outputs: Converts data from source 4-bit planar buffer to 8-bit packed
|         buffer.
|___________________________________________________________________*/

static void Convert_Scanline_16to256 (
  byte *image16,      // src buffer
  byte *image256,     // dst buffer
  int   num_bytes,    // # src bytes (per plane) to process
  int   ignore_last ) // # bits in last byte read to skip
{
  int x, num_pixels;
  byte pixel;
  byte mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
  int shift[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };
  byte *plane0, *plane1, *plane2, *plane3;

  // Init variables
  plane0 = image16;
  plane1 = image16 + num_bytes;
  plane2 = image16 + (num_bytes * 2);
  plane3 = image16 + (num_bytes * 3);

  num_pixels = num_bytes * 8 - ignore_last;

  // Process the data
  for (x=0; x<num_pixels; x++) {
    pixel = 0;
    pixel |=  ((plane0[x/8] & mask[x%8]) >> shift[x%8]);
    pixel |= (((plane1[x/8] & mask[x%8]) >> shift[x%8]) << 1);
    pixel |= (((plane2[x/8] & mask[x%8]) >> shift[x%8]) << 2);
    pixel |= (((plane3[x/8] & mask[x%8]) >> shift[x%8]) << 3);
    image256[x] = pixel;
  }
}

/*____________________________________________________________________
|
| Function: Unpack_PCX_File8
|
| Inputs: Called from Draw_File()
| Outputs: Unpacks data from a 8-bit pcx file into an image and transfers
|       the image to the page one scanline at a time.  Uses a buffer to
|       speed file input.
|___________________________________________________________________*/

static void Unpack_PCX_File8 (
  FILE     *fp,
  byte     *buffer,             /* buffer for file i/o          */
  unsigned  buffer_size,        /* size of buffer               */
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       pic_dx,             /* dimensions of pcx image      */
  int       pic_dy )
{
  int y, count, next;
  byte *src_image_data, *dst_image_data;

/*____________________________________________________________________
|
| Unpack the file
|___________________________________________________________________*/

  src_image_data = src_image + (2 * sizeof(unsigned));
  dst_image_data = dst_image + (2 * sizeof(unsigned));
  count = Get_PCX_Data (fp, buffer, buffer_size);
  next = 0;

  for (y=0; (y<pic_dy) AND count; y++) {
    /* Unpack 1 line from the PCX file */
    Unpack_PCX_Line (fp, buffer, buffer_size, src_image_data, pic_dx, &count, &next);
    // Adjust scanline to match screen format
    if (Adjust_Scanline_Pixel_Format (src_image_data, dst_image_data, palette, 1, gx_Pixel_size, pic_dx))
      /* Draw this line on screen */
      gxDrawImage (dst_image, 0, y);
    else
      gxDrawImage (src_image, 0, y);
  }
}

/*____________________________________________________________________
|
| Function: Unpack_PCX_File24
|
| Inputs: Called from Draw_File()
| Outputs: Unpacks data from a 24-bit pcx file into an image and transfers
|       the image to the page one scanline at a time.  
|___________________________________________________________________*/

static void Unpack_PCX_File24 (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       pic_dx,             // dimensions of pcx image      
  int       pic_dy )
{
  int y, count, next;
  byte *src_image_data, *dst_image_data;

/*____________________________________________________________________
|
| Unpack the file
|___________________________________________________________________*/

  // Init variables 
  src_image_data = src_image + (2 * sizeof(unsigned));
  dst_image_data = dst_image + (2 * sizeof(unsigned));
  count = Get_PCX_Data (fp, buffer, buffer_size);
  next = 0;

  for (y=0; (y<pic_dy) AND count; y++) {
    // Unpack 1 line from the PCX file 
    Unpack_PCX_Line (fp, buffer, buffer_size, src_image_data, pic_dx*3, &count, &next);
    // Adjust scanline to match screen format
    if (Adjust_Scanline_Pixel_Format (src_image_data, dst_image_data, NULL, 3, gx_Pixel_size, pic_dx))
      // Draw this line on screen 
      gxDrawImage (dst_image, 0, y);
    else
      gxDrawImage (src_image, 0, y);
  }
}

/*____________________________________________________________________
|
| Function: Unpack_PCX_Line
|
| Inputs: Called from Unpack_PCX_File8(), Unpack_PCX_File4(), 
|       Unpack_PCX_File24()
| Outputs: Unpacks 1 line of pcx data from file to image.
|___________________________________________________________________*/

static void Unpack_PCX_Line (
  FILE     *fp,
  byte     *buffer,
  unsigned  buffer_size,
  byte     *image_data,
  int       num_bytes,
  int      *count,
  int      *next )
{
  int i, n=0;
  byte c;

  /* Zero the scanline */
  memset (image_data, 0, num_bytes);

  while (n < num_bytes) {
    /* Get a key byte */
    if (*next >= *count) {
      *count = Get_PCX_Data (fp, buffer, buffer_size);
      if (*count == 0) {
        gxError ("Error reading PCX line");
        break;
      }
      *next = 0;
    }
    c = buffer[(*next)++];
    /* If it's a run of bytes field... */
    if ((c & (byte)0xC0) == (byte)0xC0) {
      /* and off the high bits */
      i = c & (byte)0x3F;
      /* get the run byte */
      if (*next >= *count) {
        *count = Get_PCX_Data (fp, buffer, buffer_size);
        if (*count == 0) {
          gxError ("Error reading PCX line");
          break;
        }
        *next = 0;
      }
      c = buffer[(*next)++];
      /* run the byte */
      while (i--)
        image_data[n++] = c;
    }
    /* else just store it */
    else
      image_data[n++] = c;
  }
}

/*____________________________________________________________________
|
| Function: Get_PCX_Data
|
| Inputs: Called from ____
| Outputs: Reads data from pcx file into a buffer.
|___________________________________________________________________*/

static int Get_PCX_Data (FILE *fp, byte *buffer, unsigned buffer_size)
{
  return ((int)fread ((void *)buffer, 1, buffer_size, fp));
}

/*____________________________________________________________________
|
| Function: Adjust_Scanline_Pixel_Format
|
| Inputs: Called from Unpack_PCX_File4()
|                     Unpack_PCX_File8()
|                     Unpack_PCX_File24()
| Outputs: This function changes the pixel format of the image, if
|   necessary, to match the pixel format of the screen. Returns true if
|   adjusted.
|___________________________________________________________________*/

static int Adjust_Scanline_Pixel_Format (
  byte *src_image,            // single scanline image
  byte *dst_image,            // single scanline image
  byte *palette,              // image palette, if image is in 8-bit color format
  int   src_bytes_per_pixel,  // # bytes per pixel in src image
  int   dst_bytes_per_pixel,  // # bytes per pixel in dst image 
  int   num_pixels )          // # pixels represented in image
{
  int i;
  unsigned pixel;
  int adjusted = FALSE;
  
/*____________________________________________________________________
|
| Convert 8-bit color to 16-bit color
|___________________________________________________________________*/

  if ((src_bytes_per_pixel == 1) AND (dst_bytes_per_pixel == 2)) {
    // Process all pixels in scanline
    for (i=num_pixels-1; i>=0; i--) {
      pixel = ((unsigned)palette[src_image[i]]   >> (8 - gx_Video.num_redbits  ) << gx_Video.low_redbit  ) |
              ((unsigned)palette[src_image[i]+1] >> (8 - gx_Video.num_greenbits) << gx_Video.low_greenbit) |
              ((unsigned)palette[src_image[i]+2] >> (8 - gx_Video.num_bluebits ) << gx_Video.low_bluebit );
      memcpy (&dst_image[i*2], &pixel, 2);
    }
    adjusted = TRUE;
  }

/*____________________________________________________________________
|
| Convert 8-bit color to 24-bit color
|___________________________________________________________________*/

  else if ((src_bytes_per_pixel == 1) AND (dst_bytes_per_pixel == 3)) {
    // Process all pixels in scanline
    for (i=num_pixels-1; i>=0; i--) {
      pixel = ((unsigned)palette[src_image[i]]   << gx_Video.low_redbit  ) |
              ((unsigned)palette[src_image[i]+1] << gx_Video.low_greenbit) |
              ((unsigned)palette[src_image[i]+2] << gx_Video.low_bluebit );
      memcpy (&dst_image[i*3], &pixel, 3);
    }
    adjusted = TRUE;
  }

/*____________________________________________________________________
|
| Convert 8-bit color to 32-bit color
|___________________________________________________________________*/

  else if ((src_bytes_per_pixel == 1) AND (dst_bytes_per_pixel == 4)) {
    pixel = 0;
    // Process all pixels in scanline
    for (i=num_pixels-1; i>=0; i--) {
      pixel = ((unsigned)palette[src_image[i]]   << gx_Video.low_redbit  ) |
              ((unsigned)palette[src_image[i]+1] << gx_Video.low_greenbit) |
              ((unsigned)palette[src_image[i]+2] << gx_Video.low_bluebit );
      memcpy (&dst_image[i*4], &pixel, 4);
    }
    adjusted = TRUE;
  }

/*____________________________________________________________________
|
| Convert 16-bit color to 24-bit color
|___________________________________________________________________*/

  else if ((src_bytes_per_pixel == 2) AND (dst_bytes_per_pixel == 3)) {
    // Process all pixels in scanline
    for (i=0; i<num_pixels; i++) {
      pixel = (unsigned)((word *)src_image)[i];
      dst_image[i]              = (byte)((pixel & gx_Video.redmask)   >> gx_Video.low_redbit   << (8 - gx_Video.num_redbits));
      dst_image[i+num_pixels]   = (byte)((pixel & gx_Video.greenmask) >> gx_Video.low_greenbit << (8 - gx_Video.num_greenbits));
      dst_image[i+num_pixels*2] = (byte)((pixel & gx_Video.bluemask)  >> gx_Video.low_bluebit  << (8 - gx_Video.num_bluebits));
    }
    adjusted = TRUE;
  }  

/*____________________________________________________________________
|
| Convert 24-bit color to 8-bit color
|___________________________________________________________________*/

  else if ((src_bytes_per_pixel == 3) AND (dst_bytes_per_pixel == 1)) {
    int j, n, entry;
    float div;
    byte rgb[3], *uniform_palette;

    // Create a temporary uniform palette
    uniform_palette = (byte *) malloc (NUM_INDEXED_COLORS * 3 * sizeof(byte));
    if (uniform_palette) {
      /* Get high spread uniform palette */
      gxGetPalette (uniform_palette, NUM_INDEXED_COLORS);
      gxSetUniformPalette (uniform_palette, gxPALETTE_SPREAD_HIGH);

      // Init values for palette spread high
      entry = 40;
      div   = 5;
      n     = 6;

      // Process all pixels in scanline
      for (i=0; i<num_pixels; i++) {
        // Get a 3-byte RGB value from scanline
        rgb[0] = src_image[i];
        rgb[1] = src_image[i + num_pixels];
        rgb[2] = src_image[i + (num_pixels*2)];
        // Convert each rgb value from 256 to n possible intensities 
        for (j=0; j<3; j++) {
          rgb[j] = (byte)( ((float)(rgb[j]) / ((float)256/(float)n)) * ((float)63/(float)div) );
          rgb[j] = rgb[j] << 2;
        }
        // Find the palette entry that matches the new rgb combination 
        for (j=entry; j<NUM_INDEXED_COLORS; j++)
          if (!memcmp ((void *)rgb, (void *)&uniform_palette[j*3], 3))
            break;
        // Error? 
        if (j == NUM_INDEXED_COLORS)
          j = 0;
        // Set new value for this pixel 
        dst_image[i] = j;
      }

      free (uniform_palette);
    }
    adjusted = TRUE;
  }

/*____________________________________________________________________
|
| Convert 24-bit color to 16-bit color
|___________________________________________________________________*/

  else if ((src_bytes_per_pixel == 3) AND (dst_bytes_per_pixel == 2)) {
    // Process all pixels in scanline
    for (i=0; i<num_pixels; i++) {
      pixel = ((unsigned)src_image[             i] >> (8 - gx_Video.num_redbits  ) << gx_Video.low_redbit  ) |
              ((unsigned)src_image[num_pixels  +i] >> (8 - gx_Video.num_greenbits) << gx_Video.low_greenbit) |
              ((unsigned)src_image[num_pixels*2+i] >> (8 - gx_Video.num_bluebits ) << gx_Video.low_bluebit );
      memcpy (dst_image, &pixel, 2);
      dst_image += 2;
    }
    adjusted = TRUE;
  }

/*____________________________________________________________________
|
| Convert 24-bit color to 24-bit color
|___________________________________________________________________*/

  else if ((src_bytes_per_pixel == 3) AND (dst_bytes_per_pixel == 3)) {
    for (i=0; i<num_pixels; i++) {
      pixel = ((unsigned)src_image[             i] << gx_Video.low_redbit  ) |
              ((unsigned)src_image[num_pixels  +i] << gx_Video.low_greenbit) |
              ((unsigned)src_image[num_pixels*2+i] << gx_Video.low_bluebit );
      memcpy (dst_image, &pixel, 3);
      dst_image += 3;
    }
    adjusted = TRUE;
  }  

/*____________________________________________________________________
|
| Convert 24-bit color to 32-bit color
|___________________________________________________________________*/

  else if ((src_bytes_per_pixel == 3) AND (dst_bytes_per_pixel == 4)) {
    // Pad every 4th byte with a zero, extending all pixels in scanline
    for (i=0; i<num_pixels; i++) {
      pixel = ((unsigned)src_image[            +i] << gx_Video.low_redbit  ) |
              ((unsigned)src_image[num_pixels  +i] << gx_Video.low_greenbit) |
              ((unsigned)src_image[num_pixels*2+i] << gx_Video.low_bluebit );
      memcpy (&dst_image[i*4], &pixel, 4);
    }
    adjusted = TRUE;
  }  

/*____________________________________________________________________
|
| Convert 32-bit color to 24-bit color
|___________________________________________________________________*/

  else if ((src_bytes_per_pixel == 4) AND (dst_bytes_per_pixel == 3)) {
    for (i=0; i<num_pixels; i++) {
      pixel = ((unsigned *)src_image)[i];
      // Compute red byte
      dst_image[i]              = pixel >> gx_Video.low_redbit;
      // Compute green byte
      dst_image[i+num_pixels]   = pixel >> gx_Video.low_greenbit;
      // Compute blue byte
      dst_image[i+num_pixels*2] = pixel >> gx_Video.low_bluebit;
    }
    adjusted = TRUE;
  }  

  return (adjusted);
}

/*____________________________________________________________________
|
| Function: gxWritePCXFile
|
| Output: Captures image on active page saving it in a PCX file.  
|   Returns true if successful, else false on any error.
|
| Description: If current screen format is 8-bit color, will save in an
|   8-bit PCX file.  If current screen format is 16,24, or 32-bit color, 
|   will save in a 24-bit BMP file.
|___________________________________________________________________*/

int gxWritePCXFile (char *filename)
{
  byte       id;
  FILE      *fp;
  PCXHeader *pcxhdr;
  byte      *src_image = NULL;
  byte      *dst_image = NULL;
  byte      *image = NULL;
  byte      *palette;
  gxBound    box;
  unsigned   size;
  int        ok = TRUE;

/*____________________________________________________________________
|
| Allocate memory for a 1 scanline image
|___________________________________________________________________*/

  box.x = 0;
  box.y = 0;
  box.w = PAGE_WIDTH;
  box.h = 1;
  size = gxImageSize (box);
  // Make sure image buffer large enough to expand from 16-bit pixels to 24-bits, if needed
  if (gx_Pixel_size == 2)
    size *= 2;
  src_image = (byte *) malloc (size);
  dst_image = (byte *) malloc (size);
  if ((src_image == NULL) OR (dst_image == NULL))
    ok = FALSE;

/*____________________________________________________________________
|
| Allocate memory for a pcx header.
|___________________________________________________________________*/

  pcxhdr = (PCXHeader *) calloc (1, sizeof(PCXHeader));
  if (pcxhdr == NULL)
    ok = FALSE;

/*____________________________________________________________________
|
| Write PCX header to file.
| Finally, call a routine to pack the pcx file.
|___________________________________________________________________*/

  if (ok) {
    fp = fopen (filename, "wb");
    if (fp == NULL) {
      ok = FALSE;
      gxError ("Can't open PCX file.");
    }
    else {
      // Create a pcx header for an 8-bit or 24-bit color file
      pcxhdr->manufacturer   = 0xA;
      pcxhdr->version        = 5;
      pcxhdr->encoding       = 1;
      pcxhdr->bits_per_pixel = 8;
      pcxhdr->xmin           = 0;
      pcxhdr->ymin           = 0;
      pcxhdr->xmax           = PAGE_WIDTH - 1;
      pcxhdr->ymax           = PAGE_HEIGHT - 1;
      pcxhdr->hres           = PAGE_WIDTH;
      pcxhdr->vres           = PAGE_HEIGHT;
      pcxhdr->reserved       = 0;
      pcxhdr->bytes_per_line = PAGE_WIDTH;
      if (gx_Pixel_size == 1) 
        pcxhdr->color_planes = 1; 
      else 
        pcxhdr->color_planes = 3; 
      pcxhdr->palette_type   = 1;
      memset (pcxhdr->filler, 0, 58);
      // Write header to file 
      fwrite ((void *)pcxhdr, sizeof(PCXHeader), 1, fp);
      
      // Pack the page data into the pcx file 
      Pack_PCX_File (fp, src_image, dst_image, PAGE_HEIGHT);

      // Write palette at end of file (8-bits files only)
      if (gx_Pixel_size == 1) {
        id = 12;
        fwrite ((void *)&id, 1, 1, fp);
        // Write palette at end of file 
        palette = (byte *) malloc (NUM_INDEXED_COLORS * 3);
        if (palette) {
          gxGetPalette (palette, NUM_INDEXED_COLORS);
          fwrite ((void *)palette, 3, NUM_INDEXED_COLORS, fp);
          free (palette);
        }
      }

      fclose (fp);
    }
  }

/*____________________________________________________________________
|
| Free memory and exit.
|___________________________________________________________________*/

  if (src_image)
    free (src_image);
  if (dst_image)
    free (dst_image);
  if (pcxhdr)
    free (pcxhdr);

  return (ok);
}

/*____________________________________________________________________
|
| Function: Pack_PCX_File
|
| Inputs: Called from gxWritePCXFile()
| Outputs: Reads page data 1 line at a time, packing it into a pcx
|       file.
|___________________________________________________________________*/

static void Pack_PCX_File (
  FILE *fp,             // pcx file             
  byte *src_image,      // 1 scanline image     
  byte *dst_image,
  int   pic_dy )
{
  int y;
  gxBound box;
  byte *src_image_data, *dst_image_data;

  src_image_data = src_image + (2 * sizeof(unsigned));
  dst_image_data = dst_image + (2 * sizeof(unsigned));
  box.x = 0;
  box.w = PAGE_WIDTH;
  box.h = 1;

  for (y=0; y<pic_dy; y++) {
    box.y = y;
    gxGetImage (box, src_image);

    // If 16-bit data received, convert to 24
    if (gx_Pixel_size == 2) 
      Adjust_Scanline_Pixel_Format (src_image_data, dst_image_data, NULL, 2, 3, box.w);  
    // If 32-bit data received, convert to 24
    else if (gx_Pixel_size == 4)
      Adjust_Scanline_Pixel_Format (src_image_data, dst_image_data, NULL, 4, 3, box.w);
    else
      dst_image_data = src_image_data;
    // Output line in 1-byte per pixel format
    if (gx_Pixel_size == 1)
      Pack_PCX_Line (fp, dst_image_data, box.w);
    // Output line in 3-byte per pixel format
    else
      Pack_PCX_Line (fp, dst_image_data, box.w * 3);
  }
}

/*____________________________________________________________________
|
| Function: Pack_PCX_Line
|
| Inputs: Called from Pack_PCX_Line()
| Outputs: Unpacks a line of image data, writing it into a pcx file.
|___________________________________________________________________*/

static void Pack_PCX_Line (
  FILE *fp,             /* pcx file                             */
  byte *image_data,     /* 1 scanline image                     */
  int   width )
{
  int i;
  int n;                /* # bytes from image packed so far     */

  for (n=0; n<width;) {
    i = 1;
    while ((image_data[n] == image_data[n+i]) AND ((n+i) < width) AND (i<63))
      i++;
    if (i > 1) {
      fputc (i | 0xC0, fp);
      fputc (image_data[n], fp);
      n += i;
    }
    else {
      if ((image_data[n] & (byte)0xC0) == (byte)0xC0)
        fputc (0xC1, fp);
      fputc (image_data[n++], fp);
    }
  }
}

/*____________________________________________________________________
|
| Function: gxGetPCXFileDimensions
|
| Outputs: Returns true if PCX file read successfully.  Also returns
|   width and height of PCX file image.  Returns FALSE on any error.
|___________________________________________________________________*/

int gxGetPCXFileDimensions (char *filename, int *width, int *height)
{
  int        bytes_read;
  PCXHeader *pcxhdr;
  FILE      *fp;

  int file_ok = FALSE;

  // Allocate memory for a PCX file header
  pcxhdr = (PCXHeader *) malloc (sizeof(PCXHeader));
  if (pcxhdr == NULL) 
    gxError ("gxGetPCXFileDimensions(): error allocating memory for header");
  else {
    // Open file for reading
    fp = fopen (filename, "rb");
    if (fp == NULL) 
      gxError ("gxGetPCXFileDimensions(): can't open PCX file");
    else {
      // Read in header
      bytes_read = (int)fread ((void *)pcxhdr, 1, sizeof(PCXHeader), fp);
      if (bytes_read != sizeof(PCXHeader)) 
        gxError ("gxGetPCXFileDimensions(): couldn't read PCX header");
      else {
        // Is this a valid PCX file?
        if ((pcxhdr->manufacturer != 0xA) OR
            (pcxhdr->encoding     != 1)) 
          gxError ("gxGetPCXFileDimensions(): bad PCX header found");
        else {
          // Get dimensions of image
          *width  = pcxhdr->xmax - pcxhdr->xmin + 1;
          *height = pcxhdr->ymax - pcxhdr->ymin + 1;
          file_ok = TRUE;
        }
      }
      fclose (fp);
    }
    free (pcxhdr);
  }

  return (file_ok);
}

/*____________________________________________________________________
|
| Function: gxConvertPCXFile
|
| Output: Converts a 16M-color or 256-color PCX file to be a 256-color
|       PCX file that uses the 216-color uniform palette (color
|       registers 40-255).
|
| Description: This function allocates a virtual page to perform it's
|       manipulation and then frees it on exit.
|___________________________________________________________________*/

int gxConvertPCXFile (
  char *infilename,
  char *outfilename,
  int   palette_spread )
{
  int         i, pic_dx, pic_dy, bytes_read, bytes_per_line;
  FILE       *fp;
  byte       *buffer = NULL;    /* file i/o buffer              */
  unsigned    buffer_size;      /* size of i/o buffer           */
  PCXHeader  *pcxhdr;
  byte       *image = NULL;
  gxBound     box;
  unsigned    size, *p;
  gxState     state;
  byte       *file_palette, *uniform_palette;
  unsigned    kilobyte = 1024;
  gxPage      vpage;
  int         ok = TRUE;

/*____________________________________________________________________
|
| Save current graphics state.
|___________________________________________________________________*/

  gxSaveState (&state);

/*____________________________________________________________________
|
| Allocate up to 100K temporary memory space for file i/o.
|___________________________________________________________________*/

  for (i=100; i>0; i-=20)
    if ((buffer = (byte *) malloc ((unsigned)i*kilobyte)) != NULL) {
      buffer_size = (unsigned)i * kilobyte;
      break;
    }
  if (i == 0) {
    gxError ("Not enough memory to display a PCX file.");
    ok = FALSE;
  }

/*____________________________________________________________________
|
| Allocate memory for several palettes.
|___________________________________________________________________*/

  /* Allocate memory for file palette */
  file_palette = (byte *) malloc (768);
  if (file_palette == NULL)
    ok = FALSE;

  /* Allocate memory for uniform palette */
  uniform_palette = (byte *) malloc (768);
  if (uniform_palette == NULL)
    ok = FALSE;

/*____________________________________________________________________
|
| Enable virtual page operation
|___________________________________________________________________*/

  if (NOT gxCreateVirtualPage (gxGetScreenWidth(), gxGetScreenHeight(), gxHINT_CREATE_IN_SYSTEM_MEMORY, &vpage))
    ok = FALSE;
  else {
    gxSetActivePage (vpage);
    gxSetWindow (&gx_Screen);
    gxSetClip   (&gx_Screen);
    gxSetClipping (TRUE);
    gxGetPalette (uniform_palette, 256);
    gxSetUniformPalette (uniform_palette, palette_spread);
  }

/*____________________________________________________________________
|
| Read PCX header from file and extract info, then call a routine to
| unpack the pcx file.
|___________________________________________________________________*/

  if (ok) {
    fp = fopen (infilename, "rb");
    if (fp == NULL) {
      ok = FALSE;
      gxError ("Can't open PCX file.");
    }
    else {
      pcxhdr = (PCXHeader *)buffer;
      bytes_read = (int)fread ((void *)pcxhdr, 1, sizeof(PCXHeader), fp);
      if (bytes_read != sizeof(PCXHeader)) {
        ok = FALSE;
        gxError ("Couldn't read PCX header.");
      }
      else {
        if ((pcxhdr->manufacturer   != 0xA) OR 
            (pcxhdr->encoding       != 1)   OR
            (pcxhdr->bits_per_pixel != 8)) {
          ok = FALSE;
          gxError ("Bad PCX header found.");
        }
        else {
          /* Retrieve some variables from header */
          pic_dx = pcxhdr->xmax - pcxhdr->xmin + 1;
          pic_dy = pcxhdr->ymax - pcxhdr->ymin + 1;
          bytes_per_line = pcxhdr->bytes_per_line;
          /* Make sure pic height isn't greater than current page height */
          if (pic_dy > PAGE_HEIGHT)
            pic_dy = PAGE_HEIGHT;

          // Allocate memory for input image scanline that is 24-bit color
          box.x = 0;
          box.y = 0;
          box.w = pic_dx;
          box.h = 1;
          size = Image_Size (box, 3);
          image = (byte *) malloc (size);
          if (image == NULL) 
            ok = FALSE;

          if (ok) {
            /* Set dimensions of transfer image */
            p = (unsigned *)image;
            *p = (unsigned)pic_dx;
            *(p+1) = 1;

            /* Read palette? */
            if (pcxhdr->color_planes == 1) {
              /* Move to the end of the file, then back up 768 bytes */
              fseek (fp, -768L, SEEK_END);
              /* Read in palette data */
              fread ((void *)file_palette, 1, 768, fp);
              /* Restore file index */
              fseek (fp, sizeof(PCXHeader), SEEK_SET);
            }

            /* Unpack pcx file data */
            if (pcxhdr->color_planes == 3)
              Unpack_PCX24_File (fp, buffer, buffer_size, image, bytes_per_line,
                                 pic_dy, palette_spread, uniform_palette);
            else if (pcxhdr->color_planes == 1)
              Unpack_PCX8_File (fp, buffer, buffer_size, image, bytes_per_line,
                                pic_dy, palette_spread, uniform_palette, file_palette);
            fclose (fp);
          }
        }
      }
    }
  }

/*____________________________________________________________________
|
| Free some memory.
|___________________________________________________________________*/

  if (buffer)
    free (buffer);
  if (image)
    free (image);
  if (file_palette)
    free (file_palette);

/*____________________________________________________________________
|
| Write output PCX file.
|___________________________________________________________________*/

  if (ok) {
    gxWritePCXFile (outfilename);

    /* Write uniform palette to file */
    fp = fopen (outfilename, "r+b");
    if (fp) {
      /* Move to the end of the file, then back up 768 bytes */
      fseek (fp, -768L, SEEK_END);
      /* Write out palette data */
      fwrite ((void *)uniform_palette, 1, 768, fp);
      /* Close the out file */
      fclose (fp);
    }
  }

/*____________________________________________________________________
|
| Free rest of memory and exit.
|___________________________________________________________________*/

  if (uniform_palette)
    free (uniform_palette);

  if (vpage != -1)
    gxFreeVirtualPage (vpage);

  gxRestoreState (&state);

  return (ok);
}

/*____________________________________________________________________
|
| Function: Unpack_PCX24_File
|
| Inputs: Called from gxConvertPCXFile()
| Outputs: Unpacks data from a pcx file into an image and transfers
|       the image to the page one scanline at a time.  Uses a
|       buffer to speed file input.
|___________________________________________________________________*/

static void Unpack_PCX24_File (
  FILE     *fp,
  byte     *buffer,             /* buffer for file i/o          */
  unsigned  buffer_size,        /* size of buffer               */
  byte     *image,              /* 1 scanline image             */
  int       pic_dx,             /* dimensions of pcx image      */
  int       pic_dy,
  int       palette_spread,
  byte     *uniform_palette )
{
  int i, j, y, count, next, n, entry;
  float div;
  byte *image_data;
  byte rgb[3];

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  if (palette_spread == gxPALETTE_SPREAD_LOW) {
    entry = 131;
    div   = 4;
    n     = 5;
  }
  else { /* gxPALETTE_SPREAD_HIGH */
    entry = 40;
    div   = 5;
    n     = 6;
  }

/*____________________________________________________________________
|
| Unpack the file
|___________________________________________________________________*/

  image_data = image + (2 * sizeof(unsigned));
  count = Get_PCX_Data (fp, buffer, buffer_size);
  next = 0;

  for (y=0; (y<pic_dy) AND count; y++) {
    /* Unpack 1 line from the PCX file */
    Unpack_PCX_Line (fp, buffer, buffer_size, image_data, pic_dx*3,
                     &count, &next);

    /* Translate each 24-bit color pixel to an 8-bit color index value */
    for (i=0; i<pic_dx; i++) {
      rgb[0] = image_data[i];
      rgb[1] = image_data[i + pic_dx];
      rgb[2] = image_data[i + (pic_dx*2)];
      /* Convert each rgb value from 256 to n possible intensities */
      for (j=0; j<3; j++) {
        rgb[j] = (int)( (float)(rgb[j]) / ((float)256.0/(float)n) * ((float)63.0/div) );
        rgb[j] = rgb[j] << 2;
      }
      /* Find the palette entry that matches the new rgb combination */
      for (j=entry; j<256; j++)
        if (!memcmp ((void *)rgb, (void *)&uniform_palette[j*3], 3))
          break;
      /* Error? */
      if (j == 256)
        j = 0;
      /* Set new value for this pixel */
      image_data[i] = j;
    }

    /* Draw this line on page */
    gxDrawImage (image, 0, y);
  }
}

/*____________________________________________________________________
|
| Function: Unpack_PCX8_File
|
| Inputs: Called from gxConvertPCXFile()
| Outputs: Unpacks data from a pcx file into an image and transfers
|       the image to the page one scanline at a time.  Uses a buffer
|       to speed file input.
|___________________________________________________________________*/

static void Unpack_PCX8_File (
  FILE     *fp,
  byte     *buffer,             /* buffer for file i/o          */
  unsigned  buffer_size,        /* size of buffer               */
  byte     *image,              /* 1 scanline image             */
  int       pic_dx,             /* dimensions of pcx image      */
  int       pic_dy,
  int       palette_spread,
  byte     *uniform_palette,
  byte     *file_palette )      /* palette read from file       */
{
  int i, j, y, count, next, n, entry;
  float div;
  byte *image_data;
  byte rgb[3];

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  if (palette_spread == gxPALETTE_SPREAD_LOW) {
    entry = 131;
    div   = 4;
    n     = 5;
  }
  else { /* gxPALETTE_SPREAD_HIGH */
    entry = 40;
    div   = 5;
    n     = 6;
  }

/*____________________________________________________________________
|
| Unpack the file
|___________________________________________________________________*/

  image_data = image + (2 * sizeof(unsigned));
  count = Get_PCX_Data (fp, buffer, buffer_size);
  next = 0;

  for (y=0; (y<pic_dy) AND count; y++) {
    /* Unpack 1 line from the PCX file */
    Unpack_PCX_Line (fp, buffer, buffer_size, image_data, pic_dx,
                     &count, &next);

    /* Translate each pixel to a color in the current palette. */
    for (i=0; i<pic_dx; i++) {
      rgb[0] = file_palette[image_data[i]*3+0];
      rgb[1] = file_palette[image_data[i]*3+1];
      rgb[2] = file_palette[image_data[i]*3+2];
      /* Convert each rgb value from 64 to n possible intensities */
      for (j=0; j<3; j++) {
        rgb[j] = (int)( (float)(rgb[j]) / ((float)256.0/(float)n) * ((float)63.0/div) );
        rgb[j] = rgb[j] << 2;
      }
      /* Find the palette entry that matches the new rgb combination */
      for (j=entry; j<256; j++)
        if (!memcmp ((void *)rgb, (void *)&uniform_palette[j*3], 3))
          break;
      /* Error? */
      if ((j == 40) OR (j == 256))
        j = 0;
      /* Set new value for this pixel */
      image_data[i] = j;
    }

    /* Draw this line on page */
    gxDrawImage (image, 0, y);
  }
}
