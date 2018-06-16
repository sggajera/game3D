/*____________________________________________________________________
|
| File: gx3d_object.cpp
|
| Description: Functions to manipulate gx3dObject data.
|
| Functions: gx3d_CreateObject
|            gx3d_CreateObjectLayer
|             Get_Layer_With_ID
|            gx3d_FreeObject
|             Free_Layer
|            gx3d_FreeAllObject
|            gx3d_CopyObject
|             Copy_Layer
|              Copy_SubLayer
|            gx3d_SetObjectName
|            gx3d_OptimizeObject
|             Optimize_Layer
|            gx3d_GetObjectInfo
|             GetObjectInfo_Layer
|            gx3d_DrawObject
|						 gx3d_DrawObjectLayer
|							Draw_Layer
|						 gx3d_Object_UpdateTransforms
|             Update_Layer_Vertices
|              Update_Layer_Morphs
|							Update_Layer_Transforms
|            gx3d_GetbjectLayer
|             gx3d_Get_Layer_With_Name
|            gx3d_SetObjectMatrix
|            gx3d_SetObjectLayerMatrix
|
|            gx3d_TwistXObject
|             TwistX_Layer
|            gx3d_TwistYObject
|             TwistY_Layer
|            gx3d_TwistZObject
|             TwistZ_Layer
|
|            gx3d_TransformObject
|             gx3d_TransformObjectLayer
|
|            gx3d_CombineObjects
|             Combine_Object_Layers
|
|            gx3d_ReadLWO2File
|            gx3d_WriteLWO2File
|
|            gx3d_WriteGX3DBINFile
|            gx3d_ReadGX3DBINFile
|
|            gx3d_ObjectBoundBoxVisible
|            gx3d_ObjectBoundSphereVisible
|
|            gx3d_MakeDoubleSidedObject
|             MakeDoubleSided_Layer
|            gx3d_MakeDoubleSidedObjectLayer
|
|            gx3d_ComputeVertexNormals
|             ComputeVertexNormal_Layer
|
|            gx3d_ComputeObjectBounds
|             Compute_Bounding_Box
|             Compute_Bounding_Sphere
|             Compute_Optimal_Bounding_Sphere
|
|            gx3d_GetMorph
|            gx3d_SetMorphAmount
|            gx3d_SetMorphAmount
|            gx3d_SetMorphAmount
|             Set_Morph_Amount
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
#include "texture.h"
#include "gx3d_lwo2.h"
#include "gx3d_gx3dbin.h"

/*___________________
|
| Constants
|__________________*/

#define ADD_TO_OBJECTLIST(_obj_)    \
  {                                 \
    if (objectlist == 0)            \
      objectlist = _obj_;           \
    else {                          \
      _obj_->next = objectlist;     \
      objectlist->previous = _obj_;	\
      objectlist = _obj_;           \
    }                               \
  }

#define REMOVE_FROM_OBJECTLIST(_obj_)           \
  {                                             \
    if (_obj_->previous)                        \
      _obj_->previous->next = _obj_->next;      \
    else                                        \
      objectlist = _obj_->next;                 \
    if (_obj_->next)                            \
      _obj_->next->previous = _obj_->previous;	\
  }

/*___________________
|
| Function Prototpyes
|__________________*/

static gx3dObjectLayer *Get_Layer_With_ID (gx3dObjectLayer *layer, int id);
static void Free_Layer (gx3dObjectLayer *layer);
static void Copy_Layer (gx3dObjectLayer *src_layer, gx3dObjectLayer **dst_layer);
static void Copy_SubLayer (gx3dObjectLayer *src_layer, gx3dObjectLayer **dst_layer);
static void Optimize_Layer (gx3dObjectLayer *layer);
static void GetObjectInfo_Layer (gx3dObjectLayer *layer, int *num_layers, int *num_vertices, int *num_polygons);
static void Draw_Layer (gx3dObjectLayer *layer, unsigned flags, bool draw_one_layer_only);
static void Update_Layer_Vertices (gx3dObjectLayer *layer);
static void Update_Layer_Morphs (gx3dObjectLayer *layer);
static void Update_Layer_Transforms (gx3dObjectLayer *layer, gx3dMatrix *parent_matrix, int parent_transform_dirty);
static gx3dObjectLayer *Get_Layer_With_Name (gx3dObjectLayer *layer, char *);
static void TwistX_Layer   (gx3dObjectLayer *layer, float twist_rate);
static void TwistY_Layer   (gx3dObjectLayer *layer, float twist_rate);
static void TwistZ_Layer   (gx3dObjectLayer *layer, float twist_rate);
static bool Combine_Object_Layers (gx3dObjectLayer *dst_layer, gx3dObjectLayer *src_layer);
static void MakeDoubleSided_Layer (gx3dObjectLayer *layer);
static void ComputeVertexNormal_Layer (gx3dObjectLayer *layer, unsigned flags);
static void Compute_Bounding_Box (gx3dObjectLayer *layer, gx3dBox *object_box);
static void Compute_Bounding_Sphere (gx3dObjectLayer *layer, gx3dSphere *object_sphere);
static void Compute_Optimal_Bounding_Sphere (gx3dObjectLayer *layer, gx3dSphere *object_sphere);
static void Set_Morph_Amount (gx3dObjectLayer *layer, char *morph_name, float amount);

/*___________________
|
| Global variables
|__________________*/

static gx3dObject *objectlist = 0;	// doubly linked list of objects

/*____________________________________________________________________
|
| Function: gx3d_CreateObject
| 
| Output: Creates a empty object and returns a pointer to it.
|___________________________________________________________________*/

gx3dObject *gx3d_CreateObject ()
{
	gx3dObject *object = 0;
    
  object = (gx3dObject *) calloc (1, sizeof(gx3dObject));
  if (object) {
    // Init transforms
    gx3d_GetIdentityMatrix (&(object->transform.local_matrix));
    gx3d_GetIdentityMatrix (&(object->transform.composite_matrix));
 		// Add to objectlist
		ADD_TO_OBJECTLIST (object);
 }

/*____________________________________________________________________
|
| Verify return params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

	return (object);
}

/*____________________________________________________________________
|
| Function: gx3d_CreateObjectLayer
| 
| Output: Creates an object layer at the top layer level in the object
|   and returns a pointer to the layer or NULL on any error.
|___________________________________________________________________*/

gx3dObjectLayer *gx3d_CreateObjectLayer (gx3dObject *object)
{
  int id, found;
	gx3dObjectLayer *layer, **lpp;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create a new layer
  layer = (gx3dObjectLayer *) calloc (1, sizeof(gx3dObjectLayer));
  if (layer) {
    // Init transforms
    gx3d_GetIdentityMatrix (&(layer->transform.local_matrix));
    gx3d_GetIdentityMatrix (&(layer->transform.composite_matrix));
    // Find a unique id for this layer
    if (object->layer == 0)
      layer->id = 1;
    else 
      for (id=1, found=FALSE; NOT found; id++) 
        if (Get_Layer_With_ID (object->layer, id) == NULL) {
          layer->id = id;
          found = TRUE;
        }
    // Attach this layer to the end of the object layer list
    for (lpp=&(object->layer); *lpp; lpp=&((*lpp)->next));
    *lpp = layer;
  }

/*____________________________________________________________________
|
| Verify return params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

	return (layer);
}

/*____________________________________________________________________
|
| Function: Get_Layer_With_ID
|                       
| Input: Called from gx3d_CreateObjectLayer()                                                                 
| Output: Returns the gx3d layer that has the id or 0 if not found.
|___________________________________________________________________*/

static gx3dObjectLayer *Get_Layer_With_ID (gx3dObjectLayer *layer, int id)
{
  gx3dObjectLayer *the_layer = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (layer) {
    // Is the input layer the one?
    if (layer->id == id)
      the_layer = layer;

    // If not found, search child layers, if any
    if (the_layer == 0) 
      if (layer->child)
        the_layer = Get_Layer_With_ID (layer->child, id);

    // If not found, search the rest of the layers on this level
    if (the_layer == NULL)
      if (layer->next)
        the_layer = Get_Layer_With_ID (layer->next, id);
  }

  return (the_layer);
}

/*____________________________________________________________________
|
| Function: gx3d_FreeObject
|
| Output: Frees all memory associated with an object.
|___________________________________________________________________*/

void gx3d_FreeObject (gx3dObject *object)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

	// Remove it from the list of objects
  REMOVE_FROM_OBJECTLIST (object)
  // Free name?
  if (object->name)
    free (object->name);
  // Free skeleton?
  if (object->skeleton)
    gx3d_Skeleton_Free (object->skeleton);
  // Free all layers
  if (object->layer)
    Free_Layer (object->layer);
  // Free the object
  free (object);
}

/*____________________________________________________________________
|
| Function: Free_Layer
|
| Input: Called from gx3d_FreeObject()
| Output: Frees all memory associated with a layer including linked layers
|   and child layers.
|___________________________________________________________________*/

static void Free_Layer (gx3dObjectLayer *layer)
{
  int i;
  gx3dObjectLayer *tlayer;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  while (layer) {
    // Free child layer/s first
    if (layer->child)
      Free_Layer (layer->child);

    // Unregister this layer
    if (layer->driver_data) 
      if (gx_Video.unregister_object) 
        (*gx_Video.unregister_object) (layer->driver_data);

    // Free layer memory
    if (layer->name)
      free (layer->name);
    if (layer->vertex)
	    free (layer->vertex);
    if (layer->X_vertex)
      free (layer->X_vertex);
    if (layer->polygon)
	    free (layer->polygon); 
    if (layer->polygon_normal)
	    free (layer->polygon_normal);
    if (layer->vertex_normal)
	    free (layer->vertex_normal);
    if (layer->X_vertex_normal)
      free (layer->X_vertex_normal);
    if (layer->diffuse)
	    free (layer->diffuse);
    if (layer->specular)
	    free (layer->specular);
    for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) {
      if (layer->tex_coords[i])
        free (layer->tex_coords[i]);
      if (layer->X_tex_coords[i])
        free (layer->X_tex_coords[i]);
      if (layer->tex_coords_w[i])
        free (layer->tex_coords_w[i]);
      if (layer->X_tex_coords_w[i])
        free (layer->X_tex_coords_w[i]);
    }
    if (layer->weight)
      free (layer->weight);
    if (layer->X_weight)
      free (layer->X_weight);
    // Free matrix palette, if any
    if (layer->matrix_palette) {
      for (i=0; i<layer->num_matrix_palette; i++)
        if (layer->matrix_palette[i].weightmap_name)
          free (layer->matrix_palette[i].weightmap_name);
      free (layer->matrix_palette);
    }
    // Free morphs, if any
    if (layer->morph) {
      for (i=0; i<layer->num_morphs; i++) {
        if (layer->morph[i].name)
          free (layer->morph[i].name);
        if (layer->morph[i].index)
          free (layer->morph[i].index);
        if (layer->morph[i].offset)
          free (layer->morph[i].offset);
      }
      free (layer->morph);
    }
    if (layer->composite_morph)
      free (layer->composite_morph);
    // Free textures
    for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) 
      if (layer->texture[i])
        gx3d_FreeTexture (layer->texture[i]);

    // Goto next layer in the list
    tlayer = layer->next;
    free (layer);
    layer = tlayer;
  }
}

/*___________________________________________________________________
|
|	Function: gx3d_FreeAllObjects
| 
|	Output: Frees all objects, if any.
|___________________________________________________________________*/

void gx3d_FreeAllObjects ()
{
#ifndef _DEBUG
  while (objectlist) 
    gx3d_FreeObject (objectlist);
#else
	int i;
	for (i=0; objectlist; i++)
    gx3d_FreeObject (objectlist);
	if (i) {
		char str[128];
		sprintf (str, "gx3d_FreeAllObjects(): Freeing %d objects left in memory", i);
		DEBUG_WRITE (str)
	}
#endif
}

/*____________________________________________________________________
|
| Function: gx3d_CopyObject
|
| Output: Makes a copy of an object, returning a pointer to the copy
|   or null on any error.
|___________________________________________________________________*/

gx3dObject *gx3d_CopyObject (gx3dObject *object)
{
  bool error = false;
  gx3dObject *copy = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Copy top-level object
|___________________________________________________________________*/

  // Create a new empty gx3d object
  copy = gx3d_CreateObject ();
  if (copy == 0)
    error = true;
  else {
    // Copy object name, if any
    if (object->name) {
      copy->name = (char *) malloc (strlen(object->name)+1);
      if (copy->name == 0)
        error = true;
      else
        strcpy (copy->name, object->name);
    }
    // Copy other values
    copy->vertex_format = object->vertex_format;
    copy->bound_box     = object->bound_box;
    copy->bound_sphere  = object->bound_sphere;
    copy->transform     = object->transform;
  }

/*____________________________________________________________________
|
| Copy layers
|___________________________________________________________________*/

  if (NOT error)
    Copy_Layer (object->layer, &copy->layer);

/*____________________________________________________________________
|
| Copy skeleton
|___________________________________________________________________*/

  if (NOT error) {
    // Copy skeleton, if any
    if (object->skeleton) {
      copy->skeleton = gx3d_Skeleton_Copy (object->skeleton);
      if (copy->skeleton == 0)
        error = true;
    }
  }

/*____________________________________________________________________
|
| On any error, free copy of object
|___________________________________________________________________*/

  if (error) {
    gx3d_FreeObject (copy);
    copy = 0;
  }

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (copy);

  return (copy);
}

/*____________________________________________________________________
|
| Function: Copy_Layer
|
| Input: Called from gx3d_CopyObject()
| Output: Copies a layer from source object to destination object 
|     including linked layers and child layers.  Creates the destination 
|     layer.
|___________________________________________________________________*/

static void Copy_Layer (gx3dObjectLayer *src_layer, gx3dObjectLayer **dst_layer)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (src_layer);
  DEBUG_ASSERT (dst_layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; src_layer; src_layer=src_layer->next, dst_layer=&(*dst_layer)->next) {
    // Process child layer/s first
    if (src_layer->child)
      Copy_Layer (src_layer->child, &(*dst_layer)->child);
    // Process this sub layer
    Copy_SubLayer (src_layer, dst_layer);
  }
}

/*____________________________________________________________________
|
| Function: Copy_SubLayer
|
| Output: Copies a layer from source object to destination object.  
|   Creates the destination layer.
|___________________________________________________________________*/

static void Copy_SubLayer (gx3dObjectLayer *src_layer, gx3dObjectLayer **dst_layer)
{
  int i;
  bool error = false;
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (src_layer);
  DEBUG_ASSERT (dst_layer);

/*____________________________________________________________________
|
| Create an empty destination layer
|___________________________________________________________________*/

  *dst_layer = (gx3dObjectLayer *) calloc (1, sizeof(gx3dObjectLayer));
  if (*dst_layer == 0)
    error = true;

/*____________________________________________________________________
|
| Copy contents of source layer to destination layer
|___________________________________________________________________*/

  if (NOT error) {
    // Copy data
    (*dst_layer)->id           = src_layer->id;
    (*dst_layer)->parent_id    = src_layer->parent_id;
    (*dst_layer)->has_parent   = src_layer->has_parent;
    (*dst_layer)->pivot        = src_layer->pivot;
    (*dst_layer)->bound_box    = src_layer->bound_box;
    (*dst_layer)->bound_sphere = src_layer->bound_sphere;
    (*dst_layer)->num_vertices = src_layer->num_vertices;
    (*dst_layer)->num_polygons = src_layer->num_polygons;
    (*dst_layer)->num_textures = src_layer->num_textures;
    (*dst_layer)->transform    = src_layer->transform;
    // Copy textures
    for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) {
      (*dst_layer)->texture[i] = src_layer->texture[i];
      Texture_AddRef ((Texture *)((*dst_layer)->texture[i]));
    }
    // Copy layer name, if any
    if (src_layer->name) {
      (*dst_layer)->name = (char *) malloc (strlen(src_layer->name)+1);
      if ((*dst_layer)->name == 0)
        error = true;
      else
        strcpy ((*dst_layer)->name, src_layer->name);
    }
    // Copy vertices, if any
    if (src_layer->vertex) {
      (*dst_layer)->vertex = (gx3dVector *) malloc (src_layer->num_vertices * sizeof(gx3dVector));
      if ((*dst_layer)->vertex == 0)
        error = true;
      else
        memcpy ((void *)(*dst_layer)->vertex, (void *)src_layer->vertex, src_layer->num_vertices * sizeof(gx3dVector));
    }
    // Copy X_vertices, if any
    if (src_layer->X_vertex) {
      (*dst_layer)->X_vertex = (gx3dVector *) malloc (src_layer->num_vertices * sizeof(gx3dVector));
      if ((*dst_layer)->X_vertex == 0)
        error = true;
      else
        memcpy ((void *)(*dst_layer)->X_vertex, (void *)src_layer->X_vertex, src_layer->num_vertices * sizeof(gx3dVector));
    }
    // Copy vertex normals, if any
    if (src_layer->vertex_normal) {
      (*dst_layer)->vertex_normal = (gx3dVector *) malloc (src_layer->num_vertices * sizeof(gx3dVector));
      if ((*dst_layer)->vertex_normal == 0)
        error = true;
      else
        memcpy ((void *)(*dst_layer)->vertex_normal, (void *)src_layer->vertex_normal, src_layer->num_vertices * sizeof(gx3dVector));
    }
    // Copy X_vertex normals, if any
    if (src_layer->X_vertex_normal) {
      (*dst_layer)->X_vertex_normal = (gx3dVector *) malloc (src_layer->num_vertices * sizeof(gx3dVector));
      if ((*dst_layer)->X_vertex_normal == 0)
        error = true;
      else
        memcpy ((void *)(*dst_layer)->X_vertex_normal, (void *)src_layer->X_vertex_normal, src_layer->num_vertices * sizeof(gx3dVector));
    }
    // Copy diffuse colors, if any
    if (src_layer->diffuse) {
      (*dst_layer)->diffuse = (gxColor *) malloc (src_layer->num_vertices * sizeof(gxColor));
      if ((*dst_layer)->diffuse == 0)
        error = true;
      else
        memcpy ((void *)(*dst_layer)->diffuse, (void *)src_layer->diffuse, src_layer->num_vertices * sizeof(gxColor));
    }
    // Copy specular colors, if any
    if (src_layer->specular) {
      (*dst_layer)->specular = (gxColor *) malloc (src_layer->num_vertices * sizeof(gxColor));
      if ((*dst_layer)->specular == 0)
        error = true;
      else
        memcpy ((void *)(*dst_layer)->specular, (void *)src_layer->specular, src_layer->num_vertices * sizeof(gxColor));
    }
    // Copy vertex weights, if any
    if (src_layer->weight) {
      (*dst_layer)->weight = (gx3dVertexWeight *) malloc (src_layer->num_vertices * sizeof(gx3dVertexWeight));
      if ((*dst_layer)->weight == 0)
        error = true;
      else
        memcpy ((void *)(*dst_layer)->weight, (void *)src_layer->weight, src_layer->num_vertices * sizeof(gx3dVertexWeight));
    }
    // Copy X_vertex weights, if any
    if (src_layer->X_weight) {
      (*dst_layer)->X_weight = (gx3dVertexWeight *) malloc (src_layer->num_vertices * sizeof(gx3dVertexWeight));
      if ((*dst_layer)->X_weight == 0)
        error = true;
      else
        memcpy ((void *)(*dst_layer)->X_weight, (void *)src_layer->X_weight, src_layer->num_vertices * sizeof(gx3dVertexWeight));
    }
    // Copy polygons, if any
    if (src_layer->polygon) {
      (*dst_layer)->polygon = (gx3dPolygon *) malloc (src_layer->num_polygons * sizeof(gx3dPolygon));
      if ((*dst_layer)->polygon == 0)
        error = true;
      else
        memcpy ((void *)(*dst_layer)->polygon, (void *)src_layer->polygon, src_layer->num_polygons * sizeof(gx3dPolygon));
    }
    // Copy polygon normals, if any
    if (src_layer->polygon_normal) {
      (*dst_layer)->polygon_normal = (gx3dVector *) malloc (src_layer->num_polygons * sizeof(gx3dVector));
      if ((*dst_layer)->polygon_normal == 0)
        error = true;
      else
        memcpy ((void *)(*dst_layer)->polygon_normal, (void *)src_layer->polygon_normal, src_layer->num_polygons * sizeof(gx3dVector));
    }
    // Copy texture coord arrays, if any
    for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) { 
      if (src_layer->tex_coords[i] AND (NOT error)) {
        (*dst_layer)->tex_coords[i] = (gx3dUVCoordinate *) malloc (src_layer->num_vertices * sizeof(gx3dUVCoordinate));
        if ((*dst_layer)->tex_coords[i] == 0)
          error = TRUE;
        else 
          memcpy ((void *)(*dst_layer)->tex_coords[i], src_layer->tex_coords[i], src_layer->num_vertices * sizeof(gx3dUVCoordinate));
      }
      if (src_layer->tex_coords_w[i] AND (NOT error)) {
        (*dst_layer)->tex_coords_w[i] = (float *) malloc (src_layer->num_vertices * sizeof(float));
        if ((*dst_layer)->tex_coords_w[i] == 0)
          error = TRUE;
        else 
          memcpy ((void *)(*dst_layer)->tex_coords_w[i], src_layer->tex_coords_w[i], src_layer->num_vertices * sizeof(float));
      }
    }
    // Copy X_texture coord arrays, if any
    for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) { 
      if (src_layer->X_tex_coords[i] AND (NOT error)) {
        (*dst_layer)->X_tex_coords[i] = (gx3dUVCoordinate *) malloc (src_layer->num_vertices * sizeof(gx3dUVCoordinate));
        if ((*dst_layer)->X_tex_coords[i] == 0)
          error = TRUE;
        else 
          memcpy ((void *)(*dst_layer)->X_tex_coords[i], src_layer->X_tex_coords[i], src_layer->num_vertices * sizeof(gx3dUVCoordinate));
      }
      if (src_layer->X_tex_coords_w[i] AND (NOT error)) {
        (*dst_layer)->X_tex_coords_w[i] = (float *) malloc (src_layer->num_vertices * sizeof(float));
        if ((*dst_layer)->X_tex_coords_w[i] == 0)
          error = TRUE;
        else 
          memcpy ((void *)(*dst_layer)->X_tex_coords_w[i], src_layer->X_tex_coords_w[i], src_layer->num_vertices * sizeof(float));
      }
    }
  }

/*____________________________________________________________________
|
| On any error, free all memory for layer copy
|___________________________________________________________________*/

  if (error) {
    Free_Layer (*dst_layer);
    *dst_layer = 0;
  }

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (*dst_layer);
}

/*____________________________________________________________________
|
| Function: gx3d_SetObjectName
|
| Output: Sets name for an object, replacing current name, if any.
|___________________________________________________________________*/

void gx3d_SetObjectName (gx3dObject *object, char *name)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);
  DEBUG_ASSERT (name);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Free old name, if any
  if (object->name) 
    free (object->name);
  // Allocate memory for new name
  object->name = (char *) malloc (strlen(name + 1));
  // Store new name
  if (object->name)
    strcpy (object->name, name);

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (object->name);
}

/*____________________________________________________________________
|
| Function: gx3d_OptimizeObject
|
| Output: Optimizes object for drawing by buffering parts of the object
|   in vram.
|___________________________________________________________________*/

void gx3d_OptimizeObject (gx3dObject *object)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Optimize all layers
  if (object->layer)
    Optimize_Layer (object->layer);
}

/*____________________________________________________________________
|
| Function: Optimize_Layer
|
| Input: Called from gx3d_OptimizeObject()
| Output: Optimizes a layer including linked layers and child layers.
|___________________________________________________________________*/

static void Optimize_Layer (gx3dObjectLayer *layer)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; layer; layer=layer->next) {
    // Optimze child layer/s first
    if (layer->child)
      Optimize_Layer (layer->child);

    // Register layer?
    if (layer->driver_data == 0) {
      if (gx_Video.register_object) 
        (*gx_Video.register_object) ( (word *)layer->polygon, 
                                             &layer->num_polygons,
                                     (float *)layer->vertex,
                                   (float **)&layer->X_vertex,
                                             &layer->num_vertices,
                                     (float *)layer->vertex_normal,
                                   (float **)&layer->X_vertex_normal,
                                      (byte *)layer->diffuse,
                                      (byte *)layer->specular,
                                    (float **)layer->tex_coords,
                                    (float **)layer->X_tex_coords,
                                    (float **)layer->tex_coords_w,
                                    (float **)layer->X_tex_coords_w,
                                      (byte *)layer->weight,
                                    (byte **)&layer->X_weight,
                                             &layer->driver_data );
    }    
    // Optimize layer
    if (layer->driver_data AND gx_Video.optimize_object)
      (*gx_Video.optimize_object) (layer->driver_data);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_GetObjectInfo
|                                                                                        
| Output: Returns in callers variables information about an object (all
|   layers combined).  Any of callers variables can be null.
|___________________________________________________________________*/
 
void gx3d_GetObjectInfo (gx3dObject *object, int *num_layers, int *num_vertices, int *num_polygons)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (num_layers)
    *num_layers = 0;
  if (num_vertices)
    *num_vertices = 0;
  if (num_polygons)
    *num_polygons = 0;
  if (object) 
    GetObjectInfo_Layer (object->layer, num_layers, num_vertices, num_polygons);
}

/*____________________________________________________________________
|
| Function: GetObjectInfo_Layer
|
| Input: Called from GetObjectInfo_Layer()
| Output: Returns in callers variables information about an object (all
|   layers combined).
|___________________________________________________________________*/

static void GetObjectInfo_Layer (gx3dObjectLayer *layer, int *num_layers, int *num_vertices, int *num_polygons)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; layer; layer=layer->next) {
    // Process child layer/s first
    if (layer->child)
      GetObjectInfo_Layer (layer->child, num_layers, num_vertices, num_polygons);
    // Process this layer
    if (num_layers)
      *num_layers   += 1;
    if (num_vertices)
      *num_vertices += layer->num_vertices;
    if (num_polygons)
      *num_polygons += layer->num_polygons;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_DrawObject
|
| Output: Draws a 3D object, setting textures as it draws.
|
| Description:
|   Flags that can be used:
|     gx3d_DONT_SET_TEXTURES     = Textures in the object aren't used when
|                                  drawing.  Caller should manually set
|                                  textures if this option is used.
|     gx3d_DONT_SET_LOCAL_MATRIX = Local matrices in the object aren't used
|                                  when drawing.  Caller should manually
|                                  set the world matrix if this option is used.
|___________________________________________________________________*/

void gx3d_DrawObject (gx3dObject *object, unsigned flags)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

	// Update vertices (bones/morphs) and transforms
	gx3d_Object_UpdateTransforms (object);

	// Draw all layers
	Draw_Layer (object->layer, flags, false);
}

/*____________________________________________________________________
|
| Function: gx3d_DrawObjectLayer
|
| Output: Draws a 3D object layer.
|
| Description:
|   Flags that can be used:
|     gx3d_DONT_SET_TEXTURES     = Textures in the object aren't used when
|                                  drawing.  Caller should manually set
|                                  textures if this option is used.
|     gx3d_DONT_SET_LOCAL_MATRIX = Local matrices in the object aren't used
|                                  when drawing.  Caller should manually
|                                  set the world matrix if this option is used.
|
|		Caller should call gx3d_Object_UpdateTransforms() before calling this
|		routine.
|___________________________________________________________________*/

void gx3d_DrawObjectLayer (gx3dObjectLayer *layer, unsigned flags)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

	// Draw the layer
	Draw_Layer (layer, flags, true);
}

/*____________________________________________________________________
|
| Function: Draw_Layer
|
| Input: Called from gx3d_DrawObject(), gx3d_DrawObjectLayer
| Output: Draws a layer including linked layers and child layers.
|___________________________________________________________________*/

static void Draw_Layer (gx3dObjectLayer *layer, unsigned flags, bool draw_one_layer_only)
{
  int i;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

	for (; layer; layer=layer->next) {

		// Draw child layer/s first
		if (NOT draw_one_layer_only)
			if (layer->child)
				Draw_Layer (layer->child, flags, false);

    // Set textures for this layer?
    if (NOT (flags & gx3d_DONT_SET_TEXTURES))
      for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) 
        gx3d_SetTexture (i, layer->texture[i]);

    // Set local matrix for this layer?
    if (NOT (flags & gx3d_DONT_SET_LOCAL_MATRIX)) 
      gx3d_SetWorldMatrix (&(layer->transform.composite_matrix));

    // Register layer?
    if (layer->driver_data == 0) {
      if (gx_Video.register_object) 
        (*gx_Video.register_object) ( (word *)layer->polygon, 
                                             &layer->num_polygons,
                                     (float *)layer->vertex,
                                   (float **)&layer->X_vertex,
                                             &layer->num_vertices,
                                     (float *)layer->vertex_normal,
                                   (float **)&layer->X_vertex_normal,
                                      (byte *)layer->diffuse,
                                      (byte *)layer->specular,
                                    (float **)layer->tex_coords,
                                    (float **)layer->X_tex_coords,
                                    (float **)layer->tex_coords_w,
                                    (float **)layer->X_tex_coords_w,
                                      (byte *)layer->weight,
                                    (byte **)&layer->X_weight,
                                             &layer->driver_data );
    }    
    // Draw layer
    if (layer->driver_data AND gx_Video.draw_object)
      (*gx_Video.draw_object) (layer->driver_data);

		// Done?
		if (draw_one_layer_only)
			break;
	}
}

/*____________________________________________________________________
|
| Function: gx3d_Object_UpdateTransforms
|
| Input: Called from gx3d_DrawObject()
| Output: Updates all layers vertices and transforms.
|___________________________________________________________________*/

void gx3d_Object_UpdateTransforms (gx3dObject *object)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Update vertices
	Update_Layer_Vertices (object->layer);
	// Update transforms
	Update_Layer_Transforms (object->layer, &(object->transform.local_matrix), object->transform.dirty);
	object->transform.dirty = FALSE;
}

/*____________________________________________________________________
|
| Function: Update_Layer_Vertices
|
| Input: Called from gx3d_Object_UpdateTransforms()
| Output: Updates all vertices in a layer according to bone weights
|   including linked layers and child layers.
|___________________________________________________________________*/

static void Update_Layer_Vertices (gx3dObjectLayer *layer)
{
  int i, j;
  gx3dVector v, morphed_vertex;
  gx3dVertexWeight *weight;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; layer; layer=layer->next) {
    // Update child layer/s first
    if (layer->child)
      Update_Layer_Vertices (layer->child);
		
    // Skip layer with no matrix palette and no morphs
    if ((layer->matrix_palette == 0) AND (layer->num_morphs == 0)) 
      continue;

    // Create X arrays if needed
    if (layer->X_vertex == 0) {
      layer->X_vertex = (gx3dVector *) malloc (layer->num_vertices * sizeof(gx3dVector));
      if (layer->X_vertex == 0) {
        DEBUG_ERROR ("Update_Layer_Vertices(): can't allocate memory for X_vertex array")
        continue;
      }
      layer->X_vertex_normal = (gx3dVector *) malloc (layer->num_vertices * sizeof(gx3dVector));
      if (layer->X_vertex_normal == 0) {
        DEBUG_ERROR ("Update_Layer_Vertices(): can't allocate memory for X_vertex_normal array")
        continue;
      }
    }

    // Update layer morphs if needed
    Update_Layer_Morphs (layer);

/*____________________________________________________________________
|
| Update using morph only
|___________________________________________________________________*/

    if (layer->num_morphs AND (layer->matrix_palette == 0)) {
      // Any active morphs?
      if (layer->num_active_morphs) {
        for (i=0; i<layer->num_vertices; i++) 
          gx3d_AddVector (&(layer->vertex[i]), &(layer->composite_morph[i]), &(layer->X_vertex[i]));
      }
      else
        memcpy ((void *)(layer->X_vertex), (void *)(layer->vertex), layer->num_vertices * sizeof(gx3dVector));
      memcpy ((void *)(layer->X_vertex_normal), (void *)(layer->vertex_normal), layer->num_vertices * sizeof(gx3dVector));
    }

/*____________________________________________________________________
|
| Update using matrix palette (and possibly morph)
|___________________________________________________________________*/

		// Make sure this layer has a matrix palette and weight maps
//		else {
		else if (layer->matrix_palette AND layer->weight) {
      // Any active morphs?
      if (layer->num_active_morphs)
        for (i=0; i<layer->num_vertices; i++) 
          gx3d_AddVector (&(layer->vertex[i]), &(layer->composite_morph[i]), &(layer->X_vertex[i]));
//        memcpy ((void *)(layer->X_vertex), (void *)(layer->composite_morph), layer->num_vertices * sizeof(gx3dVector));
      else
//        memset ((void *)(layer->X_vertex), 0, layer->num_vertices * sizeof(gx3dVector));
        memcpy ((void *)(layer->X_vertex), (void *)(layer->vertex), layer->num_vertices * sizeof(gx3dVector));
      // Init vertex normal X array
      memset ((void *)(layer->X_vertex_normal), 0, layer->num_vertices * sizeof(gx3dVector));

      // Transform vertices using matrix palette
      if (layer->X_vertex AND layer->X_vertex_normal) {
        // Update each vertex
        for (i=0; i<layer->num_vertices; i++) {
          weight = &(layer->weight[i]);
					morphed_vertex = layer->X_vertex[i];
					memset ((void *)&(layer->X_vertex[i]), 0, sizeof(gx3dVector));
          for (j=0; j<weight->num_weights; j++) {
//            gx3d_MultiplyVectorMatrix (&(layer->vertex[i]), &(layer->matrix_palette[weight->matrix_index[j]].m), &v);
            gx3d_MultiplyVectorMatrix (&morphed_vertex, &(layer->matrix_palette[weight->matrix_index[j]].m), &v);
            layer->X_vertex[i].x += (v.x * weight->value[j]);
            layer->X_vertex[i].y += (v.y * weight->value[j]);
            layer->X_vertex[i].z += (v.z * weight->value[j]);
            gx3d_MultiplyNormalVectorMatrix (&(layer->vertex_normal[i]), &(layer->matrix_palette[weight->matrix_index[j]].m), &v);
            layer->X_vertex_normal[i].x += (v.x * weight->value[j]);
            layer->X_vertex_normal[i].y += (v.y * weight->value[j]);
            layer->X_vertex_normal[i].z += (v.z * weight->value[j]);
          }
          gx3d_NormalizeVector (&(layer->X_vertex_normal[i]), &(layer->X_vertex_normal[i]));
        }
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: Update_Layer_Morphs
|                       
| Input: Called from Update_Layer_Vertices() 
| Output: Updates composite morph as needed.
|___________________________________________________________________*/

static void Update_Layer_Morphs (gx3dObjectLayer *layer)
{
  int i, j, n;
  gx3dVector v;

  DEBUG_ASSERT (layer)

  // Any active morphs?
  if (layer->num_active_morphs)
    // Any dirty morphs?
    if (layer->morphs_dirty) {
      // Zero out composite morph
      memset ((void *)(layer->composite_morph), 0, layer->num_vertices * sizeof(gx3dVector));
      // Add active morphs
      for (i=0, n=0; (i<layer->num_morphs) AND (n<layer->num_active_morphs); i++) 
        // Is this an active morph?
        if (layer->morph[i].amount != 0) {
          // Add its offsets to the composite morph
          for (j=0; j<layer->morph[i].num_entries; j++) {
            gx3d_MultiplyScalarVector (layer->morph[i].amount, &(layer->morph[i].offset[j]), &v);
            gx3d_AddVector (&(layer->composite_morph[layer->morph[i].index[j]]), &v, 
                            &(layer->composite_morph[layer->morph[i].index[j]]));
          }
          n++;
        }
      layer->morphs_dirty = false;
    }
}

/*____________________________________________________________________
|
| Function: Update_Layer_Transforms
|
| Input: Called from gx3d_Object_UpdateTransforms()
| Output: Updates layer transforms including linked layers and child layers.
|___________________________________________________________________*/

static void Update_Layer_Transforms (gx3dObjectLayer *layer, gx3dMatrix *parent_matrix, int parent_transform_dirty)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (parent_matrix);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; layer; layer=layer->next) {       
    // Update layer transform?
    if (layer->transform.dirty OR parent_transform_dirty) { 
      // Composite matrix = local matrix * parent matrix
      gx3d_MultiplyMatrix (&(layer->transform.local_matrix), parent_matrix, &(layer->transform.composite_matrix));
      layer->transform.dirty = TRUE;
    }
    // Update child layer/s
    if (layer->child)
      Update_Layer_Transforms (layer->child, &(layer->transform.composite_matrix), layer->transform.dirty);
    // Clear local transform changes
    layer->transform.dirty = FALSE;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_GetObjectLayer
|                       
| Output: Returns the first gx3d layer that has the name or NULL if not 
|   found.  
|
| Description: Note that name is not necessarily unique amoung layers  
|   in an object.  This routine will only return first occurrence of a 
|   layer with name.  Be aware that other layers with the same name may 
|   exist within the same object.
|___________________________________________________________________*/

gx3dObjectLayer *gx3d_GetObjectLayer (gx3dObject *object, char *name)
{
  gx3dObjectLayer *the_layer = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);
  DEBUG_ASSERT (name);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (object->layer)
    the_layer = Get_Layer_With_Name (object->layer, name);

  return (the_layer);
}

/*____________________________________________________________________
|
| Function: Get_Layer_With_Name
|                       
| Input: Called from gx3d_GetObjectLayer()                                                                 
| Output: Returns the first gx3d layer that has the name or 0 if not found.
|___________________________________________________________________*/

static gx3dObjectLayer *Get_Layer_With_Name (gx3dObjectLayer *layer, char *name)
{
  gx3dObjectLayer *the_layer = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (name);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (layer) {
    // Is the input layer the one?
    if (layer->name)
      if (!strcmp(layer->name, name))
        the_layer = layer;

    // If not found, search child layers, if any
    if (the_layer == NULL) 
      if (layer->child)
        the_layer = Get_Layer_With_Name (layer->child, name);

    // If not found, search the rest of the layers on this level
    if (the_layer == NULL)
      if (layer->next)
        the_layer = Get_Layer_With_Name (layer->next, name);
  }

  return (the_layer);
}

/*____________________________________________________________________
|
| Function: gx3d_SetObjectMatrix
|                       
| Output: Sets the local transform matrix for an object.
|___________________________________________________________________*/

void gx3d_SetObjectMatrix (gx3dObject *object, gx3dMatrix *m)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);
  DEBUG_ASSERT (m);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Is the new matrix different from the current local matrix?
  if (memcmp ((void *)&(object->transform.local_matrix), (void *)m, sizeof(gx3dMatrix)) != 0) {
    // Set new local matrix
    memcpy ((void *)&(object->transform.local_matrix), (void *)m, sizeof(gx3dMatrix));
    object->transform.dirty = TRUE;
//    // Set object's dynamic boxtree, if any, to dirty
//    if (object->boxtree)
//      if (object->boxtree->type == gx3d_BOXTREE_TYPE_DYNAMIC)
//        gx3d_Boxtree_SetDirty (object->boxtree);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_SetObjectLayerMatrix
|                       
| Output: Sets the local transform matrix for a layer.
|___________________________________________________________________*/

void gx3d_SetObjectLayerMatrix (gx3dObject *object, gx3dObjectLayer *layer, gx3dMatrix *m)
{
  gx3dMatrix m1, m2;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);
  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (m);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Is the new matrix different from the current local matrix?
  if (memcmp ((void *)&(layer->transform.local_matrix), (void *)m, sizeof(gx3dMatrix)) != 0) {
    // Set new local matrix
    gx3d_GetTranslateMatrix (&m1, -layer->pivot.x, -layer->pivot.y, -layer->pivot.z);
    gx3d_GetTranslateMatrix (&m2,  layer->pivot.x,  layer->pivot.y,  layer->pivot.z);
    gx3d_MultiplyMatrix (&m1, m, &m1);
    gx3d_MultiplyMatrix (&m1, &m2, &(layer->transform.local_matrix));
    layer->transform.dirty = TRUE;
//    // Set object's dynamic boxtree, if any, to dirty
//    if (object->boxtree)
//      if (object->boxtree->type == gx3d_BOXTREE_TYPE_DYNAMIC)
//        gx3d_Boxtree_SetDirty (object->boxtree);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_TwistXObject
|
| Output: Twists an object about the X axis. twist_rate is the rate of
|   twist (in degrees) per unit length along the X axis.
|___________________________________________________________________*/

void gx3d_TwistXObject (gx3dObject *object, float twist_rate)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Twist all layers
  TwistX_Layer (object->layer, twist_rate);
}

/*____________________________________________________________________
|
| Function: TwistX_Layer
|
| Input: Called from gx3d_TwistXObject()
| Output: Twists a layer about the X axis including linked layers and 
|   child layers.
|___________________________________________________________________*/

static void TwistX_Layer (gx3dObjectLayer *layer, float twist_rate)
{
  int i;
  float angle, s, c;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; layer; layer=layer->next) {
    // Twist child layer/s first
    if (layer->child)
      TwistX_Layer (layer->child, twist_rate);

    // Create X arrays if needed
    if (layer->X_vertex == NULL) {
      layer->X_vertex        = (gx3dVector *) calloc (layer->num_vertices, sizeof(gx3dVector));
      layer->X_vertex_normal = (gx3dVector *) calloc (layer->num_vertices, sizeof(gx3dVector));
    }

    // Build the set of transformed vertices
    if (layer->X_vertex AND layer->X_vertex_normal) {
      // Convert twist rate from degrees to radians
      twist_rate *= DEGREES_TO_RADIANS;
    
      // Twist each vertex depending on its distance on the x axis
      for (i=0; i<layer->num_vertices; i++) {
        angle = layer->vertex[i].x * twist_rate;
        s = sinf (angle);
        c = cosf (angle);

        layer->X_vertex[i].x = layer->vertex[i].x;
        layer->X_vertex[i].y = (layer->vertex[i].y * c) - (layer->vertex[i].z * s);
        layer->X_vertex[i].z = (layer->vertex[i].y * s) + (layer->vertex[i].z * c);

        layer->X_vertex_normal[i].x = layer->vertex_normal[i].x;
        layer->X_vertex_normal[i].y = (layer->vertex_normal[i].y * c) - (layer->vertex_normal[i].z * s);
        layer->X_vertex_normal[i].z = (layer->vertex_normal[i].y * s) + (layer->vertex_normal[i].z * c);
        gx3d_NormalizeVector (&(layer->X_vertex_normal[i]), &(layer->X_vertex_normal[i]));
      }
    }
    // On any error, delete X arrays
    else {
      if (layer->X_vertex) {
        free (layer->X_vertex);
        layer->X_vertex = NULL;
      }
      if (layer->X_vertex_normal) {
        free (layer->X_vertex_normal);
        layer->X_vertex_normal = NULL;
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: gx3d_TwistYObject
|
| Output: Twists an object about the Y axis. twist_rate is the rate of
|   twist (in degrees) per unit length along the Y axis.
|___________________________________________________________________*/

void gx3d_TwistYObject (gx3dObject *object, float twist_rate)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Twist all layers
  TwistY_Layer (object->layer, twist_rate);
}

/*____________________________________________________________________
|
| Function: TwistY_Layer
|
| Input: Called from gx3d_TwistYObject()
| Output: Twists a layer about the Y axis including linked layers and 
|   child layers.
|___________________________________________________________________*/

static void TwistY_Layer (gx3dObjectLayer *layer, float twist_rate)
{
  int i;
  float angle, s, c;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; layer; layer=layer->next) {
    // Twist child layer/s first
    if (layer->child)
      TwistY_Layer (layer->child, twist_rate);

    // Create X arrays if needed
    if (layer->X_vertex == NULL) {
      layer->X_vertex        = (gx3dVector *) calloc (layer->num_vertices, sizeof(gx3dVector));
      layer->X_vertex_normal = (gx3dVector *) calloc (layer->num_vertices, sizeof(gx3dVector));
    }

    // Build the set of transformed vertices
    if (layer->X_vertex AND layer->X_vertex_normal) {
      // Convert twist rate from degrees to radians
      twist_rate *= DEGREES_TO_RADIANS;
    
      // Twist each vertex depending on its distance on the y axis
      for (i=0; i<layer->num_vertices; i++) {
        angle = layer->vertex[i].y * twist_rate;
        s = sinf (angle);
        c = cosf (angle);

        layer->X_vertex[i].x = (layer->vertex[i].z * s) + (layer->vertex[i].x * c);
        layer->X_vertex[i].y = layer->vertex[i].y;
        layer->X_vertex[i].z = (layer->vertex[i].z * c) - (layer->vertex[i].x * s);

        layer->X_vertex_normal[i].x = (layer->vertex_normal[i].z * s) + (layer->vertex_normal[i].x * c);
        layer->X_vertex_normal[i].y = layer->vertex_normal[i].y;
        layer->X_vertex_normal[i].z = (layer->vertex_normal[i].z * c) - (layer->vertex_normal[i].x * s);
        gx3d_NormalizeVector (&(layer->X_vertex_normal[i]), &(layer->X_vertex_normal[i]));
      }
    }
    // On any error, delete X arrays
    else {
      if (layer->X_vertex) {
        free (layer->X_vertex);
        layer->X_vertex = NULL;
      }
      if (layer->X_vertex_normal) {
        free (layer->X_vertex_normal);
        layer->X_vertex_normal = NULL;
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: gx3d_TwistZObject
|
| Output: Twists an object about the Z axis. twist_rate is the rate of
|   twist (in degrees) per unit length along the Z axis.
|___________________________________________________________________*/

void gx3d_TwistZObject (gx3dObject *object, float twist_rate)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Twist all layers
  TwistZ_Layer (object->layer, twist_rate);
}

/*____________________________________________________________________
|
| Function: TwistZ_Layer
|
| Input: Called from gx3d_TwistZObject()
| Output: Twists a layer about the Z axis including linked layers and 
|   child layers.
|___________________________________________________________________*/

static void TwistZ_Layer (gx3dObjectLayer *layer, float twist_rate)
{
  int i;
  float angle, s, c;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; layer; layer=layer->next) {
    // Twist child layer/s first
    if (layer->child)
      TwistZ_Layer (layer->child, twist_rate);

    // Create X arrays if needed
    if (layer->X_vertex == NULL) {
      layer->X_vertex        = (gx3dVector *) calloc (layer->num_vertices, sizeof(gx3dVector));
      layer->X_vertex_normal = (gx3dVector *) calloc (layer->num_vertices, sizeof(gx3dVector));
    }

    // Build the set of transformed vertices
    if (layer->X_vertex AND layer->X_vertex_normal) {
      // Convert twist rate from degrees to radians
      twist_rate *= DEGREES_TO_RADIANS;
    
      // Twist each vertex depending on its distance on the z axis
      for (i=0; i<layer->num_vertices; i++) {
        angle = layer->vertex[i].z * twist_rate;
        s = sinf (angle);
        c = cosf (angle);

        layer->X_vertex[i].x = (layer->vertex[i].x * c) - (layer->vertex[i].y * s);
        layer->X_vertex[i].y = (layer->vertex[i].x * s) + (layer->vertex[i].y * c);
        layer->X_vertex[i].z = layer->vertex[i].z;

        layer->X_vertex_normal[i].x = (layer->vertex_normal[i].x * c) - (layer->vertex_normal[i].y * s);
        layer->X_vertex_normal[i].y = (layer->vertex_normal[i].x * s) + (layer->vertex_normal[i].y * c);
        layer->X_vertex_normal[i].z = layer->vertex_normal[i].z;
        gx3d_NormalizeVector (&(layer->X_vertex_normal[i]), &(layer->X_vertex_normal[i]));
      }
    }
    // On any error, delete X arrays
    else {
      if (layer->X_vertex) {
        free (layer->X_vertex);
        layer->X_vertex = NULL;
      }
      if (layer->X_vertex_normal) {
        free (layer->X_vertex_normal);
        layer->X_vertex_normal = NULL;
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: gx3d_TransformObject
|
| Output: Permanently transforms an object using a transform matrix.
|___________________________________________________________________*/

void gx3d_TransformObject (gx3dObject *object, gx3dMatrix *m)
{
	gx3dObjectLayer *layer;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Transform all layers
  for (layer=object->layer; layer; layer=layer->next) 
    gx3d_TransformObjectLayer (layer, m);
}

/*____________________________________________________________________
|
| Function: gx3d_TransformObjectLayer
|
| Output: Permanently transforms an object layer (and any child layers)
|		using a transform matrix.
|___________________________________________________________________*/

void gx3d_TransformObjectLayer (gx3dObjectLayer *layer, gx3dMatrix *m)
{
  int i, j;
  gx3dMatrix m1, m2;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (m);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Transform child layer/s first
  if (layer->child)
    gx3d_TransformObjectLayer (layer->child, m);

  // Build composite matrix
  gx3d_GetTranslateMatrix (&m1, -layer->pivot.x, -layer->pivot.y, -layer->pivot.z);
  gx3d_GetTranslateMatrix (&m2,  layer->pivot.x,  layer->pivot.y,  layer->pivot.z);
  gx3d_MultiplyMatrix (&m1, m, &m1);
  gx3d_MultiplyMatrix (&m1, &m2, &m2);

  // Transform the vertices
  if (layer->vertex AND layer->vertex_normal) 
    // Transform each vertex
    for (i=0; i<layer->num_vertices; i++) {
      gx3d_MultiplyVectorMatrix       (&(layer->vertex[i]),        &m2, &(layer->vertex[i]));
      gx3d_MultiplyNormalVectorMatrix (&(layer->vertex_normal[i]), &m2, &(layer->vertex_normal[i]));
      gx3d_NormalizeVector (&(layer->vertex_normal[i]), &(layer->vertex_normal[i]));
    }
  // Transform any morph maps
  for (i=0; i<layer->num_morphs; i++)
    for (j=0; j<layer->morph[i].num_entries; j++) 
      gx3d_MultiplyVectorMatrix (&(layer->morph[i].offset[j]), m, &(layer->morph[i].offset[j]));
}

/*____________________________________________________________________
|
| Function: gx3d_CombineObjects
|
| Output: Adds contents of src_obj to dst_obj, using these rules:
|   1) both objects should have all layers at the root level (no child layers)
|   2) each combinable layer must have same name
|   3) each combinable layer must use same textures
|   4) each combinable layers pivot's must be equal
|   5) a source object layer can be combined with the destination object
|      but the source object layer must have a name (no unnamed layers from
|      source object will be added to the destination object)
|___________________________________________________________________*/

void gx3d_CombineObjects (gx3dObject *dst_obj, gx3dObject *src_obj)
{ 
  int i, j;
  bool found;
  gx3dObjectLayer *src_layer, *dst_layer;
  bool error = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (dst_obj);
  DEBUG_ASSERT (src_obj);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // If either object has any child layers (below root layer) then can't combine them
  for (dst_layer=dst_obj->layer; dst_layer AND (NOT error); dst_layer=dst_layer->next)
    if (dst_layer->child) 
      error = true;
  for (src_layer=src_obj->layer; src_layer AND (NOT error); src_layer=src_layer->next)
    if (src_layer->child) 
      error = true;

  // Go through root layers of src object, combining with dst object if possible
  for (src_layer=src_obj->layer; src_layer AND (NOT error); src_layer=src_layer->next) {
    // Does this layer have a name?
    if (src_layer->name) {
      // Match with a layer name in dst layer
      for (dst_layer=dst_obj->layer, found=false; dst_layer AND (NOT found); dst_layer=dst_layer->next) {
        // Does this layer have a name?
        if (dst_layer->name) {
          // Is it the same as the source layer name?
          if (!strcmp (src_layer->name, dst_layer->name)) {
            error = NOT Combine_Object_Layers (dst_layer, src_layer);
            found = true;
          }
        }
      }

      // Make a copy of the source layer in the destination
      if ((NOT found) AND (NOT error)) {
        // Add a new source layer to the destination object
        dst_layer = gx3d_CreateObjectLayer (dst_obj);
        if (dst_layer == NULL)
          error = true;
        else {
          // Create memory for layer name
          dst_layer->name = (char *) calloc (strlen(src_layer->name)+1, sizeof(char));
          if (dst_layer->name == NULL)
            error = true;
          // Create memory for vertex array
          dst_layer->vertex = (gx3dVector *) malloc (src_layer->num_vertices * sizeof(gx3dVector));
          if (dst_layer->vertex == NULL)
            error = true;
          // Create memory for vertex_normal array
          dst_layer->vertex_normal = (gx3dVector *) malloc (src_layer->num_vertices * sizeof(gx3dVector));
          if (dst_layer->vertex_normal == NULL)
            error = true;
          // Create memory for vertex color diffuse array
          dst_layer->diffuse = (gxColor *) malloc (src_layer->num_vertices * sizeof(gxColor));
          if (dst_layer->diffuse == NULL)
            error = true;
          // Create memory for vertex color specular array
          if (src_layer->specular) {
            dst_layer->specular = (gxColor *) malloc (src_layer->num_vertices * sizeof(gxColor));
            if (dst_layer->specular == NULL)
              error = true;
          }
          // Create memory for polygon array
          dst_layer->polygon = (gx3dPolygon *) malloc (src_layer->num_polygons * sizeof(gx3dPolygon));
          if (dst_layer->polygon == NULL)
            error = true;
          // Create memory for polygon normal array
          dst_layer->polygon_normal = (gx3dVector *) malloc (src_layer->num_polygons * sizeof(gx3dVector));
          if (dst_layer->polygon_normal == NULL)
            error = true;
          // Create memory for texture coords arrays
          for (i=0; i<src_layer->num_textures; i++) {
            dst_layer->tex_coords[i] = (gx3dUVCoordinate *) malloc (src_layer->num_vertices * sizeof(gx3dUVCoordinate));
            if (dst_layer->tex_coords[i] == NULL)
              error = true;
          }
          // Create memory for texture coord_w array
          for (i=0; i<src_layer->num_textures; i++) 
            if (src_layer->tex_coords_w[i]) {
              dst_layer->tex_coords_w[i] = (float *) malloc (src_layer->num_vertices * sizeof(float));
              if (dst_layer->tex_coords_w[i] == NULL)
                error = true;
            }

          // Make sure all memory allocated ok
          if (error) {
            gxError ("gx3d_CombineObjects(): Error allocating memory");
            error = true;
          }

          if (NOT error) {
            // Copy layer name
            strcpy (dst_layer->name, src_layer->name);
            // Copy vertex data
            for (i=0; i<src_layer->num_vertices; i++) {
              dst_layer->vertex       [i] = src_layer->vertex       [i];
              dst_layer->vertex_normal[i] = src_layer->vertex_normal[i];
            }
            // Fill diffuse array with default values
            if (dst_layer->diffuse)
              for (i=0; i<src_layer->num_vertices; i++) {
                dst_layer->diffuse[i].r = 255;
                dst_layer->diffuse[i].g = 255;
                dst_layer->diffuse[i].b = 255;
                dst_layer->diffuse[i].a = 255;
              }
            // Fill specular array with default values
            if (dst_layer->specular)
              for (i=0; i<src_layer->num_vertices; i++) 
                dst_layer->specular[i].index = 0;
            // Copy polygon data
            for (i=0; i<src_layer->num_polygons; i++) {
              dst_layer->polygon       [i] = src_layer->polygon       [i];
              dst_layer->polygon_normal[i] = src_layer->polygon_normal[i];
            }
            // Copy tex coords
            for (i=0; i<src_layer->num_textures; i++) 
              for (j=0; j<src_layer->num_vertices; j++) 
                dst_layer->tex_coords[i][j] = src_layer->tex_coords[i][j];
            for (i=0; i<src_layer->num_textures; i++) 
              if (dst_layer->tex_coords_w[i]) 
                for (j=0; j<src_layer->num_vertices; j++) 
                  dst_layer->tex_coords_w[i][j] = src_layer->tex_coords_w[i][j];
            // Copy textures
            for (i=0; i<src_layer->num_textures; i++) {
              dst_layer->texture[i] = src_layer->texture[i];
              Texture_AddRef ((Texture *)(dst_layer->texture[i]));
            }
            // Set new # of vertices
            dst_layer->num_vertices = src_layer->num_vertices;
            // Set new # of polygons
            dst_layer->num_polygons = src_layer->num_polygons;
            // Set pivot point
            dst_layer->pivot        = src_layer->pivot;
            // Set # of textures
            dst_layer->num_textures = src_layer->num_textures;
          }
        }
      }
    } // if (slayer...
  }

/*____________________________________________________________________
|
| Compute new bounds for the destination object
|___________________________________________________________________*/
  
  if (NOT error)
    gx3d_ComputeObjectBounds (dst_obj);

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (NOT error);
}

/*____________________________________________________________________
|
| Function: Combine_Object_Layers
|
| Output: Adds contents of src_layer to dst_layer if the two layers 
|   meet these conditions:
|   1) must use same textures
|   2) pivot's must be equal
|   3) neither can have any child layers
|___________________________________________________________________*/

static bool Combine_Object_Layers (gx3dObjectLayer *dst_layer, gx3dObjectLayer *src_layer)
{
  int i, j, n;
  bool error = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (dst_layer);
  DEBUG_ASSERT (src_layer);

/*____________________________________________________________________
|
| Look for layers to combine
|___________________________________________________________________*/

  // Use same textures?
  if (dst_layer->num_textures == src_layer->num_textures) {
    if ((dst_layer->texture[0] == src_layer->texture[0]) AND 
        (dst_layer->texture[1] == src_layer->texture[1]) AND
        (dst_layer->texture[2] == src_layer->texture[2]) AND
        (dst_layer->texture[3] == src_layer->texture[3]) AND
        (dst_layer->texture[4] == src_layer->texture[4]) AND
        (dst_layer->texture[5] == src_layer->texture[5]) AND
        (dst_layer->texture[6] == src_layer->texture[6]) AND
        (dst_layer->texture[7] == src_layer->texture[7])) {
      // Use same tex_w coords?
      if (((dst_layer->tex_coords_w[0] == 0) == (src_layer->tex_coords_w[0] == 0)) AND
          ((dst_layer->tex_coords_w[1] == 0) == (src_layer->tex_coords_w[1] == 0)) AND
          ((dst_layer->tex_coords_w[2] == 0) == (src_layer->tex_coords_w[2] == 0)) AND
          ((dst_layer->tex_coords_w[3] == 0) == (src_layer->tex_coords_w[3] == 0)) AND
          ((dst_layer->tex_coords_w[4] == 0) == (src_layer->tex_coords_w[4] == 0)) AND
          ((dst_layer->tex_coords_w[5] == 0) == (src_layer->tex_coords_w[5] == 0)) AND
          ((dst_layer->tex_coords_w[6] == 0) == (src_layer->tex_coords_w[6] == 0)) AND
          ((dst_layer->tex_coords_w[7] == 0) == (src_layer->tex_coords_w[7] == 0))) {
        // Pivot's equal?
        if ((dst_layer->pivot.x == src_layer->pivot.x) AND
            (dst_layer->pivot.y == src_layer->pivot.y) AND
            (dst_layer->pivot.z == src_layer->pivot.z)) {
          // Neither has children?
          if ((dst_layer->child == NULL) AND (src_layer->child == NULL)) {
          
/*____________________________________________________________________
|
| Combine the layers into dst_layer
|___________________________________________________________________*/

            // Expand memory for vertex array
            dst_layer->vertex = (gx3dVector *) realloc (dst_layer->vertex, (dst_layer->num_vertices + src_layer->num_vertices) * sizeof(gx3dVector));
            if (dst_layer->vertex == NULL)
              error = true;

            // Expand memory for vertex normal array
            dst_layer->vertex_normal = (gx3dVector *) realloc (dst_layer->vertex_normal, (dst_layer->num_vertices + src_layer->num_vertices) * sizeof(gx3dVector));
            if (dst_layer->vertex_normal == NULL)
              error = true;
            
            // Expand memory for vertex color diffuse array
            if (dst_layer->diffuse) {
              dst_layer->diffuse = (gxColor *) realloc (dst_layer->diffuse, (dst_layer->num_vertices + src_layer->num_vertices) * sizeof(gxColor));
              if (dst_layer->diffuse == NULL)
                error = true;
            }

            // Expand memory for vertex color specular array
            if (dst_layer->specular) {
              dst_layer->specular = (gxColor *) realloc (dst_layer->specular, (dst_layer->num_vertices + src_layer->num_vertices) * sizeof(gxColor));
              if (dst_layer->specular == NULL)
                error = true;
            }

            // Expand memory for polygon array
            dst_layer->polygon = (gx3dPolygon *) realloc (dst_layer->polygon, (dst_layer->num_polygons + src_layer->num_polygons) * sizeof(gx3dPolygon));
            if (dst_layer->polygon == NULL)
              error = true;

            // Expand memory for polygon normal array
            dst_layer->polygon_normal = (gx3dVector *) realloc (dst_layer->polygon_normal, (dst_layer->num_polygons + src_layer->num_polygons) * sizeof(gx3dVector));
            if (dst_layer->polygon_normal == NULL)
              error = true;

            // Expand memory for texture coords arrays
            for (i=0; i<dst_layer->num_textures; i++) {
              dst_layer->tex_coords[i] = (gx3dUVCoordinate *) realloc (dst_layer->tex_coords[i], (dst_layer->num_vertices + src_layer->num_vertices) * sizeof(gx3dUVCoordinate));
              if (dst_layer->tex_coords[i] == NULL)
                error = true;
            }

            // Create memory for texture coord_w array
            for (i=0; i<dst_layer->num_textures; i++) {
              if (dst_layer->tex_coords_w[i]) {
                dst_layer->tex_coords_w[i] = (float *) realloc (dst_layer->tex_coords_w[i], (dst_layer->num_vertices + src_layer->num_vertices) * sizeof(float));
                if (dst_layer->tex_coords_w[i] == NULL)
                  error = true;
              }
            }

            // Make sure all memory allocated ok
            if (error) {
              gxError ("Combine_GX3D_Layers(): Error allocating memory");
              error = true;
            }

            if (NOT error) {
              // Copy vertex data
              n = dst_layer->num_vertices;
              for (i=0; i<src_layer->num_vertices; i++) {
                dst_layer->vertex       [n+i] = src_layer->vertex       [i];
                dst_layer->vertex_normal[n+i] = src_layer->vertex_normal[i];
              }
              // Fill diffuse array with default values
              if (dst_layer->diffuse)
                for (i=0; i<src_layer->num_vertices; i++) {
                  dst_layer->diffuse[n+i].r = 255;
                  dst_layer->diffuse[n+i].g = 255;
                  dst_layer->diffuse[n+i].b = 255;
                  dst_layer->diffuse[n+i].a = 255;
                }
              // Fill specular array with default values
              if (dst_layer->specular)
                for (i=0; i<src_layer->num_vertices; i++) 
                  dst_layer->specular[n+i].index = 0;
              // Copy polygon data
              n = dst_layer->num_polygons;
              for (i=0; i<src_layer->num_polygons; i++) {
                dst_layer->polygon[n+i].index[0] = src_layer->polygon[i].index[0] + dst_layer->num_vertices;
                dst_layer->polygon[n+i].index[1] = src_layer->polygon[i].index[1] + dst_layer->num_vertices;
                dst_layer->polygon[n+i].index[2] = src_layer->polygon[i].index[2] + dst_layer->num_vertices;
                dst_layer->polygon_normal[n+i]   = src_layer->polygon_normal[i];
              }
              // Copy tex coords
              n = dst_layer->num_vertices;
              for (i=0; i<dst_layer->num_textures; i++) 
                for (j=0; j<src_layer->num_vertices; j++) 
                  dst_layer->tex_coords[i][n+j] = src_layer->tex_coords[i][j];
              for (i=0; i<dst_layer->num_textures; i++) 
                if (dst_layer->tex_coords_w[i]) 
                  for (j=0; j<src_layer->num_vertices; j++) 
                    dst_layer->tex_coords_w[i][n+j] = src_layer->tex_coords_w[i][j];

              // Set new # of vertices
              dst_layer->num_vertices += src_layer->num_vertices;
              // Set new # of polygons
              dst_layer->num_polygons += src_layer->num_polygons;
            }
          }
        }
      }
    }
  }

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (NOT error);

  return (NOT error);
}

/*____________________________________________________________________
|
| Function: gx3d_ReadLWO2File
|                                                                                        
| Output: Reads a LWO2 object file and creates a gx3dObject.
|
| Description:
|   vertex_format can be any combination of gx3d_VERTEXFORMAT... flags
|   or just gx3d_VERTEXFORMAT_DEFAULT
|
|   flags that can be used:
|     gx3d_DONT_COMBINE_LAYERS    
|         Won't combine any lwo2 layers when creating the gx3d object.
|     gx3d_DONT_GENERATE_MIPMAPS  
|         Will only generate a 1-level texture
|     gx3d_SMOOTH_DISCONTINUOUS_VERTICES 
|         Polygons that aren't attached get averaged in when computing 
|         the vertex normals if vertices have the same position.
|     gx3d_MERGE_DUPLICATE_VERTICES
|         Merges any duplicate vertices (same position, tex coords, etc.)
|___________________________________________________________________*/

void gx3d_ReadLWO2File (
  char        *filename, 
  gx3dObject **object, 
  unsigned     vertex_format, // vertex format flags
  unsigned     flags )        // creation flags            
{
	int i;
  char *str;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (filename);
  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create a new empty gx3d object
  *object = gx3d_CreateObject ();
  if (*object) {
    // Set object name to filename (minus file extension)
    str = (char *) malloc (strlen(filename) + 1);
    if (str) {
      Extract_Filename_Minus_Extension (filename, str);
      (*object)->name = (char *) malloc (strlen(str) + 1);
      strcpy ((*object)->name, str);
      free (str);
    }
    // Convert the lwo2 file to a gx3d object
    if (NOT LWO2_File_To_GX3D_Object (filename, *object, vertex_format, flags, Free_Layer)) {
      // On error, free the gx3d object
      gx3d_FreeObject (*object);
      *object = 0;
      // And exit with an error message
      char str1[200], str2[100];
      for (i=0; filename[i] AND (i<100-1); i++)
        str2[i] = filename[i];
      str2[i] = 0;
      strcpy (str1, "Can't load LWO2 file: ");
      strcat (str1, str2);
      TERMINAL_ERROR (str1)
    }
  }

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (*object);
}

/*____________________________________________________________________
|
| Function: gx3d_WriteLWO2File
|                                                                                        
| Output: Writes a LWO2 file from a gx3d object.
|___________________________________________________________________*/
                         
void gx3d_WriteLWO2File (char *filename, gx3dObject *object)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (filename);
  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  GX3D_Object_To_LWO2_File (object, filename);
}

/*____________________________________________________________________
|
| Function: gx3d_WriteGX3DBINFile
|                                                                                        
| Output: Writes a GX3DBIN file from a gx3d object.
|___________________________________________________________________*/
                         
void gx3d_WriteGX3DBINFile (
  char       *filename, 
  gx3dObject *object, 
  bool        output_texcoords,
  bool        output_vertex_normals, 
  bool        output_diffuse_color,
  bool        output_specular_color,
  bool        output_weights,
  bool        output_morphs,
  bool        output_skeleton,
  bool        opengl_formatting, 
  bool        write_textfile_version )
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (filename);
  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  GX3D_Object_To_GX3DBIN_File (filename,
                               object,
                               output_texcoords,
                               output_vertex_normals,
                               output_diffuse_color,
                               output_specular_color,
                               output_weights,
                               output_morphs,
                               output_skeleton,
                               opengl_formatting, 
                               write_textfile_version );
}

/*____________________________________________________________________
|
| Function: gx3d_ReadGX3DBINFile
|                                                                                        
| Output: Reads a GX3DBIN object file and creates a gx3dObject.
|
| Description:
|   vertex_format can be any combination of gx3d_VERTEXFORMAT... flags
|   or just gx3d_VERTEXFORMAT_DEFAULT
|
|   flags that can be used:
|     gx3d_DONT_COMBINE_LAYERS    
|         Won't combine any lwo2 layers when creating the gx3d object.
|     gx3d_DONT_GENERATE_MIPMAPS  
|         Will only generate a 1-level texture
|     gx3d_SMOOTH_DISCONTINUOUS_VERTICES 
|         Polygons that aren't attached get averaged in when computing 
|         the vertex normals if vertices have the same position.
|     gx3d_MERGE_DUPLICATE_VERTICES
|         Merges any duplicate vertices (same position, tex coords, etc.)
|___________________________________________________________________*/

void gx3d_ReadGX3DBINFile (
  char        *filename, 
  gx3dObject **object, 
  unsigned     vertex_format, // vertex format flags
  unsigned     flags )        // creation flags            
{
	int i;
  char *str;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (filename);
  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Create a new empty gx3d object
  *object = gx3d_CreateObject ();
  if (*object) {
    // Set object name to filename (minus file extension)
    str = (char *) malloc (strlen(filename) + 1);
    if (str) {
      Extract_Filename_Minus_Extension (filename, str);
      (*object)->name = (char *) malloc (strlen(str) + 1);
      strcpy ((*object)->name, str);
      free (str);
    }
    // Convert the gx3dbin file to a gx3d object
    if (NOT GX3DBIN_File_TO_GX3D_Object (filename, *object, vertex_format, flags, Free_Layer)) {
      // On error, free the gx3d object
      gx3d_FreeObject (*object);
      *object = 0;
      // And exit with an error message
      char str1[200], str2[100];
      for (i=0; filename[i] AND (i<100-1); i++)
        str2[i] = filename[i];
      str2[i] = 0;
      strcpy (str1, "Can't load LWO2 file: ");
      strcat (str1, str2);
      TERMINAL_ERROR (str1)
    }
  }

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (*object);
}

/*____________________________________________________________________
|
| Function: gx3d_ObjectBoundBoxVisible
|                                                                                        
| Output: Returns nonzero (see below) if any part of objects bounding 
|   box is within view frustum, else returns 0.  Assumes the bounding 
|   box data in the object is valid.
|
|   Returns gxRESULT_OUTSIDE      = object outside of VF
|           gxRESULT_INTERSECTING = object intersects VF
|           gxRESULT_INSIDE       = object is entirely within VF
|             (can turn off clipping when drawing the object!)
|___________________________________________________________________*/
 
gxRelation gx3d_ObjectBoundBoxVisible (gx3dObject *object)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  return (gx3d_Relation_Box_Frustum (&object->bound_box, &(object->transform.local_matrix)));
}

/*____________________________________________________________________
|
| Function: gx3d_ObjectBoundSphereVisible
|                                                                                        
| Output: Returns nonzero (see below) if any part of objects bounding 
|   sphere is within view frustum, else returns 0.  Assumes the bounding 
|   sphere data in the object is valid.
|
|   Returns gxRESULT_OUTSIDE      = object outside of VF
|           gxRESULT_INTERSECTING = object intersects VF
|           gxRESULT_INSIDE       = object is entirely within VF
|
| Notes: Assumes the local transform matrix for this object will use
|   only uniform scalings (no stretching/shearing).
|___________________________________________________________________*/

gxRelation gx3d_ObjectBoundSphereVisible (gx3dObject *object)
{
  float x, y;
  gx3dSphere sphere;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Transform center of object bounding sphere into world space
  gx3d_MultiplyVectorMatrix (&(object->bound_sphere.center), &(object->transform.local_matrix), &sphere.center);
  // Set sphere radius (in world units)
  x = object->bound_sphere.radius * object->transform.local_matrix._00;
  y = object->bound_sphere.radius * object->transform.local_matrix._01;
  sphere.radius = sqrtf (x * x + y * y);

  return (gx3d_Relation_Sphere_Frustum (&sphere));
}

/*____________________________________________________________________
|
| Function: gx3d_MakeDoubleSidedObject
|                                                                                        
| Output: For every polygon in the object, adds a polygon facing the 
|   opposite direction.
|___________________________________________________________________*/
 
void gx3d_MakeDoubleSidedObject (gx3dObject *object)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  MakeDoubleSided_Layer (object->layer);
}

/*____________________________________________________________________
|
| Function: MakeDoubleSided_Layer
|
| Input: Called from gx3d_MakeDoubleSidedObject()
| Output: Makes the polygons of a layer double sided including linked 
|   layers and child layers.
|___________________________________________________________________*/

static void MakeDoubleSided_Layer (gx3dObjectLayer *layer)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; layer; layer=layer->next) {
    // Process child layer/s first
    if (layer->child)
      MakeDoubleSided_Layer (layer->child);
    // Process this layer
    gx3d_MakeDoubleSidedObjectLayer (layer);
  }
}

/*____________________________________________________________________
|
| Function: gx3d_MakeDoubleSidedObjectLayer
|
| Output: For every polygon in the layer, adds a polygon facing the 
|   opposite direction.
|___________________________________________________________________*/

void gx3d_MakeDoubleSidedObjectLayer (gx3dObjectLayer *layer)
{
  int i;
  bool error;
  float *tf;
  gx3dVector *vp;
  gxColor *cp;
  gx3dVertexWeight *wp;
  gx3dUVCoordinate *tp;
  gx3dPolygon *pp;
  
/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Unregister the layer
|___________________________________________________________________*/

  // Unregister this layer
  if (layer->driver_data) 
    if (gx_Video.unregister_object) {
      (*gx_Video.unregister_object) (layer->driver_data);
      layer->driver_data = 0;
    }

/*____________________________________________________________________
|
| Make this layer double sided
|___________________________________________________________________*/

  error = false;

  // Double vertex array
  if (NOT error) {
    vp = (gx3dVector *) realloc (layer->vertex, layer->num_vertices * 2 * sizeof(gx3dVector));
    if (vp == NULL)
      error = true;
    else {
      layer->vertex = vp;
      // Set values in second half to same as first half
      memcpy (&(layer->vertex[layer->num_vertices]), layer->vertex, layer->num_vertices * sizeof(gx3dVector));
    }
  }
  // Double X_vertex array
  if (layer->X_vertex AND (NOT error)) {
    vp = (gx3dVector *) realloc (layer->X_vertex, layer->num_vertices * 2 * sizeof(gx3dVector));
    if (vp == NULL)
      error = true;
    else {
      layer->X_vertex = vp;
      // Set values in second half to same as first half
      memcpy (&(layer->X_vertex[layer->num_vertices]), layer->X_vertex, layer->num_vertices * sizeof(gx3dVector));
    }
  }
  // Double vertex_normal array
  if (NOT error) {
    vp = (gx3dVector *) realloc (layer->vertex_normal, layer->num_vertices * 2 * sizeof(gx3dVector));
    if (vp == NULL)
      error = true;
    else {
      layer->vertex_normal = vp;
      // Set values in second half to same as first half except pointing in opposite direction
      for (i=0; i<layer->num_vertices; i++) {
        layer->vertex_normal[layer->num_vertices+i].x = -layer->vertex_normal[i].x;
        layer->vertex_normal[layer->num_vertices+i].y = -layer->vertex_normal[i].y;
        layer->vertex_normal[layer->num_vertices+i].z = -layer->vertex_normal[i].z;
      }
    }
  }
  // Double X_vertex_normal array
  if (layer->X_vertex_normal AND (NOT error)) {
    vp = (gx3dVector *) realloc (layer->X_vertex_normal, layer->num_vertices * 2 * sizeof(gx3dVector));
    if (vp == NULL)
      error = true;
    else {
      layer->X_vertex_normal = vp;
      // Set values in second half to same as first half except pointing in opposite direction
      for (i=0; i<layer->num_vertices; i++) {
        layer->X_vertex_normal[layer->num_vertices+i].x = -layer->X_vertex_normal[i].x;
        layer->X_vertex_normal[layer->num_vertices+i].y = -layer->X_vertex_normal[i].y;
        layer->X_vertex_normal[layer->num_vertices+i].z = -layer->X_vertex_normal[i].z;
      }
    }
  }
  // Double diffuse color array
  if (layer->diffuse AND (NOT error)) {
    cp = (gxColor *) realloc (layer->diffuse, layer->num_vertices * 2 * sizeof(gxColor));
    if (cp == NULL)
      error = true;
    else {
      layer->diffuse = cp;
      // Set values in second half to same as first half
      memcpy (&(layer->diffuse[layer->num_vertices]), layer->diffuse, layer->num_vertices * sizeof(gxColor));
    }
  }
  // Double specular color array
  if (layer->specular AND (NOT error)) {
    cp = (gxColor *) realloc (layer->specular, layer->num_vertices * 2 * sizeof(gxColor));
    if (cp == NULL)
      error = true;
    else {
      layer->specular = cp;
      // Set values in second half to same as first half
      memcpy (&(layer->specular[layer->num_vertices]), layer->specular, layer->num_vertices * sizeof(gxColor));
    }
  }
  // Double weight array
  if (layer->weight AND (NOT error)) {
    wp = (gx3dVertexWeight *) realloc (layer->weight, layer->num_vertices * 2 * sizeof(gx3dVertexWeight));
    if (wp == NULL)
      error = true;
    else {
      layer->weight = wp;
      // Set values in second half to same as first half
      memcpy (&(layer->weight[layer->num_vertices]), layer->weight, layer->num_vertices * sizeof(gx3dVertexWeight));
    }
  }
  // Double X_weight array
  if (layer->X_weight AND (NOT error)) {
    wp = (gx3dVertexWeight *) realloc (layer->X_weight, layer->num_vertices * 2 * sizeof(gx3dVertexWeight));
    if (wp == NULL)
      error = true;
    else {
      layer->X_weight = wp;
      // Set values in second half to same as first half
      memcpy (&(layer->X_weight[layer->num_vertices]), layer->X_weight, layer->num_vertices * sizeof(gx3dVertexWeight));
    }
  }
  // Double texture coord arrays
  for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) { 
    if (layer->tex_coords[i] AND (NOT error)) {
      tp = (gx3dUVCoordinate *) realloc (layer->tex_coords[i], layer->num_vertices * 2 * sizeof(gx3dUVCoordinate));
      if (tp == NULL)
        error = true;
      else {
        layer->tex_coords[i] = tp;
        // Set values in second half to same as first half
        memcpy (&(layer->tex_coords[i][layer->num_vertices]), layer->tex_coords[i], layer->num_vertices * sizeof(gx3dUVCoordinate));
      }
    }
    if (layer->tex_coords_w[i] AND (NOT error)) {
      tf = (float *) realloc (layer->tex_coords_w[i], layer->num_vertices * 2 * sizeof(float));
      if (tf == NULL)
        error = true;
      else {
        layer->tex_coords_w[i] = tf;
        // Set values in second half to same as first half
        memcpy (&(layer->tex_coords_w[i][layer->num_vertices]), layer->tex_coords_w[i], layer->num_vertices * sizeof(float));
      }
    }
  }
  // Double X_texture_coord arrays
  for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) {
    if (layer->X_tex_coords[i] AND (NOT error)) {
      tp = (gx3dUVCoordinate *) realloc (layer->X_tex_coords[i], layer->num_vertices * 2 * sizeof(gx3dUVCoordinate));
      if (tp == NULL)
        error = true;
      else {
        layer->X_tex_coords[i] = tp;
        // Set values in second half to same as first half
        memcpy (&(layer->X_tex_coords[i][layer->num_vertices]), layer->X_tex_coords[i], layer->num_vertices * sizeof(gx3dUVCoordinate));
      }
    }
    if (layer->X_tex_coords_w[i] AND (NOT error)) {
      tf = (float *) realloc (layer->X_tex_coords_w[i], layer->num_vertices * 2 * sizeof(float));
      if (tf == NULL)
        error = true;
      else {
        layer->X_tex_coords_w[i] = tf;
        // Set values in second half to same as first half
        memcpy (&(layer->X_tex_coords_w[i][layer->num_vertices]), layer->X_tex_coords_w[i], layer->num_vertices * sizeof(float));
      }
    }
  }
  // Double polygon array
  if (NOT error) {
    pp = (gx3dPolygon *) realloc (layer->polygon, layer->num_polygons * 2 * sizeof(gx3dPolygon));
    if (pp == NULL)
      error = true;
    else {
      layer->polygon = pp;
      // Set values in second half to same as first half except in opposite winding order
      for (i=0; i<layer->num_polygons; i++) {
        layer->polygon[layer->num_polygons+i].index[0] = layer->polygon[i].index[0];
        layer->polygon[layer->num_polygons+i].index[1] = layer->polygon[i].index[2];
        layer->polygon[layer->num_polygons+i].index[2] = layer->polygon[i].index[1];
      }
    }
  }
  // Double polygon normal array
  if (layer->polygon_normal AND (NOT error)) {
    vp = (gx3dVector *) realloc (layer->polygon_normal, layer->num_polygons * 2 * sizeof(gx3dVector));
    if (vp == NULL)
      error = true;
    else {
      layer->polygon_normal = vp;
      // Set values in second half to same as first half except pointing in opposite direction
      for (i=0; i<layer->num_polygons; i++) {
        layer->polygon_normal[layer->num_polygons+i].x = -layer->polygon_normal[i].x;
        layer->polygon_normal[layer->num_polygons+i].y = -layer->polygon_normal[i].y;
        layer->polygon_normal[layer->num_polygons+i].z = -layer->polygon_normal[i].z;
      }
    }
  }

  // Set new # vertices/polygons
  if (NOT error) {
    layer->num_vertices *= 2;
    layer->num_polygons *= 2;
  }
  else
    debug_WriteFile ("gx3d_MakeDoubleSidedObjectLayer(): Error allocating memory");

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (NOT error);
}

/*____________________________________________________________________
|
| Function: gx3d_ComputeVertexNormals
|
| Output: Computes and sets all vertex normals for a 3D object.
|
| Description:
|   Flags that can be used:
|     gx3d_SMOOTH_DISCONTINUOUS_VERTICES 
|         Polygons that aren't attached get averaged in when computing 
|         the vertex normals if vertices have the same position.
|
|       The default is any polygon that is adjacent to a vertex gets
|       averaged in when computing a vertex normal.  In some cases
|       polygons have separate vertices but the vertices are located 
|       at the same coordinate so the polygon is counted for smoothing
|       purposes.
|___________________________________________________________________*/

void gx3d_ComputeVertexNormals (gx3dObject *object, unsigned flags)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Compute vertex normals for all layers
|___________________________________________________________________*/

  ComputeVertexNormal_Layer (object->layer, flags);
}

/*____________________________________________________________________
|
| Function: ComputeVertexNormal_Layer
|
| Input: Called from gx3d_ComputeVertexNormals()
| Output: Computes vertex normals for a layer including linked layers 
|   and child layers.  Returns true on success, else false on any error. 
|
| Notes: Allocates memory for the vertex_normal array if needed.
|___________________________________________________________________*/

static void ComputeVertexNormal_Layer (gx3dObjectLayer *layer, unsigned flags)
{
  int i, j, k, poly_count;
  float f;
  bool error = false;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  for (; layer AND (NOT error); layer=layer->next) {
  
    // Process child layer/s first
    if (layer->child)
      ComputeVertexNormal_Layer (layer->child, flags);

    // Allocate memory for vertex normal array?
    if (layer->vertex_normal == NULL) {
      layer->vertex_normal = (gx3dVector *) malloc (layer->num_vertices * sizeof(gx3dVector));
      if (layer->vertex_normal  == NULL) {
        gxError ("ComputeVertexNormal_Layer(): Error allocating memory");
        error = true;
      }
    }

    // Compute a vertex normal for each vertex
    for (i=0; (i<layer->num_vertices) AND (NOT error); i++) {
      // Init variables;
      layer->vertex_normal[i].x = 0;
      layer->vertex_normal[i].y = 0;
      layer->vertex_normal[i].z = 0;
      poly_count = 0;

      if (flags & gx3d_SMOOTH_DISCONTINUOUS_VERTICES) {
        // Search each polygon to see if adjacent to this vertex (has a vertex with the same value as the vertex)
        for (j=0; j<layer->num_polygons; j++) 
          for (k=0; k<3; k++) 
            if ((!memcmp (&(layer->vertex[layer->polygon[j].index[k]]), &(layer->vertex[i]), sizeof(gx3dVector))) OR
                (!memcmp (&(layer->vertex[layer->polygon[j].index[k]]), &(layer->vertex[i]), sizeof(gx3dVector))) OR
                (!memcmp (&(layer->vertex[layer->polygon[j].index[k]]), &(layer->vertex[i]), sizeof(gx3dVector)))) {
              layer->vertex_normal[i].x += layer->polygon_normal[j].x;
              layer->vertex_normal[i].y += layer->polygon_normal[j].y;
              layer->vertex_normal[i].z += layer->polygon_normal[j].z;
              poly_count++;
              break;
            }
      }
      else {
        // Search each polygon to see if directly connected to this vertex (search each polygon for vertex i)
        for (j=0; j<layer->num_polygons; j++) 
          for (k=0; k<3; k++)
            if (layer->polygon[j].index[k] == i) {
              layer->vertex_normal[i].x += layer->polygon_normal[j].x;
              layer->vertex_normal[i].y += layer->polygon_normal[j].y;
              layer->vertex_normal[i].z += layer->polygon_normal[j].z;
              poly_count++;
              break;
            }
      }

      // Compute the normal   
      if (poly_count) {
        f = (float)1 / (float)poly_count;
        layer->vertex_normal[i].x *= f;
        layer->vertex_normal[i].y *= f;
        layer->vertex_normal[i].z *= f;
      }
      // Normalize to get the vertex normal
      gx3d_NormalizeVector (&(layer->vertex_normal[i]), &(layer->vertex_normal[i]));
    }
  }

/*____________________________________________________________________
|
| Verify output params
|___________________________________________________________________*/

  DEBUG_ASSERT (NOT error);
}

/*____________________________________________________________________
|
| Function: gx3d_ComputeObjectBounds
|                       
| Input: Called from ____
| Output: Computes bounding box and bounding sphere for the gx3d object 
|   and all layers in the object.
|___________________________________________________________________*/

void gx3d_ComputeObjectBounds (gx3dObject *object)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (object);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Object must have at least one layer
  if (object->layer) {

    // Set min and max object box bounds to values of first vertex in the first layer
    object->bound_box.min = object->layer->vertex[0];
    object->bound_box.max = object->layer->vertex[0];

    // Compute bounding box for each layer and for object
    Compute_Bounding_Box (object->layer, &object->bound_box);

    // Init bound sphere for object
    gx3d_GetBoundBoxCenter (&object->bound_box, &(object->bound_sphere.center));
    object->bound_sphere.radius = 0;

    // Compute bounding sphere for each layer and for object
    Compute_Optimal_Bounding_Sphere (object->layer, &object->bound_sphere);
  }
}

/*____________________________________________________________________
|
| Function: Compute_Bounding_Box
|                       
| Input: Called from gx3d_ComputeObjectBounds()                                                                 
| Output: Computes bounding box for all gx3d object layers.  Also expands
|   the parent object bounding box if needed to enclose the layer 
|   bounding box.
|___________________________________________________________________*/

static void Compute_Bounding_Box (gx3dObjectLayer *layer, gx3dBox *object_box)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (object_box);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute bounds, one layer at a time
  for ( ; layer; layer=layer->next) {
    // Compute bounding box
    gx3d_GetBoundBox (&layer->bound_box, layer->vertex, layer->num_vertices);
    // Update object bounding box as needed
    gx3d_EncloseBoundBox (object_box, &layer->bound_box);
    // Compute bounding box for child layers
    if (layer->child)
      Compute_Bounding_Box (layer->child, object_box);
  }    
}

/*____________________________________________________________________
|
| Function: Compute_Bounding_Sphere
|                       
| Input: Called from gx3d_ComputeObjectBounds()                                                                 
| Output: Computes bounding sphere for all gx3d object layers.  Also 
|   expands the parent object bounding sphere if needed to enclose the 
|   layer bounding sphere.
|___________________________________________________________________*/

static void Compute_Bounding_Sphere (gx3dObjectLayer *layer, gx3dSphere *object_sphere)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (object_sphere);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute bounding sphere, one layer at a time
  for ( ; layer; layer=layer->next) {
    // Compute bounding sphere for the layer
    gx3d_GetBoundSphere (&layer->bound_sphere, layer->vertex, layer->num_vertices, &layer->bound_box);
    // Update object bounding sphere as needed
    gx3d_EncloseBoundSphere (object_sphere, layer->vertex, layer->num_vertices);
    // Compute bounding sphere for child layers
    if (layer->child)
      Compute_Bounding_Sphere (layer->child, object_sphere);
  }    
}

/*____________________________________________________________________
|
| Function: Compute_Optimal_Bounding_Sphere
|                       
| Input: Called from gx3d_ComputeObjectBounds()                                                                 
| Output: Computes optimal bounding sphere for all gx3d object layers. 
|   Also expands the parent object bounding sphere if needed to enclose 
|   the layer bounding sphere.
|
|   This algorithm should do a better job than the other function in
|   this file, although note that in this function the object bounding
|   sphere is still using the older method.
|___________________________________________________________________*/

static void Compute_Optimal_Bounding_Sphere (gx3dObjectLayer *layer, gx3dSphere *object_sphere)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (object_sphere);

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Compute bounding sphere, one layer at a time
  for ( ; layer; layer=layer->next) {
    // Get bounding sphere for the layer
    gx3d_GetOptimalBoundSphere (&layer->bound_sphere, layer->vertex, layer->num_vertices);
    // Update object bounding sphere as needed
    gx3d_EncloseBoundSphere (object_sphere, layer->vertex, layer->num_vertices);
    // Compute bounding sphere for child layers
    if (layer->child)
      Compute_Optimal_Bounding_Sphere (layer->child, object_sphere);
  }    
}

/*____________________________________________________________________
|
| Function: gx3d_GetMorph
|                       
| Output: Returns morph index or -1 if not found.
|___________________________________________________________________*/

gx3dMorphIndex gx3d_GetMorph (gx3dObjectLayer *layer, char *morph_name)
{
  int i;
  gx3dMorphIndex morph_index = -1;  // assume not found

  DEBUG_ASSERT (layer)
//  DEBUG_ASSERT (layer->num_morphs >= 1)	// the layer might not necessarily have any morphs!
  DEBUG_ASSERT (morph_name)

  // Find index in morph array of the morph with this name
	for (i=0; i<layer->num_morphs; i++) {
// DEBUG CODE
		DEBUG_ASSERT (layer->morph[i].name);
////////////
		if (!strcmp(morph_name, layer->morph[i].name)) {
      morph_index = i;
      break;
    }
	}

  //if (morph_index == -1) {
  //  DEBUG_WRITE (morph_name)
  //  DEBUG_ERROR ("gx3d_GetMorph(): can't find morph by name")
  //}

  return (morph_index);
}

/*____________________________________________________________________
|
| Function: gx3d_SetMorphAmount
|                       
| Output: Set the amount of a morph (0-1), 0=disable the morph.
|___________________________________________________________________*/

void gx3d_SetMorphAmount (gx3dObjectLayer *layer, char *morph_name, float amount)
{
  gx3dMorphIndex morph_index;

  DEBUG_ASSERT (layer)
//  DEBUG_ASSERT (layer->num_morphs >= 1)	// the layer might not necessarily have any morphs!
  DEBUG_ASSERT (morph_name)
  DEBUG_ASSERT (amount >= 0)

  // Get the index for this morph
  morph_index = gx3d_GetMorph (layer, morph_name);
  // If found, set the amount of morph
  if (morph_index != -1)
    gx3d_SetMorphAmount (layer, morph_index, amount);
}

/*____________________________________________________________________
|
| Function: gx3d_SetMorphAmount
|                       
| Output: Set the amount of a morph (0-1), 0=disable the morph.
|___________________________________________________________________*/

void gx3d_SetMorphAmount (gx3dObjectLayer *layer, gx3dMorphIndex morph_index, float amount)
{
  DEBUG_ASSERT (layer)
  DEBUG_ASSERT (layer->num_morphs >= 1)
  DEBUG_ASSERT ((morph_index >= 0) AND (morph_index < layer->num_morphs))
  DEBUG_ASSERT (amount >= 0)
  
  // Enabling a morph?
  if ((layer->morph[morph_index].amount == 0) AND (amount != 0))
    layer->num_active_morphs++;
  // Disabling a morph?
  else if ((layer->morph[morph_index].amount != 0) AND (amount == 0))
    layer->num_active_morphs--;

  // Set new amount?
  if (layer->morph[morph_index].amount != amount) {
    layer->morph[morph_index].amount = amount;
    layer->morphs_dirty = true;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_SetMorphAmount
|                       
| Output: Set the amount of a morph (0-1) for all layers in the object.
|   0=disable the morph.
|___________________________________________________________________*/

void gx3d_SetMorphAmount (gx3dObject *object, char *morph_name, float amount)
{
  DEBUG_ASSERT (object)
  DEBUG_ASSERT (morph_name)
  DEBUG_ASSERT (amount >= 0)

  Set_Morph_Amount (object->layer, morph_name, amount);
}

/*____________________________________________________________________
|
| Function: Set_Morph_Amount
|                       
| Input: Called from gx3d_SetMorphAmount()
| Output: Set the amount of a morph (0-1) for layer and all child and 
|   sibling layers.
|___________________________________________________________________*/

static void Set_Morph_Amount (gx3dObjectLayer *layer, char *morph_name, float amount)
{
  while (layer) {
    // Process child layer/s first
    if (layer->child)
      Set_Morph_Amount (layer->child, morph_name, amount);
    // Process this layer
    gx3d_SetMorphAmount (layer, morph_name, amount);
    // Goto next layer in the list
    layer = layer->next;
  }
}
