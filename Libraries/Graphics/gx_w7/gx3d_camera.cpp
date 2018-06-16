/*____________________________________________________________________
|
| File: gx3d_camera.cpp
|
| Description: Functions to manipulate a 3D camera.
|
| Functions:  gx3d_CameraSetPosition
|             gx3d_CameraGetCurrentPosition
|             gx3d_CameraGetCurrentOrientation
|             gx3d_CameraGetCurrentDistance
|             gx3d_CameraGetCurrentRotation
|             gx3d_CameraRotate
|             gx3d_CameraScale
|             gx3d_CameraSetViewMatrix
|
| Notes: 
|   Coordinate system
|     A left-handed coordinate system is assumed.  Positive rotations
|     are clockwise when viewed from the positive axis toward the origin.
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|
| DEBUG_ASSERTED!
|___________________________________________________________________*/

#define DEBUG

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <math.h>

#include "dp.h"

/*___________________
|
| Type definitions
|__________________*/

typedef struct {
  gx3dVector from, to, world_up;
} CameraPosition;

/*___________________
|
| Global variables
|__________________*/

// Camera position transformation matrix
static gx3dMatrix c_position_matrix;

// Camera position, orientation
static CameraPosition c_original = {{ 0, 0, -1}, { 0, 0, 0 }, { 0, 1, 0 } };
static CameraPosition c_current  = {{ 0, 0, -1}, { 0, 0, 0 }, { 0, 1, 0 } };
static float          c_x_axis_rotate;
static float          c_y_axis_rotate;
static int            c_orientation = gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED;
static gx3dMatrix     c_m_pre, c_m_post, c_m_yi, c_m_scale, c_mt_to, c_mti_to;
static float          c_scale;

/*____________________________________________________________________
|
| Function: gx3d_CameraSetPosition
|                                                                                        
| Output: Sets new camera from and look at coordinates.  Camera position
|   defaults to:
|     from        = (0,0,-1)
|     to          = (0,0,0)
|     world_up    = (0,1,0)
|     orientation = look from or look to fixed
|___________________________________________________________________*/

void gx3d_CameraSetPosition (gx3dVector *from, gx3dVector *to, gx3dVector *world_up, int orientation)
{
  float angle;
  gx3dVector v, v1, v2;
  gx3dMatrix mt, mti, m;
  gx3dVector negative_z_axis = { 0, 0, -1 };
  gx3dVector positive_z_axis = { 0, 0, 1 };

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (from);
  DEBUG_ASSERT (to);
  DEBUG_ASSERT (world_up);
  DEBUG_ASSERT ((orientation == gx3d_CAMERA_ORIENTATION_LOOKFROM_FIXED) OR (orientation == gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED));

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Set camera location
  memcpy (&c_original.from,     from,     sizeof(gx3dVector));
  memcpy (&c_original.to,       to,       sizeof(gx3dVector));
  memcpy (&c_original.world_up, world_up, sizeof(gx3dVector));
  memcpy (&c_current, &c_original, sizeof(CameraPosition));

  // No additional rotations so far
  c_x_axis_rotate = 0;
  c_y_axis_rotate = 0;

  // Set orientation
  c_orientation = orientation;  

  // Set zoom to none (1 = normal size)
  c_scale = 1;
  gx3d_GetScaleMatrix (&c_m_scale, c_scale, c_scale, c_scale);

  switch (c_orientation) {
    case gx3d_CAMERA_ORIENTATION_LOOKFROM_FIXED:
      gx3d_GetTranslateMatrix        (&mt,  -c_original.from.x, -c_original.from.y, -c_original.from.z);
      gx3d_GetTranslateMatrixInverse (&mti, -c_original.from.x, -c_original.from.y, -c_original.from.z);
      gx3d_MultiplyVectorMatrix (&c_original.to, &mt, &v);

      // Compute angle to rotate view vector onto yz plane
      if (v.x == 0) {
        if (v.z < 0)
          angle = 180;
        else
          angle = 0;
      }
      else {
        v1 = v;
        // flatten out view vector onto xz plane
        v1.y = 0;
        v2 = positive_z_axis;
        // Get angle between view vector (flattened out onto yz plane) and positive z axis
        angle = safe_acosf (gx3d_VectorDotProduct (&v1, &v2) / (gx3d_VectorMagnitude(&v1) * gx3d_VectorMagnitude(&v2)));
        if (angle == 0)
          angle = 90;
        else 
          angle *= RADIANS_TO_DEGREES;
///// I CHANGED TO DIRECTION OF THE ARROW SIGN HERE, not sure if this is correct
        if (v.x > 0)
          angle = (180 - angle) + 180;
      }
      break;
    case gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED:
      gx3d_GetTranslateMatrix        (&mt,  -c_original.to.x, -c_original.to.y, -c_original.to.z);
      gx3d_GetTranslateMatrixInverse (&mti, -c_original.to.x, -c_original.to.y, -c_original.to.z);
      c_mt_to  = mt;
      c_mti_to = mti;
      gx3d_MultiplyVectorMatrix (&c_original.from, &mt, &v);

      // Compute angle to rotate view vector onto yz plane
      if (v.x == 0) {
        if (v.z > 0)
          angle = 180;
        else
          angle = 0;
      }
      else {
        v1 = v;
        // flatten out view vector onto xz plane
        v1.y = 0;
        v2 = negative_z_axis;
        // Get angle between view vector (flattened out onto yz plane) and negative z axis
        angle = safe_acosf (gx3d_VectorDotProduct (&v1, &v2) / (gx3d_VectorMagnitude(&v1) * gx3d_VectorMagnitude(&v2)));
        if (angle == 0)
          angle = 90;
        else 
          angle *= RADIANS_TO_DEGREES;
        if (v.x < 0)
          angle = (180 - angle) + 180;
      }
      break;
  }

  // Compute the 'pre' matrix
  gx3d_GetRotateYMatrix (&m, angle);
  gx3d_MultiplyMatrix (&mt, &m, &c_m_pre);

  // Compute the 'post' matrix
  gx3d_GetRotateYMatrix (&m, -angle);
  gx3d_MultiplyMatrix (&m, &mti, &c_m_post);
  // Save this matrix
  c_m_yi = m;

  // Build scale matrix
  gx3d_GetScaleMatrix (&m, c_scale, c_scale, c_scale);
}

/*____________________________________________________________________
|
| Function: gx3d_CameraGetCurrentPosition
|
| Output: Returns the current position of the camera after current
|   transformations (rotation/zoom) have been applied.  If any 
|   parameters are NULL, will not return anything in those parameters.
|___________________________________________________________________*/

void gx3d_CameraGetCurrentPosition (gx3dVector *from, gx3dVector *to, gx3dVector *world_up)
{
  if (from)
//    memcpy (from, &c_current.from, sizeof(gx3dVector));
	  *from = c_current.from;
  if (to)
//    memcpy (to, &c_current.to, sizeof(gx3dVector));
    *to = c_current.to;
	if (world_up)
//    memcpy (world_up, &c_current.world_up, sizeof(gx3dVector));
    *world_up = c_current.world_up;
}

/*____________________________________________________________________
|
| Function: gx3d_CameraGetCurrentOrientation
|
| Output: Returns the current orientation of the camera.
|___________________________________________________________________*/

int gx3d_CameraGetCurrentOrientation ()
{
  return (c_orientation);
}

/*____________________________________________________________________
|
| Function: gx3d_CameraGetCurrentDistance
|
| Output: Returns the current distance between the from point and the 
|   to point.
|___________________________________________________________________*/

inline float gx3d_CameraGetCurrentDistance (void)
{
  return (gx3d_Distance_Point_Point (&c_current.from, &c_current.to));
}

/*____________________________________________________________________
|
| Function: gx3d_CameraGetCurrentRotation
|
| Output: Returns the current rotate of the camera.
|___________________________________________________________________*/

void gx3d_CameraGetCurrentRotation (float *x_axis_rotate_degrees, float *y_axis_rotate_degrees)
{
  if (x_axis_rotate_degrees)
    *x_axis_rotate_degrees = c_x_axis_rotate;
  if (y_axis_rotate_degrees)
    *y_axis_rotate_degrees = c_y_axis_rotate;
}

/*____________________________________________________________________
|
| Function: gx3d_CameraRotate
|
| Output: Adds rotations to the camera location (in degrees).  Rotates 
|   about the from point or to point, depending on camera orientation.
|___________________________________________________________________*/

void gx3d_CameraRotate (float x_axis_rotate_degrees, float y_axis_rotate_degrees)
{
  gx3dMatrix m, mx, my, mxy;

  // Update current camera rotations
  c_x_axis_rotate += x_axis_rotate_degrees;
  if (c_x_axis_rotate >= 360)
    c_x_axis_rotate -= 360;
  c_y_axis_rotate += y_axis_rotate_degrees;
  if (c_y_axis_rotate >= 360)
    c_y_axis_rotate -= 360;

  switch (c_orientation) {

/*____________________________________________________________________
|
| Rotate the to point
|___________________________________________________________________*/

    case gx3d_CAMERA_ORIENTATION_LOOKFROM_FIXED:
      gx3d_GetRotateXMatrix (&mx, c_x_axis_rotate);
      gx3d_GetRotateYMatrix (&my, c_y_axis_rotate);
      gx3d_MultiplyMatrix (&mx, &my, &mxy);

      gx3d_MultiplyMatrix (&c_m_pre, &mxy, &m);
      gx3d_MultiplyMatrix (&m, &c_m_post, &m);

      // Transform the camera position
      gx3d_MultiplyVectorMatrix (&c_original.to, &m, &c_current.to);

/***** THIS WORKS BUT MAY NOT BE THE FINAL SOLUTION! *****/
      gx3d_MultiplyMatrix (&mxy, &c_m_yi, &m);
      gx3d_MultiplyVectorMatrix (&c_original.world_up, &m, &c_current.world_up);
/*****************************************************/

      break;
/*____________________________________________________________________
|
| Rotate the from point
|___________________________________________________________________*/

    case gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED:
      gx3d_GetRotateXMatrix (&mx, c_x_axis_rotate);
      gx3d_GetRotateYMatrix (&my, c_y_axis_rotate);
      gx3d_MultiplyMatrix (&mx, &my, &mxy);

      gx3d_MultiplyMatrix (&c_m_pre, &c_m_scale, &m);
      gx3d_MultiplyMatrix (&m, &mxy, &m);
      gx3d_MultiplyMatrix (&m, &c_m_post, &m);

      // Transform the camera position
      gx3d_MultiplyVectorMatrix (&c_original.from, &m, &c_current.from);

/***** THIS WORKS BUT MAY NOT BE THE FINAL SOLUTION! *****/
      gx3d_MultiplyMatrix (&mxy, &c_m_yi, &m);
      gx3d_MultiplyVectorMatrix (&c_original.world_up, &m, &c_current.world_up);
/*****************************************************/
      break;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_CameraScale
|
| Output: Allows user to zoom in or out camera. 
|___________________________________________________________________*/

void gx3d_CameraScale (float scale)
{
  c_scale = scale;

  // Build scale matrix
  gx3d_GetScaleMatrix (&c_m_scale, c_scale, c_scale, c_scale);
  // Call the rotate function to build the correct view 
  gx3d_CameraRotate (0, 0);
}

/*____________________________________________________________________
|
| Function: gx3d_CameraSetViewMatrix
|
| Output: Sets the view matrix using the camera matrix.
|___________________________________________________________________*/

inline void gx3d_CameraSetViewMatrix (void)
{
  gx3dMatrix m_view;
  
  // Set the view matrix
  gx3d_ComputeViewMatrix (&m_view, &c_current.from, &c_current.to, &c_current.world_up);
  gx3d_SetViewMatrix (&m_view);
}
