/*____________________________________________________________________
|
| File: gx3d_blendnode.cpp
|
| Description: Functions to manipulate a gx3dBlendNode.
|
| Functions:  gx3d_BlendNode_Init
|             gx3d_BlendNode_Free
|             gx3d_BlendNode_Get_Input
|             gx3d_BlendNode_Set_Output
|             gx3d_BlendNode_Set_Output
|             gx3d_BlendNode_Set_BlendMask
|             gx3d_BlendNode_Set_BlendValue
|              Valid_Track
|             gx3d_BlendNode_Update
|              Update_Single
|              Update_Lerp2
|              Update_Lerp3
|              Update_Add
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

static bool Valid_Track (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track);
static void Update_Single (gx3dBlendNode *blendnode);
static void Update_Lerp2 (gx3dBlendNode *blendnode);
static void Update_Lerp3 (gx3dBlendNode *blendnode);
static void Update_Add (gx3dBlendNode *blendnode);

/*____________________________________________________________________
|
| Function: gx3d_BlendNode_Init
| 
| Output: Creates an empty blend node.  Returns pointer to blend node 
|   or 0 on any error.
|___________________________________________________________________*/

gx3dBlendNode *gx3d_BlendNode_Init (gx3dMotionSkeleton *skeleton, gx3dBlendNodeType type)
{
  int i;
  gx3dBlendNode *blendnode = 0;
 
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skeleton)
  DEBUG_ASSERT (skeleton->num_bones)
  DEBUG_ASSERT (skeleton->bones)
  DEBUG_ASSERT ((type == gx3d_BLENDNODE_TYPE_SINGLE) OR
                (type == gx3d_BLENDNODE_TYPE_LERP2)  OR
                (type == gx3d_BLENDNODE_TYPE_LERP3)  OR
                (type == gx3d_BLENDNODE_TYPE_ADD))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Allocate memory for top-level data struct
  blendnode = (gx3dBlendNode *) calloc (1, sizeof(gx3dBlendNode));
  if (blendnode == 0)
    TERMINAL_ERROR ("gx3d_BlendNode_Init(): can't allocate memory for blend node");
  // Set type
  blendnode->type = type;
  // Point to skeleton
  blendnode->skeleton = skeleton;
  // Set number of tracks (depends on type)
  switch (type) {
    case gx3d_BLENDNODE_TYPE_SINGLE: blendnode->num_tracks = 1; break;
    case gx3d_BLENDNODE_TYPE_LERP2:  blendnode->num_tracks = 2; break;
    case gx3d_BLENDNODE_TYPE_LERP3:  blendnode->num_tracks = 3; break;
    case gx3d_BLENDNODE_TYPE_ADD:    blendnode->num_tracks = 2; break;
  }
  // Allocate memory for local, composite poses
  for (i=0; i<blendnode->num_tracks; i++)
    blendnode->input_local_pose[i] = gx3d_LocalPose_Init (skeleton);

  return (blendnode);
}

/*____________________________________________________________________
|
| Function: gx3d_BlendNode_Free
| 
| Output: Frees memory for blend node.
|___________________________________________________________________*/

void gx3d_BlendNode_Free (gx3dBlendNode *blendnode)
{
  int i;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Free pose structs
  for (i=0; i<blendnode->num_tracks; i++)
    gx3d_LocalPose_Free (blendnode->input_local_pose[i]);

  // Free top-level struct
  free (blendnode);
}

/*____________________________________________________________________
|
| Function: gx3d_BlendNode_Get_Input
| 
| Output: Returns pointer to input local pose of blendnode at track. 
|   On any error returns 0.
|___________________________________________________________________*/

gx3dLocalPose *gx3d_BlendNode_Get_Input (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT ((track == gx3d_BLENDNODE_TRACK_0) OR
                (track == gx3d_BLENDNODE_TRACK_1) OR
                (track == gx3d_BLENDNODE_TRACK_2))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (Valid_Track (blendnode, track))
    return (blendnode->input_local_pose[track]);
  else {
    DEBUG_ERROR ("gx3d_BlendNode_Get_Input(): invalid track")
    return 0;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_BlendNode_Set_Output
| 
| Output: Sets output of blendnode.  To disable output call with 
|   pose = 0.
|___________________________________________________________________*/

void gx3d_BlendNode_Set_Output (gx3dBlendNode *blendnode, gx3dLocalPose *pose)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT (blendnode->skeleton)
  DEBUG_ASSERT (pose->skeleton)
  DEBUG_ASSERT (blendnode->skeleton == pose->skeleton)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Set output pose (or disable it)
  blendnode->output_local_pose = pose;
}

/*____________________________________________________________________
|
| Function: gx3d_BlendNode_Set_Output
| 
| Output: Sets output of src blendnode to dst blendnode.
|___________________________________________________________________*/

void gx3d_BlendNode_Set_Output (gx3dBlendNode *src_blendnode, gx3dBlendNode *dst_blendnode, gx3dBlendNodeTrack dst_track)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (src_blendnode)
  DEBUG_ASSERT (dst_blendnode)
  DEBUG_ASSERT (src_blendnode->skeleton)
  DEBUG_ASSERT (dst_blendnode->skeleton)
  DEBUG_ASSERT (src_blendnode->skeleton == dst_blendnode->skeleton)
  DEBUG_ASSERT ((dst_track == gx3d_BLENDNODE_TRACK_0) OR
                (dst_track == gx3d_BLENDNODE_TRACK_1) OR
                (dst_track == gx3d_BLENDNODE_TRACK_2))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/
  
  if (Valid_Track (dst_blendnode, dst_track))
    src_blendnode->output_local_pose = dst_blendnode->input_local_pose[dst_track]; 
  else
    DEBUG_ERROR ("gx3d_BlendNode_Set_Output(): invalid track")
}

/*____________________________________________________________________
|
| Function: gx3d_BlendNode_Set_BlendMask
| 
| Output: Sets blendmask in track of blendnode. To disable blend mask
|   call this function with blendmask=0.
|___________________________________________________________________*/

void gx3d_BlendNode_Set_BlendMask (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track, gx3dBlendMask *blendmask)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT ((track == gx3d_BLENDNODE_TRACK_0) OR
                (track == gx3d_BLENDNODE_TRACK_1) OR
                (track == gx3d_BLENDNODE_TRACK_2))
  DEBUG_ASSERT (blendmask)
  DEBUG_ASSERT (blendnode->skeleton == blendmask->skeleton)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (Valid_Track (blendnode, track))
    blendnode->blend_mask[track] = blendmask;
  else
    DEBUG_ERROR ("gx3d_BlendNode_Set_BlendMask(): invalid track")
}

/*____________________________________________________________________
|
| Function: gx3d_BlendNode_Set_BlendValue
| 
| Output: Sets blend value in track of blendnode.  Only track 0 and 1
|   can contain a blend value. 
|___________________________________________________________________*/

void gx3d_BlendNode_Set_BlendValue (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track, float value)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT ((track == gx3d_BLENDNODE_TRACK_0) OR
                (track == gx3d_BLENDNODE_TRACK_1))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (Valid_Track (blendnode, track))
    blendnode->blend_value[track] = value;
 else
    DEBUG_ERROR ("gx3d_BlendNode_Set_BlendValue(): invalid track")
}

/*____________________________________________________________________
|
| Function: Valid_Track
| 
| Input: Called from gx3d_BlendNode_Set_BlendMask(),
|                    gx3d_BlendNode_Set_BlendValue()
| Output: Returns true if track is valid for the type of node, else
|   false.
|___________________________________________________________________*/

static bool Valid_Track (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track)
{
  bool valid = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT ((track == gx3d_BLENDNODE_TRACK_0) OR
                (track == gx3d_BLENDNODE_TRACK_1) OR
                (track == gx3d_BLENDNODE_TRACK_2))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Is this a valid track that can be set?
  switch (blendnode->type) {
    case gx3d_BLENDNODE_TYPE_SINGLE:  if (track == gx3d_BLENDNODE_TRACK_0)
                                        valid = true;
                                      break;
    case gx3d_BLENDNODE_TYPE_LERP2:   
    case gx3d_BLENDNODE_TYPE_ADD:     if (track <= gx3d_BLENDNODE_TRACK_1)
                                        valid = true;
                                      break;
    case gx3d_BLENDNODE_TYPE_LERP3:   if (track <= gx3d_BLENDNODE_TRACK_2)
                                        valid = true;
                                      break;
  }
  // Print error message?
  if (NOT valid) {
    char str[200];
    sprintf (str, "Valid_Track(): invalid track [%d] for this node type [%d]", track, blendnode->type);
    DEBUG_ERROR (str);
  }

  return (valid);
}

/*____________________________________________________________________
|
| Function: gx3d_BlendNode_Update
| 
| Output: Updates composite local pose by sampling all tracks and
|   combining their outputs (local poses) as necessary.
|___________________________________________________________________*/

void gx3d_BlendNode_Update (gx3dBlendNode *blendnode)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT (blendnode->skeleton)
  DEBUG_ASSERT (blendnode->output_local_pose)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  switch (blendnode->type) {
    case gx3d_BLENDNODE_TYPE_SINGLE:  Update_Single (blendnode); break;
    case gx3d_BLENDNODE_TYPE_LERP2:   Update_Lerp2  (blendnode); break;
    case gx3d_BLENDNODE_TYPE_LERP3:   Update_Lerp3  (blendnode); break;
    case gx3d_BLENDNODE_TYPE_ADD:     Update_Add    (blendnode); break;
  }
}

/*____________________________________________________________________
|
| Function: Update_Single
| 
| Input: Called from gx3d_BlendNode_Update()
| Output: Sends output of track 0 to output_pose.
|___________________________________________________________________*/

static void Update_Single (gx3dBlendNode *blendnode)
{
  int i, n;
  gx3dLocalPose *in, *out;
  gx3dBlendMask *mask;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT (blendnode->skeleton)
  DEBUG_ASSERT (blendnode->num_tracks == 1)
  DEBUG_ASSERT (blendnode->input_local_pose)
  DEBUG_ASSERT (blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_0])

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Init variables
  n    = blendnode->skeleton->num_bones;
  in   = blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_0];
  out  = blendnode->output_local_pose;
  mask = blendnode->blend_mask[gx3d_BLENDNODE_TRACK_0];

  // Output local pose unchanged?
  if (mask == 0) {
    // Output root bone translate
    out->root_translate = in->root_translate;
    // Output bone rotations                
    memcpy ((void *)(out->bone_pose), (void *)(in->bone_pose), n * sizeof(gx3dLocalBonePose));
  }
  // Output local pose modified by blend mask?
  else {
    // Output root bone translate
    gx3d_MultiplyScalarVector (mask->values[0], &(in->root_translate), &(out->root_translate));
    // Output bone rotations
    for (i=0; i<n; i++) {
      gx3d_ScaleQuaternion (&(in->bone_pose[i].q), mask->values[i], &(out->bone_pose[i].q));
      gx3d_NormalizeQuaternion (&(out->bone_pose[i].q));
    }
  }
}

/*____________________________________________________________________
|
| Function: Update_Lerp2
| 
| Input: Called from gx3d_BlendNode_Update()
| Output: Sends output to output_pose of:
|
|   track0 * blendvalue0 
|     LERP 
|   track1 * (1 - blendvalue0)
|___________________________________________________________________*/

static void Update_Lerp2 (gx3dBlendNode *blendnode)
{
  int i, n;
  float blend_value0;
  gx3dVector v0, v1;
  gx3dQuaternion q0, q1;
  gx3dLocalPose *in0, *in1, *out;
  gx3dBlendMask *mask0, *mask1;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT (blendnode->skeleton)
  DEBUG_ASSERT (blendnode->num_tracks == 2)
  DEBUG_ASSERT (blendnode->input_local_pose)
  DEBUG_ASSERT (blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_0])
  DEBUG_ASSERT (blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_1])

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Init variables
  n            = blendnode->skeleton->num_bones;
  in0          = blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_0];
  in1          = blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_1];
  out          = blendnode->output_local_pose;
  mask0        = blendnode->blend_mask[gx3d_BLENDNODE_TRACK_0];
  mask1        = blendnode->blend_mask[gx3d_BLENDNODE_TRACK_1];
  blend_value0 = blendnode->blend_value[0];

  // Process root translate
  if (mask0) 
    gx3d_MultiplyScalarVector (mask0->values[0], &(in0->root_translate), &v0);
  else
    v0 = in0->root_translate;
  if (mask1) 
    gx3d_MultiplyScalarVector (mask1->values[0], &(in1->root_translate), &v1);
  else
    v1 = in1->root_translate;
  gx3d_LerpVector (&v0, &v1, blend_value0, &(out->root_translate));

  // Go through all bones
  for (i=0; i<n; i++) {
    // Adjust q0 by blend mask?
    if (mask0) {
      gx3d_ScaleQuaternion (&(in0->bone_pose[i].q), mask0->values[i], &q0);
      gx3d_NormalizeQuaternion (&q0);
    }
    else
      q0 = in0->bone_pose[i].q;
    // Adjust q1 by blend mask?
    if (mask1) {
      gx3d_ScaleQuaternion (&(in1->bone_pose[i].q), mask1->values[i], &q1);
      gx3d_NormalizeQuaternion (&q1);
    }
    else
      q1 = in1->bone_pose[i].q;
    // Lerp them together
    gx3d_GetLerpQuaternion (&q0, &q1, blend_value0, &(out->bone_pose[i].q));
//////////////////// IS THIS NECESSARY?????
    gx3d_NormalizeQuaternion (&(out->bone_pose[i].q));
//////////////////////////////////////////
  }
}

/*____________________________________________________________________
|
| Function: Update_Lerp3
| 
| Input: Called from gx3d_BlendNode_Update()
| Output: Sends output to output_pose of:
|
|   track0 * blendvalue0 
|     LERP 
|   track1 * blendvalue1 
|     LERP 
|   track2 * (1 - blendvalue0 - blendvalue1)
|
| Note: blendvalue0 + blendvalue1 should be <= 1.  This is the callers
|   responsibility.
|___________________________________________________________________*/

static void Update_Lerp3 (gx3dBlendNode *blendnode)
{
  int i, n;
  float blend_value0, blend_value1;
  gx3dVector v0, v1, v2, vt;
  gx3dQuaternion q0, q1, q2, qt;
  gx3dLocalPose *in0, *in1, *in2, *out;
  gx3dBlendMask *mask0, *mask1, *mask2;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT (blendnode->skeleton)
  DEBUG_ASSERT (blendnode->num_tracks == 3)
  DEBUG_ASSERT (blendnode->input_local_pose)
  DEBUG_ASSERT (blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_0])
  DEBUG_ASSERT (blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_1])
  DEBUG_ASSERT (blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_2])

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Init variables
  n            = blendnode->skeleton->num_bones;
  in0          = blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_0];
  in1          = blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_1];
  in2          = blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_2];
  out          = blendnode->output_local_pose;
  mask0        = blendnode->blend_mask[gx3d_BLENDNODE_TRACK_0];
  mask1        = blendnode->blend_mask[gx3d_BLENDNODE_TRACK_1];
  mask2        = blendnode->blend_mask[gx3d_BLENDNODE_TRACK_2];
  blend_value0 = blendnode->blend_value[0];
  blend_value1 = blendnode->blend_value[1];

  // Process root translate
  if (mask0 == 0) 
    v0 = in0->root_translate;
  else
    gx3d_MultiplyScalarVector (mask0->values[0], &(in0->root_translate), &v0);
  if (mask1 == 0) 
    v1 = in1->root_translate;
  else
    gx3d_MultiplyScalarVector (mask1->values[0], &(in1->root_translate), &v1);
  if (mask2 == 0) 
    v2 = in2->root_translate;
  else
    gx3d_MultiplyScalarVector (mask2->values[0], &(in2->root_translate), &v2);
  gx3d_LerpVector (&v0, &v1, blend_value0, &vt);
  gx3d_LerpVector (&vt, &v2, blend_value1, &(out->root_translate));

  // Go through all bones
  for (i=0; i<n; i++) {
    // Adjust q0 by blend mask?
    if (mask0) {
      gx3d_ScaleQuaternion (&(in0->bone_pose[i].q), mask0->values[i], &q0);
      gx3d_NormalizeQuaternion (&q0);
    }
    else
      q0 = in0->bone_pose[i].q;
    // Adjust q1 by blend mask?
    if (mask1) {  
      gx3d_ScaleQuaternion (&(in1->bone_pose[i].q), mask1->values[i], &q1);
      gx3d_NormalizeQuaternion (&q1);
    }
    else
      q1 = in1->bone_pose[i].q;
    // Adjust q1 by blend mask?
    if (mask2) {
      gx3d_ScaleQuaternion (&(in2->bone_pose[i].q), mask1->values[i], &q2);
      gx3d_NormalizeQuaternion (&q2);
    }
    else
      q2 = in2->bone_pose[i].q;
    // Lerp them together
    gx3d_GetLerpQuaternion (&q0, &q1, blend_value0, &qt);
//////////////////// IS THIS NECESSARY?????
    gx3d_NormalizeQuaternion (&qt);
//////////////////////////////////////////
    gx3d_GetLerpQuaternion (&qt, &q2, blend_value1, &(out->bone_pose[i].q));
//////////////////// IS THIS NECESSARY?????
    gx3d_NormalizeQuaternion (&(out->bone_pose[i].q));
//////////////////////////////////////////
  }
}

/*____________________________________________________________________
|
| Function: Update_Add
| 
| Input: Called from gx3d_BlendNode_Update()
| Output: Sends output to output_pose of:
|
|   track0
|     ADD 
|   track1 * (blendvalue0)
|
| Note: track0 is the reference pose.  track1 is the additive pose.
|___________________________________________________________________*/

static void Update_Add (gx3dBlendNode *blendnode)
{
  int i, n;
  float blend_value0;
  gx3dVector v0, v1;
  gx3dQuaternion q0, q1;
  gx3dLocalPose *in0, *in1, *out;
  gx3dBlendMask *mask0, *mask1;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT (blendnode->skeleton)
  DEBUG_ASSERT (blendnode->num_tracks == 2)
  DEBUG_ASSERT (blendnode->input_local_pose)
  DEBUG_ASSERT (blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_0])
  DEBUG_ASSERT (blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_1])

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Init variables
  n            = blendnode->skeleton->num_bones;
  in0          = blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_0];
  in1          = blendnode->input_local_pose[gx3d_BLENDNODE_TRACK_1];
  out          = blendnode->output_local_pose;
  mask0        = blendnode->blend_mask[gx3d_BLENDNODE_TRACK_0];
  mask1        = blendnode->blend_mask[gx3d_BLENDNODE_TRACK_1];
  blend_value0 = blendnode->blend_value[0];

  // Process root translate
  if (mask0) 
    gx3d_MultiplyScalarVector (mask0->values[0], &(in0->root_translate), &v0);
  else
    v0 = in0->root_translate;
  if (mask1) 
    gx3d_MultiplyScalarVector (mask1->values[0], &(in1->root_translate), &v1);
  else
    v1 = in1->root_translate;
  // Scale the additive root translate
  gx3d_MultiplyScalarVector (blend_value0, &v1, &v1);
  // Add them together
  gx3d_AddVector (&v0, &v1, &(out->root_translate));                  

///////////////// DEBUGGING ///////////////
//out->root_translate = v0;
///////////////////////////////////////////

  // Go through all bones
  for (i=0; i<n; i++) {
    // Adjust q0 by blend mask?
    if (mask0) {
      if (mask0->values[i])
        gx3d_ScaleQuaternion (&(in0->bone_pose[i].q), mask0->values[i], &q0);
      else
        gx3d_GetIdentityQuaternion (&q0);
    }
    else
      q0 = in0->bone_pose[i].q;
    // Adjust q1 by blend mask?
    if (mask1) {
      if (mask1->values[i])
        gx3d_ScaleQuaternion (&(in1->bone_pose[i].q), mask1->values[i], &q1);
      else
        gx3d_GetIdentityQuaternion (&q1);
    }
    else
      q1 = in1->bone_pose[i].q;
    // Scale the additive pose by the blend factor (if factor is nonzero)
    if (blend_value0) {
      gx3d_ScaleQuaternion (&q1, blend_value0, &q1);
//    gx3d_NormalizeQuaternion (&q1); // Don't do this, probably messes things up
      // Add them together
      gx3d_MultiplyQuaternion (&q1, &q0, &(out->bone_pose[i].q));  // quaternions have to be multiplied in reverse order
    }
    else
      out->bone_pose[i].q = q0; 
    //////////////////// IS THIS NECESSARY?????
//    gx3d_NormalizeQuaternion (&(out->bone_pose[i].q));
    //////////////////////////////////////////
  }
}
