/*____________________________________________________________________
|
| File: gx3d_blendmask.cpp
|
| Description: Functions to manipulate a gx3dBlendMask.
|
| Functions:  gx3d_BlendMask_Init
|             gx3d_BlendMask_Free
|             gx3d_BlendMask_Set_All
|             gx3d_BlendMask_Set_Bone
|             gx3d_BlendMask_Set_Bone
|             gx3d_BlendMask_Set_Chain
|             gx3d_BlendMask_Set_Chain
|              Set_Chain
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

#include "dp.h"

/*___________________
|
| Function prototypes
|__________________*/

static void Set_Chain (gx3dBlendMask *blendmask, int bone_index, float value);

/*____________________________________________________________________
|
| Function: gx3d_BlendMask_Init
| 
| Output: Creates a blend mask with all amounts set to initial_value.
|   Initial value should be between 0-1.
|   Returns pointer to blend mask or 0 on any error.
|___________________________________________________________________*/

gx3dBlendMask *gx3d_BlendMask_Init (gx3dMotionSkeleton *skeleton, float initial_value)
{
  gx3dBlendMask *blendmask = 0;
 
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skeleton)
  DEBUG_ASSERT (skeleton->num_bones)
  DEBUG_ASSERT (skeleton->bones)
  DEBUG_ASSERT ((initial_value >= 0) AND (initial_value <= 1))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Allocate memory for top-level data struct
  blendmask = (gx3dBlendMask *) malloc (sizeof(gx3dBlendMask));
  if (blendmask == 0)
    TERMINAL_ERROR ("gx3d_BlendMask_Init(): can't allocate memory for blend mask");
  // Point to skeleton
  blendmask->skeleton = skeleton;
  // Allocate memory for array of amounts
  blendmask->values = (float *) malloc (blendmask->skeleton->num_bones * sizeof(float));
  if (blendmask->values == 0) {
    free (blendmask);
    TERMINAL_ERROR ("gx3d_BlendMask_Init(): can't allocate memory amounts array");
  }
  // Init amounts
  gx3d_BlendMask_Set_All (blendmask, initial_value);

  return (blendmask);
}

/*____________________________________________________________________
|
| Function: gx3d_BlendMask_Free
| 
| Output: Frees memory for blend mask.
|___________________________________________________________________*/

void gx3d_BlendMask_Free (gx3dBlendMask *blendmask)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendmask)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Free array of values first
  if (blendmask->values)
    free (blendmask->values);
  // Free top-level struct
  free (blendmask);
}

/*____________________________________________________________________
|
| Function: gx3d_BlendMask_Set_All
| 
| Output: Sets all values in mask to value.
|___________________________________________________________________*/

void gx3d_BlendMask_Set_All (gx3dBlendMask *blendmask, float value)
{
  int i;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendmask)
  DEBUG_ASSERT (blendmask->skeleton)
  DEBUG_ASSERT (blendmask->skeleton->num_bones)
  DEBUG_ASSERT (blendmask->values)
  DEBUG_ASSERT ((value >= 0) AND (value <= 1))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (i=0; i<blendmask->skeleton->num_bones; i++)
    blendmask->values[i] = value;
}

/*____________________________________________________________________
|
| Function: gx3d_BlendMask_Set_Bone
| 
| Output: Sets a bone value in mask to value.
|___________________________________________________________________*/

void gx3d_BlendMask_Set_Bone (gx3dBlendMask *blendmask, char *bone_name, float value)
{
  int bone_index;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendmask)
  DEBUG_ASSERT (blendmask->skeleton)
  DEBUG_ASSERT (blendmask->skeleton->num_bones)
  DEBUG_ASSERT (blendmask->values)
  DEBUG_ASSERT ((value >= 0) AND (value <= 1))
  DEBUG_ASSERT (bone_name);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Find index of bone in skeleton
  if (gx3d_MotionSkeleton_GetBoneIndex (blendmask->skeleton, bone_name, &bone_index))
    blendmask->values[bone_index] = value;
}

/*____________________________________________________________________
|
| Function: gx3d_BlendMask_Set_Bone
| 
| Output: Sets a bone value in mask to value.
|___________________________________________________________________*/

void gx3d_BlendMask_Set_Bone (gx3dBlendMask *blendmask, int bone_index, float value)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendmask)
  DEBUG_ASSERT (blendmask->skeleton)
  DEBUG_ASSERT (blendmask->skeleton->num_bones)
  DEBUG_ASSERT (blendmask->values)
  DEBUG_ASSERT ((value >= 0) AND (value <= 1))
  DEBUG_ASSERT ((bone_index >= 0) AND (bone_index < blendmask->skeleton->num_bones))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Set the value
  blendmask->values[bone_index] = value;
}

/*____________________________________________________________________
|
| Function: gx3d_BlendMask_Set_Chain
| 
| Output: Sets a chain of bone values in mask to value.
|___________________________________________________________________*/

void gx3d_BlendMask_Set_Chain (gx3dBlendMask *blendmask, char *bone_name, float value)
{
  int bone_index;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendmask)
  DEBUG_ASSERT (blendmask->skeleton)
  DEBUG_ASSERT (blendmask->skeleton->num_bones)
  DEBUG_ASSERT (blendmask->values)
  DEBUG_ASSERT ((value >= 0) AND (value <= 1))
  DEBUG_ASSERT (bone_name);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Find index of bone in skeleton
  if (gx3d_MotionSkeleton_GetBoneIndex (blendmask->skeleton, bone_name, &bone_index))
    Set_Chain (blendmask, bone_index, value);
}

/*____________________________________________________________________
|
| Function: gx3d_BlendMask_Set_Chain
| 
| Output: Sets a chain of bone values in mask to value.
|___________________________________________________________________*/

void gx3d_BlendMask_Set_Chain (gx3dBlendMask *blendmask, int bone_index, float value)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendmask)
  DEBUG_ASSERT (blendmask->skeleton)
  DEBUG_ASSERT (blendmask->skeleton->num_bones)
  DEBUG_ASSERT (blendmask->values)
  DEBUG_ASSERT ((value >= 0) AND (value <= 1))
  DEBUG_ASSERT ((bone_index >= 0) AND (bone_index < blendmask->skeleton->num_bones))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  Set_Chain (blendmask, bone_index, value);
}

/*____________________________________________________________________
|
| Function: Set_Chain
| 
| Output: Sets a chain of bone values in mask to value.
|___________________________________________________________________*/

static void Set_Chain (gx3dBlendMask *blendmask, int bone_index, float value)
{
  int i;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendmask)
  DEBUG_ASSERT (blendmask->skeleton)
  DEBUG_ASSERT (blendmask->skeleton->num_bones)
  DEBUG_ASSERT (blendmask->values)
  DEBUG_ASSERT ((value >= 0) AND (value <= 1))
  DEBUG_ASSERT ((bone_index >= 0) AND (bone_index < blendmask->skeleton->num_bones))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Set value at start bone in chain
  blendmask->values[bone_index] = value;
  // Set values at any children of this bone
  for (i=bone_index+1; i<blendmask->skeleton->num_bones; i++)
    // Is this bone a child?
    if (blendmask->skeleton->bones[i].parent == bone_index)
      Set_Chain (blendmask, i, value);
}
