/*____________________________________________________________________
|
| File: gx3d_lwo2.cpp
|
| Description: Functions to read/write LWO2 object files.
|
| Functions:  LWO2_File_To_GX3D_Object
|              Verify_LWO2_Object
|              Process_Geometry_Layer
|               Get_LWO2_Texture_Filenames
|               Get_Parent_GX3D_Layer
|              Process_Skeleton_Layer
|              Combine_GX3D_Layers
|              Remove_GX3D_Duplicates
|              Compute_GX3D_Polygon_Normals 
|             GX3D_Object_To_LWO2_File
|              GX3D_To_LWO2
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

//#define DEBUG

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <math.h>
#include "dp.h"
#include "lwo2.h"
#include "texture.h"

#include "gx3d_lwo2.h"

/*___________________
|
| Function prototypes
|__________________*/

static int Verify_LWO2_Object (lwo2_Object *l_object, unsigned vertex_format_flags, unsigned flags);
static void Process_Geometry_Layer (
  lwo2_Object *l_object,
  lwo2_Layer  *l_layer,
  gx3dObject  *g_object, 
  unsigned     vertex_format_flags,
  unsigned     flags,
  int         *error );
static int Get_LWO2_Texture_Filenames (
  lwo2_Object  *l_object, 
  lwo2_Layer   *l_layer, 
  char        **texture_filename, 
  char        **alpha_texture_filename, 
  char        **vmap_name );
static gx3dObjectLayer *Get_Parent_GX3D_Layer (gx3dObjectLayer *layer, int parent_id);
static void Process_Skeleton_Layer (
  lwo2_Object *l_object,
  lwo2_Layer  *l_layer,
  gx3dObject  *g_object, 
  unsigned     vertex_format_flags,
  unsigned     flags,
  int         *error );
static int Combine_GX3D_Layers (gx3dObjectLayer *layer, void (*free_layer) (gx3dObjectLayer *layer));
static int Remove_GX3D_Duplicates (gx3dObjectLayer *layer);
static int Compute_GX3D_Polygon_Normals (gx3dObjectLayer *layer);
static lwo2_Object *GX3D_To_LWO2 (gx3dObject *g_object);

/*___________________
|
| Constants
|__________________*/

// If vertex_format_flags has specified VERTEXFORMAT_TEXCOORDS, then tex coords are required from the LWO file!
#define OBJECT_REQUIRES_TEXCOORDS (vertex_format_flags & gx3d_VERTEXFORMAT_TEXCOORDS)

/*____________________________________________________________________
|
| Function: LWO2_File_To_GX3D_Object
|                       
| Input: Called from gx3d_ReadLWO2File()                                                                 
| Output: Reads data from a LWO2 File and puts it in a gx3d object.
|___________________________________________________________________*/

int LWO2_File_To_GX3D_Object  (
  char        *filename, 
  gx3dObject  *g_object, 
  unsigned     vertex_format_flags,
  unsigned     flags,
  void       (*free_layer) (gx3dObjectLayer *layer) )
{
  int          processing_layers;
  lwo2_Layer  *l_layer, *parent_layer;
  lwo2_Object *l_object;
  int          error = FALSE;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Set vertex format for object
  g_object->vertex_format = vertex_format_flags;

  // Read LWO2 data from file
  l_object = lwo2_ReadObjectFile (filename);
  if (l_object) {
    // Verify the lwo2 object is compatible
    if (NOT Verify_LWO2_Object (l_object, vertex_format_flags, flags))
      error = TRUE;
    else {
      // Set all layers to not processed
      for (l_layer=l_object->layer_list; l_layer; l_layer=l_layer->next) 
        l_layer->processed = FALSE;

/*____________________________________________________________________
|
| Choose one non-skeleton lwo2 layer at a time and process it
|___________________________________________________________________*/

#ifdef DEBUG
      DEBUG_WRITE ("LWO2_File_To_GX3D_Object(): Choose one lwo2 layer at a time to process")
#endif
      
      for (processing_layers=TRUE; processing_layers AND (NOT error);) {

        // Find a layer to process with no parent
        for (l_layer=l_object->layer_list; l_layer; l_layer=l_layer->next)
          // Does this layer need to be processed?
          if (NOT l_layer->processed)
            // Does this layer have no parent?
            if (l_layer->parent == NULL)
              break;

        // Keep looking for a layer to process?
        if (l_layer == NULL) {
          // Find a layer to process that has a parent and the parent has been processed 
          for (l_layer=l_object->layer_list; l_layer; l_layer=l_layer->next) 
            // Does this layer need to be processed?
            if (NOT l_layer->processed)
              // Does this layer have a parent?
              if (l_layer->parent) {
                // Find the parent
                for (parent_layer=l_object->layer_list; parent_layer; parent_layer=parent_layer->next)
                  // Is this the parent
                  if (parent_layer->number == *(l_layer->parent))
                    break;
                // Was the parent found (if not error!)
                if (parent_layer) {
                  // Has the parent already been processed?
                  if (parent_layer->processed)
                    break;
                }
                else {
                  gxError ("LWO2_File_To_GX3D_Object(): Error can't find parent of a layer");
                  error = TRUE;
                }
              }
        }

        // If no more layers to read, quit processing
        if (l_layer == NULL)
          processing_layers = FALSE;
        else {
          // Process a non-skeleton layer?
          if (NOT l_layer->skeleton) 
            Process_Geometry_Layer (l_object, l_layer, g_object, vertex_format_flags, flags, &error);
          l_layer->processed = TRUE;  // set this flag even if the layer is a skeleton layer and hasn't been processed yet
        }
      } // for (processing_layers ...

/*____________________________________________________________________
|
| Process skeleton layer if needed
|___________________________________________________________________*/

#ifdef DEBUG
      DEBUG_WRITE ("LWO2_File_To_GX3D_Object(): Looking for skeleton layer to process")
#endif

      // Process a skeleton layer? 
      if (vertex_format_flags & gx3d_VERTEXFORMAT_WEIGHTS) {
        // Look at all layers
        for (l_layer=l_object->layer_list; l_layer; l_layer=l_layer->next) 
          // Is this a skeleton layer
          if (l_layer->skeleton) {
            Process_Skeleton_Layer (l_object, l_layer, g_object, vertex_format_flags, flags, &error);
            break;
          }
      } 

/*____________________________________________________________________
|
| Remove duplicates from each layer and build vertex/polygon normal arrays
|___________________________________________________________________*/

#ifdef DEBUG
      DEBUG_WRITE ("LWO2_File_To_GX3D_Object(): Remove duplicates from each layer and build vertex/polygon normal arrays")
#endif
//// Combine function doesn't take into account morphs, weights, matrixpalette, ... (need to update before enabling this code) 9/06
      //// Combine layers where possible?
      //if (NOT error)
      //  if (NOT (flags & gx3d_DONT_COMBINE_LAYERS))
      //    if (g_object->layer)
      //      if (NOT Combine_GX3D_Layers (g_object->layer, free_layer))
      //        error = TRUE;

      // Remove duplicate vertices in each layer
      if (NOT error) 
        if (flags & gx3d_MERGE_DUPLICATE_VERTICES)
          if (g_object->layer)
            if (NOT Remove_GX3D_Duplicates (g_object->layer)) 
              error = TRUE;

      // Compute polygon normals in each layer
      if (NOT error)
        if (g_object->layer)
          if (NOT Compute_GX3D_Polygon_Normals (g_object->layer))
            error = TRUE;

      // Compute vertex normals in all layers
			if (NOT error) 
        gx3d_ComputeVertexNormals (g_object, flags);

      // Compute bounding box/sphere of object and for all layers
      if (NOT error)
        gx3d_ComputeObjectBounds (g_object);

/*____________________________________________________________________
|
| Error checking
|___________________________________________________________________*/

#ifdef DEBUG
      DEBUG_WRITE ("LWO2_File_To_GX3D_Object(): Error checking")
#endif

      // If no gx3d object layers created, error!
      if (g_object->layer == NULL) {
        gxError ("LWO2_File_To_GX3D_Object(): Error, no gx3dObjectLayer created");
        error = TRUE;
      }
      // On any error, free the gx3d object
      if (error)
        if (g_object) {
          gxError ("LWO2_File_To_GX3D_Object(): Error, gx3dObject not created");
          gx3d_FreeObject (g_object);
          g_object = NULL;
        }
    } // if (Verify ...

    // Free memory for LWO2 object
    lwo2_FreeObject (l_object);
  }

  return (NOT error);
}

/*____________________________________________________________________
|
| Function: Verify_LWO2_Object
|                       
| Input: Called from LWO2_File_To_GX3D_Object()                                                                 
| Output: Verifies the lwo2 object can be converted to a gx3d format.
|   Returns true if compatible, else false.
|___________________________________________________________________*/

static int Verify_LWO2_Object (lwo2_Object *l_object, unsigned vertex_format_flags, unsigned flags)
{
  int i, n, found;
  lwo2_Clip       *clip;
  lwo2_Surface    *surface;
  lwo2_Block      *block;
  lwo2_VertexMap  *vmap;
//  lwo2_PolyTag    *polytag;
  lwo2_Layer      *layer;
  int compatible = TRUE;

/*____________________________________________________________________
|
| Verify tags array
|  - for each tag, a surface must exist with the same name as the tag
|___________________________________________________________________*/
  
/*
  if (compatible) {
    for (i=0; (i<l_object->num_tags) AND compatible; i++) {
      for (surface=l_object->surface_list, found=FALSE; surface AND (NOT found); surface=surface->next) 
        if (!strcmp (l_object->tags_array[i], surface->name))  
          found = TRUE;
      if (NOT found) {
        DEBUG_ERROR ("Verify_LWO2_Object(): error lwo2 object not compatible with gx3d format (tags array)")
        compatible = FALSE;
      }
    }
  }
*/

/*____________________________________________________________________
|
| Verify clip list
|  - each clip should have a valid filename and the file should exist
|___________________________________________________________________*/

  if (compatible) {
    // Only do this check if requesting texture coords and loading textures
    if (OBJECT_REQUIRES_TEXCOORDS AND (NOT (flags & gx3d_DONT_LOAD_TEXTURES))) {
      for (clip=l_object->clip_list; clip AND compatible; clip=clip->next) {
        if (clip->filename == NULL) {
          gxError ("Verify_LWO2_Object(): error lwo2 object not compatible with gx3d format (no filename in clip)");
          compatible = FALSE;
        }
        else if (NOT File_Exists (clip->filename)) {
          char str[256];
          sprintf (str, "Verify_LWO2_Object(): error lwo2 object not compatible with gx3d format (clip file: %s doesn't exist)", clip->filename);
          gxError (str);
          compatible = FALSE;
        }
      }
    }
  }

/*____________________________________________________________________
|
| Verify surface list
|  - each block shoud have a vertexmap_name that exists
|  - each block should have a clip_id
|___________________________________________________________________*/

  if (compatible) {
    for (surface=l_object->surface_list; surface AND compatible; surface=surface->next) {
      // Make sure each block has a vertexmap_name that exists
      for (block=surface->block_list; block AND compatible; block=block->next) {
//***** Changed this so it's now legal to have blocks that don't refer to a vertexmap 2/10/02 *****
//        if (block->vertexmap_name == NULL) {
//          gxError ("Verify_LWO2_Object(): error lwo2 block->vertexmap_name is NULL");
//          compatible = FALSE;
//        }  
//        else {
        if (block->vertexmap_name) {
          for (layer=l_object->layer_list, found=FALSE; layer AND (NOT found); layer=layer->next) 
            for (vmap=layer->vmap_list; vmap AND (NOT found) AND compatible; vmap=vmap->next) {
              if (vmap->name == NULL) {
                gxError ("Verify_LWO2_Object(): error lwo2 vmap->name is NULL");
                compatible = FALSE;
              }
              else if (!strcmp (block->vertexmap_name, vmap->name)) 
                found = TRUE;
            }
          if (NOT found) {
            gxError ("Verify_LWO2_Object(): error lwo2 object not compatible with gx3d format (vertexmap name doesn't exist)");
//            gxError (block->vertexmap_name);
            compatible = FALSE;
          }
        }
      }
      // Make sure each block has a clip_id
//***** Changed this so it's now legal to have blocks that don't refer to a vertexmap 2/10/02 *****
//      for (block=surface->block_list; block AND compatible; block=block->next) {
      for (block=surface->block_list; block AND block->vertexmap_name AND compatible; block=block->next) {
        if (block->clip_id == NULL) {
          gxError ("Verify_LWO2_Object(): error lwo2 object not compatible with gx3d format (no clip_id in block)");
          compatible = FALSE;
        }
      }
    }
  }

/*____________________________________________________________________
|
| Verify layer list
|  - 1 or more uv texture maps (if using uv tex coords) for each non-skeleton layer
|___________________________________________________________________*/

  if (compatible) {
    for (layer=l_object->layer_list; layer AND compatible; layer=layer->next) {
      //***** Removed this restriction 3/13/06, layers can have > 1 material
      //// Count the number of polytags this layer has
      //for (polytag=layer->polytag_list, i=0, n=0; polytag; polytag=polytag->next, i++)
      //  if (polytag->type == lwo2_POLYTAGTYPE_SURFACE)
      //    n++;
      //// Must be 1 for non-skeleton layer
      //if ((NOT layer->skeleton) AND (n != 1)) {
      //  DEBUG_ERROR ("Verify_LWO2_Object(): error non-skeleton lwo2 layer not compatible with gx3d format (not 1 polytags)")
      //  compatible = FALSE;
      //}
      if (compatible) {
        // Count the number of UV vertexmaps this layer has
        for (vmap=layer->vmap_list, i=0; vmap; vmap=vmap->next)
          if (vmap->type == uv_map)
            if (vmap->name)
              i++;
        // Only do this check if requesting texture coords
        if (OBJECT_REQUIRES_TEXCOORDS)
          // Must be 1 or more for non-skeleton layer
          if ((NOT layer->skeleton) AND (i == 0)) { 
            gxError ("Verify_LWO2_Object(): error non-skeleton lwo2 layer not compatible with gx3d format (not 1 or more UV maps)");
            compatible = FALSE;
          }
        //***** Removed this restriction 3/13/06, layers can have > 1 material
        // For each surface polytag
        //for (polytag=layer->polytag_list; polytag AND compatible; polytag=polytag->next) { 
        //  if (polytag->type == lwo2_POLYTAGTYPE_SURFACE)
        //    // All tags_index_array must be same
        //    for (i=1; (i<layer->num_polygons) AND compatible; i++)
        //      if (polytag->tags_index_array[i] != polytag->tags_index_array[0]) {
        //        DEBUG_ERROR ("Verify_LWO2_Object(): error non-skeleton lwo2 layer has more than one material")
        //        compatible = FALSE;
        //      }
        //}
      }
    }
  }

/*____________________________________________________________________
|
| Verify skeleton
|  - max 1 skeleton layer
|  - skeleton layer must have a vertex array
|  - skeleton layer must have a polygon array
|___________________________________________________________________*/

  if (compatible) {
    // Count the number of skeleton layers
    for (layer=l_object->layer_list, n=0; layer AND compatible; layer=layer->next) 
      if (layer->skeleton) {
        n++;
        break;
      }
    if (n > 1) {
      gxError ("Verify_LWO2_Object(): error lwo2 object has more than 1 skeleton layer");
      compatible = FALSE;
    }
    // Verify this skeleton layer
    else if (n == 1) {
      if (layer->vertex_array == NULL) {
        gxError ("Verify_LWO2_Object(): error lwo2 skeleton layer has no vertex array");
        compatible = FALSE;
      }
      else if (layer->polygon_array == NULL) {
        gxError ("Verify_LWO2_Object(): error lwo2 skeleton layer has no polygon array");
        compatible = FALSE;
      }
    }
  }

  return (compatible);
}

/*____________________________________________________________________
|
| Function: Process_Geometry_Layer
|                       
| Input: Called from LWO2_File_To_GX3D_Object()                                                                 
| Output: Processes one lwo2 layer, writing data to the gx3d object.
|___________________________________________________________________*/

static void Process_Geometry_Layer (
  lwo2_Object *l_object,
  lwo2_Layer  *l_layer,
  gx3dObject  *g_object, 
  unsigned     vertex_format_flags,
  unsigned     flags,
  int         *error )
{
  int              i, j, k, n, num_textures, l_three_vertex_polygons, bone, vertex, warning;
  char            *texture_filename[gx3d_NUM_TEXTURE_STAGES];
  char            *alpha_texture_filename[gx3d_NUM_TEXTURE_STAGES];
  char            *vmap_name[gx3d_NUM_TEXTURE_STAGES];
  float            f;
  lwo2_VertexMap  *l_vertexmap, *l_uv_vertexmap[gx3d_NUM_TEXTURE_STAGES];
  gx3dTexture      g_texture[gx3d_NUM_TEXTURE_STAGES];
  gx3dObjectLayer *g_layer, *g_tlayer, **g_layerpp;
//  lwo2_Layer      *tlayer;
//  lwo2_PolyTag    *polytag;
//  lwo2_PolyTag    *l_bone_names, *l_weightmap_names;

/*____________________________________________________________________
|
| Load textures used by this lwo2 layer
|___________________________________________________________________*/

#ifdef DEBUG
  DEBUG_WRITE ("Process_Geometry_Layer(): Load textures used by this lwo2 layer")
#endif

  // Load textures for each polytag in the layer (1 polytag)
  for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) {
    g_texture[i]              = 0;
    texture_filename[i]       = 0;
    alpha_texture_filename[i] = 0;
    vmap_name[i]              = 0;
  }

  if (NOT OBJECT_REQUIRES_TEXCOORDS)
    num_textures = 0;
  else {
    // Get the texture filenames for this surface
    num_textures = Get_LWO2_Texture_Filenames (l_object, l_layer, texture_filename, alpha_texture_filename, vmap_name);
    if (num_textures == 0) {
      gxError ("Process_Geometry_Layer(): Error getting lwo2 texture filenames");
      *error = TRUE;
    }
    else {
      // Create texture/s from the filenames
      for (i=0; i<num_textures; i++) {
//char str[256];
//sprintf (str, "Loading texture: %s", texture_filename[i]);
//DEBUG_WRITE (str)

        // Load data from the texture file into memory?
        if (NOT (flags & gx3d_DONT_LOAD_TEXTURES)) {
          g_texture[i] = gx3d_InitTexture_File (texture_filename[i], alpha_texture_filename[i], flags);
          // Make sure texture was created
          if (g_texture[i] == 0) {
            gxError ("Process_Geometry_Layer(): Error calling gx3d_InitTexture_File()");
            *error = TRUE;
          }
        }
        // Find the vertexmap associated with this texture
        l_uv_vertexmap[i] = NULL;
        if (vmap_name[i]) {
          // Get the vertex map associated with texture[i]
          for (l_vertexmap=l_layer->vmap_list; l_vertexmap; l_vertexmap=l_vertexmap->next)
            if ((l_vertexmap->type == uv_map) AND l_vertexmap->name)
              if (!strcmp (vmap_name[i], l_vertexmap->name)) {
                l_uv_vertexmap[i] = l_vertexmap;
                break;
              }
        }      
        if (l_uv_vertexmap[i] == NULL) {
          gxError ("Process_Geometry_Layer(): Error looking for vertex map associated with texture");
          *error = TRUE;
        }
      }
    } 
  }

/*____________________________________________________________________
|
| Create a new gx3d layer and add it to the gx3d object
|___________________________________________________________________*/

#ifdef DEBUG
  DEBUG_WRITE ("Process_Geometry_Layer(): Create a new gx3d layer and add it to the gx3d object")
#endif

  if (NOT *error) {
    // Create a new gx3d layer
    g_layer = (gx3dObjectLayer *) calloc (1, sizeof(gx3dObjectLayer));
    if (g_layer == NULL) {
      gxError ("Process_Geometry_Layer(): Error creating a new gx3dObjectLayer");
      *error = TRUE;
    }
    else {
      // Set some variables in this new layer
      gx3d_GetIdentityMatrix (&(g_layer->transform.local_matrix));
      gx3d_GetIdentityMatrix (&(g_layer->transform.composite_matrix));
      for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++)
        g_layer->texture[i]    = g_texture[i];
      g_layer->num_textures  = num_textures;
      g_layer->pivot.x       = l_layer->pivot.x;
      g_layer->pivot.y       = l_layer->pivot.y;
      g_layer->pivot.z       = l_layer->pivot.z;
      g_layer->id            = (int)(l_layer->number);
      // Set parent id, if any
      if (l_layer->parent) {
        g_layer->has_parent = TRUE;
        g_layer->parent_id  = (int)*(l_layer->parent);
      }
      // Set layer name, if any (when layers are combined, only the first layer name is used)
      if (l_layer->name) {
        if (strlen(l_layer->name)) {
          g_layer->name = (char *) calloc (strlen(l_layer->name)+1, sizeof(char));
          if (g_layer->name == NULL) {
            gxError ("Process_Geometry_Layer(): Error creating memory for g_layer->name");
            *error = TRUE;
          }
          else
            strcpy (g_layer->name, l_layer->name);
        }
      }

      // Add the new layer to the start of an empty gx3d layer list
      if (g_object->layer == NULL) 
        g_object->layer = g_layer;
      // Find a place to put it in a non-empty gx3d layer list hierarchy
      else {
        g_layerpp = NULL;
        // If the new layer has no parent, put it at the end of the first level of layers
        if (NOT g_layer->has_parent) 
          g_layerpp=&(g_object->layer);
        // If the new layer has a parent, put it at the end of the parent's child level of layers
        else {
          g_tlayer = Get_Parent_GX3D_Layer (g_object->layer, g_layer->parent_id);
          if (g_tlayer)
            g_layerpp=&(g_tlayer->child);
        }
        // Error checking
        if (g_layerpp == NULL) {
          gxError ("Process_Geometry_Layer(): Error looking for parent layer in gx3dObject layer hierarchy");
          *error = TRUE;
        }
        // Put the new layer at the end of this level of layers
        else {
          for (; *g_layerpp; g_layerpp=&((*g_layerpp)->next));
            *g_layerpp = g_layer;
        }
      }
    }

/*____________________________________________________________________
|
| Allocate memory in the gx3d layer
|___________________________________________________________________*/

#ifdef DEBUG
  DEBUG_WRITE ("Process_Geometry_Layer(): Allocate memory in the gx3d layer")
#endif

    if (NOT *error) {
      // Count the number of 3-vertex polygons in lwo2 polygon array
      l_three_vertex_polygons = 0;
      for (i=0; i<l_layer->num_polygons; i++)
        if (l_layer->polygon_array[i].num_vertices == 3)
          l_three_vertex_polygons++;

      // Allocate memory for vertex array
      g_layer->vertex = (gx3dVector *) malloc (l_layer->num_vertices * sizeof(gx3dVector));
      if (g_layer->vertex == NULL)
        *error = TRUE;

      // Allocate memory for vertex color diffuse array?
      if (vertex_format_flags & gx3d_VERTEXFORMAT_DIFFUSE) {
        g_layer->diffuse  = (gxColor *) malloc (l_layer->num_vertices * sizeof(gxColor));
        if (g_layer->diffuse == NULL)
          *error = TRUE;
      }

      // Allocate memory for vertex color specular array?
      if (vertex_format_flags & gx3d_VERTEXFORMAT_SPECULAR) {
        g_layer->specular = (gxColor *) malloc (l_layer->num_vertices * sizeof(gxColor));
        if (g_layer->specular == NULL)
          *error = TRUE;
      }

      // Allocate memory for polygon array
      g_layer->polygon = (gx3dPolygon *) malloc (l_three_vertex_polygons * sizeof(gx3dPolygon));
      if (g_layer->polygon == NULL)
        *error = TRUE;

      // Allocate memory for texture coords arrays?
      if (OBJECT_REQUIRES_TEXCOORDS) 
        for (i=0; i<g_layer->num_textures; i++) {
          g_layer->tex_coords[i] = (gx3dUVCoordinate *) malloc (l_layer->num_vertices * sizeof(gx3dUVCoordinate));
          // Init all UV coords to outside normal 0-1 texture space (useful for some situations like single layer models with multiple UV maps, besides that this step is not necessary)
          for (int z=0; z<l_layer->num_vertices; z++) {
            g_layer->tex_coords[i][z].u = -1; // draw these types of models with addressingmode = border, border color = white
            g_layer->tex_coords[i][z].v = -1;
          }
          if (g_layer->tex_coords[i] == NULL)
            *error = TRUE;
        }

      // Allocate memory for weights array
      if (vertex_format_flags & gx3d_VERTEXFORMAT_WEIGHTS) {
        g_layer->weight = (gx3dVertexWeight *) calloc (l_layer->num_vertices, sizeof(gx3dVertexWeight));
        if (g_layer->weight == NULL)
          *error = TRUE;
      }

      // Any memory allocation errors?
      if (*error) 
        gxError ("Process_Geometry_Layer(): Error allocating memory");

/*____________________________________________________________________
|
| Copy the data from the lwo2 layer into the gx3d layer
|___________________________________________________________________*/

#ifdef DEBUG
  DEBUG_WRITE ("Process_Geometry_Layer(): Copy the data from the lwo2 layer into the gx3d layer")
#endif

      if (NOT *error) {
        // Copy vertex data
        for (i=0; i<l_layer->num_vertices; i++) {
          g_layer->vertex[i].x = l_layer->vertex_array[i].x;
          g_layer->vertex[i].y = l_layer->vertex_array[i].y;
          g_layer->vertex[i].z = l_layer->vertex_array[i].z;
        }
        // Set # of vertices
        g_layer->num_vertices = l_layer->num_vertices;
        // Copy polygon data
        j = 0;
        for (i=0; i<l_layer->num_polygons; i++) 
          if (l_layer->polygon_array[i].num_vertices == 3) {
            g_layer->polygon[j].index[0] = l_layer->polygon_array[i].index[0];
            g_layer->polygon[j].index[1] = l_layer->polygon_array[i].index[1];
            g_layer->polygon[j].index[2] = l_layer->polygon_array[i].index[2];
            j++;
          }
        // Set # of polygons
        g_layer->num_polygons = j;
      }
            
/*____________________________________________________________________
|
| Copy the texture coordinates data from the lwo2 layer into the gx3d layer
|___________________________________________________________________*/

      if (NOT *error) {
        // Copy tex coords?
        if (OBJECT_REQUIRES_TEXCOORDS)
          for (i=0; i<g_layer->num_textures; i++) {
            for (j=0; j<l_layer->num_vertices; j++) {
              k = l_uv_vertexmap[i]->index_array[j];
              g_layer->tex_coords[i][k].u = l_uv_vertexmap[i]->value_array[j*2];
              g_layer->tex_coords[i][k].v = 1 - l_uv_vertexmap[i]->value_array[j*2+1];
            }
          }
      }

/*____________________________________________________________________
|
| Copy the (diffuse) vertex colors from the lwo2 layer into the gx3d layer
|___________________________________________________________________*/

      if (NOT *error) {
        // Copy vertex colors?
        if (vertex_format_flags & gx3d_VERTEXFORMAT_DIFFUSE) {
          // Find the color map in the list of lwo vmaps, if any
          for (l_vertexmap=l_layer->vmap_list; l_vertexmap; l_vertexmap=l_vertexmap->next)
            if ((l_vertexmap->type == rgb_color_map) OR (l_vertexmap->type == rgba_color_map))
              break;
          // If color map found, make sure same number of entries as number of vertices in the layer
          if (l_vertexmap)
            if (l_vertexmap->num_entries != l_layer->num_vertices) {
              gxError ("Process_Geometry_Layer(): LWO2 layer RGBA vertex map doesn't have same # entries as # vertices in the LWO2 layer");
              l_vertexmap = 0;
              *error = TRUE;
            }
          // No color map found?
          if (l_vertexmap == 0) {
            // Set default vertex color (all white)
            for (i=0; i<l_layer->num_vertices; i++) {
              g_layer->diffuse[i].r = 255;
              g_layer->diffuse[i].g = 255;
              g_layer->diffuse[i].b = 255;
              g_layer->diffuse[i].a = 255;
            }
          }
          // RGBA color map found?
          else if (l_vertexmap->type == rgba_color_map) {
            for (i=0; i<l_vertexmap->num_entries; i++) {
              g_layer->diffuse[l_vertexmap->index_array[i]].r = (byte)(l_vertexmap->value_array[i*4+0] * 255);
              g_layer->diffuse[l_vertexmap->index_array[i]].g = (byte)(l_vertexmap->value_array[i*4+1] * 255);
              g_layer->diffuse[l_vertexmap->index_array[i]].b = (byte)(l_vertexmap->value_array[i*4+2] * 255);
              g_layer->diffuse[l_vertexmap->index_array[i]].a = (byte)(l_vertexmap->value_array[i*4+3] * 255);
            }
          }
          // RGBA color map found?
          else if (l_vertexmap->type == rgb_color_map) {
            for (i=0; i<l_vertexmap->num_entries; i++) {
              g_layer->diffuse[l_vertexmap->index_array[i]].r = (byte)(l_vertexmap->value_array[i*4+0] * 255);
              g_layer->diffuse[l_vertexmap->index_array[i]].g = (byte)(l_vertexmap->value_array[i*4+1] * 255);
              g_layer->diffuse[l_vertexmap->index_array[i]].b = (byte)(l_vertexmap->value_array[i*4+2] * 255);
              g_layer->diffuse[l_vertexmap->index_array[i]].a = 255;
            }
          }
        }
      }

/*____________________________________________________________________
|
| Fill in specular vertex colors
|
|  May want to be able to specify 2 vertex maps per layer in LW and use
|  for diffuse and specular.  Idea: call these 'diffuse' and 'specular'.
|  For now, just set specular, if used, to all 0.
|___________________________________________________________________*/

      if (NOT *error) {
        // Fill in specular colors?
        if (vertex_format_flags & gx3d_VERTEXFORMAT_SPECULAR) {
          for (i=0; i<l_layer->num_vertices; i++) 
            g_layer->specular[i].rgba = 0;
        }
      }

/*____________________________________________________________________
|
| Copy the vertex weights data from the lwo2 layer into the gx3d layer
|___________________________________________________________________*/

#ifdef DEBUG
  DEBUG_WRITE ("Process_Geometry_Layer(): Copy vertex weight data from the lwo2 layer to the gx3d layer")
#endif

      if (NOT *error) {
        // Copy vertex weights?
        if (vertex_format_flags & gx3d_VERTEXFORMAT_WEIGHTS) {

          // Find array of bone names
//***** Removed this restriction 3/06, objects can have weights without a bones layer (when using bones from .lws file)
          //l_bone_names = NULL;
          //for (tlayer=l_object->layer_list; tlayer AND (l_bone_names == NULL); tlayer=tlayer->next)
          //  if (tlayer->skeleton) {
          //    for (polytag=tlayer->polytag_list; polytag AND (l_bone_names == NULL); polytag=polytag->next) 
          //      if (polytag->type == lwo2_POLYTAGTYPE_BONE_NAME) 
          //        l_bone_names = polytag;
          //  }
          //if (l_bone_names == NULL) {
          //  gxError ("Process_Geometry_Layer(): Error can't find bone names");
          //  *error = TRUE;
          //}

          // Find array of bone weightmap names
          //l_weightmap_names = NULL;
          //for (tlayer=l_object->layer_list; tlayer AND (l_weightmap_names == NULL); tlayer=tlayer->next)
          //  if (tlayer->skeleton) {
          //    for (polytag=tlayer->polytag_list; polytag AND (l_weightmap_names == NULL); polytag=polytag->next) 
          //      if (polytag->type == lwo2_POLYTAGTYPE_BONE_WEIGHTMAP) 
          //        l_weightmap_names = polytag;
          //  }
          //if (l_weightmap_names == NULL) {
          //  gxError ("Process_Geometry_Layer(): Error can't find bone weightmap names");
          //  *error = TRUE;
          //}

          if (NOT *error) {
            warning = 0;
            // Go through all vertices in the lwo2 layer and set weights
            for (vertex=0; (vertex<l_layer->num_vertices) AND (NOT *error); vertex++) {
              // Go through all vertexmaps in the lwo2 layer
              for (l_vertexmap=l_layer->vmap_list; l_vertexmap AND (NOT *error); l_vertexmap=l_vertexmap->next) {
                // Is this vertexmap a weightmap?
                if ((l_vertexmap->type == weight_map) AND l_vertexmap->name) {
                  // Get bone # associated with this weightmap
//                  for (bone=0; bone<l_layer->num_polygons; bone++)
//                    if (!strcmp(l_vertexmap->name, l_object->tags_array[l_weightmap_names->tags_index_array[bone]]))
//                      break;
                  bone = l_vertexmap->weight_map_id;
                  // Go through weightmap looking for vertex
                  for (i=0; (i<l_vertexmap->num_entries) AND (NOT *error); i++) {
                    // Does this weightmap entry refer to vertex?
                    if (l_vertexmap->index_array[i] == vertex) {
                      // All entries in weight array filled?
                      j = g_layer->weight[vertex].num_weights;
											if (j == 4) {
#ifdef _DEBUG												
												char str[300];
												sprintf (str, "Process_Geometry_Layer(): Error weightmap already has 4 weights, in layer: %s", g_layer->name);
                        DEBUG_ERROR (str)
                        sprintf (str, "  vertex (x,y,z): %f,%f,%f", g_layer->vertex[vertex].x, g_layer->vertex[vertex].y, g_layer->vertex[vertex].z); 
                        DEBUG_ERROR (str)
#endif
											}
                      else {
                        // Set weight data
                        g_layer->weight[vertex].value[j] = l_vertexmap->value_array[i];
                        g_layer->weight[vertex].matrix_index[j] = bone;
                        g_layer->weight[vertex].num_weights++;
                      }
                    }
                  }
                }
              }
              // Each vertex should probably have at least one weight
              if (g_layer->weight[vertex].num_weights == 0) 
                warning++;
              // Normalize weights
              else {
                for (i=0, f=0; i<g_layer->weight[vertex].num_weights; i++) 
                  f += g_layer->weight[vertex].value[i];
                for (i=0; i<g_layer->weight[vertex].num_weights; i++) 
                  g_layer->weight[vertex].value[i] /= f;
              }
            }

            // Print warning to debug file if any vertices found that had no weights attached
						//  this may not necessarily be an error - a layer may not have any weightmaps for example
            if (warning) {
              static char str[256];
              sprintf (str, "Process_Geometry_Layer(): %d vertices have no weights", warning);
              DEBUG_WRITE (str);
            }
          }
        }
      }

/*____________________________________________________________________
|
| Create matrix palette in the gx3d layer
|___________________________________________________________________*/

#ifdef DEBUG
  DEBUG_WRITE ("Process_Geometry_Layer(): Create matrix palette in the gx3d layer")
#endif

      if (NOT *error) {
        // Need a matrix palette?
        if (vertex_format_flags & gx3d_VERTEXFORMAT_WEIGHTS) {
          // Count the number of weightmaps in the lwo2 layer
          for (n=0, l_vertexmap=l_layer->vmap_list; l_vertexmap; l_vertexmap=l_vertexmap->next)
            if (l_vertexmap->type == weight_map)
              n++;
          // Any weightmaps found in the lwo2 layer?
          if (n) {
            // Allocate memory for a matrix palette
            g_layer->matrix_palette = (gx3dPaletteMatrix *) malloc (n * sizeof(gx3dPaletteMatrix));
            if (g_layer->matrix_palette == 0) {
              gxError ("Process_Geometry_Layer(): error allocating memory for matrix palette");
              *error = TRUE;
            }
            else {
              g_layer->num_matrix_palette = n;
              // Set values of each entry in the matrix palette
              for (i=0; i<n; i++) {
                gx3d_GetIdentityMatrix (&(g_layer->matrix_palette[i].m));
                for (l_vertexmap=l_layer->vmap_list; l_vertexmap; l_vertexmap=l_vertexmap->next)
                  if ((l_vertexmap->type == weight_map) AND (l_vertexmap->weight_map_id == i))
                    break;
                if (l_vertexmap == 0) {
                  gxError ("Process_Geometry_Layer(): can't match a matrix palette entry with an lwo2 weightmap");
                  *error = TRUE;
                }
                else {
                  g_layer->matrix_palette[i].weightmap_name = (char *) calloc (strlen(l_vertexmap->name)+1, sizeof(char));
                  if (g_layer->matrix_palette[i].weightmap_name == 0) {
                    gxError ("Process_Geometry_Layer(): can't allocate memory for matrix palette entry weightmap name");
                    *error = TRUE;
                  }
                  else 
                    strcpy (g_layer->matrix_palette[i].weightmap_name, l_vertexmap->name);
                }
              }
            }
          }
        }
      }

/*____________________________________________________________________
|
| Copy the morph maps from the lwo2 layer into the gx3d layer
|___________________________________________________________________*/

      if (NOT *error) {
        // Copy morphs?
        if (vertex_format_flags & gx3d_VERTEXFORMAT_MORPHS) {
          // Count the number of valid morphmaps in the lwo2 layer
          for (n=0, l_vertexmap=l_layer->vmap_list; l_vertexmap; l_vertexmap=l_vertexmap->next)
            // Is this vertexmap a morph?
            if (l_vertexmap->type == morph_map) {
              // Does it have dimension 3? (all morphs should)
              if (l_vertexmap->dimension != 3)
                DEBUG_ERROR ("Process_Geometry_Layer(): lwo2 morph map doesn't have 3 dimensions")
              // Does it have a name? (all morphs should)
              else if (l_vertexmap->name == 0)
                DEBUG_ERROR ("Process_Geometry_Layer(): lwo2 morph map doesn't have a name")
              else
                n++;
            }
          // Any morphmaps found in the lwo2 layer?
          if (n) {
// DEBUG CODE						
						//char str[200];
						//sprintf (str, "%d morph maps found in the lwo2 layer", n);
						//DEBUG_WRITE (str);
						//DEBUG_WRITE (g_layer->name);
/////////////
            // Set number of morphs
            g_layer->num_morphs = n;
            // Allocate memory for composite morph map
            g_layer->composite_morph = (gx3dVector *) calloc (g_layer->num_vertices, sizeof(gx3dVector));
            if (g_layer->composite_morph == 0) {
              gxError ("Process_Geometry_Layer(): error allocating memory for composite morph");
              *error = TRUE;
            }
            // Allocate memory for array of morphs
            g_layer->morph = (gx3dVertexMorph *) calloc (g_layer->num_morphs, sizeof(gx3dVertexMorph));
            if (g_layer->morph == 0) {
              gxError ("Process_Geometry_Layer(): error allocating memory for morph array");
              *error = TRUE;
            }
            // Create each morphmap in the gx3d layer
            gx3dVertexMorph *g_morph = g_layer->morph;
            for (l_vertexmap=l_layer->vmap_list; l_vertexmap; l_vertexmap=l_vertexmap->next) {
              if ((l_vertexmap->type == morph_map) AND (l_vertexmap->dimension == 3) AND (l_vertexmap->name)) {
                // Count the number of entries (vertex offsets) in this morph that are not {0,0,0}
                for (i=0, n=0; i<l_vertexmap->num_entries; i++)
                  if ((l_vertexmap->value_array[i*3+0] != 0) OR
                      (l_vertexmap->value_array[i*3+1] != 0) OR
                      (l_vertexmap->value_array[i*3+2] != 0))
                    n++;
                // Any morphs values not equal to {0,0,0}? (there should be!)
// DEBUG CODE
								if (n == 0)
									DEBUG_ERROR ("An all zero's morph target found in lwo2 data!")
/////////////
                if (n) {
                  g_morph->num_entries = n;
                  // Set the name for this morph
                  g_morph->name = (char *) calloc (strlen(l_vertexmap->name)+1, sizeof(char));
                  if (g_morph->name == 0) {
                    gxError ("Process_Geometry_Layer(): error allocating memory for gx3d morph name");
                    *error = TRUE;
                  }
                  strcpy (g_morph->name, l_vertexmap->name);
                  // Allocate memory for indeces and vertex offsets arrays
                  g_morph->index = (int *) malloc (g_morph->num_entries * sizeof(int));
                  if (g_morph->index == 0) {
                    gxError ("Process_Geometry_Layer(): error allocating memory for gx3d morph index array");
                    *error = TRUE;
                  }
                  g_morph->offset = (gx3dVector *) malloc (g_morph->num_entries * sizeof(gx3dVector));
                  if (g_morph->offset == 0) {
                    gxError ("Process_Geometry_Layer(): error allocating memory for gx3d morph offset array");
                    *error = TRUE;
                  }
                  // Copy values from lwo2 to gx3d
                  for (i=0, n=0; i<l_vertexmap->num_entries; i++) {
                    // Is this a valid entry to copy?
                    if ((l_vertexmap->value_array[i*3+0] != 0) OR
                        (l_vertexmap->value_array[i*3+1] != 0) OR
                        (l_vertexmap->value_array[i*3+2] != 0)) {
                      g_morph->index [n]   = l_vertexmap->index_array[i];
                      g_morph->offset[n].x = l_vertexmap->value_array[i*3+0] * METERS_TO_FEET;
                      g_morph->offset[n].y = l_vertexmap->value_array[i*3+1] * METERS_TO_FEET;
                      g_morph->offset[n].z = l_vertexmap->value_array[i*3+2] * METERS_TO_FEET;
                      n++;
                    }
                  }
                }
                // Process next gx3d morph map
                g_morph++;
              }
            }
          }
        }
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: Get_LWO2_Texture_Filenames
|                       
| Input: Called from Process_Geometry_Layer()                                                                 
| Output: Returns in callers variables the texture filenames associated 
|   with the LWO2 object layer (Returns NULL if none).  Also, returns
|   the number of textures in the surface that this polytag points
|   to, which will be 1 or more.  If no textures or on any error, returns 0.
|
|   When this function is called it is assumed there are textures and
|   texture coordinates in the object layer.
|___________________________________________________________________*/

static int Get_LWO2_Texture_Filenames (
  lwo2_Object  *l_object, 
  lwo2_Layer   *l_layer, 
  char        **texture_filename, 
  char        **alpha_texture_filename, 
  char        **vmap_name )
{
  int i, j, k, num_uv_maps, found, error, num_textures, num_unique_found;
  char *polytag_name;
  bool unique;
  int unique_tags [gx3d_NUM_TEXTURE_STAGES];
  lwo2_Surface   *l_surface;
  lwo2_Block     *l_block;
  lwo2_Clip      *l_clip;
  lwo2_VertexMap *l_vmap;
  lwo2_PolyTag   *l_polytag;
  struct {
    char *filename;
    char *vmapname;
    bool alpha;
  } texture_info [gx3d_NUM_TEXTURE_STAGES];
  int num_texture_info;

/*____________________________________________________________________
|
| Init variables                      
|___________________________________________________________________*/

  error = FALSE;

  // Set default return values
  for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) {
    texture_filename[i]       = 0;
    alpha_texture_filename[i] = 0;
    vmap_name[i]              = 0;
  }
  num_textures = 0;
  num_uv_maps = 0;

/*____________________________________________________________________
|
| Find the set of surface polygon tags (should only be one set per layer)
|___________________________________________________________________*/

  for (l_polytag=l_layer->polytag_list; l_polytag; l_polytag=l_polytag->next)
    if (l_polytag->type == lwo2_POLYTAGTYPE_SURFACE)
      break;

/*____________________________________________________________________
|
| Process each tag in the set
|___________________________________________________________________*/

  num_unique_found = 0; // # of unique textures found so far

  for (i=0; i<l_layer->num_polygons; i++) {
    // Is this a unique tag (not seen so far)?
    for (j=0, unique=true; unique AND (j<num_unique_found); j++)
      if (l_polytag->tags_index_array[i] == unique_tags[j]) 
        unique = false;
    // Process this unique tag
    if (unique) {
      // Save it 
      unique_tags[num_unique_found] = l_polytag->tags_index_array[i];
      num_unique_found++;
      if (num_unique_found > gx3d_NUM_TEXTURE_STAGES) {
        gxError ("Get_LWO2_Texture_Filenames(): found more unique polygon tags than allowed");
        error = TRUE;
      }
      // Get the name for this polygon tag
      polytag_name = l_object->tags_array[l_polytag->tags_index_array[i]];
      // Look for the corresponding surface name, if any
      for (l_surface=l_object->surface_list; l_surface; l_surface=l_surface->next)
        if (!strcmp (polytag_name, l_surface->name))
          break;
      // Didn't find it?
      if (l_surface == 0) {
        gxError ("Get_LWO2_Texture_Filenames(): can't find surface name to match a polygon tag name");
        error = TRUE;
      }
      else {

/*____________________________________________________________________
|
| Get info about each texture file in this surface
|___________________________________________________________________*/

        // For each block in the surface ...
        num_texture_info = 0;
        for (l_block=l_surface->block_list; l_block; l_block=l_block->next) {
          // Does this block point to an image file?
          if (l_block->clip_id) {
            // Find the clip with the same id as the block's clip_id, if any
            for (l_clip=l_object->clip_list, found=FALSE; l_clip AND (NOT found); l_clip=l_clip->next)
              if (l_clip->id == *(l_block->clip_id))
                // Make sure this block has a vmap name
                if (l_block->vertexmap_name) {
                  found = TRUE;
                  break;
                }
            if (found) {
              // Get info about this texture file
              texture_info[num_texture_info].filename = l_clip->filename;
              texture_info[num_texture_info].vmapname = l_block->vertexmap_name;
              texture_info[num_texture_info].alpha    = (l_block->opacity_type == 5);
              // Verify the vmap exists in this layer, go through layer's vertexmap list
              for (l_vmap=l_layer->vmap_list; l_vmap AND (NOT error); l_vmap=l_vmap->next) {
                // Is this vertexmap a uv map?
                if (l_vmap->type == uv_map) {
                  // Does it have a name?
                  if (l_vmap->name == NULL) {
                    gxError ("Get_LWO2_Texture_Filenames(): lwo2 uv vmap->name is NULL");
                    error = TRUE;
                  }
                  // If so, compare it to the vmap the texture refers to
                  else if (!strcmp(texture_info[num_texture_info].vmapname, l_vmap->name))
                    break;
                }
              }
              // Must have found it
              if (l_vmap == 0) {
                gxError ("Get_LWO2_Texture_Filenames(): Can't find vmap in layer that texture refers to");
                error = TRUE;
              }
              num_texture_info++;
            }
          }
        }

/*____________________________________________________________________
|
| Record non-alpha textures to use by this surface
|___________________________________________________________________*/

        for (j=0; j<num_texture_info; j++) {
          // Is this a non-alpha file?
          if (NOT texture_info[j].alpha) {
            texture_filename[num_textures++] = texture_info[j].filename;
            vmap_name       [num_uv_maps++]  = texture_info[j].vmapname;
            // Verify not going over the number of allowed textures
            if (num_textures > gx3d_NUM_TEXTURE_STAGES) {
              gxError ("Get_LWO2_Texture_Filenames(): Too many textures in a layer");
              error = TRUE;
            }
          }
        }

/*____________________________________________________________________
|
| Record alpha textures to use by this surface
|___________________________________________________________________*/

        for (j=0; j<num_texture_info; j++) {
          // Is this an alpha file?
          if (texture_info[j].alpha) {
            // Try to match this alpha file up with an existing non-alpha file
            for (k=0; k<num_textures; k++) // assumption: num_textures is same as num_uv_maps
              if (!strcmp(vmap_name[k], texture_info[j].vmapname))
                break;
            if (k < num_textures)
              alpha_texture_filename[k] = texture_info[j].filename;
            else {
              gxError ("Get_LWO2_Texture_Filenames(): Can't match up an alpha filename with a non-alpha filename");
              error = TRUE;
            }
          }
        }
      }
    }
  }

/*____________________________________________________________________
|
| Error checking
|___________________________________________________________________*/

  if (error)
    num_textures = 0;
  if (num_textures == 0)
    gxError ("Get_LWO2_Texture_Filenames(): Error no textures found!");

  return (num_textures);
}

/*____________________________________________________________________
|
| Function: Get_LWO2_Texture_Filenames_OLD
|                       
| Input: Called from Process_Geometry_Layer()                                                                 
| Output: Returns in callers variables the texture filenames associated 
|   with the LWO2 object layer (Returns NULL if none).  Also, returns
|   the number of textures in the surface that this polytag points
|   to, which will be 1 or more.  If no textures or on any error, returns 0.
|
|   When this function is called it is assumed there are textures and
|   texture coordinates in the object layer.
|___________________________________________________________________*/

static int Get_LWO2_Texture_Filenames_OLD (
  lwo2_Object  *l_object, 
  lwo2_Layer   *l_layer, 
  char        **texture_filename, 
  char        **alpha_texture_filename, 
  char        **vmap_name )
{
  int i, num_uv_maps, found, error, num_textures;
  lwo2_Surface   *l_surface;
  lwo2_Block     *l_block;
  lwo2_Clip      *l_clip;
  lwo2_VertexMap *l_vmap;

/*____________________________________________________________________
|
| Init variables                      
|___________________________________________________________________*/

  error = FALSE;

  // Set default return values
  for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) {
    texture_filename[i]       = NULL;
    alpha_texture_filename[i] = NULL;
    vmap_name[i]              = NULL;
  }

/*____________________________________________________________________
|
| Find the surface with the matching tag as the polytag                      
|___________________________________________________________________*/

  for (l_surface=l_object->surface_list, found=FALSE; l_surface AND (NOT found); l_surface=l_surface->next) 
    if (l_surface->name AND l_object->tags_array[l_layer->polytag_list->tags_index_array[0]]) // Error checking
      if (!strcmp (l_surface->name, l_object->tags_array[l_layer->polytag_list->tags_index_array[0]])) {
        found = TRUE;
        break;
      }
  if (NOT found) {
    gxError ("Get_LWO2_Texture_Filenames(): error finding surface with matching name as polytag");
    error = TRUE;
  }

/*____________________________________________________________________
|
| Get the uv map names for this surface
|___________________________________________________________________*/
  
  if (NOT error) {
    // Go through layer's vertexmap list
    for (l_vmap=l_layer->vmap_list, num_uv_maps=0; l_vmap AND (NOT error) AND (num_uv_maps < gx3d_NUM_TEXTURE_STAGES); l_vmap=l_vmap->next) {
      // Is this vertexmap a uv map?
      if (l_vmap->type == uv_map) {
        // Does it have a name?
        if (l_vmap->name == NULL) {
          gxError ("Get_LWO2_Texture_Filenames(): error lwo2 uv vmap->name is NULL");
          error = TRUE;
        }
        // If so, save it in callers array
        else 
          vmap_name[num_uv_maps++] = l_vmap->name;
      }
    }
    // Must have found 1 or more uv maps
    if (num_uv_maps == 0) {
      gxError ("Get_LWO2_Texture_Filenames(): error # uv maps not 1 or more");
      error = TRUE;
    }
  }

/*____________________________________________________________________
|
| Get the texture filenames for this surface
|___________________________________________________________________*/

  if (NOT error) {
    // For each block in the surface ...
    for (l_block=l_surface->block_list; l_block; l_block=l_block->next) {
      // Does this block point to an image file?
      if (l_block->clip_id) {
        // Find the clip with the same id as the block's clip_id, if any
        for (l_clip=l_object->clip_list, found=FALSE; l_clip AND (NOT found); l_clip=l_clip->next)
          if (l_clip->id == *(l_block->clip_id))
            // Make sure this block has a vmap name
            if (l_block->vertexmap_name) {
              found = TRUE;
              break;
            }

        if (found) {
          // Get the corresponding uv map this image belongs to
          for (i=0, found=FALSE; (i<num_uv_maps) AND (NOT found); i++)
            if (l_block->vertexmap_name AND vmap_name[i]) // Error checking
              if (!strcmp (l_block->vertexmap_name, vmap_name[i])) {
                found = TRUE;
                break;
              }
          if (NOT found) {
            gxError ("Get_LWO2_Texture_Filenames(): Error couldn't find uv map for a block");
            error = TRUE;
          }
          else {
            // Is this an alpha file?
            if (l_block->opacity_type == 5)
              alpha_texture_filename[i] = l_clip->filename;
            else
              texture_filename[i] = l_clip->filename;
          }
        }
      }
    }
  }

/*____________________________________________________________________
|
| Set # of textures found
|___________________________________________________________________*/

  if (error)
    num_textures = 0;
  else {
    for (i=0; texture_filename[i] AND (i<gx3d_NUM_TEXTURE_STAGES); i++);
    num_textures = i;
    if (num_textures == 0)
      gxError ("Get_LWO2_Texture_Filenames(): Error no textures found!");
    else {
      // Verify at least an image filename found for each texture
      for (i=0; i<num_textures; i++)
        if (texture_filename[i] == NULL) {
          gxError ("Get_LWO2_Texture_Filenames(): Error no image filename found for the texture");
          num_textures = 0;
          break;
        }
    }
  }

  return (num_textures);
}

/*____________________________________________________________________
|
| Function: Get_Parent_GX3D_Layer
|                       
| Input: Called from Process_Geometry_Layer()                                                                 
| Output: Returns the gx3d layer that has the id = parent_id of NULL
|   on any error.
|___________________________________________________________________*/

static gx3dObjectLayer *Get_Parent_GX3D_Layer (gx3dObjectLayer *layer, int parent_id)
{
  gx3dObjectLayer *parent_layer = NULL;

  // Is the input layer the parent?
  if (layer->id == parent_id)
    parent_layer = layer;

  // If not found, search child layers, if any
  if (parent_layer == NULL) 
    if (layer->child)
      parent_layer = Get_Parent_GX3D_Layer (layer->child, parent_id);

  // If not found, search the rest of the layers on this level
  if (parent_layer == NULL)
    if (layer->next)
      parent_layer = Get_Parent_GX3D_Layer (layer->next, parent_id);

  return (parent_layer);
}

/*____________________________________________________________________
|
| Function: Process_Skeleton_Layer
|                       
| Input: Called from LWO2_File_To_GX3D_Object()                                                                 
| Output: Processes one lwo2 skeleton layer, writing data to the gx3d 
|   object.
|___________________________________________________________________*/

static void Process_Skeleton_Layer (
  lwo2_Object *l_object,
  lwo2_Layer  *l_layer,
  gx3dObject  *g_object, 
  unsigned     vertex_format_flags,
  unsigned     flags,
  int         *error )
{
  int           i, j, origin, start_point, end_point;
  char         *name;
  lwo2_PolyTag *polytag;
  gx3dVector    v1, v2, *vertices, pivot, direction;

/*____________________________________________________________________
|
| Create gx3d skeleton
|___________________________________________________________________*/

#ifdef DEBUG
  DEBUG_WRITE ("Process_Skeleton_Layer(): Allocating memory for gx3d skeleton");
#endif

  if (NOT *error) {
    // Does gx3d skeleton already exist?
    if (g_object->skeleton) {
      gxError ("Process_Skeleton_Layer(): Error gx3d skeleton already exists");
      *error = TRUE;
    }
    else {
      // Allocate memory for array of skeleton vertices
      vertices = (gx3dVector *) calloc (l_layer->num_vertices, sizeof(gx3dVector));
      if (vertices == 0)
        *error = TRUE;
      else {
        // Fill array of skeleton vertices
        for (i=0; i<l_layer->num_vertices; i++) {
          vertices[i].x = l_layer->vertex_array[i].x;
          vertices[i].y = l_layer->vertex_array[i].y;
          vertices[i].z = l_layer->vertex_array[i].z;
        }
        // Set origin for layer (assume first polygon is a bone coming off of root - root is the pivot point)
        origin = l_layer->polygon_array[0].index[0];
        // Create skeleton
        g_object->skeleton = gx3d_Skeleton_Init (l_layer->num_vertices, vertices, origin, l_layer->num_polygons);
        if (g_object->skeleton == 0)
          *error = TRUE;
        // Free memory for array of skeleton vertices
        free (vertices);
      }

      // Any memory allocation errors?
      if (*error) 
        gxError ("Process_Skeleton_Layer(): Error allocating memory");
    }

/*____________________________________________________________________
|
| Create hierarchy of bones
|___________________________________________________________________*/

#ifdef DEBUG
  DEBUG_WRITE ("Process_Skeleton_Layer(): Creating bone hierarchy");
#endif

    if (NOT *error) {
      // Process each lwo2 bone, creating a gx3d bone
      for (i=0; (i<g_object->skeleton->num_bones) AND (NOT *error); i++) {
        // Find name of this bone
        name = NULL;
        for (polytag=l_layer->polytag_list; polytag AND (name == NULL); polytag=polytag->next) 
          if (polytag->type == lwo2_POLYTAGTYPE_BONE_NAME) {
            for (j=0; j<l_layer->num_polygons; j++)
              if (polytag->polygon_array[j] == i)
                name = l_object->tags_array[polytag->tags_index_array[j]];
          }
        if (name == NULL) {
          gxError ("Process_Skeleton_Layer(): Error can't find lwo2 bone name");
          *error = TRUE;
        }
        else {
          // Set start, end point
          start_point = l_layer->polygon_array[i].index[0];
          end_point   = l_layer->polygon_array[i].index[1];
          // Set pivot point for bone
          pivot.x = l_layer->vertex_array[l_layer->polygon_array[i].index[0]].x;
          pivot.y = l_layer->vertex_array[l_layer->polygon_array[i].index[0]].y;
          pivot.z = l_layer->vertex_array[l_layer->polygon_array[i].index[0]].z;
          // Set default direction bone is pointing
          v1.x = l_layer->vertex_array[l_layer->polygon_array[i].index[0]].x;
          v1.y = l_layer->vertex_array[l_layer->polygon_array[i].index[0]].y;
          v1.z = l_layer->vertex_array[l_layer->polygon_array[i].index[0]].z;
          v2.x = l_layer->vertex_array[l_layer->polygon_array[i].index[1]].x;
          v2.y = l_layer->vertex_array[l_layer->polygon_array[i].index[1]].y;
          v2.z = l_layer->vertex_array[l_layer->polygon_array[i].index[1]].z;
          gx3d_SubtractVector (&v2, &v1, &direction);
          gx3d_NormalizeVector (&direction, &direction);

          gx3d_Skeleton_AddBone (g_object, name, &pivot, &direction, start_point, end_point);
        }
      } // for
    }
  }
}

/*____________________________________________________________________
|
| Function: Combine_GX3D_Layers
|                       
| Input: Called from LWO2_File_To_GX3D_Object()                                                                 
| Output: Combines gx3d layers if they meet certain conditions:
|   1) must use same textures
|   2) pivot's must be equal
|   3) neither can have any child layers
|   4) must have same parent (if either has a parent)
|
|   Returns true on success, else false on any error.
|
| **** NEED TO UPDATE	THIS FUNCTION TO HANDLE MORPHS AND WEIGHTMAPS ALSO ****
|___________________________________________________________________*/

static int Combine_GX3D_Layers (gx3dObjectLayer *layer, void (*free_layer) (gx3dObjectLayer *layer))
{
  int i, j, n;
  gx3dObjectLayer *layer1, *layer2, *previous_layer2;
  int error = FALSE;

/*____________________________________________________________________
|
| Look for layers to combine
|___________________________________________________________________*/

  // Try to combine two layers at a time at this level of the layer hierarchy
  for (layer1=layer; layer1 AND (NOT error); layer1=layer1->next) {
    for (previous_layer2=layer1, layer2=layer1->next; layer2 AND (NOT error); previous_layer2=layer2, layer2=layer2->next) {
      // Use same textures?
      if ((layer1->texture[0] == layer2->texture[0]) AND 
          (layer1->texture[1] == layer2->texture[1]) AND
          (layer1->texture[2] == layer2->texture[2]) AND
          (layer1->texture[3] == layer2->texture[3]) AND
          (layer1->texture[4] == layer2->texture[4]) AND
          (layer1->texture[5] == layer2->texture[5]) AND
          (layer1->texture[6] == layer2->texture[6]) AND
          (layer1->texture[7] == layer2->texture[7])) {
        // Pivot's equal?
        if ((layer1->pivot.x == layer2->pivot.x) AND
            (layer1->pivot.y == layer2->pivot.y) AND
            (layer1->pivot.z == layer2->pivot.z)) {
          // Neither has children?
          if ((layer1->child == NULL) AND (layer2->child == NULL)) {
            // Have same parent?
            if ((layer1->has_parent == layer2->has_parent) AND
                (layer1->parent_id  == layer2->parent_id)) {
          
/*____________________________________________________________________
|
| Combine two layers
|___________________________________________________________________*/

              // Expand memory for vertex array
              layer1->vertex = (gx3dVector *) realloc (layer1->vertex, (layer1->num_vertices + layer2->num_vertices) * sizeof(gx3dVector));
              if (layer1->vertex == NULL)
                error = TRUE;
              
              // Expand memory for vertex color diffuse array
              if (layer1->diffuse) {
                layer1->diffuse = (gxColor *) realloc (layer1->diffuse, (layer1->num_vertices + layer2->num_vertices) * sizeof(gxColor));
                if (layer1->diffuse == NULL)
                  error = TRUE;
              }

              // Expand memory for vertex color specular array
              if (layer1->specular) {
                layer1->specular = (gxColor *) realloc (layer1->specular, (layer1->num_vertices + layer2->num_vertices) * sizeof(gxColor));
                if (layer1->specular == NULL)
                  error = TRUE;
              }

              // Expand memory for polygon array
              layer1->polygon = (gx3dPolygon *) realloc (layer1->polygon, (layer1->num_polygons + layer2->num_polygons) * sizeof(gx3dPolygon));
              if (layer1->polygon == NULL)
                error = TRUE;

              // Expand memory for texture coords arrays
              for (i=0; i<layer1->num_textures; i++) {
                layer1->tex_coords[i] = (gx3dUVCoordinate *) realloc (layer1->tex_coords[i], (layer1->num_vertices + layer2->num_vertices) * sizeof(gx3dUVCoordinate));
                if (layer1->tex_coords[i] == NULL)
                  error = TRUE;
              }

              // Make sure all memory allocated ok
              if (error) {
                gxError ("Combine_GX3D_Layers(): Error allocating memory");
                error = TRUE;
              }

              if (NOT error) {
                // Copy vertex data
                n = layer1->num_vertices;
                for (i=0; i<layer2->num_vertices; i++) {
                  layer1->vertex[n+i].x = layer2->vertex[i].x;
                  layer1->vertex[n+i].y = layer2->vertex[i].y;
                  layer1->vertex[n+i].z = layer2->vertex[i].z;
                }
                // Fill diffuse array with default values
                if (layer1->diffuse)
                  for (i=0; i<layer2->num_vertices; i++) {
                    layer1->diffuse[n+i].r = 255;
                    layer1->diffuse[n+i].g = 255;
                    layer1->diffuse[n+i].b = 255;
                    layer1->diffuse[n+i].a = 255;
                  }
                // Fill specular array with default values
                if (layer1->specular)
                  for (i=0; i<layer2->num_vertices; i++) 
                    layer1->specular[n+i].index = 0;
                // Copy polygon data
                n = layer1->num_polygons;
                for (i=0; i<layer2->num_polygons; i++) {
                  layer1->polygon[n+i].index[0] = layer2->polygon[i].index[0] + layer1->num_vertices;
                  layer1->polygon[n+i].index[1] = layer2->polygon[i].index[1] + layer1->num_vertices;
                  layer1->polygon[n+i].index[2] = layer2->polygon[i].index[2] + layer1->num_vertices;
                }
                // Copy tex coords
                n = layer1->num_vertices;
                for (i=0; i<layer1->num_textures; i++) {
                  for (j=0; j<layer2->num_vertices; j++) {
                    layer1->tex_coords[i][n+j].u = layer2->tex_coords[i][j].u;
                    layer1->tex_coords[i][n+j].v = layer2->tex_coords[i][j].v;
                  }
                }
                // Set new # of vertices
                layer1->num_vertices += layer2->num_vertices;
                // Set new # of polygons
                layer1->num_polygons += layer2->num_polygons;

                // Remove the 'combined' layer from the list
                previous_layer2->next = layer2->next;
                // Free memory for the 'combined' layer
                layer2->next = NULL;
                (*free_layer) (layer2);
              }

              // Reset pointer to next layer to compare
              layer2 = layer1;
            }
          }
        }
      }
    }
  }

/*____________________________________________________________________
|
| Combine child layers
|___________________________________________________________________*/

  if (NOT error)
    for (; layer; layer=layer->next) 
      if (layer->child) 
        if (NOT Combine_GX3D_Layers (layer->child, free_layer))
          error = TRUE;

  return (NOT error);
}

/*____________________________________________________________________
|
| Function: Remove_GX3D_Duplicates
|                       
| Input: Called from LWO2_File_To_GX3D_Object()                                                                 
| Output: Removes duplicate vertices from a gx3d object layers.  The 
|   vertices must have the same xyz values, the same texture coords and
|   weights.  Returns true on success, else false on any error.
|
| Description: If a duplication found:
|   - updates the polygon array to replace the reference to the duplicate
|   - updates vertex array by moving everything up, overwriting duplicate
|   - updates tex coord array/s moving everything up, overwriting duplicate
|   - updates weights
|   Finally, reallocates vertex, tex coord array/s if any duplicates were
|   found.
|
|		This is an O(n*n) algorithm - not good for large models.
|___________________________________________________________________*/

static int Remove_GX3D_Duplicates (gx3dObjectLayer *layer)
{
  int i, j, k, n, duplicate, num_vertices;
  gx3dObjectLayer *tlayer;
  int error = FALSE;

  // Remove duplicates, one layer at a time
  for (tlayer=layer; tlayer; tlayer=tlayer->next) {
  
    // Init variables
    num_vertices = tlayer->num_vertices;

    // Remove duplicates
    for (i=0; i<num_vertices-1; i++) {
      for (j=i+1; j<num_vertices; j++) {
      
        // Are these two vertices the same?
        if (memcmp ((void *)&(tlayer->vertex[i]), (void *)&(tlayer->vertex[j]), sizeof(gx3dVector)) == 0) {
          // Are the texture coordinates the same?
          duplicate = TRUE;
          for (k=0; k<tlayer->num_textures; k++) 
            if (memcmp ((void *)&(tlayer->tex_coords[k][i]), (void *)&(tlayer->tex_coords[k][j]), sizeof(gx3dUVCoordinate)) != 0) {
              duplicate = FALSE;
              break;
            }
          // Are the weights the same?
          if (tlayer->weight) 
            if (memcmp ((void *)&(tlayer->weight[i]), (void *)&(tlayer->weight[j]), sizeof(gx3dVertexWeight)) != 0)
              duplicate = FALSE;

          if (duplicate) {
            // Update the polygon array
            for (k=0; k<tlayer->num_polygons; k++) {
              for (n=0; n<3; n++) {
                // Does this polygon part refer to the duplicate?
                if (tlayer->polygon[k].index[n] == j) 
                  // Change it to refer to the original
                  tlayer->polygon[k].index[n] = i;
                // Does this polygon part refer to a vertex beyond the duplicate?
                else if (tlayer->polygon[k].index[n] > j)
                  // Change it by decrementing by 1
                  tlayer->polygon[k].index[n]--;
              }
            }
            // Update vertex, texture and weight array/s
            for (k=j; k<num_vertices-1; k++) {
              tlayer->vertex[k] = tlayer->vertex[k+1];
              for (n=0; n<tlayer->num_textures; n++)
                memcpy ((void *)&(tlayer->tex_coords[n][k]), (void *)&(tlayer->tex_coords[n][k+1]), sizeof(gx3dUVCoordinate));
              if (tlayer->weight)
                tlayer->weight[k] = tlayer->weight[k+1];
            }
            num_vertices--;
            // If eliminated a duplicate, start over
            j--;
          }
        }
      }
    }

    // Any duplicates removed?
    if (num_vertices != tlayer->num_vertices) {
      // Set new count of vertices
      tlayer->num_vertices = num_vertices;
      // Reallocate vertex array
      tlayer->vertex = (gx3dVector *) realloc (tlayer->vertex, tlayer->num_vertices * sizeof(gx3dVector));
      if (tlayer->vertex == NULL)
        error = TRUE;
      // Reallocate diffuse array
      if (tlayer->diffuse) {
        tlayer->diffuse = (gxColor *) realloc (tlayer->diffuse, tlayer->num_vertices * sizeof(gxColor));
        if (tlayer->diffuse == NULL)
          error = TRUE;
      }
      // Reallocate specular array
      if (tlayer->specular) {
        tlayer->specular = (gxColor *) realloc (tlayer->specular, tlayer->num_vertices * sizeof(gxColor));
        if (tlayer->specular == NULL)
          error = TRUE;
      }
      // Reallocate tex coord arrays
      for (i=0; i<tlayer->num_textures; i++) {
        tlayer->tex_coords[i] = (gx3dUVCoordinate *) realloc (tlayer->tex_coords[i], tlayer->num_vertices * sizeof(gx3dUVCoordinate));
        if (tlayer->tex_coords[i] == NULL)
          error = TRUE;
      }
      // Reallocate weight array?
      if (tlayer->weight) {
        tlayer->weight = (gx3dVertexWeight *) realloc (tlayer->weight, tlayer->num_vertices * sizeof(gx3dVertexWeight));
        if (tlayer->weight == 0)
          error = TRUE;
      }
      // Make sure all memory reallocated ok
      if (error) {
        gxError ("Remove_GX3D_Duplicates(): Error reallocating memory");
        error = TRUE;
      }
    }
  } // for (tlayer= ...

/*____________________________________________________________________
|
| Remove duplicates from child layers
|___________________________________________________________________*/

  if (NOT error)
    for (; layer; layer=layer->next) 
      if (layer->child)
        if (NOT Remove_GX3D_Duplicates (layer->child))
          error = TRUE;

  return (NOT error);
}

/*____________________________________________________________________
|
| Function: Compute_GX3D_Polygon_Normals
|                       
| Input: Called from LWO2_File_To_GX3D_Object()                                                                 
| Output: Computes surface normals for the gx3d object layer. Returns 
|   true on success, else false on any error.
|___________________________________________________________________*/

static int Compute_GX3D_Polygon_Normals (gx3dObjectLayer *layer)
{
  int i; 
  gx3dObjectLayer *tlayer;
  int error = FALSE;

  // Compute normals, one layer at a time
  for (tlayer=layer; tlayer AND (NOT error); tlayer=tlayer->next) {

/*____________________________________________________________________
|
| Allocate memory for polygon normals
|___________________________________________________________________*/
  
    // Allocate memory for polygon normal array
    tlayer->polygon_normal = (gx3dVector *) malloc (tlayer->num_polygons * sizeof(gx3dVector));
    if (tlayer->polygon_normal == NULL) {
      gxError ("Compute_GX3D_Polygon_Normals(): Error allocating memory");
      error = TRUE;
    }

/*____________________________________________________________________
|
| Compute Surface Normals
|___________________________________________________________________*/

    for (i=0; (i<tlayer->num_polygons) AND (NOT error); i++) 
      gx3d_SurfaceNormal (&(tlayer->vertex[tlayer->polygon[i].index[0]]),
                          &(tlayer->vertex[tlayer->polygon[i].index[1]]),
                          &(tlayer->vertex[tlayer->polygon[i].index[2]]),
                          &(tlayer->polygon_normal[i]));
  }

/*____________________________________________________________________
|
| Compute normals for child layers
|___________________________________________________________________*/

  if (NOT error)
    for (; layer AND (NOT error); layer=layer->next) 
      if (layer->child)
        if (NOT Compute_GX3D_Polygon_Normals (layer->child))
          error = TRUE;

  return (NOT error);
}

/*____________________________________________________________________
|
| Function: GX3D_Object_To_LWO2_File
|                       
| Input: Called from gx3d_WriteLWO2File()                                                                 
| Output: Writes a LWO2 file using data from a GX3D object.
|___________________________________________________________________*/

void GX3D_Object_To_LWO2_File (gx3dObject *g_object, char *filename)
{
  lwo2_Object *l_object;

  // Create a lwo2 object from the gx3d object data
  l_object = GX3D_To_LWO2 (g_object);
  if (l_object) {
    // Write the LWO2 file
    lwo2_WriteObjectFile (filename, l_object);
    // Free memory for LWO2 object
    lwo2_FreeObject (l_object);
  }
}

/*____________________________________________________________________
|
| Function: GX3D_To_LWO2
|                       
| Input: Called from GX3D_Object_To_LWO2_File()                                                                 
| Output: Creates a LWO2 object using data from a GX3D object.
|___________________________________________________________________*/

static lwo2_Object *GX3D_To_LWO2 (gx3dObject *g_object)
{
  lwo2_Object *l_object = NULL;

  // ...

  return (l_object);
}
