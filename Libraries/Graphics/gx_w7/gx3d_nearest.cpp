/*____________________________________________________________________
|
| File: gx3d_nearest.cpp
|
| Description: Functions for nearest point calculations.
|
|   Unless otherwise indicated, all functions assume the objects being 
|   tested are defined in the same coordinate system (for example: world 
|   coordinates).
|
| Functions:  gx3d_Nearest_Point_Line
|             gx3d_Nearest_Point_Ray
|             gx3d_Nearest_Point_Ray
|             gx3d_Nearest_Point_Plane
|             gx3d_Nearest_Point_Sphere
|             gx3d_Nearest_Point_Box
|             gx3d_Nearest_Point_Triangle
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
| Function: gx3d_Nearest_Point_Line
|
| Output: Returns the nearest point on a line from a given point.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 278.
|   (closest point ray)
|___________________________________________________________________*/

void gx3d_Nearest_Point_Line (gx3dVector *point, gx3dLine *line, gx3dVector *nearest_point)
{
  float t;
  gx3dVector v, direction;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (line);
  DEBUG_ASSERT (nearest_point);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute direction normal of line
  gx3d_SubtractVector (&line->end, &line->start, &direction);
  gx3d_NormalizeVector (&direction, &direction);

  // Create a vector v from start to point
  gx3d_SubtractVector (point, &line->start, &v);

  // Project v onto line (t will be the parametric distance)
  t = gx3d_VectorDotProduct (&v, &direction);
  
  if (t < 0)
    // Nearest point is start of line
    *nearest_point = line->start;
  else if (t > 1)
    // Nearest point is end of line
    *nearest_point = line->end;
  else {
    // Nearest point lies on the line
    gx3d_MultiplyScalarVector (t, &direction, &direction);
    gx3d_AddVector (&line->start, &direction, nearest_point);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Nearest_Point_Ray
|
| Output: Returns the nearest point on a infinite ray from a given point.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 278.
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

void gx3d_Nearest_Point_Ray (gx3dVector *point, gx3dRay *ray, gx3dVector *nearest_point)
{
  float t;
  gx3dVector v, direction;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (ray);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ray->direction, &ray->direction) -1.0) < .01);
  DEBUG_ASSERT (nearest_point);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create a vector v from ray origin to point
  gx3d_SubtractVector (point, &ray->origin, &v);

  // Project v onto ray (t will be the parametric distance)
  t = gx3d_VectorDotProduct (&v, &ray->direction);
  
  if (t < 0)
    // Nearest point is start of ray
    *nearest_point = ray->origin;
  else {
    // Nearest point lies on the line
    gx3d_MultiplyScalarVector (t, &ray->direction, &direction);
    gx3d_AddVector (&ray->origin, &direction, nearest_point);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Nearest_Point_Ray
|
| Output: Returns the nearest point on a ray from a given point.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 278.
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

void gx3d_Nearest_Point_Ray (gx3dVector *point, gx3dRay *ray, float ray_length, gx3dVector *nearest_point)
{
  float t;
  gx3dVector v, direction;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x + ray->direction.y + ray->direction.z);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (nearest_point);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create a vector v from ray origin to point
  gx3d_SubtractVector (point, &ray->origin, &v);

  // Project v onto ray (t will be the parametric distance)
  t = gx3d_VectorDotProduct (&v, &ray->direction);
  
  if (t < 0)
    // Nearest point is start of ray
    *nearest_point = ray->origin;
  else if (t > ray_length) {
    // Nearest point is end of ray
    gx3d_MultiplyScalarVector (ray_length, &ray->direction, &direction);
    gx3d_AddVector (&ray->origin, &direction, nearest_point);
  }
  else {
    // Nearest point lies on the line
    gx3d_MultiplyScalarVector (t, &ray->direction, &direction);
    gx3d_AddVector (&ray->origin, &direction, nearest_point);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Nearest_Point_Plane
|
| Output: Returns the nearest point on a plane from a given point.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 279.
|___________________________________________________________________*/

void gx3d_Nearest_Point_Plane (gx3dVector *point, gx3dPlane *plane, gx3dVector *nearest_point)
{
  float distance;
  gx3dVector v;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) -1.0) < .01);
  DEBUG_ASSERT (nearest_point);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute distance from point to plane
  distance = gx3d_Distance_Point_Plane (point, plane);
  // Multiply the -distance by the plane normal to get offset from point to plane
  gx3d_MultiplyScalarVector (-distance, &plane->n, &v);
  // Add this amount to point to get the nearest point on the plane
  gx3d_AddVector (point, &v, nearest_point);
}

/*____________________________________________________________________
|
| Function: gx3d_Nearest_Point_Sphere
|
| Output: Returns the nearest point on a sphere from a given point.
|   If point is inside the sphere, returns point.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 280.
|___________________________________________________________________*/

void gx3d_Nearest_Point_Sphere (gx3dVector *point, gx3dSphere *sphere, gx3dVector *nearest_point)
{
  float distance;
  gx3dVector v;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);
  DEBUG_ASSERT (nearest_point);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create a vector from point to sphere center
  gx3d_SubtractVector (&sphere->center, point, &v);
  // Get distance from point to sphere center
  distance = gx3d_VectorMagnitude (&v);

  if (distance < sphere->radius)
    // Point is inside sphere so just return original point
    *nearest_point = *point;
  else {
    // Compute displacement
    gx3d_MultiplyScalarVector ((distance-sphere->radius)/distance, &v, &v);
    // Add displacement to point to project onto sphere
    gx3d_AddVector (point, &v, nearest_point);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Nearest_Point_Box
|
| Output: Returns the nearest point in an AAB box from a given point.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 280.
|___________________________________________________________________*/

void gx3d_Nearest_Point_Box (gx3dVector *point, gx3dBox *box, gx3dVector *nearest_point)
{
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (box);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (point->x < box->min.x)
    nearest_point->x = box->min.x;
  else if (point->x > box->max.x)
    nearest_point->x = box->max.x;
  else
    nearest_point->x = point->x;

  if (point->y < box->min.y)
    nearest_point->y = box->min.y;
  else if (point->y > box->max.y)
    nearest_point->y = box->max.y;
  else
    nearest_point->y = point->y;

  if (point->z < box->min.z)
    nearest_point->z = box->min.z;
  else if (point->z > box->max.z)
    nearest_point->z = box->max.z;
  else
    nearest_point->z = point->z;
}

/*____________________________________________________________________
|
| Function: gx3d_Nearest_Point_Triangle
|
| Output: Returns the nearest point on a triangle from a given point.
|
| Reference: 3D Math for Graphics and Game Development (pg. 286)
|___________________________________________________________________*/

void gx3d_Nearest_Point_Triangle (gx3dVector *point, gx3dVector *vertices, gx3dVector *nearest_point)
{
  int i, s, e, nearest, edge_intersect[3], num_edge_intersect;
  float d, distance, nearest_distance_squared;
  gx3dVector v, edge;
  gx3dPlane plane;
  gx3dLine line;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (nearest_point);

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Determine triangle vertex that is nearest to point
  nearest = 0;
  nearest_distance_squared = gx3d_DistanceSquared_Point_Point (point, &vertices[nearest]);
  for (i=1; i<3; i++) {
    d = gx3d_DistanceSquared_Point_Point (point, &vertices[i]);
    if (d < nearest_distance_squared) {
      nearest = i;
      nearest_distance_squared = d;
    }
  }

  // Compute a vector from nearest vertex to point
  gx3d_SubtractVector (point, &vertices[nearest], &v);

  // Determine which edges intersect sphere surrounding point - check first two edges connected to nearest vertex
  for (i=0, num_edge_intersect=0; i<2; i++) {
    gx3d_SubtractVector (&vertices[nearest+i%3], &vertices[nearest], &edge);
    if (gx3d_VectorDotProduct (&v, &edge) > 0) {
      edge_intersect[i] = 1;
      num_edge_intersect++;
    }
    else
      edge_intersect[i] = 0;
  }
  // Check third edge only if first two don't both intersect the sphere
  if (num_edge_intersect < 2) {
    line.start = vertices[nearest+1%3];
    line.end   = vertices[nearest+2%3];
    gx3d_Nearest_Point_Line (point, &line, &v);
    if (gx3d_DistanceSquared_Point_Point (point, &v) <= nearest_distance_squared) {
      edge_intersect[2] = 1;
      num_edge_intersect++;
    }
    else
      edge_intersect[2] = 0;
  }
  else
    edge_intersect[2] = 0;

/*____________________________________________________________________
|
| Find nearest point on the plane of the triangle
|___________________________________________________________________*/

  if (num_edge_intersect >= 2) {
    // Compute the plane formed by the triangle
    gx3d_GetPlane (&vertices[0], &vertices[1], &vertices[2], &plane);
    distance = gx3d_Distance_Point_Plane (point, &plane);
    // Multiply the -distance by the plane normal to get offset from point to plane
    gx3d_MultiplyScalarVector (-distance, &plane.n, &v);
    // Add this amount to point to get the nearest point on the plane
    gx3d_AddVector (point, &v, nearest_point);
  }

/*____________________________________________________________________
|
| Find nearest point on the line formed by two points of the triangle
|___________________________________________________________________*/

  else if (num_edge_intersect == 1) {
    for (i=0; i<3; i++) 
      if (edge_intersect[i]) {
        switch (i) {
          case 0:
          case 1: s = nearest;
                  e = nearest+i%3;
                  break;
          case 2: s = nearest+1%3;
                  e = nearest+2%3;
                  break;
        }
        line.start = vertices[s];
        line.end   = vertices[e];
        gx3d_Nearest_Point_Line (point, &line, nearest_point);
//        distance = gx3d_Distance_Point_Point (point, nearest_point);
        break;
      }
  }

/*____________________________________________________________________
|
| Nearest point is nearest point on the triangle
|___________________________________________________________________*/

  else {
    *nearest_point = vertices[nearest];
//    nearest_distance_squared = sqrtf (nearest_distance_squared);
  }
}
