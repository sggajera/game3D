/*____________________________________________________________________
|
| File: prim.cpp
|
| Description: Functions for graphics primitives.
|
| Functions:    gxDrawPixel
|               gxGetPixel
|               gxDrawLine
|               gxDrawRectangle
|               gxDrawFillRectangle
|               gxDrawPoly
|               gxDrawCircle
|               gxDrawFillCircle
|               gxDrawEllipse
|               gxDrawFillEllipse
|                Fill_Ellipse_Points
|               gxDrawArc
|                Points_In_Quad
|                Get_Points_In_Quad
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

#include "drawline.h"

/*___________________
|
| Function prototypes
|__________________*/

static void Fill_Ellipse_Points (int ctrx, int ctry, int x, int y);
static int  Points_In_Quad (int radius);
static void Get_Points_In_Quad (int radius, int *quad);

/*____________________________________________________________________
|
| Function: gxDrawPixel
|
| Output: Draws a pixel in current window, clipped to current clipping
|       rectangle, if clipping enabled.
|___________________________________________________________________*/

void gxDrawPixel (int x, int y)
{
  int visible = TRUE;

  // Adjust coords relative to window
  x += gx_Window.xleft;
  y += gx_Window.ytop;

  // Clip
  if (gx_Clipping)
    if (NOT gxClipPoint (x, y))
      visible = FALSE;

  // Draw
  if (visible)
    (*gx_Video.draw_pixel) (x, y);
}

/*____________________________________________________________________
|
| Function: gxGetPixel
|
| Output: Returns color of pixel at x,y on page.
|___________________________________________________________________*/

gxColor gxGetPixel (int x, int y)
{
  gxColor color;

  ZERO_COLOR (color);
  (*gx_Video.get_pixel)(x, y, &color.r, &color.g, &color.b);

  return (color);
}

/*____________________________________________________________________
|
| Function: gxDrawLine
|
| Output: Draws a line in current window, clipped to current clipping
|       rectangle, if clipping enabled.
|___________________________________________________________________*/

void gxDrawLine (int x1, int y1, int x2, int y2)
{
  int visible = TRUE;

  // Adjust coords relative to window
  x1 += gx_Window.xleft;
  y1 += gx_Window.ytop;
  x2 += gx_Window.xleft;
  y2 += gx_Window.ytop;

  // Clip
  if (gx_Clipping)
    if (NOT gxClipLine (&x1, &y1, &x2, &y2))
      visible = FALSE;

  // Draw
  if (visible) {
    if ((gx_Line_width != 1) OR gx_Line_style_enabled)
      Draw_Styled_Line (x1, y1, x2, y2);
    else
      (*gx_Video.draw_line) (x1, y1, x2, y2);
  }
}

/*____________________________________________________________________
|
| Function: gxDrawRectangle
|
| Output: Draws a wire-frame rectangle in current window, clipped to
|       current clipping rectangle.
|___________________________________________________________________*/

void gxDrawRectangle (int x1, int y1, int x2, int y2)
{
  gxDrawLine (x1, y1, x2, y1);
  gxDrawLine (x1, y2, x2, y2);
  gxDrawLine (x1, y1, x1, y2);
  gxDrawLine (x2, y1, x2, y2);
}

/*____________________________________________________________________
|
| Function: gxDrawFillRectangle
|
| Output: Draws a wire-frame rectangle in current window, clipped to
|       current clipping rectangle.
|___________________________________________________________________*/

void gxDrawFillRectangle (int x1, int y1, int x2, int y2)
{
  int tmp, y;
  int visible = TRUE;

  // Switch points if needed
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

  // Adjust coords relative to window
  x1 += gx_Window.xleft;
  y1 += gx_Window.ytop;
  x2 += gx_Window.xleft;
  y2 += gx_Window.ytop;

  // Clip
  if (gx_Clipping)
    if (NOT gxClipRectangle (&x1, &y1, &x2, &y2))
      visible = FALSE;

  // Draw
  if (visible) {
    if (gx_Fill_pattern != gxPATTERN_SOLID)
      for (y=y1; y<=y2; y++)
        Draw_Pattern_Line (x1, x2, y);
    else
      (*gx_Video.draw_fill_rectangle) (x1, y1, x2, y2);
  }
}

/*____________________________________________________________________
|
| Function: gxDrawPoly
|
| Output: Draws a wire-frame polygon in current window, clipped to
|       current clipping rectangle.
|___________________________________________________________________*/

void gxDrawPoly (int num_points, int *points)
{
  int i, j;

  for (i=0,j=0; i<num_points-1; i++,j+=2)
    gxDrawLine (points[j], points[j+1], points[j+2], points[j+3]);
  // Connect last point to first point
  gxDrawLine (points[j], points[j+1], points[0], points[1]);
}

/*____________________________________________________________________
|
| Function: gxDrawCircle
|
| Output: Draws a circle on current page in current drawing color in
|       current window.  This routine will clip either to the current
|       window, if set, or to the page.
|___________________________________________________________________*/

void gxDrawCircle (int ctrx, int ctry, int radius)
{
  int yradius;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (radius >= 1);

/*____________________________________________________________________
|
| Main routine
|___________________________________________________________________*/

  yradius = (int)((float)radius * gx_Aspect_ratio);
  if (yradius)
    gxDrawEllipse (ctrx, ctry, radius, yradius);
}

/*____________________________________________________________________
|
| Function: gxDrawFillCircle
|
| Output: Draws a filled circle in current window, clipped to current
|       clipping rectangle.
|___________________________________________________________________*/

void gxDrawFillCircle (int ctrx, int ctry, int radius)
{
  int yradius;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (radius >= 1);

/*____________________________________________________________________
|
| Main routine
|___________________________________________________________________*/

  yradius = (int)((float)radius * gx_Aspect_ratio);
  if (yradius)
    gxDrawFillEllipse (ctrx, ctry, radius, yradius);
}

/*____________________________________________________________________
|
| Function: gxDrawEllipse
|
| Output: Draws an ellipse on current page in current drawing color in
|       current window.
|
| Description: This function will draw according to the current line
|       width but not the line style.
|___________________________________________________________________*/

void gxDrawEllipse (int ctrx, int ctry, int xradius, int yradius)
{
#define ELLIPSE_POINTS(_cx,_cy)         \
  {                                     \
    Draw_Point (ctrx+_cx, ctry+_cy);    \
    Draw_Point (ctrx-_cx, ctry+_cy);    \
    Draw_Point (ctrx+_cx, ctry-_cy);    \
    Draw_Point (ctrx-_cx, ctry-_cy);    \
  }

  int x, y;
  long a,b,a_squared,two_a_squared,b_squared,two_b_squared,d,dx,dy;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (xradius >= 1);
  DEBUG_ASSERT (yradius >= 1);

/*____________________________________________________________________
|
| Main routine
|___________________________________________________________________*/

  // Adjust for current window
  ctrx += gx_Window.xleft;
  ctry += gx_Window.ytop;

  x = 0;
  y = yradius;
  a = xradius;
  b = yradius;
  a_squared = a * a;
  two_a_squared = 2 * a_squared;
  b_squared = b * b;
  two_b_squared = 2 * b_squared;
  d  = b_squared - a_squared * b + a_squared / 4;
  dx = 0;
  dy = two_a_squared * b;

  while (dx < dy) {
    ELLIPSE_POINTS (x, y)
    if (d > 0) {
      y--;
      dy -= two_a_squared;
      d -= dy;
    }
    x++;
    dx += two_b_squared;
    d += b_squared + dx;
  }
  d += (3 * (a_squared - b_squared) / 2 - (dx + dy)) / 2;
  while (y >= 0) {
    ELLIPSE_POINTS (x, y)
    if (d < 0) {
      x++;
      dx += two_b_squared;
      d += dx;
    }
    y--;
    dy -= two_a_squared;
    d += a_squared - dy;
  }
}

/*____________________________________________________________________
|
| Function: gxDrawFillEllipse
|
| Output: Draws a filled ellipse in current window, clipped to current
|       clipping rectangle.
|___________________________________________________________________*/

void gxDrawFillEllipse (int ctrx, int ctry, int xradius, int yradius)
{
  int x,y;
  long a,b,a_squared,two_a_squared,b_squared,two_b_squared,d,dx,dy;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (xradius >= 1);
  DEBUG_ASSERT (yradius >= 1);

/*____________________________________________________________________
|
| Main routine
|___________________________________________________________________*/

  // Adjust for current window
  ctrx += gx_Window.xleft;
  ctry += gx_Window.ytop;

  x = 0;
  y = yradius;
  a = xradius;
  b = yradius;
  a_squared = a * a;
  two_a_squared = 2 * a_squared;
  b_squared = b * b;
  two_b_squared = 2 * b_squared;
  d  = b_squared - a_squared * b + a_squared / 4;
  dx = 0;
  dy = two_a_squared * b;

  while (dx < dy) {
    Fill_Ellipse_Points (ctrx, ctry, x, y);
    if (d > 0) {
      y--;
      dy -= two_a_squared;
      d -= dy;
    }
    x++;
    dx += two_b_squared;
    d += b_squared + dx;
  }
  d += (3 * (a_squared - b_squared) / 2 - (dx + dy)) / 2;
  while (y >= 0) {
    Fill_Ellipse_Points (ctrx, ctry, x, y);
    if (d < 0) {
      x++;
      dx += two_b_squared;
      d += dx;
    }
    y--;
    dy -= two_a_squared;
    d += a_squared - dy;
  }
}

/*____________________________________________________________________
|
| Function: Fill_Ellipse_Points
|
| Input: Called from gxDrawFillEllipse()
| Output: Draws a filled ellipse in current window, clipped to current
|       clipping rectangle.
|___________________________________________________________________*/

static void Fill_Ellipse_Points (int ctrx, int ctry, int x, int y)
{
  int x1, y1, x2, y2, visible;

  x1 = ctrx - x;
  y1 = ctry - y;
  x2 = ctrx + x;
  y2 = ctry - y;
  visible = TRUE;
  if (gx_Clipping)
    if (NOT gxClipLine (&x1, &y1, &x2, &y2))
      visible = FALSE;
  if (visible) {
    if (gx_Fill_pattern != gxPATTERN_SOLID)
      Draw_Pattern_Line (x1, x2, y1);
    else
      (*gx_Video.draw_line) (x1, y1, x2, y2);
  }
  x1 = ctrx - x;
  y1 = ctry + y;
  x2 = ctrx + x;
  y2 = ctry + y;
  visible = TRUE;
  if (gx_Clipping)
    if (NOT gxClipLine (&x1, &y1, &x2, &y2))
      visible = FALSE;
  if (visible) {
    if (gx_Fill_pattern != gxPATTERN_SOLID)
      Draw_Pattern_Line (x1, x2, y2);
    else
      (*gx_Video.draw_line) (x1, y1, x2, y2);
  }
}

/*____________________________________________________________________
|
| Function: gxDrawArc
|
| Output: Draws an arc in current window, clipped to current clipping
|       rectangle.  Draws clockwise from start angle to end angle.
|___________________________________________________________________*/

void gxDrawArc (int ctrx, int ctry, int radius, int start_angle, int end_angle)
{
#define START   0x1     /* arc starts in this quadrant          */
#define END     0x2     /* arc ends in this quadrant            */
#define FULL    0x4     /* arc fully encompasses this quadrant  */
#define PARTIAL 0x3     /* arc starts and ends in this quadrant */

  int i, j, n, *quad, q[4];
  int start_quad, end_quad, s, e;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (radius >= 1);

/*____________________________________________________________________
|
| Init variables, data
|___________________________________________________________________*/

  // Adjust for current window
  ctrx += gx_Window.xleft;
  ctry += gx_Window.ytop;

  // Compute # pixels in 1 quad of circle
  n = Points_In_Quad (radius);

  // Create an array for these pixel coordinates
  quad = (int *) malloc (n * 2 * sizeof(int));
  if (quad) {
    // Fill array with x,y pixel values in this quadrant
    Get_Points_In_Quad (radius, quad);

/*____________________________________________________________________
|
| Keep angles within 0-360
|___________________________________________________________________*/

    while (start_angle < 0)
      start_angle += 360;
    while (start_angle > 360)
      start_angle -= 360;

    while (end_angle < 0)
      end_angle += 360;
    while (end_angle > 360)
      end_angle -= 360;

/*____________________________________________________________________
|
| Determine which quads will have pixels
|___________________________________________________________________*/

    if (start_angle < 90)
      start_quad = 0;
    else if (start_angle < 180)
      start_quad = 1;
    else if (start_angle < 270)
      start_quad = 2;
    else
      start_quad = 3;

    if (end_angle < 90)
      end_quad = 0;
    else if (end_angle < 180)
      end_quad = 1;
    else if (end_angle < 270)
      end_quad = 2;
    else
      end_quad = 3;

    for (i=0; i<4; i++)
      q[i] = FALSE;
    if (end_quad < start_quad)
      end_quad += 4;
    for (i=start_quad; i <= end_quad; i++) {
      if (i == start_quad)
        q[i] |= START;
      else if (i == end_quad)
        q[i%4] |= END;
      else
        q[i%4] = FULL;
    }

/*____________________________________________________________________
|
| Draw any pixels in quad 0 (0-90 degrees)
|___________________________________________________________________*/

    if (q[0]) {
      if (q[0] == FULL) {
        s = 0;
        e = n;
      }
      else if (q[0] == START) {
        s = (int)((float)start_angle/(float)90.0 * (float)n);
        e = n;
      }
      else if (q[0] == END) {
        s = 0;
        e = (int)((float)end_angle/(float)90.0 * (float)n);
      }
      else if (q[0] == PARTIAL) {
        s = (int)((float)start_angle/(float)90.0 * (float)n);
        e = (int)((float)end_angle/(float)90.0 * (float)n);
      }
      for (j=s*2; s<e; s++,j+=2)
        Draw_Point (ctrx+quad[j], ctry+quad[j+1]);
    }

/*____________________________________________________________________
|
| Draw any pixels in quad 1 (90-180 degrees)
|___________________________________________________________________*/

    if (q[1]) {
      if (q[1] == FULL) {
        s = 0;
        e = n;
      }
      else if (q[1] == START) {
        s = (int)((float)(start_angle-90)/(float)90.0 * (float)n);
        e = n;
      }
      else if (q[1] == END) {
        s = 0;
        e = (int)((float)(end_angle-90)/(float)90.0 * (float)n);
      }
      else if (q[1] == PARTIAL) {
        s = (int)((float)(start_angle-90)/(float)90.0 * (float)n);
        e = (int)((float)(end_angle-90)/(float)90.0 * (float)n);
      }
      for (j=s*2; s<e; s++,j+=2)
        Draw_Point (ctrx-quad[j+1], ctry+quad[j]);
    }

/*____________________________________________________________________
|
| Draw any pixels in quad 2 (180-270 degrees)
|___________________________________________________________________*/

    if (q[2]) {
      if (q[2] == FULL) {
        s = 0;
        e = n;
      }
      else if (q[2] == START) {
        s = (int)((float)(start_angle-180)/(float)90.0 * (float)n);
        e = n;
      }
      else if (q[2] == END) {
        s = 0;
        e = (int)((float)(end_angle-180)/(float)90.0 * (float)n);
      }
      else if (q[2] == PARTIAL) {
        s = (int)((float)(start_angle-180)/(float)90.0 * (float)n);
        e = (int)((float)(end_angle-180)/(float)90.0 * (float)n);
      }
      for (j=s*2; s<e; s++,j+=2)
        Draw_Point (ctrx-quad[j], ctry-quad[j+1]);
    }

/*____________________________________________________________________
|
| Draw any pixels in quad 3 (270-360 degrees).
|___________________________________________________________________*/

    if (q[3]) {
      if (q[3] == FULL) {
        s = 0;
        e = n;
      }
      else if (q[3] == START) {
        s = (int)((float)(start_angle-270)/(float)90.0 * (float)n);
        e = n;
      }
      else if (q[3] == END) {
        s = 0;
        e = (int)((float)(end_angle-270)/(float)90.0 * (float)n);
      }
      else if (q[3] == PARTIAL) {
        s = (int)((float)(start_angle-270)/(float)90.0 * (float)n);
        e = (int)((float)(end_angle-270)/(float)90.0 * (float)n);
      }
      for (j=s*2; s<e; s++,j+=2)
        Draw_Point (ctrx+quad[j+1], ctry-quad[j]);
    }

    free (quad);
  }

#undef START
#undef END
#undef FULL
#undef PARTIAL
}

/*____________________________________________________________________
|
| Function: Points_In_Quad
|
| Input: Called from gxDrawArc()
| Output: Returns the number of pixels in 1 quadrant of a circle with
|       this radius.
|___________________________________________________________________*/

static int Points_In_Quad (int radius)
{
  int x, y, r_squared, two_r_squared, d, dx, dy;
  int n = 0;

  x = 0;
  y = -radius;
  r_squared = radius * radius;
  two_r_squared = 2 * r_squared;
  d  = r_squared - (r_squared * radius) + r_squared / 4;
  dx = 0;
  dy = two_r_squared * radius;

  while (dx < dy) {
    n++;
    if (d > 0) {
      y++;
      dy -= two_r_squared;
      d  -= dy;
    }
    x++;
    dx += two_r_squared;
    d  += r_squared + dx;
  }
  d -= (dx + dy)/2;
  while (y < 0) {
    n++;
    if (d < 0) {
      x++;
      dx += two_r_squared;
      d  += dx;
    }
    y++;
    dy -= two_r_squared;
    d  += r_squared - dy;
  }

  return (n);
}

/*____________________________________________________________________
|
| Function: Get_Points_In_Quad
|
| Input: Called from gxDrawArc()
| Output: Fills input array with pixel coordinates for all pixels in
|       upper right quadrant of a circle with this radius.  Assumes
|       center of circle is at 0,0.
|___________________________________________________________________*/

static void Get_Points_In_Quad (int radius, int *quad)
{
  int x, y, r_squared, two_r_squared, d, dx, dy;
  int n = 0;

  x = 0;
  y = -radius;
  r_squared = radius * radius;
  two_r_squared = 2 * r_squared;
  d  = r_squared - (r_squared * radius) + r_squared / 4;
  dx = 0;
  dy = two_r_squared * radius;

  while (dx < dy) {
    quad[n++] = x;
    quad[n++] = y;
    if (d > 0) {
      y++;
      dy -= two_r_squared;
      d  -= dy;
    }
    x++;
    dx += two_r_squared;
    d  += r_squared + dx;
  }
  d -= (dx + dy)/2;
  while (y < 0) {
    quad[n++] = x;
    quad[n++] = y;
    if (d < 0) {
      x++;
      dx += two_r_squared;
      d  += dx;
    }
    y++;
    dy -= two_r_squared;
    d  += r_squared - dy;
  }
}
