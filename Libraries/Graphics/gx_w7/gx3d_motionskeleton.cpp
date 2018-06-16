/*____________________________________________________________________
|
| File: gx3d_motionskeleton.cpp
|
| Description: Functions to manipulate a gx3dMotionSkeleton.
|
| Functions:  gx3d_MotionSkeleton_Init
|             gx3d_MotionSkeleton_Read_LWS_File
|             gx3d_MotionSkeleton_Read_GX3DSKEL_File
|              Verify_Skeleton
|             gx3d_MotionSkeleton_Free
|             gx3d_MotionSkeleton_Free_All
|             gx3d_MotionSkeleton_Print
|             gx3d_MotionSkeleton_Write_GX3DSKEL_File
|             gx3d_MotionSkeleton_GetBoneIndex
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

/*___________________
|
| Constants
|__________________*/

#define ADD_TO_SKELETONLIST(_skel_)     \
  {                                     \
    if (skeletonlist == 0)              \
      skeletonlist = _skel_;            \
    else {                              \
      _skel_->next = skeletonlist;      \
      skeletonlist->previous = _skel_;  \
      skeletonlist = _skel_;            \
    }                                   \
  }

#define REMOVE_FROM_SKELETONLIST(_skel_)          \
  {                                               \
    if (_skel_->previous)                         \
      _skel_->previous->next = _skel_->next;      \
    else                                          \
      skeletonlist = _skel_->next;                \
    if (_skel_->next)                             \
      _skel_->next->previous = _skel_->previous;  \
  }

/*___________________
|
| Function prototypes
|__________________*/

static bool Verify_Skeleton (gx3dMotionSkeleton *skeleton);

/*___________________
|
| Global variables
|__________________*/

static gx3dMotionSkeleton *skeletonlist = 0;	// doubly linked list of skeletons

/*____________________________________________________________________
|
| Function: gx3d_MotionSkeleton_Init
| 
| Output: Creates an empty skeleton.  Returns pointer or 0 on any error.
|___________________________________________________________________*/

gx3dMotionSkeleton *gx3d_MotionSkeleton_Init ()
{
  gx3dMotionSkeleton *skeleton = 0;

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Allocate memory for top-level data struct
  skeleton = (gx3dMotionSkeleton *) calloc (1, sizeof(gx3dMotionSkeleton));
  if (skeleton == 0) 
    TERMINAL_ERROR ("gx3d_MotionSkeleton_Init(): can't allocate memory for skeleton");
  // Add to skeletonlist
  ADD_TO_SKELETONLIST (skeleton);

  return (skeleton);
}

/*____________________________________________________________________
|
| Function: gx3d_MotionSkeleton_Read_LWS_File
| 
| Output: Creates a skeleton from a LWS file.  Motion file should 
|   contain the bind pose.  No animation data is necessary.  Returns 
|   pointer to skeleton or 0 on any error.
|___________________________________________________________________*/

gx3dMotionSkeleton *gx3d_MotionSkeleton_Read_LWS_File (char *filename)
{
  gx3dMotionSkeleton *skeleton = 0;
 
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (filename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create a new empty gx3d skeleton
  skeleton = gx3d_MotionSkeleton_Init ();
  // Read in data from LWS file
  LWS_File_To_GX3D_MotionSkeleton (filename, skeleton);
  // Verify read in correctly
  if (NOT Verify_Skeleton (skeleton))
    TERMINAL_ERROR ("gx3d_MotionSkeleton_Read_LWS_File(): skeleton bones not in parent-child relationship order");

  return (skeleton);
}

/*____________________________________________________________________
|
| Function: gx3d_MotionSkeleton_Read_GX3DSKEL_File
| 
| Output: Creates a skeleton from a GX3DSKEL file.  Motion file should 
|   contain the bind pose.  No animation data is necessary.  Returns 
|   pointer to skeleton or 0 on any error.
|___________________________________________________________________*/

gx3dMotionSkeleton *gx3d_MotionSkeleton_Read_GX3DSKEL_File (char *filename)
{
  int i;
  FILE *fp;
  gx3dMotionSkeleton *skeleton = 0;
 
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (filename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create a new empty skeleton
  skeleton = gx3d_MotionSkeleton_Init ();
  
  // Open input file
  fp = fopen (filename, "rb");
  if (fp == 0)
    TERMINAL_ERROR ("gx3d_MotionSkeleton_Read_GX3DSKEL_File(): can't open input file")
  else {
    // Read num bones
    fread (&(skeleton->num_bones), sizeof(int), 1, fp);
    DEBUG_ASSERT (skeleton->num_bones);
    // Allocate memory for bones
    skeleton->bones = (gx3dMotionSkeletonBone *) malloc (skeleton->num_bones * sizeof(gx3dMotionSkeletonBone));
    if (skeleton->bones == 0)
      TERMINAL_ERROR ("gx3d_MotionSkeleton_Read_GX3DSKEL_File(): can't allocate memory for bones array")
    // Read out each bone
    for (i=0; i<skeleton->num_bones; i++) {
      // Read pre matrix
      fread (&(skeleton->bones[i].pre), sizeof(gx3dMatrix), 1, fp);
      // Read post matrix
      fread (&(skeleton->bones[i].post), sizeof(gx3dMatrix), 1, fp);
      // Read name
      fread (skeleton->bones[i].name, sizeof(char), gx_ASCIIZ_STRING_LENGTH_LONG, fp);
      // Read parent
      fread (&(skeleton->bones[i].parent), sizeof(unsigned char), 1, fp);
    }
    // Close input file
    fclose (fp);
  }

  // Verify read in correctly
  if (NOT Verify_Skeleton (skeleton))
    TERMINAL_ERROR ("gx3d_MotionSkeleton_Read_GX3DSKEL_File(): skeleton bones not in parent-child relationship order");

  return (skeleton);
}

/*____________________________________________________________________
|
| Function: Verify_Skeleton
| 
| Input: Called from gx3d_MotionSkeleton_Read_LWS_File()
|                    gx3d_MotionSkeleton_Read_GX3DSKEL_File()
| Output: Returns true if skeleton verified.
|___________________________________________________________________*/

static bool Verify_Skeleton (gx3dMotionSkeleton *skeleton)
{
  int i;
  bool verified = true;

  // Make sure root bone has no parent
  if (skeleton->bones[0].parent != 0xFF)
    verified = false;
  // Make sure each bone's parent is earlier in the array - don't bother checking root bone
  if (verified)
    for (i=1; i<skeleton->num_bones; i++)
      if ((skeleton->bones[i].parent == 0xFF) OR (skeleton->bones[i].parent >= i)) {
        verified = false;
        break;
      }

  return (verified);
}

/*____________________________________________________________________
|
| Function: gx3d_MotionSkeleton_Free
| 
| Output: Frees memory for the skeleton.
|___________________________________________________________________*/

void gx3d_MotionSkeleton_Free (gx3dMotionSkeleton *skeleton)
{
 
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skeleton)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

	// Remove it from the list of skeletons
  REMOVE_FROM_SKELETONLIST (skeleton)
  // Free array of bones first
  if (skeleton->bones)
    free (skeleton->bones);
  // Free top-level struct
  free (skeleton);
}

/*____________________________________________________________________
|
| Function: gx3d_MotionSkeleton_Free_All
| 
| Output: Frees memory for all skeletons.
|___________________________________________________________________*/

void gx3d_MotionSkeleton_Free_All () 
{
#ifndef _DEBUG
  // Free every skeleton in the linked list of skeletons
  while (skeletonlist)
    gx3d_MotionSkeleton_Free (skeletonlist);
#else
  int i;
  for (i=0; skeletonlist; i++)
    gx3d_MotionSkeleton_Free (skeletonlist);
  if (i) {
    char str[128];
    sprintf (str, "gx3d_MotionSkeleton_Free_All(): Freeing %d skeletons left in memory", i);
    DEBUG_WRITE (str)
  }
#endif
}

/*____________________________________________________________________
|
| Function: gx3d_MotionSkeleton_Print
| 
| Output: Prints contents of motion skeleton to a text file.
|___________________________________________________________________*/

void gx3d_MotionSkeleton_Print (gx3dMotionSkeleton *skeleton, char *outputfilename)
{
  int i;
  ofstream out;

  DEBUG_ASSERT (skeleton)
  DEBUG_ASSERT (outputfilename)

  out.open (outputfilename);
  if (NOT out)
		DEBUG_ERROR ("gx3d_MotionSkeleton_Print(): Can't open output text file")
  else 
    for (i=0; i<skeleton->num_bones; i++) {
      if (skeleton->bones[i].parent == 0xFF)
        out << "Bone: " << skeleton->bones[i].name << " [root]" << endl;
      else
        out << "Bone: " << skeleton->bones[i].name << " [parent = " << skeleton->bones[skeleton->bones[i].parent].name << "]" << endl;
    }
}

/*____________________________________________________________________
|
| Function: gx3d_MotionSkeleton_Write_GX3DSKEL_File
| 
| Output: Writes out skeleton to a file.
|___________________________________________________________________*/

void gx3d_MotionSkeleton_Write_GX3DSKEL_File (gx3dMotionSkeleton *skeleton, char *filename)
{
  int i;
  FILE *fp;

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

  // Open output file
  fp = fopen (filename, "wb");
  if (fp == 0)
    DEBUG_ERROR ("gx3d_MotionSkeleton_Write_GX3DSKEL_File(): can't open output file")
  else {
    // Write num bones
    fwrite (&(skeleton->num_bones), sizeof(int), 1, fp);
    // Write out each bone
    for (i=0; i<skeleton->num_bones; i++) {
      // Write pre matrix
      fwrite (&(skeleton->bones[i].pre), sizeof(gx3dMatrix), 1, fp);
      // Write post matrix
      fwrite (&(skeleton->bones[i].post), sizeof(gx3dMatrix), 1, fp);
      // Write name
      fwrite (skeleton->bones[i].name, sizeof(char), gx_ASCIIZ_STRING_LENGTH_LONG, fp);
      // Write parent
      fwrite (&(skeleton->bones[i].parent), sizeof(unsigned char), 1, fp);
    }
    // Close output file
    fclose (fp);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_MotionSkeleton_GetBoneIndex
| 
| Output: Returns true if bone found and returns bone_index in callers
|   variable, else returns false.
|___________________________________________________________________*/

bool gx3d_MotionSkeleton_GetBoneIndex (gx3dMotionSkeleton *skeleton, char *bone_name, int *bone_index)
{
  int i;
  bool found = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skeleton)
  DEBUG_ASSERT (skeleton->num_bones)
  DEBUG_ASSERT (bone_name)
  DEBUG_ASSERT (bone_index)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/
  
  // Look for this bone name
  for (i=0; i<skeleton->num_bones; i++)
    if (!strcmp(bone_name, skeleton->bones[i].name)) {
      found = true;
      break;
    }
  if (found)
    *bone_index = i;

  return (found);
}
