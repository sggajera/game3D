/*____________________________________________________________________
|
| File: img_clr.cpp
|
| Description: Functions that operate on images.
|
| Functions:  Put_Image_Pixel_Color
|             Get_Image_Pixel_Color    
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

/*____________________________________________________________________
|
| Function: Put_Image_Pixel_Color
|
| Output: Draws a pixel in an image (or the color index if using 
|   indexed-color mode).
|___________________________________________________________________*/

void Put_Image_Pixel_Color (byte *image_data, int x, int y, int dx, gxColor color)
{
  byte     *pixel;
  unsigned  raw_color;

  // Get address of first byte of this pixel
  pixel = image_data + (y * dx * gx_Pixel_size) + (x * gx_Pixel_size);

  if (gx_Pixel_size == 1)
    *pixel = color.index;
  else {
    raw_color = 0;
    raw_color |= (unsigned)(color.r) << gx_Video.low_redbit;
    raw_color |= (unsigned)(color.g) << gx_Video.low_greenbit;
    raw_color |= (unsigned)(color.b) << gx_Video.low_bluebit;
    memcpy (pixel, (void *)&raw_color, gx_Pixel_size);
  }
}

/*____________________________________________________________________
|
| Function: Get_Image_Pixel_Color
|
| Output: Returns the color of an image pixel (or the color index if
|       using indexed-color mode).
|___________________________________________________________________*/

gxColor Get_Image_Pixel_Color (byte *image_data, int x, int y, int dx)
{
  byte     *pixel;
  unsigned  raw_color;
  gxColor   color;

  // Get address of first byte of this pixel
  pixel = image_data + (y * dx * gx_Pixel_size) + (x * gx_Pixel_size);

  if (gx_Pixel_size == 1)
    color.index = (unsigned)(*pixel);
  else {
    if (gx_Pixel_size == 2)
      // Get the 2 bytes at this address
      raw_color = (unsigned) (*((word *)pixel));
    else if (gx_Pixel_size == 3)
      // Get the 3 bytes at this address
      memcpy (&raw_color, pixel, 3);
    else if (gx_Pixel_size == 4)
      // Get the 4 bytes at this address
      raw_color = *((unsigned *)pixel);
  
    color.r = (byte)((raw_color & gx_Video.redmask)   >> gx_Video.low_redbit);
    color.g = (byte)((raw_color & gx_Video.greenmask) >> gx_Video.low_greenbit);
    color.b = (byte)((raw_color & gx_Video.bluemask)  >> gx_Video.low_bluebit);
    color.a = 0;
  }

  return (color);
}
