/*____________________________________________________________________
|
| File: image.cpp
|
| Description: A function to draw an image.
|
| Functions:    Draw_Image
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
#include "image.h"

/*____________________________________________________________________
|
| Function: Draw_Image
|
| Output: Draws an image int the current window on the active page.
|___________________________________________________________________*/

void Draw_Image (
  byte   *image,
  int     image_dx,
  int     image_dy,
  int     image_x,
  int     image_y,
  int     x,
  int     y,
  int     dx,
  int     dy )
{
  int xright, ybottom;
  int totally_clipped = FALSE;

/*____________________________________________________________________
|
| Adjust for current window.
|___________________________________________________________________*/

  x += gx_Window.xleft;
  y += gx_Window.ytop;

/*____________________________________________________________________
|
| If clipping is on, clip the image.
|___________________________________________________________________*/

  if (gx_Clipping) {

    /* Get coordinates of lower right corner of image */
    xright  = x + dx - 1;
    ybottom = y + dy - 1;

    /* Is image completely clipped? */
    if ((xright  < gx_Clip.xleft) OR (x > gx_Clip.xright) OR
        (ybottom < gx_Clip.ytop)  OR (y > gx_Clip.ybottom))
      totally_clipped = TRUE;
    else {
      /* Clip against BOTTOM edge of clip */
      if (ybottom > gx_Clip.ybottom)
        dy -= (ybottom - gx_Clip.ybottom);

      /* Clip against TOP edge of clip */
      if (y < gx_Clip.ytop) {
        image_y = gx_Clip.ytop - y;
        dy -= (gx_Clip.ytop - y);
        y = gx_Clip.ytop;
      }

      /* Clip against RIGHT edge of clip */
      if (xright > gx_Clip.xright)
        dx -= (xright - gx_Clip.xright);

      /* Clip against LEFT edge of clip */
      if (x < gx_Clip.xleft) {
        image_x = gx_Clip.xleft - x;
        dx -= (gx_Clip.xleft - x);
        x = gx_Clip.xleft;
      }
    }
  }

/*____________________________________________________________________
|
| Draw image.
|___________________________________________________________________*/

  // Make sure image is contained entirely within page
  DEBUG_ASSERT (x >= 0);
  DEBUG_ASSERT (x < PAGE_WIDTH);
  DEBUG_ASSERT (y >= 0);
  DEBUG_ASSERT (y < PAGE_HEIGHT);
  DEBUG_ASSERT (x+dx-1 <= PAGE_WIDTH-1);
  DEBUG_ASSERT (y+dy-1 <= PAGE_HEIGHT-1);

  if (NOT totally_clipped) 
    (*gx_Video.put_image)(image, image_dx, image_dy, image_x, image_y, x, y, dx, dy, 0);
}
