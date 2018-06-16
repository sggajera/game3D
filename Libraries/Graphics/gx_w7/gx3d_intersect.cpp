/*____________________________________________________________________
|
| File: gx3d_intersect.cpp
|
| Description: Functions for intersection testing between 3D objects.
|
|   Unless otherwise indicated, all functions assume the objects being 
|   tested are defined in the same coordinate system (for example: world 
|   coordinates).
|
| Functions:  gx3d_Intersect_Rect_Rect
|             gx3d_Intersect_Line_Line
|             gx3d_Intersect_Line_Plane
|             gx3d_Intersect_Ray_Ray
|             gx3d_Intersect_Ray_Ray
|             gx3d_Intersect_Ray_Ray
|             gx3d_Intersect_Ray_Plane
|             gx3d_Intersect_Ray_Plane
|             gx3d_Intersect_Ray_Sphere
|             gx3d_Intersect_Ray_Sphere
|             gx3d_Intersect_Ray_Box
|             gx3d_Intersect_Ray_Box
|             gx3d_Intersect_Ray_Triangle
|             gx3d_Intersect_Ray_Triangle
|             gx3d_Intersect_Ray_TriangleFront
|             gx3d_Intersect_Ray_TriangleFront
|             gx3d_Intersect_Box_Box
|              Min
|              Max
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
| Function prototypes
|__________________*/

static inline float Min (float f1, float f2);
static inline float Max (float f1, float f2);

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
| Function: gx3d_Intersect_Rect_Rect
|
| Output: Returns intersection of 2 rectangles.
|
|   Returns gxRELATION_INTERSECT * = rectangles intersect
|           gxRELATION_OUTSIDE     = rectangles do not intersect
|
|   * Optionally returns intersection rectangle.
|
| Notes: Rectangles are specified in the xy plane, where positive x
|   is right and positive y is up.
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Rect_Rect (gx3dRectangle *r1, gx3dRectangle *r2, gx3dRectangle *intersection_rect)
{
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (r1);
  DEBUG_ASSERT (r2);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Check for no overlap
  if ((r1->xleft   > r2->xright) OR
      (r1->xright  < r2->xleft)  OR
      (r1->ybottom > r2->ytop)   OR
      (r1->ytop    < r2->ybottom))
    result = gxRELATION_OUTSIDE;
  else {
    // Compute intersection rectangle
    if (intersection_rect) {
      intersection_rect->xleft   = Max (r1->xleft,   r2->xleft);
      intersection_rect->xright  = Min (r1->xright,  r2->xright);
      intersection_rect->ybottom = Max (r1->ybottom, r2->ybottom);
      intersection_rect->ytop    = Min (r1->ytop,    r2->ytop);
    }
    result = gxRELATION_INTERSECT;
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Line_Line
|
| Output: Returns intersection point of 2 lines.
|
|   Returns gxRELATION_INTERSECT * = intersection point is within proximity
|           gxRELATION_OUTSIDE     = lines do not intersect
|           gxRELATION_PARALLEL    = lines are parallel (could also be coincident)
|
|   * Optionally returns intersection point.
|
| Notes: A suggested proximity is 0.001.  Assumes both lines have a length.
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Line_Line (gx3dLine *l1, gx3dLine *l2, float proximity, gx3dVector *intersection)
{
  float m, r1_length, r2_length;
  gx3dRay r1, r2;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (l1);
  DEBUG_ASSERT (l2);
  DEBUG_ASSERT (proximity >= 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Convert line1 to a ray
  r1.origin = l1->start;
  gx3d_SubtractVector (&l1->end, &l1->start, &r1.direction);
  r1_length = gx3d_VectorMagnitude (&r1.direction);
  if (r1_length == 0)   // don't let length be zero
    r1_length = EPSILON;
  m = (float)1 / r1_length;
  r1.direction.x *= m;  // normalize the direction vector
  r1.direction.y *= m;
  r1.direction.z *= m;

  // Convert line2 to a ray
  r2.origin = l2->start;
  gx3d_SubtractVector (&l2->end, &l2->start, &r2.direction);
  r2_length = gx3d_VectorMagnitude (&r2.direction);
  if (r2_length == 0)   // don't let length be zero
    r2_length = EPSILON;
  m = (float)1 / r2_length;
  r2.direction.x *= m;  // normalize the direction vector
  r2.direction.y *= m;
  r2.direction.z *= m;

  // Test the rays
  return (gx3d_Intersect_Ray_Ray (&r1, r1_length, &r2, r2_length, proximity, intersection));
}

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Line_Plane
|
| Output: Returns intersection point of a line with a plane.
|
|   Returns gxRELATION_FRONT       = line is entirely in front of plane
|           gxRELATION_BACK        = line is entirely behind plane
|           gxRELATION_INTERSECT * = line intersects plane
|
|   * Optionally returns intersection point.
|
| Note: Assumes endpoints of line are different.
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Line_Plane (gx3dLine *line, gx3dPlane *plane, gx3dVector *intersection)
{
  float ray_length;
  gx3dRay ray;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (line);
  DEBUG_ASSERT (plane);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Convert line to a ray
  ray.origin = line->start;
  gx3d_SubtractVector (&line->end, &line->start, &ray.direction);
  // Normalize the direction vector
  gx3d_NormalizeVector (&(ray.direction), &(ray.direction), &ray_length);

  return (gx3d_Intersect_Ray_Plane (&ray, ray_length, plane, NULL, intersection));
}

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Ray
|
| Output: Returns intersection of two infinite rays.
|
|   Returns gxRELATION_INTERSECT * = intersection point is within proximity
|           gxRELATION_OUTSIDE     = rays do not intersect
|           gxRELATION_PARALLEL    = rays are parallel (could also be coincident)
|
|   * Optionally returns intersection point.
|
| Notes: A suggested proximity is 0.001.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 283.  
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_Ray (gx3dRay *r1, gx3dRay *r2, float proximity, gx3dVector *intersection)
{
  float dem, distance, t;
  gx3dVector cp, v1, v2, p1, p2;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (r1);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&r1->direction, &r1->direction) - 1) < .01);
  DEBUG_ASSERT (r2);                                                             
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&r2->direction, &r2->direction) - 1) < .01);
  DEBUG_ASSERT (proximity >= 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_VectorCrossProduct (&r1->direction, &r2->direction, &cp);
  if (EQUAL_ZERO(cp.x) AND EQUAL_ZERO(cp.y) AND EQUAL_ZERO(cp.z))
    result = gxRELATION_PARALLEL;
  else {
    dem = gx3d_VectorDotProduct (&cp, &cp);
    gx3d_SubtractVector (&r2->origin, &r1->origin, &v1);
    
    // Compute possible intersect point on ray 1
    gx3d_VectorCrossProduct (&v1, &r2->direction, &v2);
    t = gx3d_VectorDotProduct (&v2, &cp) / dem;
    if (t < 0)
      p1 = r1->origin;
    else {
      gx3d_MultiplyScalarVector (t, &r1->direction, &p1);
      gx3d_AddVector (&r1->origin, &p1, &p1);
    }

    // Compute possible intersect point on ray 2
    gx3d_VectorCrossProduct (&v1, &r1->direction, &v2);
    t = gx3d_VectorDotProduct (&v2, &cp) / dem;
    if (t < 0)
      p2 = r2->origin;
    else {
      gx3d_MultiplyScalarVector (t, &r2->direction, &p2);
      gx3d_AddVector (&r2->origin, &p2, &p2);
    }

    // Compute distance between these two points
    distance = gx3d_DistanceSquared_Point_Point (&v1, &v2);
    if (distance > (proximity * proximity))
      result = gxRELATION_OUTSIDE;
    else {
      if (intersection) {
        // Compute intersection point - a point between the two points on the rays
        intersection->x = v1.x + (v2.x - v1.x) / 2;
        intersection->y = v1.y + (v2.y - v1.y) / 2;
        intersection->z = v1.z + (v2.z - v1.z) / 2;
      }
      result = gxRELATION_INTERSECT;
    }
  }

  return (result);
}  

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Ray
|
| Output: Returns intersection of an infinite ray and a ray.
|
|   Returns gxRELATION_INTERSECT * = intersection point is within proximity
|           gxRELATION_OUTSIDE     = rays do not intersect
|           gxRELATION_PARALLEL    = rays are parallel (could also be coincident)
|
|   * Optionally returns distance and intersection point.
|
| Notes: A suggested proximity is 0.001.  Assumes the finite ray direction 
|   is not the zero vector.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 283.  
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_Ray (
  gx3dRay    *r1, 
  gx3dRay    *r2, 
  float       r2_length, 
  float       proximity, 
  gx3dVector *intersection )
{
  float dem, distance, t;
  gx3dVector cp, v1, v2, p1, p2;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (r1);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&r1->direction, &r1->direction) - 1) < .01);
  DEBUG_ASSERT (r2);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (r2->direction.x OR r2->direction.y OR r2->direction.z);
  DEBUG_ASSERT (r2_length > 0);
  DEBUG_ASSERT (proximity >= 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_VectorCrossProduct (&r1->direction, &r2->direction, &cp);
  if (EQUAL_ZERO(cp.x) AND EQUAL_ZERO(cp.y) AND EQUAL_ZERO(cp.z))
   result = gxRELATION_PARALLEL;
  else {
    dem = gx3d_VectorDotProduct (&cp, &cp);
    gx3d_SubtractVector (&r2->origin, &r1->origin, &v1);
    
    // Compute possible intersect point on ray 1
    gx3d_VectorCrossProduct (&v1, &r2->direction, &v2);
    t = gx3d_VectorDotProduct (&v2, &cp) / dem;
    if (t < 0)
      p1 = r1->origin;
    else {
      gx3d_MultiplyScalarVector (t, &r1->direction, &p1);
      gx3d_AddVector (&r1->origin, &p1, &p1);
    }

    // Compute possible intersect point on ray 2
    gx3d_VectorCrossProduct (&v1, &r1->direction, &v2);
    t = gx3d_VectorDotProduct (&v2, &cp) / dem;
    if (t < 0)
      p2 = r2->origin;
    else {
      if (t > r2_length)
        t = r2_length;
      gx3d_MultiplyScalarVector (t, &r2->direction, &p2);
      gx3d_AddVector (&r2->origin, &p2, &p2);
    }

    // Compute distance between these two points
    distance = gx3d_DistanceSquared_Point_Point (&v1, &v2);
    if (distance > (proximity * proximity))
      result = gxRELATION_OUTSIDE;
    else {
      if (intersection) {
        // Compute intersection point - a point between the two points on the rays
        intersection->x = v1.x + (v2.x - v1.x) / 2;
        intersection->y = v1.y + (v2.y - v1.y) / 2;
        intersection->z = v1.z + (v2.z - v1.z) / 2;
      }
      result = gxRELATION_INTERSECT;
    }
  }

  return (result);
}  

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Ray
|
| Output: Returns intersection of two rays.
|
|   Returns gxRELATION_INTERSECT * = intersection point is within proximity
|           gxRELATION_OUTSIDE     = rays do not intersect
|           gxRELATION_PARALLEL    = rays are parallel (could also be coincident)
|
|   * Optionally returns distance and intersection point.
|
| Notes: A suggested proximity is 0.001.  Assumes both ray directions are
|   not the zero vector.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 283.  
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_Ray (
  gx3dRay    *r1, 
  float       r1_length,
  gx3dRay    *r2, 
  float       r2_length, 
  float       proximity, 
  gx3dVector *intersection )
{
  float dem, distance, t;
  gx3dVector cp, v1, v2, p1, p2;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (r1);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (r1->direction.x OR r1->direction.y OR r1->direction.z);
  DEBUG_ASSERT (r1_length);
  DEBUG_ASSERT (r2);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (r2->direction.x OR r2->direction.y OR r2->direction.z);
  DEBUG_ASSERT (r2_length > 0);
  DEBUG_ASSERT (proximity >= 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_VectorCrossProduct (&r1->direction, &r2->direction, &cp);
  if (EQUAL_ZERO(cp.x) AND EQUAL_ZERO(cp.y) AND EQUAL_ZERO(cp.z))
    result = gxRELATION_PARALLEL;
  else {
    dem = gx3d_VectorDotProduct (&cp, &cp);
    gx3d_SubtractVector (&r2->origin, &r1->origin, &v1);
    
    // Compute possible intersect point on ray 1
    gx3d_VectorCrossProduct (&v1, &r2->direction, &v2);
    t = gx3d_VectorDotProduct (&v2, &cp) / dem;
    if (t < 0)
      p1 = r1->origin;
    else {
      if (t > r1_length)
        t = r1_length;
      gx3d_MultiplyScalarVector (t, &r1->direction, &p1);
      gx3d_AddVector (&r1->origin, &p1, &p1);
    }

    // Compute possible intersect point on ray 2
    gx3d_VectorCrossProduct (&v1, &r1->direction, &v2);
    t = gx3d_VectorDotProduct (&v2, &cp) / dem;
    if (t < 0)
      p2 = r2->origin;
    else {
      if (t > r2_length)
        t = r2_length;
      gx3d_MultiplyScalarVector (t, &r2->direction, &p2);
      gx3d_AddVector (&r2->origin, &p2, &p2);
    }

    // Compute distance between these two points
    distance = gx3d_DistanceSquared_Point_Point (&v1, &v2);
    if (distance > (proximity * proximity))
      result = gxRELATION_OUTSIDE;
    else {
      if (intersection) {
        // Compute intersection point - a point between the two points on the rays
        intersection->x = v1.x + (v2.x - v1.x) / 2;
        intersection->y = v1.y + (v2.y - v1.y) / 2;
        intersection->z = v1.z + (v2.z - v1.z) / 2;
        result = gxRELATION_INTERSECT;
      }
    }
  }

  return (result);
}  

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Plane
|
| Output: Returns intersection of an infinite ray with a plane.
|
|   Returns gxRELATION_FRONT *        = ray is entirely in front of plane
|           gxRELATION_BACK *         = ray is entirely behind plane
|           gxRELATION_PARALLEL       = ray is coincident and parallel to plane
|           gxRELATION_PARALLEL_FRONT = ray is in front and parallel to plane
|           gxRELATION_PARALLEL_BACK  = ray is in back and parallel to plane
|           gxRELATION_INTERSECT *    = ray intersects plane 
|
|   * Optionally returns distance and intersection point.  Distance returned 
|   can be negative if plane is behind ray.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 284.
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_Plane (
  gx3dRay    *ray, 
  gx3dPlane  *plane, 
  float      *distance, 
  gx3dVector *intersection )
{
  float dist, dot, t;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x OR ray->direction.y OR ray->direction.z);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) -1.0) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  dot = gx3d_VectorDotProduct (&ray->direction, &plane->n);
  // Is ray parallel to plane?
  if (EQUAL_ZERO(dot)) {
    result = gx3d_Relation_Point_Plane (&ray->origin, plane, 0);
    if (result == gxRELATION_FRONT)
      result = gxRELATION_PARALLEL_FRONT;
    else if (result == gxRELATION_BACK)
      result = gxRELATION_PARALLEL_BACK;
    else
      result = gxRELATION_PARALLEL;
  }
  else {
    // Compute the straight line distance from ray origin to the plane
    dist = gx3d_Distance_Point_Plane (&ray->origin, plane);
    // Is ray origin on plane?
    if (dist == 0) {
      t = 0;
      result = gxRELATION_INTERSECT;
    }
    else {
      // Compute distance along ray from ray origin to plane
      t = -dist / dot;
      // Is plane behind ray?
      if (t < 0) {
        if (dot < 0)
          result = gxRELATION_FRONT;
        else
          result = gxRELATION_BACK;
      }
      else
        result = gxRELATION_INTERSECT; // dot>0 = intersecting front, dot<0 = intersecting back
    }
  }

/*____________________________________________________________________
|
| Compute distance, intersection point
|___________________________________________________________________*/

  if ((result == gxRELATION_INTERSECT) OR (result == gxRELATION_FRONT) OR (result == gxRELATION_BACK)) {
    // Return distance?
    if (distance)
      *distance = t;
    // Return intersection point?
    if (intersection) {
      if (t == 0) 
        *intersection = ray->origin;
      else {
        gx3d_MultiplyScalarVector (t, &ray->direction, intersection);
        gx3d_AddVector (&ray->origin, intersection, intersection);
      }
    }
  }

  return (result);
}  

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Plane
|
| Output: Returns intersection of a ray with a plane.
|
|   Returns gxRELATION_FRONT *        = ray is entirely in front of plane
|           gxRELATION_BACK *         = ray is entirely behind plane
|           gxRELATION_PARALLEL       = ray is coincident and parallel to plane
|           gxRELATION_PARALLEL_FRONT = ray is in front and parallel to plane
|           gxRELATION_PARALLEL_BACK  = ray is in back and parallel to plane
|           gxRELATION_INTERSECT *    = ray intersects plane 
|
|   * Optionally returns distance and intersection point.  Distance returned 
|   can be negative if plane is behind ray.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 284.
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_Plane (
  gx3dRay    *ray, 
  float       ray_length, 
  gx3dPlane  *plane, 
  float      *distance, 
  gx3dVector *intersection )
{
  float dist, dot, t;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x OR ray->direction.y OR ray->direction.z);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) -1.0) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  dot = gx3d_VectorDotProduct (&ray->direction, &plane->n);
  // Is ray parallel to plane?
  if (EQUAL_ZERO(dot)) {
    result = gx3d_Relation_Point_Plane (&ray->origin, plane, 0);
    if (result == gxRELATION_FRONT)
      result = gxRELATION_PARALLEL_FRONT;
    else if (result == gxRELATION_BACK)
      result = gxRELATION_PARALLEL_BACK;
    else
      result = gxRELATION_PARALLEL;
  }
  else {
    // Compute the straight line distance from ray origin to the plane
    dist = gx3d_Distance_Point_Plane (&ray->origin, plane);
    // Is ray origin on plane?
    if (dist == 0) {
      t = 0;
      result = gxRELATION_INTERSECT;
    }
    else {
      // Compute distance along ray from ray origin to plane
      t = -dist / dot;
      // Is plane behind ray?
      if (t < 0) {
        if (dot < 0)
          result = gxRELATION_FRONT;
        else
          result = gxRELATION_BACK;
      }
      // Is ray too short to hit plane?
      else if (t > ray_length) {
        if (dot < 0)
          result = gxRELATION_FRONT;
        else
          result = gxRELATION_BACK;
      }
      else 
        result = gxRELATION_INTERSECT; // dot>0 = intersecting front, dot<0 = intersecting back
    }
  }

/*____________________________________________________________________
|
| Compute distance, intersection point
|___________________________________________________________________*/

  if ((result == gxRELATION_INTERSECT) OR (result == gxRELATION_FRONT) OR (result == gxRELATION_BACK)) {
    // Return distance?
    if (distance)
      *distance = t;
    // Return intersection point?
    if (intersection) {
      if (t == 0) 
        *intersection = ray->origin;
      else {
        gx3d_MultiplyScalarVector (t, &ray->direction, intersection);
        gx3d_AddVector (&ray->origin, intersection, intersection);
      }
    }
  }

  return (result);
}  

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Sphere
|
| Output: Returns intersection of an infinite ray with sphere.
|
|   Returns gxRELATION_OUTSIDE     = ray outside sphere
|           gxRELATION_INSIDE      = ray is inside sphere
|           gxRELATION_INTERSECT * = ray intersects sphere
|
|   * Optionally returns distance and intersection point.
|
| Reference: Real-Time Rendering, 2nd ed., pg. 570
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_Sphere (
  gx3dRay    *ray, 
  gx3dSphere *sphere, 
  float      *distance, 
  gx3dVector *intersection )
{
  float s, l2, r2, m2, t;
  gx3dVector l;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ray->direction, &ray->direction) -1.0) < .01);
  DEBUG_ASSERT (sphere);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_SubtractVector (&sphere->center, &ray->origin, &l);
  l2 = gx3d_VectorDotProduct (&l, &l);
  r2 = sphere->radius * sphere->radius;
  // Is ray origin inside sphere?
  if (l2 < r2)
    result = gxRELATION_INSIDE;
  else {
    s = gx3d_VectorDotProduct (&l, &ray->direction);
    if (s < 0)
      result = gxRELATION_OUTSIDE;
    else {
      // Compute the squared distance from the sphere center to the projection
      m2 = l2 - (s * s);
      if (m2 > r2)
        result = gxRELATION_OUTSIDE;
      else {
        // Ray will hit the sphere
        result = gxRELATION_INTERSECT;
        // Compute the intersection
        if (distance) {
          t = s - sqrtf (r2 - m2);
          *distance = t;
        }
        if (intersection) {
          if (distance == NULL)
            t = s - sqrtf (r2 - m2);
          gx3d_MultiplyScalarVector (t, &ray->direction, intersection);
          gx3d_AddVector (&ray->origin, intersection, intersection);
        }
      }
    }
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Sphere
|
| Output: Returns intersection of a ray with sphere.
|
|   Returns gxRELATION_OUTSIDE     = ray outside sphere
|           gxRELATION_INSIDE      = ray is inside sphere
|           gxRELATION_INTERSECT * = ray intersects sphere 
|
|   * Optionally returns distance and intersection point.
|
| Reference: Real-Time Rendering, 2nd ed., pg. 570
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_Sphere (
  gx3dRay    *ray, 
  float       ray_length,
  gx3dSphere *sphere, 
  float      *distance, 
  gx3dVector *intersection )
{
  float s, l2, r2, m2, t;
  gx3dVector l;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x OR ray->direction.y OR ray->direction.z);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (sphere);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_SubtractVector (&sphere->center, &ray->origin, &l);
  l2 = gx3d_VectorDotProduct (&l, &l);
  r2 = sphere->radius * sphere->radius;
  // Is ray origin inside sphere?
  if (l2 < r2)
    result = gxRELATION_INSIDE;
  else {
    s = gx3d_VectorDotProduct (&l, &ray->direction);
    if (s < 0)
      result = gxRELATION_OUTSIDE;
    else {
      // Compute the squared distance from the sphere center to the projection
      m2 = l2 - (s * s);
      if (m2 > r2)
        result = gxRELATION_OUTSIDE;
      else {
        t = s - sqrtf (r2 - m2);
        if (t > ray_length)
          result = gxRELATION_OUTSIDE;
        else {
          // Ray will hit the sphere
          result = gxRELATION_INTERSECT;
          // Compute the intersection
          if (distance) 
            *distance = t;
          if (intersection) {
            gx3d_MultiplyScalarVector (t, &ray->direction, intersection);
            gx3d_AddVector (&ray->origin, intersection, intersection);
          }
        }
      }
    }
  }

  return (result);
} 

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Box
|
| Output: Returns intersection of an infinite ray with an AAB box.
|
|   Returns gxRELATION_OUTSIDE     = ray outside box
|           gxRELATION_INSIDE      = ray origin is inside box
|           gxRELATION_INTERSECT * = ray intersects box 
|
|   * Optionally returns distance and intersection point.
|
| Reference: 3D Math Primer for Graphics and Games Development, pg. 307,
|   Graphics Gems I, pg. 395, 736.
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_Box (gx3dRay *ray, gx3dBox *box, float *distance, gx3dVector *intersection)
{
  int which;
  float t, xt, yt, zt;
  gx3dVector pt;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ray->direction, &ray->direction) -1.0) < .01);
  DEBUG_ASSERT (box);

/*____________________________________________________________________
|
| Is ray origin inside box?
|___________________________________________________________________*/

  if (gx3d_Relation_Point_Box (&ray->origin, box) == gxRELATION_INSIDE) 
    return (gxRELATION_INSIDE);

/*____________________________________________________________________
|
| Compute intersection, if any
|___________________________________________________________________*/

  // Compute t intersections for each x,y,z axis
  if (ray->direction.x == 0)
    xt = -1;
  else if (ray->origin.x < box->min.x) {
    pt.x = box->min.x;
    xt   = (box->min.x - ray->origin.x) / ray->direction.x;
  }
  else if (ray->origin.x > box->max.x)  {
    pt.x = box->max.x;
    xt   = (box->max.x - ray->origin.x) / ray->direction.x;
  }
  else
    xt = -1;

  if (ray->direction.y == 0)
    yt = -1;
  else if (ray->origin.y < box->min.y) {
    pt.y = box->min.y;
    yt   = (box->min.y - ray->origin.y) / ray->direction.y;
  }
  else if (ray->origin.y > box->max.y) {
    pt.y = box->max.y;
    yt   = (box->max.y - ray->origin.y) / ray->direction.y;
  }
  else
    yt = -1;

  if (ray->direction.z == 0)
    zt = -1;
  else if (ray->origin.z < box->min.z) {
    pt.z = box->min.z;
    zt   = (box->min.z - ray->origin.z) / ray->direction.z;
  }
  else if (ray->origin.z > box->max.z) {
    pt.z = box->max.z;
    zt   = (box->max.z - ray->origin.z) / ray->direction.z;
  }
  else
    zt = -1;

  // Select farthest plane - the plane of intersection
  which = 0;
  t = xt;
  if (yt > t) {
    which = 1;
    t = yt;
  }
  if (zt > t) {
    which = 2;
    t = zt;
  }

  // Is intersection point behind ray origin?
  if (t < 0)
    return (gxRELATION_OUTSIDE);

  // Compute intersection point
  switch (which) {
    case 0: // intersect with yz plane
            pt.y = ray->origin.y + (ray->direction.y * t);
            if ((pt.y < box->min.y) OR (pt.y > box->max.y))
              return (gxRELATION_OUTSIDE);
            pt.z = ray->origin.z + (ray->direction.z * t);
            if ((pt.z < box->min.z) OR (pt.z > box->max.z))
              return (gxRELATION_OUTSIDE);
            break;
    case 1: // intersect with xz plane
            pt.x = ray->origin.x + (ray->direction.x * t);
            if ((pt.x < box->min.x) OR (pt.y > box->max.x))
              return (gxRELATION_OUTSIDE);
            pt.z = ray->origin.z + (ray->direction.z * t);
            if ((pt.z < box->min.z) OR (pt.z > box->max.z))
              return (gxRELATION_OUTSIDE);
            break;
    case 2: // intersect with xy plane
            pt.x = ray->origin.x + (ray->direction.x * t);
            if ((pt.x < box->min.x) OR (pt.x > box->max.x))
              return (gxRELATION_OUTSIDE);
            pt.y = ray->origin.y + (ray->direction.y * t);
            if ((pt.y < box->min.y) OR (pt.y > box->max.y))
              return (gxRELATION_OUTSIDE);
            break;
  }
  if (distance) 
    *distance = gx3d_Distance_Point_Point (&ray->origin, &pt);
  if (intersection) 
    *intersection = pt;

  return (gxRELATION_INTERSECT);
} 

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Box
|
| Output: Returns intersection of a ray with an AAB box.
|
|   Returns gxRELATION_OUTSIDE     = ray outside box
|           gxRELATION_INSIDE      = ray origin is inside box
|           gxRELATION_INTERSECT * = ray intersects box 
|
|   * Optionally returns distance and intersection point.
|
| Note: Assumes ray direction is not the zero vector.
|
| Reference: 3D Math Primer for Graphics and Games Development, pg. 307,
|   Graphics Gems I, pg. 395, 736, Collision Detection Using Ray Casting,
|   Aug 2001 Game Developer, pg. 54.
|___________________________________________________________________*/

#define OUT_LEFT    0x1
#define OUT_RIGHT   0x2
#define OUT_BOTTOM  0x4
#define OUT_TOP     0x8
#define OUT_NEAR    0x10
#define OUT_FAR     0x20

gxRelation gx3d_Intersect_Ray_Box (gx3dRay *ray, float ray_length, gx3dBox *box, float *distance, gx3dVector *intersection)
{
  int which;
  float t, xt, yt, zt;
  gx3dVector pt;
  gx3dVector ray_end;
  byte start_code, end_code;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x OR ray->direction.y OR ray->direction.z);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (box);

/*____________________________________________________________________
|
| Is ray origin inside box?
|___________________________________________________________________*/

  if (gx3d_Relation_Point_Box (&ray->origin, box) == gxRELATION_INSIDE) {
//    debug_WriteFile ("gx3d_Intersect_Ray_Box(): ray origin inside box");
    return (gxRELATION_INSIDE);
  }

/*____________________________________________________________________
|
| Try to trivially reject ray completely outside of box
|___________________________________________________________________*/

  // Compute end point of ray
  gx3d_MultiplyScalarVector (ray_length, &(ray->direction), &ray_end);
  gx3d_AddVector (&(ray->origin), &ray_end, &ray_end);
  
  start_code = 0;
  // Is ray start point left of left clip plane?
  if (ray->origin.x < box->min.x)
    start_code |= OUT_LEFT;
  // Is ray start point right of right clip plane?
  if (ray->origin.x > box->max.x)
    start_code |= OUT_RIGHT;
  // Is ray start point below bottom clip plane?
  if (ray->origin.y < box->min.y)
    start_code |= OUT_BOTTOM;
  // Is ray start point above top clip plane?
  if (ray->origin.y > box->max.y)
    start_code |= OUT_TOP;
  // Is ray start point behind near clip plane?
  if (ray->origin.z < box->min.z)
    start_code |= OUT_NEAR;
  // Is ray start point beyond far clip plane?
  if (ray->origin.z > box->max.z)
    start_code |= OUT_FAR;

  end_code = 0;
  // Is ray end point left of left clip plane?
  if (ray_end.x < box->min.x)
    end_code |= OUT_LEFT;
  // Is ray end point right of right clip plane?
  if (ray_end.x > box->max.x)
    end_code |= OUT_RIGHT;
  // Is ray end point below bottom clip plane?
  if (ray_end.y < box->min.y)
    end_code |= OUT_BOTTOM;
  // Is ray end point above top clip plane?
  if (ray_end.y > box->max.y)
    end_code |= OUT_TOP;
  // Is ray end point behind near clip plane?
  if (ray_end.z < box->min.z)
    end_code |= OUT_NEAR;
  // Is ray end point beyond far clip plane?
  if (ray_end.z > box->max.z)
    end_code |= OUT_FAR;

  // If nonzero, ray is completely outside box (if zero, both points inside box)
  if (start_code & end_code) {
//    debug_WriteFile ("gx3d_Intersect_Ray_Box(): ray completely outside box");
    return (gxRELATION_OUTSIDE);
  }

/*____________________________________________________________________
|
| Compute intersection, if any
|___________________________________________________________________*/

  // Compute t intersections for each x,y,z axis
  if (ray->direction.x == 0)
    xt = -1;
  else if (start_code & OUT_LEFT) {
    pt.x = box->min.x;
    xt   = (box->min.x - ray->origin.x) / ray->direction.x;
  }
  else if (start_code & OUT_RIGHT)  {
    pt.x = box->max.x;
    xt   = (box->max.x - ray->origin.x) / ray->direction.x;
  }
  else
    xt = -1;

  if (ray->direction.y == 0)
    yt = -1;
  else if (start_code & OUT_BOTTOM) {
    pt.y = box->min.y;
    yt   = (box->min.y - ray->origin.y) / ray->direction.y;
  }
  else if (start_code & OUT_TOP) {
    pt.y = box->max.y;
    yt   = (box->max.y - ray->origin.y) / ray->direction.y;
  }
  else
    yt = -1;

  if (ray->direction.z == 0)
    zt = -1;
  else if (start_code & OUT_NEAR) {
    pt.z = box->min.z;
    zt   = (box->min.z - ray->origin.z) / ray->direction.z;
  }
  else if (start_code & OUT_FAR) {
    pt.z = box->max.z;
    zt   = (box->max.z - ray->origin.z) / ray->direction.z;
  }
  else
    zt = -1;

  // Select farthest plane - the plane of intersection
  which = 0;
  t = xt;
  if (yt > t) {
    which = 1;
    t = yt;
  }
  if (zt > t) {
    which = 2;
    t = zt;
  }

  // Is intersection point behind ray origin?
  if (t < 0) {
//    debug_WriteFile ("gx3d_Intersect_Ray_Box(): ray intersection point behind ray origin");
    return (gxRELATION_OUTSIDE);
  }
  // Is intersection beyond end of ray?
  else if (t > ray_length) {
//    debug_WriteFile ("gx3d_Intersect_Ray_Box(): ray intersection beyond end of ray");
    return (gxRELATION_OUTSIDE);
  }

  // Compute intersection point
  switch (which) {
    case 0: // intersect with yz plane
            pt.y = ray->origin.y + (ray->direction.y * t);
            if ((pt.y < box->min.y) OR (pt.y > box->max.y)) {
//              debug_WriteFile ("gx3d_Intersect_Ray_Box(): ray outside yz plane (1)");
              return (gxRELATION_OUTSIDE);
            }
            pt.z = ray->origin.z + (ray->direction.z * t);
            if ((pt.z < box->min.z) OR (pt.z > box->max.z)) {
//              debug_WriteFile ("gx3d_Intersect_Ray_Box(): ray outside yz plane (2)");
              return (gxRELATION_OUTSIDE);
            }
            break;
    case 1: // intersect with xz plane
            pt.x = ray->origin.x + (ray->direction.x * t);
            if ((pt.x < box->min.x) OR (pt.x > box->max.x)) {
//              debug_WriteFile ("gx3d_Intersect_Ray_Box(): ray outside xz plane (1)");
              return (gxRELATION_OUTSIDE);
            }
            pt.z = ray->origin.z + (ray->direction.z * t);
            if ((pt.z < box->min.z) OR (pt.z > box->max.z)) {
//              debug_WriteFile ("gx3d_Intersect_Ray_Box(): ray outside xz plane (2)");
              return (gxRELATION_OUTSIDE);
            }
            break;
    case 2: // intersect with xy plane
            pt.x = ray->origin.x + (ray->direction.x * t);
            if ((pt.x < box->min.x) OR (pt.x > box->max.x)) {
//              debug_WriteFile ("gx3d_Intersect_Ray_Box(): ray outside xy plane (1)");
              return (gxRELATION_OUTSIDE);
            }
            pt.y = ray->origin.y + (ray->direction.y * t);
            if ((pt.y < box->min.y) OR (pt.y > box->max.y)) {
//              debug_WriteFile ("gx3d_Intersect_Ray_Box(): ray outside xy plane (2)");
              return (gxRELATION_OUTSIDE);
            }
            break;
  }
  if (distance) 
    *distance = gx3d_Distance_Point_Point (&ray->origin, &pt);
  if (intersection) 
    *intersection = pt;

  return (gxRELATION_INTERSECT);

#undef OUT_LEFT
#undef OUT_RIGHT
#undef OUT_BOTTOM
#undef OUT_TOP
#undef OUT_NEAR
#undef OUT_FAR
} 

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Triangle
|
| Output: Returns intersection of an infinite ray with a triangle.  The
|   intersection can occur with the ray going either through the front
|   side of the triangle or the back side.  If only front side intersection
|   is needed use gx3d_Intersect_Ray_TriangleFront()
|
|   Returns gxRELATION_OUTSIDE     = ray does not intersect triangle
|           gxRELATION_INTERSECT * = ray intersects triangle
|
|   * Optionally returns distance, intersection point and barycentric
|   coords.
|
| Note: This routine could be speeded up if the plane for the traingle
|   were precomputed (see "Collision Detection Using Ray Casting", August
|   2002 Game Developer).
|
| Reference: Real-Time Rendering, 2nd ed., pg. 578
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_Triangle (
  gx3dRay    *ray, 
  gx3dVector *vertices, 
  float      *distance,
  gx3dVector *intersection,
  float      *barycentric_u,
  float      *barycentric_v )
{
  float det, inv_det, u, v, t;
  gx3dVector edge1, edge2, tvec, pvec, qvec;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ray->direction, &ray->direction) -1.0) < .01);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Find vectors for two edges sharing vert0
  gx3d_SubtractVector (&vertices[1], &vertices[0], &edge1);
  gx3d_SubtractVector (&vertices[2], &vertices[0], &edge2);

  // Begin calculating determinant - also used to calculate U parameter
  gx3d_VectorCrossProduct (&ray->direction, &edge2, &pvec);

  // If determinant is near zero, ray lies in plane of triangle
  det = gx3d_VectorDotProduct (&edge1, &pvec);
  
  // So, avoid determinants near zero
  if (EQUAL_ZERO(det))
    result = gxRELATION_OUTSIDE;
  else {
    inv_det = 1.0f / det;

    // Calculate distance from vert0 to ray origin
    gx3d_SubtractVector (&ray->origin, &vertices[0], &tvec);

    // Calculate U parameter and test bounds
    u = gx3d_VectorDotProduct (&tvec, &pvec) * inv_det;
    if ((u < 0) OR (u > 1))
      result = gxRELATION_OUTSIDE;
    else {
      // Prepare to test V parameter
      gx3d_VectorCrossProduct (&tvec, &edge1, &qvec);

      // Calculate V parameter and test bounds
      v = gx3d_VectorDotProduct (&ray->direction, &qvec) * inv_det;
      if ((v < 0) OR ((u + v) > 1))
        result = gxRELATION_OUTSIDE;
      else {
        // Ray intersects triangle so calculate t
        t = gx3d_VectorDotProduct (&edge2, &qvec) * inv_det;
        if (distance)
          *distance = t;
        // Compute intersectin point
        if (intersection) {
          gx3d_MultiplyScalarVector (t, &ray->direction, intersection);
          gx3d_AddVector (&ray->origin, intersection, intersection);
        }
        if (barycentric_u)
          *barycentric_u = u;
        if (barycentric_v)
          *barycentric_v = v;
        result = gxRELATION_INTERSECT;
      }
    }
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_Triangle
|
| Output: Returns intersection of a ray with a triangle.  The intersection
|   can occur with the ray going either through the front side of the
|   triangle or the back side.  If only front side intersection is needed
|   use gx3d_Intersect_Ray_TriangleFront()
|
|   Returns gxRELATION_OUTSIDE     = ray does not intersect triangle
|           gxRELATION_INTERSECT * = ray intersects triangle
|
|   * Optionally returns distance, intersection point and barycentric
|   coords.
|
| Note: Assumes ray direction is not the zero vector.
|
| Reference: Real-Time Rendering, 2nd ed., pg. 578
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_Triangle (
  gx3dRay    *ray, 
  float       ray_length,
  gx3dVector *vertices, 
  float      *distance,
  gx3dVector *intersection,
  float      *barycentric_u,
  float      *barycentric_v )
{
  float t;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x OR ray->direction.y OR ray->direction.z);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute intersection with an infinite ray
  result = gx3d_Intersect_Ray_Triangle (ray, vertices, &t, intersection, barycentric_u, barycentric_v);

  // If ray intersected triangle
  if (result == gxRELATION_INTERSECT) 
    // If intersection was beyond end of ray
    if (t > ray_length)
      // Then no intersection
      result = gxRELATION_OUTSIDE;

  // Compute distance to intersection?
  if (result == gxRELATION_INTERSECT)
    if (distance)
      *distance = t * ray_length;

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_TriangleFront
|
| Output: Returns intersection of an infinite ray with a triangle.  The
|   intersection can occur only with the ray going through the 
|   front side of the triangle.  If intersection testing of both sides
|   is needed use gx3d_Intersect_Ray_Triangle()
|
|   Returns gxRELATION_OUTSIDE     = ray does not intersect triangle
|           gxRELATION_INTERSECT * = ray intersects triangle
|
|   * Optionally returns distance, intersection point and barycentric
|   coords.
|
| Reference: Real-Time Rendering, 2nd ed., pg. 578
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_TriangleFront (
  gx3dRay    *ray, 
  gx3dVector *vertices, 
  float      *distance,
  gx3dVector *intersection,
  float      *barycentric_u,
  float      *barycentric_v )
{
  float det, inv_det, u, v, t;
  gx3dVector edge1, edge2, tvec, pvec, qvec;
  gxRelation result;
/*
  char str[200];
  debug_WriteFile ("__________________________________");
  debug_WriteFile ("gx3d_Intersect_Ray_TriangleFront():");
  for (int i=0; i<3; i++) {
    sprintf (str, "p[%d] = %f,%f,%f", i, vertices[i].x, vertices[i].y, vertices[i].z);
    debug_WriteFile (str);
  }
  debug_WriteFile ("(ray)");
  sprintf (str, "  origin = %f,%f,%f", ray->origin.x, ray->origin.y, ray->origin.z);
  debug_WriteFile (str);
  sprintf (str, "  direction = %f,%f,%f", ray->direction.x, ray->direction.y, ray->direction.z);
  debug_WriteFile (str);
  debug_WriteFile ("__________________________________");
*/
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ray->direction, &ray->direction) -1.0) < .01);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Find vectors for two edges sharing vert0
  gx3d_SubtractVector (&vertices[1], &vertices[0], &edge1);
  gx3d_SubtractVector (&vertices[2], &vertices[0], &edge2);

  // Begin calculating determinant - also used to calculate U parameter
  gx3d_VectorCrossProduct (&ray->direction, &edge2, &pvec);

  // If determinant is near zero, ray lies in plane of triangle
  det = gx3d_VectorDotProduct (&edge1, &pvec);
  
  // So, avoid determinants near zero or back sides of triangles
  if (det < EPSILON)
    result = gxRELATION_OUTSIDE;
  else {
    // Calculate distance from vert0 to ray origin
    gx3d_SubtractVector (&ray->origin, &vertices[0], &tvec);

    // Calculate U parameter and test bounds
    u = gx3d_VectorDotProduct (&tvec, &pvec);
    if ((u < 0) OR (u > det))
      result = gxRELATION_OUTSIDE;
    else {
      // Prepare to test V parameter
      gx3d_VectorCrossProduct (&tvec, &edge1, &qvec);

      // Calculate V parameter and test bounds
      v = gx3d_VectorDotProduct (&ray->direction, &qvec);
      if ((v < 0) OR ((u + v) > det))
        result = gxRELATION_OUTSIDE;
      else {
        // Ray intersects triangle so calculate t
        t = gx3d_VectorDotProduct (&edge2, &qvec);
        inv_det = 1 / det;
        t *= inv_det;
//        u *= inv_det;
//        v *= inv_det; 
        if (distance)
          *distance = t;
        // Compute intersectin point
        if (intersection) {
          gx3d_MultiplyScalarVector (t, &ray->direction, intersection);
          gx3d_AddVector (&ray->origin, intersection, intersection);
        }
        if (barycentric_u)
          *barycentric_u = u * inv_det;
        if (barycentric_v)
          *barycentric_v = v * inv_det;
        result = gxRELATION_INTERSECT;
      }
    }
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Ray_TriangleFront
|
| Output: Returns intersection of a ray with a triangle.  The
|   intersection can occur only with the ray going through the 
|   front side of the triangle.  If intersection testing of both sides
|   is needed use gx3d_Intersect_Ray_Triangle()
|
|   Returns gxRELATION_OUTSIDE     = ray does not intersect triangle
|           gxRELATION_INTERSECT * = ray intersects triangle
|
|   * Optionally returns distance, intersection point and barycentric
|   coords.
|
| Note: Assumes ray direction is not the zero vector.
|
| Reference: Real-Time Rendering, 2nd ed., pg. 578
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Ray_TriangleFront (
  gx3dRay    *ray, 
  float       ray_length,
  gx3dVector *vertices, 
  float      *distance,
  gx3dVector *intersection,
  float      *barycentric_u,
  float      *barycentric_v )
{
  float t;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (ray);
  // Make sure ray direction is not 0
  DEBUG_ASSERT (ray->direction.x OR ray->direction.y OR ray->direction.z);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (vertices);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute intersection with an infinite ray
  result = gx3d_Intersect_Ray_TriangleFront (ray, vertices, &t, intersection, barycentric_u, barycentric_v);

  // If ray intersected triangle
  if (result == gxRELATION_INTERSECT) 
    // If intersection was beyond end of ray
    if (t > ray_length)
      // Then no intersection
      result = gxRELATION_OUTSIDE;

  // Compute distance to intersection?
  if (result == gxRELATION_INTERSECT)
    if (distance)
      *distance = t * ray_length;

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Intersect_Box_Box
|
| Output: Returns intersection of an AAB box with an AAB box.
|
|   Returns gxRELATION_OUTSIDE     = boxes do not intersect
|           gxRELATION_INTERSECT * = boxes intersect
|
|   * Optionally returns distance and intersection point.
|
| Reference: 3D Math Primer for Graphics and Games Development, pg. 312
|___________________________________________________________________*/

gxRelation gx3d_Intersect_Box_Box (gx3dBox *box1, gx3dBox *box2, gx3dBox *intersection_box)
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

  // Check for no overlap
  if ((box1->min.x > box2->max.x) OR
      (box1->max.x < box2->min.x) OR
      (box1->min.y > box2->max.y) OR
      (box1->max.y < box2->min.y) OR
      (box1->min.z > box2->max.z) OR
      (box1->max.z < box2->min.z))
    result = gxRELATION_OUTSIDE;
  else {
    // Compute intersection box
    if (intersection_box) {
      intersection_box->min.x = Max (box1->min.x, box2->min.x);
      intersection_box->max.x = Min (box1->max.x, box2->max.x);
      intersection_box->min.y = Max (box1->min.y, box2->min.y);
      intersection_box->max.y = Min (box1->max.y, box2->max.y);
      intersection_box->min.z = Max (box1->min.z, box2->min.z);
      intersection_box->max.z = Min (box1->max.z, box2->max.z);
    }
    result = gxRELATION_INTERSECT;
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: Min
|
| Input: Called from gx3d_Intersect_Box_Box()
| Output: Returns minimum of two values.
|___________________________________________________________________*/

static inline float Min (float f1, float f2)
{
  if (f1 < f2)
    return (f1);
  else
    return (f2);
}

/*____________________________________________________________________
|
| Function: Max
|
| Input: Called from gx3d_Intersect_Box_Box()
| Output: Returns maximum of two values.
|___________________________________________________________________*/

static inline float Max (float f1, float f2)
{
  if (f1 > f2)
    return (f1);
  else
    return (f2);
}
