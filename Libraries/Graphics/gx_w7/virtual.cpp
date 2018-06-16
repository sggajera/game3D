/*____________________________________________________________________
|
| File: virtual.cpp
|
| Description: Functions for virtual screen drawing.
|
| Functions:    virtual_Init                
|               virtual_DrawPixel
|                Convert_Pixel_To_Screen_Format           
|               virtual_GetPixel
|               virtual_DrawLine
|               virtual_DrawFillRectangle
|               virtual_PutImage
|               virtual_GetImage
|               virtual_PutBitmap
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
#include "img_clr.h"

#include "virtual.h"

/*___________________
|
| Function prototypes
|__________________*/

static unsigned Convert_Pixel_To_Screen_Format (gxColor color);

/*___________________
|
| Macros
|__________________*/

#define DRAW_POINT(_pix_,_val_)                                                 \
  {                                                                             \
    int i, tmp;                                                                 \
    float factor;                                                               \
    switch (gx_Logic_op) {                                                      \
      case gxSET: memcpy ((void *)(_pix_), (void *)&_val_, gx_Pixel_size);      \
                  break;                                                        \
      case gxAND: for (i=0; i<gx_Pixel_size; i++)                               \
                    (byte)(_pix_)[i] &= ((byte *)&_val_)[i];                    \
                  break;                                                        \
      case gxOR:  for (i=0; i<gx_Pixel_size; i++)                               \
                    (byte)(_pix_)[i] |= ((byte *)&_val_)[i];                    \
                  break;                                                        \
      case gxXOR: for (i=0; i<gx_Pixel_size; i++)                               \
                    (byte)(_pix_)[i] ^= ((byte *)&_val_)[i];                    \
                  break;                                                        \
      case gxADD: for (i=0; i<gx_Pixel_size; i++) {                             \
                    tmp = (int)((byte)(_pix_)[i]) + (int)(((byte *)&_val_)[i]); \
                    if (tmp > 255)                                              \
                      (byte)(_pix_)[i] = 255;                                   \
                    else                                                        \
                      (byte)(_pix_)[i] = (byte)tmp;                             \
                  }                                                             \
                  break;                                                        \
      case gxSUBTRACT:                                                          \
                  for (i=0; i<gx_Pixel_size; i++) {                             \
                    tmp = (int)((byte)(_pix_)[i]) + (int)(((byte *)&_val_)[i]); \
                    if (tmp < 0)                                                \
                      (byte)(_pix_)[i] = 0;                                     \
                    else                                                        \
                      (byte)(_pix_)[i] = (byte)tmp;                             \
                  }                                                             \
                  break;                                                        \
      case gxSHL:                                                               \
                  for (i=0; i<gx_Pixel_size; i++)                               \
                    (byte)(_pix_)[i] <<= ((byte *)&_val_)[i];                   \
                  break;                                                        \
      case gxSHR:                                                               \
                  for (i=0; i<gx_Pixel_size; i++)                               \
                    (byte)(_pix_)[i] >>= ((byte *)&_val_)[i];                   \
                  break;                                                        \
      case gxMULTIPLY:                                                          \
                  factor = (float)gx_Fore_color.r * (float)100;                 \
                  for (i=0; i<gx_Pixel_size; i++)                               \
                    (byte)(_pix_)[i] = (byte)((float)((byte)(_pix_)[i]) * factor);  \
                  break;                                                        \
    }                                                                           \
  }                                                                             

/*___________________
|
| Global variables
|__________________*/

// virtual page framebuffer
static byte     *Buffer;
// width of framebuffer in pixels (multiply by gx_Pixel_size to get width in bytes)
static unsigned  Buffer_dx;

/*____________________________________________________________________
|
| Function: virtual_Init
|
| Output: Initializes global variables needed by routines in this file.
|___________________________________________________________________*/

void virtual_Init (int page)
{
  Buffer    = gx_Page_list[page].buffer;
  Buffer_dx = gx_Page_list[page].width;
}

/*____________________________________________________________________
|
| Function: virtual_DrawPixel
|
| Output: Draws a pixel in current color using current logic operation.
|___________________________________________________________________*/

void virtual_DrawPixel (int x, int y)
{
  byte *pix;
  unsigned val;

  pix = Buffer + (y * Buffer_dx * gx_Pixel_size) + (x * gx_Pixel_size);

  val = Convert_Pixel_To_Screen_Format (gx_Fore_color);
  DRAW_POINT(pix, val)
}

/*____________________________________________________________________
|
| Function: Convert_Pixel_To_Screen_Format
|
| Input: Called from virtual_DrawPixel()
|                    virtual_DrawLine()
|                    virtual_PutBitmap()
| Output: If color is a 24-bit color, converts it to pixel format used 
|   by screen.
|___________________________________________________________________*/

static unsigned Convert_Pixel_To_Screen_Format (gxColor color)
{
  unsigned pixel;

/*____________________________________________________________________
|
| If 8-bit color, just return it
|___________________________________________________________________*/

  if (gx_Pixel_size == 1)
    pixel = color.index;

/*____________________________________________________________________
|
| Convert 24-bit color to 16-bit color
|___________________________________________________________________*/

  else if (gx_Pixel_size == 2) 
    pixel = ((unsigned)color.r >> (8 - gx_Video.num_redbits  ) << gx_Video.low_redbit  ) |
            ((unsigned)color.g >> (8 - gx_Video.num_greenbits) << gx_Video.low_greenbit) |
            ((unsigned)color.b >> (8 - gx_Video.num_bluebits ) << gx_Video.low_bluebit );

/*____________________________________________________________________
|
| Convert 24-bit color to 24-bit color
|___________________________________________________________________*/

  else if (gx_Pixel_size == 3) 
    pixel = ((unsigned)color.r << gx_Video.low_redbit  ) |
            ((unsigned)color.g << gx_Video.low_greenbit) |
            ((unsigned)color.b << gx_Video.low_bluebit );

/*____________________________________________________________________
|
| Convert 24-bit color to 32-bit color
|___________________________________________________________________*/

  else if (gx_Pixel_size == 4)
    pixel = ((unsigned)color.r << gx_Video.low_redbit  ) |
            ((unsigned)color.g << gx_Video.low_greenbit) |
            ((unsigned)color.b << gx_Video.low_bluebit );

  return (pixel);
}

/*____________________________________________________________________
|
| Function: virtual_GetPixel
|
| Output: Returns color of pixel at x,y on bitmap.
|___________________________________________________________________*/

void virtual_GetPixel (int x, int y, byte *r, byte *g, byte *b)
{
  gxColor color;

  // Get color of this pixel
  color = Get_Image_Pixel_Color (Buffer, x, y, Buffer_dx);
  *r = color.r;
  *g = color.g;
  *b = color.b;
}

/*____________________________________________________________________
|
| Function: virtual_DrawLine
|
| Output: Draws a 1-pixel wide line in current color using current
|       logic operation.
|___________________________________________________________________*/

void virtual_DrawLine (int x1, int y1, int x2, int y2)
{
  int i, error;
  int dx, dy, xinc, yinc, dx2, dy2, dxy;
  byte *pix;
  unsigned val;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  val = Convert_Pixel_To_Screen_Format (gx_Fore_color);

  if (x2 < x1) {
    dx   = x1 - x2;
    xinc = -1 * gx_Pixel_size;
  }
  else {
    dx   = x2 - x1;
    xinc = 1 * gx_Pixel_size;
  }
  dx2 = dx * 2;

  if (y2 < y1) {
    dy   = y1 - y2;
    yinc = -1 * Buffer_dx * gx_Pixel_size;
  }
  else {
    dy   = y2 - y1;
    yinc = 1 * Buffer_dx * gx_Pixel_size;
  }
  dy2 = dy * 2;

  pix = Buffer + (y1 * Buffer_dx * gx_Pixel_size) + (x1 * gx_Pixel_size);

/*____________________________________________________________________
|
| process horizontal line
|___________________________________________________________________*/

  if (dy == 0) {
    for (i=0; i<=dx; i++) {
      DRAW_POINT(pix, val)
      pix += xinc;
    }
  }

/*____________________________________________________________________
|
| process vertical line
|___________________________________________________________________*/

  else if (dx == 0) {
    for (i=0; i<=dy; i++) {
      DRAW_POINT(pix, val)
      pix += yinc;
    }
  }

/*____________________________________________________________________
|
| process diagonal line
|___________________________________________________________________*/

  else if (dx >= dy) {
    error = dy2 - dx;
    dxy   = dy2 - dx2;
    for (i=0; i<=dx; i++) {
      DRAW_POINT(pix, val)
      pix += xinc;
      if (error < 0)
        error += dy2;
      else {
        pix += yinc;
        error += dxy;
      }
    }
  }
  else {
    error = dx2 - dy;
    dxy   = dx2 - dy2;
    for (i=0; i<=dy; i++) {
      DRAW_POINT(pix, val)
      pix += yinc;
      if (error < 0)
        error += dx2;
      else {
        pix += xinc;
        error += dxy;
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: virtual_DrawFillRectangle
|
| Output: Draws a filled rectangle in current color using current logic
|       operation.
|___________________________________________________________________*/

#define ADD_BYTE(_b1_,_b2_)           \
  {                                   \
    tmp = (int)(_b1_) + (int)(_b2_);  \
    if (tmp > 255)                    \
      (_b1_) = 255;                   \
    else                              \
      (_b1_) = (byte)tmp;             \
  }
#define SUBTRACT_BYTE(_b1_,_b2_)      \
  {                                   \
    tmp = (int)(_b1_) - (int)(_b2_);  \
    if (tmp < 0)                      \
      (_b1_) = 0;                     \
    else                              \
      (_b1_) = (byte)tmp;             \
  }

void virtual_DrawFillRectangle (int x1, int y1, int x2, int y2)
{
  int x, y, dx, dy, i, tmp;
  byte *pix;
  unsigned image_color;
  float factor;

  // Convert current color to same format as image
  image_color = Convert_Pixel_To_Screen_Format (gx_Fore_color);

  // Init variables
  dx = x2 - x1 + 1;
  dy = y2 - y1 + 1;

  // Get address of first pixel
  pix = Buffer + (y1 * Buffer_dx * gx_Pixel_size) + (x1 * gx_Pixel_size);

  switch (gx_Logic_op) {
    case gxSET: for (y=0; y<dy; y++) {
									for (x=0; x<dx; x++)
										memcpy ((void *)(&pix[x*gx_Pixel_size]), (byte *)&image_color, gx_Pixel_size);
                  pix += (Buffer_dx * gx_Pixel_size);
                }
                break;
    case gxAND: for (y=0; y<dy; y++) {
                  for (x=0; x<dx; x++)
                    for (i=0; i<gx_Pixel_size; i++)
                      pix[x+i] &= ((byte *)&image_color)[i];
                  pix += (Buffer_dx * gx_Pixel_size);
                }
                break;
    case gxOR:  for (y=0; y<dy; y++) {
                  for (x=0; x<dx; x++)
                    for (i=0; i<gx_Pixel_size; i++)
                      pix[x+i] |= ((byte *)&image_color)[i];
                  pix += (Buffer_dx * gx_Pixel_size);
                }
                break;
    case gxXOR: for (y=0; y<dy; y++) {
                  for (x=0; x<dx; x++)
                    for (i=0; i<gx_Pixel_size; i++)
                      pix[x+i] ^= ((byte *)&image_color)[i];
                  pix += (Buffer_dx * gx_Pixel_size);
                }
                break;
    case gxADD: for (y=0; y<dy; y++) {
                  for (x=0; x<dx; x++)
                    for (i=0; i<gx_Pixel_size; i++)
                      ADD_BYTE (pix[x+i], ((byte *)&image_color)[i])
//                      pix[x+i] += ((byte *)&image_color)[i];
                  pix += (Buffer_dx * gx_Pixel_size);
                }
                break;
    case gxSUBTRACT: 
                for (y=0; y<dy; y++) {
                  for (x=0; x<dx; x++)
                    for (i=0; i<gx_Pixel_size; i++)
                      SUBTRACT_BYTE (pix[x+i], ((byte *)&image_color)[i])
//                      pix[x+i] += ((byte *)&image_color)[i];
                  pix += (Buffer_dx * gx_Pixel_size);
                }
                break;
    case gxSHL:
                for (y=0; y<dy; y++) {
                  for (x=0; x<dx; x++)
                    for (i=0; i<gx_Pixel_size; i++)
                      pix[x+i] <<= ((byte *)&image_color)[i];
                  pix += (Buffer_dx * gx_Pixel_size);
                }
                break;
    case gxSHR:
                for (y=0; y<dy; y++) {
                  for (x=0; x<dx; x++)
                    for (i=0; i<gx_Pixel_size; i++)
                      pix[x+i] >>= ((byte *)&image_color)[i];
                  pix += (Buffer_dx * gx_Pixel_size);
                }
                break;
    case gxMULTIPLY:
                factor = (float)gx_Fore_color.r * (float)100;             
                for (y=0; y<dy; y++) {
                  for (x=0; x<dx; x++)
                    for (i=0; i<gx_Pixel_size; i++)
                      pix[x+i] = (byte)((float)(pix[x+i]) * factor);
                  pix += (Buffer_dx * gx_Pixel_size);
                }
                break;
  }
}

#undef ADD_BYTE
#undef SUBTRACT_BYTE

/*____________________________________________________________________
|
| Function: virtual_PutImage
|
| Output: Copies image data from image to virtual buffer.
|___________________________________________________________________*/

void virtual_PutImage (
  byte *image,
  int   image_dx,
  int   image_dy,
  int   image_x,
  int   image_y,
  int   x,
  int   y,
  int   dx,
  int   dy,
  int   or_image )      // boolean
{
  int row, col; 
  unsigned bytes_per_row, image_incr, pix_incr;
  byte *pix;

  // Get address of first pixel in image
  image += ((image_y * image_dx * gx_Pixel_size) + (image_x * gx_Pixel_size));
  // Get address of first pixel
  pix = Buffer + (y * Buffer_dx * gx_Pixel_size) + (x * gx_Pixel_size);

  image_incr    = image_dx  * gx_Pixel_size;
  pix_incr      = Buffer_dx * gx_Pixel_size;
  bytes_per_row = dx * gx_Pixel_size;

  if (NOT or_image) {
    // Transfer the data
    for (row=0; row<dy; row++) {
      memcpy (pix, image, bytes_per_row);
      image += image_incr;
      pix   += pix_incr;
    }
  }
  else {
    // Transfer the data using OR operation
    for (row=0; row<dy; row++) {
      for (col=0; col<(int)bytes_per_row; col++)
        pix[col] |= image[col];
      image += image_incr;
      pix   += pix_incr;
    }
  }
}

/*____________________________________________________________________
|
| Function: virtual_GetImage
|
| Output: Copies image data from virtual buffer to callers image.
|___________________________________________________________________*/

void virtual_GetImage (
  byte *image,
  int   image_dx,
  int   image_dy,
  int   image_x,
  int   image_y,
  int   x,
  int   y,
  int   dx,
  int   dy )
{
  int row;
  byte *pix;
  unsigned bytes_per_row, image_incr, pix_incr;

  // Get address of first byte in image
  image += ((image_y * image_dx * gx_Pixel_size) + (image_x * gx_Pixel_size));
  // Get address of first pixel
  pix = Buffer + (y * Buffer_dx * gx_Pixel_size) + (x * gx_Pixel_size);

  image_incr    = image_dx  * gx_Pixel_size;
  pix_incr      = Buffer_dx * gx_Pixel_size;
  bytes_per_row = dx * gx_Pixel_size;

  // Transfer the data
  for (row=0; row<dy; row++) {
    memcpy (image, pix, bytes_per_row);
    image += image_incr;
    pix   += pix_incr;
  }
}

/*____________________________________________________________________
|
| Function: virtual_PutBitmap
|
| Output: Copies data from bitmap to virtual buffer.
|
| Description: Buffer consists of one bit per pixel.  Only '1' bits are
|       drawn to the virtual buffer in the color.  '0' bits are ignored 
|       and not drawn.
|___________________________________________________________________*/

void virtual_PutBitmap (
  byte *bitmap,
  int   bitmap_dx,
  int   bitmap_dy,
  int   bitmap_x,
  int   bitmap_y,
  int   x,
  int   y,
  int   dx,
  int   dy,
  byte  r,
  byte  g,
  byte  b )
{
  int i, row, col, n, bytes_per_row, shift_left, save_logic_op;
  unsigned buffer_incr, val;
  byte *pix;
  word mask;
  dword bits;
  gxColor save_color, color;

  save_color = gxGetColor ();
  save_logic_op = gxGetLogicOp ();
  gxSetLogicOp (gxSET);
  // Set color to draw in
  ZERO_COLOR (color);
  color.r = r;
  color.g = g;
  color.b = b;
  gxSetColor (color);

  // Compute # bytes per row in the bitmap
  bytes_per_row = (bitmap_dx+7)/8;
  // Compute shift left alignment 
  shift_left = bitmap_x % 8;

  // Get address of first byte in bitmap
  bitmap += (bitmap_y * bytes_per_row) + (bitmap_x / 8);
  // Get address of first pixel to write
  pix = Buffer + (y * Buffer_dx * gx_Pixel_size) + (x * gx_Pixel_size);

  // Get amount to incr buffer to next row down
  buffer_incr = Buffer_dx * gx_Pixel_size;
  
  // Get color to write
  val = Convert_Pixel_To_Screen_Format (gx_Fore_color);

  // Transfer the data
  if (shift_left == 0) {
    for (row=0; row<dy; row++) {
      n = 0;
      for (col=0; col<dx;) {
        // Get 16 bits (max) to process
				bits = (bitmap[n] << 8) | bitmap[n+1];
        mask = 0x8000;
        for (i=0; (i<16) AND (col<dx); col++,i++) {
          if (bits & mask)
            DRAW_POINT (&pix[col*gx_Pixel_size], val)
          mask = mask >> 1;
        }
        n += 2;
      }
      pix    += buffer_incr;
      bitmap += bytes_per_row;
    }
  }
  else {
    for (row=0; row<dy; row++) {
      n = 0;
      for (col=0; col<dx; ) {
        // Get 16 bits (max) to process
        bits = (bitmap[n] << 16) | (bitmap[n+1] << 8) | bitmap[n+2];
        bits = bits >> (8 - shift_left);
        mask = 0x8000;
        for (i=0; (i<16) AND (col<dx); col++,i++) {
          if (bits & mask)
            DRAW_POINT (&pix[col*gx_Pixel_size], val)
          mask = mask >> 1;
        }
        n += 2;
      }
      pix    += buffer_incr;
      bitmap += bytes_per_row;
    }
  }

  gxSetLogicOp (save_logic_op);
  gxSetColor (save_color);
}
