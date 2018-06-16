/*____________________________________________________________________
|
| File: gx3d_blendtree.cpp
|
| Description: Functions to manipulate a gx3dBlendTree.
|
| Functions:  gx3d_BlendTree_Init
|             gx3d_BlendTree_Free
|             gx3d_BlendTree_Add_Node
|             gx3d_BlendTree_Remove_Node
|             gx3d_BlendTree_Remove_All_Nodes
|             gx3d_BlendTree_Set_Output
|             gx3d_BlendTree_Update
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

/*____________________________________________________________________
|
| Function: gx3d_BlendTree_Init
| 
| Output: Creates an empty blend tree.  Returns pointer to blend tree 
|   or 0 on any error.
|___________________________________________________________________*/

gx3dBlendTree *gx3d_BlendTree_Init (gx3dMotionSkeleton *skeleton)
{
  gx3dBlendTree *blendtree = 0;
 
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
  blendtree = (gx3dBlendTree *) calloc (1, sizeof(gx3dBlendTree));
  if (blendtree == 0)
    TERMINAL_ERROR ("gx3d_BlendTree_Init(): can't allocate memory for blend tree");
  // Set skeleton
  blendtree->skeleton = skeleton;
  // Init local pose
  blendtree->local_pose  = gx3d_LocalPose_Init (skeleton);
  // Init global pose
  blendtree->global_pose = gx3d_GlobalPose_Init (skeleton);
  // Create array of palette indeces
  blendtree->target_matrix_palette_index = (int *) malloc (skeleton->num_bones * sizeof(int));
  if (blendtree->target_matrix_palette_index == 0)
    TERMINAL_ERROR ("gx3d_BlendTree_Init(): can't allocate memory for palette index array");

  return (blendtree);
}

/*____________________________________________________________________
|
| Function: gx3d_BlendTree_Free
| 
| Output: Frees memory for blend tree.  Doesn't free any nodes in the
|   tree.
|___________________________________________________________________*/

void gx3d_BlendTree_Free (gx3dBlendTree *blendtree)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendtree)
  DEBUG_ASSERT (blendtree->local_pose)
  DEBUG_ASSERT (blendtree->global_pose)
  DEBUG_ASSERT (blendtree->target_matrix_palette_index)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Free memory
  gx3d_LocalPose_Free (blendtree->local_pose);
  gx3d_GlobalPose_Free (blendtree->global_pose);
  free (blendtree->target_matrix_palette_index);

  // Free top-level struct
  free (blendtree);
}

/*____________________________________________________________________
|
| Function: gx3d_BlendTree_Add_Node
| 
| Output: Adds a node to the end of the blendtree (after the last node).
|   If no nodes in the tree, this node becomes the first node.
|___________________________________________________________________*/

void gx3d_BlendTree_Add_Node (gx3dBlendTree *blendtree, gx3dBlendNode *blendnode)
{
  gx3dBlendNode **npp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendtree)
  DEBUG_ASSERT (blendnode)
  DEBUG_ASSERT (blendtree->skeleton)
  DEBUG_ASSERT (blendnode->skeleton)
  DEBUG_ASSERT (blendtree->skeleton == blendnode->skeleton)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (npp=&(blendtree->nodes); *npp; npp=&((*npp)->next));
  *npp = blendnode;
  blendnode->next = 0;
}

/*____________________________________________________________________
|
| Function: gx3d_BlendTree_Remove_Node
| 
| Output: Removes a node from the tree. Doesn't free memory for the 
|   removed node.
|___________________________________________________________________*/

void gx3d_BlendTree_Remove_Node (gx3dBlendTree *blendtree, gx3dBlendNode *blendnode)
{
  gx3dBlendNode **npp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendtree)
  DEBUG_ASSERT (blendnode)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (npp=&(blendtree->nodes); *npp; npp=&((*npp)->next))
    if (*npp == blendnode)
      break;
  if (*npp) 
    *npp = (*npp)->next;
}

/*____________________________________________________________________
|
| Function: gx3d_BlendTree_Remove_All_Node
| 
| Output: Removes all nodes, if any, from the tree. Doesn't free memory
|   for the removed nodes.
|___________________________________________________________________*/

void gx3d_BlendTree_Remove_All_Nodes (gx3dBlendTree *blendtree)
{
  gx3dBlendNode *np;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendtree)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Cancel the outputs of all nodes in the tree
  for (np=blendtree->nodes; np; np=np->next) 
    // Cancel the output of this node
    np->output_local_pose = 0;
  // Set the node list to empty
  blendtree->nodes = 0;
}

/*____________________________________________________________________
|
| Function: gx3d_BlendTree_Set_Output
| 
| Output: Sets output of tree global pose to an object.  Call with 
|   objectlayer = 0 to disable.
|___________________________________________________________________*/

void gx3d_BlendTree_Set_Output (gx3dBlendTree *blendtree, gx3dObjectLayer *objectlayer)
{
  int i, j;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendtree)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Enable output?
  if (objectlayer) {
    // Point the output of the tree to this objectlayer
    blendtree->target_objectlayer = objectlayer;
    // Assume all skeleton bones do not correspond with objectlayer bones (weightmaps), at least not yet
    for (i=0; i<blendtree->skeleton->num_bones; i++)
      blendtree->target_matrix_palette_index[i] = -1;  // invalid array index
    // Now look at each skeleton bone
    for (i=0; i<blendtree->skeleton->num_bones; i++)
      // Does it have a corresponding (same name) weightmap in the objectlayer?
      for (j=0; j<objectlayer->num_matrix_palette; j++)
        if (!strcmp(blendtree->skeleton->bones[i].name, objectlayer->matrix_palette[j].weightmap_name)) {
          // Connect this skeleton bone with the objectlayer palette matrix
          blendtree->target_matrix_palette_index[i] = j;
          break;
        }
  }
  // Disable it
  else
    blendtree->target_objectlayer = 0;
}

/*____________________________________________________________________
|
| Function: gx3d_BlendTree_Update
| 
| Output: Calles Update() on all nodes, in order they are listed in 
|   the linked list of nodes.  Optionally, returns final position of
|   root bone.
|___________________________________________________________________*/

void gx3d_BlendTree_Update (gx3dBlendTree *blendtree, gx3dVector *new_position)
{
  int i, parent, index;
  gx3dMatrix m, mt;
  gx3dBlendNode *np;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (blendtree)

/*____________________________________________________________________
|
| Update each blendnode in the tree
|___________________________________________________________________*/

  // Update all nodes
  for (np=blendtree->nodes; np; np=np->next) {
    // If this is the last node, redirect it's output to the tree local pose
    if (np->next == 0)
      np->output_local_pose = blendtree->local_pose;
    // Update this blendnode
    gx3d_BlendNode_Update (np);
  }

/*____________________________________________________________________
|
| Update each global bone pose
|___________________________________________________________________*/

  // Output to global pose - convert each local bone pose into global poses (matrices)
  for (i=0; i<blendtree->skeleton->num_bones; i++) {
    // Build a matrix from the quaternion
    gx3d_GetQuaternionMatrix (&(blendtree->local_pose->bone_pose[i].q), &m);
    gx3d_MultiplyMatrix (&(blendtree->skeleton->bones[i].pre), &m, &m);
    gx3d_MultiplyMatrix (&m, &(blendtree->skeleton->bones[i].post), &m);
    // If root bone, translate also
    if (blendtree->skeleton->bones[i].parent == 0xFF) {
      gx3d_GetTranslateMatrix (&mt, blendtree->local_pose->root_translate.x, blendtree->local_pose->root_translate.y, blendtree->local_pose->root_translate.z); 
      gx3d_MultiplyMatrix (&m, &mt, &m);
      // Send root bone position back to caller?
      if (new_position)
        *new_position = blendtree->local_pose->root_translate;
    }
    blendtree->global_pose->bone_pose[i].transform.local_matrix = m;
  }

/*____________________________________________________________________
|
| Walk the tree and update all composite matrices
|___________________________________________________________________*/

  for (i=0; i<blendtree->skeleton->num_bones; i++) {
    parent = blendtree->skeleton->bones[i].parent;
    if (parent == 0xFF)
      blendtree->global_pose->bone_pose[i].transform.composite_matrix = blendtree->global_pose->bone_pose[i].transform.local_matrix;
    else 
      // Composite matrix = local * parent matrix
      gx3d_MultiplyMatrix (&(blendtree->global_pose->bone_pose[i].transform.local_matrix),
                           &(blendtree->global_pose->bone_pose[parent].transform.composite_matrix),
                           &(blendtree->global_pose->bone_pose[i].transform.composite_matrix));
  }

/*____________________________________________________________________
|
| Output to target objectlayer, if any
|___________________________________________________________________*/

  if (blendtree->target_objectlayer) 
    for (i=0; i<blendtree->skeleton->num_bones; i++) {
      index = blendtree->target_matrix_palette_index[i];
      if (index != -1)
        blendtree->target_objectlayer->matrix_palette[index].m = blendtree->global_pose->bone_pose[i].transform.composite_matrix;
    }
}
