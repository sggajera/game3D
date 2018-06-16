/*____________________________________________________________________
|
| File: gx3d_math.cpp
|
| Description: Functions for 3D math.
|
| Functions:  gx3d_ComputeViewMatrix
|             gx3d_ComputeProjectionMatrix
|             gx3d_ComputeProjectionMatrixHV
|       
|             gx3d_SetWorldMatrix
|             gx3d_GetWorldMatrix
|             gx3d_SetViewMatrix
|             gx3d_SetViewMatrix
|             gx3d_GetViewMatrix
|             gx3d_SetProjectionMatrix
|             gx3d_SetProjectionMatrix
|             gx3d_SetProjectionMatrix
|             gx3d_GetProjectionMatrix
|
|             gx3d_GetViewFrustum
|             gx3d_GetWorldFrustum
|
|             gx3d_GetIdentityMatrix
|             gx3d_GetTransposeMatrix
|             gx3d_OrthogonalizeMatrix
|             gx3d_GetTranslateMatrix
|             gx3d_GetTranslateMatrixInverse
|             gx3d_GetScaleMatrix
|             gx3d_GetScaleMatrixInverse
|             gx3d_GetRotateXMatrix
|             gx3d_GetRotateXMatrixInverse
|             gx3d_GetRotateYMatrix
|             gx3d_GetRotateYMatrixInverse
|             gx3d_GetRotateZMatrix
|             gx3d_GetRotateZMatrixInverse
|             gx3d_GetRotateMatrix
|
|             gx3d_EnableTextureMatrix
|             gx3d_EnableTextureMatrix3D
|             gx3d_DisableTextureMatrix
|             gx3d_SetTextureMatrix
|             gx3d_GetTextureMatrix
|             gx3d_GetTranslateTextureMatrix
|             gx3d_GetTranslateTextureMatrixInverse
|             gx3d_GetScaleTextureMatrix
|             gx3d_GetScaleTextureMatrixInverse
|             gx3d_GetRotateXTextureMatrix
|             gx3d_GetRotateXTextureMatrixInverse
|             gx3d_GetRotateYTextureMatrix
|             gx3d_GetRotateYTextureMatrixInverse
|
|             gx3d_MultiplyMatrix
|             gx3d_MultiplyScalarMatrix
|             gx3d_MultiplyVectorMatrix
|             gx3d_MultiplyNormalVectorMatrix
|             gx3d_MultiplyVector4DMatrix
|             gx3d_MultiplyScalarVector
|
|             gx3d_NormalizeVector
|             gx3d_NormalizeVector
|             gx3d_VectorMagnitude
|             gx3d_VectorDotProduct
|             gx3d_AngleBetweenVectors
|             gx3d_AngleBetweenUnitVectors
|             gx3d_VectorCrossProduct
|             gx3d_AddVector
|             gx3d_SubtractVector
|             gx3d_NegateVector
|             gx3d_ProjectVectorOntoVector
|             gx3d_ProjectVectorOntoVector
|             gx3d_ProjectVectorOntoUnitVector
|             gx3d_ProjectVectorOntoUnitVector
|             gx3d_SurfaceNormal
|             gx3d_Lerp
|             gx3d_LerpVector
|             gx3d_Clamp
|              
|             gx3d_GetPlane
|             gx3d_GetPlane
|
|             gx3d_GetBillboardRotateXYMatrix
|             gx3d_GetBillboardRoatteXMatrix
|             gx3d_GetBillboardRotateYMatrix
|
|             gx3d_HeadingToXZVector
|             gx3d_HeadingToYZVector
|             gx3d_XZVectorToHeading
|             gx3d_YZVectorToHeading
|  
| Notes: 
|   Coordinate system
|     A left-handed coordinate system is assumed.  Positive rotations
|     are therefore counterclockwise when viewed from the positive axis
|     toward the origin.  Objects vertices should be specified in a 
|     clockwise order.
|
|   Vector/Matrix Math
|     This code adopts the convention used in "3D Computer Graphics",
|     Second Edition by Alan Watt for vector/matrix math.  Row vectors 
|     are used to represent 3D points.  The reason for this is that 
|     when transformation matrices are concatenated the left to right 
|     order in which they appear in the concatenation product is the 
|     order in which the corresponding transformations are applied.  
|
|   Tip - 'Transforming normals'
|     When multiplying a 4x4 matrix by a 4D vector, if the w part of 
|     the vector is 0, this 'disables' the translation portion of the
|     4x4 matrix.  This is useful since some vectors such as surface
|     normals don't need to be translated.
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
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
| Macros
|__________________*/

#define IDENTITY_MATRIX(_m_)            \
  {                                     \
    memset (_m_, 0, 16*sizeof(float));  \
    _m_->_00 = 1;                       \
    _m_->_11 = 1;                       \
    _m_->_22 = 1;                       \
    _m_->_33 = 1;                       \
  }      

/*____________________________________________________________________
|
| Function: gx3d_ComputeViewMatrix
|
| Output: Computes the view matrix from user parameters.  Given an eye
|   position (from), a look at point (to), and a normalized world up vector, 
|   typically (0, 1, 0), computes the 4x4 view matrix.
|___________________________________________________________________*/

void gx3d_ComputeViewMatrix (
  gx3dMatrix *m,          // the resulting matrix
  gx3dVector *from,       // the eye point
  gx3dVector *to,         // the look at point
  gx3dVector *world_up )  // the world up vector, typically (0,1,0)
{                                                    
  float dot_product;
  gx3dVector v_view, v_up, v_right;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (m);
  DEBUG_ASSERT (from);
  DEBUG_ASSERT (to);
  DEBUG_ASSERT (world_up);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Get the z basis vector, which points straight ahead - 
  //  the difference from the eyepoint to the look at point
  gx3d_SubtractVector (to, from, &v_view);

  // Normalize the z basis vector
  gx3d_NormalizeVector (&v_view, &v_view);

  // Get the dot product, and calculate the projection of the z basis
  //  vector onto the up vector. The projection is the y basis vector.
  dot_product = gx3d_VectorDotProduct (world_up, &v_view);

  v_up.x = world_up->x - (dot_product * v_view.x);
  v_up.y = world_up->y - (dot_product * v_view.y);
  v_up.z = world_up->z - (dot_product * v_view.z);

  // Normalize the y basis vector
  gx3d_NormalizeVector (&v_up, &v_up);

  // The x basis vector is found simply with the cross product of the y and z basis vectors
  gx3d_VectorCrossProduct (&v_up, &v_view, &v_right);

  // Start building the matrix. The first three rows contains the basis
  //  vectors used to rotate the view to point at the lookat point
  IDENTITY_MATRIX (m)
  m->_00 = v_right.x;    m->_01 = v_up.x;    m->_02 = v_view.x;
  m->_10 = v_right.y;    m->_11 = v_up.y;    m->_12 = v_view.y;
  m->_20 = v_right.z;    m->_21 = v_up.z;    m->_22 = v_view.z;

  // Do the translation values (rotations are still about the eye point)
  m->_30 = -(gx3d_VectorDotProduct (from, &v_right));
  m->_31 = -(gx3d_VectorDotProduct (from, &v_up));
  m->_32 = -(gx3d_VectorDotProduct (from, &v_view));
}

/*____________________________________________________________________
|
| Function: gx3d_ComputeProjectionMatrix
|
| Output: Computes the perspective projection matrix from user parameters.  
|   Note the matrix is normalized for element [2][3] to be 1.0.  This 
|   is performed so that W-based range fog will work correctly.
|___________________________________________________________________*/

void gx3d_ComputeProjectionMatrix (
  gx3dMatrix *m,            // the resulting matrix
  float       fov,          // horizontal field of view in degrees (0.1 - 179.9)
  float       near_plane,   // in world z units
  float       far_plane )   // in world z units
{
  int dx, dy;
  float radians, q, aspect_ratio;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (m);
  DEBUG_ASSERT (fov > 0);
  DEBUG_ASSERT (fov < 180);
  DEBUG_ASSERT (near_plane > 0);
  DEBUG_ASSERT (far_plane > near_plane);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  dx = gx3d_Viewport.xright  - gx3d_Viewport.xleft + 1;
  dy = gx3d_Viewport.ybottom - gx3d_Viewport.ytop  + 1;
  aspect_ratio = (float)dx / (float)dy;
  q = far_plane / (far_plane - near_plane);

  memset (m, 0, 16*sizeof(float));  
  // Is field of view 90 degrees?
  if (fabsf(fov-90.0f) <= 0.001) {
    radians = (float)((double)fov * DEGREES_TO_RADIANS);
    m->_00 = 1;
    m->_11 = aspect_ratio;
  }
  else {
    radians = (float)((double)fov * DEGREES_TO_RADIANS);
    m->_00 = 1 / tanf(radians * 0.5);
    m->_11 = 1 / tanf(((double)((double)fov/aspect_ratio) * DEGREES_TO_RADIANS) * 0.5);
  }
  m->_22 = q;
  m->_23 = 1;
  m->_32 = -q * near_plane;
}

/*____________________________________________________________________
|
| Function: gx3d_ComputeProjectionMatrixHV
|
| Output: Computes the perspective projection matrix from user parameters.  
|   Note the matrix is normalized for element [2][3] to be 1.0.  This 
|   is performed so that W-based range fog will work correctly.
|___________________________________________________________________*/

void gx3d_ComputeProjectionMatrixHV (
  gx3dMatrix *m,            // the resulting matrix
  float       hfov,         // horizontal field of view in degrees (0.1 - 179.9)
  float       vfov,         // vertical field of view in degrees (0.1 - 179.9)
  float       near_plane,   // in world z units
  float       far_plane )   // in world z units
{
  float hradians, vradians, q;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (m);
  DEBUG_ASSERT (hfov > 0);
  DEBUG_ASSERT (hfov < 180);
  DEBUG_ASSERT (vfov > 0);
  DEBUG_ASSERT (vfov < 180);
  DEBUG_ASSERT (near_plane > 0);
  DEBUG_ASSERT (far_plane > near_plane);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  hradians = (float)((double)hfov * DEGREES_TO_RADIANS);
  vradians = (float)((double)vfov * DEGREES_TO_RADIANS);
  q = far_plane / (far_plane - near_plane);

  memset (m, 0, 16*sizeof(float));  
  m->_00 = 1 / tanf(hradians * 0.5);
  m->_11 = 1 / tanf(vradians * 0.5);
  m->_22 = q;
  m->_23 = 1;
  m->_32 = -q * near_plane;
}

/*____________________________________________________________________
|
| Function: gx3d_SetWorldMatrix
|
| Output: Sets the world transformation matrix.
|___________________________________________________________________*/

void gx3d_SetWorldMatrix (gx3dMatrix *m)
{
  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (gx_Video.set_world_matrix);

  // Set world matrix
  (*gx_Video.set_world_matrix) ((void *)m);
}

/*____________________________________________________________________
|
| Function: gx3d_GetWorldMatrix
|
| Output: Gets the world transformation matrix.
|___________________________________________________________________*/

void gx3d_GetWorldMatrix (gx3dMatrix *m)
{                                
  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (gx_Video.get_world_matrix);

  // Get world matrix
  (*gx_Video.get_world_matrix) ((void *)m);
}

/*____________________________________________________________________
|
| Function: gx3d_SetViewMatrix
|
| Output: Sets the view transformation matrix. 
|___________________________________________________________________*/

void gx3d_SetViewMatrix (gx3dMatrix *m)
{
  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (gx_Video.set_view_matrix);

  // Set view matrix
  (*gx_Video.set_view_matrix) ((void *)m);

  // Set globals
  gx3d_View_matrix = *m;
  gx3d_View_projection_matrix_dirty = true;
}

/*____________________________________________________________________
|
| Function: gx3d_SetViewMatrix
|
| Output: Sets the view transformation matrix. 
|___________________________________________________________________*/

void gx3d_SetViewMatrix (
  gx3dVector *from,       // the eye point
  gx3dVector *to,         // the look at point
  gx3dVector *world_up )  // the world up vector, typically (0,1,0)
{
  gx3dMatrix m;

  // Verify input params
  DEBUG_ASSERT (from);
  DEBUG_ASSERT (to);
  DEBUG_ASSERT (world_up);

  // Compute and set world matrix
  gx3d_ComputeViewMatrix (&m, from, to, world_up);
  gx3d_SetViewMatrix (&m);
}

/*____________________________________________________________________
|
| Function: gx3d_GetViewMatrix
|
| Output: Gets the view transformation matrix.  Returns true on success.
|___________________________________________________________________*/

void gx3d_GetViewMatrix (gx3dMatrix *m)
{
  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (gx_Video.get_view_matrix);

  // Get view matrix
  (*gx_Video.get_view_matrix) ((void *)m);
}

/*____________________________________________________________________
|
| Function: gx3d_SetProjectionMatrix
|
| Output: Sets the projection transformation matrix.  
|___________________________________________________________________*/

void gx3d_SetProjectionMatrix (gx3dMatrix *m)
{
  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (gx_Video.set_projection_matrix);

  // Set projection matrix
  (*gx_Video.set_projection_matrix) ((void *)m);

  // Set globals
  gx3d_View_projection_matrix_dirty = true;
  gx3d_View_frustum_dirty           = true;
}

/*____________________________________________________________________
|
| Function: gx3d_SetProjectionMatrix
|
| Output: Computes and sets the projection transformation matrix.  Also
|   sets the frustum clip planes.  
|
|   This is a preferred function to call for setting the projection
|   matrix since it also computes the view frustum clip planes.
|___________________________________________________________________*/

void gx3d_SetProjectionMatrix (
  float fov,        // field of view in degrees
  float near_plane, // in world z units
  float far_plane ) // in world z units
{
  int dx, dy;
  float aspect_ratio;
  gx3dMatrix m;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (fov > 0);
  DEBUG_ASSERT (fov < 180);
  DEBUG_ASSERT (near_plane > 0);
  DEBUG_ASSERT (far_plane > near_plane);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Set the projection matrix
  gx3d_ComputeProjectionMatrix (&m, fov, near_plane, far_plane);
  gx3d_SetProjectionMatrix (&m);

  // Compute viewport aspect ratio
  dx = gx3d_Viewport.xright  - gx3d_Viewport.xleft + 1;
  dy = gx3d_Viewport.ybottom - gx3d_Viewport.ytop  + 1;
  aspect_ratio = (float)dx / (float)dy;

  // Set globals
  gx3d_Projection_hfov       = fov;
  gx3d_Projection_vfov       = fov / aspect_ratio;
  gx3d_Projection_near_plane = near_plane;
  gx3d_Projection_far_plane  = far_plane;
}

/*____________________________________________________________________
|
| Function: gx3d_SetProjectionMatrix
|
| Output: Computes and sets the projection transformation matrix.  Also
|   sets the frustum clip planes.  
|
|   This is a preferred function to call for setting the projection
|   matrix since it also computes the view frustum clip planes.
|___________________________________________________________________*/

void gx3d_SetProjectionMatrix (
  float hfov,       // horizontal field of view in degrees (0.1 - 179.9)
  float vfov,       // vertical field of view in degrees (0.1 - 179.9)
  float near_plane, // in world z units
  float far_plane ) // in world z units
{
  gx3dMatrix m;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (hfov > 0);
  DEBUG_ASSERT (hfov < 180);
  DEBUG_ASSERT (vfov > 0);
  DEBUG_ASSERT (vfov < 180);
  DEBUG_ASSERT (near_plane > 0);
  DEBUG_ASSERT (far_plane > near_plane);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_ComputeProjectionMatrixHV (&m, hfov, vfov, near_plane, far_plane);
  gx3d_SetProjectionMatrix (&m);

  // Set globals
  gx3d_Projection_hfov       = hfov;
  gx3d_Projection_vfov       = vfov;
  gx3d_Projection_near_plane = near_plane;
  gx3d_Projection_far_plane  = far_plane;
}

/*____________________________________________________________________
|
| Function: gx3d_GetProjectionMatrix
|
| Output: Gets the projection transformation matrix. 
|___________________________________________________________________*/

void gx3d_GetProjectionMatrix (gx3dMatrix *m)
{
  // Verify input pararms
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (gx_Video.get_projection_matrix);

  // Get projection matrix
  (*gx_Video.get_projection_matrix) ((void *)m);
}

/*____________________________________________________________________
|
| Function: gx3d_GetViewFrustum
|
| Output: Returns a copy of the current view frustum, which is computed
|   by gx using parameters from the projection.  The copy returned will
|   only be valid as long as the projection is not changed.  The 
|   projection is changed by calling gx3d_SetProjectionMatrix().
|___________________________________________________________________*/

void gx3d_GetViewFrustum (gx3dViewFrustum *vf)
{
  // Verify input pararms
  DEBUG_ASSERT (vf);

  // Get view frustum
  if (gx3d_View_frustum_dirty) 
     gx3d_UpdateViewFrustum ();

  *vf = gx3d_View_frustum;
}

/*____________________________________________________________________
|
| Function: gx3d_GetWorldFrustum
|
| Output: Computes a world frustum from a view frustum.  A world 
|   frustum is the corresponding view frustum transformed into 
|   world space.  It also contains values that define the main diagonal
|   on an AAB box in world space, for each plane in the frustum.
|
| References: 
|   Mathematics for 3D Game Programming & Computer Graphics,
|   by Eric Lengyel, pp. 88-89.
|   Real-Time Rendering, 2nd Edition, pp. 35.
|___________________________________________________________________*/

void gx3d_GetWorldFrustum (gx3dViewFrustum *vf, gx3dWorldFrustum *wf)
{
  int i;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (vf);
  DEBUG_ASSERT (wf);
  // Make sure view frustum plane normals are normalized
  for (i=0; i<gx3d_NUM_FRUSTUM_PLANES; i++)
    DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&(vf->plane[i].n), &(vf->plane[i].n)) - 1) < .01);

/*____________________________________________________________________
|
| Compute world frustum (the code below is an optimization, could also
|   just call function to multiply plane (as a 4D vector) by the view
|   matrix)
|___________________________________________________________________*/

  for (i=0; i<gx3d_NUM_FRUSTUM_PLANES; i++) {
    switch (i) {
      case gx3d_FRUSTUM_PLANE_NEAR:
        // Transform near view frustum plane into world space (view frustum near plane normal is 0,0,1)
        wf->plane[i].n.x = gx3d_View_matrix._20;
        wf->plane[i].n.y = gx3d_View_matrix._21;
        wf->plane[i].n.z = gx3d_View_matrix._22;
        wf->plane[i].d   = gx3d_View_matrix._23;
        break;
      case gx3d_FRUSTUM_PLANE_FAR:
        // Transform near view frustum plane into world space (view frustum far plane normal is 0,0,-1)
        wf->plane[i].n.x = -gx3d_View_matrix._20;
        wf->plane[i].n.y = -gx3d_View_matrix._21;
        wf->plane[i].n.z = -gx3d_View_matrix._22;
        wf->plane[i].d   = -gx3d_View_matrix._23;
        break;
      case gx3d_FRUSTUM_PLANE_LEFT:
      case gx3d_FRUSTUM_PLANE_RIGHT:
      case gx3d_FRUSTUM_PLANE_TOP:
      case gx3d_FRUSTUM_PLANE_BOTTOM:
        // Transform view frustum plane into world space
        wf->plane[i].n.x = vf->plane[i].n.x * gx3d_View_matrix._00 +
                           vf->plane[i].n.y * gx3d_View_matrix._10 +
                           vf->plane[i].n.z * gx3d_View_matrix._20; 
//                         + gx3d_View_matrix._30; // don't need this since it will always be 0
        wf->plane[i].n.y = vf->plane[i].n.x * gx3d_View_matrix._01 +
                           vf->plane[i].n.y * gx3d_View_matrix._11 +
                           vf->plane[i].n.z * gx3d_View_matrix._21;
//                         + gx3d_View_matrix._31;
        wf->plane[i].n.z = vf->plane[i].n.x * gx3d_View_matrix._02 +
                           vf->plane[i].n.y * gx3d_View_matrix._12 +
                           vf->plane[i].n.z * gx3d_View_matrix._22; 
//                         + gx3d_View_matrix._32;
        wf->plane[i].d   = vf->plane[i].n.x * gx3d_View_matrix._03 +
                           vf->plane[i].n.y * gx3d_View_matrix._13 +
                           vf->plane[i].n.z * gx3d_View_matrix._23;
        break;
    }
  }

/*____________________________________________________________________
|
| Compute for each plane, the main diagonal of a AABB that is most
| algined with the plane.  This is a preprocessing step for 
| gx3d_Relation_Box_Frustum()
|___________________________________________________________________*/

  for (i=0; i<gx3d_NUM_FRUSTUM_PLANES; i++) {
    if (wf->plane[i].n.x >= 0) {
      wf->box_diagonal[i].minx = 0;
      wf->box_diagonal[i].maxx = 3;
    }
    else {
      wf->box_diagonal[i].minx = 3;
      wf->box_diagonal[i].maxx = 0;
    }
    if (wf->plane[i].n.y >= 0) {
      wf->box_diagonal[i].miny = 1;
      wf->box_diagonal[i].maxy = 4;
    }
    else {
      wf->box_diagonal[i].miny = 4;
      wf->box_diagonal[i].maxy = 1;
    }
    if (wf->plane[i].n.z >= 0) {
      wf->box_diagonal[i].minz = 2;
      wf->box_diagonal[i].maxz = 5;
    }
    else {
      wf->box_diagonal[i].minz = 5;
      wf->box_diagonal[i].maxz = 2;
    }
  }

/*____________________________________________________________________
|
| Verify output
|___________________________________________________________________*/

  // Make sure world frustum plane normals are normalized
  for (i=0; i<gx3d_NUM_FRUSTUM_PLANES; i++)
    DEBUG_ASSERT (fabsf(gx3d_VectorDotProduct (&(wf->plane[i].n), &(wf->plane[i].n)) - 1) < .01);
}

/*____________________________________________________________________
|
| Function: gx3d_GetIdentityMatrix
|
| Output: Sets m to the identity matrix.
|___________________________________________________________________*/

inline void gx3d_GetIdentityMatrix (gx3dMatrix *m)
{
  IDENTITY_MATRIX (m)
}

/*____________________________________________________________________
|
| Function: gx3d_GetTransposeMatrix
|
| Output: Sets mresult to the transpose of m.  This flips the matrix m
|   along its main diagonal - rows become columns and columns become 
|   rows.  If a matrix consists only of rotations, the transpose of that
|   matrix is the inverse.
|___________________________________________________________________*/

void gx3d_GetTransposeMatrix (gx3dMatrix *m, gx3dMatrix *mresult)
{
  int i, j;
  gx3dMatrix mtemp;

  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (mresult);

  // Main routine
  mtemp = *m;
  for (i=0; i<4; i++)
    for (j=0; j<4; j++)
      ((float *)mresult)[j*4+i] = ((float *)&mtemp)[i*4+j];
}

/*____________________________________________________________________
|
| Function: gx3d_OrthogonalizeMatrix
|
| Output: Uses Gram-Schmidt algorithm to make m an orthogonal matrix.
|
| Note: This is typically used to orthogonalize a slightly un-orthogonal
|   matrix.  This function is biased toward the first basis vector in 
|   the matrix.  This function only operates on the top-left 3x3 of the 
|   matrix.  The other components are left unchanged.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 134.
|___________________________________________________________________*/

void gx3d_OrthogonalizeMatrix (gx3dMatrix *m)
{
  float s, dot_r1r1;
  gx3dVector r1, r2, r3, vtemp1, vtemp2;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (m);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Get original row 1
  r1.x = m->_00;
  r1.y = m->_01;
  r1.z = m->_02;
  // Get original row 2
  r2.x = m->_10;
  r2.y = m->_11;
  r2.z = m->_12;
  // Get original row 3
  r3.x = m->_20;
  r3.y = m->_21;
  r3.z = m->_22;

  // Compute values
  dot_r1r1 = gx3d_VectorDotProduct (&r1, &r1);
  
  // Compute new row 2
  s = gx3d_VectorDotProduct (&r1, &r2) / dot_r1r1;
  gx3d_MultiplyScalarVector (s, &r1, &vtemp1);
  gx3d_SubtractVector (&r2, &vtemp1, &r2);

  // Compute new row 3
  s = gx3d_VectorDotProduct (&r3, &r2) / gx3d_VectorDotProduct (&r2, &r2);
  gx3d_MultiplyScalarVector (s, &r2, &vtemp2);
  s = gx3d_VectorDotProduct (&r3, &r1) / dot_r1r1;
  gx3d_MultiplyScalarVector (s, &r1, &vtemp1);
  gx3d_SubtractVector (&r3, &vtemp1, &r3);
  gx3d_SubtractVector (&r3, &vtemp2, &r3);

  // Set new row 2 (row 1 is not changed)
  m->_10 = r2.x;
  m->_11 = r2.y;
  m->_12 = r2.z;
  // Set new row 3
  m->_20 = r3.x;
  m->_21 = r3.y;
  m->_22 = r3.z;
}

/*____________________________________________________________________
|
| Function: gx3d_GetTranslateMatrix
|
| Output: Sets m to the translation matrix.
|___________________________________________________________________*/

void gx3d_GetTranslateMatrix (gx3dMatrix *m, float tx, float ty, float tz)
{
  // Verify input params
  DEBUG_ASSERT (m);

  IDENTITY_MATRIX (m)
  m->_30 = tx;
  m->_31 = ty;
  m->_32 = tz;
}

/*____________________________________________________________________
|
| Function: gx3d_GetTranslateMatrixInverse
|
| Output: Sets m to the inverse translation matrix.
|___________________________________________________________________*/

void gx3d_GetTranslateMatrixInverse (gx3dMatrix *m, float tx, float ty, float tz)
{
  // Verify input params
  DEBUG_ASSERT (m);

  IDENTITY_MATRIX (m)
  m->_30 = -tx;
  m->_31 = -ty;
  m->_32 = -tz;
}

/*____________________________________________________________________
|
| Function: gx3d_GetScaleMatrix
|
| Output: Sets m to the scaling matrix.
|___________________________________________________________________*/

void gx3d_GetScaleMatrix (gx3dMatrix *m, float sx, float sy, float sz)
{
  // Verify input params
  DEBUG_ASSERT (m);

  IDENTITY_MATRIX (m)
  m->_00 = sx;
  m->_11 = sy;
  m->_22 = sz;
}

/*____________________________________________________________________
|
| Function: gx3d_GetScaleMatrixInverse
|
| Output: Sets m to the inverse scaling matrix.
|___________________________________________________________________*/

void gx3d_GetScaleMatrixInverse (gx3dMatrix *m, float sx, float sy, float sz)
{
  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (sx != 0);
  DEBUG_ASSERT (sy != 0);
  DEBUG_ASSERT (sz != 0);

  // Create scale matrix
  IDENTITY_MATRIX (m)
  if ((sx != 0) AND (sy != 0) AND (sz != 0)) {
    m->_00 = 1/sx;
    m->_11 = 1/sy;
    m->_22 = 1/sz;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_GetRotateXMatrix
|
| Output: Sets m to the x-axis rotation matrix.
|___________________________________________________________________*/

void gx3d_GetRotateXMatrix (gx3dMatrix *m, float degrees)
{
  float s, c;

  // Verify input params
  DEBUG_ASSERT (m);

  // Create rotate matrix
  s = sinf (degrees * DEGREES_TO_RADIANS);
  c = cosf (degrees * DEGREES_TO_RADIANS);
  IDENTITY_MATRIX (m)
  m->_11 = c;
  m->_12 = s;
  m->_21 = -s;
  m->_22 = c;
}

/*____________________________________________________________________
|
| Function: gx3d_GetRotateXMatrixInverse
|
| Output: Sets m to the inverse x-axis rotation matrix.
|___________________________________________________________________*/

void gx3d_GetRotateXMatrixInverse (gx3dMatrix *m, float degrees)
{
  float s, c;

  // Verify input params
  DEBUG_ASSERT (m);

  // Create rotate matrix
  s = sinf (-degrees * DEGREES_TO_RADIANS);
  c = cosf (-degrees * DEGREES_TO_RADIANS);
  IDENTITY_MATRIX (m)
  m->_11 = c;
  m->_12 = s;
  m->_21 = -s;
  m->_22 = c;
}

/*____________________________________________________________________
|
| Function: gx3d_GetRotateYMatrix
|
| Output: Sets m to the y-axis rotation matrix.
|___________________________________________________________________*/

void gx3d_GetRotateYMatrix (gx3dMatrix *m, float degrees)
{
  float s, c;

  // Verify input params
  DEBUG_ASSERT (m);

  // Create rotate matrix
  s = sinf (degrees * DEGREES_TO_RADIANS);
  c = cosf (degrees * DEGREES_TO_RADIANS);
  IDENTITY_MATRIX (m)
  m->_00 = c;
  m->_02 = -s;
  m->_20 = s;
  m->_22 = c;
}

/*____________________________________________________________________
|
| Function: gx3d_GetRotateYMatrixInverse
|
| Output: Sets m to the y-axis rotation matrix.
|___________________________________________________________________*/

void gx3d_GetRotateYMatrixInverse (gx3dMatrix *m, float degrees)
{
  float s, c;

  // Verify input params
  DEBUG_ASSERT (m);

  // Create rotate matrix
  s = sinf (-degrees * DEGREES_TO_RADIANS);
  c = cosf (-degrees * DEGREES_TO_RADIANS);
  IDENTITY_MATRIX (m)
  m->_00 = c;
  m->_02 = -s;
  m->_20 = s;
  m->_22 = c;
}

/*____________________________________________________________________
|
| Function: gx3d_GetRotateZMatrix
|
| Output: Sets m to the z-axis rotation matrix.
|___________________________________________________________________*/

void gx3d_GetRotateZMatrix (gx3dMatrix *m, float degrees)
{
  float s, c;

  // Verify input params
  DEBUG_ASSERT (m);

  // Create rotate matrix
  s = sinf (degrees * DEGREES_TO_RADIANS);
  c = cosf (degrees * DEGREES_TO_RADIANS);
  IDENTITY_MATRIX (m)
  m->_00 = c;
  m->_01 = s;
  m->_10 = -s;
  m->_11 = c;
}

/*____________________________________________________________________
|
| Function: gx3d_GetRotateZMatrixInverse
|
| Output: Sets m to the z-axis rotation matrix.
|___________________________________________________________________*/

void gx3d_GetRotateZMatrixInverse (gx3dMatrix *m, float degrees)
{
  float s, c;

  // Verify input params
  DEBUG_ASSERT (m);

  // Create rotate matrix
  s = sinf (-degrees * DEGREES_TO_RADIANS);
  c = cosf (-degrees * DEGREES_TO_RADIANS);
  IDENTITY_MATRIX (m)
  m->_00 = c;
  m->_01 = s;
  m->_10 = -s;
  m->_11 = c;
}

/*____________________________________________________________________
|
| Function: gx3d_GetRotateMatrix
|
| Output: Sets m to the ?-axis rotation matrix.
|___________________________________________________________________*/

void gx3d_GetRotateMatrix (gx3dMatrix *m, gx3dVector *axis, float degrees)
{
  float s, c;
  gx3dVector v;

  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (axis);

  // Create rotate matrix
  s = sinf (degrees * DEGREES_TO_RADIANS);
  c = cosf (degrees * DEGREES_TO_RADIANS);

  v = *axis;
  gx3d_NormalizeVector (axis, &v);
    
  IDENTITY_MATRIX (m)
  m->_00 = (v.x * v.x) * (1 - c) + c;
  m->_01 = (v.y * v.x) * (1 - c) + (v.z * s);
  m->_02 = (v.z * v.x) * (1 - c) - (v.y * s);

  m->_10 = (v.x * v.y) * (1 - c) - (v.z * s);
  m->_11 = (v.y * v.y) * (1 - c) + c ;
  m->_12 = (v.z * v.y) * (1 - c) + (v.x * s);

  m->_20 = (v.x * v.z) * (1 - c) + (v.y * s);
  m->_21 = (v.y * v.z) * (1 - c) - (v.x * s);
  m->_22 = (v.z * v.z) * (1 - c) + c;
}

/*____________________________________________________________________
|
| Function: gx3d_EnableTextureMatrix
|
| Output: Enables the texture matrix to allow caller to transform
|     2D texture coordinates.
|___________________________________________________________________*/

void gx3d_EnableTextureMatrix (int stage)
{
  // Make sure function exists
  DEBUG_ASSERT (gx_Video.enable_texture_matrix);

  // Enable the texture matrix
  (*gx_Video.enable_texture_matrix) (stage, 2, TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableTextureMatrix3D
|
| Output: Enables the texture matrix to allow caller to transform
|     3D texture coordinates.
|___________________________________________________________________*/

void gx3d_EnableTextureMatrix3D (int stage)
{
  // Make sure function exists
  DEBUG_ASSERT (gx_Video.enable_texture_matrix);

  // Enable the texture matrix
  (*gx_Video.enable_texture_matrix) (stage, 3, TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableTextureMatrix
|
| Output: Disables the texture matrix.
|___________________________________________________________________*/

void gx3d_DisableTextureMatrix (int stage)
{
  // Make sure function exists
  DEBUG_ASSERT (gx_Video.enable_texture_matrix);

  // Disable the texture matrix
  (*gx_Video.enable_texture_matrix) (stage, 0, FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_SetTextureMatrix
|
| Output: Sets the texture transformation matrix for this texture stage.
|___________________________________________________________________*/

void gx3d_SetTextureMatrix (int stage, gx3dMatrix *m)
{
  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (gx_Video.set_texture_matrix);

  // Set the texture matrix
  (*gx_Video.set_texture_matrix) (stage, (void *)m);
}

/*____________________________________________________________________
|
| Function: gx3d_GetTextureMatrix
|
| Output: Gets the texture transformation matrix for this texture stage 
|   (0-gx3d_NUM_TEXTURE_STAGES-1). 
|___________________________________________________________________*/

void gx3d_GetTextureMatrix (int stage, gx3dMatrix *m)
{
  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (gx_Video.get_texture_matrix);

  // Get the texture matrix
  (*gx_Video.get_texture_matrix) (stage, (void *)m);
}

/*____________________________________________________________________
|
| Function: gx3d_GetTranslateTextureMatrix
|
| Output: Sets m to the translation texture matrix.
|___________________________________________________________________*/

void gx3d_GetTranslateTextureMatrix (gx3dMatrix *m, float tx, float ty)
{
  // Verify input params
  DEBUG_ASSERT (m);

  IDENTITY_MATRIX (m)
  m->_20 = tx;
  m->_21 = ty;
}

/*____________________________________________________________________
|
| Function: gx3d_GetTranslateTextureMatrixInverse
|
| Output: Sets m to the inverse translation texture matrix.
|___________________________________________________________________*/

void gx3d_GetTranslateTextureMatrixInverse (gx3dMatrix *m, float tx, float ty)
{
  // Verify input params
  DEBUG_ASSERT (m);

  IDENTITY_MATRIX (m)
  m->_20 = -tx;
  m->_21 = -ty;
}

/*____________________________________________________________________
|
| Function: gx3d_GetScaleTextureMatrix
|
| Output: Sets m to the scaling texture matrix.
|___________________________________________________________________*/

void gx3d_GetScaleTextureMatrix (gx3dMatrix *m, float sx, float sy)
{
  // Verify input params
  DEBUG_ASSERT (m);

  IDENTITY_MATRIX (m)
  m->_00 = sx;
  m->_11 = sy;
}

/*____________________________________________________________________
|
| Function: gx3d_GetScaleTextureMatrixInverse
|
| Output: Sets m to the inverse scaling texture matrix.
|___________________________________________________________________*/

void gx3d_GetScaleTextureMatrixInverse (gx3dMatrix *m, float sx, float sy)
{
  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (sx != 0);
  DEBUG_ASSERT (sy != 0);

  IDENTITY_MATRIX (m)
  if ((sx != 0) AND (sy != 0)) {
    m->_00 = 1/sx;
    m->_11 = 1/sy;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_GetRotateTextureMatrix
|
| Output: Sets m to the rotation texture matrix.
|___________________________________________________________________*/

inline void gx3d_GetRotateTextureMatrix (gx3dMatrix *m, float degrees)
{
  gx3d_GetRotateZMatrix (m, degrees);
}

/*____________________________________________________________________
|
| Function: gx3d_GetRotateTextureMatrixInverse
|
| Output: Sets m to the inverse rotation texture matrix.
|___________________________________________________________________*/

inline void gx3d_GetRotateTextureMatrixInverse (gx3dMatrix *m, float degrees)
{
  gx3d_GetRotateZMatrixInverse (m, degrees);
}

/*____________________________________________________________________
|
| Function: gx3d_MultiplyMatrix
|
| Output: Multiplies m1 * m2, putting result in mresult.
|___________________________________________________________________*/

void gx3d_MultiplyMatrix (gx3dMatrix *m1, gx3dMatrix *m2, gx3dMatrix *mresult)
{
  int i, j, k;
  gx3dMatrix mtemp;
  
  // Verify input params
  DEBUG_ASSERT (m1);
  DEBUG_ASSERT (m2);
  DEBUG_ASSERT (mresult);

  // Multiply
  memset ((void *)&mtemp, 0, 16*sizeof(float));
  for (i=0; i<4; i++)
    for (j=0; j<4; j++)
      for (k=0; k<4; k++)
        ((float *)&mtemp)[i*4+j] += (((float *)m1)[i*4+k] * ((float *)m2)[k*4+j]);

  // Put result into mresult
  *mresult = mtemp;
}

/*____________________________________________________________________
|
| Function: gx3d_MultiplyScalarMatrix
|
| Output: Multiplies s * m, putting result in mresult.
|___________________________________________________________________*/

void gx3d_MultiplyScalarMatrix (float s, gx3dMatrix *m, gx3dMatrix *mresult)
{
  int i, j;

  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (mresult);

  // Multiply
  for (i=0; i<4; i++)
    for (j=0; j<4; j++)
      ((float *)mresult)[i*4+j] = ((float *)m)[i*4+j] * s;
}

/*____________________________________________________________________
|
| Function: gx3d_MultiplyVectorMatrix
|
| Output: Multiplies v * m, putting result in vresult.  v and vresult 
|   are 1x3 vectors.
|___________________________________________________________________*/

void gx3d_MultiplyVectorMatrix (gx3dVector *v, gx3dMatrix *m, gx3dVector *vresult)
{
  gx3dVector vorig;

  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (vresult);

  // Make a copy of v in case v and vresult are the same  
  vorig.x = v->x;
  vorig.y = v->y;
  vorig.z = v->z;

  vresult->x = vorig.x * m->_00 +
               vorig.y * m->_10 +
               vorig.z * m->_20 +
                         m->_30;
  vresult->y = vorig.x * m->_01 +
               vorig.y * m->_11 +
               vorig.z * m->_21 +
                         m->_31;
  vresult->z = vorig.x * m->_02 +
               vorig.y * m->_12 +
               vorig.z * m->_22 +
                         m->_32;
}

/*____________________________________________________________________
|
| Function: gx3d_MultiplyNormalVectorMatrix
|
| Output: Multiplies v * m, putting result in vresult.  v and vresult 
|   are 1x3 vectors.  v is a normal vector so don't need to add in the
|   translate part of the vector.  m should be a matrix that consists
|   of only rotation, translations and uniform scalings.
|___________________________________________________________________*/

void gx3d_MultiplyNormalVectorMatrix (gx3dVector *v, gx3dMatrix *m, gx3dVector *vresult)
{
  gx3dVector vorig;

  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (vresult);

  // Make a copy of v in case v and vresult are the same  
  vorig.x = v->x;
  vorig.y = v->y;
  vorig.z = v->z;

  vresult->x = vorig.x * m->_00 +
               vorig.y * m->_10 +
               vorig.z * m->_20;
  vresult->y = vorig.x * m->_01 +
               vorig.y * m->_11 +
               vorig.z * m->_21;
  vresult->z = vorig.x * m->_02 +
               vorig.y * m->_12 +
               vorig.z * m->_22;
}

/*____________________________________________________________________
|
| Function: gx3d_MultiplyVector4DMatrix
|
| Output: Multiplies v * m, putting result in vresult.  v and vresult 
|   are 1x4 vectors.
|___________________________________________________________________*/

void gx3d_MultiplyVector4DMatrix (gx3dVector4D *v, gx3dMatrix *m, gx3dVector4D *vresult)
{
  int i, s;
  float v4[4];

  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (vresult);

  // Copy v into a temp vector
//  memcpy (v4, v, sizeof(gx3dVector4D));
//  v4[3] = 1;
  v4[0] = v->x;
  v4[1] = v->y;
  v4[2] = v->z;
  v4[3] = v->w;
  // Zero vresult vector
  memset (vresult, 0, sizeof(gx3dVector4D));
  // Calculate vresult vector
  for (i=0; i<4; i++)
    for (s=0; s<4; s++)
      ((float *)vresult)[i] += (v4[s] * ((float *)m)[s*4+i]);
}

/*____________________________________________________________________
|
| Function: gx3d_MultiplyScalarVector
|
| Output: Multiplies a vector by a scalar, returning result in vresult
|   vector.  Has the effect of scaling the length of the vector by s.
|   Also, if s < 0, the direction of the vector is flipped.
|___________________________________________________________________*/

inline void gx3d_MultiplyScalarVector (float s, gx3dVector *v, gx3dVector *vresult)
{
  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (vresult);

  vresult->x = v->x * s;
  vresult->y = v->y * s;
  vresult->z = v->z * s;
}

/*____________________________________________________________________
|
| Function: gx3d_NormalizeVector
|
| Output: Normalizes a vector, returning result in vnormal vector.
|
| Complexity: (Worst/Average) 1 sqrt, 1 divide, 6 multiply
|___________________________________________________________________*/

inline void gx3d_NormalizeVector (gx3dVector *v, gx3dVector *normal)
{
  float m, magnitude;

  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (normal);

  // Get the magnitude of the vector 
  magnitude = gx3d_VectorMagnitude (v);

  // Compute the normal vector
  if (magnitude == 0)
    *normal = *v;
  else {
    m = 1 / magnitude;
    gx3d_MultiplyScalarVector (m, v, normal);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_NormalizeVector
|
| Output: Normalizes a vector, returning result in vnormal vector.
|
| Complexity: (Worst/Average) 1 sqrt, 1 divide, 6 multiply
|___________________________________________________________________*/

inline void gx3d_NormalizeVector (gx3dVector *v, gx3dVector *normal, float *magnitude)
{
  float m;

  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (normal);
  DEBUG_ASSERT (magnitude);

  // Get the magnitude of the vector 
  *magnitude = gx3d_VectorMagnitude (v);

  // Compute the normal vector
  if (*magnitude == 0)
    *normal = *v;
  else {
    m = 1 / *magnitude;
    gx3d_MultiplyScalarVector (m, v, normal);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_VectorMagnitude
|
| Output: Returns the magnitude of the vector (the length).
|
| Complexity: 1 sqrt, 3 multiply
|___________________________________________________________________*/

inline float gx3d_VectorMagnitude (gx3dVector *v)
{
  // Verify input params
  DEBUG_ASSERT (v);

  return (sqrtf (v->x*v->x + v->y*v->y + v->z*v->z));
}

/*____________________________________________________________________
|
| Function: gx3d_VectorDotProduct
|
| Output: Computes the dot product of two vectors.
| Note: The result is 0 if and only if v1 is perpendicular to v2 (and 
|   v1 and v2 are nonzero vectors).  If the result is > 0, the angle
|   between the two vectors is acute (betwween 0-89.9 degrees).  If the
|   result is < 0, the angle between the two vectors is obtuse (between
|   90.1-180).
|
| Complexity: 3 multiply
|___________________________________________________________________*/

inline float gx3d_VectorDotProduct (gx3dVector *v1, gx3dVector *v2)
{
  // Verify input params
  DEBUG_ASSERT (v1);
  DEBUG_ASSERT (v2);

  return (v1->x * v2->x + 
          v1->y * v2->y + 
          v1->z * v2->z);
}

/*____________________________________________________________________
|
| Function: gx3d_AngleBetweenVectors
|
| Output: Returns the angle in degrees between two non-zero vectors. 
|   If either vector is a zero vector (0,0,0), returns 0.
|___________________________________________________________________*/

inline float gx3d_AngleBetweenVectors (gx3dVector *v1, gx3dVector *v2)
{
  float divisor;
  float angle = 0;

  // Verify input params
  DEBUG_ASSERT (v1);
  DEBUG_ASSERT (v2);

  divisor = gx3d_VectorMagnitude (v1) * gx3d_VectorMagnitude (v2);
  if (divisor != 0)
    angle = safe_acosf (gx3d_VectorDotProduct (v1, v2) / divisor) * RADIANS_TO_DEGREES;
//#ifdef ERROR_CHECKING
//  else
//    gxError ("gx3d_AngleBetweenVectors(): one or both input vectors are 0 length vectors");
//#endif

  return (angle);
}

/*____________________________________________________________________
|
| Function: gx3d_AngleBetweenUnitVectors
|
| Output: Returns the angle in degrees between two unit vectors.  
| Note: Assumes the input vectors are unit vectors.
|
| Complexity: 1 acos, 4 multiply
|___________________________________________________________________*/

inline float gx3d_AngleBetweenUnitVectors (gx3dVector *v1, gx3dVector *v2)
{
  // Verify input params
  DEBUG_ASSERT (v1);
  DEBUG_ASSERT (v2);

  return (safe_acosf (gx3d_VectorDotProduct (v1, v2)) * RADIANS_TO_DEGREES);
}

/*____________________________________________________________________
|
| Function: gx3d_VectorCrossProdcut
|
| Output: Computes the cross product of two vectors.  If the two input
|   vectors are parallel the result is undefined.  The result vector
|   is a vector perpendicular to the two input vectors.  A vector
|   pointing in the opposite direction can be obtained by passing the
|   two input vectors switched - v2, v1 instead of v1, v2.
|
|   In a LHS, a x b points toward you if the vectors a,b make a clockwise
|   turn from your viewpoint.  If counterclockwise, a x b points away
|   from you.
|
|   Put tail of b at head of a to judge clockwise or counterclockwise
|   orientation.
|
| Complexity: 6 multiply
|___________________________________________________________________*/

inline void gx3d_VectorCrossProduct (gx3dVector *v1, gx3dVector *v2, gx3dVector *vresult)
{
  gx3dVector v;

  // Verify input params
  DEBUG_ASSERT (v1);
  DEBUG_ASSERT (v2);
  DEBUG_ASSERT (vresult);

  v.x = v1->y * v2->z - v1->z * v2->y;
  v.y = v1->z * v2->x - v1->x * v2->z;
  v.z = v1->x * v2->y - v1->y * v2->x;

  *vresult = v;
}

/*____________________________________________________________________
|
| Function: gx3d_AddVector
|
| Output: Computes vresult = v1 + v2
|___________________________________________________________________*/

inline void gx3d_AddVector (gx3dVector *v1, gx3dVector *v2, gx3dVector *vresult)
{
  // Verify input params
  DEBUG_ASSERT (v1);
  DEBUG_ASSERT (v2);
  DEBUG_ASSERT (vresult);

  vresult->x = v1->x + v2->x;
  vresult->y = v1->y + v2->y;
  vresult->z = v1->z + v2->z;
}

/*____________________________________________________________________
|
| Function: gx3d_SubtractVector
|
| Output: Computes vresult = v1 - v2.  This is useful to compute the
|   displacement from a to b, then compute b - a.
|___________________________________________________________________*/

inline void gx3d_SubtractVector (gx3dVector *v1, gx3dVector *v2, gx3dVector *vresult)
{
  // Verify input params
  DEBUG_ASSERT (v1);
  DEBUG_ASSERT (v2);
  DEBUG_ASSERT (vresult);

  vresult->x = v1->x - v2->x;
  vresult->y = v1->y - v2->y;
  vresult->z = v1->z - v2->z;
}

/*____________________________________________________________________
|
| Function: gx3d_NegateVector
|
| Output: Computes vresult = -v
|___________________________________________________________________*/

inline void gx3d_NegateVector (gx3dVector *v, gx3dVector *vresult)
{
  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (vresult);

  vresult->x = -v->x;
  vresult->y = -v->y;
  vresult->z = -v->z;
}

/*____________________________________________________________________
|
| Function: gx3d_ProjectVectorOntoVector
|
| Output: Projects vector v onto vector n, returning a vector parallel
|   to n and a vector perpindicular to n such that v = vparl + vperp.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 61,
|   Real-Time Rendering, 2nd ed., pg. 721.
|___________________________________________________________________*/

void gx3d_ProjectVectorOntoVector (gx3dVector *v, gx3dVector *n, gx3dVector *v_parallel, gx3dVector *v_perpindicular)
{
  float s, m;

  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (n);
  DEBUG_ASSERT (v_parallel);
  DEBUG_ASSERT (v_perpindicular);

//  m = gx3d_VectorMagnitude (n);  // not needed if n is a unit vector
//  m = m * m;
  m = gx3d_VectorDotProduct (n, n); // not needed if n is a unit vector
  s = gx3d_VectorDotProduct (v, n) / m;;
  gx3d_MultiplyScalarVector (s, n, v_parallel);    

  gx3d_SubtractVector (v, v_parallel, v_perpindicular);
}

/*____________________________________________________________________
|
| Function: gx3d_ProjectVectorOntoVector
|
| Output: Projects vector v onto vector n, returning a vector parallel
|   to n.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 61,
|   Real-Time Rendering, 2nd ed., pg. 721.
|___________________________________________________________________*/

void gx3d_ProjectVectorOntoVector (gx3dVector *v, gx3dVector *n, gx3dVector *v_parallel)
{
  float s, m;

  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (n);
  DEBUG_ASSERT (v_parallel);

//  m = gx3d_VectorMagnitude (n);  // not needed if n is a unit vector
//  m = m * m;
  m = gx3d_VectorDotProduct (n, n); // not needed if n is a unit vector
  s = gx3d_VectorDotProduct (v, n) / m;
  gx3d_MultiplyScalarVector (s, n, v_parallel);    
}

/*____________________________________________________________________
|
| Function: gx3d_ProjectVectorOntoUnitVector
|
| Output: Projects vector v onto vector n, returning a vector parallel
|   to n and a vector perpindicular to n such that v = vparl + vperp.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 61,
|   Real-Time Rendering, 2nd ed., pg. 721.
|___________________________________________________________________*/

void gx3d_ProjectVectorOntoUnitVector (gx3dVector *v, gx3dVector *n, gx3dVector *v_parallel, gx3dVector *v_perpindicular)
{
  float s;

  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (n);
  DEBUG_ASSERT (v_parallel);
  DEBUG_ASSERT (v_perpindicular);

  s = gx3d_VectorDotProduct (v, n);
  gx3d_MultiplyScalarVector (s, n, v_parallel);    

  gx3d_SubtractVector (v, v_parallel, v_perpindicular);
}

/*____________________________________________________________________
|
| Function: gx3d_ProjectVectorOntoUnitVector
|
| Output: Projects vector v onto vector n, returning a vector parallel
|   to n.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 61,
|   Real-Time Rendering, 2nd ed., pg. 721.
|___________________________________________________________________*/

void gx3d_ProjectVectorOntoUnitVector (gx3dVector *v, gx3dVector *n, gx3dVector *v_parallel)
{
  float s;

  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (n);
  DEBUG_ASSERT (v_parallel);

  s = gx3d_VectorDotProduct (v, n);
  gx3d_MultiplyScalarVector (s, n, v_parallel);    
}

/*____________________________________________________________________
|
| Function: gx3d_SurfaceNormal
|
| Output: Using 3 points, computes the normal vector, if possible.
|   Returns true if normal computed successfully else false if points
|   are collinear (and therefore cannot define a plane).
|
|   Assumes points are given in a clockwise order (for left-handed 
|   coordinate system). 
|___________________________________________________________________*/

int gx3d_SurfaceNormal (gx3dVector *p1, gx3dVector *p2, gx3dVector *p3, gx3dVector *normal)
{
  gx3dVector a, b;
  int success = FALSE;

  // Verify input params
  DEBUG_ASSERT (p1);
  DEBUG_ASSERT (p2);
  DEBUG_ASSERT (p3);
  DEBUG_ASSERT (normal);

  // Compute 2 vectors from 3 points
  gx3d_SubtractVector (p2, p1, &a);
  gx3d_SubtractVector (p3, p1, &b);
    
  // Compute cross product of 2 vectors
  gx3d_VectorCrossProduct (&a, &b, normal);

  // Are 3 points non-collinear?
  if ((normal->x != 0) OR (normal->y != 0) OR (normal->z != 0)) {
    // Normalize the result
    gx3d_NormalizeVector (normal, normal);
    success = TRUE;
  }

  return (success);
}

/*____________________________________________________________________
|
| Function: gx3d_Lerp
|
| Output: Linerally interpolates between two points for a given distance.
|___________________________________________________________________*/

inline float gx3d_Lerp (float start, float end, float t)  // t is between 0-1 normally
{
  return (start + t * (end - start));
}

/*____________________________________________________________________
|
| Function: gx3d_LerpVector
|
| Output: Linerally interpolates between two vectors for a given distance.
|___________________________________________________________________*/

void gx3d_LerpVector (gx3dVector *start, gx3dVector *end, float t, gx3dVector *vresult)  // t is between 0-1 normally
{
  vresult->x = gx3d_Lerp (start->x, end->x, t);
  vresult->y = gx3d_Lerp (start->y, end->y, t);
  vresult->z = gx3d_Lerp (start->z, end->z, t);
}

/*____________________________________________________________________
|
| Function: gx3d_Clamp
|
| Output: Returns value clamped to range low,high.
|___________________________________________________________________*/

inline float gx3d_Clamp (float value, float low, float high)
{
  if (value < low)
    return (low);
  else if (value > high)
    return (high);
  else
    return (value);
}

/*____________________________________________________________________
|
| Function: gx3d_GetPlane
|
| Output: Calculates a plane given 3 points.  The plane is defined by
|   ax + by + cz + d = 0.
|___________________________________________________________________*/

void gx3d_GetPlane (gx3dVector *p1, gx3dVector *p2, gx3dVector *p3, gx3dPlane *plane)
{
  gx3dVector normal;

  // Verify input params
  DEBUG_ASSERT (p1);
  DEBUG_ASSERT (p2);
  DEBUG_ASSERT (p3);
  DEBUG_ASSERT (plane);

  gx3d_SurfaceNormal (p1, p2, p3, &normal);
  plane->n.x = normal.x;
  plane->n.y = normal.y;
  plane->n.z = normal.z;
  plane->d = -gx3d_VectorDotProduct (&normal, p1);
}

/*____________________________________________________________________
|
| Function: gx3d_GetPlane
|                              
| Output: Calculates a plane given a normal and a point on the plane.
|   The plane is defined by ax + by + dz + d = 0
|___________________________________________________________________*/

void gx3d_GetPlane (gx3dVector *point, gx3dVector *normal, gx3dPlane *plane)
{
  // Verify input params
  DEBUG_ASSERT (point);
  DEBUG_ASSERT (normal);
  DEBUG_ASSERT (plane);

  plane->n.x = normal->x;
  plane->n.y = normal->y;
  plane->n.z = normal->z;
  plane->d = -gx3d_VectorDotProduct (normal, point);
}

/*____________________________________________________________________
|
| Function: gx3d_GetBillboardRotateXYMatrix
|
| Output: Computes the matrix from user parameters.  Assumes world up
|   is (0,1,0).
|___________________________________________________________________*/

void gx3d_GetBillboardRotateXYMatrix (gx3dMatrix *m, gx3dVector *billboard_normal, gx3dVector *view_normal)
{
  float angle;
  gx3dVector v_view, v_billboard, vn;
  gx3dMatrix m_xrot, m_yrot;

  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (billboard_normal);
  DEBUG_ASSERT (view_normal);

  // Invert view normal
  vn.x = view_normal->x * -1;
  vn.y = view_normal->y * -1;
  vn.z = view_normal->z * -1;

  // Compute angle between the flattened normal and the xz plane
  angle = 0;
  if (vn.y)
    angle += (asinf (vn.y) * RADIANS_TO_DEGREES);
  // Compute angle between the flattened normal and the xz plane
  if (billboard_normal->y)
    angle += (asinf (billboard_normal->y) * RADIANS_TO_DEGREES);
  // Build the x-axis rotation needed
  gx3d_GetRotateXMatrix (&m_xrot, angle);

  // Flatten out view normal onto the xz plane
  v_view.x = vn.x;
  v_view.y = 0;
  v_view.z = vn.z;
//  gx3d_NormalizeVector (&v_view, &v_view);
  // Flatten out billboard normal onto the xz plane
  v_billboard.x = billboard_normal->x;
  v_billboard.y = 0;
  v_billboard.z = billboard_normal->z;
//  gx3d_NormalizeVector (&v_billboard, &v_billboard);
  // Compute angle between the two
//  angle = safe_acosf (gx3d_VectorDotProduct (&v_view, &v_billboard)) * RADIANS_TO_DEGREES;
  angle = gx3d_AngleBetweenVectors (&v_view, &v_billboard);
  if ((v_view.x - v_billboard.x) > 0)
    angle *= -1;
  // Build the y-axis rotation needed
  gx3d_GetRotateYMatrix (&m_yrot, angle);

  // Build final matrix
  gx3d_MultiplyMatrix (&m_xrot, &m_yrot, m);
}

/*____________________________________________________________________
|
| Function: gx3d_GetBillboardXMatrix
|
| Output: Computes the matrix from user parameters.  Assumes world up
|   is (0,1,0).
|___________________________________________________________________*/

void gx3d_GetBillboardRotateXMatrix (gx3dMatrix *m, gx3dVector *billboard_normal, gx3dVector *view_normal)
{
  float angle;
  gx3dVector vn;

  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (billboard_normal);
  DEBUG_ASSERT (view_normal);

  // Invert view normal
  vn.x = view_normal->x * -1;
  vn.y = view_normal->y * -1;
  vn.z = view_normal->z * -1;

  // Compute angle between the flattened normal and the xz plane
  angle = 0;
  if (vn.y)
    angle += (asinf (vn.y) * RADIANS_TO_DEGREES);
  // Compute angle between the flattened normal and the xz plane
  if (billboard_normal->y)
    angle += (asinf (billboard_normal->y) * RADIANS_TO_DEGREES);
  // Build the x-axis rotation needed
  gx3d_GetRotateXMatrix (m, angle);
}

/*____________________________________________________________________
|
| Function: gx3d_GetBillboardRotateYMatrix
|
| Output: Computes the matrix from user parameters.  Assumes world up
|   is (0,1,0).
|___________________________________________________________________*/

void gx3d_GetBillboardRotateYMatrix (gx3dMatrix *m, gx3dVector *billboard_normal, gx3dVector *view_normal)
{
  float angle;
  gx3dVector v_view, v_billboard, vn;

  // Verify input params
  DEBUG_ASSERT (m);
  DEBUG_ASSERT (billboard_normal);
  DEBUG_ASSERT (view_normal);

  // Invert view normal
  vn.x = view_normal->x * -1;
  vn.y = view_normal->y * -1;
  vn.z = view_normal->z * -1;

  // Flatten out view normal onto the xz plane
  v_view.x = vn.x;
  v_view.y = 0;
  v_view.z = vn.z;
//  gx3d_NormalizeVector (&v_view, &v_view);
  // Flatten out billboard normal onto the xz plane
  v_billboard.x = billboard_normal->x;
  v_billboard.y = 0;
  v_billboard.z = billboard_normal->z;
//  gx3d_NormalizeVector (&v_billboard, &v_billboard);
  // Compute angle between the two
//  angle = safe_acosf (gx3d_VectorDotProduct (&v_view, &v_billboard)) * RADIANS_TO_DEGREES;
  angle = gx3d_AngleBetweenVectors (&v_view, &v_billboard);
  if ((v_view.x - v_billboard.x) < 0)
    angle *= -1;
  // Build the y-axis rotation needed
  gx3d_GetRotateYMatrix (m, angle);
}

/*____________________________________________________________________
|
| Function: gx3d_HeadingToXZVector
|
| Output: Converts a heading 0-360 (where 0 degrees is 0,0,1 and 90 
|   degrees is 1,0,0) into a normalized vector flat on the xz plane.
|___________________________________________________________________*/

void gx3d_HeadingToXZVector (float heading, gx3dVector *v)
{
  float s, c;
  gx3dVector north = {0,0,1};

  // Verify input params
  DEBUG_ASSERT (v);

  // Limit heading to 0-359 
  if (heading < 0)
    heading += (360 * (fabsf(heading) / 360 + 1));
  else if (heading >= 360)
    heading -= (360 * (heading / 360));

  s = sinf (heading * DEGREES_TO_RADIANS);
  c = cosf (heading * DEGREES_TO_RADIANS);
  v->x = north.x * c + north.z * s;
  v->y = 0;
  v->z = north.x * -s + north.z * c;
  gx3d_NormalizeVector (v, v);
}

/*____________________________________________________________________
|
| Function: gx3d_HeadingToYZVector
|
| Output: Converts a heading 0-360 (where 0 degrees is 0,0,1 and 90 
|   degrees is 1,0,0) into a normalized vector flat on the yz plane.
|___________________________________________________________________*/

void gx3d_HeadingToYZVector (float heading, gx3dVector *v)
{
  float s, c;
  gx3dVector north = {0,0,1};

  // Verify input params
  DEBUG_ASSERT (v);

  // Limit heading to 0-359 
  if (heading < 0)
    heading += (360 * (fabsf(heading) / 360 + 1));
  else if (heading >= 360)
    heading -= (360 * (heading / 360));

  s = sinf (heading * DEGREES_TO_RADIANS);
  c = cosf (heading * DEGREES_TO_RADIANS);
  v->x = 0;
  v->y = north.y * c + north.z * s;
  v->z = north.y * -s + north.z * c;
  gx3d_NormalizeVector (v, v);
}

/*____________________________________________________________________
|
| Function: gx3d_XZVectorToHeading
|
| Output: Converts a vector into a heading.
|___________________________________________________________________*/

void gx3d_XZVectorToHeading (gx3dVector *v, float *heading)
{
  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (heading);

  if (v->x == 0) {
    if (v->z >= 0)
      *heading = 0;
    else
      *heading = 180;
  }
  else if (v->x > 0)
    *heading = 90 - atanf(v->z/v->x) * RADIANS_TO_DEGREES;
  else
    *heading = 270 - atanf(v->z/v->x) * RADIANS_TO_DEGREES;
}

/*____________________________________________________________________
|
| Function: gx3d_YZVectorToHeading
|
| Output: Converts a vector into a heading.
|___________________________________________________________________*/

void gx3d_YZVectorToHeading (gx3dVector *v, float *heading)
{
  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (heading);

  if (v->y == 0) {
    if (v->z >= 0)
      *heading = 0;
    else
      *heading = 180;
  }
  else if (-v->y > 0)
    *heading = 90 - atanf(v->z/-v->y) * RADIANS_TO_DEGREES;
  else
    *heading = 270 - atanf(v->z/-v->y) * RADIANS_TO_DEGREES;
}
