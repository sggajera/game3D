/*____________________________________________________________________
|
| File: clippoly.cpp
|
| Description: A routine for polygon clipping using the Sutherland-
|       Hodgman algorithm.  Adapted from "Computer Graphics: Principles
|       and Practice", pg. 126.
|
| Functions:    Clip_Polygon
|                Clip_Polygon_Left
|                Clip_Polygon_Right
|                Clip_Polygon_Top
|                Clip_Polygon_Bottom
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

/*___________________
|
| Type definitions
|__________________*/

struct Point {
  int x, y;
};

/*___________________
|
| Function Prototypes
|__________________*/

static void Clip_Polygon_Left (
  Point *inpoints,
  Point *outpoints,
  int    num_in );
static void Clip_Polygon_Right (
  Point *inpoints,
  Point *outpoints,
  int    num_in );
static void Clip_Polygon_Top (
  Point *inpoints,
  Point *outpoints,
  int    num_in );
static void Clip_Polygon_Bottom (
  Point *inpoints,
  Point *outpoints,
  int    num_in );

/*___________________
|
| Global variables
|__________________*/

static int Num_out;

/*____________________________________________________________________
|
| Function: Clip_Polygon
|
| Output: Fills outpoints array with clipped polygon vertices.
|       outputs array should be twice as large as inpoints array.
|       Returns true if any part of the polygon is visible inside the
|       current clipping window.
|___________________________________________________________________*/

int Clip_Polygon (
  int *inpoints,        // input vertices
  int *outpoints,       // output vertices
  int  num_in,          // # input vertices
  int *num_out )        // # output vertices
{
  unsigned size;
  Point *outpoints1, *outpoints2, *outpoints3, *outpoints4;
  int visible = FALSE;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (inpoints);
  DEBUG_ASSERT (outpoints);
  DEBUG_ASSERT (num_in > 0);
  DEBUG_ASSERT (num_out);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create temporary array
  size = num_in * 2;
  outpoints1 = new Point [4 * size];

  if (outpoints1) {
    /* Init variables */
    outpoints2 = &outpoints1[size*1];
    outpoints3 = &outpoints1[size*2];
    outpoints4 = &outpoints1[size*3];

    // Clip the polygon
    Clip_Polygon_Left ((Point *)inpoints, outpoints1, num_in);
    if (Num_out >= 3)
      Clip_Polygon_Right  (outpoints1, outpoints2, Num_out);
    if (Num_out >= 3)
      Clip_Polygon_Top    (outpoints2, outpoints3, Num_out);
    if (Num_out >= 3)
      Clip_Polygon_Bottom (outpoints3, outpoints4, Num_out);
    // Is polygon still visible?
    if (Num_out >= 3) {
      *num_out = Num_out;
      memcpy (outpoints, outpoints4, Num_out * sizeof(Point));
      visible = TRUE;
    }

    // Free memory used
    delete [] outpoints1;
  }

  return (visible);
}

/*____________________________________________________________________
|
| Function: Clip_Polygon_Left
|
| Input: Called from Clip_Polygon()
| Output: Fills outpoints array with clipped polygon vertices.
|       outputs array should be twice as large as inpoints array. This
|       function clips the polygon against one clipping edge.
|___________________________________________________________________*/

static void Clip_Polygon_Left (
  Point *inpoints,
  Point *outpoints,
  int      num_in )
{
  int i;
  Point s, p, newp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (inpoints);
  DEBUG_ASSERT (outpoints);
  DEBUG_ASSERT (num_in > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Init variables
  Num_out = 0;
  // Start with last vertex in input array
  s = inpoints[num_in-1];

  for (i=0; i<num_in; i++) {
    p = inpoints[i];
    if ((p.x - gx_Clip.xleft) >= 0) {
      if ((s.x - gx_Clip.xleft) >= 0)
        outpoints[Num_out++] = p;
      else {
        newp.x = gx_Clip.xleft;
        newp.y = p.y + (s.y-p.y) * (gx_Clip.xleft-p.x) / (s.x-p.x);
        outpoints[Num_out++] = newp;
        outpoints[Num_out++] = p;
      }
    }
    else if ((s.x - gx_Clip.xleft) >= 0) {
      newp.x = gx_Clip.xleft;
      newp.y = p.y + (s.y-p.y) * (gx_Clip.xleft-p.x) / (s.x-p.x);
      outpoints[Num_out++] = newp;
    }
    s = p;
  }
}

/*____________________________________________________________________
|
| Function: Clip_Polygon_Right
|
| Input: Called from Clip_Polygon()
| Output: Fills outpoints array with clipped polygon vertices.
|       outputs array should be twice as large as inpoints array. This
|       function clips the polygon against one clipping edge.
|___________________________________________________________________*/

static void Clip_Polygon_Right (
  Point *inpoints,
  Point *outpoints,
  int    num_in )
{
  int i;
  Point s, p, newp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (inpoints);
  DEBUG_ASSERT (outpoints);
  DEBUG_ASSERT (num_in > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Init variables
  Num_out = 0;
  // Start with last vertex in input array
  s = inpoints[num_in-1];

  for (i=0; i<num_in; i++) {
    p = inpoints[i];
    if ((gx_Clip.xright - p.x) >= 0) {
      if ((gx_Clip.xright - s.x) >= 0)
        outpoints[Num_out++] = p;
      else {
        newp.x = gx_Clip.xright;
        newp.y = p.y + (s.y-p.y) * (gx_Clip.xright-p.x) / (s.x-p.x);
        outpoints[Num_out++] = newp;
        outpoints[Num_out++] = p;
      }
    }
    else if ((gx_Clip.xright - s.x) >= 0) {
      newp.x = gx_Clip.xright;
      newp.y = p.y + (s.y-p.y) * (gx_Clip.xright-p.x) / (s.x-p.x);
      outpoints[Num_out++] = newp;
    }
    s = p;
  }
}

/*____________________________________________________________________
|
| Function: Clip_Polygon_Top
|
| Input: Called from Clip_Polygon()
| Output: Fills outpoints array with clipped polygon vertices.
|       outputs array should be twice as large as inpoints array. This
|       function clips the polygon against one clipping edge.
|___________________________________________________________________*/

static void Clip_Polygon_Top (
  Point *inpoints,
  Point *outpoints,
  int    num_in )
{
  int i;
  Point s, p, newp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (inpoints);
  DEBUG_ASSERT (outpoints);
  DEBUG_ASSERT (num_in > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Init variables
  Num_out = 0;
  // Start with last vertex in input array
  s = inpoints[num_in-1];

  for (i=0; i<num_in; i++) {
    p = inpoints[i];
    if ((p.y - gx_Clip.ytop) >= 0) {
      if ((s.y - gx_Clip.ytop) >= 0)
        outpoints[Num_out++] = p;
      else {
        newp.x = p.x + (s.x-p.x) * (gx_Clip.ytop-p.y) / (s.y-p.y);
        newp.y = gx_Clip.ytop;
        outpoints[Num_out++] = newp;
        outpoints[Num_out++] = p;
      }
    }
    else if ((s.y - gx_Clip.ytop) >= 0) {
      newp.x = p.x + (s.x-p.x) * (gx_Clip.ytop-p.y) / (s.y-p.y);
      newp.y = gx_Clip.ytop;
      outpoints[Num_out++] = newp;
    }
    s = p;
  }
}

/*____________________________________________________________________
|
| Function: Clip_Polygon_Bottom
|
| Input: Called from Clip_Polygon()
| Output: Fills outpoints array with clipped polygon vertices.
|       outputs array should be twice as large as inpoints array. This
|       function clips the polygon against one clipping edge.
|___________________________________________________________________*/

static void Clip_Polygon_Bottom (
  Point *inpoints,
  Point *outpoints,
  int    num_in )
{
  int i;
  Point s, p, newp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (inpoints);
  DEBUG_ASSERT (outpoints);
  DEBUG_ASSERT (num_in > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Init variables
  Num_out = 0;
  // Start with last vertex in input array
  s = inpoints[num_in-1];

  for (i=0; i<num_in; i++) {
    p = inpoints[i];
    if ((gx_Clip.ybottom - p.y) >= 0) {
      if ((gx_Clip.ybottom - s.y) >= 0)
        outpoints[Num_out++] = p;
      else {
        newp.x = p.x + (s.x-p.x) * (gx_Clip.ybottom-p.y) / (s.y-p.y);
        newp.y = gx_Clip.ybottom;
        outpoints[Num_out++] = newp;
        outpoints[Num_out++] = p;
      }
    }
    else if ((gx_Clip.ybottom - s.y) >= 0) {
      newp.x = p.x + (s.x-p.x) * (gx_Clip.ybottom-p.y) / (s.y-p.y);
      newp.y = gx_Clip.ybottom;
      outpoints[Num_out++] = newp;
    }
    s = p;
  }
}
