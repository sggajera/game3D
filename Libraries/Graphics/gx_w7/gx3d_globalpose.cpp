/*____________________________________________________________________
|
| File: gx3d_globalpose.cpp
|
| Description: Functions to manipulate a gx3dGlobalPose.
|
| Functions:  gx3d_GlobalPose_Init
|             gx3d_GlobalPose_Free
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

/*____________________________________________________________________
|
| Function: gx3d_GlobalPose_Init
| 
| Output: Creates a global pose data structure based on skeleton.
|___________________________________________________________________*/

gx3dGlobalPose *gx3d_GlobalPose_Init (gx3dMotionSkeleton *skeleton)
{
  gx3dGlobalPose *pose = 0;
 
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skeleton)
  DEBUG_ASSERT (skeleton->num_bones)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Allocate memory for top-level data struct
  pose = (gx3dGlobalPose *) calloc (1, sizeof(gx3dGlobalPose));
  if (pose == 0)
    TERMINAL_ERROR ("gx3d_GlobalPose_Init(): can't allocate memory for pose struct");
  // Point to skeleton
  pose->skeleton = skeleton;
  // Allocate memory for bone poses
  pose->bone_pose = (gx3dGlobalBonePose *) calloc (skeleton->num_bones, sizeof(gx3dGlobalBonePose));
  if (pose->bone_pose == 0)
    TERMINAL_ERROR ("gx3d_LocalPose_Init(): can't allocate memory for bone pose array");

  return (pose);
}

/*____________________________________________________________________
|
| Function: gx3d_GlobalPose_Free
| 
| Output: Frees memory for global pose.
|___________________________________________________________________*/

void gx3d_GlobalPose_Free (gx3dGlobalPose *pose)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (pose)
  DEBUG_ASSERT (pose->bone_pose)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Free bone poses array
  free (pose->bone_pose);
  // Free top-level struct
  free (pose);
}
