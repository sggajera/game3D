/*____________________________________________________________________
|
| File: gx3d_quaternion.cpp
|
| Description: Functions for quaternions.
|
| Functions:  gx3d_GetAxisAngleQuaternion
|             gx3d_GetQuaternionAxisAngle
|             gx3d_GetMatrixQuaternion
|             gx3d_GetEulerQuaternion
|             gx3d_GetQuaternionMatrix
|             gx3d_MultiplyQuaternion
|             gx3d_QuaternionDotProduct
|             gx3d_GetLerpQuaternion
|             gx3d_GetSlerpQuaternion
|             gx3d_GetIdentityQuaternion
|             gx3d_GetInverseQuaternion
|             gx3d_GetConjugateQuaternion
|             gx3d_NormalizeQuaternion
|             gx3d_NormalizeQuaternion
|             gx3d_MultiplyVectorQuaternion
|             gx3d_ScaleQuaternion
|             gx3d_SubtractQuaternion
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
| Macros
|__________________*/

#define QUATERNION_MAGNITUDE(_q_) (_q_->w * _q_->w + _q_->x * _q_->x + _q_->y * _q_->y + _q_->z * _q_->z)

/*____________________________________________________________________
|
| Function: gx3d_GetQuaternionAxisAngle
|
| Output: Returns the axis angle from a quaternion.
|___________________________________________________________________*/

void gx3d_GetQuaternionAxisAngle (gx3dQuaternion *q, gx3dVector *axis, float *angle)
{
  float sqr_len, inv_len;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (q);
  DEBUG_ASSERT (axis);
  DEBUG_ASSERT (angle);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  sqr_len = q->x * q->x + q->y * q->y + q->z * q->z;
  if (sqr_len > 0) {
    *angle = safe_acosf (q->w) * 2 * RADIANS_TO_DEGREES;
    inv_len = 1 / sqrtf (sqr_len);
    axis->x = q->x * inv_len;
    axis->y = q->y * inv_len;
    axis->z = q->z * inv_len;
  }
  else {
    // Angle is 0 (mod 2*pi), so any axis will work
    *angle = 0;
    q->x = 1;
    q->y = 0;
    q->z = 0;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_GetAxisAngleQuaternion
|
| Output: Builds a quaternion of unit length from an angle and axis.
|   The input axis doesn't have to be a unit vector since this function
|   takes care of that.
|___________________________________________________________________*/

void gx3d_GetAxisAngleQuaternion (gx3dVector *axis, float angle, gx3dQuaternion *q)
{
  float s, half_angle;
  gx3dVector normal;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (axis);
  DEBUG_ASSERT (q);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  gx3d_NormalizeVector (axis, &normal);
  angle *= DEGREES_TO_RADIANS;
  half_angle = (float)0.5 * angle;
  s = sinf (half_angle);
  q->w = cosf (half_angle);
  q->x = s * normal.x;
  q->y = s * normal.y;
  q->z = s * normal.z;
}

/*____________________________________________________________________
|
| Function: gx3d_GetMatrixQuaternion 
|
| Output: Builds a quaternion from a rotation matrix.
|
| Note: Algorithm from Ken Shoemake's article in 1987 SIGGRAPH course
|   notes "Quaternion Calculus and Fast Animation"
|___________________________________________________________________*/

void gx3d_GetMatrixQuaternion (gx3dMatrix *m, gx3dQuaternion *q)
{
  int i, j, k;
  float root, trace, *quat[3];
  static int next[3] = {1, 2, 0};

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (m);
  DEBUG_ASSERT (q);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  trace = 1 + m->_00 + m->_11 + m->_22;

  if (trace > 0) {
    // |w| > 1/2, may as well choose w > 1/2
    root = sqrtf (trace);
    q->w = (float)0.5 * root;
    if (root != 0)
      root = (float)0.5 / root;
    q->x = (m->_21 - m->_12) * root;
    q->y = (m->_02 - m->_20) * root;
    q->z = (m->_10 - m->_01) * root;
  }
  else {
    // |w| <= 1/2
    i = 0;
    if (m->_11 > m->_00)
      i = 1;
    if (m->_22 > ((float *)m)[i*4+i])
      i = 2;
    j = next[i];
    k = next[j];
    root = sqrtf (((float *)m)[i*4+i] - ((float *)m)[j*4+j] - ((float *)m)[k*4+k] + 1);

    quat[0] = &(q->x);
    quat[1] = &(q->y);
    quat[2] = &(q->z);

    *quat[i] = (float)0.5 * root;
    if (root != 0)
      root = (float)0.5 / root;
    q->w     = (((float *)m)[k*4+j] - ((float *)m)[j*4+k]) * root;
    *quat[j] = (((float *)m)[j*4+i] - ((float *)m)[i*4+j]) * root;
    *quat[k] = (((float *)m)[k*4+i] - ((float *)m)[i*4+k]) * root;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_GetEulerQuaternion 
|
| Output: Builds a quaternion from an euler angle
|
| Note: Algorithm from Nick Bobick's article in Feb. 98 Game Developer,
|   "Rotating Objects Using Quaternions"
|___________________________________________________________________*/

void gx3d_GetEulerQuaternion (float roll, float pitch, float yaw, gx3dQuaternion *q)
{
  float cr, cp, cy, sr, sp, sy, cpcy, spsy;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (q);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Calculate trig identities
  cr = cosf (roll  * 0.5f);
  cp = cosf (pitch * 0.5f);
  cy = cosf (yaw   * 0.5f);
  sr = sinf (roll  * 0.5f);
  sp = sinf (pitch * 0.5f);
  sy = sinf (yaw   * 0.5f);

  cpcy = cp * cy;
  spsy = sp * sy;

  q->x = sr * cpcy - cr * spsy;
  q->y = cr * sp * cy + sr * cp * sy;
  q->z = cr * cp * sy - sr * sp * cy;
  q->w = cr * cpcy + sr * spsy;
}

/*____________________________________________________________________
|
| Function: gx3d_GetQuaternionMatrix
|
| Output: Builds a rotation matrix from a quaternion.
|___________________________________________________________________*/

void gx3d_GetQuaternionMatrix (gx3dQuaternion *q, gx3dMatrix *m)
{
  float x2, y2, z2, wx, wy, wz, xx, xy, xz, yy, yz, zz;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (q);
  DEBUG_ASSERT (m);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  x2 = q->x + q->x;
  y2 = q->y + q->y;
  z2 = q->z + q->z;
  wx = q->w * x2;
  wy = q->w * y2;
  wz = q->w * z2;
  xx = q->x * x2;
  xy = q->x * y2;
  xz = q->x * z2;
  yy = q->y * y2;
  yz = q->y * z2;
  zz = q->z * z2;
/*
  m->_00 = (float)1 - (yy + zz);
  m->_01 = xy + wz;
  m->_02 = xz - wy;
  m->_03 = 0;
  m->_10 = xy - wz;
  m->_11 = (float)1 - (xx + zz);
  m->_12 = yz + wx;
  m->_13 = 0;
  m->_20 = xz + wy;
  m->_21 = yz - wx;
  m->_22 = (float)1 - (xx + yy);
  m->_23 = 0;
  m->_30 = 0;
  m->_31 = 0;
  m->_32 = 0;
  m->_33 = 1;
*/
  m->_00 = (float)1 - (yy + zz);    // I think this is the OpenGL version - Nope, looks like the right one (2/25/12)
  m->_01 = xy - wz;
  m->_02 = xz + wy;
  m->_03 = 0;
  m->_10 = xy + wz;
  m->_11 = (float)1 - (xx + zz);
  m->_12 = yz - wx;
  m->_13 = 0;
  m->_20 = xz - wy;
  m->_21 = yz + wx;
  m->_22 = (float)1 - (xx + yy);
  m->_23 = 0;
  m->_30 = 0;
  m->_31 = 0;
  m->_32 = 0;
  m->_33 = 1;
}

/*____________________________________________________________________
|
| Function: gx3d_MultiplyQuaternion
|
| Output: Multiplies two quaternions.
|
| Notes: When concatenating (multiplying) quaternions together, they
|   should be multiplied in reverse order (in contrast to matrices which
|   are multliplied in order to achieve a certain sequence of transformations).
|___________________________________________________________________*/

void gx3d_MultiplyQuaternion (gx3dQuaternion *q1, gx3dQuaternion *q2, gx3dQuaternion *qresult)
{
  gx3dQuaternion qtemp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (q1);
  DEBUG_ASSERT (q2);
  DEBUG_ASSERT (qresult);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  qtemp.x = (q1->w * q2->x) + (q1->x * q2->w) + (q1->y * q2->z) - (q1->z * q2->y);
  qtemp.y = (q1->w * q2->y) + (q1->y * q2->w) + (q1->z * q2->x) - (q1->x * q2->z);
  qtemp.z = (q1->w * q2->z) + (q1->z * q2->w) + (q1->x * q2->y) - (q1->y * q2->x);
  qtemp.w = (q1->w * q2->w) - (q1->x * q2->x) - (q1->y * q2->y) - (q1->z * q2->z);

  // Put result into qresult
  *qresult = qtemp;
}

/*____________________________________________________________________
|
| Function: gx3d_QuaternionDotProduct
|
| Output: Gets dot product of two quaternions.
|___________________________________________________________________*/

inline float gx3d_QuaternionDotProduct (gx3dQuaternion *q1, gx3dQuaternion *q2)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (q1);
  DEBUG_ASSERT (q2);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (q1->w * q2->w + 
          q1->x * q2->x +
          q1->y * q2->y + 
          q2->z * q2->z);
}

/*____________________________________________________________________
|
| Function: gx3d_GetLerpQuaternion
|
| Output: Lineraly interpolates between two quaternions returning a 
|   result quaternion.  Amount should be a value between 0 and 1.
|___________________________________________________________________*/

void gx3d_GetLerpQuaternion (gx3dQuaternion *from, gx3dQuaternion *to, float amount, gx3dQuaternion *qresult)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (from);
  DEBUG_ASSERT (to);
  DEBUG_ASSERT (qresult);
  DEBUG_ASSERT ((amount >= 0) AND (amount <= 1));

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Note: if 2 quaternions are normalized then qresult should be normalized
  qresult->x = gx3d_Lerp (from->x, to->x, amount);
  qresult->y = gx3d_Lerp (from->y, to->y, amount);
  qresult->z = gx3d_Lerp (from->z, to->z, amount);
  qresult->w = gx3d_Lerp (from->w, to->w, amount);
}

/*____________________________________________________________________
|
| Function: gx3d_GetSlerpQuaternion
|
| Output: Spherically interpolates between two quaternions returning a 
|   result quaternion.  Amount should be a value between 0 and 1.
|
| Reference: 3D Math Primer for Graphics and Game Development, pg. 176.
|___________________________________________________________________*/

//#define EPSILON ((float)0.001)

void gx3d_GetSlerpQuaternion (gx3dQuaternion *from, gx3dQuaternion *to, float amount, gx3dQuaternion *qresult)
{
  float theta, sin_theta, cos_theta, k0, k1, one_over_sin_theta;
  gx3dQuaternion qtemp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (from);
  DEBUG_ASSERT (to);
  DEBUG_ASSERT (qresult);
  DEBUG_ASSERT ((amount >= 0) AND (amount <= 1));

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute cos of angle between them using dot product
  cos_theta = gx3d_QuaternionDotProduct (from, to);
  // If negative dot, negate one of the input quaternions to take the shorter 4D arc
  if (cos_theta < 0) {
    qtemp.x = -from->x;
    qtemp.y = -from->y;
    qtemp.z = -from->z;
    qtemp.w = -from->w;
    cos_theta = -cos_theta;
  }
  else
    qtemp = *from;
  // Check is they are very close to prevent divide by zero
  if (cos_theta > 0.9999f) {
    // Very close - just use Lerp
    k0 = 1 - amount;
    k1 = amount;
  }
  else {
    // Compute sin of the angle using trig identity sin^2(theta) + cos^2(theta) = 1
    sin_theta = sqrtf (1 - cos_theta * cos_theta);
    // Compute the angle from its sin and cos
    theta = atan2f (sin_theta, cos_theta);
    // Comput inverse of denominator, so only have to divide once
    one_over_sin_theta = 1 / sin_theta;
    // Compute interpolation params
    k0 = sinf ((1 - amount) * theta) * one_over_sin_theta;
    k1 = sin (amount * theta) * one_over_sin_theta;
  }
  // Interpolate
  qresult->x = qtemp.x * k0 + to->x * k1;
  qresult->y = qtemp.y * k0 + to->y * k1;
  qresult->z = qtemp.z * k0 + to->z * k1;
  qresult->w = qtemp.w * k0 + to->w * k1;

  // My old code - was working!
  //cos_theta = gx3d_QuaternionDotProduct (from, to);
  //theta     = safe_acosf (cos_theta);
  //sin_theta = sinf (theta);

  //if (sin_theta > EPSILON) {
  //  one_over_sin_theta = 1.0f / sin_theta;
  //  k0 = sinf ((1 - amount) * theta) * one_over_sin_theta;
  //  k1 = sinf (amount * theta) * one_over_sin_theta;
  //}
  //else {
  //  k0 = 1 - amount;
  //  k1 = amount;
  //}
  //qresult->x = from->x * k0 + to->x * k1;
  //qresult->y = from->y * k0 + to->y * k1;
  //qresult->z = from->z * k0 + to->z * k1;
  //qresult->w = from->w * k0 + to->w * k1;
}

//#undef EPSILON

/*____________________________________________________________________
|
| Function: gx3d_GetIdentityQuaternion
|
| Output: Returns an identity quaternion.
|___________________________________________________________________*/

inline void gx3d_GetIdentityQuaternion (gx3dQuaternion *q)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (q);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  q->x = 0;
  q->y = 0;
  q->z = 0;
  q->w = 1;
}

/*____________________________________________________________________
|
| Function: gx3d_GetInverseQuaternion
|
| Output: Computes the inverse of a quaternion if possible.  Returns true
|   on success, else false if the inverse can't be computed.
|___________________________________________________________________*/

int gx3d_GetInverseQuaternion (gx3dQuaternion *q, gx3dQuaternion *qinverse)
{
  float norm, inv_norm;
  int inverse_computed = FALSE;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (q);
  DEBUG_ASSERT (qinverse);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  norm = QUATERNION_MAGNITUDE(q);
  if (norm > 0) {
    inv_norm = (float)1 / norm;
    qinverse->x = -q->x * inv_norm;
    qinverse->y = -q->y * inv_norm;
    qinverse->z = -q->z * inv_norm;
    qinverse->w =  q->w * inv_norm;
    inverse_computed = TRUE;
  }

  return (inverse_computed);
}

/*____________________________________________________________________
|
| Function: gx3d_GetConjugateQuaternion
|
| Output: Computes the conjugate of a quaternion.  If the quaternion
|   is normalized then the conjugate is also the inverse.
|
| Reference: Game Engine Architecture, pg. 171.
|___________________________________________________________________*/

void gx3d_GetConjugateQuaternion (gx3dQuaternion *q, gx3dQuaternion *qconjugate)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (q);
  DEBUG_ASSERT (qconjugate);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute the conjugate quaternion
  qconjugate->x = -q->x;
  qconjugate->y = -q->y;
  qconjugate->z = -q->z;
  qconjugate->w =  q->w;
}

/*____________________________________________________________________
|
| Function: gx3d_NormalizeQuaternion
|
| Output: Normalizes a quaternion.
|___________________________________________________________________*/

inline void gx3d_NormalizeQuaternion (gx3dQuaternion *q)
{
  gx3d_NormalizeQuaternion (q, q);
}

/*____________________________________________________________________
|
| Function: gx3d_NormalizeQuaternion
|
| Output: Normalizes a quaternion, returning result in qnormal quaternion.
|___________________________________________________________________*/

#define EPSILON ((float)0.00001)

inline void gx3d_NormalizeQuaternion (gx3dQuaternion *q, gx3dQuaternion *qnormal)
{
  float v, magnitude;

  // Verify input params
  DEBUG_ASSERT (q);
  DEBUG_ASSERT (qnormal);

  // Get the magnitude of the quaternion 
  v = QUATERNION_MAGNITUDE(q);
  // Need to take square root?
  if (fabs(1 - v) > EPSILON)
    magnitude = sqrtf (v);
  else
    magnitude = 1;

  // Compute the normal
  if ((magnitude == 0) OR (magnitude == 1))
    *qnormal = *q;
  else {
    qnormal->x = q->x / magnitude;
    qnormal->y = q->y / magnitude;
    qnormal->z = q->z / magnitude;
    qnormal->w = q->w / magnitude;
  }
}

#undef EPSILON

/*____________________________________________________________________
|
| Function: gx3d_MultiplyVectorQuaternion
|
| Output: Multiplies v * q, putting result in vresult.
|
| Reference: Game Engine Architecture, pg. 172.
|___________________________________________________________________*/

void gx3d_MultiplyVectorQuaternion (gx3dVector *v, gx3dQuaternion *q, gx3dVector *vresult)
{
  gx3dQuaternion vq, qi;

  // Verify input params
  DEBUG_ASSERT (v);
  DEBUG_ASSERT (q);
  DEBUG_ASSERT (vresult);

  // Put the vector into quaternion form
  vq.x = v->x;
  vq.y = v->y;
  vq.z = v->z;
  vq.w = 0;

  // vresult = q * v * q(inverse)
  gx3d_GetInverseQuaternion (q, &qi);
  gx3d_MultiplyQuaternion (q, &vq, &vq);
  gx3d_MultiplyQuaternion (&vq, &qi, &vq);
  
  // Extract the new vector
  vresult->x = vq.x;
  vresult->y = vq.y;
  vresult->z = vq.z;
}

/*____________________________________________________________________
|
| Function: gx3d_ScaleQuaternion
|
| Output: Scales a unit quaternion.
|___________________________________________________________________*/

void gx3d_ScaleQuaternion (gx3dQuaternion *q, float amount, gx3dQuaternion *qresult)
{
  // Verify input params
  DEBUG_ASSERT (q);
  DEBUG_ASSERT (qresult);

  gx3dQuaternion qidentity;
  gx3d_GetIdentityQuaternion (&qidentity);
  gx3d_GetLerpQuaternion (&qidentity, q, amount, qresult);

  // Scale the quaternion
  //qresult->x = q->x * amount;
  //qresult->y = q->y * amount;
  //qresult->z = q->z * amount;
  //qresult->w = q->w * amount;
}

/*____________________________________________________________________
|
| Function: gx3d_SubtractQuaternion
|
| Output: Computes q1 - q2.  The resulting quaternion is sufficient to
|   rotate from q1 to q2 (it is the difference between them).
|   
|   If using rotation quaternions, probably should normalize the result
|   returned by this function.
|___________________________________________________________________*/

void gx3d_SubtractQuaternion (gx3dQuaternion *q1, gx3dQuaternion *q2, gx3dQuaternion *qresult)
{
  gx3dQuaternion qi2;

  // Verify input params
  DEBUG_ASSERT (q1);
  DEBUG_ASSERT (q2);
  DEBUG_ASSERT (qresult);

  gx3d_GetInverseQuaternion (q2, &qi2);
  // tq = q1 * q2(inverse)
  gx3d_MultiplyQuaternion (q1, &qi2, qresult);
}
