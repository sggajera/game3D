/*____________________________________________________________________
|
| File: gx3d_relation.cpp
|
| Description: Functions to test relationships between 3D objects.
|
|   Unless otherwise indicated, all functions assume the objects being 
|   tested are defined in the same coordinate system (for example: world 
|   coordinates).
|
| Functions:  gx3d_Relation_Point_Plane
|             gx3d_Relation_Line_Plane
|             gx3d_Relation_Ray_Plane
|             gx3d_Relation_Ray_Plane
|             gx3d_Relation_Sphere_Plane
|             gx3d_Relation_Box_Plane
|             gx3d_Relation_Box_Plane
|             gx3d_Relation_Triangle_Plane
|
|             gx3d_Relation_Point_Sphere
|             gx3d_Relation_Line_Sphere
|             gx3d_Relation_Ray_Sphere
|             gx3d_Relation_Sphere_Sphere
|             gx3d_Relation_Box_Sphere
|             gx3d_Relation_Triangle_Sphere
|
|             gx3d_Relation_Point_Box
|             gx3d_Relation_Ray_Box
|             gx3d_Relation_Ray_Box
|             gx3d_Relation_Box_Box
|             gx3d_Relation_Triangle_Box
|
|             gx3d_Relation_Ray_Triangle
|             gx3d_Relation_Ray_Triangle
|             gx3d_Relation_Ray_TriangleFront
|             gx3d_Relation_Ray_TriangleFront
|             gx3d_Relation_Triangle_Triangle
|
|             gx3d_Relation_Point_Frustum
|             gx3d_Relation_Point_Frustum
|             gx3d_Relation_Sphere_Frustum
|             gx3d_Relation_Sphere_Frustum
|             gx3d_Relation_Sphere_Frustum
|             gx3d_Relation_Sphere_Frustum
|             gx3d_Relation_Box_Frustum
|             gx3d_Relation_Box_Frustum
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

/*___________________
|
| Constants
|__________________*/

#define EPSILON ((float)0.000001)
#define EQUAL_ZERO(_val_) (((_val_) > -EPSILON) AND ((_val_) < EPSILON))  
#define GREATER_THAN_ZERO(_val_) ((_val_) > EPSILON)
#define LESS_THAN_ZERO(_val_) ((_val_) < -EPSILON)

/*____________________________________________________________________
|
| Function: gx3d_Relation_Point_Plane
|
| Output: Returns position of point relative to plane.
|
|   Returns gxRELATION_FRONT     = point is in front of plane
|           gxRELATION_BACK      = point is behind plane
|           gxRELATION_INTERSECT = point is within proximity to plane
|
| Notes: A suggested proximity is 0.001.
|___________________________________________________________________*/

gxRelation gx3d_Relation_Point_Plane (gx3dVector *point, gx3dPlane *plane, float proximity)
{
  float distance;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) - 1) < .01);
  DEBUG_ASSERT (proximity >= 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  distance = gx3d_Distance_Point_Plane (point, plane);

  if (distance > proximity)
    result = gxRELATION_FRONT;
  else if (distance < -proximity)
    result = gxRELATION_BACK;
  else
    result = gxRELATION_INTERSECT;  // coplanr

  return (result);
}  

/*____________________________________________________________________
|
| Function: gx3d_Relation_Line_Plane
|
| Output: Returns position of a line relative to a plane.
|
|   Returns gxRELATION_FRONT     = line is entirely in front of plane
|           gxRELATION_BACK      = line is entirely behind plane
|           gxRELATION_INTERSECT = line intersects plane
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Line_Plane (gx3dLine *line, gx3dPlane *plane)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (line);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) - 1) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Line_Plane (line, plane, NULL));
}  

/*____________________________________________________________________
|
| Function: gx3d_Relation_Ray_Plane
|
| Output: Returns position of an infinite ray relative to plane.
|
|   Returns gxRELATION_FRONT     = ray is entirely in front of plane
|           gxRELATION_BACK      = ray is entirely behind plane
|           gxRELATION_INTERSECT = ray intersects plane
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Ray_Plane (gx3dRay *ray, gx3dPlane *plane)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ray->direction, &ray->direction) - 1) < .01);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) - 1) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Ray_Plane (ray, plane, NULL, NULL));
}  

/*____________________________________________________________________
|
| Function: gx3d_Relation_Ray_Plane
|
| Output: Returns position of an ray relative to plane.
|
|   Returns gxRELATION_FRONT     = ray is entirely in front of plane
|           gxRELATION_BACK      = ray is entirely behind plane
|           gxRELATION_INTERSECT = ray intersects plane
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Ray_Plane (gx3dRay *ray, float ray_length, gx3dPlane *plane)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x + ray->direction.y + ray->direction.z);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) - 1) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Ray_Plane (ray, ray_length, plane, NULL, NULL));
}  

/*____________________________________________________________________
|
| Function: gx3d_Relation_Sphere_Plane
|                                                                                        
| Output: Returns position of sphere relative to plane.
|
|   Returns gxRELATION_FRONT     = sphere is in front of plane
|           gxRELATION_BACK      = sphere is behind plane
|           gxRELATION_INTERSECT = sphere intersects plane
|___________________________________________________________________*/

gxRelation gx3d_Relation_Sphere_Plane (gx3dSphere *sphere, gx3dPlane *plane)
{
  float distance;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) - 1) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  distance = gx3d_Distance_Point_Plane (&sphere->center, plane);

  if (distance > sphere->radius)
    result = gxRELATION_FRONT;
  else if (distance < -sphere->radius)
    result = gxRELATION_BACK;
  else
    result = gxRELATION_INTERSECT;

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Box_Plane
|                                                                                        
| Output: Returns position of axis-aligned box relative to plane.
|
|   Returns gxRELATION_FRONT     = box is in front of plane
|           gxRELATION_BACK      = box is behind plane
|           gxRELATION_INTERSECT = box intersects plane
|
| Notes: The box should be aligned to the same coordinate space as the
|   plane.  For example both could be in world space.  If the box and
|   the plane are in different spaces, use the other function that
|   takes as a parameter a matrix to xform the box into the same
|   coordinate space as the plane.
|
| Reference: Real-Time Rendering, 2nd ed., pg. 587
|___________________________________________________________________*/

gxRelation gx3d_Relation_Box_Plane (gx3dBox *box, gx3dPlane *plane)
{
  gx3dVector vmin, vmax;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) - 1) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Find the diagonal most closely aligned with the plane normal
  if (plane->n.x >= 0) {
    vmin.x = box->min.x;
    vmax.x = box->max.x;
  }
  else {
    vmin.x = box->max.x;
    vmax.x = box->min.x;
  }
  if (plane->n.y >= 0) {
    vmin.y = box->min.y;
    vmax.y = box->max.y;
  }
  else {
    vmin.y = box->max.y;
    vmax.y = box->min.y;
  }
  if (plane->n.z >= 0) {
    vmin.z = box->min.z;
    vmax.z = box->max.z;
  }
  else {
    vmin.z = box->max.z;
    vmax.z = box->min.z;
  }
  // Is box in front of plane?
  if (gx3d_Distance_Point_Plane (&vmin, plane) > 0)
    result = gxRELATION_FRONT;
  // Is box behind plane?
  else if (gx3d_Distance_Point_Plane (&vmax, plane) < 0)
    result = gxRELATION_BACK;
  // Must be intersecting
  else
    result = gxRELATION_INTERSECT;

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Box_Plane
|                                                                                        
| Output: Returns position of box relative to plane.  Box is an AAB 
|   but is transformed by box_transform matrix.
|
|   Returns gxRELATION_FRONT     = box is in front of plane
|           gxRELATION_BACK      = box is behind plane
|           gxRELATION_INTERSECT = box intersects plane
|
| Note: To test a box against a Frustum plane, use the view matrix for 
|   box_transform since the Frustum plane are defined in view space.
|
| Reference: Real-Time Rendering, 2nd ed., pg. 588
|___________________________________________________________________*/

gxRelation gx3d_Relation_Box_Plane (
  gx3dBox    *box, 
  gx3dMatrix *box_transform, // to transform AAB into same coordinate system as plane
  gx3dPlane  *plane )
{
  gx3dVector nt, vmin, vmax;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (box_transform);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) - 1) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Transform plane normal into coordinate system of box
  nt.x = plane->n.x * box_transform->_00 +
         plane->n.y * box_transform->_10 +
         plane->n.z * box_transform->_20 +
                      box_transform->_30;
  nt.y = plane->n.x * box_transform->_01 +
         plane->n.y * box_transform->_11 +
         plane->n.z * box_transform->_21 +
                      box_transform->_31;
  nt.z = plane->n.x * box_transform->_02 +
         plane->n.y * box_transform->_12 +
         plane->n.z * box_transform->_22 +
                      box_transform->_32;

  // Find the diagonal most closely aligned with the plane normal
  if (nt.x >= 0) {
    vmin.x = box->min.x;
    vmax.x = box->max.x;
  }
  else {
    vmin.x = box->max.x;
    vmax.x = box->min.x;
  }
  if (nt.y >= 0) {
    vmin.y = box->min.y;
    vmax.y = box->max.y;
  }
  else {
    vmin.y = box->max.y;
    vmax.y = box->min.y;
  }
  if (nt.z >= 0) {
    vmin.z = box->min.z;
    vmax.z = box->max.z;
  }
  else {
    vmin.z = box->max.z;
    vmax.z = box->min.z;
  }
  // Is box in front of plane?
  if ((gx3d_VectorDotProduct (&plane->n, &vmin) + plane->d) > 0)
    result = gxRELATION_FRONT;
  // Is box behind plane?
  else if ((gx3d_VectorDotProduct (&plane->n, &vmax) + plane->d) < 0)
    result = gxRELATION_BACK;
  // Must be intersecting
  else
    result = gxRELATION_INTERSECT;

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Triangle_Plane
|
| Output: Returns position of triangle relative to plane.
|
|   Returns gxRELATION_FRONT     = triangle is in front of plane
|           gxRELATION_BACK      = triangle is behind plane
|           gxRELATION_INTERSECT = triangle is intersecting plane
|___________________________________________________________________*/

gxRelation gx3d_Relation_Triangle_Plane (gx3dVector *vertices, gx3dPlane *plane)
{
  int i, front, back;
  bool done;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) - 1) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Count the number of points in front and in back of the plane
  for (i=0, front=0, back=0, done=false; (i<3) AND (NOT done); i++) {
    result = gx3d_Relation_Point_Plane (&vertices[i], plane, 0);
    switch (result) {
      case gxRELATION_FRONT: front++; 
                             if (back)
                               done = true;
                             break;
      case gxRELATION_BACK:    back++;  
                             if (front)
                               done = true;
                             break;
      case gxRELATION_INTERSECT:
                             done = true;
                             break;
    }
  }

  if (front == 3)
    return (gxRELATION_FRONT);
  else if (back == 3)
    return (gxRELATION_BACK);
  else
    return (gxRELATION_INTERSECT);
}  

/*____________________________________________________________________
|
| Function: gx3d_Relation_Point_Sphere
|
| Output: Returns position of a point relative to sphere.
|
|   Returns gxRELATION_OUTSIDE = point outside sphere
|           gxRELATION_INSIDE  = point is inside (or on) sphere
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Point_Sphere (gx3dVector *point, gx3dSphere *sphere)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (gx3d_DistanceSquared_Point_Point (point, &sphere->center) <= (sphere->radius * sphere->radius))
    return (gxRELATION_INSIDE);
  else
    return (gxRELATION_OUTSIDE);
} 

/*____________________________________________________________________
|
| Function: gx3d_Relation_Line_Sphere
|
| Output: Returns position of a line relative to sphere.
|
|   Returns gxRELATION_OUTSIDE   = line outside sphere
|           gxRELATION_INSIDE    = line is inside (or on) sphere
|           gxRELATION_INTERSECT = line intersects sphere 
|
| Note: Assumes 2 line endpoints are different.
|___________________________________________________________________*/

gxRelation gx3d_Relation_Line_Sphere (gx3dLine *line, gx3dSphere *sphere)
{
  float m, ray_length;
  gx3dRay ray;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (line);
  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Convert line to a ray
  ray.origin = line->start;
  gx3d_SubtractVector (&line->end, &line->start, &ray.direction);
  ray_length = gx3d_VectorMagnitude (&ray.direction);
  if (ray_length == 0)  // don't let ray_length be zero
    ray_length = EPSILON;
  m = 1 / ray_length;
  ray.direction.x *= m;  // normalize the direction vector
  ray.direction.y *= m;
  ray.direction.z *= m;

  // Test the ray
  return (gx3d_Intersect_Ray_Sphere (&ray, ray_length, sphere, NULL, NULL));
} 

/*____________________________________________________________________
|
| Function: gx3d_Relation_Ray_Sphere
|
| Output: Returns position of an infinite ray relative to sphere.
|
|   Returns gxRELATION_OUTSIDE   = ray outside sphere
|           gxRELATION_INSIDE    = ray is inside sphere
|           gxRELATION_INTERSECT = ray intersects sphere 
|
| Note: Assumes ray direction is not the zero vector.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 286.
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Ray_Sphere (gx3dRay *ray, gx3dSphere *sphere)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ray->direction, &ray->direction) - 1) < .01);
  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Ray_Sphere (ray, sphere, NULL, NULL));
} 

/*____________________________________________________________________
|
| Function: gx3d_Relation_Ray_Sphere
|
| Output: Returns position of a ray relative to sphere.
|
|   Returns gxRELATION_OUTSIDE   = ray outside sphere
|           gxRELATION_INSIDE    = ray is inside sphere
|           gxRELATION_INTERSECT = ray intersects sphere 
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Ray_Sphere (gx3dRay *ray, float ray_length, gx3dSphere *sphere)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x +
          ray->direction.y +
          ray->direction.z);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Ray_Sphere (ray, ray_length, sphere, NULL, NULL));
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Sphere_Sphere
|
| Output: Returns position of sphere1 relative to sphere2.
|
|   Returns gxRELATION_OUTSIDE   = sphere1 is outside sphere2
|           gxRELATION_INSIDE    = sphere1 is inside sphere2  (only if exact=true)
|           gxRELATION_INTERSECT = spheres intersects sphere2
|
| Note: If exact=true, return value is one of the above three.
|       If exact=false, return value will only be OUTSIDE or INTERSECT
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 288.
|___________________________________________________________________*/

gxRelation gx3d_Relation_Sphere_Sphere (gx3dSphere *s1, gx3dSphere *s2, bool exact)
{
  float d, min;
  gxRelation result;
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (s1);
  DEBUG_ASSERT (s1->radius > 0);
  DEBUG_ASSERT (s2);
  DEBUG_ASSERT (s2->radius > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  d = gx3d_DistanceSquared_Point_Point (&s1->center, &s2->center);
  min = (s1->radius + s2->radius) * (s1->radius + s2->radius);
  if (d > min)
    result = gxRELATION_OUTSIDE;
  else {
    if (exact) {
      d = sqrtf (d);
      if ((d + s1->radius) <= s2->radius)
        result = gxRELATION_INSIDE;
      else
        result = gxRELATION_INTERSECT;
    }
    else
      result = gxRELATION_INTERSECT;
  }

  return (result);
} 

/*____________________________________________________________________
|
| Function: gx3d_Relation_Box_Sphere
|
| Output: Returns position of box relative to sphere.
|
|   Returns gxRELATION_OUTSIDE   = box is outside sphere
|           gxRELATION_INTERSECT = box intersects sphere (or inside)
|
| Reference: Real-Time Rendering, 2nd ed. (pg. 599), Graphics Gems (pg. 335)
|___________________________________________________________________*/

gxRelation gx3d_Relation_Box_Sphere (gx3dBox *box, gx3dSphere *sphere)
{
  float d;
//  gx3dVector point;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Get nearest point in box to sphere center
//  gx3d_Nearest_Point_Box (&sphere->center, box, &point);
//  d = gx3d_DistanceSquared_Point_Point (&point, &sphere->center);
  d = 0;
  if (sphere->center.x < box->min.x)
    d = d + (sphere->center.x - box->min.x) * (sphere->center.x - box->min.x);
  else if (sphere->center.x > box->max.x)
    d = d + (sphere->center.x - box->max.x) * (sphere->center.x - box->max.x);
  if (sphere->center.y < box->min.y)
    d = d + (sphere->center.y - box->min.y) * (sphere->center.y - box->min.y);
  else if (sphere->center.y > box->max.y)
    d = d + (sphere->center.y - box->max.y) * (sphere->center.y - box->max.y);
  if (sphere->center.z < box->min.z)
    d = d + (sphere->center.z - box->min.z) * (sphere->center.z - box->min.z);
  else if (sphere->center.z > box->max.z)
    d = d + (sphere->center.z - box->max.z) * (sphere->center.z - box->max.z);

  if (d > (sphere->radius * sphere->radius))
    result = gxRELATION_OUTSIDE;
  else 
    result = gxRELATION_INTERSECT;

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Triangle_Sphere
|
| Output: Returns position of triangle relative to sphere.
|
|   Returns gxRELATION_OUTSIDE   = triangle is outside sphere
|           gxRELATION_INSIDE    = triangle is inside sphere
|           gxRELATION_INTERSECT = triangle intersects sphere
|___________________________________________________________________*/

gxRelation gx3d_Relation_Triangle_Sphere (gx3dVector *vertices, gx3dSphere *sphere)
{
  int i;
  gx3dLine line;
  gxRelation test[3], result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Test all three vertices
  for (i=0; i<3; i++)
    test[i] = gx3d_Relation_Point_Sphere (&vertices[i], sphere);
  // All three inside sphere?
  if ((test[0] == gxRELATION_INSIDE) AND
      (test[1] == gxRELATION_INSIDE) AND
      (test[2] == gxRELATION_INSIDE))
    result = gxRELATION_INSIDE;
  // Some inside and some outside?
  else if ((test[0] != test[1]) OR
           (test[0] != test[2]) OR
           (test[1] != test[2]))
    result = gxRELATION_INTERSECT;
  else {
    result = gxRELATION_OUTSIDE;
    for (i=0; i<3; i++) {
      line.start = vertices[i];
      line.end   = vertices[i+1%3];
      if (gx3d_Relation_Line_Sphere (&line, sphere) != gxRELATION_OUTSIDE) {
        result = gxRELATION_INTERSECT;
        break;
      }
    }
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Point_Box
|
| Output: Returns position of a point relative to an axis-aligned box.
|
|   Returns gxRELATION_OUTSIDE  = point is outside box
|           gxRELATION_INSIDE   = point is inside (or on) box
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Point_Box (gx3dVector *point, gx3dBox *box)
{
  gxRelation result;

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

  if ((point->x >= box->min.x) AND
      (point->x <= box->max.x) AND
      (point->y >= box->min.y) AND
      (point->y <= box->max.y) AND
      (point->z >= box->min.z) AND
      (point->z <= box->max.z))
    result = gxRELATION_INSIDE;
  else
    result = gxRELATION_OUTSIDE;

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Ray_Box
|
| Output: Returns position of a point relative to an axis-aligned box.
|
|   Returns gxRELATION_OUTSIDE   = ray outside box
|           gxRELATION_INSIDE    = ray origin is inside box
|           gxRELATION_INTERSECT = ray intersects box 
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Ray_Box (gx3dRay *ray, gx3dBox *box)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ray->direction, &ray->direction) - 1) < .01);
  DEBUG_ASSERT (box);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Ray_Box (ray, box, NULL, NULL));
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Ray_Box
|
| Output: Returns position of a point relative to an axis-aligned box.
|
|   Returns gxRELATION_OUTSIDE   = ray outside box
|           gxRELATION_INSIDE    = ray origin is inside box
|           gxRELATION_INTERSECT = ray intersects box 
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Ray_Box (gx3dRay *ray, float ray_length, gx3dBox *box)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x + ray->direction.y + ray->direction.z);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (box);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Ray_Box (ray, ray_length, box, NULL, NULL));
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Box_Box
|
| Output: Returns position of an AAB box relative to another AAB box.
|
|   Returns gxRELATION_OUTSIDE   = boxes do not intersect
|           gxRELATION_INTERSECT = boxes intersect
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Box_Box (gx3dBox *box1, gx3dBox *box2)
{
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box1);
  DEBUG_ASSERT (box2);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Check for no overlap (a separating axis found?)
  if ((box1->min.x > box2->max.x) OR
      (box1->max.x < box2->min.x) OR
      (box1->min.y > box2->max.y) OR
      (box1->max.y < box2->min.y) OR
      (box1->min.z > box2->max.z) OR
      (box1->max.z < box2->min.z))
    result = gxRELATION_OUTSIDE;
  else 
    result = gxRELATION_INTERSECT;

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Triangle_Box
|
| Output: Returns position of an AAB box relative to a triangle.
|
|   Returns gxRELATION_OUTSIDE   = triangle does not intersect box
|           gxRELATION_INTERSECT = triangle intersects box
|
| Reference: Real-Time Rendering, 2nd ed., pg. 596
|___________________________________________________________________*/

// X tests
#define AXISTEST_X01(a, b, fa, fb)                    \
{                                                     \
	p0 = a*v[0].y - b*v[0].z;                           \
	p2 = a*v[2].y - b*v[2].z;                           \
  if (p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * half_size.y + fb * half_size.z;          \
	if (min>rad OR max<-rad)                            \
    return (gxRELATION_OUTSIDE);                      \
}                                                     

#define AXISTEST_X2(a, b, fa, fb)                     \
{                                                     \
	p0 = a*v[0].y - b*v[0].z;                           \
	p1 = a*v[1].y - b*v[1].z;                           \
  if (p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * half_size.y + fb * half_size.z;          \
	if (min>rad OR max<-rad)                            \
    return (gxRELATION_OUTSIDE);                      \
}

// Y tests
#define AXISTEST_Y02(a, b, fa, fb)                    \
{                                                     \
	p0 = -a*v[0].x + b*v[0].z;                          \
	p2 = -a*v[2].x + b*v[2].z;                          \
  if (p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * half_size.x + fb * half_size.z;          \
	if (min>rad OR max<-rad)                            \
    return (gxRELATION_OUTSIDE);                      \
}

#define AXISTEST_Y1(a, b, fa, fb)                     \
{                                                     \
	p0 = -a*v[0].x + b*v[0].z;                          \
	p1 = -a*v[1].x + b*v[1].z;                          \
  if (p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * half_size.x + fb * half_size.z;          \
	if (min>rad OR max<-rad)                            \
    return (gxRELATION_OUTSIDE);                      \
}

// Z tests
#define AXISTEST_Z12(a, b, fa, fb)                    \
{                                                     \
	p1 = a*v[1].x - b*v[1].y;                           \
	p2 = a*v[2].x - b*v[2].y;                           \
  if (p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * half_size.x + fb * half_size.y;          \
	if (min>rad OR max<-rad)                            \
    return (gxRELATION_OUTSIDE);                      \
}

#define AXISTEST_Z0(a, b, fa, fb)                     \
{                                                     \
	p0 = a*v[0].x - b*v[0].y;                           \
	p1 = a*v[1].x - b*v[1].y;                           \
  if (p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * half_size.x + fb * half_size.y;          \
	if (min>rad OR max<-rad)                            \
    return (gxRELATION_OUTSIDE);                      \
}

gxRelation gx3d_Relation_Triangle_Box (gx3dVector *vertices, gx3dBox *box)
{
  int i;
  float p0, p1, p2, rad, min, max;
  gx3dVector center, half_size, f, v[3], e[3];
  gx3dPlane tri_plane;
  gx3dBox tri_box;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (vertices);
  DEBUG_ASSERT (box);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute the minimum AAB box for the triangle
  gx3d_GetBoundBox (&tri_box, vertices, 3);
  // Compare the two boxes
  if (gx3d_Relation_Box_Box (box, &tri_box) == gxRELATION_OUTSIDE)
    result = gxRELATION_OUTSIDE;
  else {
    // Compute the triangle normal
    gx3d_GetPlane (&vertices[0], &vertices[1], &vertices[2], &tri_plane);
    // Compare the box to the plane the triangle lies in
    if (gx3d_Relation_Box_Plane (box, &tri_plane) != gxRELATION_INTERSECT)
      result = gxRELATION_OUTSIDE;
    else {
      // Init variables
      half_size.x = (box->max.x - box->min.x) / 2;
      half_size.y = (box->max.y - box->min.y) / 2;
      half_size.z = (box->max.z - box->min.z) / 2;
      center.x    = box->min.x + half_size.x;
      center.y    = box->min.y + half_size.y;
      center.z    = box->min.z + half_size.z;
      // Translate so box center is at origin
      for (i=0; i<3; i++)
        gx3d_SubtractVector (&vertices[i], &center, &v[i]);
      // Compute triangle edges
      for (i=0; i<3; i++)
        gx3d_SubtractVector (&vertices[i+1%3], &vertices[i], &e[i]);
      // Do the 9 edge tests
      f = e[0];
      AXISTEST_X01(e[0].z, e[0].y, f.z, f.y);
      AXISTEST_Y02(e[0].z, e[0].x, f.z, f.x);
      AXISTEST_Z12(e[0].y, e[0].x, f.y, f.x);
      f = e[1];
      AXISTEST_X01(e[1].z, e[1].y, f.z, f.y);
      AXISTEST_Y02(e[1].z, e[1].x, f.z, f.x);
      AXISTEST_Z0 (e[1].y, e[1].x, f.y, f.x);
      f = e[2];
      AXISTEST_X2 (e[2].z, e[2].y, f.z, f.y);
      AXISTEST_Y1 (e[2].z, e[2].x, f.z, f.x);
      AXISTEST_Z12(e[2].y, e[2].x, f.y, f.x);

      // All outside tests failed, must intersect!
      result = gxRELATION_INTERSECT;
    }
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Ray_Triangle
|
| Output: Returns position of an infinite ray relative to a triangle.
|
|   Returns gxRELATION_OUTSIDE   = ray does not intersect triangle
|           gxRELATION_INTERSECT = ray intersects triangle
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Ray_Triangle (gx3dRay *ray, gx3dVector *vertices)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ray->direction, &ray->direction) - 1) < .01);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Ray_Triangle (ray, vertices, NULL, NULL, NULL, NULL));
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Ray_Triangle
|
| Output: Returns position of a ray relative to a triangle.
|
|   Returns gxRELATION_OUTSIDE   = ray does not intersect triangle
|           gxRELATION_INTERSECT = ray intersects triangle
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Ray_Triangle (gx3dRay *ray, float ray_length, gx3dVector *vertices)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x + ray->direction.y + ray->direction.z);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Ray_Triangle (ray, ray_length, vertices, NULL, NULL, NULL, NULL));
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Ray_TriangleFront
|
| Output: Returns position of an infinite ray relative to the front side
|   of a triangle.
|
|   Returns gxRELATION_OUTSIDE   = ray does not intersect triangle
|           gxRELATION_INTERSECT = ray intersects triangle
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Ray_TriangleFront (gx3dRay *ray, gx3dVector *vertices)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ray->direction, &ray->direction) - 1) < .01);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Ray_TriangleFront (ray, vertices, NULL, NULL, NULL, NULL));
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Ray_TriangleFront
|
| Output: Returns position of a ray relative to the front side of a 
|   triangle.
|
|   Returns gxRELATION_OUTSIDE   = ray does not intersect triangle
|           gxRELATION_INTERSECT = ray intersects triangle
|
| Note: Assumes ray direction is not the zero vector.
|___________________________________________________________________*/

inline gxRelation gx3d_Relation_Ray_TriangleFront (gx3dRay *ray, float ray_length, gx3dVector *vertices)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x + ray->direction.y + ray->direction.z);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Intersect_Ray_TriangleFront (ray, ray_length, vertices, NULL, NULL, NULL, NULL));
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Triangle_Triangle
|
| Output: Returns position of a triangle relative to another triangle.
|
|   Returns gxRELATION_OUTSIDE   = triangles do not intersect
|           gxRELATION_INTERSECT = triangles intersect
|
| Reference: Real-Time Rendering, 2nd ed., pg. 594
|___________________________________________________________________*/

gxRelation gx3d_Relation_Triangle_Triangle (gx3dVector *vertices1, gx3dVector *vertices2)
{               
  int i, index, sign[3];
  float d, distance[3];
  gx3dVector v, v1, v2;
  gx3dPlane plane2;
  gxPointF tri1[3], tri2[3], p1, p2;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (vertices1);
  DEBUG_ASSERT (vertices2);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute plane triangle 2 is in
  gx3d_GetPlane (&vertices2[0], &vertices2[1], &vertices2[2], &plane2);
  // Compute signed distances of vertices in triangle 1 to plane 2
  distance[0] = gx3d_Distance_Point_Plane (&vertices2[0], &plane2);
  distance[1] = gx3d_Distance_Point_Plane (&vertices2[1], &plane2);
  distance[2] = gx3d_Distance_Point_Plane (&vertices2[2], &plane2);
  
  // Are all vertices in triangle 1 in front of the plane?
  if (GREATER_THAN_ZERO(distance[0]) AND GREATER_THAN_ZERO(distance[1]) AND GREATER_THAN_ZERO(distance[2]))
    result = gxRELATION_OUTSIDE;
  // Are all vertices in triangle 1 in back of the plane?
  else if (LESS_THAN_ZERO(distance[0]) AND LESS_THAN_ZERO(distance[1]) AND LESS_THAN_ZERO(distance[2]))
    result = gxRELATION_OUTSIDE;
  else {
    // Are triangles coplanar?
    if (EQUAL_ZERO(distance[0]) AND EQUAL_ZERO(distance[1]) AND EQUAL_ZERO(distance[2])) {
      // Project triangles onto the axis-aligned plane where the area of triangle 2 is maximized
      if ((plane2.n.x >= plane2.n.y) AND (plane2.n.x >= plane2.n.z)) 
        // Drop x and project onto yz plane
        for (i=0; i<3; i++) {
          tri1[i].x = vertices1[i].y;
          tri1[i].y = vertices1[i].z;
          tri2[i].x = vertices2[i].y;
          tri2[i].y = vertices2[i].z;
        }
      else if ((plane2.n.y >= plane2.n.x) AND (plane2.n.y >= plane2.n.z))
        // Drop y and project onto xz plane
        for (i=0; i<3; i++) {
          tri1[i].x = vertices1[i].x;
          tri1[i].y = vertices1[i].z;
          tri2[i].x = vertices2[i].x;
          tri2[i].y = vertices2[i].z;
        }
      else
        // Drop z and project onto xy plane
        for (i=0; i<3; i++) {
          tri1[i].x = vertices1[i].x;
          tri1[i].y = vertices1[i].y;
          tri2[i].x = vertices2[i].x;
          tri2[i].y = vertices2[i].y;
        }
      // Use 2D triangle-triangle test
      result = gxRelation_Triangle_Triangle (tri1, tri2);
    }
    // Triangles are not coplanar and points of triangle 1 are on opposite sides of plane 2
    else {
      // Check signs of distances
      for (i=0; i<3; i++)
        if (distance[i] < 0)
          sign[i] = 1;
        else
          sign[i] = 0;
      // Get index (0-2) of point in triangle 1 that is on opposite side of plane as other two points
      if (sign[1] == sign[2])
        index = 0;
      else if (sign[0] == sign[2])
        index = 1;
      else
        index = 2;
      // Compute intersection points of the two edges in triangle 1 that pass through the plane
      gx3d_SubtractVector (&vertices1[index+1%3], &vertices1[index], &v);
      d = distance[index] / (distance[index] - distance[index+1%3]);
      gx3d_MultiplyScalarVector (d, &v, &v);
      gx3d_AddVector (&vertices1[index], &v, &v1);

      gx3d_SubtractVector (&vertices1[index+2%3], &vertices1[index], &v);
      d = distance[index] / (distance[index] - distance[index+2%3]);
      gx3d_MultiplyScalarVector (d, &v, &v);
      gx3d_AddVector (&vertices1[index], &v, &v2);
      // Project triangle2,v1,v2 onto the axis-aligned plane where the area of triangle 2 is maximized
      if ((plane2.n.x >= plane2.n.y) AND (plane2.n.x >= plane2.n.z)) {
        // Drop x and project onto yz plane
        for (i=0; i<3; i++) {
          tri2[i].x = vertices2[i].y;
          tri2[i].y = vertices2[i].z;
        }
        p1.x = v1.y;
        p1.y = v1.z;
        p2.x = v2.y;
        p2.y = v2.z;
      }
      else if ((plane2.n.y >= plane2.n.x) AND (plane2.n.y >= plane2.n.z)) {
        // Drop y and project onto xz plane
        for (i=0; i<3; i++) {
          tri2[i].x = vertices2[i].x;
          tri2[i].y = vertices2[i].z;
        }
        p1.x = v1.x;
        p1.y = v1.z;
        p2.x = v2.x;
        p2.y = v2.z;
      }
      else {
        // Drop z and project onto xy plane
        for (i=0; i<3; i++) {
          tri2[i].x = vertices2[i].x;
          tri2[i].y = vertices2[i].y;
        }
        p1.x = v1.x;
        p1.y = v1.y;
        p2.x = v2.x;
        p2.y = v2.y;
      }
      // Use 2D triangle-triangle test
      result = gxRelation_Line_Triangle (&p1, &p2, tri2);
    }
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Point_Frustum
|                                                                                        
| Output: Returns position of point (in world space) relative to the 
|   defautl view Frustum.
|
|   Returns gxRELATION_OUTSIDE = point outside of VF
|           gxRELATION_INSIDE  = point is entirely within VF
|
| Notes: Assumes point is in world coordinates.
|___________________________________________________________________*/

gxRelation gx3d_Relation_Point_Frustum (gx3dVector *point)
{
  float distance;
  gx3dVector point_view_pos;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  if (gx3d_View_frustum_dirty)
    gx3d_UpdateViewFrustum ();
  
/*____________________________________________________________________
|
| Test point z coordinate against front/back Frustum planes
|___________________________________________________________________*/

  // Transform z into view space
  point_view_pos.z = gx3d_View_matrix._02 * point->x +
                     gx3d_View_matrix._12 * point->y +
                     gx3d_View_matrix._22 * point->z +
                     gx3d_View_matrix._32;
  // Compute distance to near plane (distance will be positive behind near plane)
  distance = gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_NEAR].d - point_view_pos.z;
  // Behind near plane?
  if (distance > 0)
    result = gxRELATION_OUTSIDE;
  else {
    // Compute distance to far plane (distance will be positive beyond near plane)
    distance = point_view_pos.z - gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_FAR].d;
    // Beyond far plane?
    if (distance > 0)
      result = gxRELATION_OUTSIDE;
    else {

/*____________________________________________________________________
|
| Test point x coordinate against left/right Frustum planes
|___________________________________________________________________*/

      // Transform x into view space
      point_view_pos.x = gx3d_View_matrix._00 * point->x +
                         gx3d_View_matrix._10 * point->y +
                         gx3d_View_matrix._20 * point->z +
                         gx3d_View_matrix._30;
      // Compute distance to left plane (distance will be negative on left side of left plane)
      distance = (point_view_pos.x * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_LEFT].n.x) +
                 (point_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_LEFT].n.z);
      // Left of left plane?
      if (distance < 0) 
        result = gxRELATION_OUTSIDE;
      else {
        // Compute distance to right plane (distance will be negative on right side of right plane)
        distance = (point_view_pos.x * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_RIGHT].n.x) +
                   (point_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_RIGHT].n.z);
        // Right of right plane?
        if (distance < 0) 
          result = gxRELATION_OUTSIDE;
        else {

/*____________________________________________________________________
|
| Test sphere center y coordinate against top/bottom, Frustum planes
|___________________________________________________________________*/

          // Transform y into view space
          point_view_pos.y = gx3d_View_matrix._01 * point->x +
                             gx3d_View_matrix._11 * point->y +
                             gx3d_View_matrix._21 * point->z +
                             gx3d_View_matrix._31;
          // Compute distance to top plane (distance will be negative above top plane)
          distance = (point_view_pos.y * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_TOP].n.y) +
                     (point_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_TOP].n.z);
          // Above top plane?
          if (distance < 0) 
            result = gxRELATION_OUTSIDE;
          else {
            // Compute distance to bottom plane (distance will be negative below bottom plane)
            distance = (point_view_pos.y * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.y) +
                       (point_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.z);
            // Below bottom plane?
            if (distance < 0) 
              result = gxRELATION_OUTSIDE;
            else
              result = gxRELATION_INSIDE;
          }
        }
      }
    }
  }
 
  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Point_Frustum
|                                                                                        
| Output: Returns position of point (in world space) relative to a view
|   Frustum.
|
|   Returns gxRELATION_OUTSIDE = point outside of VF
|           gxRELATION_INSIDE  = point is entirely within VF
|
| Notes: Assumes point is in world coordinates.
|___________________________________________________________________*/

gxRelation gx3d_Relation_Point_Frustum (gx3dVector *point, gx3dViewFrustum *vf)
{
  float distance;
  gx3dVector point_view_pos;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (point);
  DEBUG_ASSERT (vf);

/*____________________________________________________________________
|
| Test point z coordinate against front/back Frustum planes
|___________________________________________________________________*/

  // Transform z into view space
  point_view_pos.z = gx3d_View_matrix._02 * point->x +
                     gx3d_View_matrix._12 * point->y +
                     gx3d_View_matrix._22 * point->z +
                     gx3d_View_matrix._32;
  // Compute distance to near plane (distance will be positive behind near plane)
  distance = vf->plane[gx3d_FRUSTUM_PLANE_NEAR].d - point_view_pos.z;
  // Behind near plane?
  if (distance > 0)
    result = gxRELATION_OUTSIDE;
  else {
    // Compute distance to far plane (distance will be positive beyond near plane)
    distance = point_view_pos.z - vf->plane[gx3d_FRUSTUM_PLANE_FAR].d;
    // Beyond far plane?
    if (distance > 0)
      result = gxRELATION_OUTSIDE;
    else {

/*____________________________________________________________________
|
| Test point x coordinate against left/right Frustum planes
|___________________________________________________________________*/

      // Transform x into view space
      point_view_pos.x = gx3d_View_matrix._00 * point->x +
                         gx3d_View_matrix._10 * point->y +
                         gx3d_View_matrix._20 * point->z +
                         gx3d_View_matrix._30;
      // Compute distance to left plane (distance will be negative on left side of left plane)
      distance = (point_view_pos.x * vf->plane[gx3d_FRUSTUM_PLANE_LEFT].n.x) +
                 (point_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_LEFT].n.z);
      // Left of left plane?
      if (distance < 0) 
        result = gxRELATION_OUTSIDE;
      else {
        // Compute distance to right plane (distance will be negative on right side of right plane)
        distance = (point_view_pos.x * vf->plane[gx3d_FRUSTUM_PLANE_RIGHT].n.x) +
                   (point_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_RIGHT].n.z);
        // Right of right plane?
        if (distance < 0) 
          result = gxRELATION_OUTSIDE;
        else {

/*____________________________________________________________________
|
| Test sphere center y coordinate against top/bottom, Frustum planes
|___________________________________________________________________*/

          // Transform y into view space
          point_view_pos.y = gx3d_View_matrix._01 * point->x +
                             gx3d_View_matrix._11 * point->y +
                             gx3d_View_matrix._21 * point->z +
                             gx3d_View_matrix._31;
          // Compute distance to top plane (distance will be negative above top plane)
          distance = (point_view_pos.y * vf->plane[gx3d_FRUSTUM_PLANE_TOP].n.y) +
                     (point_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_TOP].n.z);
          // Above top plane?
          if (distance < 0) 
            result = gxRELATION_OUTSIDE;
          else {
            // Compute distance to bottom plane (distance will be negative below bottom plane)
            distance = (point_view_pos.y * vf->plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.y) +
                       (point_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.z);
            // Below bottom plane?
            if (distance < 0) 
              result = gxRELATION_OUTSIDE;
            else
              result = gxRELATION_INSIDE;
          }
        }
      }
    }
  }
 
  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Sphere_Frustum
|                                                                                        
| Output: Returns position of sphere (in world space) relative to the
|   default view Frustum.
|
|   Returns gxRELATION_OUTSIDE   = sphere outside of VF
|           gxRELATION_INSIDE    = sphere is entirely within VF
|           gxRELATION_INTERSECT = sphere intersects VF
|
| Notes: Assumes sphere is in world coordinates.
|
| Reference: Tim Round, "Object Occlusion Culling", Game Programming
|   Gems, DeLoura, pp. 421-431, 2000.
|___________________________________________________________________*/

gxRelation gx3d_Relation_Sphere_Frustum (gx3dSphere *sphere)
{
  float distance;
  gx3dVector sphere_view_pos;
  gxRelation result;
  bool intersecting = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  if (gx3d_View_frustum_dirty)
    gx3d_UpdateViewFrustum ();
  
/*____________________________________________________________________
|
| Test sphere center z coordinate against front/back Frustum planes
|___________________________________________________________________*/

  // Transform z into view space
  sphere_view_pos.z = gx3d_View_matrix._02 * sphere->center.x +
                      gx3d_View_matrix._12 * sphere->center.y +
                      gx3d_View_matrix._22 * sphere->center.z +
                      gx3d_View_matrix._32;
  // Compute distance to near plane (distance will be positive behind near plane)
  distance = gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_NEAR].d - sphere_view_pos.z;
  // Behind near plane?
  if (distance > sphere->radius)
    result = gxRELATION_OUTSIDE;
	else {
    // Intersects near plane?
		if (-distance <= sphere->radius) 
			intersecting = true;
		// Compute distance to far plane (distance will be positive beyond near plane)
		distance = sphere_view_pos.z - gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_FAR].d;
		// Beyond far plane?
		if (distance > sphere->radius)
			result = gxRELATION_OUTSIDE;
		else {
  		// Intersects far plane?
			if (-distance <= sphere->radius) 
				intersecting = true;

/*____________________________________________________________________
|
| Test sphere center x coordinate against left/right Frustum planes
|___________________________________________________________________*/

      // Transform x into view space
      sphere_view_pos.x = gx3d_View_matrix._00 * sphere->center.x +
                          gx3d_View_matrix._10 * sphere->center.y +
                          gx3d_View_matrix._20 * sphere->center.z +
                          gx3d_View_matrix._30;
      // Compute distance to left plane (distance will be negative on left side of left plane)
      distance = (sphere_view_pos.x * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_LEFT].n.x) +
                 (sphere_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_LEFT].n.z);
      // Left of left plane?
      if (distance < -sphere->radius) 
        result = gxRELATION_OUTSIDE;
			else {
        // Intersects left plane?
        if (distance <= sphere->radius) 
          intersecting = true;
        // Compute distance to right plane (distance will be positive on right side of right plane)
        distance = (sphere_view_pos.x * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_RIGHT].n.x) +
                   (sphere_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_RIGHT].n.z);
        // Right of right plane?
        if (distance < -sphere->radius) 
          result = gxRELATION_OUTSIDE;
				else {
          // Intersects right plane?
          if (distance <= sphere->radius) 
            intersecting = true;

/*____________________________________________________________________
|
| Test sphere center y coordinate against top/bottom, Frustum planes
|___________________________________________________________________*/

          // Transform y into view space
          sphere_view_pos.y = gx3d_View_matrix._01 * sphere->center.x +
                              gx3d_View_matrix._11 * sphere->center.y +
                              gx3d_View_matrix._21 * sphere->center.z +
                              gx3d_View_matrix._31;
          // Compute distance to top plane (distance will be negative above top plane)
          distance = (sphere_view_pos.y * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_TOP].n.y) +
                     (sphere_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_TOP].n.z);
          // Above top plane?
          if (distance < -sphere->radius) 
            result = gxRELATION_OUTSIDE;
					else {
            // Intersects top plane?
            if (distance <= sphere->radius) 
              intersecting = true;
            // Compute distance to bottom plane (distance will be negative below bottom plane)
            distance = (sphere_view_pos.y * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.y) +
                       (sphere_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.z);
            // Below bottom plane?
            if (distance < -sphere->radius) 
              result = gxRELATION_OUTSIDE;
            else if (intersecting OR (distance <= sphere->radius))
              result = gxRELATION_INTERSECT;
            else
              result = gxRELATION_INSIDE;
          }
        }
      }
    }
  }
 
  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Sphere_Frustum
|                                                                                        
| Output: Returns position of sphere (in world space) relative to a 
|   view Frustum.
|
|   Returns gxRELATION_OUTSIDE   = sphere outside of VF
|           gxRELATION_INSIDE    = sphere is entirely within VF
|           gxRELATION_INTERSECT = sphere intersects VF
|
| Notes: Assumes sphere is in world coordinates.
|
| Reference: Tim Round, "Object Occlusion Culling", Game Programming
|   Gems, DeLoura, pp. 421-431, 2000.
|___________________________________________________________________*/

gxRelation gx3d_Relation_Sphere_Frustum (gx3dSphere *sphere, gx3dViewFrustum *vf)
{
  float distance;
  gx3dVector sphere_view_pos;
  gxRelation result;
  bool intersecting = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);
  DEBUG_ASSERT (vf);

/*____________________________________________________________________
|
| Test sphere center z coordinate against front/back Frustum planes
|___________________________________________________________________*/

  // Transform z into view space
  sphere_view_pos.z = gx3d_View_matrix._02 * sphere->center.x +
                      gx3d_View_matrix._12 * sphere->center.y +
                      gx3d_View_matrix._22 * sphere->center.z +
                      gx3d_View_matrix._32;
  // Compute distance to near plane (distance will be positive behind near plane)
  distance = vf->plane[gx3d_FRUSTUM_PLANE_NEAR].d - sphere_view_pos.z;
  // Behind near plane?
  if (distance > sphere->radius)
    result = gxRELATION_OUTSIDE;
	else {
		// Intersects near plane?
		if (-distance <= sphere->radius)
      intersecting = true;
    // Compute distance to far plane (distance will be positive beyond near plane)
    distance = sphere_view_pos.z - vf->plane[gx3d_FRUSTUM_PLANE_FAR].d;
    // Beyond far plane?
    if (distance > sphere->radius)
      result = gxRELATION_OUTSIDE;
		else {
      // Intersects far plane?
      if (-distance <= sphere->radius) 
				intersecting = true;

/*____________________________________________________________________
|
| Test sphere center x coordinate against left/right Frustum planes
|___________________________________________________________________*/

      // Transform x into view space
      sphere_view_pos.x = gx3d_View_matrix._00 * sphere->center.x +
                          gx3d_View_matrix._10 * sphere->center.y +
                          gx3d_View_matrix._20 * sphere->center.z +
                          gx3d_View_matrix._30;
      // Compute distance to left plane (distance will be negative on left side of left plane)
      distance = (sphere_view_pos.x * vf->plane[gx3d_FRUSTUM_PLANE_LEFT].n.x) +
                 (sphere_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_LEFT].n.z);
      // Left of left plane?
      if (distance < -sphere->radius) 
        result = gxRELATION_OUTSIDE;
			else {
        // Intersects left plane?
        if (distance <= sphere->radius) 
          intersecting = true;
        // Compute distance to right plane (distance will be negative on right side of right plane)
        distance = (sphere_view_pos.x * vf->plane[gx3d_FRUSTUM_PLANE_RIGHT].n.x) +
                   (sphere_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_RIGHT].n.z);
        // Right of right plane?
        if (distance < -sphere->radius) 
          result = gxRELATION_OUTSIDE;
				else {
          // Intersects right plane?
          if (distance <= sphere->radius) 
            intersecting = true;

/*____________________________________________________________________
|
| Test sphere center y coordinate against top/bottom, Frustum planes
|___________________________________________________________________*/

          // Transform y into view space
          sphere_view_pos.y = gx3d_View_matrix._01 * sphere->center.x +
                              gx3d_View_matrix._11 * sphere->center.y +
                              gx3d_View_matrix._21 * sphere->center.z +
                              gx3d_View_matrix._31;
          // Compute distance to top plane (distance will be negative above top plane)
          distance = (sphere_view_pos.y * vf->plane[gx3d_FRUSTUM_PLANE_TOP].n.y) +
                     (sphere_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_TOP].n.z);
          // Above top plane?
          if (distance < -sphere->radius) 
            result = gxRELATION_OUTSIDE;
					else {
            // Intersects top plane?
            if (distance <= sphere->radius) 
              intersecting = true;
            // Compute distance to bottom plane (distance will be negative below bottom plane)
            distance = (sphere_view_pos.y * vf->plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.y) +
                       (sphere_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.z);
            // Below bottom plane?
            if (distance < -sphere->radius) 
              result = gxRELATION_OUTSIDE;
            else if (intersecting OR (distance <= sphere->radius))
              result = gxRELATION_INTERSECT;
            else
              result = gxRELATION_INSIDE;
          }
        }
      }
    }
  }
 
  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Sphere_Frustum
|                                                                                        
| Output: Returns position of sphere (in world space) relative to the
|   default view Frustum.
|
|   Returns gxRELATION_OUTSIDE   = sphere outside of VF
|           gxRELATION_INSIDE    = sphere is entirely within VF
|           gxRELATION_INTERSECT = sphere intersects VF
|
| Notes: Assumes sphere is in world coordinates.
|
|   Doesn't check planes already determined to be inside.  Updates orientation
|   with any new planes found to be inside.
|
| Reference: Tim Round, "Object Occlusion Culling", Game Programming
|   Gems, DeLoura, pp. 421-431, 2000.
|___________________________________________________________________*/

gxRelation gx3d_Relation_Sphere_Frustum (gx3dSphere *sphere, gx3dFrustumOrientation *orientation)
{
  float distance;
  gx3dVector sphere_view_pos;
  bool intersecting = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);
  DEBUG_ASSERT (orientation);

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  if (gx3d_View_frustum_dirty)
    gx3d_UpdateViewFrustum ();
  
/*____________________________________________________________________
|
| Test sphere center z coordinate against front/back Frustum planes
|___________________________________________________________________*/

  // Transform z into view space
  sphere_view_pos.z = gx3d_View_matrix._02 * sphere->center.x +
                      gx3d_View_matrix._12 * sphere->center.y +
                      gx3d_View_matrix._22 * sphere->center.z +
                      gx3d_View_matrix._32;

  // Check only if not already known to be inside near plane
  if (orientation->inside_near == 0) {
    // Compute distance to near plane (distance will be positive behind near plane)
    distance = gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_NEAR].d - sphere_view_pos.z;
    // Behind near plane?
    if (distance > sphere->radius)
      return (gxRELATION_OUTSIDE);
    // Intersects near plane?
    else if (-distance <= sphere->radius)
      intersecting = true;
    else 
      // Update orientation
      orientation->inside_near = 1;
  }

  // Check only if not already known to be inside far plane
  if (orientation->inside_far == 0) {
    // Compute distance to far plane (distance will be positive beyond near plane)
    distance = sphere_view_pos.z - gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_FAR].d;
    // Beyond far plane?
    if (distance > sphere->radius)
      return (gxRELATION_OUTSIDE);
    // Intersects far plane?
    else if (-distance <= sphere->radius)
      intersecting = true;
    else 
      // Udpate orientation
      orientation->inside_far = 1;
  }

/*____________________________________________________________________
|
| Test sphere center x coordinate against left/right Frustum planes
|___________________________________________________________________*/

  // Check only if not already known to be inside both left and right planes
  if ((orientation->inside_left == 0) OR (orientation->inside_right == 0)) {
    // Transform x into view space
    sphere_view_pos.x = gx3d_View_matrix._00 * sphere->center.x +
                        gx3d_View_matrix._10 * sphere->center.y +
                        gx3d_View_matrix._20 * sphere->center.z +
                        gx3d_View_matrix._30;

    // Check only if not already known to be inside left plane
    if (orientation->inside_left == 0) {
      // Compute distance to left plane (distance will be negative on left side of left plane)
      distance = (sphere_view_pos.x * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_LEFT].n.x) +
                 (sphere_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_LEFT].n.z);
      // Left of left plane?
      if (distance < -sphere->radius) 
        return (gxRELATION_OUTSIDE);
      // Intersects left plane?
      else if (distance <= sphere->radius)
        intersecting = true;
      else 
        // Update orientation
        orientation->inside_left = 1;
    }

    // Check only if not already known to be inside right plane
    if (orientation->inside_right == 0) {
      // Compute distance to right plane (distance will be negative on right side of right plane)
      distance = (sphere_view_pos.x * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_RIGHT].n.x) +
                 (sphere_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_RIGHT].n.z);
      // Right of right plane?
      if (distance < -sphere->radius) 
        return (gxRELATION_OUTSIDE);
      // Intersects right plane?
      else if (distance <= sphere->radius)
        intersecting = true;
      else 
        // Update orientation
        orientation->inside_right = 1;
    }
  }

/*____________________________________________________________________
|
| Test sphere center y coordinate against top/bottom, Frustum planes
|___________________________________________________________________*/

  // Check only if not already known to be inside both top and bottom planes
  if ((orientation->inside_top == 0) OR (orientation->inside_bottom == 0)) {
    // Transform y into view space
    sphere_view_pos.y = gx3d_View_matrix._01 * sphere->center.x +
                        gx3d_View_matrix._11 * sphere->center.y +
                        gx3d_View_matrix._21 * sphere->center.z +
                        gx3d_View_matrix._31;

    // Check only if not already known to be inside top plane
    if (orientation->inside_top == 0) {
      // Compute distance to top plane (distance will be negative above top plane)
      distance = (sphere_view_pos.y * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_TOP].n.y) +
                 (sphere_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_TOP].n.z);
      // Above top plane?
      if (distance < -sphere->radius) 
        return (gxRELATION_OUTSIDE);
      // Intersects top plane?
      else if (distance <= sphere->radius)
        intersecting = true;
      else 
        // Update orientation
        orientation->inside_top = 1;
    }

    // Check only if not already known to be inside bottom plane
    if (orientation->inside_bottom == 0) {
      // Compute distance to bottom plane (distance will be negative below bottom plane)
      distance = (sphere_view_pos.y * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.y) +
                 (sphere_view_pos.z * gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.z);
      // Below bottom plane?
      if (distance < -sphere->radius) 
        return (gxRELATION_OUTSIDE);
      else if (intersecting OR (distance <= sphere->radius))
        return (gxRELATION_INTERSECT);
      else
        // Update orientation
        orientation->inside_bottom = 1;
    }
  }

  return (gxRELATION_INSIDE);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Sphere_Frustum
|                                                                                        
| Output: Returns position of sphere (in world space) relative to the
|   a view Frustum.
|
|   Returns gxRELATION_OUTSIDE   = sphere outside of VF
|           gxRELATION_INSIDE    = sphere is entirely within VF
|           gxRELATION_INTERSECT = sphere intersects VF
|
| Notes: Assumes sphere is in world coordinates.
|
|   Doesn't check planes already determined to be inside.  Updates orientation
|   with any new planes found to be inside.
|
| Reference: Tim Round, "Object Occlusion Culling", Game Programming
|   Gems, DeLoura, pp. 421-431, 2000.
|___________________________________________________________________*/

gxRelation gx3d_Relation_Sphere_Frustum (
  gx3dSphere             *sphere, 
  gx3dViewFrustum        *vf, 
  gx3dFrustumOrientation *orientation )
{
  float distance;
  gx3dVector sphere_view_pos;
  bool intersecting = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);
  DEBUG_ASSERT (vf);
  DEBUG_ASSERT (orientation);

/*____________________________________________________________________
|
| Test sphere center z coordinate against front/back Frustum planes
|___________________________________________________________________*/

  // Transform z into view space
  sphere_view_pos.z = gx3d_View_matrix._02 * sphere->center.x +
                      gx3d_View_matrix._12 * sphere->center.y +
                      gx3d_View_matrix._22 * sphere->center.z +
                      gx3d_View_matrix._32;

  // Check only if not already known to be inside near plane
  if (orientation->inside_near == 0) {
    // Compute distance to near plane (distance will be positive behind near plane)
    distance = vf->plane[gx3d_FRUSTUM_PLANE_NEAR].d - sphere_view_pos.z;
    // Behind near plane?
    if (distance > sphere->radius)
      return (gxRELATION_OUTSIDE);
    // Intersects near plane?
    else if (-distance <= sphere->radius)
      intersecting = true;
    else 
      // Update orientation
      orientation->inside_near = 1;
  }

  // Check only if not already known to be inside far plane
  if (orientation->inside_far == 0) {
    // Compute distance to far plane (distance will be positive beyond near plane)
    distance = sphere_view_pos.z - vf->plane[gx3d_FRUSTUM_PLANE_FAR].d;
    // Beyond far plane?
    if (distance > sphere->radius)
      return (gxRELATION_OUTSIDE);
    // Intersects far plane?
    else if (-distance <= sphere->radius)
      intersecting = true;
    else 
      // Udpate orientation
      orientation->inside_far = 1;
  }

/*____________________________________________________________________
|
| Test sphere center x coordinate against left/right Frustum planes
|___________________________________________________________________*/

  // Check only if not already known to be inside both left and right planes
  if ((orientation->inside_left == 0) OR (orientation->inside_right == 0)) {
    // Transform x into view space
    sphere_view_pos.x = gx3d_View_matrix._00 * sphere->center.x +
                        gx3d_View_matrix._10 * sphere->center.y +
                        gx3d_View_matrix._20 * sphere->center.z +
                        gx3d_View_matrix._30;

    // Check only if not already known to be inside left plane
    if (orientation->inside_left == 0) {
      // Compute distance to left plane (distance will be negative on left side of left plane)
      distance = (sphere_view_pos.x * vf->plane[gx3d_FRUSTUM_PLANE_LEFT].n.x) +
                 (sphere_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_LEFT].n.z);
      // Left of left plane?
      if (distance < -sphere->radius) 
        return (gxRELATION_OUTSIDE);
      // Intersects left plane?
      else if (distance <= sphere->radius)
        intersecting = true;
      else 
        // Update orientation
        orientation->inside_left = 1;
    }

    // Check only if not already known to be inside right plane
    if (orientation->inside_right == 0) {
      // Compute distance to right plane (distance will be negative on right side of right plane)
      distance = (sphere_view_pos.x * vf->plane[gx3d_FRUSTUM_PLANE_RIGHT].n.x) +
                 (sphere_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_RIGHT].n.z);
      // Right of right plane?
      if (distance < -sphere->radius) 
        return (gxRELATION_OUTSIDE);
      // Intersects right plane?
      else if (distance <= sphere->radius)
        intersecting = true;
      else 
        // Update orientation
        orientation->inside_right = 1;
    }
  }

/*____________________________________________________________________
|
| Test sphere center y coordinate against top/bottom, Frustum planes
|___________________________________________________________________*/

  // Check only if not already known to be inside both top and bottom planes
  if ((orientation->inside_top == 0) OR (orientation->inside_bottom == 0)) {
    // Transform y into view space
    sphere_view_pos.y = gx3d_View_matrix._01 * sphere->center.x +
                        gx3d_View_matrix._11 * sphere->center.y +
                        gx3d_View_matrix._21 * sphere->center.z +
                        gx3d_View_matrix._31;

    // Check only if not already known to be inside top plane
    if (orientation->inside_top == 0) {
      // Compute distance to top plane (distance will be negative above top plane)
      distance = (sphere_view_pos.y * vf->plane[gx3d_FRUSTUM_PLANE_TOP].n.y) +
                 (sphere_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_TOP].n.z);
      // Above top plane?
      if (distance < -sphere->radius) 
        return (gxRELATION_OUTSIDE);
      // Intersects top plane?
      else if (distance <= sphere->radius)
        intersecting = true;
      else 
        // Update orientation
        orientation->inside_top = 1;
    }

    // Check only if not already known to be inside bottom plane
    if (orientation->inside_bottom == 0) {
      // Compute distance to bottom plane (distance will be negative below bottom plane)
      distance = (sphere_view_pos.y * vf->plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.y) +
                 (sphere_view_pos.z * vf->plane[gx3d_FRUSTUM_PLANE_BOTTOM].n.z);
      // Below bottom plane?
      if (distance < -sphere->radius) 
        return (gxRELATION_OUTSIDE);
      else if (intersecting OR (distance <= sphere->radius))
        return (gxRELATION_INTERSECT);
      else
        // Update orientation
        orientation->inside_bottom = 1;
    }
  }

  return (gxRELATION_INSIDE);
}

/*____________________________________________________________________
|
| Function: gx3d_Relation_Box_Frustum
|                                                                                        
| Output: Returns position of sphere relative to the default view frustum.
|   Box is axis-aligned but is transformed by box_transform matrix
|   (typically a object to world transform), which can makes it an 
|   oriented box.
|
|   This function works for the oriented box since the tests occur in
|   projection space.
|
|   Returns gxRELATION_OUTSIDE   = box outside of VF
|           gxRELATION_INTERSECT = box intersects VF
|           gxRELATION_INSIDE    = box is entirely within VF
|___________________________________________________________________*/
 
#define OUT_LEFT    0x1
#define OUT_RIGHT   0x2
#define OUT_BOTTOM  0x4
#define OUT_TOP     0x8
#define OUT_NEAR    0x10
#define OUT_FAR     0x20

gxRelation gx3d_Relation_Box_Frustum (
  gx3dBox    *box, 
  gx3dMatrix *box_transform ) // to transform AAB into world space
{
  int i;
  byte outcode[8], code;
  gx3dMatrix m;
  gx3dVector4D point[8];
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (box_transform);

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  if (gx3d_View_projection_matrix_dirty)
    gx3d_UpdateViewProjectionMatrix ();

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Get matrix to transform object bounding box into projection space
  gx3d_MultiplyMatrix (box_transform, &gx3d_View_projection_matrix, &m);

  // Init points of the bounding box
  point[0].x = box->min.x; // front left bottom corner
  point[0].y = box->min.y;
  point[0].z = box->min.z;
  point[1].x = box->min.x; // front left top corner
  point[1].y = box->max.y;
  point[1].z = box->min.z;
  point[2].x = box->max.x; // front right bottom corner
  point[2].y = box->min.y;
  point[2].z = box->min.z;
  point[3].x = box->max.x; // front right top corner
  point[3].y = box->max.y;
  point[3].z = box->min.z;
  point[4].x = box->min.x; // back left bottom corner
  point[4].y = box->min.y;
  point[4].z = box->max.z;
  point[5].x = box->min.x; // back left top corner
  point[5].y = box->max.y;
  point[5].z = box->max.z;
  point[6].x = box->max.x; // back right bottom corner
  point[6].y = box->min.y;
  point[6].z = box->max.z;
  point[7].x = box->max.x; // back right top corner
  point[7].y = box->max.y;
  point[7].z = box->max.z;

  // Compute outcodes
  for (i=0; i<8; i++) {
    outcode[i] = 0;
    point[i].w = 1;
    gx3d_MultiplyVector4DMatrix (&point[i], &m, &point[i]);
    // Is point left of left clip plane?
    if (point[i].x < -point[i].w)
      outcode[i] |= OUT_LEFT;
    // Is point right of right clip plane?
    else if (point[i].x > point[i].w)
      outcode[i] |= OUT_RIGHT;
    // Is point below bottom clip plane?
    if (point[i].y < -point[i].w)
      outcode[i] |= OUT_BOTTOM;
    // Is point above top clip plane?
    else if (point[i].y > point[i].w)
      outcode[i] |= OUT_TOP;
    // Is point behind near clip plane?
    if (point[i].z < 0)
      outcode[i] |= OUT_NEAR;
    // Is point beyond far clip plane?
    else if (point[i].z > point[i].w)
      outcode[i] |= OUT_FAR;
  }

  // If nonzero, outside view Frustum
  code = outcode[0] & outcode[1] & outcode[2] & outcode[3] &
         outcode[4] & outcode[5] & outcode[6] & outcode[7];
  if (code) 
    result = gxRELATION_OUTSIDE;
  else {
    // If zero, completely inside view Frustum
    code = outcode[0] | outcode[1] | outcode[2] | outcode[3] |
           outcode[4] | outcode[5] | outcode[6] | outcode[7];
    if (code == 0)
      result = gxRELATION_INSIDE;
    else
      result = gxRELATION_INTERSECT;
  }
  
  return (result);
}

#undef OUT_LEFT
#undef OUT_RIGHT
#undef OUT_BOTTOM
#undef OUT_TOP
#undef OUT_NEAR
#undef OUT_FAR

/*____________________________________________________________________
|
| Function: gx3d_Relation_Box_Frustum
|                                                                                        
| Output: Returns position of box relative to view frustum. Box is 
|   axis-aligned in world space coordinates.
|
|   Returns gxRELATION_OUTSIDE   = box outside of VF
|           gxRELATION_INSIDE    = box is entirely within VF
|           gxRELATION_INTERSECT = box intersects VF
|___________________________________________________________________*/

// Find the diagonal most closely aligned with the plane normal
#define GET_BOX_DIAGONAL(_plane_)                          \
{                                                          \
  vmin.x = ((float *)box)[wf->box_diagonal[_plane_].minx]; \
  vmin.y = ((float *)box)[wf->box_diagonal[_plane_].miny]; \
  vmin.z = ((float *)box)[wf->box_diagonal[_plane_].minz]; \
  vmax.x = ((float *)box)[wf->box_diagonal[_plane_].maxx]; \
  vmax.y = ((float *)box)[wf->box_diagonal[_plane_].maxy]; \
  vmax.z = ((float *)box)[wf->box_diagonal[_plane_].maxz]; \
}

gxRelation gx3d_Relation_Box_Frustum (gx3dBox *box, gx3dWorldFrustum *wf)
{
  int i;
  float min_distance, max_distance;
  gx3dVector vmin, vmax;
  bool intersecting = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (wf);

/*____________________________________________________________________
|
| Test box against near/far Frustum planes
|___________________________________________________________________*/

  GET_BOX_DIAGONAL (gx3d_FRUSTUM_PLANE_NEAR)

  // Compute min distance to origin
  min_distance = gx3d_Distance_Point_Plane (&vmin, &(wf->plane[gx3d_FRUSTUM_PLANE_NEAR]));
  
  // Behind near plane?
  if (min_distance < wf->plane[gx3d_FRUSTUM_PLANE_NEAR].d)
    return (gxRELATION_OUTSIDE);
  // Compute max distance to origin
  max_distance = gx3d_Distance_Point_Plane (&vmax, &(wf->plane[gx3d_FRUSTUM_PLANE_NEAR]));
  // Intersects near plane?
  if (max_distance <= wf->plane[gx3d_FRUSTUM_PLANE_NEAR].d)
    intersecting = true;
  
  // Beyond far plane?
  if (max_distance > wf->plane[gx3d_FRUSTUM_PLANE_FAR].d)
    return (gxRELATION_OUTSIDE);
  // Intersects far plane?
  else if (min_distance >= wf->plane[gx3d_FRUSTUM_PLANE_FAR].d)
    intersecting = true;

/*____________________________________________________________________
|
| Test box against left,right,top,bottom Frustum planes
|___________________________________________________________________*/

  for (i=gx3d_FRUSTUM_PLANE_LEFT; i<gx3d_NUM_FRUSTUM_PLANES; i++) {     
    GET_BOX_DIAGONAL (i)
   // Outside plane?
   if (gx3d_Distance_Point_Plane (&vmin, &(wf->plane[i])) < 0)
     return (gxRELATION_OUTSIDE);
   // Intersects plane?
   else if (gx3d_Distance_Point_Plane (&vmax, &(wf->plane[i])) <= 0)
     intersecting = true;
  }

/*____________________________________________________________________
|
| Return result, if not already returned
|___________________________________________________________________*/

  if (intersecting)
    return (gxRELATION_INTERSECT);
  else
    return (gxRELATION_INSIDE);
}

#undef GET_BOX_DIAGONAL 

/*____________________________________________________________________
|
| Function: gx3d_Relation_Box_Frustum
|                                                                                        
| Output: Returns position of box relative to view Frustum.
|   Box is axis-aligned in world space coordinates.
|
|   Returns gxRELATION_OUTSIDE   = box outside of VF
|           gxRELATION_INSIDE    = box is entirely within VF
|           gxRELATION_INTERSECT = box intersects VF
|
| Notes: Doesn't check planes already determined to be inside. Updates
|   orientation with any new planes found to be inside.
|___________________________________________________________________*/

// Find the diagonal most closely aligned with the plane normal
#define GET_BOX_DIAGONAL(_plane_)                          \
{                                                          \
  vmin.x = ((float *)box)[wf->box_diagonal[_plane_].minx]; \
  vmin.y = ((float *)box)[wf->box_diagonal[_plane_].miny]; \
  vmin.z = ((float *)box)[wf->box_diagonal[_plane_].minz]; \
  vmax.x = ((float *)box)[wf->box_diagonal[_plane_].maxx]; \
  vmax.y = ((float *)box)[wf->box_diagonal[_plane_].maxy]; \
  vmax.z = ((float *)box)[wf->box_diagonal[_plane_].maxz]; \
}

gxRelation gx3d_Relation_Box_Frustum (gx3dBox *box, gx3dWorldFrustum *wf, gx3dFrustumOrientation *orientation)
{
  float min_distance, max_distance;
  gx3dVector vmin, vmax;
  bool intersecting = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (wf);
  DEBUG_ASSERT (orientation);

/*____________________________________________________________________
|
| Test box against near/far frustum planes
|___________________________________________________________________*/

  GET_BOX_DIAGONAL (gx3d_FRUSTUM_PLANE_NEAR)

  // Check only if not already known to be inside near or far planes
  if ((orientation->inside_near == 0) OR (orientation->inside_far == 0)) {
    // Compute min distance to origin
    min_distance = gx3d_Distance_Point_Plane (&vmin, &(wf->plane[gx3d_FRUSTUM_PLANE_NEAR]));
    // Behind near plane?
    if (min_distance < wf->plane[gx3d_FRUSTUM_PLANE_NEAR].d)
      return (gxRELATION_OUTSIDE);
    // Compute max distance to origin
    max_distance = gx3d_Distance_Point_Plane (&vmax, &(wf->plane[gx3d_FRUSTUM_PLANE_NEAR]));
    // Intersects near plane?
    if (max_distance <= wf->plane[gx3d_FRUSTUM_PLANE_NEAR].d)
      intersecting = true;
    else 
      // Update orientation
      orientation->inside_near = 1;
    
    // Beyond far plane?
    if (max_distance > wf->plane[gx3d_FRUSTUM_PLANE_FAR].d)
      return (gxRELATION_OUTSIDE);
    // Intersects far plane?
    else if (min_distance >= wf->plane[gx3d_FRUSTUM_PLANE_FAR].d)
      intersecting = true;
    else 
      // Update orientation
      orientation->inside_far = 1;
  }

/*____________________________________________________________________
|
| Test box against left frustum plane
|___________________________________________________________________*/

  // Check only if not already known to be inside plane
  if (orientation->inside_left == 0) {
    GET_BOX_DIAGONAL (gx3d_FRUSTUM_PLANE_LEFT)
    // Outside plane?
    if (gx3d_Distance_Point_Plane (&vmin, &(wf->plane[gx3d_FRUSTUM_PLANE_LEFT])) < 0)
      return (gxRELATION_OUTSIDE);
    // Intersects plane?
    else if (gx3d_Distance_Point_Plane (&vmax, &(wf->plane[gx3d_FRUSTUM_PLANE_LEFT])) <= 0)
      intersecting = true;
    // Update orientation
    else
      orientation->inside_left = 1;
  }

/*____________________________________________________________________
|
| Test box against right frustum plane
|___________________________________________________________________*/

  // Check only if not already known to be inside plane
  if (orientation->inside_right == 0) {
    GET_BOX_DIAGONAL (gx3d_FRUSTUM_PLANE_RIGHT)
    // Outside plane?
    if (gx3d_Distance_Point_Plane (&vmin, &(wf->plane[gx3d_FRUSTUM_PLANE_RIGHT])) < 0)
      return (gxRELATION_OUTSIDE);
    // Intersects plane?
    else if (gx3d_Distance_Point_Plane (&vmax, &(wf->plane[gx3d_FRUSTUM_PLANE_RIGHT])) <= 0)
      intersecting = true;
    // Update orientation
    else
      orientation->inside_right = 1;
  }

/*____________________________________________________________________
|
| Test box against top frustum plane
|___________________________________________________________________*/

  // Check only if not already known to be inside plane
  if (orientation->inside_top == 0) {
    GET_BOX_DIAGONAL (gx3d_FRUSTUM_PLANE_TOP)
    // Outside plane?
    if (gx3d_Distance_Point_Plane (&vmin, &(wf->plane[gx3d_FRUSTUM_PLANE_TOP])) < 0)
      return (gxRELATION_OUTSIDE);
    // Intersects plane?
    else if (gx3d_Distance_Point_Plane (&vmax, &(wf->plane[gx3d_FRUSTUM_PLANE_TOP])) <= 0)
      intersecting = true;
    // Update orientation
    else
      orientation->inside_top = 1;
  }

/*____________________________________________________________________
|
| Test box against bottom frustum plane
|___________________________________________________________________*/

  // Check only if not already known to be inside plane
  if (orientation->inside_bottom == 0) {
    GET_BOX_DIAGONAL (gx3d_FRUSTUM_PLANE_BOTTOM)
    // Outside plane?
    if (gx3d_Distance_Point_Plane (&vmin, &(wf->plane[gx3d_FRUSTUM_PLANE_BOTTOM])) < 0)
      return (gxRELATION_OUTSIDE);
    // Intersects plane?
    else if (gx3d_Distance_Point_Plane (&vmax, &(wf->plane[gx3d_FRUSTUM_PLANE_BOTTOM])) <= 0)
      intersecting = true;
    // Update orientation
    else
      orientation->inside_bottom = 1;
  }

/*____________________________________________________________________
|
| Return result, if not already returned
|___________________________________________________________________*/

  if (intersecting)
    return (gxRELATION_INTERSECT);
  else
    return (gxRELATION_INSIDE);
}

#undef GET_BOX_DIAGONAL 
