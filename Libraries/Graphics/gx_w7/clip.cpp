/*____________________________________________________________________
|
| File: clip.cpp
|
| Description: A module to perform clipping of primitives to a clipping
|       rectangle.  This module has been optimized by making some
|       parameters global and eliminating the passing of unused parameters.
|
| Functions:    gxClipPoint
|               gxClipRectangle
|               gxClipLine
|                Leftcolumn
|                Topleftcorner
|                Leftbottomregion
|                Leftedge
|                P2bottom
|                Centercolumn
|                Inside
|                P2left
|                P2lefttop
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
| Function prototypes
|__________________*/

static void Leftcolumn (
  int  xleft,
  int  ytop,
  int  xright,
  int  ybottom );
static void Topleftcorner (
  int  xleft,
  int  ytop,
  int  xright,
  int  ybottom );
static void Leftbottomregion (
  int  xleft,
  int  xright,
  int  ybottom,
  int  relx2,
  int  rely2,
  int  leftproduct );
static void Leftedge (
  int  xleft,
  int  ytop,
  int  xright,
  int  ybottom );
static void P2bottom (
  int  xleft,
  int  xright,
  int  ybottom );
static void Centercolumn (
  int  xleft,
  int  ytop,
  int  xright,
  int  ybottom );
static void Inside (
  int  xleft,
  int  ytop,
  int  xright,
  int  ybottom );
static void P2left (
  int  xleft,
  int  ytop,
  int  ybottom );
static void P2lefttop (
  int  xleft,
  int  ytop );

/*___________________
|
| Macros
|__________________*/

/* Rotations, clockwise about the origin */
#define ROTATE90C(_x,_y)        \
  {                             \
  t  = _x;                      \
  _x = -_y;                     \
  _y = t;                       \
  }
#define ROTATE180C(_x,_y) \
  {                       \
  _x = -_x;               \
  _y = -_y;               \
  }
#define ROTATE270C(_x,_y)       \
  {                             \
  t  = _x;                      \
  _x = _y;                      \
  _y = -t;                      \
  }

/* Reflection about the line x = y */
#define REFLECTXEQUALY(_x,_y)   \
  {                             \
  t  = _x;                      \
  _x = _y;                      \
  _y = t;                       \
  }

/* Reflection about the x-axis */
#define REFLECTXAXIS(_y) \
  _y = -_y;

/*___________________
|
| Global variables
|__________________*/

static int t;   /* used by macros, above */

static int tx1, ty1, tx2, ty2;
static int visible;

/*____________________________________________________________________
|
| Function: gxClipPoint
|
| Input: Coordinates of a point.
| Output: Returns true if point is visible.
|___________________________________________________________________*/

int gxClipPoint (int x, int y)
{
  if ((x < gx_Clip.xleft) OR (x > gx_Clip.xright) OR
      (y < gx_Clip.ytop)  OR (y > gx_Clip.ybottom))
    return (FALSE);
  else
    return (TRUE);
}

/*____________________________________________________________________
|
| Function: gxClipRectangle
|
| Input: Coordinates of a rectangle.
| Output: Returns true if any part of the input rectangle is visible.
|       Clips the rectangle if necessary.
|___________________________________________________________________*/

int gxClipRectangle (int *xleft, int *ytop, int *xright, int *ybottom)
{
  if (*xleft > gx_Clip.xright)
    return (FALSE);
  else if (*xleft < gx_Clip.xleft)
    *xleft = gx_Clip.xleft;

  if (*xright < gx_Clip.xleft)
    return (FALSE);
  else if (*xright > gx_Clip.xright)
    *xright = gx_Clip.xright;

  if (*ytop > gx_Clip.ybottom)
    return (FALSE);
  else if (*ytop < gx_Clip.ytop)
    *ytop = gx_Clip.ytop;

  if (*ybottom < gx_Clip.ytop)
    return (FALSE);
  else if (*ybottom > gx_Clip.ybottom)
    *ybottom = gx_Clip.ybottom;

  return (TRUE);
}

/*____________________________________________________________________
|
| Function: gxClipLine
|
| Input: Coordinates of a line.
| Output: Clips line coordinates in preparation for displaying.
|       Returns true if any part of the line is visible.  visible is a
|       global boolean set by subordinate routines.
|
| Description: Performs line clipping in 2-D using the Nicholl-Lee-Nicholl
|       algorithm.
|___________________________________________________________________*/

int gxClipLine (int *x1, int *y1, int *x2, int *y2)
{
  tx1 = *x1;
  ty1 = *y1;
  tx2 = *x2;
  ty2 = *y2;

  if (tx1 < gx_Clip.xleft)
    Leftcolumn (gx_Clip.xleft, gx_Clip.ytop, gx_Clip.xright, gx_Clip.ybottom);
  else if (tx1 > gx_Clip.xright) {
    ROTATE180C (tx1, ty1);
    ROTATE180C (tx2, ty2);
    Leftcolumn (-gx_Clip.xright, -gx_Clip.ybottom, -gx_Clip.xleft, -gx_Clip.ytop);
    ROTATE180C (tx1, ty1);
    ROTATE180C (tx2, ty2);
  }
  else
    Centercolumn (gx_Clip.xleft, gx_Clip.ytop, gx_Clip.xright, gx_Clip.ybottom);

  *x1 = tx1;
  *y1 = ty1;
  *x2 = tx2;
  *y2 = ty2;

  return (visible);
}

/*____________________________________________________________________
|
| Function: Leftcolumn
|
| Input: Called from LineClip()
|___________________________________________________________________*/

static void Leftcolumn (
  int xleft,
  int ytop,
  int xright,
  int ybottom )
{
  if (tx2 < xleft)
    visible = FALSE;
  else if (ty1 < ytop)
    Topleftcorner (xleft, ytop, xright, ybottom);
  else if (ty1 > ybottom) {
    REFLECTXAXIS (ty1);
    REFLECTXAXIS (ty2);
    Topleftcorner (xleft, -ybottom, xright, -ytop);
    REFLECTXAXIS (ty1);
    REFLECTXAXIS (ty2);
  }
  else
    Leftedge (xleft, ytop, xright, ybottom);
}

/*____________________________________________________________________
|
| Function: Topleftcorner
|
| Input: Called from Leftcolumn()
|___________________________________________________________________*/

static void Topleftcorner (
  int  xleft,
  int  ytop,
  int  xright,
  int  ybottom )
{
  static int relx2, rely2;
  static int topproduct, leftproduct;

  if (ty2 < ytop)
    visible = FALSE;
  else {
    relx2 = tx2 - tx1;
    rely2 = ty2 - ty1;
    topproduct  = (ytop - ty1) * relx2;
    leftproduct = (xleft - tx1) * rely2;
    if (topproduct < leftproduct)
      Leftbottomregion (xleft, xright, ybottom, relx2, rely2, leftproduct);
    else {
      REFLECTXEQUALY (tx1, ty1);
      REFLECTXEQUALY (tx2, ty2);
      Leftbottomregion (ytop, ybottom, xright, rely2, relx2, topproduct);
      REFLECTXEQUALY (tx1, ty1);
      REFLECTXEQUALY (tx2, ty2);
    }
  }
}

/*____________________________________________________________________
|
| Function: Leftbottomregion
|
| Input: Called from Topleftcorner()
|___________________________________________________________________*/

static void Leftbottomregion (
  int xleft,
  int xright,
  int ybottom,
  int relx2,
  int rely2,
  int leftproduct )
{
  static int bottomproduct, rightproduct;

  if (ty2 <= ybottom) {
    if (tx2 > xright) {
      ty2 = ty1 + (xright - tx1) * rely2 / relx2;
      tx2 = xright;
    }
    ty1 = ty1 + leftproduct / relx2;
    tx1 = xleft;
    visible = TRUE;
  }
  else {
    bottomproduct = (ybottom - ty1) * relx2;
    if (bottomproduct < leftproduct)
      visible = FALSE;
    else {
      if (tx2 > xright) {
        rightproduct = (xright - tx1) * rely2;
        if (bottomproduct < rightproduct) {
          tx2 = tx1 + bottomproduct / rely2;
          ty2 = ybottom;
        }
        else {
          ty2 = ty1 + rightproduct / relx2;
          tx2 = xright;
        }
      }
      else {
        tx2 = tx1 + bottomproduct / rely2;
        ty2 = ybottom;
      }
      ty1 = ty1 + leftproduct / relx2;
      tx1 = xleft;
      visible = TRUE;
    }
  }
}

/*____________________________________________________________________
|
| Function: Leftedge
|
| Input: Called from Leftcolumn(), Centercolumn()
|___________________________________________________________________*/

static void Leftedge (
  int xleft,
  int ytop,
  int xright,
  int ybottom )
{
  static int relx2, rely2;

  if (tx2 < xleft)
    visible = FALSE;
  else if (ty2 > ybottom)
    P2bottom (xleft, xright, ybottom);
  else if (ty2 < ytop) {
    REFLECTXAXIS (ty1);
    REFLECTXAXIS (ty2);
    P2bottom (xleft, xright, -ytop);
    REFLECTXAXIS (ty1);
    REFLECTXAXIS (ty2);
  }
  else {
    relx2 = tx2 - tx1;
    rely2 = ty2 - ty1;
    if (tx2 > xright ) {
      ty2 = ty1 + rely2 * (xright - tx1) / relx2;
      tx2 = xright;
    }
    ty1 = ty1 + rely2 * (xleft - tx1) / relx2;
    tx1 = xleft;
    visible = TRUE;
  }
}

/*____________________________________________________________________
|
| Function: P2bottom
|
| Input: Called from Leftedge()
|___________________________________________________________________*/

static void P2bottom (
  int xleft,
  int xright,
  int ybottom )
{
  static int relx2, rely2, leftproduct, bottomproduct, rightproduct;

  relx2 = tx2 - tx1;
  rely2 = ty2 - ty1;
  leftproduct = (xleft - tx1) * rely2;
  bottomproduct = (ybottom - ty1) * relx2;
  if (bottomproduct < leftproduct)
    visible = FALSE;
  else {
    if (tx2 < xright) {
      tx2 = tx1 + bottomproduct / rely2;
      ty2 = ybottom;
    }
    else {
      rightproduct = (xright - tx1) * rely2;
      if (bottomproduct < rightproduct) {
        tx2 = tx1 + bottomproduct / rely2;
        ty2 = ybottom;
      }
      else {
        ty2 = ty1 + rightproduct / relx2;
        tx2 = xright;
      }
    }
    ty1 = ty1 + leftproduct / relx2;
    tx1 = xleft;
    visible = TRUE;
  }
}

/*____________________________________________________________________
|
| Function: Centercolumn
|
| Input: Called from LineClip()
|___________________________________________________________________*/

static void Centercolumn (
  int xleft,
  int ytop,
  int xright,
  int ybottom )
{
  if (ty1 < ytop) {
    ROTATE270C (tx1, ty1);
    ROTATE270C (tx2, ty2);
    Leftedge (ytop, -xright, ybottom, -xleft);
    ROTATE90C (tx1, ty1);
    ROTATE90C (tx2, ty2);
  }
  else if (ty1 > ybottom) {
    ROTATE90C (tx1, ty1);
    ROTATE90C (tx2, ty2);
    Leftedge (-ybottom, xleft, -ytop, xright);
    ROTATE270C (tx1, ty1);
    ROTATE270C (tx2, ty2);
  }
  else
    Inside (xleft, ytop, xright, ybottom);
}

/*____________________________________________________________________
|
| Function: Inside
|
| Input: Called from Centercolumn()
|___________________________________________________________________*/

static void Inside (
  int xleft,
  int ytop,
  int xright,
  int ybottom )
{
  visible = TRUE;
  if (tx2 < xleft)
    P2left (xleft, ytop, ybottom);
  else if (tx2 > xright) {
    ROTATE180C (tx1, ty1);
    ROTATE180C (tx2, ty2);
    P2left (-xright, -ybottom, -ytop);
    ROTATE180C (tx1, ty1);
    ROTATE180C (tx2, ty2);
  }
  else if (ty2 < ytop) {
    tx2 = tx1 + (tx2 - tx1) * (ytop - ty1) / (ty2 - ty1);
    ty2 = ytop;
  }
  else if (ty2 > ybottom) {
    tx2 = tx1 + (tx2 - tx1) * (ybottom - ty1) / (ty2 - ty1);
    ty2 = ybottom;
  }
  /* else P2 is inside, no need to clip it */
}

/*____________________________________________________________________
|
| Function: P2left
|
| Input: Called from Inside()
|___________________________________________________________________*/

static void P2left (
  int xleft,
  int ytop,
  int ybottom )
{
  if (ty2 < ytop)
    P2lefttop (xleft, ytop);
  else if (ty2 > ybottom) {
    ROTATE90C (tx1, ty1);
    ROTATE90C (tx2, ty2);
    P2lefttop (-ybottom, xleft);
    ROTATE270C (tx1, ty1);
    ROTATE270C (tx2, ty2);
  }
  else {
    ty2 = ty1 + (ty2 - ty1) * (xleft - tx1) / (tx2 - tx1);
    tx2 = xleft;
  }
}

/*____________________________________________________________________
|
| Function: P2lefttop
|
| Input: Called from P2left()
|___________________________________________________________________*/

static void P2lefttop (
  int xleft,
  int ytop )
{
  static int relx2, rely2, leftproduct, topproduct;

  relx2 = tx2 - tx1;
  rely2 = ty2 - ty1;
  leftproduct = rely2 * (xleft - tx1);
  topproduct = relx2 * (ytop - ty1);
  if (topproduct < leftproduct) {
    tx2 = tx1 + topproduct / rely2;
    ty2 = ytop;
  }
  else {
    ty2 = ty1 + leftproduct / relx2;
    tx2 = xleft;
  }
}
