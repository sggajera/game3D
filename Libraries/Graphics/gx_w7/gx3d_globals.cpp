/*____________________________________________________________________
|
| File: gx3d_globals.cpp
|
| Description: Functions to manage global variables.
|
| Functions:  gx3d_UpdateViewProjectionMatrix
|             gx3d_UpdateViewFrustum
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
| Function: gx3d_UpdateViewProjectionMatrix
|
| Output: Computes the view frustum clip planes based on parameters
|   about the projection.  This function should be called anytime the
|   projection is changed (gx3d_SetProjectionMatrix()).
|___________________________________________________________________*/

void gx3d_UpdateViewProjectionMatrix ()
{
  gx3dMatrix mp;

  DEBUG_ASSERT (gx3d_View_projection_matrix_dirty);

  // Compute the view * projection matrix
  gx3d_GetProjectionMatrix (&mp);
  gx3d_MultiplyMatrix (&gx3d_View_matrix, &mp, &gx3d_View_projection_matrix);

  gx3d_View_projection_matrix_dirty = false;
}

/*____________________________________________________________________
|
| Function: gx3d_UpdateViewFrustum
|
| Output: Computes the view frustum clip planes based on parameters
|   about the projection.  This function should be called anytime the
|   projection is changed (gx3d_SetProjectionMatrix()).
|___________________________________________________________________*/

void gx3d_UpdateViewFrustum ()
{
  float hradians, vradians, far_x, far_y, xtan, ytan;
  gx3dVector p1, p2, p3;

  DEBUG_ASSERT (gx3d_View_frustum_dirty);

  // Init variables
  hradians = (float)((double)gx3d_Projection_hfov * DEGREES_TO_RADIANS * 0.5);
  vradians = (float)((double)gx3d_Projection_vfov * DEGREES_TO_RADIANS * 0.5);
  xtan = tanf (hradians);
  ytan = tanf (vradians);
  far_x = gx3d_Projection_far_plane * xtan;
  far_y = gx3d_Projection_far_plane * ytan;

  // Compute near plane rectangle (in world space, and y points up)
  gx3d_View_frustum.view_plane.xright  = gx3d_Projection_near_plane * xtan;
  gx3d_View_frustum.view_plane.xleft   = -gx3d_View_frustum.view_plane.xright;
  gx3d_View_frustum.view_plane.ytop    = gx3d_Projection_near_plane * ytan;
  gx3d_View_frustum.view_plane.ybottom = -gx3d_View_frustum.view_plane.ytop;

  // Set near plane
  gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_NEAR].n.x = 0;
  gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_NEAR].n.y = 0;
  gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_NEAR].n.z = 1;
  gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_NEAR].d = gx3d_Projection_near_plane;

  // Set far plane
  gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_FAR].n.x = 0;
  gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_FAR].n.y = 0;
  gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_FAR].n.z = -1;
  gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_FAR].d = gx3d_Projection_far_plane;

  // Calculate left plane using 3 points
  p1.x = 0;
  p1.y = 0;
  p1.z = 0;
  p2.x = -far_x;
  p2.y =  far_y;
  p2.z =  gx3d_Projection_far_plane;
  p3.x = -far_x;
  p3.y = -far_y;
  p3.z =  gx3d_Projection_far_plane;
  gx3d_GetPlane (&p1, &p2, &p3, &gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_LEFT]);

  // Calculate right plane using 3 points
  p1.x = 0;
  p1.y = 0;
  p1.z = 0;
  p2.x =  far_x;
  p2.y = -far_y;
  p2.z =  gx3d_Projection_far_plane;
  p3.x =  far_x;
  p3.y =  far_y;
  p3.z =  gx3d_Projection_far_plane;
  gx3d_GetPlane (&p1, &p2, &p3, &gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_RIGHT]);

  // Calculate top plane using 3 points
  p1.x = 0;
  p1.y = 0;
  p1.z = 0;
  p2.x =  far_x;
  p2.y =  far_y;
  p2.z =  gx3d_Projection_far_plane;
  p3.x = -far_x;
  p3.y =  far_y;
  p3.z =  gx3d_Projection_far_plane;
  gx3d_GetPlane (&p1, &p2, &p3, &gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_TOP]);

  // Calculate bottom plane using 3 points
  p1.x = 0;
  p1.y = 0;
  p1.z = 0;
  p2.x = -far_x;
  p2.y = -far_y;
  p2.z =  gx3d_Projection_far_plane;
  p3.x =  far_x;
  p3.y = -far_y;
  p3.z =  gx3d_Projection_far_plane;
  gx3d_GetPlane (&p1, &p2, &p3, &gx3d_View_frustum.plane[gx3d_FRUSTUM_PLANE_BOTTOM]);

  gx3d_View_frustum_dirty = false;
}
