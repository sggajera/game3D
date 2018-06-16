/*____________________________________________________________________
|
| File: gx3d_collide.cpp
|
| Description: Functions for dynamic intersection testing.
|
|   Unless otherwise indicated, all functions assume the objects being 
|   tested are defined in the same coordinate system (for example: world 
|   coordinates).
|
|   There are two general types of functions.  The first set is moving 
|   objects colliding with static objects.  The second set is moving
|   objects colliding with moving objects.  Within each of these two sets
|   there are two overloaded functions for each object pair.  One function
|   will accept trajectory/s and a (delta) time.  The other function will
|   accept a projected trajectory (vector that encodes direction & velocity).
|
| Functions:  gx3d_Collide_Sphere_StaticPlane
|             gx3d_Collide_Sphere_StaticPlane
|             gx3d_Collide_Sphere_StaticSphere
|             gx3d_Collide_Sphere_StaticSphere
|             gx3d_Collide_Sphere_Sphere
|             gx3d_Collide_Sphere_Sphere
|             gx3d_Collide_Sphere_StaticBox       // not done (see pg. 224 Math for 3D Game Programmers ...)
|             gx3d_Collide_Sphere_StaticBox       // not done
|             gx3d_Collide_Sphere_Box             // not done
|             gx3d_Collide_Sphere_Box             // not done
|             gx3d_Collide_Sphere_StaticTriangle  // not done (see raycasting article in aug.02 GD)
|             gx3d_Collide_Sphere_StaticTriangle  // not done ( & pg. 624 Real-Time Rendering, 2nd ed.)
|             gx3d_Collide_Box_StaticPlane
|             gx3d_Collide_Box_StaticPlane
|             gx3d_Collide_Box_StaticSphere       // not done
|             gx3d_Collide_Box_StaticSphere       // not done
|             gx3d_Collide_Box_StaticBox
|             gx3d_Collide_Box_StaticBox
|             gx3d_Collide_Box_Box
|             gx3d_Collide_Box_Box
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
| Function: gx3d_Collide_Sphere_StaticPlane
|
| Output: Returns collision info regarding a sphere moving relative to
|   a static plane.  Returns relation and optionally, the parametric 
|   collision time in the range 0-1.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = sphere collides with plane
|
| Reference: Real Time Rendering, 2nd ed., pg. 621
|___________________________________________________________________*/

gxRelation gx3d_Collide_Sphere_StaticPlane (
  gx3dSphere     *sphere,
  gx3dTrajectory *trajectory,
  float           dtime,                      // period of time sphere moves
  gx3dPlane      *plane,
  float          *parametric_collision_time ) // NULL if not needed
{
  float sc, se;
  gx3dVector end;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);
  DEBUG_ASSERT (trajectory);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&trajectory->direction, &trajectory->direction) -1.0) < .01);
  DEBUG_ASSERT (dtime > 0);
  DEBUG_ASSERT (plane);
  // Make sure normals are normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) -1.0) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // end = sphere_center + (velocity * dtime)
  gx3d_MultiplyScalarVector (trajectory->velocity * dtime, &trajectory->direction, &end);
  gx3d_AddVector (&end, &sphere->center, &end);

  // Compute distance from sphere center to plane at start of movement
  sc = gx3d_Distance_Point_Plane (&sphere->center, plane);
  // Compute distance from sphere center to plane at end of movement
  se = gx3d_Distance_Point_Plane (&end, plane);

  // No intersection?
  if (((sc * se) > 0) AND (sc > sphere->radius) AND (se > sphere->radius))
    result = gxRELATION_OUTSIDE;
  else {
    if (parametric_collision_time) 
      *parametric_collision_time = (sc - sphere->radius) / (sc - se);
    result = gxRELATION_INTERSECT;
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Collide_Sphere_StaticPlane
|
| Output: Returns collision info regarding a sphere moving relative to
|   a static plane.  Returns relation and optionally, the parametrric 
|   collision time in the range 0-1.
|
|   The movement being calculated occurs over a default 1 time unit.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = sphere collides with plane
|
| Reference: Real Time Rendering, 2nd ed., pg. 621
|___________________________________________________________________*/

gxRelation gx3d_Collide_Sphere_StaticPlane (
  gx3dSphere              *sphere,
  gx3dProjectedTrajectory *ptrajectory,
  gx3dPlane               *plane,
  float                   *parametric_collision_time )  // NULL if not needed
{
  gx3dTrajectory trajectory;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);
  DEBUG_ASSERT (ptrajectory);
  // Make sure trajectory direction is not 0
  DEBUG_ASSERT (ptrajectory->direction.x + ptrajectory->direction.y + ptrajectory->direction.z);
  DEBUG_ASSERT (plane);
  // Make sure normals are normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) -1.0) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/
  
  // Extract normal and velocity from projected trajectory (time is assumed to be 1 unit)
  gx3d_NormalizeVector (&ptrajectory->direction, &trajectory.direction, &trajectory.velocity);
  
  return (gx3d_Collide_Sphere_StaticPlane (sphere, &trajectory, 1, plane, parametric_collision_time));
}

/*____________________________________________________________________
|
| Function: gx3d_Collide_Sphere_StaticSphere
|
| Output: Returns collision info regarding a moving sphere relative to
|   a static sphere.  Returns relation and optionally, collision time
|   in the range 0-1.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = sphere1 collides with sphere2
|
| Reference: Real Time Rendering, 2nd ed., pg. 622
|___________________________________________________________________*/

gxRelation gx3d_Collide_Sphere_StaticSphere (
  gx3dSphere     *sphere,
  gx3dTrajectory *trajectory1,
  float           dtime,                      // period of time spheres move
  gx3dSphere     *static_sphere,
  float          *parametric_collision_time ) // NULL if not needed
{
  float ray_length, distance;
  gx3dRay ray;
  gx3dSphere big_static_sphere;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);
  DEBUG_ASSERT (trajectory1);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&trajectory1->direction, &trajectory1->direction) -1.0) < .01);
  DEBUG_ASSERT (static_sphere);
  DEBUG_ASSERT (static_sphere->radius > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute the ray of movement of sphere1 relative to sphere2
  ray.origin    = sphere->center;
  ray.direction = trajectory1->direction;
  ray_length    = trajectory1->velocity * dtime;

  // Enlarge static sphere
  big_static_sphere.center = static_sphere->center;
  big_static_sphere.radius = static_sphere->radius + sphere->radius;

  // Use ray sphere test
  if (parametric_collision_time) 
    result = gx3d_Intersect_Ray_Sphere (&ray, ray_length, &big_static_sphere, &distance, NULL);
  else
    result = gx3d_Intersect_Ray_Sphere (&ray, ray_length, &big_static_sphere, NULL, NULL);
  
  // Ignore 'inside' results
  if (result != gxRELATION_INTERSECT)
    result = gxRELATION_OUTSIDE;

  // Compute intersection time?
  if (parametric_collision_time)
    *parametric_collision_time = distance / ray_length;

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Collide_Sphere_StaticSphere
|
| Output: Returns collision info regarding a moving sphere relative to
|   a static sphere.  Returns relation and optionally, collision time
|   in the range 0-1.
|
|   The movement being calculated occurs over a default 1 time unit.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = sphere1 collides with sphere2
|
| Reference: Real Time Rendering, 2nd ed., pg. 622
|___________________________________________________________________*/

gxRelation gx3d_Collide_Sphere_StaticSphere (
  gx3dSphere              *sphere,
  gx3dProjectedTrajectory *ptrajectory1,
  gx3dSphere              *static_sphere,
  float                   *parametric_collision_time )  // NULL if not needed
{
  gx3dTrajectory trajectory;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere);
  DEBUG_ASSERT (sphere->radius > 0);
  DEBUG_ASSERT (ptrajectory1);
  // Make sure trajectory direction is not 0
  DEBUG_ASSERT (ptrajectory1->direction.x + ptrajectory1->direction.y + ptrajectory1->direction.z);
  DEBUG_ASSERT (static_sphere);
  DEBUG_ASSERT (static_sphere->radius > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/
  
  // Extract normal and velocity from projected trajectory (time is assumed to be 1 unit)
  gx3d_NormalizeVector (&ptrajectory1->direction, &trajectory.direction, &trajectory.velocity);
  
  return (gx3d_Collide_Sphere_StaticSphere (sphere, &trajectory, 1, static_sphere, parametric_collision_time));
}

/*____________________________________________________________________
|
| Function: gx3d_Collide_Sphere_Sphere
|
| Output: Returns collision info regarding a moving sphere relative to
|   another moving sphere.  Returns relation and optionally, collision 
|   time in the range 0-1.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = sphere1 collides with sphere2
|
| Reference: Real Time Rendering, 2nd ed., pg. 622
|___________________________________________________________________*/

gxRelation gx3d_Collide_Sphere_Sphere (
  gx3dSphere     *sphere1,
  gx3dTrajectory *trajectory1,
  float           dtime,                      // period of time spheres move
  gx3dSphere     *sphere2,
  gx3dTrajectory *trajectory2,
  float          *parametric_collision_time ) // NULL if not needed
{
  float ray_length, distance;
  gx3dVector dv1, dv2;
  gx3dRay ray;
  gx3dSphere big_sphere2;
  gx3dSphere *s_temp;
  gx3dTrajectory *t_temp;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere1);
  DEBUG_ASSERT (sphere1->radius > 0);
  DEBUG_ASSERT (trajectory1);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&trajectory1->direction, &trajectory1->direction) -1.0) < .01);
  DEBUG_ASSERT (dtime > 0);
  DEBUG_ASSERT (sphere2);
  DEBUG_ASSERT (sphere2->radius > 0);
  DEBUG_ASSERT (trajectory2);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&trajectory2->direction, &trajectory2->direction) -1.0) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Set sphere1 = fastest moving sphere
  if (trajectory1->velocity < trajectory2->velocity) {
    s_temp      = sphere1;
    sphere1     = sphere2;
    sphere2     = s_temp;
    t_temp      = trajectory1;
    trajectory1 = trajectory2;
    trajectory2 = t_temp;
  }
  
  // Compute projected directions for both moving spheres
  gx3d_MultiplyScalarVector (trajectory1->velocity * dtime, &trajectory1->direction, &dv1);
  gx3d_MultiplyScalarVector (trajectory2->velocity * dtime, &trajectory2->direction, &dv2);

  // Compute the ray of movement of sphere1 relative to sphere2
  ray.origin = sphere1->center;
  gx3d_SubtractVector (&dv1, &dv2, &ray.direction);
  gx3d_NormalizeVector (&ray.direction, &ray.direction, &ray_length);

  // Enlarge sphere2
  big_sphere2.center = sphere2->center;
  big_sphere2.radius = sphere2->radius + sphere1->radius;

  // Use ray sphere test
  if (parametric_collision_time) 
    result = gx3d_Intersect_Ray_Sphere (&ray, ray_length, &big_sphere2, &distance, NULL);
  else
    result = gx3d_Intersect_Ray_Sphere (&ray, ray_length, &big_sphere2, NULL, NULL);

  // Ignore 'inside' results
  if (result != gxRELATION_INTERSECT)
    result = gxRELATION_OUTSIDE;

  // Compute intersection time?
  if (parametric_collision_time)
    *parametric_collision_time = distance / ray_length;

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Collide_Sphere_Sphere
|
| Output: Returns collision info regarding a moving sphere relative to
|   another moving sphere.  Returns relation and optionally, collision 
|   time in the range 0-1.
|
|   The movement being calculated occurs over a default 1 time unit.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = sphere1 collides with sphere2
|
| Reference: Real Time Rendering, 2nd ed., pg. 622
|___________________________________________________________________*/

gxRelation gx3d_Collide_Sphere_Sphere (
  gx3dSphere              *sphere1,
  gx3dProjectedTrajectory *ptrajectory1,
  gx3dSphere              *sphere2,
  gx3dProjectedTrajectory *ptrajectory2,
  float                   *parametric_collision_time )  // NULL if not needed
{
  gx3dTrajectory trajectory1, trajectory2;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sphere1);
  DEBUG_ASSERT (sphere1->radius > 0);
  DEBUG_ASSERT (ptrajectory1);
  // Make sure trajectory direction is not 0
  DEBUG_ASSERT (ptrajectory1->direction.x + ptrajectory1->direction.y + ptrajectory1->direction.z);
  DEBUG_ASSERT (sphere2);
  DEBUG_ASSERT (sphere2->radius > 0);
  DEBUG_ASSERT (ptrajectory2);
  // Make sure trajectory direction is not 0
  DEBUG_ASSERT (ptrajectory2->direction.x + ptrajectory2->direction.y + ptrajectory2->direction.z);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/
  
  // Extract normal and velocity from projected trajectories
  gx3d_NormalizeVector (&ptrajectory1->direction, &trajectory1.direction, &trajectory1.velocity);
  gx3d_NormalizeVector (&ptrajectory2->direction, &trajectory2.direction, &trajectory2.velocity);
  
  return (gx3d_Collide_Sphere_Sphere (sphere1, &trajectory1, 1, sphere2, &trajectory2, parametric_collision_time));
}

/*____________________________________________________________________
|
| Function: gx3d_Collide_Box_StaticPlane
|
| Output: Returns collision info regarding a AAA box moving relative to
|   a static plane.  Returns relation and optionally, the parametric 
|   collision time in the range 0-1.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = box collides with plane
|
| Reference: 3D Math for Graphics and Games Development (pg. 310, 284),
|   Real-Time Rendering, 2nd ed. (pg. 587)
|___________________________________________________________________*/

gxRelation gx3d_Collide_Box_StaticPlane (
  gx3dBox        *box,
  gx3dTrajectory *trajectory,
  float           dtime,                      // period of time box moves
  gx3dPlane      *plane,
  float          *parametric_collision_time ) // NULL if not needed
{
  float t, dot, mind, maxd;
  gx3dVector vmin, vmax;
  gxRelation result;
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (trajectory);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&trajectory->direction, &trajectory->direction) - 1) < .01);
  DEBUG_ASSERT (dtime > 0);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) - 1) < .01);

/*____________________________________________________________________
|
| Find the diagonal most closely aligned with the plane normal (see
|   gxRelation_Box_Plane())
|___________________________________________________________________*/

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
  mind = gx3d_Distance_Point_Plane (&vmin, plane);
  maxd = gx3d_Distance_Point_Plane (&vmax, plane);
                 
  // Compute angle between trajectory and plane normal
  dot = gx3d_VectorDotProduct (&trajectory->direction, &plane->n);

/*____________________________________________________________________
|
| Process a box moving parallel to plane
|___________________________________________________________________*/

  if (EQUAL_ZERO(dot)) {
    // Is box already intersecting plane?
    if ((mind * maxd) <= 0) {
      result = gxRELATION_INTERSECT;
      t = 0;
    }
    else
      result = gxRELATION_OUTSIDE;
  }
  else {

/*____________________________________________________________________
|
| Process a box in front of plane (and not moving parallel to plane)
|___________________________________________________________________*/

    // Is vmin in front of plane?
    if (mind > 0) { 
      // Compute intersection of ray starting at vmin, going in trajectory->direction, with plane
      t = -mind / dot;
      // Is box moving away from plane?
      if (t < 0) 
        result = gxRELATION_OUTSIDE;
      else 
        result = gxRELATION_INTERSECT;
    }

/*____________________________________________________________________
|
| Process a box in back of plane
|___________________________________________________________________*/

    // Is vmax in back of plane?
    else if (maxd < 0) { 
      // Compute intersection of ray starting at vmin, going in trajectory->direction, with plane
      t = -maxd / dot;
      // Is box moving away from plane?
      if (t < 0) 
        result = gxRELATION_OUTSIDE;
      else 
        result = gxRELATION_INTERSECT;
    }

/*____________________________________________________________________
|
| Process a box already touching plane
|___________________________________________________________________*/

    else 
      result = gxRELATION_INTERSECT;
  }

/*____________________________________________________________________
|
| Scale collision time by velocity to get a value between 0-1
|___________________________________________________________________*/

  if (result == gxRELATION_INTERSECT) {
    t /= (trajectory->velocity * dtime);
    if (t > 1)
      result = gxRELATION_OUTSIDE;
    else if (parametric_collision_time)
      *parametric_collision_time = t;
  }

  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Collide_Box_StaticPlane
|
| Output: Returns collision info regarding a AAA box moving relative to
|   a static plane.  Returns relation and optionally, the parametric 
|   collision time in the range 0-1.
|
|   The movement being calculated occurs over a default 1 time unit.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = box collides with plane
|
| Reference: 3D Math for Graphics and Games Development (pg. 310, 284),
|   Real-Time Rendering, 2nd ed. (pg. 587)
|___________________________________________________________________*/

gxRelation gx3d_Collide_Box_StaticPlane (
  gx3dBox                 *box,
  gx3dProjectedTrajectory *ptrajectory,
  gx3dPlane               *plane,
  float                   *parametric_collision_time ) // NULL if not needed
{
  gx3dTrajectory trajectory;
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (ptrajectory);
  // Make sure trajectory direction is not 0
  DEBUG_ASSERT (ptrajectory->direction.x + ptrajectory->direction.y + ptrajectory->direction.z);
  DEBUG_ASSERT (plane);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&plane->n, &plane->n) -1.0) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Extract normal and velocity from projected trajectory (time is assumed to be 1 unit)
  gx3d_NormalizeVector (&ptrajectory->direction, &trajectory.direction, &trajectory.velocity);
  
  return (gx3d_Collide_Box_StaticPlane (box, &trajectory, 1, plane, parametric_collision_time));
}

/*____________________________________________________________________
|
| Function: gx3d_Collide_Box_StaticBox
|
| Output: Returns collision info regarding a AAA box moving relative to
|   a static AAB box.  Returns relation and optionally, the parametric 
|   collision time in the range 0-1.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = box collides with box
|
| Reference: 3D Math for Graphics and Games Development (pg. 313)
|___________________________________________________________________*/

gxRelation gx3d_Collide_Box_StaticBox (
  gx3dBox        *box,
  gx3dTrajectory *trajectory,
  float           dtime,                      // period of time box moves
  gx3dBox        *static_box,
  float          *parametric_collision_time ) // NULL if not needed
{
  float f, t_enter, t_leave, enter, leave;
  gx3dProjectedTrajectory ptrajectory;
  gxRelation result = gxRELATION_INTERSECT; // assume intersecting
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (trajectory);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&trajectory->direction, &trajectory->direction) - 1) < .01);
  DEBUG_ASSERT (dtime > 0);
  DEBUG_ASSERT (static_box);

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Compute the projected trajectory
  gx3d_MultiplyScalarVector (trajectory->velocity * dtime, &trajectory->direction, &ptrajectory.direction);
  
  // Initialize interval to contain all the time under consideration
  t_enter = 0;
  t_leave = 1;

/*____________________________________________________________________
|
| Compute interval of overlap on each dimension, and intersect this 
| interval with the interval accumulated so far.  When an empty interval
| is found, return a negative result (no intersection).  Look out for an
| infinite or empty interval of each dimension.
|___________________________________________________________________*/
  
  // Check x-axis
  if (result != gxRELATION_OUTSIDE) {
    // Empty or infinite interval on x?
    if (ptrajectory.direction.x == 0) {
      // Empty interval on x?
      if ((static_box->min.x >= box->max.x) OR
          (static_box->max.x <= box->min.x)) 
        result = gxRELATION_OUTSIDE;
    }
    else {
      // Compute the time value when they begin and end overlap
      f = 1 / ptrajectory.direction.x;
      enter = (static_box->min.x - box->max.x) * f;
      leave = (static_box->max.x - box->min.x) * f;
      // Check for interval out of order
      if (enter > leave) {
        f = enter;
        enter = leave;
        leave = f;
      }
      // Update interval
      if (enter > t_enter) 
        t_enter = enter;
      if (leave > t_leave)
        t_leave = leave;
      // Check if this resulted in empty interval
      if (t_enter > t_leave)
        result = gxRELATION_OUTSIDE;
    }
  }

  // Check y-axis
  if (result != gxRELATION_OUTSIDE) {
    // Empty or infinite interval on y?
    if (ptrajectory.direction.y == 0) {
      // Empty interval on y?
      if ((static_box->min.y >= box->max.y) OR
          (static_box->max.y <= box->min.y)) 
        result = gxRELATION_OUTSIDE;
    }
    else {
      // Compute the time value when they begin and end overlap
      f = 1 / ptrajectory.direction.y;
      enter = (static_box->min.y - box->max.y) * f;
      leave = (static_box->max.y - box->min.y) * f;
      // Check for interval out of order
      if (enter > leave) {
        f = enter;
        enter = leave;
        leave = f;
      }
      // Update interval
      if (enter > t_enter) 
        t_enter = enter;
      if (leave > t_leave)
        t_leave = leave;
      // Check if this resulted in empty interval
      if (t_enter > t_leave)
        result = gxRELATION_OUTSIDE;
    }
  }

  // Check z-axis
  if (result != gxRELATION_OUTSIDE) {
    // Empty or infinite interval on z?
    if (ptrajectory.direction.z == 0) {
      // Empty interval on z?
      if ((static_box->min.z >= box->max.z) OR
          (static_box->max.z <= box->min.z)) 
        result = gxRELATION_OUTSIDE;
    }
    else {
      // Compute the time value when they begin and end overlap
      f = 1 / ptrajectory.direction.z;
      enter = (static_box->min.z - box->max.z) * f;
      leave = (static_box->max.z - box->min.z) * f;
      // Check for interval out of order
      if (enter > leave) {
        f = enter;
        enter = leave;
        leave = f;
      }
      // Update interval
      if (enter > t_enter) 
        t_enter = enter;
      if (leave > t_leave)
        t_leave = leave;
      // Check if this resulted in empty interval
      if (t_enter > t_leave)
        result = gxRELATION_OUTSIDE;
    }
  }

  // Compute collision time
  if (parametric_collision_time)
    *parametric_collision_time = t_enter / (trajectory->velocity * dtime);
  
  return (result);
}

/*____________________________________________________________________
|
| Function: gx3d_Collide_Box_StaticBox
|
| Output: Returns collision info regarding a AAA box moving relative to
|   a static AAB box.  Returns relation and optionally, the parametric 
|   collision time in the range 0-1.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = box collides with box
|
| Reference: 3D Math for Graphics and Games Development (pg. 313)
|___________________________________________________________________*/

gxRelation gx3d_Collide_Box_StaticBox (
  gx3dBox                 *box,
  gx3dProjectedTrajectory *ptrajectory,
  gx3dBox                 *static_box,
  float                   *parametric_collision_time )  // NULL if not needed
{
  gx3dTrajectory trajectory;
  gxRelation result = gxRELATION_INTERSECT; // assume intersecting
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box);
  DEBUG_ASSERT (ptrajectory);
  // Make sure trajectory direction is not 0
  DEBUG_ASSERT (ptrajectory->direction.x + ptrajectory->direction.y + ptrajectory->direction.z);
  DEBUG_ASSERT (static_box);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Extract normal and velocity from projected trajectory (time is assumed to be 1 unit)
  gx3d_NormalizeVector (&ptrajectory->direction, &trajectory.direction, &trajectory.velocity);
  
  return (gx3d_Collide_Box_StaticBox (box, &trajectory, 1, static_box, parametric_collision_time));
}

/*____________________________________________________________________
|
| Function: gx3d_Collide_Box_Box
|
| Output: Returns collision info regarding a moving AAB box relative 
|   to another moving AAB box.  Returns relation and optionally, collision 
|   time in the range 0-1.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = box1 collides with box2
|___________________________________________________________________*/

gxRelation gx3d_Collide_Box_Box (
  gx3dBox        *box1,
  gx3dTrajectory *trajectory1,
  float           dtime,                      // period of time box moves
  gx3dBox        *box2,
  gx3dTrajectory *trajectory2,
  float          *parametric_collision_time ) // NULL if not needed
{
  gx3dVector dv1, dv2;
  gx3dBox *b_temp;
  gx3dTrajectory *t_temp, trajectory;
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box1);
  DEBUG_ASSERT (trajectory1);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&trajectory1->direction, &trajectory1->direction) -1.0) < .01);
  DEBUG_ASSERT (dtime > 0);
  DEBUG_ASSERT (box2);
  DEBUG_ASSERT (trajectory2);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&trajectory2->direction, &trajectory2->direction) -1.0) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Set box1 = fastest moving box
  if (trajectory1->velocity < trajectory2->velocity) {
    b_temp      = box1;
    box1        = box2;
    box2        = b_temp;
    t_temp      = trajectory1;
    trajectory1 = trajectory2;
    trajectory2 = t_temp;
  }

  // Compute projected directions for both moving boxes
  gx3d_MultiplyScalarVector (trajectory1->velocity * dtime, &trajectory1->direction, &dv1);
  gx3d_MultiplyScalarVector (trajectory2->velocity * dtime, &trajectory2->direction, &dv2);

  // Compute the trajectory of movement of box1 relative to box2
  gx3d_SubtractVector (&dv1, &dv2, &trajectory.direction);
  gx3d_NormalizeVector (&trajectory.direction, &trajectory.direction);

  return (gx3d_Collide_Box_StaticBox (box1, &trajectory, dtime, box2, parametric_collision_time));
}
  
/*____________________________________________________________________
|
| Function: gx3d_Collide_Box_Box
|
| Output: Returns collision info regarding a moving AAB box relative 
|   to another moving AAB box.  Returns relation and optionally, collision 
|   time in the range 0-1.
|
|   Returns gxRELATION_OUTSIDE   = no collision occurs
|           gxRELATION_INTERSECT = box1 collides with box2
|___________________________________________________________________*/

gxRelation gx3d_Collide_Box_Box (
  gx3dBox                 *box1,
  gx3dProjectedTrajectory *ptrajectory1,
  gx3dBox                 *box2,
  gx3dProjectedTrajectory *ptrajectory2,
  float                   *parametric_collision_time )  // NULL if not needed
{
  gx3dTrajectory trajectory1, trajectory2;
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (box1);
  DEBUG_ASSERT (ptrajectory1);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ptrajectory1->direction, &ptrajectory1->direction) -1.0) < .01);
  DEBUG_ASSERT (box2);
  DEBUG_ASSERT (ptrajectory2);
  // Make sure normal is normalized
  DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&ptrajectory2->direction, &ptrajectory2->direction) -1.0) < .01);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Extract normal and velocity from projected trajectories (time is assumed to be 1 unit)
  gx3d_NormalizeVector (&ptrajectory1->direction, &trajectory1.direction, &trajectory1.velocity);
  gx3d_NormalizeVector (&ptrajectory2->direction, &trajectory2.direction, &trajectory2.velocity);
  
  return (gx3d_Collide_Box_Box (box1, &trajectory1, 1, box2, &trajectory2, parametric_collision_time));
}
