/*____________________________________________________________________
|
| File: fillpoly.cpp
|
| Description: Routines for drawing a filled polygon.
|
| Functions:    gxDrawFillPoly
|                Scan_Convert_Polygon
|                 Build_Edge_Table
|                 Insert_Into_AET
|                 Draw_PolyLine
|                  Clip_PolyLine
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

#include "clippoly.h"
#include "drawline.h"

/*___________________
|
| Type definitions
|__________________*/

//typedef struct etentry {  // changed to newer type struct below 3/13/2012
//  int ymax;     /* max y-coord of edge          */
//  int xmin;     /* x-coord of bottom endpoint   */
//  int xdist;
//  int ydist;
//  int xincr;    /* x-incr used in stepping from 1 scan line to next */
//  int xincr_fraction;
//  int error;
//  struct etentry *next;
//} ETentry, *ETentryPtr;
struct ETentry {
  int ymax;     /* max y-coord of edge          */
  int xmin;     /* x-coord of bottom endpoint   */
  int xdist;
  int ydist;
  int xincr;    /* x-incr used in stepping from 1 scan line to next */
  int xincr_fraction;
  int error;
  ETentry *next;
};
typedef ETentry *ETentryPtr;

/*___________________
|
| Function Prototypes
|__________________*/

static void Scan_Convert_Polygon (
  int num_vertices,
  int *vertices,
  int height,
  int ymin );
static void Build_Edge_Table (
  ETentryPtr *edge_table,
  int         num_vertices, 
  int        *vertices,
  int         poly_ymin,    // smallest y-coord in polygon
  int         poly_ymax );  // largest y-coord in polygon 
static void Insert_Into_AET (
  ETentryPtr *edge_bucket,
  ETentryPtr *active_edge_table );
static void Draw_PolyLine (int x1, int x2, int y);
static int Clip_PolyLine (int *x1, int *x2, int y);

/*____________________________________________________________________
|
| Function: gxDrawFillPoly
|
| Output: Draws a filled polygon in current window, clipped to current
|       clipping rectangle.  Automatically connects first and last point.
|___________________________________________________________________*/

void gxDrawFillPoly (int num_points, int *points)
{
  int i, j;
  int ymin, ymax, yspan;
  int xlo, xhi, ylo, yhi;
  int *poly;
  int clipped_poly [30];
  int num_clipped_poly;
  gxRectangle clip, rect;
  int visible = TRUE;
  int mem_allocated = FALSE;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (num_points >= 3);
  DEBUG_ASSERT (points);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  /* A polygon with less than 3 vertices is not valid */
  if (num_points >= 3) {
    /* Allocate memory to process polygon */
    poly = (int *) malloc ((num_points+1) * 2 * sizeof(int));
    if (poly != NULL) {
      mem_allocated = TRUE;
      /* Adjust all coords relative to window */
      for (i=0,j=0; i<num_points; i++,j+=2) {
        poly[j]   = points[j]   + gx_Window.xleft;
        poly[j+1] = points[j+1] + gx_Window.ytop;
      }

      /* Can polygon be drawn with a hardware drawing routine? */
      if ((num_points == 3) AND
          (gx_Fill_pattern == gxPATTERN_SOLID) AND
          gx_Video.draw_fill_poly) {
        /* Make sure to clip before calling hardware! */
        if (gx_Clipping)
          visible = Clip_Polygon (poly, clipped_poly, num_points, &num_clipped_poly);
        else {
          gxGetClip (&clip);
          rect.xleft   = 0;
          rect.ytop    = 0;
          rect.xright  = PAGE_WIDTH-1;
          rect.ybottom = PAGE_HEIGHT-1;
          gxSetClip (&rect);
          visible = Clip_Polygon (poly, clipped_poly, num_points, &num_clipped_poly);
          gxSetClip (&clip);
        }
        free (poly);
        mem_allocated = FALSE;
        if (NOT visible)
          return;
        else {
          num_points = num_clipped_poly;
          poly       = clipped_poly;
          /* If poly is still 3 vertices, call hardware drawing routine? */
          if ((num_points == 3) AND gx_Video.draw_fill_poly) {
            (*gx_Video.draw_fill_poly) (num_points, poly);
            return;
          }
        }
      }

      /* Connect first and last vertices with a new edge */
      poly[num_points*2]   = poly[0];
      poly[num_points*2+1] = poly[1];
      num_points++;

      /* Init variables */
      ymin = poly[1];
      ymax = poly[1];
      xlo = TRUE;
      xhi = TRUE;
      ylo = TRUE;
      yhi = TRUE;
      for (i=1,j=2; i<num_points; i++,j+=2) {
        /* Compute min/max y-coord in polygon */
        if (poly[j+1] < ymin)
          ymin = poly[j+1];
        else if (poly[j+1] > ymax)
          ymax = poly[j+1];
        /* Check for completely clipped */
        if (xlo AND (poly[j]   >= gx_Clip.xleft))
          xlo = FALSE;
        if (xhi AND (poly[j]   <= gx_Clip.xright))
          xhi = FALSE;
        if (ylo AND (poly[j+1] >= gx_Clip.ytop))
          ylo = FALSE;
        if (yhi AND (poly[j+1] <= gx_Clip.ybottom))
          yhi = FALSE;
      }

      /* If not completely clipped, continue */
      if (!xlo AND !xhi AND !ylo AND !xhi) {
        /* Compute height of polygon */
        yspan = ymax - ymin + 1;
        /* Draw the polygon */
        Scan_Convert_Polygon (num_points, poly, yspan, ymin);
      }

      /* Free memory */
      if (mem_allocated)
        free (poly);
    }
  }
}

/*____________________________________________________________________
|
| Function: Scan_Convert_Polygon
|
| Input: Called from gxDrawFillPoly()
| Output: Draws a filled polygon in current window, clipped to current
|       clipping rectangle.
|___________________________________________________________________*/

static void Scan_Convert_Polygon (
  int num_vertices,
  int *vertices,
  int height,
  int poly_ymin )
{
  int y, poly_ymax;
  ETentryPtr *edge_table;
  ETentryPtr active_edge_table = NULL;
  ETentryPtr ep, tep, e1, e2, *epp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (num_vertices > 0);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  edge_table = (ETentryPtr *) calloc (height+1, sizeof(ETentryPtr));
  if (edge_table != NULL) {
    poly_ymax = poly_ymin + height - 1;
    Build_Edge_Table (edge_table, num_vertices, vertices, poly_ymin, poly_ymax);
    y = poly_ymin;
    /* Main loop */
    do {
      /* Move buckets from ET to AET */
      Insert_Into_AET (&(edge_table[y-poly_ymin]), &active_edge_table);
      /* Draw spans */
      ep = active_edge_table;
      while (ep) {
        e1 = ep;
        e2 = ep->next;
        if (e1 AND e2) {
          Draw_PolyLine (e1->xmin, e2->xmin, y);
          ep=e2->next;
        }
        else {
          Draw_PolyLine (e1->xmin, e1->xmin, y);
          ep = NULL;
        }
      }
      /* Remove all edges from AET not involved in next scan line */
      for (epp=&active_edge_table; *epp != NULL;) {
        tep = *epp;
        if (tep->ymax == y) {
          *epp = tep->next;
          free (tep);
        }
        else
          epp = &(*epp)->next;
      }
      /* Increment to next scan line */
      y++;
      /* For each nonvertical edge in AET, update x for the new y */
      for (ep=active_edge_table; ep; ep=ep->next)
        /* Is edge nonvertical? */
        if (ep->xdist != 0) {
          ep->xmin += ep->xincr;
          ep->error += ep->xincr_fraction;
          if (ep->error >= ep->ydist) {
            if (ep->xdist < 0)
              ep->xmin -= 1;
            else
              ep->xmin += 1;
            ep->error -= ep->ydist;
          }
        }

    } while ((y <= poly_ymax) OR active_edge_table);

    /* Free edge table - should be empty! */
    for (y=0; y<height; y++)
      if (edge_table[y])
        for (ep=edge_table[y]; ep;) {
          tep = ep;
          ep = ep->next;
          free (tep);
        }
    free (edge_table);
  }
}

/*____________________________________________________________________
|
| Function: Build_Edge_Table
|
| Input: Called from Scan_Convert_Polygon()
| Output: Builds edge table from vertices array.
|___________________________________________________________________*/

static void Build_Edge_Table (
  ETentryPtr *edge_table,
  int         num_vertices, 
  int        *vertices,
  int         poly_ymin,    // smallest y-coord in polygon
  int         poly_ymax )   // largest y-coord in polygon 
{
  int i, j, index;
  int ymin, ymax, xmin, xmax;
  ETentryPtr node, *epp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (edge_table);
  DEBUG_ASSERT (num_vertices > 0);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  /* Put each edge in edge table */
  for (i=0,j=0; i<num_vertices-1; i++,j+=2) {
    /* Set variables for this edge */
    if (vertices[j+1] <= vertices[j+3]) {
      xmin = vertices[j];
      ymin = vertices[j+1];
      xmax = vertices[j+2];
      ymax = vertices[j+3];
    }
    else {
      xmax = vertices[j];
      ymax = vertices[j+1];
      xmin = vertices[j+2];
      ymin = vertices[j+3];
    }
    /* Make sure this isn't a horizontal edge */
    if (ymax != ymin) {
      /* Alloate memory for an edge table bucket */
      node = (ETentryPtr) malloc (sizeof(ETentry));
      if (node != NULL) {
        /* Put info in bucket */
        node->ymax  = ymax - 1;
        node->xmin  = xmin;
        node->xdist = xmax - xmin;
        node->ydist = ymax - ymin;
        node->xincr = node->xdist / node->ydist;
        node->xincr_fraction = abs(node->xdist) % node->ydist;
        node->error = 0;
        /* Make sure last scanline is drawn */
        if (ymax == poly_ymax)
          node->ymax++;
        /* Compute index into edge table */
        index = ymin - poly_ymin;
        /* Insert bucket into edge table */
        for (epp=&(edge_table[index]); (*epp != NULL) AND (node->xmin >= (*epp)->xmin); epp=&(*epp)->next);
        node->next = *epp;
        *epp = node;
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: Insert_Into_AET
|
| Input: Called from Scan_Convert_Polygon()
| Output: Inserts into active edge table, buckets from edge table and
|       then bubble sorts active edge table on increasing xmin elements.
|___________________________________________________________________*/

static void Insert_Into_AET (ETentryPtr *edge_bucket, ETentryPtr *active_edge_table)
{
  int swapped;
  ETentryPtr *epp, *eppnext;
  ETentryPtr e1, e2;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (edge_bucket);
  DEBUG_ASSERT (active_edge_table);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  /* Add contents of bucket to end of AET */
  if (*edge_bucket != NULL) {
    for (epp=active_edge_table; *epp != NULL; epp=&(*epp)->next);
    *epp = *edge_bucket;
    /* Remove contents from bucket */
    *edge_bucket = NULL;
  }

  /* Bubble sort AET */
  if (*active_edge_table) {
    do {
      epp = active_edge_table;
      eppnext = &(*epp)->next;
      swapped = FALSE;
      while (*eppnext) {
        e1 = *epp;
        e2 = *eppnext;
        if (e2->xmin < e1->xmin) {
          *epp = e2;
          *eppnext = e2->next;
          e2->next = e1;
          swapped = TRUE;
        }
        epp = &(*epp)->next;
        eppnext = &(*epp)->next;
      }
    } while (swapped);
  }
}

/*____________________________________________________________________
|
| Function: Draw_PolyLine
|
| Input: Called from Scan_Convert_Polygon()
| Output: Draws a line of a polygon in current window, clipped to current
|       clipping rectangle.
|___________________________________________________________________*/

static void Draw_PolyLine (int x1, int x2, int y)
{
  int visible = TRUE;

  /* Clip */
  if (gx_Clipping)
    if (NOT Clip_PolyLine (&x1, &x2, y))
      visible = FALSE;

  /* Draw */
  if (visible) {
    if (gx_Fill_pattern != gxPATTERN_SOLID)
      Draw_Pattern_Line (x1, x2, y);
    else
      (*gx_Video.draw_line) (x1, y, x2, y);
  }
}

/*____________________________________________________________________
|
| Function: Clip_PolyLine
|
| Input: Called from Draw_PolyLine()
| Output: Clips line to current clipping rectangle.  Returns true if
|       any part of line is within clipping rectangle, else false if
|       line is entirely clipped.
|
| Description: Assumes line is horizontal.
|___________________________________________________________________*/

static int Clip_PolyLine (int *x1, int *x2, int y)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (x1);
  DEBUG_ASSERT (x2);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if ((y   < gx_Clip.ytop) OR
      (y   > gx_Clip.ybottom) OR
      (*x1 > gx_Clip.xright) OR
      (*x2 < gx_Clip.xleft))
    return (FALSE);
  else {
    if (*x1 < gx_Clip.xleft)
      *x1 = gx_Clip.xleft;
    if (*x2 > gx_Clip.xright)
      *x2 = gx_Clip.xright;
    return (TRUE);
  }
}
