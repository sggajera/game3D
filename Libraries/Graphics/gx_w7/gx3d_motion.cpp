/*____________________________________________________________________
|
| File: gx3d_motion.cpp
|
| Description: Functions for keyframe animation (motions).
|
| Functions:  gx3d_Motion_Init
|             gx3d_Motion_Read_LWS_File
|              Verify_Motion_Skeleton
|             gx3d_Motion_Read_GX3DANI_File
|              Read_GX3DANI_File
|             gx3d_Motion_Copy
|              Copy_Name
|             gx3d_Motion_Compute_Difference
|              Compute_Difference_Position
|              Compute_Difference_Rotation
|             gx3d_Motion_Free
|              Free_Bones
|              Free_Metadata
|             gx3d_Motion_Free_All
|             gx3d_Motion_Set_Output
|             gx3d_Motion_Update
|              Animate_Bones
|             gx3d_Motion_Write_GX3DANI_File
|             gx3d_Motion_GetMetadata
|             gx3d_MotionMetadata_GetSample
|             gx3d_MotionMetadata_Copy
|             gx3d_Motion_Print
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
#include "gx3d_lws.h"
#include "quantize.h"

/*___________________
|
| Constants
|__________________*/

#define ADD_TO_MOTIONLIST(_mtn_)    \
  {                                 \
    if (motionlist == 0)            \
      motionlist = _mtn_;           \
    else {                          \
      _mtn_->next = motionlist;     \
      motionlist->previous = _mtn_;	\
      motionlist = _mtn_;           \
    }                               \
  }

#define REMOVE_FROM_MOTIONLIST(_mtn_)           \
  {                                             \
    if (_mtn_->previous)                        \
      _mtn_->previous->next = _mtn_->next;      \
    else                                        \
      motionlist = _mtn_->next;                 \
    if (_mtn_->next)                            \
      _mtn_->next->previous = _mtn_->previous;	\
  }

#define COMPRESS_QUATERNION(_q_,_cq_)       \
{                                           \
  _cq_.x = CompressQuaternionValue(_q_.x);  \
  _cq_.y = CompressQuaternionValue(_q_.y);  \
  _cq_.z = CompressQuaternionValue(_q_.z);  \
  _cq_.w = CompressQuaternionValue(_q_.w);  \
}

#define DECOMPRESS_QUATERNION(_cq_,_q_)       \
{                                             \
  _q_.x = DecompressQuaternionValue(_cq_.x);  \
  _q_.y = DecompressQuaternionValue(_cq_.y);  \
  _q_.z = DecompressQuaternionValue(_cq_.z);  \
  _q_.w = DecompressQuaternionValue(_cq_.w);  \
}

static float ONE_OVER_THOUSAND = 1.0f / 1000.0f;

/*___________________
|
| Function prototypes
|__________________*/

static bool Verify_Motion_Skeleton (gx3dMotion *motion);
static void Read_GX3DANI_File (gx3dMotion *motion, char *filename);
static void Copy_Name (char *dst, char *src, int maxlength);
static void Compute_Difference_Position (gx3dVector **src_keys, int *src_nkeys, gx3dVector *ref_keys, int ref_nkeys);
static void Compute_Difference_Rotation (gx3dCompressedQuaternion **src_keys, int *src_nkeys, gx3dCompressedQuaternion *ref_keys, int ref_nkeys);
static void Free_Bones (gx3dMotion *motion);
static void Free_Metadata (gx3dMotion *motion);
static void Animate_Bones (gx3dMotion *motion, unsigned elapsed_time);

/*___________________
|
| Global variables
|__________________*/

static struct {
  unsigned channel_id;
  int      channel_index;
  char    *channel_string;
} channel_info[gx3dMotionMetadata_MAX_CHANNELS] = {
  { gx3dMotionMetadataChannel_POS_X, gx3dMotionMetadataChannelIndex_POS_X, "POS_X" },
  { gx3dMotionMetadataChannel_POS_Y, gx3dMotionMetadataChannelIndex_POS_Y, "POS_Y" },
  { gx3dMotionMetadataChannel_POS_Z, gx3dMotionMetadataChannelIndex_POS_Z, "POS_Z" },
  { gx3dMotionMetadataChannel_ROT_X, gx3dMotionMetadataChannelIndex_ROT_X, "ROT_X" },
  { gx3dMotionMetadataChannel_ROT_Y, gx3dMotionMetadataChannelIndex_ROT_Y, "ROT_Y" },
  { gx3dMotionMetadataChannel_ROT_Z, gx3dMotionMetadataChannelIndex_ROT_Z, "ROT_Z" }
};

static gx3dMotion *motionlist = 0;	// doubly linked list of motions

/*____________________________________________________________________
|
| Function: gx3d_Motion_Init
| 
| Output: Creates an empty motion.  Returns pointer or 0 on any error.
|___________________________________________________________________*/

gx3dMotion *gx3d_Motion_Init (gx3dMotionSkeleton *skeleton)
{
  gx3dMotion *motion = 0;
 
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skeleton)
  DEBUG_ASSERT (skeleton->num_bones)
  DEBUG_ASSERT (skeleton->bones)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Allocate memory for top-level data struct
  motion = (gx3dMotion *) calloc (1, sizeof(gx3dMotion));
  if (motion == 0)
    TERMINAL_ERROR ("gx3d_Motion_Init(): can't allocate memory for motion");
  // Point to skeleton
  motion->skeleton = skeleton;
  // Add to motionlist
  ADD_TO_MOTIONLIST (motion);

  return (motion);
}

/*____________________________________________________________________
|
| Function: gx3d_Motion_Read_LWS_File
| 
| Input: fps                    = desired fps or 0=fps in file
|        metadata_requested     = 0=none requested
|        num_metadata_requested = 0=none requested
|        load_all_metadata      = if true then ignore 2 previous parameters
| Output: Creates motion from a LWS file. Returns pointer to motion or
|   0 on any error.
|___________________________________________________________________*/

gx3dMotion *gx3d_Motion_Read_LWS_File (gx3dMotionSkeleton *skeleton, char *filename, int fps, gx3dMotionMetadataRequest *metadata_requested, int num_metadata_requested, bool load_all_metadata)
{
  gx3dMotion *motion = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skeleton)
  DEBUG_ASSERT (filename)
  DEBUG_ASSERT (fps >= 0)
  DEBUG_ASSERT ((metadata_requested AND num_metadata_requested >= 1) OR (metadata_requested == 0 AND num_metadata_requested == 0))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create a new empty gx3d motion
  motion = gx3d_Motion_Init (skeleton);
  if (motion) {
    // Convert the lws file to a gx3d motion
    LWS_File_To_GX3D_Motion (filename, motion, fps, metadata_requested, num_metadata_requested, load_all_metadata);
    // Make sure skeleton and motion are compatible - same bones and structure
    if (NOT Verify_Motion_Skeleton (motion))
      TERMINAL_ERROR ("gx3d_Motion_Read_LWS_File(): LWS file skeleton not compatible with requested skeleton");
  }

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (motion)

  return (motion);
}

/*____________________________________________________________________
|
| Function: Verify_Motion_Skeleton
| 
| Input: Called from: gx3d_Motion_Read_LWS_File(), 
|                     gx3d_Motion_Read_GX3DANI_File()
| Output: Returns true if motion->skeleton and data in motion have 
|   exactly the same skeleton structure
|___________________________________________________________________*/

static bool Verify_Motion_Skeleton (gx3dMotion *motion)
{
  int i, j, n, i_parent, j_parent;
  bool found;
  bool verified = true; // assume ok

  DEBUG_ASSERT (motion)
  DEBUG_ASSERT (motion->skeleton)

  // Number of bones must be the same
  if (motion->skeleton->num_bones != motion->num_bones) {
    verified = false;
    DEBUG_ASSERT (verified)
  }
  else {
    // Look at each skeleton bone   
    for (i=0, n=motion->num_bones; (i<n) AND verified; i++) {
      // Get index to parent bone
      i_parent = motion->skeleton->bones[i].parent;
      // Look for corresponding bone name in motion
      for (j=0, found=false; (j<n) AND (NOT found); j++) {
        // Same bone names?
        if (!strcmp(motion->skeleton->bones[i].name, motion->bones[j].name)) {
          // Get index to parent bone
          j_parent = motion->bones[j].parent;   
          // Are they both the root bone?
          if ((i_parent == 0xFF) AND (j_parent == 0xFF))
            found = true;
          // Do they both have the same parent bone name?
          else if (!strcmp(motion->skeleton->bones[i_parent].name, motion->bones[j_parent].name))
            found = true;
        }
      }
      if (NOT found) {
        verified = false;
        DEBUG_ASSERT (verified)
      }
    }
  }

  return (verified);
}

/*____________________________________________________________________
|
| Function: gx3d_Motion_Read_GX3DANI_File
| 
| Output: Creates motion from a GX3DANI file. Returns pointer to motion or
|   0 on any error.
|___________________________________________________________________*/

gx3dMotion *gx3d_Motion_Read_GX3DANI_File (gx3dMotionSkeleton *skeleton, char *filename)
{
  gx3dMotion *motion = 0;
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skeleton)
  DEBUG_ASSERT (filename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create a new empty gx3d motion
  motion = gx3d_Motion_Init (skeleton);
  if (motion) {
    // Convert the gx3dani file to a gx3d motion
    Read_GX3DANI_File (motion, filename);
    // Make sure skeleton and motion are compatible - same bones and structure
    if (NOT Verify_Motion_Skeleton (motion))
      TERMINAL_ERROR ("gx3d_Motion_Read_GX3DANI_File(): LWS file skeleton not compatible with requested skeleton");
  }

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (motion)

  return (motion);
}

/*____________________________________________________________________
|
| Function: Read_GX3DANI_File
| 
| Input: Called from gx3d_Motion_Read_GX3DANI_File()
| Output: Reads in GX3DANI file into motion.
|___________________________________________________________________*/

static void Read_GX3DANI_File (gx3dMotion *motion, char *filename)
{
  int i, j, n;
  FILE *fp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (motion)
  DEBUG_ASSERT (filename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Open input file
  fp = fopen (filename, "rb");
  if (fp == 0)
    DEBUG_ERROR ("Read_GX3DANI_File(): can't open input file")
  else {
    // Read name
    fread (motion->name, sizeof(char), gx_ASCIIZ_STRING_LENGTH_LONG, fp);
    // Read position
    fread (&(motion->position), sizeof(gx3dVector), 1, fp);
    // Read rotation
    fread (&(motion->rotation), sizeof(gx3dVector), 1, fp);
    // Read keys_per_second
    fread (&(motion->keys_per_second), sizeof(int), 1, fp);
    // Read max_nkeys
    fread (&(motion->max_nkeys), sizeof(int), 1, fp);
    // Read duration
    fread (&(motion->duration), sizeof(unsigned), 1, fp);
    // Read num_bones
    fread (&(motion->num_bones), sizeof(int), 1, fp);
    DEBUG_ASSERT (motion->num_bones);
    // Read num_metadata
    fread (&(motion->num_metadata), sizeof(int), 1, fp);

    // Allocate memory for bones array
    motion->bones = (gx3dMotionBone *) calloc (motion->num_bones, sizeof(gx3dMotionBone));
    if (motion->bones == 0)
      TERMINAL_ERROR ("Read_GX3DANI_File(): can't allocate memory for bones array");
    // Read bones
    for (i=0; i<motion->num_bones; i++) {
      // Read name
      fread (motion->bones[i].name, sizeof(char), gx_ASCIIZ_STRING_LENGTH_LONG, fp);
      // Read weightmap_name
      fread (motion->bones[i].weightmap_name, sizeof(char), gx_ASCIIZ_STRING_LENGTH_LONG, fp);
      // Read position
      fread (&(motion->bones[i].pivot), sizeof(gx3dVector), 1, fp);
      // Read qrotation
      fread (&(motion->bones[i].qrotation), sizeof(gx3dQuaternion), 1, fp);
      // Read active
      fread (&(motion->bones[i].active), sizeof(bool), 1, fp);
      // Read nkeys
      fread (&(motion->bones[i].nkeys), sizeof(int), 1, fp);
      // Read parent
      fread (&(motion->bones[i].parent), sizeof(unsigned char), 1, fp);
      // Read pos_keys?
      if (motion->bones[i].parent == 0xFF) {
        DEBUG_ASSERT (motion->bones[i].nkeys);
        // Allocate memory for pos_key array
        motion->bones[i].pos_key = (gx3dVector *) calloc (motion->bones[i].nkeys, sizeof(gx3dVector));
        if (motion->bones[i].pos_key == 0)
          TERMINAL_ERROR ("Read_GX3DANI_File(): can't allocate memory for pos_key array");
        // Read pos_keys
        fread (motion->bones[i].pos_key, sizeof(gx3dVector), motion->bones[i].nkeys, fp);
      }
      // Read rot_keys?
      if (motion->bones[i].active) {
        DEBUG_ASSERT (motion->bones[i].nkeys);
        // Allocate memory for rot_key array
        motion->bones[i].rot_key = (gx3dCompressedQuaternion *) calloc (motion->bones[i].nkeys, sizeof(gx3dCompressedQuaternion));
        if (motion->bones[i].rot_key == 0)
          TERMINAL_ERROR ("Read_GX3DANI_File(): can't allocate memory for rot_key array");
        // Read rot_keys
        fread (motion->bones[i].rot_key, sizeof(gx3dCompressedQuaternion), motion->bones[i].nkeys, fp);
      }
    }

    // Read metadata, if any
    if (motion->num_metadata) {
      // Allocate memory for metadata
      motion->metadata = (gx3dMotionMetadata *) calloc (motion->num_bones, sizeof(gx3dMotionMetadata));
      if (motion->metadata == 0)
        TERMINAL_ERROR ("Read_GX3DANI_File(): can't allocate memory for metadata array");
      // Read metadata
      for (i=0; i<motion->num_metadata; i++) {
        // Read name
        fread (motion->metadata[i].name, sizeof(char), gx_ASCIIZ_STRING_LENGTH_LONG, fp);
        // Read channels_present
        fread (&(motion->metadata[i].channels_present), sizeof(unsigned), 1, fp);
        // Read duration
        fread (&(motion->metadata[i].duration), sizeof(unsigned), 1, fp);
        // Read channels
        for (j=0; j<gx3dMotionMetadata_MAX_CHANNELS; j++) {
          n = channel_info[j].channel_index;
          if (motion->metadata[i].channels_present & channel_info[j].channel_id) {
            fread (&(motion->metadata[i].channel[n].nkeys), sizeof(int), 1, fp);
            DEBUG_ASSERT (motion->metadata[i].channel[n].nkeys);
            // Allocate memory for keys
            motion->metadata[i].channel[n].keys = (gx3dMotionMetadataKey *) calloc (motion->metadata[i].channel[n].nkeys, sizeof(gx3dMotionMetadataKey));
            DEBUG_ASSERT (motion->metadata[i].channel[n].keys);
            // Read keys
            fread (motion->metadata[i].channel[n].keys, sizeof(gx3dMotionMetadataKey), motion->metadata[i].channel[n].nkeys, fp);
          }
        }
      }   
    }

    // Close input file
    fclose (fp);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Motion_Copy
| 
| Output: Performs a deep copy.  Returns pointer to new motion or 0 on
|   any error.
|___________________________________________________________________*/

gx3dMotion *gx3d_Motion_Copy (gx3dMotion *motion)
{
  int i;
  gx3dMotion *new_motion = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (motion);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create new empty motion
  new_motion = gx3d_Motion_Init (motion->skeleton);
  // Copy data
  new_motion->output_local_pose = motion->output_local_pose;
  Copy_Name (new_motion->name, motion->name, gx_ASCIIZ_STRING_LENGTH_LONG);
  new_motion->position        = motion->position;
  new_motion->rotation        = motion->rotation;
  new_motion->keys_per_second = motion->keys_per_second;
  new_motion->max_nkeys       = motion->max_nkeys;
  new_motion->duration        = motion->duration;
  new_motion->num_bones       = motion->num_bones;
  if (motion->bones) {
    // Create new array of bones
    new_motion->bones = (gx3dMotionBone *) malloc (motion->num_bones * sizeof(gx3dMotionBone));
    if (new_motion->bones == 0)
      TERMINAL_ERROR ("gx3d_Motion_Copy(): Can't allocate memory for bones array");
    // Copy data for each bone
    for (i=0; i<motion->num_bones; i++) {
      // Peform shallow copy first
      new_motion->bones[i] = motion->bones[i];
      // Create array of pos_key?
      if (motion->bones[i].pos_key) {
        new_motion->bones[i].pos_key = (gx3dVector *) malloc (motion->bones[i].nkeys * sizeof(gx3dVector));
        if (new_motion->bones[i].pos_key == 0)
          TERMINAL_ERROR ("gx3d_Motion_Copy(): Can't allocate memory for pos_key array");
        memcpy ((void *)(new_motion->bones[i].pos_key), (void *)(motion->bones[i].pos_key), motion->bones[i].nkeys * sizeof(gx3dVector));
      }
      // Create array of rot_key?
      if (motion->bones[i].rot_key) {
        new_motion->bones[i].rot_key = (gx3dCompressedQuaternion *) malloc (motion->bones[i].nkeys * sizeof(gx3dCompressedQuaternion));
        if (new_motion->bones[i].rot_key == 0)
          TERMINAL_ERROR ("gx3d_Motion_Copy(): Can't allocate memory for rot_key array");
        memcpy ((void *)(new_motion->bones[i].rot_key), (void *)(motion->bones[i].rot_key), motion->bones[i].nkeys * sizeof(gx3dCompressedQuaternion));
      }
    }
  }
  new_motion->num_metadata = motion->num_metadata;
  // Copy metadata, if any
  if (motion->metadata)
    new_motion->metadata = gx3d_MotionMetadata_Copy (motion->metadata);

  return (new_motion);
}

/*____________________________________________________________________
|
| Function: Copy_Name
| 
| Input: Called gx3d_Motion_Copy()
| Output: Copies string into dst, checking for overflow.
|___________________________________________________________________*/

static void Copy_Name (char *dst, char *src, int maxlength)
{
  DEBUG_ASSERT (dst)
  DEBUG_ASSERT (src)

  strncpy (dst, src, maxlength);
  dst[maxlength-1] = 0;  // append null in case source string was longer than gx_ASCIIZ_STRING_LENGTH_LONG  
  // If source name was too long, generate a warning
  if ((int)(strlen(src)) > maxlength-1) {
    char str[200];
    sprintf (str, "Copy_Name(): name [%s] too long, cannot exceed %d characters", src, maxlength-1);
    DEBUG_ERROR (str);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Motion_Compute_Difference
| 
| Output: Creates a difference motion for use in additive blending.  
|   Returns pointer to the new difference motion or 0 on any error.  
|
|   Both motions must have the same duration.  Computes:
|     D = S - R (changes to R that exhibit attributes of S)
|   R should be a normal animation (walking, running, etc.)
|   S is a special version of R (tired, angry, etc.)
|   Can generate animations partway between S and R by adding a
|    percentage of D to R.
|
|   Reference: Game Engine Architecture, pg. 538.
|___________________________________________________________________*/

gx3dMotion *gx3d_Motion_Compute_Difference (gx3dMotion *reference_motion, gx3dMotion *source_motion)
{
  int i;
  gx3dMotion *diff_motion = 0;
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (reference_motion);
  DEBUG_ASSERT (source_motion);
  DEBUG_ASSERT (reference_motion->skeleton        == source_motion->skeleton)
  DEBUG_ASSERT (reference_motion->keys_per_second == source_motion->keys_per_second)
  DEBUG_ASSERT (reference_motion->duration        == source_motion->duration)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Copy source motion
  diff_motion = gx3d_Motion_Copy (source_motion);
  // Subtract reference motion - to get the difference between the reference and source motions
  for (i=0; i<diff_motion->num_bones; i++) {
    if (diff_motion->bones[i].pos_key)
      Compute_Difference_Position (&(diff_motion->bones[i].pos_key), &(diff_motion->bones[i].nkeys), reference_motion->bones[i].pos_key, reference_motion->bones[i].nkeys);
    if (diff_motion->bones[i].rot_key)
      Compute_Difference_Rotation (&(diff_motion->bones[i].rot_key), &(diff_motion->bones[i].nkeys), reference_motion->bones[i].rot_key, reference_motion->bones[i].nkeys);
    else // non-active bone
      gx3d_GetIdentityQuaternion (&(diff_motion->bones[i].qrotation));
  }

  return (diff_motion);
}

/*____________________________________________________________________
|
| Function: Compute_Difference_Position
| 
| Input: Called from gx3d_Motion_Compute_Difference() 
| Output: Subtracts src keys from ref keys, storing restuls in ref keys.
|   May reallocate size of ref keys if needed.
|___________________________________________________________________*/

static void Compute_Difference_Position (gx3dVector **src_keys, int *src_nkeys, gx3dVector *ref_keys, int ref_nkeys)
{
  int i, n;
  gx3dVector *vref, *vsrc, *diff_keys;   

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (src_keys)
  DEBUG_ASSERT (*src_keys)
  DEBUG_ASSERT (*src_nkeys >= 1)
  DEBUG_ASSERT (ref_keys)
  DEBUG_ASSERT (ref_nkeys >= 1)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute max keys to compute
  n = *src_nkeys;
  if (ref_nkeys > n)
    n = ref_nkeys;
  // Allocate memory for an array of keys
  diff_keys = (gx3dVector *) malloc (n * sizeof(gx3dVector));
  if (diff_keys == 0)
    TERMINAL_ERROR ("Compute_Difference_Position(): can't allocate array of keys")
  // Compute keys
  vsrc = *src_keys;
  vref =  ref_keys;
  for (i=0; i<n; i++) {
    gx3d_SubtractVector (vsrc, vref, &diff_keys[i]);
    // Go to next src key, if any
    if (i < *src_nkeys-1)
      vsrc++;
    // Go to next ref key, if any
    if (i < ref_nkeys-1)
      vref++;
  }
  // Free memory for src keys array
  free (*src_keys);
  // Connect new array
  *src_keys = diff_keys;
  // Set size of new array
  *src_nkeys = n;
}

/*____________________________________________________________________
|
| Function: Compute_Difference_Rotation
| 
| Input: Called from gx3d_Motion_Compute_Difference() 
| Output: Subtracts src keys from ref keys, storing restuls in ref keys.
|   May reallocate size of ref keys if needed.
|___________________________________________________________________*/

static void Compute_Difference_Rotation (gx3dCompressedQuaternion **src_keys, int *src_nkeys, gx3dCompressedQuaternion *ref_keys, int ref_nkeys)
{
  int i, n;
  gx3dQuaternion qref, qsrc, qdiff;
  gx3dCompressedQuaternion *cqref, *cqsrc, *diff_keys;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (src_keys)
  DEBUG_ASSERT (*src_keys)
  DEBUG_ASSERT (*src_nkeys >= 1)
  DEBUG_ASSERT (ref_keys)
  DEBUG_ASSERT (ref_nkeys >= 1)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute max keys to compute
  n = *src_nkeys;
  if (ref_nkeys > n)
    n = ref_nkeys;
   // Allocate memory for an array of keys
  diff_keys = (gx3dCompressedQuaternion *) malloc (n * sizeof(gx3dCompressedQuaternion));
  if (diff_keys == 0)
    TERMINAL_ERROR ("Compute_Difference_Rotation(): can't allocate array of keys")
  // Compute keys
  cqsrc = *src_keys;
  cqref =  ref_keys;
  for (i=0; i<n; i++) {
    // Decompress keys
    qsrc.x = DecompressQuaternionValue (cqsrc->x);
    qsrc.y = DecompressQuaternionValue (cqsrc->y);
    qsrc.z = DecompressQuaternionValue (cqsrc->z);
    qsrc.w = DecompressQuaternionValue (cqsrc->w);
    qref.x = DecompressQuaternionValue (cqref->x);
    qref.y = DecompressQuaternionValue (cqref->y);
    qref.z = DecompressQuaternionValue (cqref->z);
    qref.w = DecompressQuaternionValue (cqref->w);
    // Subtract if not the same
    if (memcmp((void *)&cqsrc, (void *)&cqref, sizeof(gx3dCompressedQuaternion))) {
      gx3d_SubtractQuaternion (&qsrc, &qref, &qdiff);
//      gx3d_NormalizeQuaternion (&qdiff);
    }
    else
      gx3d_GetIdentityQuaternion (&qdiff);
    // Make sure qdiff elements are between -1 and 1
    qdiff.x = gx3d_Clamp (qdiff.x, -1, 1);
    qdiff.y = gx3d_Clamp (qdiff.y, -1, 1);
    qdiff.z = gx3d_Clamp (qdiff.z, -1, 1);
    qdiff.w = gx3d_Clamp (qdiff.w, -1, 1);
    // Compress diff key              
    diff_keys[i].x = CompressQuaternionValue (qdiff.x);
    diff_keys[i].y = CompressQuaternionValue (qdiff.y);
    diff_keys[i].z = CompressQuaternionValue (qdiff.z);
    diff_keys[i].w = CompressQuaternionValue (qdiff.w);
    // Go to next src key, if any
    if (i < *src_nkeys-1)
      cqsrc++;
    // Go to next ref key, if any       
    if (i < ref_nkeys-1)
      cqref++;
  }
  // Free memory for src keys array
  free (*src_keys);
  // Connect new array
  *src_keys = diff_keys;
  // Set size of new array
  *src_nkeys = n;
}

/*____________________________________________________________________
|
| Function: gx3d_Motion_Free
| 
| Output: Frees memory for a motion.
|___________________________________________________________________*/

void gx3d_Motion_Free (gx3dMotion *motion)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (motion)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

	// Remove it from the list of motions
  REMOVE_FROM_MOTIONLIST (motion)
  // Free all bones
  Free_Bones (motion);
  // Free any metadata
  Free_Metadata (motion);
  // Free the motion
  free (motion);
}

/*____________________________________________________________________
|
| Function: Free_Bones
|
| Input: Called from gx3d_Motion_Free()
| Output: Frees all memory associated with bones array.
|___________________________________________________________________*/

static void Free_Bones (gx3dMotion *motion)
{
  int i;

  DEBUG_ASSERT (motion);

  if (motion->bones) {
    for (i=0; i<motion->num_bones; i++) {
      if (motion->bones[i].pos_key)
        free (motion->bones[i].pos_key);
      if (motion->bones[i].rot_key)
        free (motion->bones[i].rot_key);
    }
    free (motion->bones);
  }
}

/*____________________________________________________________________
|
| Function: Free_Metadata
|
| Input: Called from gx3d_Motion_Free()
| Output: Frees all memory associated with metadata.
|___________________________________________________________________*/

static void Free_Metadata (gx3dMotion *motion)
{
  int i, j;

  DEBUG_ASSERT (motion)

  if (motion->metadata) {
    for (i=0; i<motion->num_metadata; i++) 
      for (j=0; j<gx3dMotionMetadata_MAX_CHANNELS; j++)
        if (motion->metadata[i].channel[j].keys)
          free (motion->metadata[i].channel[j].keys);
    free (motion->metadata);
  }
}
 
/*____________________________________________________________________
|
| Function: gx3d_Motion_Free_All
| 
| Output: Frees memory for all motions.
|___________________________________________________________________*/

void gx3d_Motion_Free_All () 
{
#ifndef _DEBUG
  // Free every motion in the linked list of motions
  while (motionlist)
    gx3d_Motion_Free (motionlist);
#else
  int i;
  for (i=0; motionlist; i++)
    gx3d_Motion_Free (motionlist);
  if (i) {
    char str[128];
    sprintf (str, "gx3d_Motion_Free_All(): Freeing %d motions left in memory", i);
    DEBUG_WRITE (str)
  }
#endif
}

/*____________________________________________________________________
|
| Function: gx3d_Motion_Set_Output
| 
| Output: Sets output of motion to a track of a blendnode.
|___________________________________________________________________*/

void gx3d_Motion_Set_Output (gx3dMotion *motion, gx3dBlendNode *blendnode, gx3dBlendNodeTrack track)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (motion)
  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT ((track == gx3d_BLENDNODE_TRACK_0) OR
                (track == gx3d_BLENDNODE_TRACK_1) OR
                (track == gx3d_BLENDNODE_TRACK_2))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/
  
  motion->output_local_pose = gx3d_BlendNode_Get_Input (blendnode, track);
}

/*____________________________________________________________________
|
| Function: gx3d_Motion_Udpate
| 
| Output: Samples motion at local_time.  If local_time = 0, this is the
|   start of the animation, etc.
|
|   Returns true if the animation is playing or false if local_time
|   is greater than the length of the animation (the animation has
|   stopped) and the animation doesn't loop.
|___________________________________________________________________*/

bool gx3d_Motion_Update (gx3dMotion *motion, float local_time, bool repeat)
{
  unsigned milliseconds;
  bool playing;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (motion)
  DEBUG_ASSERT (motion->max_nkeys)
  DEBUG_ASSERT (motion->keys_per_second)
  DEBUG_ASSERT (motion->duration)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/
 
  // Convert local time into milliseconds
  milliseconds = (unsigned)(local_time * 1000);

  if (repeat)
    milliseconds %= motion->duration;

  // Is this motion still playing?
  if (milliseconds <= motion->duration) 
    playing = true;
  else
    playing = false;

  if (playing) 
    // Animate all bones
    Animate_Bones (motion, milliseconds);

  return (playing);
}

/*____________________________________________________________________
|
| Function: Animate_Bones
|
| Input: Called from gx3d_Motion_Update()
| Output: Animates a bone including linked bones and child bones.
|   Returns true if updated, else false if no changes.  Assumes bones
|   in the array are ordered with no child before its parent.
|___________________________________________________________________*/

static void Animate_Bones (gx3dMotion *motion, unsigned milliseconds)
{
  int i;
  int key, curkey;
  float t;
  gx3dMotionBone *bone;
  gx3dVector v;
  gx3dQuaternion q1, q2;
  gx3dCompressedQuaternion cq1, cq2;

  DEBUG_ASSERT (motion)
  DEBUG_ASSERT (motion->output_local_pose)

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/
  
  // Compute current key (rounded down)
	curkey = (int)(milliseconds * motion->keys_per_second * ONE_OVER_THOUSAND);
  // Compute time between curkey and next key as a value between 0-1
  t = (float)(milliseconds * motion->keys_per_second % 1000) * ONE_OVER_THOUSAND;

/*____________________________________________________________________
|
| Animate all bones
|___________________________________________________________________*/

  for (i=0; i<motion->num_bones; i++) {
    bone = &(motion->bones[i]);
    // Calculate current key
    if (curkey < bone->nkeys)
      key = curkey;
    else
      key = bone->nkeys - 1;

/*____________________________________________________________________
|
| Rotate bone
|___________________________________________________________________*/
		
    // Set new local matrix
		if (bone->nkeys) {
      // Use last key?
      if (key == bone->nkeys-1) {
        cq1 = bone->rot_key[key];
        DECOMPRESS_QUATERNION (cq1, q1)
      }
      // Interpolate between 2 keys
      else {
        cq1 = bone->rot_key[key];                              
        DECOMPRESS_QUATERNION (cq1, q1)
        cq2 = bone->rot_key[key+1];
        DECOMPRESS_QUATERNION (cq2, q2)
        gx3d_GetSlerpQuaternion (&q1, &q2, t, &q1);
//        gx3d_NormalizeQuaternion (&q1);             // ********** Is this needed????? - nope, apparently not
      }
		}
    // For inactive bones (bones with no keyframes) just use default bone pose (rotation)
		else 
      q1 = bone->qrotation;

    // Output this quaternion
    motion->output_local_pose->bone_pose[i].q = q1;

/*____________________________________________________________________
|
| Translate root bone as needed (in case root bone is being translated 
|   to move the entire model)
|___________________________________________________________________*/

		// If this is root bone, translate root bone position (only root bone has pos keys)
//		if (bone->parent == 0xFF) {
    if (i == 0) {
      // Use last key?
      if (key == bone->nkeys-1) {
        v.x = bone->pos_key[key].x;// - bone->pivot.x;    // see lws.cpp (line 813), don't need to do this here since it's handled there
        v.y = bone->pos_key[key].y;// - bone->pivot.y;
        v.z = bone->pos_key[key].z;// - bone->pivot.z;
      }
      // Interpolate between 2 keys
      else {
        v.x = gx3d_Lerp (bone->pos_key[key].x, bone->pos_key[key+1].x, t);// - bone->pivot.x;
        v.y = gx3d_Lerp (bone->pos_key[key].y, bone->pos_key[key+1].y, t);// - bone->pivot.y;
        v.z = gx3d_Lerp (bone->pos_key[key].z, bone->pos_key[key+1].z, t);// - bone->pivot.z;
      }
      // Output this translation vector
      motion->output_local_pose->root_translate = v;
    }
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Motion_Write_GX3DANI_File
| 
| Output: Writes out motion to a file.
|
| Note: May need to do translation for opengl_formatting of rotation
|   data - won't know until test this data with an opengl program.
|___________________________________________________________________*/

void gx3d_Motion_Write_GX3DANI_File (gx3dMotion *motion, char *filename, bool opengl_formatting)
{
  int i, j, n;
  gx3dVector v;
  FILE *fp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (motion)
  DEBUG_ASSERT (filename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Open output file
  fp = fopen (filename, "wb");
  if (fp == 0)
    DEBUG_ERROR ("gx3d_Motion_Write_GX3DANI_File(): can't open output file")
  else {
    // Write name
    fwrite (motion->name, sizeof(char), gx_ASCIIZ_STRING_LENGTH_LONG, fp);
    // Write position
    v = motion->position;
    if (opengl_formatting)
      v.z = -v.z;
    fwrite (&v, sizeof(gx3dVector), 1, fp);
    // Write rotation
    fwrite (&(motion->rotation), sizeof(gx3dVector), 1, fp);
    // Write keys_per_second
    fwrite (&(motion->keys_per_second), sizeof(int), 1, fp);
    // Write max_nkeys
    fwrite (&(motion->max_nkeys), sizeof(int), 1, fp);
    // Write duration
    fwrite (&(motion->duration), sizeof(unsigned), 1, fp);
    // Write num_bones
    fwrite (&(motion->num_bones), sizeof(int), 1, fp);
    // Write num_metadata
    fwrite (&(motion->num_metadata), sizeof(int), 1, fp);

    // Write bones
    for (i=0; i<motion->num_bones; i++) {
      // Write name
      fwrite (motion->bones[i].name, sizeof(char), gx_ASCIIZ_STRING_LENGTH_LONG, fp);
      // Write weightmap_name
      fwrite (motion->bones[i].weightmap_name, sizeof(char), gx_ASCIIZ_STRING_LENGTH_LONG, fp);
      // Write position
      v = motion->bones[i].pivot;
      if (opengl_formatting)
        v.z = -v.z;
      fwrite (&v, sizeof(gx3dVector), 1, fp);
      // Write qrotation
      fwrite (&(motion->bones[i].qrotation), sizeof(gx3dQuaternion), 1, fp);
      // Write active
      fwrite (&(motion->bones[i].active), sizeof(bool), 1, fp);
      // Write nkeys
      fwrite (&(motion->bones[i].nkeys), sizeof(int), 1, fp);
      // Write parent
      fwrite (&(motion->bones[i].parent), sizeof(unsigned char), 1, fp);
      // Write pos_keys
      if (motion->bones[i].parent == 0xFF) {
        DEBUG_ASSERT (motion->bones[i].pos_key);  // root bone only should have pos_keys
        if (opengl_formatting) {
          for (j=0; j<motion->bones[i].nkeys; j++) {
            v = motion->bones[i].pos_key[j];
            v.z = -v.z;
            fwrite (&v, sizeof(gx3dVector), 1, fp);
          }
        }
        else
          fwrite (motion->bones[i].pos_key, sizeof(gx3dVector), motion->bones[i].nkeys, fp);
      }
      else {
       DEBUG_ASSERT (motion->bones[i].pos_key == 0);
      }
      // Write rot_keys
      if (motion->bones[i].active) {
        DEBUG_ASSERT (motion->bones[i].rot_key); // active bones should have rot_keys
        fwrite (motion->bones[i].rot_key, sizeof(gx3dCompressedQuaternion), motion->bones[i].nkeys, fp);
      }
      else {
        DEBUG_ASSERT (motion->bones[i].rot_key == 0);
      }
    }

    // Write metadata
    for (i=0; i<motion->num_metadata; i++) {
      // Write name
      fwrite (motion->metadata[i].name, sizeof(char), gx_ASCIIZ_STRING_LENGTH_LONG, fp);
      // Write channels_present
      fwrite (&(motion->metadata[i].channels_present), sizeof(unsigned), 1, fp);
      // Write duration
      fwrite (&(motion->metadata[i].duration), sizeof(unsigned), 1, fp);
      // Write channels
      for (j=0; j<gx3dMotionMetadata_MAX_CHANNELS; j++) {
        n = channel_info[j].channel_index;
        if (motion->metadata[i].channels_present & channel_info[j].channel_id) {
          DEBUG_ASSERT (motion->metadata[i].channel[n].nkeys);
          fwrite (&(motion->metadata[i].channel[n].nkeys), sizeof(int), 1, fp);
          DEBUG_ASSERT (motion->metadata[i].channel[n].keys);
          fwrite (motion->metadata[i].channel[n].keys, sizeof(gx3dMotionMetadataKey), motion->metadata[i].channel[n].nkeys, fp);
        }
      }
    }   

    // Close output file
    fclose (fp);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Motion_GetMetadata
| 
| Output: Returns pointer to metadata or null if not found.
|___________________________________________________________________*/

gx3dMotionMetadata *gx3d_Motion_GetMetadata (gx3dMotion *motion, char *name)
{
  int i;
  gx3dMotionMetadata *metadata = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (motion)
  DEBUG_ASSERT (name)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Look at any metadata in this motion
  for (i=0; i<motion->num_metadata; i++) 
    // Is this metadata we are looking for?
    if (!strcmp(motion->metadata[i].name, name)) {
      metadata = &(motion->metadata[i]);
      break;
    }

  return (metadata);
}

/*____________________________________________________________________
|
| Function: gx3d_MotionMetadata_GetSample
| 
| Output: Returns sample of channel at time.  If the channel doesn't
|   have any data then returns 0 in sample. time in milliseconds is
|   local to the animation (0=start of animation).
|   
|   Returns true if animation is playing, false if not playing.  If not
|   playing returns 0 in sample.
|
| TO DO: Modify to take into account time coherence instead of doing
|   linear search.
|___________________________________________________________________*/

bool gx3d_MotionMetadata_GetSample (gx3dMotionMetadata *metadata, gx3dMotionMetadataChannelIndex channel_index, float local_time, bool repeat, float *sample)
{
  int i, j;
  unsigned milliseconds;
  float t;
  bool playing;
  gx3dMotionMetadataChannel *metachannel;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (metadata)
  DEBUG_ASSERT ((channel_index == gx3dMotionMetadataChannelIndex_POS_X) OR
                (channel_index == gx3dMotionMetadataChannelIndex_POS_Y) OR
                (channel_index == gx3dMotionMetadataChannelIndex_POS_Z) OR
                (channel_index == gx3dMotionMetadataChannelIndex_ROT_X) OR
                (channel_index == gx3dMotionMetadataChannelIndex_ROT_Y) OR
                (channel_index == gx3dMotionMetadataChannelIndex_ROT_Z))
  DEBUG_ASSERT (sample)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Convert local time into milliseconds
  milliseconds = (unsigned)(local_time * 1000);

  if (repeat)
    milliseconds %= metadata->duration;    

  // Is this motion still playing?
  if (milliseconds <= metadata->duration) 
    playing = true;
  else {
    playing = false;
    *sample = 0;
  }
    
  if (playing) {
     metachannel = &(metadata->channel[channel_index]);
    // If channel has no data just return 0
    if (metachannel->nkeys == 0)
      *sample = 0;
    else {
      // Compute time in seconds
      t = milliseconds * ONE_OVER_THOUSAND;
      // Find closest value of t in timeline (linear search - could probably be improved on by taking into account time coherence)
      for (i=0; i<metachannel->nkeys AND t>metachannel->keys[i].time; i++);
      if (i > 0) // round down to previous key
        i--;                                                      
      j = i + 1;  // lerp with next key           
      if (j > metachannel->nkeys-1) // make sure don't go past last key       
        j = metachannel->nkeys-1;
      // Compute time between first and next key as a value between 0-1
      t = (t - metachannel->keys[i].time) / (metachannel->keys[j].time - metachannel->keys[i].time);
      // Lerp the 2 keys
      *sample = gx3d_Lerp (metachannel->keys[i].value, metachannel->keys[j].value, t);
    }
  }

  return (playing);
}

/*____________________________________________________________________
|
| Function: gx3d_MotionMetadata_Copy
| 
| Output: Performs deep copy.  Returns pointer to metadata or 0 on any
|   error.
|___________________________________________________________________*/

gx3dMotionMetadata *gx3d_MotionMetadata_Copy (gx3dMotionMetadata *metadata)
{
  int i;
  gx3dMotionMetadata *new_metadata = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (metadata)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create metadata struct
  new_metadata = (gx3dMotionMetadata *) calloc (1, sizeof(gx3dMotionMetadata));
  if (new_metadata == 0)
    TERMINAL_ERROR ("gx3d_MotionMetadata_Copy(): can't allocate memory for gx3dMotionMetadata")
  // Peform shallow copy
  *new_metadata = *metadata;
  // Copy channel data, if any
  for (i=0; i<gx3dMotionMetadata_MAX_CHANNELS; i++) 
    if (metadata->channel[i].keys) {
      new_metadata->channel[i].keys = (gx3dMotionMetadataKey *) malloc (metadata->channel[i].nkeys * sizeof(gx3dMotionMetadataKey));
      if (new_metadata->channel[i].keys == 0)
        TERMINAL_ERROR ("gx3d_MotionMetadata_Copy(): can't allocate memory for gx3dMotionMetadataKey array")
      memcpy ((void *)(new_metadata->channel[i].keys), (void *)(metadata->channel[i].keys), metadata->channel[i].nkeys * sizeof(gx3dMotionMetadataKey));
    }
  // Copy next node in linked list?
  if (metadata->next)
    new_metadata->next = gx3d_MotionMetadata_Copy (metadata->next);

  return (new_metadata);
}











/*____________________________________________________________________
|
| Function: Animate_Bones_OLD
|
| Input: Called from gx3d_Motion_Animate()
| Output: Animates a bone including linked bones and child bones.
|   Returns true if updated, else false if no changes.  Assumes bones
|   in the array are ordered with no child before its parent.
|___________________________________________________________________*/

static void Animate_Bones_OLD (gx3dMotion *motion, unsigned milliseconds, gx3dVector *new_position)
{
  //int i;
  //int key, curkey;
  //float t;
  //gx3dMotionBone *bone;
  //gx3dMatrix mt, m;
  //gx3dVector v;
  //gx3dQuaternion q1, q2;
  //gx3dCompressedQuaternion cq1, cq2;

  //DEBUG_ASSERT (motion)

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/
  
 // // Compute current key (rounded down)
	//curkey = (int)(milliseconds * motion->keys_per_second * ONE_OVER_THOUSAND);
 // // Compute time between curkey and next key as a value between 0-1
 // t = (float)(milliseconds * motion->keys_per_second % 1000) * ONE_OVER_THOUSAND;

/*____________________________________________________________________
|
| Animate all bones
|___________________________________________________________________*/

  //for (i=0; i<motion->num_bones; i++) {
  //  bone = &(motion->bones[i]);
  //  // Calculate current key
  //  if (curkey < bone->nkeys)
  //    key = curkey;
  //  else
  //    key = bone->nkeys - 1;

/*____________________________________________________________________
|
| Rotate bone
|___________________________________________________________________*/
		
  //  // Set new local matrix
		//if (bone->nkeys) {
  //    // Use last key?
  //    if (key == bone->nkeys-1) {
  //      cq1 = bone->rot_key[key];
  //      DECOMPRESS_QUATERNION (cq1, q1)
  //    }
  //    // Interpolate between 2 keys
  //    else {
  //      cq1 = bone->rot_key[key];                              
  //      DECOMPRESS_QUATERNION (cq1, q1)
  //      cq2 = bone->rot_key[key+1];
  //      DECOMPRESS_QUATERNION (cq2, q2)
  //      gx3d_GetSlerpQuaternion (&q1, &q2, t, &q1);
  //    }
		//}
  //  // For inactive bones (bones with no keyframes) just use default bone pose (rotation)
		//else 
  //    q1 = bone->qrotation;

  //  // Build a matrix from the quaternion
  //  gx3d_GetQuaternionMatrix (&q1, &m);

/*____________________________________________________________________
|
| Combine with:
|   pre  - Translate (-bone pivot) 
|          Rotate (inverse parent bone composite rotation)
|          Rotate (inverse bone rotation)
|   post - Rotate (parent bone composite rotation)
|          Translate (bone pivot)
|___________________________________________________________________*/

    //gx3d_MultiplyMatrix (&(bone->pre), &m, &m);
    //gx3d_MultiplyMatrix (&m, &(bone->post), &m);

/*____________________________________________________________________
|
| Translate root bone as needed (in case root bone is being translated to move the entire model)
|___________________________________________________________________*/

		//// If this is root bone, translate root bone position (only root bone has pos keys)
		//if (bone->parent == 0xFF) {
  //    // Use last key?
  //    if (key == bone->nkeys-1) {
  //      v.x = bone->pos_key[key].x - bone->pivot.x;
  //      v.y = bone->pos_key[key].y - bone->pivot.y;
  //      v.z = bone->pos_key[key].z - bone->pivot.z;
  //    }
  //    // Interpolate between 2 keys
  //    else {
  //      v.x = gx3d_Lerp (bone->pos_key[key].x, bone->pos_key[key+1].x, t) - bone->pivot.x;
  //      v.y = gx3d_Lerp (bone->pos_key[key].y, bone->pos_key[key+1].y, t) - bone->pivot.y;
  //      v.z = gx3d_Lerp (bone->pos_key[key].z, bone->pos_key[key+1].z, t) - bone->pivot.z;
  //    }
  //    gx3d_GetTranslateMatrix (&mt, v.x, v.y, v.z);
		//	gx3d_MultiplyMatrix (&m, &mt, &m);
  //    // Send this info back to caller?
  //    if (new_position)
  //      *new_position = v;
  //  }
  //	bone->transform.local_matrix = m;
  //}
}

/*____________________________________________________________________
|
| Function: gx3d_Motion_Print
| 
| Output: Prints (some) contents of motion to a text file.
|___________________________________________________________________*/

void gx3d_Motion_Print (gx3dMotion *motion, char *outputfilename)
{
  int i, j, k, n;
  ofstream out;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (motion)
  DEBUG_ASSERT (outputfilename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Open output file
  out.open (outputfilename);
//  if (out == 0)
//    DEBUG_ERROR ("gx3d_Motion_Print(): can't open output file")
//  else {
    // Write name
    out << "[Name] " << motion->name << endl;
    // Write position
    out << "[Position] " << motion->position.x << "," << motion->position.y << "," << motion->position.z << endl;
    // Write rotation
    out << "[Rotation] " << motion->rotation.x << "," << motion->rotation.y << "," << motion->rotation.z << endl;
    // Write keys_per_second
    out << "[Keys-per-second] " << motion->keys_per_second << endl;
    // Write max_nkeys
    out << "[Max-nkeys] " << motion->max_nkeys << endl;
    // Write duration
    out << "[Duration] " << motion->duration << endl;
    // Write num_bones
    out << "[Num-bones] " << motion->num_bones << endl;
    // Write num_metadata
    out << "[Num-metadata] " << motion->num_metadata << endl;
    out << endl;

    // Write bones
    for (i=0; i<motion->num_bones; i++) {
      // Write name
      out << "[Bone-name] " << motion->bones[i].name << endl;
      // Write weightmap_name
      out << "[Weightmap-name] " << motion->bones[i].weightmap_name << endl;
      // Write position
      out << "[Position] " << motion->bones[i].pivot.x << "," << motion->bones[i].pivot.y << "," << motion->bones[i].pivot.z << endl;
      // Write qrotation
      out << "[Qrotation] " << motion->bones[i].qrotation.x << "," << motion->bones[i].qrotation.y << "," << motion->bones[i].qrotation.z << "," << motion->bones[i].qrotation.w << endl;
      // Write active
      out << "[Active] " << motion->bones[i].active << endl;
      // Write nkeys
      out << "[Nkeys] " << motion->bones[i].nkeys << endl;
      // Write parent
      if (motion->bones[i].parent == 0xFF)
        out << "[Parent]" << endl; // no parent
      else
        out << "[Parent] " << (int)(motion->bones[i].parent) << endl;
      // Write pos_keys
      if (motion->bones[i].parent == 0xFF) {
        DEBUG_ASSERT (motion->bones[i].pos_key);  // root bone only should have pos_keys
        out << "[Pos-keys]" << endl;
        for (j=0; j<motion->bones[i].nkeys; j++)
          out << "  [" << j << "] "<< motion->bones[i].pos_key[j].x << "," << motion->bones[i].pos_key[j].y << "," << motion->bones[i].pos_key[j].z << endl;
      }
      else {
        DEBUG_ASSERT (motion->bones[i].pos_key == 0);
      }
      // Write rot_keys
      if (motion->bones[i].active) {
        DEBUG_ASSERT (motion->bones[i].rot_key); // active bones should have rot_keys
        out << "[Rot-keys]" << endl;
        for (j=0; j<motion->bones[i].nkeys; j++)
          out << "  [" << j << "] " << motion->bones[i].rot_key[j].x << "," << motion->bones[i].rot_key[j].y << "," << motion->bones[i].rot_key[j].z << "," << motion->bones[i].rot_key[j].w << endl;
      }
      else {
        DEBUG_ASSERT (motion->bones[i].rot_key == 0);
      }
      out << endl;
    }

    // Write metadata
    for (i=0; i<motion->num_metadata; i++) {
      // Write name
      out << "[Metadata-name] " << motion->metadata[i].name << endl;
      // Write channels_present
      out << "[Channels-present] " << endl;
      for (j=0; j<gx3dMotionMetadata_MAX_CHANNELS; j++)
        if (motion->metadata[i].channels_present & channel_info[j].channel_id)
          out << "  " << channel_info[j].channel_string << endl;
      // Write duration
      out << "[Duration] " << motion->metadata[i].duration << endl;
      // Write channels
      for (j=0; j<gx3dMotionMetadata_MAX_CHANNELS; j++) {
        n = channel_info[j].channel_index;
        if (motion->metadata[i].channels_present & channel_info[j].channel_id) {
          out << "[Channel] " << channel_info[j].channel_string << endl;
          DEBUG_ASSERT (motion->metadata[i].channel[n].nkeys);
          out << "  [Nkeys] " << motion->metadata[i].channel[n].nkeys << endl;
          DEBUG_ASSERT (motion->metadata[i].channel[n].keys);
          for (k=0; k<motion->metadata[i].channel[n].nkeys; k++)
            out << "    (time,value) [" << k << "] " << motion->metadata[i].channel[n].keys[k].time << "," << motion->metadata[i].channel[n].keys[k].value << endl;
        }
      }
      out << endl;
    }   
//  }
}
