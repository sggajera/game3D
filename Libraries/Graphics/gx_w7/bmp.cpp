/*____________________________________________________________________
|
| File: bmp.cpp
|
| Description: Functions for reading/writing BMP files.
|
| Functions:    gxReadBMPFile
|                Draw_File
|                 Unpack_BMP_File4
|                 Unpack_BMP_File8
|                 Unpack_BMP_File24
|                  Unpack_BMP_Line
|                   Get_BMP_Data
|                 Unpack_BMP_File4_Encoded
|                 Unpack_BMP_File8_Encoded
|                  Adjust_Scanline_Pixel_Format
|               gxWriteBMPFile
|                Pack_BMP_File
|                 Pack_BMP_Line
|               gxGetBMPFileDimensions
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
  // Bitmap File Header
  char     file_type[2];					// always ASCII 'BM'                            
  unsigned file_size;							// max file size, could be less if data encoded 
  unsigned reserved;							// must be 0                                    
  unsigned bitmap_data_offset;		// offset from start of file of bitmap data     
  // Bitmap Information
  unsigned bytes_bitmap_info;			// currently 40                                 
  unsigned bitmap_width;					// in pixels                                    
  unsigned bitmap_height;					// in pixels                                    
  word     color_planes;					// must be 1                                    
  word     bits_per_pixel;				// 1,4,8 or 24 (if not 24, tells palette size)  
  unsigned encoding;							// 0=none, 1=run length (8 bit/pixel), 2=run length (4 bit/pixel) 
  unsigned image_size;						// max size (may include 32-bit scanline padding) 
  unsigned hres;									// in pixels/meter (set to 1)                   
  unsigned vres;									// in pixels/meter (set to 1)                   
  unsigned num_colors_used;				// # color indexes used by bitmap, 0=all used   
  unsigned num_colors_important;	// # color indexes important, 0=all important  
} BMPHeader;  

/*___________________
|
| Function prototypes
|__________________*/

static int Draw_File (FILE *fp, int set_palette);

static void Unpack_BMP_File4 (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       bytes_per_line,
  int       pic_dx,             // dimensions of bmp image      
  int       pic_dy );
static void Unpack_BMP_File8 (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       bytes_per_line,
  int       pic_dx,             // dimensions of bmp image      
  int       pic_dy );
static void Unpack_BMP_File24 (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       bytes_per_line,
  int       pic_dx,             // dimensions of bmp image      
  int       pic_dy );
static void Unpack_BMP_Line (
  FILE     *fp,
  byte     *buffer,
  unsigned  buffer_size,
  byte     *image_data,
  int       num_file_bytes,     // per row (could be > num_data_bytes)
  int       num_data_bytes,     // per row
  int      *count,
  int      *next );
static int Get_BMP_Data (FILE *fp, byte *buffer, unsigned buffer_size);
static void Unpack_BMP_File4_Encoded (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       pic_dx,             // dimensions of bmp image      
  int       pic_dy );
static void Unpack_BMP_File8_Encoded (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       pic_dx,             // dimensions of bmp image      
  int       pic_dy );
static int Adjust_Scanline_Pixel_Format (
  byte *src_image,            // single scanline image
  byte *dst_image,            // single scanline image
  byte *palette,              // image palette, if image is in 8-bit color format
  int   src_bytes_per_pixel,  // # bytes per pixel in src image
  int   dst_bytes_per_pixel,  // # bytes per pixel in dst image 
  int   num_pixels );         // # pixels represented in image
static void Pack_BMP_File (
  FILE *fp,             // bmp file
  byte *src_image,      // 1 scanline image
  byte *dst_image,    
  int   pic_dy );
static void Pack_BMP_Line (
  FILE *fp,             // bmp file
  byte *image_data,     // 1 scanline image
  int  width );

/*___________________
|
| Defines
|__________________*/

#define ENCODING_NONE           0
#define ENCODING_RUNLENGTH8     1
#define ENCODING_RUNLENGTH4     2

/*____________________________________________________________________
|
| Function: gxReadBMPFile
|
| Outputs: Draws a 4,8 or 24-bit BMP file on the active page.  Returns
|       true, if successful, else false on any error.  Will optionally,
|       set a new color palette using palette info from BMP file.
|___________________________________________________________________*/

int gxReadBMPFile (char *filename, int set_palette)
{
  FILE *fp;
  int   ok = TRUE;

  fp = fopen (filename, "rb");
  if (fp == NULL) {
    ok = FALSE;
    gxError ("Can't open BMP file.");
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
| Inputs: Called from gxReadBMPFile()
| Outputs: Draws a BMP file on the active page.  Returns true
|       if successful, else false on any error.  Will optionally, set
|       a new color palette using palette info from BMP file.
|___________________________________________________________________*/

static int Draw_File (FILE *fp, int set_palette)
{
  int         i, pic_dx, pic_dy, bytes_read, bytes_per_line, num_bits;
  byte       *buffer = NULL;  // file i/o buffer              
  unsigned    buffer_size;    // size of i/o buffer           
  byte       *palette = NULL; // palette read from file   
  BMPHeader  *bmphdr;
  byte       *src_image = NULL;
  byte       *dst_image = NULL;
  gxBound     box;
  unsigned    size, *p;
  gxRectangle rect;
  gxState     state;
  byte        dummy;
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
    gxError ("Not enough memory to display a BMP file.");
    ok = FALSE;
  }

/*____________________________________________________________________
|
| Read BMP header from file and extract info.  Set palette if needed.
| Finally, call a routine to unpack the bmp file.
|___________________________________________________________________*/

  if (ok) {
    bmphdr = (BMPHeader *)buffer;
    bytes_read = (int)fread ((void *)bmphdr, 1, sizeof(BMPHeader), fp);
    if (bytes_read != sizeof(BMPHeader)) {
      ok = FALSE;
      gxError ("Couldn't read BMP header.");
    }
    else {
      // Is this a 4-bit BMP file?
      if ((bmphdr->file_type[0]   == 'B') AND
          (bmphdr->file_type[1]   == 'M') AND
          (bmphdr->reserved       == 0)   AND
          (bmphdr->color_planes   == 1)   AND
          (bmphdr->bits_per_pixel == 4)   AND
          ((bmphdr->encoding == ENCODING_NONE) OR (bmphdr->encoding == ENCODING_RUNLENGTH4)))
        num_bits = 4;
      // Is this a 8-bit BMP file?
      else if ((bmphdr->file_type[0]   == 'B') AND
               (bmphdr->file_type[1]   == 'M') AND
               (bmphdr->reserved       == 0)   AND
               (bmphdr->color_planes   == 1)   AND
               (bmphdr->bits_per_pixel == 8)   AND
               ((bmphdr->encoding == ENCODING_NONE) OR (bmphdr->encoding == ENCODING_RUNLENGTH8)))
        num_bits = 8;
      // Is this a 24-bit BMP file?
      else if ((bmphdr->file_type[0]   == 'B') AND
               (bmphdr->file_type[1]   == 'M') AND
               (bmphdr->reserved       == 0)   AND
               (bmphdr->color_planes   == 1)   AND
               (bmphdr->bits_per_pixel == 24)  AND
               (bmphdr->encoding       == ENCODING_NONE))
        num_bits = 24;
      else {
        ok = FALSE;
        gxError ("Bad BMP header found.");
      }

      if (ok) {
        // Retrieve some variables from header
        pic_dx = bmphdr->bitmap_width;
        pic_dy = bmphdr->bitmap_height;
        switch (num_bits) {
          case 4:  bytes_per_line = pic_dx / 2;
                   break;
          case 8:  bytes_per_line = pic_dx;
                   break;
          case 24: bytes_per_line = pic_dx * 3;
                   break;
        } 
        // BMP files pad each scanline with 0-3 bytes so each scanline can start on a 4-byte boundaries
        bytes_per_line = (bytes_per_line+3)/4*4;
        // Make sure pic height isn't greater than current page height
        if (pic_dy > PAGE_HEIGHT)
          pic_dy = PAGE_HEIGHT;

        // Allocate memory for input image scanline
        box.x = 0;
        box.y = 0;
        box.w = pic_dx;
        box.h = 1;
        size = bytes_per_line + (2 * sizeof(unsigned));
        src_image = (byte *) malloc (size);
        // Allocate memory for output image scanline
        size = gxImageSize (box);
        dst_image = (byte *) malloc (size);
        if ((src_image == NULL) OR (dst_image == NULL)) 
          ok = FALSE;

        if (ok) {
          // Set dimensions of transfer images
          p = (unsigned *)src_image;
          *p = (unsigned)pic_dx;
          *(p+1) = 1;
          p = (unsigned *)dst_image;
          *p = (unsigned)pic_dx;
          *(p+1) = 1;
        
          // Read in palette?
          if (num_bits == 8) {
            // Allocate memory for palette
            palette = (byte *) malloc (NUM_INDEXED_COLORS*3);
            if (palette) {
              if (num_bits == 8) {
                // Read in palette data
                for (i=0; i<NUM_INDEXED_COLORS; i++) {
                  fread (&palette[i*3+2], 1, 1, fp);
                  fread (&palette[i*3+1], 1, 1, fp);
                  fread (&palette[i*3+0], 1, 1, fp);
                  fread (&dummy, 1, 1, fp);
                }
                // Set the new palette?
                if (set_palette)
                  gxSetPalette (palette, NUM_INDEXED_COLORS);
              }
              else if (num_bits == 4) {
                // Get current palette
                gxGetPalette (palette, NUM_INDEXED_COLORS);
                // Read in 16 colors from BMP header
                for (i=0; i<16; i++) {
                  fread (&palette[i*3+2], 1, 1, fp);
                  fread (&palette[i*3+1], 1, 1, fp);
                  fread (&palette[i*3+0], 1, 1, fp);
                  fread (&dummy, 1, 1, fp);
                }
                // Set the new palette?
                if (set_palette)
                  gxSetPalette (palette, NUM_INDEXED_COLORS);
              }
            }
          }
        }
        
        // Set file pointer to start of data
        fseek (fp, bmphdr->bitmap_data_offset, SEEK_SET);
        
        // Unpack file data
        if ((num_bits == 4) AND (bmphdr->encoding == ENCODING_NONE)) 
          Unpack_BMP_File4  (fp, buffer, buffer_size, palette, src_image, dst_image, bytes_per_line, pic_dx, pic_dy);
        else if ((num_bits == 4) AND (bmphdr->encoding == ENCODING_RUNLENGTH4))
          Unpack_BMP_File4_Encoded  (fp, buffer, buffer_size, palette, src_image, dst_image, pic_dx, pic_dy);
        else if ((num_bits == 8) AND (bmphdr->encoding == ENCODING_NONE))
          Unpack_BMP_File8  (fp, buffer, buffer_size, palette, src_image, dst_image, bytes_per_line, pic_dx, pic_dy);
        else if ((num_bits == 8) AND (bmphdr->encoding == ENCODING_RUNLENGTH8))
          Unpack_BMP_File8_Encoded  (fp, buffer, buffer_size, palette, src_image, dst_image, pic_dx, pic_dy);
        else if ((num_bits == 24) AND (bmphdr->encoding == ENCODING_NONE))
          Unpack_BMP_File24 (fp, buffer, buffer_size, src_image, dst_image, bytes_per_line, pic_dx, pic_dy);
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
| Function: Unpack_BMP_File4
|
| Inputs: Called from Draw_File()
| Outputs: Unpacks data from a 4-bit bmp file into an image and transfers
|       the image to the page one scanline at a time.  Uses a buffer to
|       speed file input.
|___________________________________________________________________*/

static void Unpack_BMP_File4 (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       bytes_per_line,
  int       pic_dx,             // dimensions of bmp image      
  int       pic_dy ) 
{
  int i, y, count, next, data_bytes;
  byte *src_image_data, *image16, *dst_image_data;

/*____________________________________________________________________
|
| Create a transfer buffer
|___________________________________________________________________*/

  data_bytes = (pic_dx+1)/2*2;

  // Create a 16-color planar image buffer
  image16 = (byte *) malloc (data_bytes);

/*____________________________________________________________________
|
| Unpack the file
|___________________________________________________________________*/

  if (image16) {
    // Init variables
    src_image_data = src_image + (2 * sizeof(unsigned));
    dst_image_data = dst_image + (2 * sizeof(unsigned));
    count = Get_BMP_Data (fp, buffer, buffer_size);
    next = 0;

    for (y=0; (y<pic_dy) AND count; y++) {
      // Unpack data from file into 16-color image
      Unpack_BMP_Line (fp, buffer, buffer_size, image16, bytes_per_line, data_bytes,
                       &count, &next);
      // Translate each packed 4-bit color pixel to an 8-bit color index value
      for (i=0; i<pic_dx; ) {
        src_image_data[i++] = (image16[i/2] >> 4) & 0xF;
        if (i < pic_dx)
          src_image_data[i++] = image16[i/2] & 0xF;
      }
      // Adjust scanline to match screen format
      if (Adjust_Scanline_Pixel_Format (src_image_data, dst_image_data, palette, 1, gx_Pixel_size, pic_dx))
        // Draw this line on screen
        gxDrawImage (dst_image, 0, pic_dy-1-y);
      else
        gxDrawImage (src_image, 0, pic_dy-1-y);
    }

    // Free 16-color image buffer
    free (image16);
  }
}

/*____________________________________________________________________
|
| Function: Unpack_BMP_File8
|
| Inputs: Called from Draw_File()
| Outputs: Unpacks data from a 8-bit bmp file into an image and transfers
|       the image to the page one scanline at a time.  Uses a buffer to
|       speed file input.
|___________________________________________________________________*/

static void Unpack_BMP_File8 (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       bytes_per_line,
  int       pic_dx,             // dimensions of bmp image      
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
  count = Get_BMP_Data (fp, buffer, buffer_size);
  next = 0;

  for (y=0; (y<pic_dy) AND count; y++) {
    // Unpack 1 line from the BMP file
    Unpack_BMP_Line (fp, buffer, buffer_size, src_image_data, bytes_per_line, pic_dx, &count, &next);
    // Adjust scanline to match screen format
    if (Adjust_Scanline_Pixel_Format (src_image_data, dst_image_data, palette, 1, gx_Pixel_size, pic_dx))
      // Draw this line on screen
      gxDrawImage (dst_image, 0, pic_dy-1-y);
    else
      gxDrawImage (src_image, 0, pic_dy-1-y);
  }
}

/*____________________________________________________________________
|
| Function: Unpack_BMP_File24
|
| Inputs: Called from Draw_File()
| Outputs: Unpacks data from a 24-bit bmp file into an image and transfers
|       the image to the page one scanline at a time.
|___________________________________________________________________*/

static void Unpack_BMP_File24 (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       bytes_per_line,
  int       pic_dx,             // dimensions of bmp image      
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
  count = Get_BMP_Data (fp, buffer, buffer_size);
  next = 0;

  for (y=0; (y<pic_dy) AND count; y++) {
    // Unpack 1 line from the BMP file
    Unpack_BMP_Line (fp, buffer, buffer_size, src_image_data, bytes_per_line, pic_dx*3, &count, &next);
    // Adjust scanline to match screen format
    if (Adjust_Scanline_Pixel_Format (src_image_data, dst_image_data, NULL, 3, gx_Pixel_size, pic_dx))
      // Draw this line on screen
      gxDrawImage (dst_image, 0, pic_dy-1-y);
    else
      gxDrawImage (src_image, 0, pic_dy-1-y);
  }
}

/*____________________________________________________________________
|
| Function: Unpack_BMP_Line
|
| Inputs: Called from Unpack_BMP_File8(), Unpack_BMP_File4(), 
|       Unpack_BMP_File24()
| Outputs: Unpacks 1 line of bmp data from file to image.
|___________________________________________________________________*/

// Need to read more data into file input buffer?
#define REFRESH_INPUT_BUFFER                            \
  if (*next >= *count) {                                \
    *count = Get_BMP_Data (fp, buffer, buffer_size);    \
    if (*count == 0) {                                  \
      gxError ("Error reading BMP line");               \
      break;                                            \
    }                                                   \
    *next = 0;                                          \
  }

static void Unpack_BMP_Line (
  FILE     *fp,
  byte     *buffer,
  unsigned  buffer_size,
  byte     *image_data,
  int       num_file_bytes,     // per row (could be > num_data_bytes)
  int       num_data_bytes,     // per row
  int      *count,
  int      *next )
{
  int n=0;
  byte c;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Zero the scanline
  memset (image_data, 0, num_file_bytes);

/*____________________________________________________________________
|
| Unpack a line with no encoding
|___________________________________________________________________*/

  while (n < num_file_bytes) {
    // Get a byte from file input buffer
    REFRESH_INPUT_BUFFER
    c = buffer[(*next)++];
    // Put in image?
    if (n < num_data_bytes)
      image_data[n] = c;
    n++;
  }
}

#undef REFRESH_INPUT_BUFFER

/*____________________________________________________________________
|
| Function: Get_BMP_Data
|
| Inputs: Called from ____
| Outputs: Reads data from bmp file into a buffer.
|___________________________________________________________________*/

static int Get_BMP_Data (FILE *fp, byte *buffer, unsigned buffer_size)
{
  return ((int)fread ((void *)buffer, 1, buffer_size, fp));
}

/*____________________________________________________________________
|
| Function: Unpack_BMP_File4_Encoded
|
| Inputs: Called from Draw_File()
| Outputs: Unpacks data from a 4-bit bmp file into an image and transfers
|       the image to the page one scanline at a time.  Uses a buffer to
|       speed file input.
|___________________________________________________________________*/

// Need to read more data into file input buffer?
#define REFRESH_INPUT_BUFFER                            \
  if (next >= count) {                                  \
    count = Get_BMP_Data (fp, buffer, buffer_size);     \
    if (count == 0) {                                   \
      gxError ("Error reading BMP line");               \
      end_of_bitmap = TRUE;                             \
      break;                                            \
    }                                                   \
    next = 0;                                           \
  }

static void Unpack_BMP_File4_Encoded (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o
  unsigned  buffer_size,        // size of buffer
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       pic_dx,             // dimensions of bmp image
  int       pic_dy )
{
  int i, n, x, y, count, next, end_of_line, end_of_bitmap;
  unsigned *p;
  byte b1, b2, *src_image_data, *dst_image_data, xmove, ymove;

/*____________________________________________________________________
|
| Unpack the file
|___________________________________________________________________*/

  p = (unsigned *)src_image;
  src_image_data = src_image + (2 * sizeof(unsigned));
  dst_image_data = dst_image + (2 * sizeof(unsigned));
  count = Get_BMP_Data (fp, buffer, buffer_size);
  next = 0;

  end_of_bitmap = FALSE;
  for (y=pic_dy-1,x=0; (y>=0) AND count AND (NOT end_of_bitmap); ) {
    n = 0;
    xmove = 0;
    ymove = 0;
    for (end_of_line=FALSE; (NOT end_of_line) AND (NOT end_of_bitmap); ) {
      // Get first two bytes and process them
      REFRESH_INPUT_BUFFER
      b1 = buffer[next++];
      REFRESH_INPUT_BUFFER
      b2 = buffer[next++];
      // Add a run of identical bytes to scanline?
      if (b1 != 0) {
        // Add bytes to scanline
        for (; b1 AND (n<(pic_dx-x)); ) {
          src_image_data[n++] = (b2 >> 4);
          b1--;
          if (b1 AND (n<(pic_dx-x))) {
            src_image_data[n++] = (b2 & 0xF);
            b1--;
          }
        }
      }
      // Add a run of unique bytes to scanline?
      else if ((b1 == 0) AND (b2 >= 3)) {
        i = b2;
        // Add bytes to scanline
        for (; b2 AND (n<(pic_dx-x)); ) {
          REFRESH_INPUT_BUFFER
          b1 = buffer[next++];
          src_image_data[n++] = (b1 >> 4);
          b2--;
          if (b2 AND (n<(pic_dx-x))) {
            src_image_data[n++] = (b1 & 0xF);
            b2--;
          }
        }
        // Skip a filler byte?
        if (((i+1)/2)%2) {
          REFRESH_INPUT_BUFFER
          b1 = buffer[next++];
        }
      }
      // End of line command?
      else if ((b1 == 0) AND (b2 == 0))
        end_of_line = TRUE;
      // End of bitmap reached?
      else if ((b1 == 0) AND (b2 == 1)) {
        end_of_line = TRUE;
        end_of_bitmap = TRUE;
      }
      // Move position command?
      else if ((b1 == 0) AND (b2 == 2)) {
        end_of_line = TRUE;
        REFRESH_INPUT_BUFFER
        xmove = buffer[next++];
        REFRESH_INPUT_BUFFER
        ymove = buffer[next++];
      }

      // Draw scanline?
      if (end_of_line) {
        *p = (unsigned)n;
        // Adjust scanline to match screen format
        if (Adjust_Scanline_Pixel_Format (src_image_data, dst_image_data, palette, 1, gx_Pixel_size, pic_dx))
          // Draw this line on screen
          gxDrawImage (dst_image, x, y);
        else
          gxDrawImage (src_image, x, y);
        x += n;
        y--;

        // Adjust bitmap position coordinates
        if (xmove)
          x += (int)xmove;
        else
          x = 0;
        if (ymove)
          y -= (ymove-1);
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: Unpack_BMP_File8_Encoded
|
| Inputs: Called from Draw_File()
| Outputs: Unpacks data from a 8-bit bmp file into an image and transfers
|       the image to the page one scanline at a time.  Uses a buffer to
|       speed file input.
|___________________________________________________________________*/

static void Unpack_BMP_File8_Encoded (
  FILE     *fp,
  byte     *buffer,             // buffer for file i/o          
  unsigned  buffer_size,        // size of buffer               
  byte     *palette,
  byte     *src_image,          // 1 scanline image             
  byte     *dst_image,          // 1 scanline image
  int       pic_dx,             // dimensions of bmp image      
  int       pic_dy )
{
  int i, n, x, y, count, next, end_of_line, end_of_bitmap;
  unsigned *p;
  byte b1, b2, *src_image_data, *dst_image_data, xmove, ymove;

/*____________________________________________________________________
|
| Unpack the file
|___________________________________________________________________*/

  p = (unsigned *)src_image;
  src_image_data = src_image + (2 * sizeof(unsigned));
  dst_image_data = dst_image + (2 * sizeof(unsigned));
  count = Get_BMP_Data (fp, buffer, buffer_size);
  next = 0;

  end_of_bitmap = FALSE;
  for (y=pic_dy-1,x=0; (y>=0) AND count AND (NOT end_of_bitmap); ) {
    n = 0;
    xmove = 0;
    ymove = 0;
    for (end_of_line=FALSE; (NOT end_of_line) AND (NOT end_of_bitmap); ) {
      // Get first two bytes and process them
      REFRESH_INPUT_BUFFER
      b1 = buffer[next++];
      REFRESH_INPUT_BUFFER
      b2 = buffer[next++];
      // Add a run of identical bytes to scanline?
      if (b1 != 0) {
        // Add bytes to scanline
        for (; b1 AND (n<(pic_dx-x)); b1--)
          src_image_data[n++] = b2;
      }
      // Add a run of unique bytes to scanline?
      else if ((b1 == 0) AND (b2 >= 3)) {
        i = b2;
        // Add bytes to scanline
        for (; b2 AND (n<(pic_dx-x)); b2--) {
          REFRESH_INPUT_BUFFER
          b1 = buffer[next++];
          src_image_data[n++] = b1;
        }
        // Skip a filler byte?
        if (i%2) {
          REFRESH_INPUT_BUFFER
          b1 = buffer[next++];
        }
      }
      // End of line command?
      else if ((b1 == 0) AND (b2 == 0))
        end_of_line = TRUE;
      // End of bitmap reached?
      else if ((b1 == 0) AND (b2 == 1)) {
        end_of_line = TRUE;
        end_of_bitmap = TRUE;
      }
      // Move position command?
      else if ((b1 == 0) AND (b2 == 2)) {
        end_of_line = TRUE;
        REFRESH_INPUT_BUFFER
        xmove = buffer[next++];
        REFRESH_INPUT_BUFFER
        ymove = buffer[next++];
      }

      // Draw scanline?
      if (end_of_line) {
        *p = (unsigned)n;
        // Adjust scanline to match screen format
        if (Adjust_Scanline_Pixel_Format (src_image_data, dst_image_data, palette, 1, gx_Pixel_size, pic_dx))
          // Draw this line on screen
          gxDrawImage (dst_image, x, y);
        else
          gxDrawImage (src_image, x, y);
        x += n;
        y--;

        // Adjust bitmap position coordinates
        if (xmove)
          x += (int)xmove;
        else
          x = 0;
        if (ymove)
          y -= (ymove-1);
      }
    }
  }
}

#undef REFRESH_INPUT_BUFFER

/*____________________________________________________________________
|
| Function: Adjust_Scanline_Pixel_Format
|
| Inputs: Called from Unpack_BMP_File4()
|                     Unpack_BMP_File8()
|                     Unpack_BMP_File24()
|                     Unpack_BMP_File4_Encoded()
|                     Unpack_BMP_File8_Encoded()
| Outputs: This function changes the pixel format of the image, if
|   necessary, to match the pixel format of the screen.  Assumes enough
|   extra space in image buffer to hold, possible expanded, new image
|   data.
|
|   Returns true if adjusted.
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
      pixel = ((unsigned)palette[src_image[i]*3]   >> (8 - gx_Video.num_redbits  ) << gx_Video.low_redbit  ) |
              ((unsigned)palette[src_image[i]*3+1] >> (8 - gx_Video.num_greenbits) << gx_Video.low_greenbit) |
              ((unsigned)palette[src_image[i]*3+2] >> (8 - gx_Video.num_bluebits ) << gx_Video.low_bluebit );
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
      pixel = ((unsigned)palette[src_image[i]*3]   << gx_Video.low_redbit  ) |
              ((unsigned)palette[src_image[i]*3+1] << gx_Video.low_greenbit) |
              ((unsigned)palette[src_image[i]*3+2] << gx_Video.low_bluebit );
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
      pixel = ((unsigned)palette[src_image[i]*3]   << gx_Video.low_redbit  ) |
              ((unsigned)palette[src_image[i]*3+1] << gx_Video.low_greenbit) |
              ((unsigned)palette[src_image[i]*3+2] << gx_Video.low_bluebit );
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
      dst_image[i*3+2] = (byte)((pixel & gx_Video.redmask)   >> gx_Video.low_redbit   << (8 - gx_Video.num_redbits));
      dst_image[i*3+1] = (byte)((pixel & gx_Video.greenmask) >> gx_Video.low_greenbit << (8 - gx_Video.num_greenbits));
      dst_image[i*3]   = (byte)((pixel & gx_Video.bluemask)  >> gx_Video.low_bluebit  << (8 - gx_Video.num_bluebits));
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
    byte rgb[3], *uniform_palette, *dst, *src;

    // Create a temporary uniform palette
    uniform_palette = (byte *) malloc (NUM_INDEXED_COLORS * 3 * sizeof(byte));
    if (uniform_palette) {
      // Get high spread uniform palette
      gxGetPalette (uniform_palette, NUM_INDEXED_COLORS);
      gxSetUniformPalette (uniform_palette, gxPALETTE_SPREAD_HIGH);

      // Init values for palette spread high
      entry = 40;
      div   = 5;
      n     = 6;

      // Process all pixels in scanline
      for (i=0, src=src_image, dst=dst_image; i<num_pixels; i++, src+=3, dst++) {
        // Get a 3-byte RGB value from scanline
        memcpy (rgb, src, 3);
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
        *dst = j;
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
      pixel = ((unsigned)src_image[i*3+2] >> (8 - gx_Video.num_redbits  ) << gx_Video.low_redbit  ) |
              ((unsigned)src_image[i*3+1] >> (8 - gx_Video.num_greenbits) << gx_Video.low_greenbit) |
              ((unsigned)src_image[i*3  ] >> (8 - gx_Video.num_bluebits ) << gx_Video.low_bluebit );
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
      pixel = ((unsigned)src_image[i*3+2] << gx_Video.low_redbit  ) |
              ((unsigned)src_image[i*3+1] << gx_Video.low_greenbit) |
              ((unsigned)src_image[i*3  ] << gx_Video.low_bluebit );
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
      pixel = ((unsigned)src_image[i*3+2] << gx_Video.low_redbit  ) |
              ((unsigned)src_image[i*3+1] << gx_Video.low_greenbit) |
              ((unsigned)src_image[i*3  ] << gx_Video.low_bluebit );
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
      dst_image[i*3+2] = pixel >> gx_Video.low_redbit;
      dst_image[i*3+1] = pixel >> gx_Video.low_greenbit;
      dst_image[i*3  ] = pixel >> gx_Video.low_bluebit;
    }
    adjusted = TRUE;
  }  

  return (adjusted);
}

/*____________________________________________________________________
|
| Function: gxWriteBMPFile
|
| Output: Captures image on active page saving it in a BMP file.  
|   Returns true if successful, else false on any error.
|
| Description: If current screen format is 8-bit color, will save in an
|   8-bit uncompressed BMP file.  If current screen format is 16,24, or 
|   32-bit color, will save in a 24-bit uncompressed BMP file.
|___________________________________________________________________*/

int gxWriteBMPFile (char *filename)
{
  int        i, image_size;
  FILE      *fp;
  BMPHeader *bmphdr;
  byte      *src_image = NULL;
  byte      *dst_image = NULL;
  byte      *palette, dummy;
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
| Allocate memory for a bmp header.
|___________________________________________________________________*/

  bmphdr = (BMPHeader *) calloc (1, sizeof(BMPHeader));
  if (bmphdr == NULL)
    ok = FALSE;

/*____________________________________________________________________
|
| Write BMP header to file.
| Finally, call a routine to pack the bmp file.
|___________________________________________________________________*/

  if (ok) {
    fp = fopen (filename, "wb");
    if (fp == NULL) {
      ok = FALSE;
      gxError ("Can't open BMP file.");
    }
    else {
      image_size = PAGE_WIDTH * PAGE_HEIGHT * gx_Pixel_size;

      // Create a bmp header for an 8-bit color file
      if (gx_Pixel_size == 1) {
        bmphdr->file_type[0]         = 'B';
        bmphdr->file_type[1]         = 'M';
        bmphdr->file_size            = sizeof(BMPHeader) + (NUM_INDEXED_COLORS*4) + image_size;
        bmphdr->reserved             = 0;
        bmphdr->bitmap_data_offset   = sizeof(BMPHeader) + (NUM_INDEXED_COLORS*4);
        bmphdr->bytes_bitmap_info    = 40;
        bmphdr->bitmap_width         = PAGE_WIDTH;
        bmphdr->bitmap_height        = PAGE_HEIGHT;
        bmphdr->color_planes         = 1;
        bmphdr->bits_per_pixel       = 8;
        bmphdr->encoding             = ENCODING_NONE;
        bmphdr->image_size           = image_size;
        bmphdr->hres                 = 1; // PAGE_WIDTH;
        bmphdr->vres                 = 1; // PAGE_HEIGHT;
        bmphdr->num_colors_used      = NUM_INDEXED_COLORS;
        bmphdr->num_colors_important = NUM_INDEXED_COLORS;
        // Write header to file
        fwrite ((void *)bmphdr, sizeof(BMPHeader), 1, fp);
        // Write palette to file
        palette = (byte *) malloc (NUM_INDEXED_COLORS * 3);
        if (palette) {
          gxGetPalette (palette, NUM_INDEXED_COLORS);
          for (i=0; i<NUM_INDEXED_COLORS; i++) {
            fwrite (&palette[i*3+2], 1, 1, fp);
            fwrite (&palette[i*3+1], 1, 1, fp);
            fwrite (&palette[i*3+0], 1, 1, fp);
            fwrite (&dummy, 1, 1, fp);
          }
          free (palette);
        }
      }
      // Create a bmp header for a 24-bit color file
      else {
        bmphdr->file_type[0]         = 'B';
        bmphdr->file_type[1]         = 'M';
        bmphdr->file_size            = sizeof(BMPHeader) + image_size;
        bmphdr->reserved             = 0;
        bmphdr->bitmap_data_offset   = sizeof(BMPHeader);
        bmphdr->bytes_bitmap_info    = 40;
        bmphdr->bitmap_width         = PAGE_WIDTH;
        bmphdr->bitmap_height        = PAGE_HEIGHT;
        bmphdr->color_planes         = 1;
        bmphdr->bits_per_pixel       = 24;
        bmphdr->encoding             = ENCODING_NONE;
        bmphdr->image_size           = image_size;
        bmphdr->hres                 = 1; // PAGE_WIDTH;
        bmphdr->vres                 = 1; // PAGE_HEIGHT;
        bmphdr->num_colors_used      = 0;
        bmphdr->num_colors_important = 0;
        // Write header to file
        fwrite ((void *)bmphdr, sizeof(BMPHeader), 1, fp);
      }

      // Pack the page data into the bmp file
      Pack_BMP_File (fp, src_image, dst_image, PAGE_HEIGHT);
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
  if (bmphdr)
    free (bmphdr);

  return (ok);
}

/*____________________________________________________________________
|
| Function: Pack_BMP_File
|
| Inputs: Called from gxWriteBMPFile()
| Outputs: Reads page data 1 line at a time, packing it into a bmp
|       file.
|___________________________________________________________________*/

static void Pack_BMP_File (
  FILE *fp,             // bmp file
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
    box.y = pic_dy-1-y;
    // Get a scanline
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
      Pack_BMP_Line (fp, dst_image_data, box.w);
    // Output line in 3-byte per pixel format
    else
      Pack_BMP_Line (fp, dst_image_data, box.w * 3);
  }
}

/*____________________________________________________________________
|
| Function: Pack_BMP_Line
|
| Inputs: Called from Pack_BMP_Line()
| Outputs: Packs a line of image data, writing it into a bmp file. Uses
|       no compression.
|___________________________________________________________________*/

static void Pack_BMP_Line (
  FILE *fp,             // bmp file                             
  byte *image_data,     // 1 scanline image                     
  int   width )
{
  byte dummy = 0;
//  int i;
//  int n;                // # bytes from image packed so far     

  fwrite (image_data, width, 1, fp);
  // Pad line to make 2-byte aligned?
  if (width % 2)
    fwrite (&dummy, 1, 1, fp);

//  for (n=0; n<width;) {
//    i = 1;
//    while ((image_data[n] == image_data[n+i]) AND ((n+i) < width) AND (i<63))
//      i++;
//    if (i > 1) {
//      fputc (i | 0xC0, fp);
//      fputc (image_data[n], fp);
//      n += i;
//    }
//    else {
//      if ((image_data[n] & (byte)0xC0) == (byte)0xC0)
//      fputc (0xC1, fp);
//      fputc (image_data[n++], fp);
//    }
//  }
}                         

/*____________________________________________________________________
|
| Function: gxGetBMPFileDimensions
|
| Outputs: Returns true if BMP file read successfully.  Also returns
|   width and height of BMP file image.  Returns FALSE on any error.
|___________________________________________________________________*/

int gxGetBMPFileDimensions (char *filename, int *width, int *height, int *bits_per_pixel)
{
  int        bytes_read;
  BMPHeader *bmphdr;
  FILE      *fp;

  int file_ok = FALSE;

  // Allocate memory for a BMP file header
  bmphdr = (BMPHeader *) malloc (sizeof(BMPHeader));
  if (bmphdr == NULL) 
    gxError ("gxGetBMPFileDimensions(): error allocating memory for header");
  else {
    // Open file for reading
    fp = fopen (filename, "rb");
    if (fp == NULL) 
      gxError ("gxGetBMPFileDimensions(): can't open BMP file");
    else {
      // Read in header
      bytes_read = (int)fread ((void *)bmphdr, 1, sizeof(BMPHeader), fp);
      if (bytes_read != sizeof(BMPHeader)) 
        gxError ("gxGetBMPFileDimensions(): couldn't read BMP header");
      else {
        // Is this a valid BMP file?
        if ((bmphdr->file_type[0]   != 'B') OR
            (bmphdr->file_type[1]   != 'M') OR
            (bmphdr->reserved       != 0)   OR
            (bmphdr->color_planes   != 1)) 
          gxError ("gxGetBMPFileDimensions(): bad BMP header found");
        else {
          // Get dimensions of image
          if (width)
            *width          = bmphdr->bitmap_width;
          if (height)
            *height         = bmphdr->bitmap_height;
          if (bits_per_pixel)
            *bits_per_pixel = bmphdr->bits_per_pixel;
          file_ok = TRUE;
        }
      }
      fclose (fp);
    }
    free (bmphdr);
  }

  return (file_ok);
}
