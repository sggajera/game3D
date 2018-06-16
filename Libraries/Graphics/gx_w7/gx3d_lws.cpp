/*____________________________________________________________________
|
| File: gx3d_lws.cpp
|
| Description: Functions to reading LWS scene files.
|
| Functions:  LWS_File_To_GX3D_Motion
|              Verify_LWS_ObjectLayer
|              Copy_Name
|              Add_Metadata
|              Add_Metadata
|               Copy_Metadata_Channel
|              Convert_Metadata_List_To_Array
|             LWS_File_To_GX3D_MotionSkeleton
|              Parent_Bone
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
#include "lws.h"
#include "quantize.h"

#include "gx3d_lws.h"

/*___________________
|
| Function prototypes
|__________________*/

static bool Verify_LWS_ObjectLayer (lws_ObjectLayer *l_olayer);
static void Copy_Name (char *dst, char *src, int maxlength);
static void Add_Metadata (gx3dMotion *g_motion, lws_ObjectLayer *l_olayer);
static void Add_Metadata (gx3dMotion *g_motion, lws_ObjectLayer *l_olayer, gx3dMotionMetadataRequest *metadata_requested, int num_metadata_requested);
static void Copy_Metadata_Channel (gx3dMotionMetadataChannel *g_channel, lws_Channel *l_channel);
static void Convert_Metadata_List_To_Array (gx3dMotion *g_motion);
static lws_Bone *Parent_Bone (lws_ObjectLayer *olayer, lws_Bone *bone);

/*___________________
|
| Global variables
|__________________*/

static unsigned channel_id [gx3dMotionMetadata_MAX_CHANNELS] = {
  gx3dMotionMetadataChannel_POS_X,
  gx3dMotionMetadataChannel_POS_Y,
  gx3dMotionMetadataChannel_POS_Z,
  gx3dMotionMetadataChannel_ROT_X,
  gx3dMotionMetadataChannel_ROT_Y,
  gx3dMotionMetadataChannel_ROT_Z
};
#ifdef _DEBUG
static char *channel_name [gx3dMotionMetadata_MAX_CHANNELS] = { // only used in debug build to generate error messages
  "gx3dMotionMetadataChannel_POS_X",
  "gx3dMotionMetadataChannel_POS_Y",
  "gx3dMotionMetadataChannel_POS_Z",
  "gx3dMotionMetadataChannel_ROT_X",
  "gx3dMotionMetadataChannel_ROT_Y",
  "gx3dMotionMetadataChannel_ROT_Z"
};
#endif

/*____________________________________________________________________
|
| Function: LWS_File_To_GX3D_Motion
|                       
| Input: Called from gx3d_ReadLWSFile()                                                                 
| Output: Convert an LWS file to a gx3dMotion.  Returns true on success.
|___________________________________________________________________*/

void LWS_File_To_GX3D_Motion  (
  char                      *filename, 
  gx3dMotion                *g_motion,                // ptr to empty motion (all zeroed out)
  int                        frames_per_second,       // target framerate for gx3dMotion or 0=use file framerate
  gx3dMotionMetadataRequest *metadata_requested, 
  int                        num_metadata_requested,
  bool                       load_all_metadata )
{
  int i, j;
  gx3dMatrix m, mx, my, mz;
  gx3dQuaternion q;
  lws_ObjectLayer *l_olayer;
  lws_Bone        *l_bone, *pbone;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (filename)
  DEBUG_ASSERT (g_motion)
  DEBUG_ASSERT (frames_per_second >= 0)
  DEBUG_ASSERT ((metadata_requested AND num_metadata_requested >= 1) OR (metadata_requested == 0 AND num_metadata_requested == 0))

/*____________________________________________________________________
|
| Read and verify LWS file
|___________________________________________________________________*/

  // Read LWS data from file
  l_olayer = lws_ReadFile (filename, &frames_per_second, (bool)(num_metadata_requested != 0));
  if (l_olayer == 0) 
     TERMINAL_ERROR ("LWS_File_To_GX3D_Motion(): can't read LWS file")
  else {
    // Verify the lws objectlayer is compatible
    if (NOT Verify_LWS_ObjectLayer (l_olayer))
       TERMINAL_ERROR ("LWS_File_To_GX3D_Motion(): can't verify LWS objectlayer")
    else {
 
/*____________________________________________________________________
|
| Set data in motion
|___________________________________________________________________*/
      
      // Set name
      Copy_Name (g_motion->name, l_olayer->name, gx_ASCIIZ_STRING_LENGTH_LONG);
      // Set lwo object layer id
//      g_motion->object_layer_id = l_olayer->id;
			// Set position and rotation
			g_motion->position = l_olayer->position;
			g_motion->rotation = l_olayer->rotation;
      // Set framerate
      g_motion->keys_per_second = frames_per_second;
      // Set max nkeys (of any bone)
      g_motion->max_nkeys = l_olayer->max_nkeys;
      // Set motion duration (in milliseconds)
      g_motion->duration = (g_motion->max_nkeys - 1) * 1000 / g_motion->keys_per_second;
      // Set number of bones
      g_motion->num_bones = l_olayer->num_bones;
      // Allocate memory for array of bones
      g_motion->bones = (gx3dMotionBone *) calloc (g_motion->num_bones, sizeof(gx3dMotionBone));
      if (g_motion->bones == 0)
        TERMINAL_ERROR ("LWS_File_To_GX3D_Motion(): can't allocate memory for bones array")
/*____________________________________________________________________
|
| Copy bone data (relies on bones in the lws objectlayer being in 
|   hierarcical order, starting with root bone first)
|___________________________________________________________________*/

      for (i=0, l_bone=l_olayer->bones; i<g_motion->num_bones; i++, l_bone=l_bone->next) {
        // Make sure first bone in list is root bone
        if (i == 0) {
          if (l_bone->parent_id != -1)
            TERMINAL_ERROR ("LWS_File_To_GX3D_Motion(): first bone in olayer list must be the root bone")
        }
        // Make sure all other bones are not root
        else {
          if (l_bone->parent_id == -1)
            TERMINAL_ERROR ("LWS_File_To_GX3D_Motion(): found another root bone in olayer")
        }
        // Copy bone name
        Copy_Name (g_motion->bones[i].name, l_bone->name, gx_ASCIIZ_STRING_LENGTH_LONG);
        // Set pivot point for bone
	      g_motion->bones[i].pivot = l_bone->pivot;
        // Set bone rest rotation quaternion (only used by inactive bones, since they don't have keyframes)
        gx3d_GetRotateZMatrix (&mz, l_bone->rotation.z);
 	      gx3d_GetRotateXMatrix (&mx, l_bone->rotation.x);
        gx3d_GetRotateYMatrix (&my, l_bone->rotation.y);
        gx3d_MultiplyMatrix (&mz, &mx, &m);
        gx3d_MultiplyMatrix (&m, &my, &m);
        gx3d_GetMatrixQuaternion (&m, &q);
        gx3d_NormalizeQuaternion (&q, &q);
        g_motion->bones[i].qrotation = q;
        if (l_bone->weightmap_name) 
          // Copy name of weightmap
          Copy_Name (g_motion->bones[i].weightmap_name, l_bone->weightmap_name, gx_ASCIIZ_STRING_LENGTH_LONG);
        // Set active flag
        g_motion->bones[i].active = (l_bone->active == 1);
        // Set # keys
        if (g_motion->bones[i].active)
          g_motion->bones[i].nkeys = l_bone->motion.nkeys;
        // Set position keys (should be root bone only)
        if (l_bone->motion.pos) {
          // Allocate memory for array of keys
          g_motion->bones[i].pos_key = (gx3dVector *) malloc (g_motion->bones[i].nkeys * sizeof(gx3dVector));
          if (g_motion->bones[i].pos_key == 0)
            TERMINAL_ERROR ("LWS_File_To_GX3D_Motion(): Error allocating memory for position keys")
          // Copy position keys
          memcpy ((void *)(g_motion->bones[i].pos_key), (void *)(l_bone->motion.pos), g_motion->bones[i].nkeys * sizeof(gx3dVector));
        }
        // Set rotation keys (should be every bone, except inactive bones)
        if (g_motion->bones[i].active) {
          // Allocate memory for array of quaternion keys
          g_motion->bones[i].rot_key = (gx3dCompressedQuaternion *) malloc (g_motion->bones[i].nkeys * sizeof(gx3dCompressedQuaternion));
          if (g_motion->bones[i].rot_key == 0)
            TERMINAL_ERROR ("LWS_File_To_GX3D_Motion(): Error allocating memory for quaternion rotation keys")
          // Calculate quaternions
          for (j=0; j<g_motion->bones[i].nkeys; j++) {
            // Get bone rotation for this keyframe
            gx3d_GetRotateZMatrix (&mz, l_bone->motion.rot[j].z);
 	  	      gx3d_GetRotateXMatrix (&mx, l_bone->motion.rot[j].x);
            gx3d_GetRotateYMatrix (&my, l_bone->motion.rot[j].y);
            // Put all rotations together
            gx3d_MultiplyMatrix (&mz, &mx, &m);
            gx3d_MultiplyMatrix (&m, &my, &m);
            // Build a normalized quaternion from this rotation matrix
            gx3d_GetMatrixQuaternion (&m, &q);
            gx3d_NormalizeQuaternion (&q, &q);
            // Save as a compressed quaternion
            g_motion->bones[i].rot_key[j].x = CompressQuaternionValue (q.x); // shouldn't result in a loss of quality
            g_motion->bones[i].rot_key[j].y = CompressQuaternionValue (q.y);
            g_motion->bones[i].rot_key[j].z = CompressQuaternionValue (q.z);
            g_motion->bones[i].rot_key[j].w = CompressQuaternionValue (q.w);
          }
        }
      }

/*____________________________________________________________________
|
| Compute each bone's parent
|___________________________________________________________________*/

      // Set first bone in array as root (already verified this in above loop)
      g_motion->bones[0].parent = 0xFF;
      // Set parents of other bones in array
      for (i=1; i<g_motion->num_bones; i++) {
        // Find l_bone with same name as the motion bone
        for (l_bone=l_olayer->bones; l_bone; l_bone=l_bone->next) 
          if (!strcmp(g_motion->bones[i].name, l_bone->name))
            break;
        // Get parent of this l_bone
        pbone = Parent_Bone (l_olayer, l_bone);
        // Find same bone in motion bones array
        for (j=0; j<g_motion->num_bones; j++) 
          if (!strcmp(g_motion->bones[j].name, pbone->name)) {
            // This is the array index of the parent bone - save it
            g_motion->bones[i].parent = j;
            break;
          }
      }

/*____________________________________________________________________
|
| Read in metadata?
|___________________________________________________________________*/

      // Set metadata?
      if (load_all_metadata OR num_metadata_requested) {
        if (load_all_metadata)
          Add_Metadata (g_motion, l_olayer);
        else
          Add_Metadata (g_motion, l_olayer, metadata_requested, num_metadata_requested);
        // Convert metadata list into an array
        if (g_motion->num_metadata)
          Convert_Metadata_List_To_Array (g_motion);
      }
    } 

/*____________________________________________________________________
|
| Free memory
|___________________________________________________________________*/

    // Free memory for LWS objectlayer
    lws_FreeObjectLayer (l_olayer);
  }
}

/*____________________________________________________________________
|
| Function: Verify_LWS_ObjectLayer
|                       
| Input: Called from LWS_File_To_GX3D()                                                                 
| Output: Verifies the lws objectlayer can be converted to a gx3d format.
|   Returns true if compatible, else false.
|___________________________________________________________________*/

static bool Verify_LWS_ObjectLayer (lws_ObjectLayer *l_olayer)
{
  DEBUG_ASSERT (l_olayer)
  DEBUG_ASSERT (l_olayer->name)
  DEBUG_ASSERT (l_olayer->num_bones)
  DEBUG_ASSERT (l_olayer->bones)

  return (true);
}

/*____________________________________________________________________
|
| Function: Copy_Name
| 
| Input: Called LWS_File_To_GX3D()
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
| Function: Add_Metadata
| 
| Input: Called from LWS_File_To_GX3D()
| Output: Adds all metadata to gx3dMotion.
|___________________________________________________________________*/

static void Add_Metadata (gx3dMotion *g_motion, lws_ObjectLayer *l_olayer)
{
  int i;
  lws_Metadata *l_metadata;
  gx3dMotionMetadata *g_metadata;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (g_motion);
  DEBUG_ASSERT (l_olayer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Load each metadata, if any
  for (l_metadata=l_olayer->metadata; l_metadata; l_metadata=l_metadata->next) {
    // Create metadata struct
    g_metadata = (gx3dMotionMetadata *) calloc (1, sizeof(gx3dMotionMetadata));
    if (g_metadata == 0)
      TERMINAL_ERROR ("Add_Metadata(): can't allocate memory for gx3dMotionMetadata")
    // Copy name
    Copy_Name (g_metadata->name, l_metadata->name, gx_ASCIIZ_STRING_LENGTH_SHORT);
    // Set channels present
    for (i=0; i<gx3dMotionMetadata_MAX_CHANNELS; i++)
      // Does the corresponding LWS channel have data?
      if (l_metadata->channel[i].nkeys) 
        g_metadata->channels_present |= channel_id[i];
    // Set duration - same as parent motion
    g_metadata->duration = g_motion->duration;     
    // Copy requested channels
    for (i=0; i<gx3dMotionMetadata_MAX_CHANNELS; i++)
      if (l_metadata->channel[i].nkeys)
        Copy_Metadata_Channel (&(g_metadata->channel[i]), &(l_metadata->channel[i]));         
          
    // Add this metadata to linked list of metadata in gx3dMotion
    gx3dMotionMetadata **mpp;
    for (mpp=&(g_motion->metadata); *mpp; mpp=&((*mpp)->next));
    *mpp = g_metadata;

    // Increment number of metadata in g_motion
    g_motion->num_metadata++;
  }
}

/*____________________________________________________________________
|
| Function: Add_Metadata
| 
| Input: Called from LWS_File_To_GX3D()
| Output: Adds requested metadata to gx3dMotion.
|___________________________________________________________________*/

static void Add_Metadata (gx3dMotion *g_motion, lws_ObjectLayer *l_olayer, gx3dMotionMetadataRequest *metadata_requested, int num_metadata_requested)
{
  int i, j;
  bool found, compatible;
  char str[200];
  lws_Metadata *l_metadata;
  gx3dMotionMetadata *g_metadata;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (g_motion);
  DEBUG_ASSERT (l_olayer);
  DEBUG_ASSERT ((metadata_requested AND num_metadata_requested >= 1) OR (metadata_requested == 0 AND num_metadata_requested == 0))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Look for each requested metadata
  for (i=0; i<num_metadata_requested; i++) {
    // Try to match with an olayer metadata
    found = false;
    for (l_metadata=l_olayer->metadata; l_metadata; l_metadata=l_metadata->next) {
      // Same names?
      if (!strcmp(metadata_requested[i].name, l_metadata->name)) {
        found = true;
        // Make sure all channel data requested is available
        compatible = true;  // assume yes
        for (j=0; j<gx3dMotionMetadata_MAX_CHANNELS; j++) 
          // Is this a requested channel?
          if (metadata_requested[i].channels_requested & channel_id[j])
            // Does the corresponding LWS channel have no data?
            if (l_metadata->channel[j].nkeys == 0) {
              // Then, not compatible
              compatible = false;
              sprintf (str, "Add_Metadata(): requested metadata [%s] channel [%s] has no data - can't load this metadata", metadata_requested[i].name, channel_name[j]);
              DEBUG_ERROR (str);
              break;
            }
        // If compatible, create metadata data structure and copy data into it
        if (compatible) {
          // Create metadata struct
          g_metadata = (gx3dMotionMetadata *) calloc (1, sizeof(gx3dMotionMetadata));
          if (g_metadata == 0)
            TERMINAL_ERROR ("Add_Metadata(): can't allocate memory for gx3dMotionMetadata")
          // Copy name
          Copy_Name (g_metadata->name, metadata_requested[i].name, gx_ASCIIZ_STRING_LENGTH_SHORT);
          // Set channels that will have data
          g_metadata->channels_present = metadata_requested[i].channels_requested;
          // Set duration - same as parent motion
          g_metadata->duration = g_motion->duration;
          // Copy requested channels
          for (j=0; j<gx3dMotionMetadata_MAX_CHANNELS; j++) 
            if (metadata_requested[i].channels_requested & channel_id[j]) 
              Copy_Metadata_Channel (&(g_metadata->channel[j]), &(l_metadata->channel[j]));         
          
          // Add this metadata to linked list of metadata in gx3dMotion
          gx3dMotionMetadata **mpp;
          for (mpp=&(g_motion->metadata); *mpp; mpp=&((*mpp)->next));
          *mpp = g_metadata;

          // Increment number of metadata in g_motion
          g_motion->num_metadata++;
        }
      }
    }
    // If didn't find metadata, generate error message
    if (NOT found) {
      sprintf (str, "Add_Metadata(): metadata [%s] not found in LWS file", metadata_requested[i].name); 
      DEBUG_ERROR (str);
    }
  }
}

/*____________________________________________________________________
|
| Function: Copy_Metadata_Channel
| 
| Input: Called from Add_Metadata()
| Output: Copies metadata channel from olayer to gx3dMotion.
|___________________________________________________________________*/

static void Copy_Metadata_Channel (gx3dMotionMetadataChannel *g_channel, lws_Channel *l_channel)
{
  int size;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (l_channel);
  DEBUG_ASSERT (l_channel->nkeys);
  DEBUG_ASSERT (g_channel);
  // gx3dMotionMetadataKey and lws_Key must be same size struct
  DEBUG_ASSERT (sizeof(gx3dMotionMetadataKey) == sizeof(lws_Key));

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Set number of keys
  g_channel->nkeys = l_channel->nkeys;
  // Compute size in bytes of keys to copy
  size = g_channel->nkeys * sizeof(gx3dMotionMetadataKey);
  // Create memory for keys
  g_channel->keys = (gx3dMotionMetadataKey *) calloc (1, size);
  if (g_channel->keys == 0)
    TERMINAL_ERROR ("Copy_Metadata_Channel(): can't allocate memory for keys array")
  // Copy the keys
  memcpy ((void *)(g_channel->keys), (void *)(l_channel->keys), size);
}

/*____________________________________________________________________
|
| Function: Convert_Metadata_List_To_Array
| 
| Input: Called from LWS_File_To_GX3D()
| Output: Converts linked list of metadata into an array - for more
|   efficient access.
|___________________________________________________________________*/

static void Convert_Metadata_List_To_Array (gx3dMotion *g_motion)
{
  int i;
  gx3dMotionMetadata *marray, *mp, *tmp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (g_motion)
  DEBUG_ASSERT (g_motion->num_metadata)
  DEBUG_ASSERT (g_motion->metadata)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Allocate memory 
  marray = (gx3dMotionMetadata *) calloc (g_motion->num_metadata, sizeof(gx3dMotionMetadata));
  if (marray == 0)
    TERMINAL_ERROR ("Convert_Metadata_List_To_Array(): can't allocate memory for gx3dMotionMetadata array")
  // Copy list items into array (note this is not a deep copy - not needed)
  for (i=0,mp=g_motion->metadata; i<g_motion->num_metadata; i++,mp=mp->next)
    memcpy ((void *)(&marray[i]), (void *)mp, sizeof(gx3dMotionMetadata));
  // Delete the linked list
  for (mp=g_motion->metadata; mp; ) {
    tmp = mp;
    mp = mp->next;
    free (tmp);
  }
  // Attach array to gx3dMotion
  g_motion->metadata = marray;
}

/*____________________________________________________________________
|
| Function: LWS_File_To_GX3D_MotionSkeleton
|                       
| Input: Called from gx3d_ReadLWSFile()                                                                 
| Output: Convert an LWS file to a gx3dMotionSkeleton.  Returns true on 
|   success.
|___________________________________________________________________*/

void LWS_File_To_GX3D_MotionSkeleton  (
  char               *filename, 
  gx3dMotionSkeleton *g_skeleton )  // ptr to empty motion skeleton (all zeroed out)
{
  int i, j;
  lws_ObjectLayer *l_olayer;
  lws_Bone        *l_bone, *pbone;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (filename)
  DEBUG_ASSERT (g_skeleton)

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Read LWS data from file
//  l_olayer = lws_ReadFile (filename, 10, false);
  l_olayer = lws_ReadFile (filename);
  if (l_olayer == 0) 
    TERMINAL_ERROR ("LWS_File_To_GX3D_MotionSkeleton(): can't read LWS file")
  else {
    // Verify the lws objectlayer is compatible
    if (NOT Verify_LWS_ObjectLayer (l_olayer))
      TERMINAL_ERROR ("LWS_File_To_GX3D_MotionSkeleton(): LWS file not compatible")
    else {
      // Set number of bones
      g_skeleton->num_bones = l_olayer->num_bones;
      // Allocate memory for array of bones
      g_skeleton->bones = (gx3dMotionSkeletonBone *) calloc (g_skeleton->num_bones, sizeof(gx3dMotionSkeletonBone));
      if (g_skeleton->bones == 0)
        TERMINAL_ERROR ("LWS_File_To_GX3D_MotionSkeleton(): can't allocate memory for bones array")
      // Copy bone data (relies on bones in the lws objectlayer being in hierarcical order, starting with root bone first)
      for (i=0, l_bone=l_olayer->bones; i<g_skeleton->num_bones; i++, l_bone=l_bone->next) {
        // Make sure first bone in list is root bone
        if (i == 0) {
          if (l_bone->parent_id != -1)
            TERMINAL_ERROR ("LWS_File_To_GX3D_MotionSkeleton(): first bone in olayer list must be the root bone")
        }
        // Make sure all other bones are not root
        else {
          if (l_bone->parent_id == -1)
            TERMINAL_ERROR ("LWS_File_To_GX3D_MotionSkeleton(): found another root bone in olayer")
        }
        // Copy bone name
        Copy_Name (g_skeleton->bones[i].name, l_bone->name, gx_ASCIIZ_STRING_LENGTH_LONG);
        // Copy pre, post matrices
        g_skeleton->bones[i].pre  = l_bone->pre;
        g_skeleton->bones[i].post = l_bone->post;
      }
      // Set first bone in array as root (already verified this in above loop)
      g_skeleton->bones[0].parent = 0xFF;
      // Set parents of other bones in array
      for (i=1; i<g_skeleton->num_bones; i++) {
        // Find l_bone with same name as the skeleton bone
        for (l_bone=l_olayer->bones; l_bone; l_bone=l_bone->next) 
          if (!strcmp(g_skeleton->bones[i].name, l_bone->name))
            break;
        // Get parent of this l_bone
        pbone = Parent_Bone (l_olayer, l_bone);
        // Find same bone in skeleton bones array
        for (j=0; j<g_skeleton->num_bones; j++) 
          if (!strcmp(g_skeleton->bones[j].name, pbone->name)) {
            // This is the array index of the parent bone - save it
            g_skeleton->bones[i].parent = j;
            break;
          }
      }
    } 
    // Free memory for LWS objectlayer
    lws_FreeObjectLayer (l_olayer);
  }
}

/*____________________________________________________________________
|
| Function: Parent_Bone
|
| Input: Called from LWS_File_To_GX3D_Motion(), 
|                    LWS_File_To_GX3D_MotionSkeleton()
| Output: Returns parent bone of a bone.
|___________________________________________________________________*/

static lws_Bone *Parent_Bone (lws_ObjectLayer *olayer, lws_Bone *bone)
{
  lws_Bone *tbone, *parent_bone = 0;

	DEBUG_ASSERT (olayer)
	DEBUG_ASSERT (bone)

  for (tbone=olayer->bones; tbone; tbone=tbone->next)
    if (tbone->id == bone->parent_id) {
      parent_bone = tbone;
      break;
    }

  DEBUG_ASSERT (parent_bone)

  return (parent_bone);
}
