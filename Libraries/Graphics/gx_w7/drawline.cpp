/*____________________________________________________________________
|
| File: drawline.cpp
|
| Description: Functions to draw lines and points on the current page.
|
| Functions:    Draw_Styled_Line
|               Draw_Point
|               Draw_Pattern_Line
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

#include "drawline.h"

/*___________________
|
| Macros
|__________________*/

#define DRAW_CLIPPED_POINT(_x_,_y_)       \
  {                                       \
    if (gxClipPoint (_x_, _y_))           \
      (*gx_Video.draw_pixel) (_x_, _y_);  \
  }

#define DRAW_CLIPPED_LINE(_x1_,_y1_,_x2_,_y2_)  \
  {                                             \
    x1 = _x1_;                                  \
    y1 = _y1_;                                  \
    x2 = _x2_;                                  \
    y2 = _y2_;                                  \
    if (gxClipLine (&x1, &y1, &x2, &y2))        \
      (*gx_Video.draw_line) (x1, y1, x2, y2);   \
  }

#define DRAW_POINT(_x_,_y_)             \
  (*gx_Video.draw_pixel) (_x_, _y_);

#define DRAW_LINE(_x1_,_y1_,_x2_,_y2_)  \
  (*gx_Video.draw_line) (_x1_, _y1_, _x2_, _y2_);


/*____________________________________________________________________
|
| Function: Draw_Styled_Line
|
| Output: Draws a line on the current page.  The line can have a width
|       greater than 1 and/or a line style enabled.
|___________________________________________________________________*/

void Draw_Styled_Line (int x1, int y1, int x2, int y2)
{
  int i, error;
  int dx, dy, xinc, yinc, dx2, dy2, dxy;

  if (x2 < x1) {
    dx   = x1 - x2;
    xinc = -1;
  }
  else {
    dx   = x2 - x1;
    xinc = 1;
  }
  dx2 = dx * 2;

  if (y2 < y1) {
    dy   = y1 - y2;
    yinc = -1;
  }
  else {
    dy   = y2 - y1;
    yinc = 1;
  }
  dy2 = dy * 2;

/*____________________________________________________________________
|
| process horizontal line
|___________________________________________________________________*/

  if (dy == 0) {
    if (NOT gx_Line_style_enabled) {
      for (i=0; i<=dx; i++) {
        Draw_Point (x1, y1);
        x1 += xinc;
      }
    }
    else {
      for (i=0; i<=dx; i++) {
        /* Get first portion with data */
        while (gx_Line_style_count == 0) {
          gx_Line_style_index = (gx_Line_style_index+1) % NUM_STYLE_ELEMENTS;
          gx_Line_style_count = gx_Line_style[gx_Line_style_index];
        }
        /* If segment portion?*/
        if ((gx_Line_style_index & 1) == 0)
          Draw_Point (x1, y1);
        /* Decrement # of pixels drawn (or not drawn) */
        gx_Line_style_count--;
        /* Increment pixel address */
        x1 += xinc;
      }
    }
  }

/*____________________________________________________________________
|
| process vertical line
|___________________________________________________________________*/

  else if (dx == 0) {
    if (NOT gx_Line_style_enabled) {
      for (i=0; i<=dy; i++) {
        Draw_Point (x1, y1);
        y1 += yinc;
      }
    }
    else {
      for (i=0; i<=dy; i++) {
        while (gx_Line_style_count == 0) {
          gx_Line_style_index = (gx_Line_style_index+1) % NUM_STYLE_ELEMENTS;
          gx_Line_style_count = gx_Line_style[gx_Line_style_index];
        }
        if ((gx_Line_style_index & 1) == 0)
          Draw_Point (x1, y1);
        gx_Line_style_count--;
        y1 += yinc;
      }
    }
  }

/*____________________________________________________________________
|
| process diagonal line
|___________________________________________________________________*/

  else if (dx >= dy) {
    error = dy2 - dx;
    dxy   = dy2 - dx2;
    if (NOT gx_Line_style_enabled) {
      for (i=0; i<=dx; i++) {
        Draw_Point (x1, y1);
        x1 += xinc;
        if (error < 0)
          error += dy2;
        else {
          y1 += yinc;
          error += dxy;
        }
      }
    }
    else {
      for (i=0; i<=dx; i++) {
        while (gx_Line_style_count == 0) {
          gx_Line_style_index = (gx_Line_style_index+1) % NUM_STYLE_ELEMENTS;
          gx_Line_style_count = gx_Line_style[gx_Line_style_index];
        }
        if ((gx_Line_style_index & 1) == 0)
          Draw_Point (x1, y1);
        gx_Line_style_count--;
        x1 += xinc;
        if (error < 0)
          error += dy2;
        else {
          y1 += yinc;
          error += dxy;
        }
      }
    }
  }
  else {
    error = dx2 - dy;
    dxy   = dx2 - dy2;
    if (NOT gx_Line_style_enabled) {
      for (i=0; i<=dy; i++) {
        Draw_Point (x1, y1);
        y1 += yinc;
        if (error < 0)
          error += dx2;
        else {
          x1 += xinc;
          error += dxy;
        }
      }
    }
    else {
      for (i=0; i<=dy; i++) {
        while (gx_Line_style_count == 0) {
          gx_Line_style_index = (gx_Line_style_index+1) % NUM_STYLE_ELEMENTS;
          gx_Line_style_count = gx_Line_style[gx_Line_style_index];
        }
        if ((gx_Line_style_index & 1) == 0)
          Draw_Point (x1, y1);
        gx_Line_style_count--;
        y1 += yinc;
        if (error < 0)
          error += dx2;
        else {
          x1 += xinc;
          error += dxy;
        }
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: Draw_Point
|
| Output: Draws 1 point, clipped to current clipping rectangle.
|___________________________________________________________________*/

void Draw_Point (int x, int y)
{
  int x1, y1, x2, y2;

/*____________________________________________________________________
|
| Draw the point while clipping is active.
|___________________________________________________________________*/

  if (gx_Clipping) {
    switch (gx_Line_width) {
      case gxLINE_WIDTH_SQUARE_1:
        DRAW_CLIPPED_POINT (x, y)
        break;
      case gxLINE_WIDTH_SQUARE_2:
        DRAW_CLIPPED_POINT (x, y)
        DRAW_CLIPPED_POINT (x+1, y  )
        DRAW_CLIPPED_POINT (x,   y+1)
        DRAW_CLIPPED_POINT (x+1, y+1)
        break;
      case gxLINE_WIDTH_SQUARE_3:
        DRAW_CLIPPED_LINE (x-1, y-1, x+1, y-1)
        DRAW_CLIPPED_LINE (x-1, y,   x+1, y  )
        DRAW_CLIPPED_LINE (x-1, y+1, x+1, y+1)
        break;
      case gxLINE_WIDTH_SQUARE_4:
        DRAW_CLIPPED_LINE (x-1, y-1, x+2, y-1)
        DRAW_CLIPPED_LINE (x-1, y,   x+2, y  )
        DRAW_CLIPPED_LINE (x-1, y+1, x+2, y+1)
        DRAW_CLIPPED_LINE (x-1, y+2, x+2, y+2)
        break;
      case gxLINE_WIDTH_SQUARE_5:
        DRAW_CLIPPED_LINE (x-2, y-2, x+2, y-2)
        DRAW_CLIPPED_LINE (x-2, y-1, x+2, y-1)
        DRAW_CLIPPED_LINE (x-2, y,   x+2, y  )
        DRAW_CLIPPED_LINE (x-2, y+1, x+2, y+1)
        DRAW_CLIPPED_LINE (x-2, y+2, x+2, y+2)
        break;
      case gxLINE_WIDTH_SQUARE_6:
        DRAW_CLIPPED_LINE (x-2, y-2, x+3, y-2)
        DRAW_CLIPPED_LINE (x-2, y-1, x+3, y-1)
        DRAW_CLIPPED_LINE (x-2, y,   x+3, y  )
        DRAW_CLIPPED_LINE (x-2, y+1, x+3, y+1)
        DRAW_CLIPPED_LINE (x-2, y+2, x+3, y+2)
        DRAW_CLIPPED_LINE (x-2, y+3, x+3, y+3)
        break;
      case gxLINE_WIDTH_SQUARE_7:
        DRAW_CLIPPED_LINE (x-3, y-3, x+3, y-3)
        DRAW_CLIPPED_LINE (x-3, y-2, x+3, y-2)
        DRAW_CLIPPED_LINE (x-3, y-1, x+3, y-1)
        DRAW_CLIPPED_LINE (x-3, y,   x+3, y  )
        DRAW_CLIPPED_LINE (x-3, y+1, x+3, y+1)
        DRAW_CLIPPED_LINE (x-3, y+2, x+3, y+2)
        DRAW_CLIPPED_LINE (x-3, y+3, x+3, y+3)
        break;
      case gxLINE_WIDTH_CIRCLE_3:
        DRAW_CLIPPED_POINT (x,   y-1)
        DRAW_CLIPPED_POINT (x, y)
        DRAW_CLIPPED_POINT (x-1, y  )
        DRAW_CLIPPED_POINT (x+1, y  )
        DRAW_CLIPPED_POINT (x,   y+1)
        break;
      case gxLINE_WIDTH_CIRCLE_5:
        DRAW_CLIPPED_LINE (x-1, y-2, x+1, y-2)
        DRAW_CLIPPED_LINE (x-2, y-1, x+2, y-1)
        DRAW_CLIPPED_LINE (x-2, y,   x+2, y  )
        DRAW_CLIPPED_LINE (x-2, y+1, x+2, y+1)
        DRAW_CLIPPED_LINE (x-1, y+2, x+1, y+2)
        break;
      case gxLINE_WIDTH_CIRCLE_7:
        DRAW_CLIPPED_LINE (x-1, y-3, x+1, y-3)
        DRAW_CLIPPED_LINE (x-2, y-2, x+2, y-2)
        DRAW_CLIPPED_LINE (x-3, y-1, x+3, y-1)
        DRAW_CLIPPED_LINE (x-3, y,   x+3, y  )
        DRAW_CLIPPED_LINE (x-3, y+1, x+3, y+1)
        DRAW_CLIPPED_LINE (x-2, y+2, x+2, y+2)
        DRAW_CLIPPED_LINE (x-1, y+3, x+1, y+3)
        break;
      case gxLINE_WIDTH_CIRCLE_9:
        DRAW_CLIPPED_LINE (x-1, y-4, x+1, y-4)
        DRAW_CLIPPED_LINE (x-3, y-3, x+3, y-3)
        DRAW_CLIPPED_LINE (x-3, y-2, x+3, y-2)
        DRAW_CLIPPED_LINE (x-4, y-1, x+4, y-1)
        DRAW_CLIPPED_LINE (x-4, y,   x+4, y  )
        DRAW_CLIPPED_LINE (x-4, y+1, x+4, y+1)
        DRAW_CLIPPED_LINE (x-3, y+2, x+3, y+2)
        DRAW_CLIPPED_LINE (x-3, y+3, x+3, y+3)
        DRAW_CLIPPED_LINE (x-1, y+4, x+1, y+4)
        break;
      case gxLINE_WIDTH_CIRCLE_11:
        DRAW_CLIPPED_LINE (x-2, y-5, x+2, y-5)
        DRAW_CLIPPED_LINE (x-3, y-4, x+3, y-4)
        DRAW_CLIPPED_LINE (x-4, y-3, x+4, y-3)
        DRAW_CLIPPED_LINE (x-5, y-2, x+5, y-2)
        DRAW_CLIPPED_LINE (x-5, y-1, x+5, y-1)
        DRAW_CLIPPED_LINE (x-5, y,   x+5, y  )
        DRAW_CLIPPED_LINE (x-5, y+1, x+5, y+1)
        DRAW_CLIPPED_LINE (x-5, y+2, x+5, y+2)
        DRAW_CLIPPED_LINE (x-4, y+3, x+4, y+3)
        DRAW_CLIPPED_LINE (x-3, y+4, x+3, y+4)
        DRAW_CLIPPED_LINE (x-2, y+5, x+2, y+5)
        break;
      case gxLINE_WIDTH_VERTICAL_2:
        DRAW_CLIPPED_POINT (x, y)
        DRAW_CLIPPED_POINT (x, y+1)
        break;
      case gxLINE_WIDTH_VERTICAL_3:
        DRAW_CLIPPED_POINT (x, y)
        DRAW_CLIPPED_POINT (x, y-1)
        DRAW_CLIPPED_POINT (x, y+1)
        break;
      case gxLINE_WIDTH_HORIZONTAL_2:
        DRAW_CLIPPED_POINT (x,   y)
        DRAW_CLIPPED_POINT (x+1, y)
        break;
      case gxLINE_WIDTH_HORIZONTAL_3:
        DRAW_CLIPPED_POINT (x-1, y)
        DRAW_CLIPPED_POINT (x,   y)
        DRAW_CLIPPED_POINT (x+1, y)
        break;
      case gxLINE_WIDTH_SPRAY_3:
        DRAW_CLIPPED_POINT (x-2, y-2)
        DRAW_CLIPPED_POINT (x+2, y)
        DRAW_CLIPPED_POINT (x-1, y+2)
        break;
      case gxLINE_WIDTH_SPRAY_5:
        DRAW_CLIPPED_POINT (x,   y)
        DRAW_CLIPPED_POINT (x,   y-4)
        DRAW_CLIPPED_POINT (x-4, y)
        DRAW_CLIPPED_POINT (x+4, y-1)
        DRAW_CLIPPED_POINT (x+1, y+4)
        break;
    }
  }

/*____________________________________________________________________
|
| Draw the point with no clipping
|___________________________________________________________________*/

  else {
    switch (gx_Line_width) {
      case gxLINE_WIDTH_SQUARE_1:
        DRAW_POINT (x, y)
        break;
      case gxLINE_WIDTH_SQUARE_2:
        DRAW_POINT (x, y)
        DRAW_POINT (x+1, y  )
        DRAW_POINT (x,   y+1)
        DRAW_POINT (x+1, y+1)
        break;
      case gxLINE_WIDTH_SQUARE_3:
        DRAW_LINE (x-1, y-1, x+1, y-1)
        DRAW_LINE (x-1, y,   x+1, y  )
        DRAW_LINE (x-1, y+1, x+1, y+1)
        break;
      case gxLINE_WIDTH_SQUARE_4:
        DRAW_LINE (x-1, y-1, x+2, y-1)
        DRAW_LINE (x-1, y,   x+2, y  )
        DRAW_LINE (x-1, y+1, x+2, y+1)
        DRAW_LINE (x-1, y+2, x+2, y+2)
        break;
      case gxLINE_WIDTH_SQUARE_5:
        DRAW_LINE (x-2, y-2, x+2, y-2)
        DRAW_LINE (x-2, y-1, x+2, y-1)
        DRAW_LINE (x-2, y,   x+2, y  )
        DRAW_LINE (x-2, y+1, x+2, y+1)
        DRAW_LINE (x-2, y+2, x+2, y+2)
        break;
      case gxLINE_WIDTH_SQUARE_6:
        DRAW_LINE (x-2, y-2, x+3, y-2)
        DRAW_LINE (x-2, y-1, x+3, y-1)
        DRAW_LINE (x-2, y,   x+3, y  )
        DRAW_LINE (x-2, y+1, x+3, y+1)
        DRAW_LINE (x-2, y+2, x+3, y+2)
        DRAW_LINE (x-2, y+3, x+3, y+3)
        break;
      case gxLINE_WIDTH_SQUARE_7:
        DRAW_LINE (x-3, y-3, x+3, y-3)
        DRAW_LINE (x-3, y-2, x+3, y-2)
        DRAW_LINE (x-3, y-1, x+3, y-1)
        DRAW_LINE (x-3, y,   x+3, y  )
        DRAW_LINE (x-3, y+1, x+3, y+1)
        DRAW_LINE (x-3, y+2, x+3, y+2)
        DRAW_LINE (x-3, y+3, x+3, y+3)
        break;
      case gxLINE_WIDTH_CIRCLE_3:
        DRAW_POINT (x,   y-1)
        DRAW_POINT (x, y)
        DRAW_POINT (x-1, y  )
        DRAW_POINT (x+1, y  )
        DRAW_POINT (x,   y+1)
        break;
      case gxLINE_WIDTH_CIRCLE_5:
        DRAW_LINE (x-1, y-2, x+1, y-2)
        DRAW_LINE (x-2, y-1, x+2, y-1)
        DRAW_LINE (x-2, y,   x+2, y  )
        DRAW_LINE (x-2, y+1, x+2, y+1)
        DRAW_LINE (x-1, y+2, x+1, y+2)
        break;
      case gxLINE_WIDTH_CIRCLE_7:
        DRAW_LINE (x-1, y-3, x+1, y-3)
        DRAW_LINE (x-2, y-2, x+2, y-2)
        DRAW_LINE (x-3, y-1, x+3, y-1)
        DRAW_LINE (x-3, y,   x+3, y  )
        DRAW_LINE (x-3, y+1, x+3, y+1)
        DRAW_LINE (x-2, y+2, x+2, y+2)
        DRAW_LINE (x-1, y+3, x+1, y+3)
        break;
      case gxLINE_WIDTH_CIRCLE_9:
        DRAW_LINE (x-1, y-4, x+1, y-4)
        DRAW_LINE (x-3, y-3, x+3, y-3)
        DRAW_LINE (x-3, y-2, x+3, y-2)
        DRAW_LINE (x-4, y-1, x+4, y-1)
        DRAW_LINE (x-4, y,   x+4, y  )
        DRAW_LINE (x-4, y+1, x+4, y+1)
        DRAW_LINE (x-3, y+2, x+3, y+2)
        DRAW_LINE (x-3, y+3, x+3, y+3)
        DRAW_LINE (x-1, y+4, x+1, y+4)
        break;
      case gxLINE_WIDTH_CIRCLE_11:
        DRAW_LINE (x-2, y-5, x+2, y-5)
        DRAW_LINE (x-3, y-4, x+3, y-4)
        DRAW_LINE (x-4, y-3, x+4, y-3)
        DRAW_LINE (x-5, y-2, x+5, y-2)
        DRAW_LINE (x-5, y-1, x+5, y-1)
        DRAW_LINE (x-5, y,   x+5, y  )
        DRAW_LINE (x-5, y+1, x+5, y+1)
        DRAW_LINE (x-5, y+2, x+5, y+2)
        DRAW_LINE (x-4, y+3, x+4, y+3)
        DRAW_LINE (x-3, y+4, x+3, y+4)
        DRAW_LINE (x-2, y+5, x+2, y+5)
        break;
      case gxLINE_WIDTH_VERTICAL_2:
        DRAW_POINT (x, y)
        DRAW_POINT (x, y+1)
        break;
      case gxLINE_WIDTH_VERTICAL_3:
        DRAW_POINT (x, y)
        DRAW_POINT (x, y-1)
        DRAW_POINT (x, y+1)
        break;
      case gxLINE_WIDTH_HORIZONTAL_2:
        DRAW_POINT (x, y)
        DRAW_POINT (x+1, y)
        break;
      case gxLINE_WIDTH_HORIZONTAL_3:
        DRAW_POINT (x, y)
        DRAW_POINT (x-1, y)
        DRAW_POINT (x+1, y)
        break;
      case gxLINE_WIDTH_SPRAY_3:
        DRAW_POINT (x-2, y-2)
        DRAW_POINT (x+2, y)
        DRAW_POINT (x-1, y+2)
        break;
      case gxLINE_WIDTH_SPRAY_5:
        DRAW_POINT (x,   y)
        DRAW_POINT (x,   y-4)
        DRAW_POINT (x-4, y)
        DRAW_POINT (x+4, y-1)
        DRAW_POINT (x+1, y+4)
        break;
    }
  }
}

/*____________________________________________________________________
|
| Function: Draw_Pattern_Line
|
| Output: Draws a horizontal line on page using the current fill pattern.
|___________________________________________________________________*/

void Draw_Pattern_Line (int x1, int x2, int y)
{
  int x, dx, dy, tmp, transparent_background;
  gxColor color, fore_color, back_color, save_color;
  byte pixel, *data;
  static byte mask_bit [8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  if (x2 < x1) {
    tmp = x1;
    x1 = x2;
    x2 = tmp;
  }

  dx = gx_Pattern_list[gx_Fill_pattern].dx;
  dy = gx_Pattern_list[gx_Fill_pattern].dy;

/*____________________________________________________________________
|
| Draw a line using a bitmap pattern
|___________________________________________________________________*/

  if (gx_Pattern_list[gx_Fill_pattern].type == PATTERN_TYPE_BITMAP) {

    /* Save current color */
    save_color = gxGetColor ();

    /* Get colors to draw in */
    fore_color = gx_Pattern_list[gx_Fill_pattern].fore_color;
    back_color = gx_Pattern_list[gx_Fill_pattern].back_color;
    transparent_background = gx_Pattern_list[gx_Fill_pattern].transparent_background;

    /* Get address of pattern row */
    data = gx_Pattern_list[gx_Fill_pattern].data +
           ((y%dy) * gx_Pattern_list[gx_Fill_pattern].bytes_per_row);

    /* Draw the line */
    for (x=x1; x<=x2; x++) {
      pixel = data[(x%dx)/8] & mask_bit[(x%dx)%8];
      if (pixel) {
        gxSetColor (fore_color);
        (*gx_Video.draw_pixel) (x, y);
      }
      else if (NOT transparent_background) {
        gxSetColor (back_color);
        (*gx_Video.draw_pixel) (x, y);
      }
    }

    /* Restore current color */
    gxSetColor (save_color);
  }

/*____________________________________________________________________
|
| Draw a line using an image pattern
|___________________________________________________________________*/

  else {
    /* Get address of pattern row */
    data = gx_Pattern_list[gx_Fill_pattern].data +
           ((y%dy) * gx_Pattern_list[gx_Fill_pattern].bytes_per_row);

    /* Draw the line */
    for (x=x1; x<=x2; x++) {
      // Get color of pixel in image
      color = Get_Image_Pixel_Color (data, x%dx, 0, 0);
//      pixel = &data[x%dx];
      // Set the color
      gxSetColor (color);
      // Draw the new pixel
      (*gx_Video.draw_pixel) (x, y);
    }
  }
}
