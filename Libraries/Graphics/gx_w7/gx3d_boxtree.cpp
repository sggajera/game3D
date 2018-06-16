/*____________________________________________________________________
|
| File: gx3d_boxtree.cpp
|
| Description: Functions to manipulate gx3dBoxtree.
|
| Functions: gx3d_Boxtree_Init        
|             Init_Static_Boxtree
|              Get_Static_Geometry
|              Get_Static_Bound_Box        
|              Make_Static_Tree
|               Subdivide_Static_Subtree
|                Make_Node
|                Get_Best_Split
|                Shrink_To_Fit
|             Init_Dynamic_Boxtree        // not done
|              Get_Dynamic_Geometry       // not done
|              Make_Dynamic_Tree          // not done
|            gx3d_Boxtree_Free
|             Free_Subtree
|            gx3d_Boxtree_SetDirty
|            gx3d_Boxtree_Update
|             Update_Static_BoxTree
|             Update_Dynamic_BoxTree      // not done
|            gx3d_Boxtree_Intersect_Ray
|             Subtree_Intersect_Ray
|
| Description: A boxtree is an AABB hierarchy used for collision 
|   detection that is similar to a BSP tree.  Geometry is split
|   on axis-aligned planes, not necessarily evenly.
|
|   Boxtrees are created using model space coordinates and refer to
|   exactly one gx3dObject.
|
|   There are two types of boxtrees - dynamic and static.
|   For geometry that changes by using transforms, (calling 
|   gx3d_SetObjectMatrix() or gx3d_SetObjectLayerMatrix(), a dynamic
|   boxtree should be used.  Otherwise a static boxtree should be used
|   (when all object and object layers transform matrices are the identity
|   matrix).
|
|   A static boxtree has references to its object's data and so uses less
|   memory.  A dynamic boxtree has copies of vertices and so uses more
|   memory.
|
|   The boxtree update function is useful if the actual geometry of a 
|   model changes.  For example if gx3d_TransformObject() or
|   gx3d_TransformObjectLayer() were called on an object.  In this case
|   the boxtree needs to be recomputed from scratch (either static or
|   dynamic).
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|
| DEBUG_ASSERTED!
|___________________________________________________________________*/

//#define DEBUG

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <math.h>

#include "dp.h"

/*___________________
|
| Constants
|__________________*/

#define X_AXIS  0
#define Y_AXIS  1
#define Z_AXIS  2

#define MAX_LEVEL 8 // don't subdivide past this level (root is level 1)

/*___________________
|
| Function Prototypes
|__________________*/

static gx3dBoxtree *Init_Static_Boxtree  (gx3dObject *object);
static void         Get_Static_Geometry  (gx3dObjectLayer *layer, gx3dBoxtree *boxtree);
static void         Get_Static_Bound_Box (gx3dBoxtree *boxtree);
static bool         Make_Static_Tree     (gx3dBoxtree *boxtree);
static bool Subdivide_Static_Subtree (
  gx3dBoxtree     *boxtree, 
  gx3dBoxtreeNode *subtree, 
  int              level ); // level at subtree (root is level 1)
static gx3dBoxtreeNode *Make_Node (int num_polys);
static float Get_Best_Split (gx3dBoxtree *boxtree, gx3dBoxtreeNode *node, char axis);
static bool Shrink_To_Fit (gx3dBoxtreeNode *node, int current_size);
static gx3dBoxtree *Init_Dynamic_Boxtree (gx3dObject *object);
static void         Get_Dynamic_Geometry (gx3dObjectLayer *layer, gx3dBoxtree *boxtree, int *n);
static bool         Make_Dynamic_Tree    (gx3dBoxtree *boxtree);
static void         Free_Subtree         (gx3dBoxtreeNode *subtree);
static void         Update_Static_Boxtree  (gx3dBoxtree *boxtree);
static void         Update_Dynamic_Boxtree (gx3dBoxtree *boxtree);
static void Subtree_Intersect_Ray (
  gx3dBoxtree     *boxtree,
  gx3dBoxtreeNode *subtree,         
  gx3dRay         *ray,
  float            ray_length,     
  bool            *intersection_found,
  gx3dVector      *intersection,
  float           *closest_distance,
  char           **name );

/*____________________________________________________________________
|
| Function: gx3d_Boxtree_Init
| 
| Output: Creates a boxtree for an object.
|___________________________________________________________________*/

gx3dBoxtree *gx3d_Boxtree_Init (gx3dObject *object, gx3dBoxtreeType type)
{
  gx3dBoxtree *boxtree = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);
  DEBUG_ASSERT ((type == gx3d_BOXTREE_TYPE_STATIC) OR (type == gx3d_BOXTREE_TYPE_DYNAMIC));

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  switch (type) {
    case gx3d_BOXTREE_TYPE_STATIC:
      boxtree = Init_Static_Boxtree (object);
      break;
    case gx3d_BOXTREE_TYPE_DYNAMIC:
      boxtree = Init_Dynamic_Boxtree (object);
      break;
  }      

  return (boxtree);
}

/*____________________________________________________________________
|
| Function: Init_Static_Boxtree
| 
| Input: Called from gx3d_Boxtree_Init()
| Output: Creates a static boxtree of an object.
|___________________________________________________________________*/

static gx3dBoxtree *Init_Static_Boxtree (gx3dObject *object)
{
  int num_vertices, num_polygons;
  bool error;
  gx3dBoxtree *boxtree = 0;

  error = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Allocate memory 
|___________________________________________________________________*/

  boxtree = (gx3dBoxtree *) calloc (1, sizeof(gx3dBoxtree));
  if (boxtree == 0)
    error = true;

/*____________________________________________________________________
|
| Build arrays of object geometry
|___________________________________________________________________*/

  if (NOT error) {
    // Init boxtree members
    boxtree->type = gx3d_BOXTREE_TYPE_STATIC;
/***** I want to get rid of this dependency! ************/
//    boxtree->object = object;
/********************************************************/
    // Count total number of vertices in the object (from all layers)
    gx3d_GetObjectInfo (object, 0, &num_vertices, &num_polygons);
    // Allocate arrays of pointers to object data 
    boxtree->poly_layer = (gx3dObjectLayer **) calloc (num_polygons, sizeof(gx3dObjectLayer *));
    if (boxtree->poly_layer == 0)
      error = true;
    boxtree->poly_box = (gx3dBox *) calloc (num_polygons, sizeof(gx3dBox));
    if (boxtree->poly_box == 0)
      error = true;
    boxtree->poly_box_center = (gx3dVector *) calloc (num_polygons, sizeof(gx3dVector));
    if (boxtree->poly_box_center == 0)
      error = true;
    boxtree->poly = (gx3dPolygon *) calloc (num_polygons, sizeof(gx3dPolygon));
    if (boxtree->poly == 0)
      error = true;
    // Get pointers to objects vertices
    boxtree->s.vertex = (gx3dVector **) calloc (num_vertices, sizeof(gx3dVector *));
    if (boxtree->s.vertex == 0)
      error = true;
  }

/*____________________________________________________________________
|
| Init data
|___________________________________________________________________*/
  
  if (NOT error) {
    // Make sure this object has a layer
    if (object->layer) {
      // Fill arrays with data, starting with first layer in object
      Get_Static_Geometry (object->layer, boxtree);
      // Compute bound boxes
      Get_Static_Bound_Box (boxtree);
      // Make bsp tree
      if (NOT Make_Static_Tree (boxtree))
        error = true;
    }
  }

/*____________________________________________________________________
|
| On any error, free memory
|___________________________________________________________________*/

  if (error) {
    gx3d_Boxtree_Free (boxtree);
    boxtree = 0;
  }

  return (boxtree);
}

/*____________________________________________________________________
|
| Function: Get_Static_Geometry
|
| Input: Called from Init_Static_Boxtree()
| Output: Goes through object layer (and child and sibling layers) and 
|   fills boxtree arrays.
|___________________________________________________________________*/

static void Get_Static_Geometry (gx3dObjectLayer *layer, gx3dBoxtree *boxtree)
{
  int i;
  bool first = true;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (boxtree);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; layer; layer=layer->next) {
    // Process child layer/s first
    if (layer->child)
      Get_Static_Geometry (layer->child, boxtree);
    // Process this layer
    for (i=0; i<layer->num_polygons; i++) {
      boxtree->poly_layer[boxtree->num_polygons] = layer;
      boxtree->poly      [boxtree->num_polygons].index[0] = layer->polygon[i].index[0] + (word)(boxtree->num_vertices);
      boxtree->poly      [boxtree->num_polygons].index[1] = layer->polygon[i].index[1] + (word)(boxtree->num_vertices);
      boxtree->poly      [boxtree->num_polygons].index[2] = layer->polygon[i].index[2] + (word)(boxtree->num_vertices);
      boxtree->num_polygons++;
    }
    for (i=0; i<layer->num_vertices; i++) 
      boxtree->s.vertex[boxtree->num_vertices++] = &(layer->vertex[i]);
  }
}

/*____________________________________________________________________
|
| Function: Get_Static_Bound_Box
|
| Input: Called from Init_Static_Boxtree()
| Output: Goes through object layer (and child and sibling layers) and 
|   computes all bounding boxes (each triangle and overall box for the
|   boxtree).
|___________________________________________________________________*/

static void Get_Static_Bound_Box (gx3dBoxtree *boxtree)
{
  int i;
  gx3dVector triangle[3];

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Make sure this boxtree has some polygons
  if (boxtree->num_polygons) {
    // Make overall box - tightest fit possible
    gx3d_GetBoundBox (&(boxtree->box), boxtree->s.vertex, boxtree->num_vertices);
    // Make box for each polygon
    for (i=0; i<boxtree->num_polygons; i++) {
      triangle[0] = *(boxtree->s.vertex[boxtree->poly[i].index[0]]);
      triangle[1] = *(boxtree->s.vertex[boxtree->poly[i].index[1]]);
      triangle[2] = *(boxtree->s.vertex[boxtree->poly[i].index[2]]);
      gx3d_GetBoundBox (&(boxtree->poly_box[i]), triangle, 3);
      // Compute box center
      gx3d_GetBoundBoxCenter (&(boxtree->poly_box[i]), &(boxtree->poly_box_center[i]));
    }
  }
}

/*____________________________________________________________________
|
| Function: Make_Static_Tree
|
| Input: Called from Init_Static_Boxtree()
| Output: Creates static bsp tree.  Returns true on success or false 
|   on any error.
|___________________________________________________________________*/

static bool Make_Static_Tree (gx3dBoxtree *boxtree)
{
  int i;
  gx3dBoxtreeNode *node;
  bool success = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create root node
  node = Make_Node (boxtree->num_polygons);
  if (node) {
    // Make this node the root of the bsp tree
    boxtree->root = node;
    // Put all polygons in root node
    for (i=0; i<boxtree->num_polygons; i++)
      node->poly_index[i] = i;
    node->num_polys = boxtree->num_polygons;
    // Set box of this node
    node->box = boxtree->box;
    // Subdivide this node
    if (Subdivide_Static_Subtree (boxtree, boxtree->root, 1))
      success = true;
  }

#ifdef DEBUG
  if (NOT success)
    debug_WriteFile ("Make_Static_Tree(): Error");
#endif

  return (success);
}

/*____________________________________________________________________
|
| Function: Subdivide_Static_Subtree
|
| Input: Called from Make_Static_Tree()
| Output: Subdivides static bsp subtree.  Returns true on success or 
|   false on any error.
|___________________________________________________________________*/

static bool Subdivide_Static_Subtree (
  gx3dBoxtree     *boxtree, 
  gx3dBoxtreeNode *subtree, 
  int              level )  // level at subtree (root is level 1)
{
  int i, j, which;
  float t, dx, dy, dz, split;
  bool subdivide;
  gx3dBoxtreeNode *left, *right;
  bool error = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);
  DEBUG_ASSERT (subtree);
  DEBUG_ASSERT (level >= 1);

#ifdef DEBUG
  char str[256];
  sprintf (str, "BSP tree node has %d polys, boxsize=%f,%f,%f", 
    subtree->num_polys,
    subtree->box.max.x - subtree->box.min.x,
    subtree->box.max.y - subtree->box.min.y,
    subtree->box.max.z - subtree->box.min.z );
  debug_WriteFile (str);
#endif

/*____________________________________________________________________
|
| Check if need to subdivide this node
|___________________________________________________________________*/

  // Assume yes
  subdivide = true;
  // If max level reached, don't subdivide any more
  if (level == MAX_LEVEL)
    subdivide = false;
  // If only 1 or 2 polys left, don't subdivide
  else if (subtree->num_polys <= 2)
    subdivide = false;
  
/*____________________________________________________________________
|
| Subdivide this node
|___________________________________________________________________*/

  if (subdivide) {

    // Create child nodes
    left  = Make_Node (subtree->num_polys);
    right = Make_Node (subtree->num_polys);
    if ((left == 0) OR (right == 0))
      error = true;

    if (NOT error) {
      // Get dimension of box
      dx = fabs (subtree->box.max.x - subtree->box.min.x);
      dy = fabs (subtree->box.max.y - subtree->box.min.y);
      dz = fabs (subtree->box.max.z - subtree->box.min.z);
      // Select largest dimension of box
      which = X_AXIS;
      t = dx;
      if (dz > t) {
        which = Z_AXIS;
        t = dz;
      }
      if (dy > t) 
        which = Y_AXIS;
      // Split on the selected axis
      left->box  = subtree->box;
      right->box = subtree->box;
      switch (which) {
        case X_AXIS: split = Get_Best_Split (boxtree, subtree, 'x');
                     left->box.max.x  = split;
                     right->box.min.x = split;
#ifdef DEBUG
                     debug_WriteFile ("  splitting on X-axis");
#endif
                     break;
                     break;
        case Y_AXIS: split = Get_Best_Split (boxtree, subtree, 'y');
                     left->box.max.y  = split;
                     right->box.min.y = split;
#ifdef DEBUG
                     debug_WriteFile ("  splitting on Y-axis");
#endif
                     break;
        case Z_AXIS: split = Get_Best_Split (boxtree, subtree, 'z');
                     left->box.max.z  = split;
                     right->box.min.z = split;
#ifdef DEBUG
                     debug_WriteFile ("  splitting on Z-axis");
#endif
      }
      // Put polys from subtree node into left and right children (each poly will go into either left or right)
      for (i=0; i<subtree->num_polys; i++) {
        j = subtree->poly_index[i];
        if (gx3d_Relation_Point_Box (&(boxtree->poly_box_center[j]), &(left->box)) == gxRELATION_INSIDE)
          left->poly_index[left->num_polys++] = j;
        else
          right->poly_index[right->num_polys++] = j;
      }
      // Relax child bounding boxes to encompass boxes of all polys in the child
      if (left->num_polys) {
        left->box = boxtree->poly_box[left->poly_index[0]];
        for (i=1; i<left->num_polys; i++)
          gx3d_EncloseBoundBox (&(left->box), &(boxtree->poly_box[left->poly_index[i]]));
      }
      if (right->num_polys) {
        right->box = boxtree->poly_box[right->poly_index[0]];
        for (i=1; i<right->num_polys; i++)
          gx3d_EncloseBoundBox (&(right->box), &(boxtree->poly_box[right->poly_index[i]]));
      }
      // Reallocate poly arrays in children as needed
      error = NOT Shrink_To_Fit (left,  subtree->num_polys);
      error = NOT Shrink_To_Fit (right, subtree->num_polys);
      if (NOT error) {
        // If left child has any polygons, attach the child to the subtree
        if (left->num_polys)
          subtree->left = left;
        else
          Free_Subtree (left);
        // If right child has any polygons, attach the child to the subtree
        if (right->num_polys)
          subtree->right = right;
        else
          Free_Subtree (right);
        // Delete the subtree poly array since all polys have been assigned to children
        free (subtree->poly_index);
        subtree->poly_index = 0;
      }
    } // if (NOT error)

/*____________________________________________________________________
|
| Subdivide children
|___________________________________________________________________*/

    // Subdivide left subtree, if any
    if (NOT error)
      if (subtree->left)
        error = NOT Subdivide_Static_Subtree (boxtree, subtree->left, level+1);
    // Subdivide right subtree, if any
    if (NOT error)
      if (subtree->right)
        error = NOT Subdivide_Static_Subtree (boxtree, subtree->right, level+1);

  } // if (subdivide)

#ifdef DEBUG
  if (error)
    debug_WriteFile ("Subdivide_Static_Subtree(): Error");
#endif

  return (NOT error);
}

/*____________________________________________________________________
|
| Function: Make_Node
| 
| Input: Called from Make_Static_Tree(), Subdivide_Static_Subtree() 
| Output: Creates a bsp tree node. Returns pointer to the node created
|   or 0 on any error.
|___________________________________________________________________*/

static gx3dBoxtreeNode *Make_Node (int num_polys)
{
  gx3dBoxtreeNode *node = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (num_polys > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create the node
  node = (gx3dBoxtreeNode *) calloc (1, sizeof(gx3dBoxtreeNode));
  if (node) {
    // Create memory for poly indexes array
    node->poly_index = (word *) malloc (num_polys * sizeof(word));
    // If error, delete node
    if (node->poly_index == 0) {
      free (node);
      node = 0;
    }
  }

#ifdef DEBUG
  if (node == 0)
    debug_WriteFile ("Make_Node(): Error");
#endif

  return (node);
}

/*____________________________________________________________________
|
| Function: Get_Best_Split
|
| Input: Called from Subdivide_Static_Subtree()
| Output: Computes best split point on an axis by average the center
|   points (on the given axis) of all polygons in the node.
|___________________________________________________________________*/

static float Get_Best_Split (gx3dBoxtree *boxtree, gx3dBoxtreeNode *node, char axis)
{
  int i;
  float sum = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);
  DEBUG_ASSERT (node);
  DEBUG_ASSERT ((axis == 'x') OR (axis == 'y') OR (axis == 'z'));

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  switch (axis) {
    case 'x': 
      for (i=0; i<node->num_polys; i++)
        sum += boxtree->poly_box_center[node->poly_index[i]].x;
      break;
    case 'y': 
      for (i=0; i<node->num_polys; i++)
        sum += boxtree->poly_box_center[node->poly_index[i]].y;
      break;
    case 'z': 
      for (i=0; i<node->num_polys; i++)
        sum += boxtree->poly_box_center[node->poly_index[i]].z;
      break;
  }
  return (sum / i);
}

/*____________________________________________________________________
|
| Function: Shrink_To_Fit
| 
| Input: Called from Subdivide_Static_Subtree() 
| Output: Shrinks poly_index array to fit exactly the number of polys.
|   Returns true on success, else false on any error.
|___________________________________________________________________*/

static bool Shrink_To_Fit (gx3dBoxtreeNode *node, int current_size)
{
  word *array;
  int error = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (node);
  DEBUG_ASSERT (current_size > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Any polys in this node?
  if (node->num_polys)
    // Too much space being used for this set of polys?
    if (node->num_polys < current_size) {
      // Allocate new, smaller, array
      array = (word *) malloc (node->num_polys * sizeof(word));
      if (array == 0)
        error = true;
      else {
        // Copy data into new array
        memcpy ((void *)array, (void *)(node->poly_index), node->num_polys * sizeof(word));
        // Delete old array
        free (node->poly_index);
        // Attach new array
        node->poly_index = array;
      }
    }

#ifdef DEBUG
  if (error)
    debug_WriteFile ("Shrink_To_Fit(): Error");
#endif

  return (NOT error);
}

/*____________________________________________________________________
|
| Function: Init_Dynamic_Boxtree
| 
| Input: Called from gx3d_Boxtree_Init()
| Output: Creates a dynamic boxtree of an object.
|___________________________________________________________________*/

static gx3dBoxtree *Init_Dynamic_Boxtree (gx3dObject *object)
{
  int num_vertices, num_polygons;
  bool error;
  gx3dBoxtree *boxtree = 0;

  error = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Allocate memory 
|___________________________________________________________________*/

  boxtree = (gx3dBoxtree *) calloc (1, sizeof(gx3dBoxtree));
  if (boxtree == 0)
    error = true;

/*____________________________________________________________________
|
| Build arrays of object geometry
|___________________________________________________________________*/

  if (NOT error) {
    // Init boxtree members
    boxtree->type = gx3d_BOXTREE_TYPE_DYNAMIC;
/***** I want to get rid of this dependency! ************/
//    boxtree->object = object;
/********************************************************/
    // Count total number of vertices and polygons in the object (from all layers)
    gx3d_GetObjectInfo (object, 0, &num_vertices, &num_polygons);
    // Allocate arrays of pointers to object data 
    boxtree->poly_layer = (gx3dObjectLayer **) calloc (num_polygons, sizeof(gx3dObjectLayer *));
    if (boxtree->poly_layer == 0)
      error = true;
    boxtree->poly_box = (gx3dBox *) calloc (num_polygons, sizeof(gx3dBox));
    if (boxtree->poly_box == 0)
      error = true;
    boxtree->poly_box_center = (gx3dVector *) calloc (num_polygons, sizeof(gx3dVector));
    if (boxtree->poly_box_center == 0)
      error = true;
    boxtree->poly = (gx3dPolygon *) calloc (num_polygons, sizeof(gx3dPolygon));
    if (boxtree->poly == 0)
      error = true;
    // Make local copy of actual vertices
    boxtree->d.vertex = (gx3dVector *) calloc (num_vertices, sizeof(gx3dVector));
    if (boxtree->d.vertex == 0)
      error = true;
  }

/*____________________________________________________________________
|
| Init data
|___________________________________________________________________*/
  
/*
  if (NOT error) {
    if (object->layer) {
      // Set initial overall bound box for boxtree
      boxtree->box.min = object->layer->vertex[0];
      boxtree->box.max = object->layer->vertex[0];
      // Fill arrays with data
      n = 0;
      m = object->transform.local_matrix;
      Get_Static_Geometry (object->layer, boxtree, &n, m);
      // Make bsp tree
      if (NOT Make_Dynamic_Tree (boxtree))
        error = true;
    }
  }
*/
  
/*____________________________________________________________________
|
| On any error, free memory
|___________________________________________________________________*/

  if (error) {
    gx3d_Boxtree_Free (boxtree);
    boxtree = 0;
  }

  return (boxtree);
}

/*____________________________________________________________________
|
| Function: Get_Dynamic_Geometry
|
| Input: Called from Init_Dynamic_Boxtree()
| Output: Goes through object layer (and child and sibling layers) and 
|   fills boxtree arrays.
|___________________________________________________________________*/

static void Get_Dynamic_Geometry (gx3dObjectLayer *layer, gx3dBoxtree *boxtree, int *n)
//static void Get_Dynamic_Geometry (gx3dObjectLayer *layer, gx3dBoxtree *boxtree, int *n, gx3dMatrix m)
{
  int i;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (boxtree);
  DEBUG_ASSERT (n);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

//***** TO DO: This function needs to compute overall boxtree box 

  for (; layer; layer=layer->next) {
    // Process child layer/s first
    if (layer->child)
      Get_Dynamic_Geometry (layer->child, boxtree, n);
    // Process this layer
    for (i=0; i<layer->num_polygons; i++) {
      boxtree->poly_layer[boxtree->num_polygons] = layer;
      boxtree->poly      [boxtree->num_polygons].index[0] = layer->polygon[i].index[0] + (word)(*n);
      boxtree->poly      [boxtree->num_polygons].index[1] = layer->polygon[i].index[1] + (word)(*n);
      boxtree->poly      [boxtree->num_polygons].index[2] = layer->polygon[i].index[2] + (word)(*n);
      boxtree->num_polygons++;
    }
    *n += layer->num_vertices; 
  }
}

/*____________________________________________________________________
|
| Function: Make_Dynamic_Tree
|
| Input: Called from Init_Static_Boxtree()
| Output: Creates dynamic bsp tree.  Returns true on success or false 
|   on any error.
|___________________________________________________________________*/

static bool Make_Dynamic_Tree (gx3dBoxtree *boxtree)
{
  bool error = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (NOT error);
}

/*____________________________________________________________________
|
| Function: gx3d_Boxtree_Free
|
| Output: Frees all memory for a boxtree.
|___________________________________________________________________*/

void gx3d_Boxtree_Free (gx3dBoxtree *boxtree)
{
  if (boxtree) {
    // Free all memory for boxtree
    if (boxtree->poly_layer)
      free (boxtree->poly_layer);
    if (boxtree->poly_box)
      free (boxtree->poly_box);
    if (boxtree->poly_box_center)
      free (boxtree->poly_box_center);
    if (boxtree->poly)
      free (boxtree->poly);
    switch (boxtree->type) {
      case gx3d_BOXTREE_TYPE_STATIC:
        if (boxtree->s.vertex)
          free (boxtree->s.vertex);
        break;
      case gx3d_BOXTREE_TYPE_DYNAMIC:
        if (boxtree->d.vertex)
          free (boxtree->d.vertex);
        break;
    }      
    Free_Subtree (boxtree->root);
    free (boxtree);
  }
}

/*____________________________________________________________________
|
| Function: Free_Subtree
|
| Input: Called from gx3d_Boxtree_Free
| Output: Frees memory for the bsp subtree of boxtree.
|___________________________________________________________________*/

static void Free_Subtree (gx3dBoxtreeNode *subtree)
{
  if (subtree) {
    // Free left subtree
    Free_Subtree (subtree->left);
    // Free right subtree
    Free_Subtree (subtree->right);
    // Free memory root of this subtree
    if (subtree->poly_index)
      free (subtree->poly_index);
    free (subtree);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_Boxtree_SetDirty
|
| Output: Sets dirty flag of a dynamic boxtree.  For static boxtrees
|   this function has no effect.
|___________________________________________________________________*/

void gx3d_Boxtree_SetDirty (gx3dBoxtree *boxtree)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (boxtree->type == gx3d_BOXTREE_TYPE_DYNAMIC)
    boxtree->d.dirty = true;
}

/*____________________________________________________________________
|
| Function: g3d_Boxtree_Update
|
| Output: Recomputes bsp tree of boxtree.
|___________________________________________________________________*/

void g3d_Boxtree_Update (gx3dBoxtree *boxtree)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  switch (boxtree->type) {
    case gx3d_BOXTREE_TYPE_STATIC:
      Update_Static_Boxtree (boxtree);
      break;
    case gx3d_BOXTREE_TYPE_DYNAMIC:
      Update_Dynamic_Boxtree (boxtree);
      break;
  }      
}

/*____________________________________________________________________
|
| Function: Update_Static_Boxtree
|
| Output: Updates a static boxtree. This will typically happen after a
|   call to gx3d_TransformObject() or gx3d_TransformObjectLayer() which
|   both permanently transform the vertices of an object.  In that case
|   the boxtree bsp tree needs to be rebuilt from scratch.
|___________________________________________________________________*/

static void Update_Static_Boxtree (gx3dBoxtree *boxtree)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Free current bsp tree
  Free_Subtree (boxtree->root);
  // Recompute bound boxes
  Get_Static_Bound_Box (boxtree);
  // Recompute bsp tree
  Make_Static_Tree (boxtree);
}

/*____________________________________________________________________
|
| Function: Update_Dynamic_Boxtree
|
| Output: Updates a static boxtree.
|___________________________________________________________________*/

static void Update_Dynamic_Boxtree (gx3dBoxtree *boxtree)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/


}

/*____________________________________________________________________
|
| Function: g3d_Boxtree_Intersect_Ray
|
| Output: Returns intersection of an infinite ray with a boxtree.
|
|   Returns gxRELATION_OUTSIDE     = ray outside all geometry in boxtree (doesn't intersect)
|           gxRELATION_INTERSECT * = ray intersects (or is inside) a poly
|
|   * Optionally returns distance, intersection point and pointer to
|     layer name (if any) of the layer containing the poly hit.
|___________________________________________________________________*/

gxRelation gx3d_Boxtree_Intersect_Ray (
  gx3dBoxtree *boxtree,         
  gx3dRay     *ray,
  float        ray_length,     
  float       *distance,        // optional
  gx3dVector  *intersection,    // optional
  char       **name )           // optional
{
  bool intersection_found;
  float closest_distance;
  gx3dVector intersection_point;
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);
  DEBUG_ASSERT (ray);
  DEBUG_ASSERT (ray_length > 0);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  intersection_found = false;
  Subtree_Intersect_Ray (boxtree, boxtree->root, ray, ray_length, &intersection_found, &intersection_point, &closest_distance, name);
  if (intersection_found) {
    result = gxRELATION_INTERSECT;
    if (distance) 
      *distance = gx3d_Distance_Point_Point (&ray->origin, &intersection_point);
    if (intersection)
      *intersection = intersection_point;
  }
  else
    result = gxRELATION_OUTSIDE;

  return (result);
}

/*____________________________________________________________________
|
| Function: Subtree_Intersect_Ray
|
| Input: Called from gx3d_Boxtree_Intersect_Ray()
| Output: Returns intersection information of ray with boxtree
|___________________________________________________________________*/

static void Subtree_Intersect_Ray (
  gx3dBoxtree     *boxtree,
  gx3dBoxtreeNode *subtree,         
  gx3dRay         *ray,
  float            ray_length,     
  bool            *intersection_found,
  gx3dVector      *intersection,
  float           *closest_distance,
  char           **name )               // optional
{
  int i;
  float new_distance;
  gx3dVector intersection_point, triangle[3];
  gxRelation result;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (boxtree);
  DEBUG_ASSERT (subtree);
  DEBUG_ASSERT (ray);
  DEBUG_ASSERT (ray_length > 0);
  DEBUG_ASSERT (intersection_found);
  DEBUG_ASSERT (intersection);
  DEBUG_ASSERT (closest_distance);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

    // Does ray intersect (or is inside of) box for this subtree?
/*
char str[200];
debug_WriteFile ("(box)");
sprintf (str, "  xmin = %f", subtree->box.min.x);
debug_WriteFile (str);
sprintf (str, "  ymin = %f", subtree->box.min.y);
debug_WriteFile (str);
sprintf (str, "  zmin = %f", subtree->box.min.z);
debug_WriteFile (str);
sprintf (str, "  xmax = %f", subtree->box.max.x);
debug_WriteFile (str);
sprintf (str, "  ymax = %f", subtree->box.max.y);
debug_WriteFile (str);
sprintf (str, "  zmax = %f", subtree->box.max.z);
debug_WriteFile (str);
debug_WriteFile ("(ray)");
sprintf (str, "  origin = %f,%f,%f", ray->origin.x, ray->origin.y, ray->origin.z);
debug_WriteFile (str);
sprintf (str, "  direction = %f,%f,%f", ray->direction.x, ray->direction.y, ray->direction.z);
debug_WriteFile (str);
sprintf (str, "  length = %f", ray_length);
debug_WriteFile (str);
*/
  if (gx3d_Intersect_Ray_Box (ray, ray_length, &(subtree->box), 0, 0) != gxRELATION_OUTSIDE) {
//      debug_WriteFile ("Ray intersects 1");
    // Is this a nonterminal node?
    if (subtree->left OR subtree->right) {
      // Then try intersection with child nodes
      if (subtree->left)
        Subtree_Intersect_Ray (boxtree, subtree->left, ray, ray_length, intersection_found, intersection, closest_distance, name);
      if (subtree->right)
        Subtree_Intersect_Ray (boxtree, subtree->right, ray, ray_length, intersection_found, intersection, closest_distance, name);
    }
    // This is a terminal node, containing bounding boxes for individual polys
    else {
      // Test against all poly boxes
      for (i=0; i<subtree->num_polys; i++) {
        // Test against 1 poly box
        result = gx3d_Intersect_Ray_Box (ray, ray_length, &(boxtree->poly_box[subtree->poly_index[i]]), 0, 0);
        if (result != gxRELATION_OUTSIDE) {
//            debug_WriteFile ("Ray intersects 2");
          triangle[0] = *(boxtree->s.vertex[boxtree->poly[subtree->poly_index[i]].index[0]]);
          triangle[1] = *(boxtree->s.vertex[boxtree->poly[subtree->poly_index[i]].index[1]]);
          triangle[2] = *(boxtree->s.vertex[boxtree->poly[subtree->poly_index[i]].index[2]]);
          result = gx3d_Intersect_Ray_TriangleFront (ray, ray_length, triangle, 0, &intersection_point, 0, 0);
          if (result == gxRELATION_INTERSECT) {
//              debug_WriteFile ("Ray intersects 3");
            // Compute distance (squared) from the ray origin to the intersection point
            new_distance = gx3d_DistanceSquared_Point_Point (&(ray->origin), &intersection_point);
            // Have any other intersections been found?
            if (*intersection_found) {
              // Is this one closer than the closest so far?
              if (new_distance < *closest_distance) {
                *closest_distance = new_distance;
                if (intersection)
                  *intersection = intersection_point;
                if (name)
                  *name = boxtree->poly_layer[i]->name;
              }
            }
            else {
              *intersection_found = true;
              *closest_distance = new_distance;
              if (intersection)
                *intersection = intersection_point;
              if (name)
                *name = boxtree->poly_layer[i]->name;
            }
          }
        }
      }
    }
  }
}
