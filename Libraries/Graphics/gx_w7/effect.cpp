/*____________________________________________________________________
|
| File: effect.cpp
|
| Description: Graphics functions for special effects.
|
| Functions:    gxDrawImageEffect
|               gxDrawSpriteEffect
|               gxDrawFillRectangleEffect
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|
| DEBUG_ASSERTED!
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include "dp.h"
#include "drawline.h"
#include "img_clr.h"

/*____________________________________________________________________
|
| Function: gxDrawImageEffect
|
| Input: Screen coordinates and an image buffer.
| Output: Draws an image in the current window on the active page with
|       the specified effect.
|___________________________________________________________________*/

void gxDrawImageEffect (byte *image, int x, int y, int effect)
{
  int dx, dy, xright, ybottom, save_x, save_y;
  int clip_x, clip_y, clip_dx, clip_dy;
  int xpix, ypix, num_pixels;
  unsigned *p;
  gxColor color, save_color;
  int totally_clipped = FALSE;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (image);

/*____________________________________________________________________
|
| Get image width, height, and other variables.
|___________________________________________________________________*/

  p = (unsigned *) image;
  dx = *p;
  dy = *(p+1);

  clip_x  = 0;
  clip_y  = 0;
  clip_dx = dx;
  clip_dy = dy;

  save_x = x;
  save_y = y;
  save_color = gxGetColor ();

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

    // Get coordinates of lower right corner of image
    ybottom = y + dy - 1;
    xright  = x + dx - 1;

    // Is image completely clipped?
    if ((xright  < gx_Clip.xleft) OR (x > gx_Clip.xright) OR
        (ybottom < gx_Clip.ytop)  OR (y > gx_Clip.ybottom))
      totally_clipped = TRUE;
    else {
      // Clip against BOTTOM edge of clip
      if (ybottom > gx_Clip.ybottom)
        clip_dy -= (ybottom - gx_Clip.ybottom);

      // Clip against TOP edge of clip
      if (y < gx_Clip.ytop) {
        clip_y = gx_Clip.ytop - y;
        clip_dy -= clip_y;
      }

      // Clip against RIGHT edge of clip
      if (xright > gx_Clip.xright)
        clip_dx -= (xright - gx_Clip.xright);

      // Clip against LEFT edge of clip
      if (x < gx_Clip.xleft) {
        clip_x = gx_Clip.xleft - x;
        clip_dx -= clip_x;
      }
    }
  }

/*____________________________________________________________________
|
| Make sure image is contanined within the page area
|___________________________________________________________________*/

  DEBUG_ASSERT (x >= 0);
  DEBUG_ASSERT (x < PAGE_WIDTH);
  DEBUG_ASSERT (y >= 0);
  DEBUG_ASSERT (y < PAGE_HEIGHT);
  DEBUG_ASSERT (x+clip_dx-1 <= PAGE_WIDTH-1);
  DEBUG_ASSERT (y+clip_dy-1 <= PAGE_HEIGHT-1);

/*____________________________________________________________________
|
| Draw the image with the effect.
|___________________________________________________________________*/

  if (NOT totally_clipped) {

    // Init variables
    image += (2 * sizeof(unsigned));

    if (effect == gxFADE_IN) {
      // Init random number generator
      random_Init ();

      // Randomly draw pixels 
      for (num_pixels=(int)(dx*dy*(float)2.5); num_pixels; num_pixels--) {
        xpix  = (int)(random_GetFloat() * (clip_dx-1) + clip_x);
        ypix  = (int)(random_GetFloat() * (clip_dy-1) + clip_y);
        // Get color of this pixel
        color = Get_Image_Pixel_Color (image, xpix, ypix, dx);
        // Set the color
        gxSetColor (color);
        // Draw the pixel
        (*gx_Video.draw_pixel)(x+xpix, y+ypix);
      }
      // Draw all remaining pixels
      (*gx_Video.put_image)(image, dx, dy, clip_x, clip_y,
                            x+clip_x, y+clip_y, clip_dx, clip_dy, 0);
    }
    else
      gxError ("Invalid effect input to gxDrawImageEffect()\n");
  }

  gxSetColor (save_color);
}

/*____________________________________________________________________
|
| Function: gxDrawSpriteEffect
|
| Input: Screen coordinates and a sprite buffer.
| Output: Draws a sprite in the current window on the active page with
|       the specified effect.
|___________________________________________________________________*/

void gxDrawSpriteEffect (byte *sprite, int x, int y, int effect)
{
  int dx, dy, xright, ybottom, save_x, save_y;
  int clip_x, clip_y, clip_dx, clip_dy;
  int xpix, ypix, num_pixels, bitmap_dx;
  unsigned *p;
  gxColor color, save_color;
  byte *image, *bitmap, *sprite_data;
  int totally_clipped = FALSE;

  static byte mask [8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sprite);

/*____________________________________________________________________
|
| Get sprite width, height, and other variables.
|___________________________________________________________________*/

  p = (unsigned *) sprite;
  dx = *p;
  dy = *(p+1);

  bitmap_dx = (dx+7)/8;

  clip_x  = 0;
  clip_y  = 0;
  clip_dx = dx;
  clip_dy = dy;

  save_x = x;
  save_y = y;
  save_color = gxGetColor ();

  sprite_data = sprite + (2 * sizeof(unsigned));

/*____________________________________________________________________
|
| Adjust for current window.
|___________________________________________________________________*/

  x += gx_Window.xleft;
  y += gx_Window.ytop;

/*____________________________________________________________________
|
| If clipping is on, clip the sprite
|___________________________________________________________________*/

  if (gx_Clipping) {

    /* Get coordinates of lower right corner of image */
    ybottom = y + dy - 1;
    xright  = x + dx - 1;

    /* Is image completely clipped? */
    if ((xright  < gx_Clip.xleft) OR (x > gx_Clip.xright) OR
        (ybottom < gx_Clip.ytop)  OR (y > gx_Clip.ybottom))
      totally_clipped = TRUE;
    else {
      /* Clip against BOTTOM edge of clip */
      if (ybottom > gx_Clip.ybottom)
        clip_dy -= (ybottom - gx_Clip.ybottom);

      /* Clip against TOP edge of clip */
      if (y < gx_Clip.ytop) {
        clip_y = gx_Clip.ytop - y;
        clip_dy -= clip_y;
      }

      /* Clip against RIGHT edge of clip */
      if (xright > gx_Clip.xright)
        clip_dx -= (xright - gx_Clip.xright);

      /* Clip against LEFT edge of clip */
      if (x < gx_Clip.xleft) {
        clip_x = gx_Clip.xleft - x;
        clip_dx -= clip_x;
      }
    }
  }

/*____________________________________________________________________
|
| Make sure sprite is contanined within the page area
|___________________________________________________________________*/

  DEBUG_ASSERT (x >= 0);
  DEBUG_ASSERT (x < PAGE_WIDTH);
  DEBUG_ASSERT (y >= 0);
  DEBUG_ASSERT (y < PAGE_HEIGHT);
  DEBUG_ASSERT (x+clip_dx-1 <= PAGE_WIDTH-1);
  DEBUG_ASSERT (y+clip_dy-1 <= PAGE_HEIGHT-1);

/*____________________________________________________________________
|
| Draw the sprite with the effect.
|___________________________________________________________________*/

  if (NOT totally_clipped) {

    /* Init variables */
    image  = sprite + 2 * sizeof(unsigned);
    bitmap = image + (dx * dy * sizeof(byte));

    if (effect == gxFADE_IN) {
      /* Init random number generator */
      random_Init ();

      /* Randomly draw pixels */
      for (num_pixels=(int)(dx*dy*(float)2.5); num_pixels; num_pixels--) {
        xpix  = (int)(random_GetFloat() * (clip_dx-1) + clip_x);
        ypix  = (int)(random_GetFloat() * (clip_dy-1) + clip_y);
        // Get color of this pixel
        color = Get_Image_Pixel_Color (image, xpix, ypix, dx);
        /* Draw an image pixel? */
        if (color.index != 0) {
          gxSetColor (color);
          (*gx_Video.draw_pixel)(x+xpix, y+ypix);
        }
        /* Draw a mask pixel? */
        else if (bitmap[(ypix*bitmap_dx)+(xpix/8)] & mask[xpix%8]) {
          gxSetColor (color); // Sets color to 0
          (*gx_Video.draw_pixel)(x+xpix, y+ypix);
        }
      }
      /* Draw all remaining pixels */
      (*gx_Video.put_bitmap)(bitmap, dx, dy, clip_x, clip_y,
                             x+clip_x, y+clip_y, clip_dx, clip_dy, 0, 0, 0);
      (*gx_Video.put_image)(image, dx, dy, clip_x, clip_y,
                            x+clip_x, y+clip_y, clip_dx, clip_dy, 1);
    }
    else
      gxError ("Invalid effect input to gxDrawSpriteEffect()\n");
  }

  gxSetColor (save_color);
}

/*____________________________________________________________________
|
| Function: gxDrawFillRectangleEffect
|
| Output: Draws a filled rectangle in current window, clipped to current
|       clipping rectangle using a special effect.
|___________________________________________________________________*/

void gxDrawFillRectangleEffect (int x1, int y1, int x2, int y2, int effect)
{
  int tmp, num_pixels, xpix, ypix, dx, dy;
  int save_x1, save_y1, save_x2, save_y2;
  int visible = TRUE;

  /* Save original coordinates */
  save_x1 = x1;
  save_y1 = y1;
  save_x2 = x2;
  save_y2 = y2;

  /* Switch points if needed */
  if (x1 > x2) {
    tmp = x1;
    x1 = x2;
    x2 = tmp;
  }
  if (y1 > y2) {
    tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  /* Adjust coords relative to window */
  x1 += gx_Window.xleft;
  y1 += gx_Window.ytop;
  x2 += gx_Window.xleft;
  y2 += gx_Window.ytop;

  /* Clip */
  if (gx_Clipping)
    if (NOT gxClipRectangle (&x1, &y1, &x2, &y2))
      visible = FALSE;

/*____________________________________________________________________
|
| Make sure sprite is contanined within the page area
|___________________________________________________________________*/

  DEBUG_ASSERT (x1 >= 0);
  DEBUG_ASSERT (x2 >= 0);
  DEBUG_ASSERT (x1 < PAGE_WIDTH);
  DEBUG_ASSERT (x2 < PAGE_WIDTH);
  DEBUG_ASSERT (y1 >= 0);
  DEBUG_ASSERT (y2 >= 0);
  DEBUG_ASSERT (y1 < PAGE_HEIGHT);
  DEBUG_ASSERT (y2 < PAGE_HEIGHT);

/*____________________________________________________________________
|
| Draw the rectangle with the effect
|___________________________________________________________________*/

  if (visible) {

    dx = x2 - x1 + 1;
    dy = y2 - y1 + 1;

    if (effect == gxFADE_IN) {
      for (num_pixels=(int)(dx*dy*(float)2.5); num_pixels; num_pixels--) {
        xpix = (int)(random_GetFloat() * (dx-1));
        ypix = (int)(random_GetFloat() * (dy-1));
        if (gx_Fill_pattern != gxPATTERN_SOLID)
          Draw_Pattern_Line (x1+xpix, x1+xpix, y1+ypix);
        else
          (*gx_Video.draw_pixel)(x1+xpix, y1+ypix);
      }
      /* draw all remaining pixels */
      gxDrawFillRectangle (save_x1, save_y1, save_x2, save_y2);
    }
    else
      gxError ("Invalid effect input to gxDrawFillRectangleEffect()\n");
  }
}
