/*____________________________________________________________________
|
| File: gx3d_bv.cpp
|
| Description: Functions to deal with bounding volumes.
|
| Functions:  gx3d_GetBoundBox
|             gx3d_GetBoundBox
|             gx3d_GetBoundBox
|             gx3d_EncloseBoundBox
|             gx3d_EncloseBoundBox
|             gx3d_EncloseBoundBox
|             gx3d_GetBoundBoxCenter
|             gx3d_TransformBoundBox
|
|             gx3d_GetBoundSphere
|             gx3d_GetBoundSphere
|             gx3d_GetBoundSphere
|             gx3d_GetOptimalBoundSphere
|             gx3d_EncloseBoundSphere
|             gx3d_EncloseBoundSphere
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

#include <math.h>
#include "dp.h"

/*____________________________________________________________________
|
| Function: gx3d_GetBoundBox
|                                                                                        
| Output: Returns an AAB box for a set of points.
|___________________________________________________________________*/

void gx3d_GetBoundBox (gx3dBox *box, gx3dVector *vertices, int num_vertices)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (num_vertices >= 1);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Set min and max box bounds to values of first vertices
  box->min = vertices[0];
  box->max = vertices[0];

  // Go through rest of vertices
  gx3d_EncloseBoundBox (box, &vertices[1], num_vertices-1);
}

/*____________________________________________________________________
|
| Function: gx3d_GetBoundBox
|                                                                                        
| Output: Returns an AAB box for a set of points.
|___________________________________________________________________*/

void gx3d_GetBoundBox (gx3dBox *box, gx3dVector **vertices, int num_vertices)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (*vertices);
  DEBUG_ASSERT (num_vertices >= 1);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Set min and max box bounds to values of first vertices
  box->min = *(vertices[0]);
  box->max = *(vertices[0]);

  // Go through rest of vertices
  gx3d_EncloseBoundBox (box, &vertices[1], num_vertices-1);
}

/*____________________________________________________________________
|
| Function: gx3d_GetBoundBox
|                                                                                        
| Output: Returns an AAB box that encloses two AAB boxes.
|___________________________________________________________________*/

void gx3d_GetBoundBox (gx3dBox *new_box, gx3dBox *box1, gx3dBox *box2)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (new_box);
  DEBUG_ASSERT (box1);
  DEBUG_ASSERT (box2);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (box1->min.x < box2->min.x)
    new_box->min.x = box1->min.x;
  else
    new_box->min.x = box2->min.x;
  if (box1->max.x > box2->max.x)
    new_box->max.x = box1->max.x;
  else
    new_box->max.x = box2->max.x;

  if (box1->min.y < box2->min.y)
    new_box->min.y = box1->min.y;
  else
    new_box->min.y = box2->min.y;
  if (box1->max.y > box2->max.y)
    new_box->max.y = box1->max.y;
  else
    new_box->max.y = box2->max.y;

  if (box1->min.z < box2->min.z)
    new_box->min.z = box1->min.z;
  else
    new_box->min.z = box2->min.z;
  if (box1->max.z > box2->max.z)
    new_box->max.z = box1->max.z;
  else
    new_box->max.z = box2->max.z;
}

/*____________________________________________________________________
|
| Function: gx3d_EncloseBoundBox
|                                                                                        
| Output: Increases the size of a bounding box, if needed, to enclose
|   a set of points.
|___________________________________________________________________*/

void gx3d_EncloseBoundBox (gx3dBox *box, gx3dVector *vertices, int num_vertices)
{
  int i;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (num_vertices >= 1);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Go through vertices
  for (i=0; i<num_vertices; i++) {
    if (vertices[i].x < box->min.x)
      box->min.x = vertices[i].x;
    if (vertices[i].x > box->max.x)
      box->max.x = vertices[i].x;

    if (vertices[i].y < box->min.y)
      box->min.y = vertices[i].y;
    if (vertices[i].y > box->max.y)
      box->max.y = vertices[i].y;

    if (vertices[i].z < box->min.z)
      box->min.z = vertices[i].z;
    if (vertices[i].z > box->max.z)
      box->max.z = vertices[i].z;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_EncloseBoundBox
|                                                                                        
| Output: Increases the size of a bounding box, if needed, to enclose
|   a set of points.
|___________________________________________________________________*/

void gx3d_EncloseBoundBox (gx3dBox *box, gx3dVector **vertices, int num_vertices)
{
  int i;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (*vertices);
  DEBUG_ASSERT (num_vertices >= 1);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Go through vertices
  for (i=0; i<num_vertices; i++) {
    if (vertices[i]->x < box->min.x)
      box->min.x = vertices[i]->x;
    if (vertices[i]->x > box->max.x)
      box->max.x = vertices[i]->x;

    if (vertices[i]->y < box->min.y)
      box->min.y = vertices[i]->y;
    if (vertices[i]->y > box->max.y)
      box->max.y = vertices[i]->y;

    if (vertices[i]->z < box->min.z)
      box->min.z = vertices[i]->z;
    if (vertices[i]->z > box->max.z)
      box->max.z = vertices[i]->z;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_EncloseBoundBox
|                                                                                        
| Output: Increases the size of a bounding box, if needed, to enclose
|   another bounding box.
|___________________________________________________________________*/

void gx3d_EncloseBoundBox (gx3dBox *box, gx3dBox *box_to_enclose)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (box_to_enclose);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_GetBoundBox (box, box, box_to_enclose);
}

/*____________________________________________________________________
|
| Function: gx3d_GetBoundBoxCenter
|                       
| Output: Computes center of a bounding box.
|___________________________________________________________________*/

void gx3d_GetBoundBoxCenter (gx3dBox *box, gx3dVector *center)
{
  gx3dVector dimension;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (center);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute the size (width, height, depth) of the bounding box
  dimension.x = box->max.x - box->min.x;
  dimension.y = box->max.y - box->min.y;
  dimension.z = box->max.z - box->min.z;
    
  // Set center to center of bounding box
  center->x = box->min.x + dimension.x / (float)2;
  center->y = box->min.y + dimension.y / (float)2;
  center->z = box->min.z + dimension.z / (float)2;
}

/*____________________________________________________________________
|
| Function: gx3d_TransformBoundBox
|                       
| Output: Transforms a bounding box.  The result is a bounding box at
|   least as large as the original and may be larger.
|
|   The transform is restricted to affine transformations - any combination
|   of rotation, scaling (uniform & non-uniform) and translation.
|
| Reference: Graphics Gems, pg. 548, 785, 3D Math for Graphics and Game
|   Development, pg. 304
|___________________________________________________________________*/

void gx3d_TransformBoundBox (gx3dBox *box, gx3dMatrix *m, gx3dBox *new_box)
{
  gx3dBox xbox;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (new_box);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Start with translation
  xbox.min.x = xbox.max.x = m->_30;
  xbox.min.y = xbox.max.y = m->_31;
  xbox.min.z = xbox.max.z = m->_32;

  // Find the extreme points by looking at product of the min and max with each component of matrix
  if (m->_00 > 0) {
    xbox.min.x += (m->_00 * box->min.x);
    xbox.max.x += (m->_00 * box->max.x);
  }
  else {
    xbox.min.x += (m->_00 * box->max.x);
    xbox.max.x += (m->_00 * box->min.x);
  }

  if (m->_01 > 0) {
    xbox.min.y += (m->_01 * box->min.x);
    xbox.max.y += (m->_01 * box->max.x);
  }
  else {
    xbox.min.y += (m->_01 * box->max.x);
    xbox.max.y += (m->_01 * box->min.x);
  }

  if (m->_02 > 0) {
    xbox.min.z += (m->_02 * box->min.x);
    xbox.max.z += (m->_02 * box->max.x);
  }
  else {
    xbox.min.z += (m->_02 * box->max.x);
    xbox.max.z += (m->_02 * box->min.x);
  }

  if (m->_10 > 0) {
    xbox.min.x += (m->_10 * box->min.y);
    xbox.max.x += (m->_10 * box->max.y);
  }
  else {
    xbox.min.x += (m->_10 * box->max.y);
    xbox.max.x += (m->_10 * box->min.y);
  }

  if (m->_11 > 0) {
    xbox.min.y += (m->_11 * box->min.y);
    xbox.max.y += (m->_11 * box->max.y);
  }
  else {
    xbox.min.y += (m->_11 * box->max.y);
    xbox.max.y += (m->_11 * box->min.y);
  }

  if (m->_12 > 0) {
    xbox.min.z += (m->_12 * box->min.y);
    xbox.max.z += (m->_12 * box->max.y);
  }
  else {
    xbox.min.z += (m->_12 * box->max.y);
    xbox.max.z += (m->_12 * box->min.y);
  }

  if (m->_20 > 0) {
    xbox.min.x += (m->_20 * box->min.z);
    xbox.max.x += (m->_20 * box->max.z);
  }
  else {
    xbox.min.x += (m->_20 * box->max.z);
    xbox.max.x += (m->_20 * box->min.z);
  }

  if (m->_21 > 0) {
    xbox.min.y += (m->_21 * box->min.z);
    xbox.max.y += (m->_21 * box->max.z);
  }
  else {
    xbox.min.y += (m->_21 * box->max.z);
    xbox.max.y += (m->_21 * box->min.z);
  }

  if (m->_22 > 0) {
    xbox.min.z += (m->_22 * box->min.z);
    xbox.max.z += (m->_22 * box->max.z);
  }
  else {
    xbox.min.z += (m->_22 * box->max.z);
    xbox.max.z += (m->_22 * box->min.z);
  }

  // Copy the transformed box into the new box
  *new_box = xbox;
}

/*____________________________________________________________________
|
| Function: gx3d_GetBoundSphere
|                                                                                        
| Output: Returns a bounding sphere for a set of points.
|___________________________________________________________________*/

void gx3d_GetBoundSphere (gx3dSphere *sphere, gx3dVector *vertices, int num_vertices)
{
  gx3dBox box;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (num_vertices >= 1);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute bounding box for the set of points
  gx3d_GetBoundBox (&box, vertices, num_vertices);

  // Compute bounding sphere
  gx3d_GetBoundSphere (sphere, vertices, num_vertices, &box);
}

/*____________________________________________________________________
|
| Function: gx3d_GetBoundSphere
|                                                                                        
| Output: Returns a bounding sphere for a set of points, given a previously
|   computed bounding box.
|___________________________________________________________________*/

void gx3d_GetBoundSphere (gx3dSphere *sphere, gx3dVector *vertices, int num_vertices, gx3dBox *bound_box)
{
  int i;
  float len;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (num_vertices >= 1);
  DEBUG_ASSERT (bound_box);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Init sphere
  gx3d_GetBoundBoxCenter (bound_box, &sphere->center);
  sphere->radius = 0;

  // Compute sphere radius
  for (i=0; i<num_vertices; i++) {
    len = gx3d_DistanceSquared_Point_Point (&sphere->center, &vertices[i]);
    if (len > sphere->radius)
      sphere->radius = len;
  }
  // Adjust to the actual distance
  sphere->radius = sqrtf (sphere->radius);
}

/*____________________________________________________________________
|
| Function: gx3d_GetBoundSphere
|                                                                                        
| Output: Returns a sphere that encloses two spheres.
|___________________________________________________________________*/

void gx3d_GetBoundSphere (gx3dSphere *new_sphere, gx3dSphere *sphere1, gx3dSphere *sphere2)
{
  gx3dVector direction, v, p1, p2;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (new_sphere);
  DEBUG_ASSERT (sphere1);
  DEBUG_ASSERT (sphere2);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute the line between the 2 spheres
  gx3d_SubtractVector (&sphere2->center, &sphere1->center, &v);
  // Compute the direction normal of this line
  gx3d_NormalizeVector (&v, &direction);
  // Compute new endpoints of the line stretched across both spheres
  gx3d_MultiplyScalarVector (sphere2->radius, &direction, &v);
  gx3d_AddVector (&sphere2->center, &v, &p2);
  gx3d_MultiplyScalarVector (-sphere1->radius, &direction, &v);
  gx3d_AddVector (&sphere1->center, &v, &p1);
  // Compute new sphere center
  gx3d_SubtractVector (&p2, &p1, &v);
  gx3d_MultiplyScalarVector (.5, &v, &v);
  gx3d_AddVector (&p1, &v, &new_sphere->center);
  // Compute new sphere radius
  new_sphere->radius = gx3d_VectorMagnitude (&v);
}

/*____________________________________________________________________
|
| Function: gx3d_GetOptimalBoundSphere
|                       
| Output: Returns a near optimal bounding sphere for a set of points.
|
| Reference: Graphics Gems, pg 301,723.
|___________________________________________________________________*/

void gx3d_GetOptimalBoundSphere (gx3dSphere *sphere, gx3dVector *vertices, int num_vertices)
{
  int i;
  float rad, rad_sq, xspan, yspan, zspan, maxspan, old_to_p, old_to_p_sq, old_to_new;
  gx3dVector xmin, xmax, ymin, ymax, zmin, zmax, dia1, dia2, cen;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (num_vertices >= 1);

/*____________________________________________________________________
|
| Find 6 min/max points                      
|___________________________________________________________________*/

  // Init for min/max compare
  xmin = vertices[0];
  ymin = vertices[0];
  zmin = vertices[0];
  xmax = vertices[0];
  ymax = vertices[0];
  zmax = vertices[0];

  // Find min/max vertices for all three coordinates axes
  for (i=1; i<num_vertices; i++) {
    if (vertices[i].x < xmin.x)
      xmin = vertices[i];
    if (vertices[i].x > xmax.x)
      xmax = vertices[i];
    if (vertices[i].y < ymin.y)
      ymin = vertices[i];
    if (vertices[i].y > ymax.y)
      ymax = vertices[i];
    if (vertices[i].z < zmin.z)
      zmin = vertices[i];
    if (vertices[i].z > zmax.z)
      zmax = vertices[i];
  }
    
/*____________________________________________________________________
|
| Calculate the initial sphere from the 6 points
|___________________________________________________________________*/

  // Set xspan = distance between the 2 points xmin, xmax (squared)
  xspan = gx3d_DistanceSquared_Point_Point (&xmin, &xmax);
  // Set yspan = distance between the 2 points ymin, ymax (squared)
  yspan = gx3d_DistanceSquared_Point_Point (&ymin, &ymax);
  // Set zspan = distance between the 2 points zmin, zmax (squared)
  zspan = gx3d_DistanceSquared_Point_Point (&ymin, &ymax);

  // Set points dia1, dia2 to the maximally separated pair (dia1,dia2 is diameter of initial sphere)
  dia1 = xmin;  // assume xspan is the largest
  dia2 = xmax;
  maxspan = xspan;
  if (yspan > maxspan) {
    maxspan = yspan;
    dia1 = ymin;
    dia2 = ymax;
  }
  if (zspan > maxspan) {
    dia1 = zmin;
    dia2 = zmax;
  }

  // Calculate initial center
  cen.x = (dia1.x + dia2.x) / 2;
  cen.y = (dia1.y + dia2.y) / 2;
  cen.z = (dia1.z + dia2.z) / 2;

  // Calculate initial radius squared and radius
  rad_sq = gx3d_DistanceSquared_Point_Point (&dia2, &cen);
  rad = sqrtf (rad_sq);

/*____________________________________________________________________
|
| Grow sphere to encompass all points
|___________________________________________________________________*/

  for (i=0; i<num_vertices; i++) {
    // Get distance from center to a point
    old_to_p_sq = gx3d_DistanceSquared_Point_Point (&vertices[i], &cen);
    // Is this point outside of current sphere?
    if (old_to_p_sq > rad_sq) {
      old_to_p = sqrtf (old_to_p_sq);
      // Calculate radius of new sphere
      rad = (rad + old_to_p) / 2.0f;
      rad_sq = rad * rad;
      old_to_new = old_to_p - rad;
      // Calculate center of new sphere
      cen.x = (rad*cen.x + old_to_new*vertices[i].x) / old_to_p;
      cen.y = (rad*cen.y + old_to_new*vertices[i].y) / old_to_p;
      cen.z = (rad*cen.z + old_to_new*vertices[i].z) / old_to_p;
    }
  }

  sphere->center = cen;
  sphere->radius = rad;
}

/*____________________________________________________________________
|
| Function: gx3d_EncloseBoundSphere
|                                                                                        
| Output: Increases the size of a bounding sphere, if needed, to enclose
|   a set of points.
|___________________________________________________________________*/

void gx3d_EncloseBoundSphere (gx3dSphere *sphere, gx3dVector *vertices, int num_vertices)
{
  int i;
  bool update;
  float len, radius_squared;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (num_vertices >= 1);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  radius_squared = sphere->radius * sphere->radius;
  for (i=0, update=false; i<num_vertices; i++) {
    len = gx3d_DistanceSquared_Point_Point (&sphere->center, &vertices[i]);
    if (len > radius_squared) {
      radius_squared = len;
      update = true;
    }
  }
  // Adjust to the actual distance?
  if (update)
    sphere->radius = sqrtf (radius_squared);
}

/*____________________________________________________________________
|
| Function: gx3d_EncloseBoundSphere
|                                                                                        
| Output: Increases the size of a bounding sphere, if needed, to enclose
|   another bounding sphere.
|___________________________________________________________________*/

void gx3d_EncloseBoundSphere (gx3dSphere *sphere, gx3dSphere *sphere_to_enclose)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere_to_enclose);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_GetBoundSphere (sphere, sphere, sphere_to_enclose);
}
