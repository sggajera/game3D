/*____________________________________________________________________
|
| File: fld_fill.cpp
|
| Description: Function for flood fill algorithm.
|
| Functions:    gxFloodFill
|                Identify_Spans
|                 Compute_Span
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

/*___________________
|
| Type definitions
|__________________*/

typedef struct span_node {
  short xleft;
  short xright;
  short y;
  struct span_node *next;
} SpanNode, *SpanNodePtr;

/*___________________
|
| Function Prototypes
|__________________*/

static void Identify_Spans (int xleft, int xright, int y);
static void Compute_Span (int x, int y, int *xleft, int *xright);

/*___________________
|
| Global variables
|__________________*/

static SpanNodePtr Span_list;
static gxRectangle Boundary;
static gxColor     Old_color;

/*____________________________________________________________________
|
| Function: gxFloodFill
|
| Output: Performs a flood fill in the current window with the current
|       color starting at a seed point.  The fill is bounded by the
|       window relative rectangle bounds.
|
| Description: This function fills with a solid color only and cannot
|       use the current fill pattern.
|___________________________________________________________________*/

void gxFloodFill (int seed_x, int seed_y, gxRectangle *bounds)
{
  int seed_ok, xleft, xright;
  gxRectangle clip;
  SpanNodePtr span;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (bounds);

/*____________________________________________________________________
|
| Make sure seed x,y is within bounds.
|___________________________________________________________________*/

  gxGetClip (&clip);
  gxSetClip (bounds);
  seed_ok = gxClipPoint (seed_x, seed_y);
  gxSetClip (&clip);

  if (NOT seed_ok)
    return;

/*____________________________________________________________________
|
| Initialize variables
|___________________________________________________________________*/

  // Adjust coords relative to window
  seed_x += gx_Window.xleft;
  seed_y += gx_Window.ytop;

  // Set globals
  Boundary.xleft   = bounds->xleft   + gx_Window.xleft;
  Boundary.ytop    = bounds->ytop    + gx_Window.ytop;
  Boundary.xright  = bounds->xright  + gx_Window.xleft;
  Boundary.ybottom = bounds->ybottom + gx_Window.ytop;

  Old_color = gxGetPixel (seed_x, seed_y);
  // If old color and fill color are this same, nothing to do!
  if (gx_Fore_color.index == Old_color.index)
    return;

  Span_list = NULL;

/*____________________________________________________________________
|
| Compute info about seed span and put into span list.
|___________________________________________________________________*/

  Compute_Span (seed_x, seed_y, &xleft, &xright);
  span = (SpanNodePtr) malloc (sizeof (SpanNode));
  if (span) {
    span->xleft  = xleft;
    span->xright = xright;
    span->y      = seed_y;
    span->next   = NULL;
    Span_list = span;
  }

/*____________________________________________________________________
|
| Main loop to process each span.
|___________________________________________________________________*/

  while (Span_list) {
    // Remove next entry from span list
    span = Span_list;
    Span_list = Span_list->next;

    // Fill the span line with the current color
    (*gx_Video.draw_line) (span->xleft, span->y, span->xright, span->y);

    // Add new spans to span list
    if ((span->y-1) >= Boundary.ytop)
      Identify_Spans (span->xleft, span->xright, span->y-1);
    if ((span->y+1) <= Boundary.ybottom)
      Identify_Spans (span->xleft, span->xright, span->y+1);

    // Free memory for this span node
    free (span);
  }
}

/*____________________________________________________________________
|
| Function: Identify_Spans
|
| Input: Called from gxFloodFill()
| Output: Given a y-coord and xleft, xright range, identifies spans
|       and puts them onto the front of the span list.
|___________________________________________________________________*/

static void Identify_Spans (int xleft, int xright, int y)
{
  int x, left, right;
  SpanNodePtr span;
  gxColor color;

  for (x=xleft; x <= xright;) {
    // Is this pixel a candidate for a new span?
    color = gxGetPixel (x, y);
    if (color.index == Old_color.index) {
      // Compute width of span
      Compute_Span (x, y, &left, &right);
      // Create a node and store info about this span
      span = (SpanNodePtr) malloc (sizeof (SpanNode));
      if (span) {
        span->xleft  = left;
        span->xright = right;
        span->y      = y;
        // Put this node into span list
        span->next = Span_list;
        Span_list = span;
      }
      // Move inspection to next candidate pixel (2 pixels right of this span)
      x = right + 1;
    }
    else
      x++;
  }
}

/*____________________________________________________________________
|
| Function: Compute_Span
|
| Input: Called from gxFloodFill(), Identify_Spans()
| Output: Given a seed x,y, computes the span of pixels on that scanline
|       that should be filled, keeping the span within the boundary.
|       Assumes the pixel at x,y contains Old_color.
|___________________________________________________________________*/

static void Compute_Span (int x, int y, int *xleft, int *xright)
{
  gxColor color;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (xleft);
  DEBUG_ASSERT (xright);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute left edge of seed span
  for (*xleft=x;;) {
    if (*xleft > Boundary.xleft) {
      color = gxGetPixel (*xleft-1, y);
      if (color.index == Old_color.index)
        *xleft -= 1;
      else
        break;
    }
    else
      break;
  }
  // Compute right edge of seed span
  for (*xright=x;;) {
    if (*xright < Boundary.xright) {
      color = gxGetPixel (*xright+1, y);
      if (color.index == Old_color.index)
        *xright += 1;
      else
        break;
    }
    else
      break;
  }
}
