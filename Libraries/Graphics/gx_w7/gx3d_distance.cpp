/*____________________________________________________________________
|
| File: gx3d_distance.cpp
|
| Description: Functions for distance calculations.
|
|   Unless otherwise indicated, all functions assume the objects being 
|   tested are defined in the same coordinate system (for example: world 
|   coordinates).
|
| Functions:  gx3d_Distance_Point_Point
|             gx3d_DistanceSquared_Point_Point
|             gx3d_Distance_Point_Line
|             gx3d_Distance_Point_Ray
|             gx3d_Distance_Point_Ray
|             gx3d_Distance_Point_Plane
|             gx3d_Distance_Point_Sphere
|             gx3d_Distance_Point_Box
|             gx3d_Distance_Point_Triangle
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
| Function: gx3d_Distance_Point_Point
|
| Output: Returns the absolute (positive) distance between two 3D points.
|___________________________________________________________________*/

inline float gx3d_Distance_Point_Point (gx3dVector *p1, gx3dVector *p2)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (p1);
  DEBUG_ASSERT (p2);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (sqrtf ((p2->x - p1->x) * (p2->x - p1->x) +
                 (p2->y - p1->y) * (p2->y - p1->y) +
                 (p2->z - p1->z) * (p2->z - p1->z)));
}

/*____________________________________________________________________
|
| Function: gx3d_DistanceSquared_Point_Point
|
| Output: Returns the absolute (positive) distance squared between two 
|   3D points.  This is useful for some tests.  To get the actual distance
|   take the sqrt() of the value returned by this function.
|___________________________________________________________________*/

inline float gx3d_DistanceSquared_Point_Point (gx3dVector *p1, gx3dVector *p2)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (p1);
  DEBUG_ASSERT (p2);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return ((p2->x - p1->x) * (p2->x - p1->x) +
          (p2->y - p1->y) * (p2->y - p1->y) +
          (p2->z - p1->z) * (p2->z - p1->z));
}

/*____________________________________________________________________
|
| Function: gx3d_Distance_Point_Line
|
| Output: Returns the absolute (positive) distance from a point to the 
|   nearest point on a line.  
|___________________________________________________________________*/

float gx3d_Distance_Point_Line (gx3dVector *point, gx3dLine *line)
{
  gx3dVector nearest_point;
  float distance;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (line);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_Nearest_Point_Line (point, line, &nearest_point);
  distance = gx3d_Distance_Point_Point (point, &nearest_point);

  return (distance);
}

/*____________________________________________________________________
|
| Function: gx3d_Distance_Point_Ray
|
| Output: Returns the absolute (postive) distance from a point to the
|   nearest point on an infinite ray.
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

float gx3d_Distance_Point_Ray (gx3dVector *point, gx3dRay *ray)
{
  gx3dVector nearest_point;
  float distance;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (ray);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_Nearest_Point_Ray (point, ray, &nearest_point);
  distance = gx3d_Distance_Point_Point (point, &nearest_point);

  return (distance);
}

/*____________________________________________________________________
|
| Function: gx3d_Distance_Point_Ray
|
| Output: Returns the absolute (postive) distance from a point to the
|   nearest point on an ray.
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

float gx3d_Distance_Point_Ray (gx3dVector *point, gx3dRay *ray, float ray_length)
{
  gx3dVector nearest_point;
  float distance;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (ray);
  DEBUG_ASSERT (ray_length > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_Nearest_Point_Ray (point, ray, ray_length, &nearest_point);
  distance = gx3d_Distance_Point_Point (point, &nearest_point);

  return (distance);
}

/*____________________________________________________________________
|
| Function: gx3d_Distance_Point_Plane
|
| Output: Returns the distance of a point to a plane.  Result will be
|   zero if point is on plane, positive if on normal side of plane, else 
|   negative.
|
| Reference: Graphics Gems 3, pg. 511.
|___________________________________________________________________*/

inline float gx3d_Distance_Point_Plane (gx3dVector *point, gx3dPlane *plane)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (plane);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // This is the dot product + d
  return (plane->n.x * point->x +
          plane->n.y * point->y +
          plane->n.z * point->z +
          plane->d);
}  

/*____________________________________________________________________
|
| Function: gx3d_Distance_Point_Sphere
|
| Output: Returns the distance between a point and the edge of a sphere.
|   If the point is inside the sphere, distance returned is negative. 
|   If the point is equal to the center of the sphere, returns 0.
|___________________________________________________________________*/

inline float gx3d_Distance_Point_Sphere (gx3dVector *point, gx3dSphere *sphere)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (sphere);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Distance_Point_Point (point, &sphere->center) - sphere->radius);
} 

/*____________________________________________________________________
|
| Function: gx3d_Distance_Point_Box
|
| Output: Returns the absolute (positive) distance between a point and
|   an AAB box.  If the point is inside the box, returns 0.
|___________________________________________________________________*/

float gx3d_Distance_Point_Box (gx3dVector *point, gx3dBox *box)
{
  gx3dVector nearest_point;
  float distance;

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

  gx3d_Nearest_Point_Box (point, box, &nearest_point);
  distance = gx3d_Distance_Point_Point (point, &nearest_point);

  return (distance);
}

/*____________________________________________________________________
|
| Function: gx3d_Distance_Point_Triangle
|
| Output: Returns the absolute (positive) distance between a point and
|   the nearest point on a triangle.
|___________________________________________________________________*/

float gx3d_Distance_Point_Triangle (gx3dVector *point, gx3dVector *vertices)
{
  gx3dVector nearest_point;
  float distance;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_Nearest_Point_Triangle (point, vertices, &nearest_point);
  distance = gx3d_Distance_Point_Point (point, &nearest_point);

  return (distance);
}
