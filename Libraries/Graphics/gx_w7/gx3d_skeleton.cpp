/*____________________________________________________________________
|
| File: gx3d_skeleton.cpp
|
| Description: Functions to manipulate a 3D skeleton.
|
| Functions:  gx3d_Skeleton_Init
|             gx3d_Skeleton_AddBone	
|							 Count_Layers_Using_Bone
|							 Find_Layers_Using_Bone
|              Get_Parent_GX3D_Bone	
|             gx3d_Skeleton_Free					
|              Free_Bone		
|             gx3d_Skeleton_Copy
|              Copy_Bone				
|               Copy_SubBone		
|
|             gx3d_Skeleton_GetBone						
|              Get_Bone_With_Name							
|             gx3d_Skeleton_SetMatrix				  
|             gx3d_Skeleton_SetBoneMatrix		
|             gx3d_Skeleton_UpdateTransforms	
|              Update_Bone_Transforms
|							gx3d_Skeleton_Attach						
|							gx3d_Skeleton_Detach
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

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <math.h>

#include "dp.h"

/*___________________
|
| Function prototypes
|__________________*/

static void							 Count_Layers_Using_Bone (gx3dObjectLayer *layer, char *name, int *n);
static void							 Find_Layers_Using_Bone (gx3dObjectLayer *layer, char *name, gx3dMatrix **marray, int *n);
static gx3dSkeletonBone *Get_Parent_GX3D_Bone (gx3dSkeletonBone *bone, int parent_end_point);
static void							 Free_Bone (gx3dSkeletonBone *bone);
static void							 Copy_Bone (gx3dSkeletonBone *src_bone, gx3dSkeletonBone **dst_bone);
static void							 Copy_SubBone (gx3dSkeletonBone *src_bone, gx3dSkeletonBone **dst_bone);
static gx3dSkeletonBone *Get_Bone_With_Name (gx3dSkeletonBone *bone, char *name);
static void Update_Bone_Transforms (
  gx3dSkeleton		 *skel, 
  gx3dSkeletonBone *bone, 
  gx3dMatrix			 *parent_matrix, 
  bool							parent_transform_dirty );

/*___________________
|
| Macros
|__________________*/

#define SKELETON_TRANSFORM  object->skeleton->root_transform
#define BONE_TRANSFORM      bone->transform

/*____________________________________________________________________
|
| Function: gx3d_Skeleton_Init
| 
| Output: Creates a skeleton and returns a pointer to it.  The skeleton
|   will not have any bones.  The caller should add bones to the skeleton
|   by calling gx3d_Skeleton_AddBone
|___________________________________________________________________*/

gx3dSkeleton *gx3d_Skeleton_Init (
  int         num_vertices,
  gx3dVector *vertices,       // array of vertices
  int         origin_point,   // origin of root of skeleton (index into vertices array)
  int         num_bones )
{                      
	gx3dSkeleton *skel;
  bool error = false;
    
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (num_vertices >= 1)
  DEBUG_ASSERT (vertices)
  DEBUG_ASSERT ((origin_point >= 0) AND (origin_point < num_vertices))
  DEBUG_ASSERT (num_bones >= 1)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  skel = (gx3dSkeleton *) calloc (1, sizeof(gx3dSkeleton));
  if (skel == 0)
    error = true;
  else {
    // Set members
    skel->origin_point = origin_point;
    skel->num_vertices = num_vertices;
    skel->num_bones    = num_bones;
    // Allocate memory for array of vertices
    skel->vertex = (gx3dVector *) calloc (num_vertices, sizeof(gx3dVector));
    if (skel->vertex == 0)
      error = true;
		if (NOT error) {
      // Copy array of vertices?
      memcpy ((void *)(skel->vertex), (void *)vertices, num_vertices * sizeof(gx3dVector));
      // Init root transform to identity
      gx3d_GetIdentityMatrix (&(skel->root_transform.local_matrix));
      gx3d_GetIdentityMatrix (&(skel->root_transform.composite_matrix));
    }
  }

/*____________________________________________________________________
|
| On any error, free memory
|___________________________________________________________________*/

  if (error) {
    if (skel) {
      gx3d_Skeleton_Free (skel);
      skel = 0;
    }
  }

	return (skel);
}

/*____________________________________________________________________
|
| Function: gx3d_Skeleton_AddBone
| 
| Output: Adds a bone to a skeleton.  Can't add more bones that skeleton
|   has previously been defined as having (defined in gx3d_Skeleton_Init()).
|___________________________________________________________________*/

void gx3d_Skeleton_AddBone (
  gx3dObject	*object,
  char        *name,          // name of bone (should be unique)
  gx3dVector  *pivot,         // bone pivot point (relative to the local coordinate origin)
  gx3dVector  *direction,     // normalized direction bone begins pointing
  int          start_point,		// indexes into skeleton vertex array - used to build hierarchy
  int          end_point )
{
	int n;
	gx3dSkeletonBone *bone, *tbone, **bonepp;
  bool error = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object)
	DEBUG_ASSERT (object->skeleton)
  DEBUG_ASSERT (name)
  DEBUG_ASSERT (pivot)
  DEBUG_ASSERT (direction)
  DEBUG_ASSERT ((start_point >= 0) AND (start_point < object->skeleton->num_vertices))
  DEBUG_ASSERT ((end_point   >= 0) AND (end_point   < object->skeleton->num_vertices))

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  bone = (gx3dSkeletonBone *) calloc (1, sizeof(gx3dSkeletonBone));
  if (bone == 0) {
    gxError ("gx3d_Skeleton_AddBone(): Error allocating memory for gx3dSkeletonBone");
    error = true;
  }
  else {
    // Allocate memory for bone name
    bone->name = (char *) calloc (strlen(name)+1, sizeof(char));
    if (bone->name == 0) {
      gxError ("gx3d_Skeleton_AddBone(): Error allocating memory for gx3dSkeletonBone name");
      error = true;
    }
    else {
      // Copy name of bone
      strcpy (bone->name, name);
      // Set pivot point for bone
      bone->pivot = *pivot;
      // Set direction bone is pointing
      bone->direction = *direction;
      // Set start, end point
      bone->start_point = start_point;
      bone->end_point   = end_point;
      // Init transform
      gx3d_GetIdentityMatrix (&(bone->transform.local_matrix));
      gx3d_GetIdentityMatrix (&(bone->transform.composite_matrix));
			// Count number of object layers that use this bone (have a weightmap with same name as bone)
			n = 0;
			Count_Layers_Using_Bone (object->layer, bone->name, &n);
			// Allocate an array of pointers to nonlocal matrices (in the layers that use the bone)
			if (n) {
				bone->nonlocal_matrices = (gx3dMatrix **) malloc (n * sizeof(gx3dMatrix *));
				if (bone->nonlocal_matrices == 0)
					DEBUG_ERROR ("gx3d_Skeleton_AddBone(): Error allocating memory for array of pointers to nonlocal matrices")
				else {
					bone->num_nonlocal_matrices = n;
					// Get pointers to nonlocal matrices in layers that use this bone
					n = 0;
					Find_Layers_Using_Bone (object->layer, bone->name, bone->nonlocal_matrices, &n);
				}
			}
			
      // Add the new bone to the start of an empty bone list
      if (object->skeleton->bones == 0) 
        object->skeleton->bones = bone;
      // Find a place to put it in a non-empty bone list hierarchy
      else {
        bonepp = NULL;
        // If the new bone has no parent (attached to root), put it at the end of the first level of bones
        if (bone->start_point == object->skeleton->bones->start_point)
          bonepp=&(object->skeleton->bones);
        // If the new bone has a parent, put it at the end of the parent's child level of bones
        else {
          tbone = Get_Parent_GX3D_Bone (object->skeleton->bones, bone->start_point);
          if (tbone) {
            bonepp=&(tbone->child);
          }
        }
        // Error checking
        if (bonepp == 0) {
          gxError ("gx3d_Skeleton_AddBone(): Error can't find parent bone in gx3dSkeleton bone hierarchy");
          error = true;
        }
        // Put the new bone at the end of this level of bones
        else {
          for (; *bonepp; bonepp=&((*bonepp)->next));
            *bonepp = bone;
        }
      }
    }
  }

/*____________________________________________________________________
|
| On any error, free memory
|___________________________________________________________________*/

  if (error) {
    if (bone) {
      Free_Bone (bone);
      bone = 0;
    }
  }
}

/*____________________________________________________________________
|
| Function: Count_Layers_Using_Bone
| 
| Input: Called from gx3d_Skeleton_AddBone()
| Output: Counts the number of layers that have weightmaps with name.
|___________________________________________________________________*/

static void Count_Layers_Using_Bone (gx3dObjectLayer *layer, char *name, int *n)
{
	int i;

	// Look at all layers
	for (; layer; layer=layer->next) {
		// Look at all entries in matrix palette, if any
		for (i=0; i<layer->num_matrix_palette; i++) 
			// Does this entry have the same weightmap name as the bone?
			if (!strcmp (layer->matrix_palette[i].weightmap_name, name)) {
				(*n)++;
				break;
			}
		// Look at child layers
		if (layer->child)
			Count_Layers_Using_Bone (layer->child, name, n);
  }
}

/*____________________________________________________________________
|
| Function: Find_Layers_Using_Bone
| 
| Input: Called from gx3d_Skeleton_AddBone()
| Output: Finds the layers that have weightmaps with name and copies a
|		pointer to the layers matrix palette entry with that weightmap name
|		to the callers array.
|___________________________________________________________________*/

static void Find_Layers_Using_Bone (gx3dObjectLayer *layer, char *name, gx3dMatrix **marray, int *n)
{
	int i;

	// Look at all layers
	for (; layer; layer=layer->next) {
		// Look at all entries in matrix palette, if any
		for (i=0; i<layer->num_matrix_palette; i++) 
			// Does this entry have the same weightmap name as the bone?
			if (!strcmp (layer->matrix_palette[i].weightmap_name, name)) {
				marray[*n] = &(layer->matrix_palette[i].m);
				(*n)++;
				break;
			}
		// Look at child layers
		if (layer->child)
			Count_Layers_Using_Bone (layer->child, name, n);
  }
}

/*____________________________________________________________________
|
| Function: Get_Parent_GX3D_Bone
|                       
| Input: Called from gx3d_Skeleton_AddBone()                                                                 
| Output: Returns the parent bone of the bone.  The parent's end point 
|   will be the same as the new bones start point.
|___________________________________________________________________*/

static gx3dSkeletonBone *Get_Parent_GX3D_Bone (gx3dSkeletonBone *bone, int parent_end_point)
{
  gx3dSkeletonBone *parent_bone = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (bone)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Is the input bone the parent?
  if (bone->end_point == parent_end_point)
    parent_bone = bone;

  // If not found, search child bones, if any
  if (parent_bone == NULL) 
    if (bone->child)
      parent_bone = Get_Parent_GX3D_Bone (bone->child, parent_end_point);

  // If not found, search the rest of the bones on this level
  if (parent_bone == NULL)
    if (bone->next)
      parent_bone = Get_Parent_GX3D_Bone (bone->next, parent_end_point);

  return (parent_bone);
}

/*____________________________________________________________________
|
| Function: gx3d_Skeleton_Free
| 
| Output: Frees memory for a skeleton.
|___________________________________________________________________*/

void gx3d_Skeleton_Free (gx3dSkeleton *skel)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skel)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Free all bones
  Free_Bone (skel->bones);
  // Free vertex array
  if (skel->vertex)
    free (skel->vertex);
  // Free the skeleton
  free (skel);
}

/*____________________________________________________________________
|
| Function: Free_Bone
|
| Input: Called from gx3d_Skeleton_AddBone(), gx3d_Skeleton_Free()
| Output: Frees all memory associated with a bone including linked bones
|   and child bones.
|___________________________________________________________________*/

static void Free_Bone (gx3dSkeletonBone *bone)
{
  gx3dSkeletonBone *tbone;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (bone)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  while (bone) {
    // Free child bone/s first
    if (bone->child)
      Free_Bone (bone->child);

    // Free any memory used by this bone
    if (bone->name)
      free (bone->name);
		if (bone->nonlocal_matrices)
			free (bone->nonlocal_matrices);

    // Goto next bone in the list
    tbone = bone->next;
    free (bone);
    bone = tbone;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Skeleton_Copy
| 
| Output: Makes a copy of skeleton and returns a pointer to it.
|___________________________________________________________________*/

gx3dSkeleton *gx3d_Skeleton_Copy (gx3dSkeleton *skel)
{
  gx3dSkeleton *copy;
  bool error = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skel)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create skeleton copy
  copy = gx3d_Skeleton_Init (skel->num_vertices, skel->vertex, skel->origin_point, skel->num_bones);
  if (copy == 0)
    error = true;
  else {
    // Copy root transform
    memcpy ((void *)&(copy->root_transform), (void *)&(skel->root_transform), sizeof(gx3dTransform));
    // Copy bones
    Copy_Bone (skel->bones, &(copy->bones));
  }

/*____________________________________________________________________
|
| On any error, free memory
|___________________________________________________________________*/

  if (error) {
    if (copy) {
      gx3d_Skeleton_Free (copy);
      copy = 0;
    }
  }

  return (copy);
}

/*____________________________________________________________________
|
| Function: Copy_Bone
|
| Input: Called from gx3d_Skeleton_Copy()
| Output: Copies a bone from source object to destination object 
|     including linked bones and child bones.  Creates the destination 
|     bone.
|___________________________________________________________________*/

static void Copy_Bone (gx3dSkeletonBone *src_bone, gx3dSkeletonBone **dst_bone)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (src_bone)
  DEBUG_ASSERT (dst_bone)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; src_bone; src_bone=src_bone->next, dst_bone=&(*dst_bone)->next) {
    // Process child bone/s first
    if (src_bone->child)
      Copy_Bone (src_bone->child, &(*dst_bone)->child);
    // Process this sub bone
    Copy_SubBone (src_bone, dst_bone);
  }
}

/*____________________________________________________________________
|
| Function: Copy_SubBone
|
| Input: Called from Copy_Bone()
| Output: Copies a bone from source object to destination object.  
|   Creates the destination bone.
|___________________________________________________________________*/

static void Copy_SubBone (gx3dSkeletonBone *src_bone, gx3dSkeletonBone **dst_bone)
{
  bool error = false;
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (src_bone)
  DEBUG_ASSERT (dst_bone)

/*____________________________________________________________________
|
| Create an empty destination bone
|___________________________________________________________________*/

  *dst_bone = (gx3dSkeletonBone *) calloc (1, sizeof(gx3dSkeletonBone));
  if (*dst_bone == 0)
    error = true;

/*____________________________________________________________________
|
| Copy contents of source bone to destination bone
|___________________________________________________________________*/

  if (NOT error) {
    // Copy data
    (*dst_bone)->pivot                 = src_bone->pivot;
    (*dst_bone)->direction             = src_bone->direction;
    (*dst_bone)->start_point           = src_bone->start_point;
    (*dst_bone)->end_point             = src_bone->end_point;
    (*dst_bone)->transform             = src_bone->transform;
		(*dst_bone)->num_nonlocal_matrices = src_bone->num_nonlocal_matrices;
    // Copy bone name, if any
    if (src_bone->name) {
      (*dst_bone)->name = (char *) malloc (strlen(src_bone->name)+1);
      if ((*dst_bone)->name == 0)
        error = true;
      else
        strcpy ((*dst_bone)->name, src_bone->name);
    }
		// Copy array of pointers to nonlocal matrices, if any
		if (src_bone->nonlocal_matrices) {
			(*dst_bone)->nonlocal_matrices = (gx3dMatrix **) malloc (src_bone->num_nonlocal_matrices * sizeof(gx3dMatrix *));
			if ((*dst_bone)->nonlocal_matrices == 0)
				error = true;
			else
				memcpy ((*dst_bone)->nonlocal_matrices, src_bone->nonlocal_matrices, src_bone->num_nonlocal_matrices * sizeof(gx3dMatrix *));
		}
  }

/*____________________________________________________________________
|
| On any error, free all memory for bone copy
|___________________________________________________________________*/

  if (error) {
    Free_Bone (*dst_bone);
    *dst_bone = 0;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Skeleton_GetBone
|                       
| Output: Returns the first gx3d bone that has the name or NULL if not 
|   found.  
|___________________________________________________________________*/

gx3dSkeletonBone *gx3d_Skeleton_GetBone (gx3dObject *object, char *name)
{
  gx3dSkeletonBone *bone = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object)
	DEBUG_ASSERT (object->skeleton)
  DEBUG_ASSERT (name)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  bone = Get_Bone_With_Name (object->skeleton->bones, name);

  return (bone);
}

/*____________________________________________________________________
|
| Function: Get_Bone_With_Name
|                     
| Input: Called from gx3d_Skeleton_GetBone()  
| Output: Returns the first gx3d bone that has the name or NULL if not 
|   found.  
|___________________________________________________________________*/

static gx3dSkeletonBone *Get_Bone_With_Name (gx3dSkeletonBone *bone, char *name)
{
  gx3dSkeletonBone *the_bone = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (bone)
  DEBUG_ASSERT (name)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (bone) {
    // Is the input bone the one?
    if (bone->name)
      if (!strcmp(bone->name, name))
        the_bone = bone;

    // If not found, search child bones, if any
    if (the_bone == 0) 
      if (bone->child)
        the_bone = Get_Bone_With_Name (bone->child, name);

    // If not found, search the rest of the bones on this level
    if (the_bone == 0)
      if (bone->next)
        the_bone = Get_Bone_With_Name (bone->next, name);
  }

  return (the_bone);
}

/*____________________________________________________________________
|
| Function: gx3d_Skeleton_SetMatrix
|                       
| Output: Sets the local transform matrix for a skeleton.
|___________________________________________________________________*/

void gx3d_Skeleton_SetMatrix (gx3dObject *object, gx3dMatrix *m)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

	DEBUG_ASSERT (object)
  DEBUG_ASSERT (object->skeleton)
  DEBUG_ASSERT (m)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Is the new matrix different from the current local matrix?
  if (memcmp ((void *)&(SKELETON_TRANSFORM.local_matrix), (void *)m, sizeof(gx3dMatrix)) != 0) {
    // Set new local matrix
    memcpy ((void *)&(SKELETON_TRANSFORM.local_matrix), (void *)m, sizeof(gx3dMatrix));
    SKELETON_TRANSFORM.dirty = true;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Skeleton_SetBoneMatrix
|                       
| Output: Sets the local transform matrix for a bone.
|___________________________________________________________________*/

void gx3d_Skeleton_SetBoneMatrix (gx3dSkeletonBone *bone, gx3dMatrix *m)
{
  gx3dMatrix m1, m2;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (bone)
  DEBUG_ASSERT (m)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (bone)
    // Is the new matrix different from the current local matrix?
    if (memcmp ((void *)&(BONE_TRANSFORM.local_matrix), (void *)m, sizeof(gx3dMatrix)) != 0) {
      // Set new local matrix
      gx3d_GetTranslateMatrix (&m1, -bone->pivot.x, -bone->pivot.y, -bone->pivot.z);
      gx3d_GetTranslateMatrix (&m2,  bone->pivot.x,  bone->pivot.y,  bone->pivot.z);
      gx3d_MultiplyMatrix (&m1, m, &m1);
      gx3d_MultiplyMatrix (&m1, &m2, &(BONE_TRANSFORM.local_matrix));
      BONE_TRANSFORM.dirty = true;
    }
}

/*____________________________________________________________________
|
| Function: gx3d_Skeleton_UpdateTransforms
|
| Output: Updates all skeleton and attached bone tranforms.
|___________________________________________________________________*/

void gx3d_Skeleton_UpdateTransforms (gx3dObject *object)
{
  DEBUG_ASSERT (object)
	DEBUG_ASSERT (object->skeleton)

  Update_Bone_Transforms (object->skeleton, 
													object->skeleton->bones, 
													&(object->skeleton->root_transform.local_matrix), 
													object->skeleton->root_transform.dirty);
  object->skeleton->root_transform.dirty = false;
}

/*____________________________________________________________________
|
| Function: Update_Bone_Transforms
|
| Input: Called from gx3d_Skeleton_UpdateTransforms()
| Output: Updates a bone transform including linked bones and child
|   bones.
|___________________________________________________________________*/

static void Update_Bone_Transforms (
  gx3dSkeleton		 *skel, 
  gx3dSkeletonBone *bone, 
  gx3dMatrix			 *parent_matrix, 
  bool							parent_transform_dirty )
{
	int i;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (skel)
  DEBUG_ASSERT (bone)
  DEBUG_ASSERT (parent_matrix)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; bone; bone=bone->next) {
    // Update bone transform?
    if (bone->transform.dirty OR parent_transform_dirty) {
      // Composite matrix = local matrix * parent matrix
      gx3d_MultiplyMatrix (&(bone->transform.local_matrix), parent_matrix, &(bone->transform.composite_matrix));
      bone->transform.dirty = true;
			// Is the skeleton attached to the object?
			if (skel->attached) 
				// Copy bone matrix to each of the layer's matrix palette, if any
				for (i=0; i<bone->num_nonlocal_matrices; i++)
					*(bone->nonlocal_matrices[i]) = bone->transform.composite_matrix;
    }
    // Update child bones transforms
    if (bone->child)
      Update_Bone_Transforms (skel, bone->child, &(bone->transform.composite_matrix), bone->transform.dirty);
    // Clear local transform changes
    bone->transform.dirty = true;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Skeleton_Attach
| 
| Output: Attaches skeleton to an object.
|___________________________________________________________________*/

void gx3d_Skeleton_Attach (gx3dObject *object)
{
  DEBUG_ASSERT (object)
	DEBUG_ASSERT (object->skeleton)

	object->skeleton->attached = true;
}

/*____________________________________________________________________
|
| Function: gx3d_Skeleton_Detach
| 
| Output: Detaches a motion from any object.
|___________________________________________________________________*/

void gx3d_Skeleton_Detach (gx3dObject *object)
{
  DEBUG_ASSERT (object)
	DEBUG_ASSERT (object->skeleton)

	object->skeleton->attached = false;
}
