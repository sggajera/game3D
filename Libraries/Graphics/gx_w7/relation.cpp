/*____________________________________________________________________
|
| File: relation.cpp
|
| Description: Functions to test relationships between 2D primitives.
|
| Functions:  gxRelation_Line_Line
|             gxRelation_Point_Polygon
|             gxRelation_Line_Triangle
|             gxRelation_Triangle_Triangle
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

/*____________________________________________________________________
|
| Function: gxRelation_Line_Line
|
| Output: Returns position of a line relative to another line.  The first
|   line is given by endpoints p1,p2.  The second line is given by p3,p4.
|   Optionally, this function will compute the intersection point and
|   return it in callers variables.
|
|   Returns gxRELATION_OUTSIDE   = lines do not intersect (or are coincident)
|           gxRELATION_PARALLEL  = lines are parallel
|           gxRELATION_INTERSECT = lines intersect
|
| Reference: Real-Time Rendering, 2nd ed., pg. 617
|___________________________________________________________________*/

gxRelation gxRelation_Line_Line (
  gxPointF *p1, 
  gxPointF *p2, 
  gxPointF *p3, 
  gxPointF *p4, 
  gxPointF *intersection )  // NULL if no return value needed
{
  int sign[2];
  float ax, bx, cx, ay, by, cy, d, e, f, num, offset, x1lo, x1hi, y1lo, y1hi;

/*____________________________________________________________________
|
| Test bounding boxes for early rejection
|___________________________________________________________________*/

  ax = p2->x - p1->x;
  bx = p3->x - p4->x;
  // X bound test
  if (ax < 0) {
    x1lo = p2->x; 
    x1hi = p1->x;
  } 
  else {
    x1lo = p1->x;
    x1hi = p2->x; 
  }
  if (bx > 0) {
    if ((x1hi < p4->x) OR (p3->x < x1lo)) 
      return (gxRELATION_OUTSIDE);
  } 
  else {
    if ((x1hi < p3->x) OR (p4->x < x1lo))
      return (gxRELATION_OUTSIDE);
  }

  ay = p2->y - p1->y;
  by = p3->y - p4->y;
  // Y bound test
  if (ay < 0) {
    y1lo = p2->y; 
    y1hi = p1->y;
  } 
  else {
    y1hi = p2->y; 
    y1lo = p1->y;
  }
  if (by > 0) {
    if ((y1hi < p4->y) OR (p3->y < y1lo)) 
      return (gxRELATION_OUTSIDE);
  } 
  else {
    if ((y1hi < p3->y) OR (p4->y < y1lo)) 
      return (gxRELATION_OUTSIDE);
  }

/*____________________________________________________________________
|
| More rejection tests
|___________________________________________________________________*/

  cx = p1->x - p3->x;
  cy = p1->y - p3->y;
  d = by * cx - bx * cy;  // alpha numerator
  f = ay * bx - ax * by;	// both denominator
  // alpha tests
  if (f > 0) {
    if ((d < 0) OR (d > f)) 
      return (gxRELATION_OUTSIDE);
  } 
  else {
    if ((d > 0) OR (d < f)) 
      return (gxRELATION_OUTSIDE);
  }

  e = ax * cy - ay * cx;  // beta numerator
  // beta tests
  if (f > 0) {
    if ((e < 0) OR (e > f)) 
      return (gxRELATION_OUTSIDE);
  } 
  else {
    if ((e > 0) OR (e < f)) 
      return (gxRELATION_OUTSIDE);
  }

/*____________________________________________________________________
|
| Reject parallel lines
|___________________________________________________________________*/

  if (f == 0)
    return (gxRELATION_PARALLEL);

/*____________________________________________________________________
|
| Lines intersect - compute intersection coords
|___________________________________________________________________*/

  if (intersection) {
    // Init variables
    offset = f / 2;
    if (f < 0)
      sign[0] = 1;
    else
      sign[0] = 0;

    num = d * ax;
    if (num < 0)
      sign[1] = 1;
    else
      sign[1] = 0;
    if (sign[0] != sign[1])
      offset = -offset;
    intersection->x = p1->x + (num+offset) / f;		

    num = d * ay;
    if (num < 0)
      sign[0] = 1;
    else
      sign[0] = 0;
    if (sign[0] != sign[1])
      offset = -offset;
    intersection->y = p1->y + (num+offset) / f;
  }

  return (gxRELATION_INTERSECT);
}

/*____________________________________________________________________
|
| Function: gxRelation_Point_Polygon
|
| Output: Returns position of point relative to polygon.  Uses odd 
|   crossings test (see reference).  
|
|   Returns gxRELATION_OUTSIDE = point is outside polygon
|           gxRELATION_INSIDE  = point is inside polygon
|
| Reference: Real-Time Rendering, 2nd ed., pg. 584
|___________________________________________________________________*/

gxRelation gxRelation_Point_Polygon (float x, float y, gxPointF *poly, int num_poly_points)
{
  int i, e0, e1, inside;
  bool y0, y1;

  e0 = (num_poly_points-1) * 2;
  e1 = 0;
  y0 = (poly[e0].y >= y);
  for (i=2, inside=0; i<=(num_poly_points*2); i+=2) {
    y1 = (poly[e1].y >= y);
    if (y0 != y1)
      if (((poly[e1].y - y) * (poly[e0].x - poly[e1].x) >= (poly[e1].x - x) * (poly[e0].y - poly[e1].y)) == y1)
        inside ^= 1;
    y0 = y1;
    e0 = e1;
    e1 = i;
  }
  if (inside)
    return (gxRELATION_INSIDE);
  else
    return (gxRELATION_OUTSIDE);
}  

/*____________________________________________________________________
|
| Function: gxRelation_Line_Triangle
|
| Output: Returns position of a line relative to a triangle.
|
|   Returns gxRELATION_OUTSIDE   = line is outside triangle
|           gxRELATION_INTERSECT = line intersects triangle (or is inside)
|___________________________________________________________________*/

gxRelation gxRelation_Line_Triangle (gxPointF *p1, gxPointF *p2, gxPointF *triangle)
{
  int i;
  gxRelation result = gxRELATION_OUTSIDE; // assume outside

  // Test line for intersection with the edges of triangle
  for (i=0; i<3; i++)
    if (gxRelation_Line_Line (p1, p2, &triangle[i], &triangle[i+1%3], NULL) == gxRELATION_INTERSECT) {
      result = gxRELATION_INTERSECT;
      break;
    }

  // Test for line totally contained in triangle
  if (result == gxRELATION_OUTSIDE) {
    if (gxRelation_Point_Polygon (p1->x, p1->y, triangle, 3) == gxRELATION_INTERSECT)
      result = gxRELATION_INTERSECT;
    else if (gxRelation_Point_Polygon (p2->x, p2->y, triangle, 3) == gxRELATION_INTERSECT)
      result = gxRELATION_INTERSECT;
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: gxRelation_Triangle_Triangle
|
| Output: Returns position of triangle relative to another triangle.
|
|   Returns gxRELATION_OUTSIDE   = triangle1 is outside triangle2
|           gxRELATION_INTERSECT = triangle1 intersects triangle2
|
| Notes: If either triangle is inside the other, the return value is
|   gxRELATION_INTERSECT.
|
| Reference: Real-Time Rendering, 2nd ed., pg. 592
|___________________________________________________________________*/

gxRelation gxRelation_Triangle_Triangle (gxPointF *triangle1, gxPointF *triangle2)
{
  int i, j;
  gxRelation result = gxRELATION_OUTSIDE; // assume outside

  // Test all edges of t1 for intersection with the edges of t2
  for (i=0; i<3; i++)
    for (j=0; j<3; j++) 
      if (gxRelation_Line_Line (&triangle1[i], &triangle1[i+1%3], 
                                &triangle2[j], &triangle2[j+1%3], NULL) == gxRELATION_INTERSECT) {
        result = gxRELATION_INTERSECT;
        break;
      }

  // Test for t1 totally contained in t2 or vice versa
  if (result == gxRELATION_OUTSIDE) {
    // Test for t1 contained in t2
    for (i=0; i<3; i++) 
      if (gxRelation_Point_Polygon (triangle1[i].x, triangle1[i].y, triangle2, 3) == gxRELATION_OUTSIDE)
        break;
    if (i == 3)
      result = gxRELATION_INTERSECT;
    else {
      // Test for t2 contained in t1
      for (i=0; i<3; i++) 
        if (gxRelation_Point_Polygon (triangle2[i].x, triangle2[i].y, triangle1, 3) == gxRELATION_OUTSIDE)
          break;
      if (i == 3)
        result = gxRELATION_INTERSECT;
    }
  }

  return (result);
}
