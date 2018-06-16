/*____________________________________________________________________
|
| File: gx3d_gx3dbin.cpp
|
| Description: Functions to read/write GX3DBIN files.
|
| Functions:  GX3D_Object_To_GX3DBIN_File
|              Count_Layers
|              Process_Layers
|              Process_Geometry_Layer
|               Copy_String
|             GX3DBIN_File_TO_GX3D_Object
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
#include "gx3dbin.h"

#include "gx3d_gx3dbin.h"

/*___________________
|
| Type definitions
|__________________*/

/*___________________
|
| Function prototypes
|__________________*/

static void Count_Layers (gx3dObjectLayer *layer, int *n);
static void Process_Layers (
  gx3dObjectLayer *layer, 
  unsigned         vertex_format, 
  bool             output_texcoords,
  bool             output_vertex_normals,
  bool             output_diffuse_color,
  bool             output_specular_color,
  bool             output_weights,
  bool             output_morphs,
  bool             opengl_formatting,
  bool             write_textfile_version,
  FILE            *out); 
static void Process_Geometry_Layer (
  gx3dObjectLayer *layer, 
  unsigned         vertex_format, 
  bool             output_texcoords,
  bool             output_vertex_normals,
  bool             output_diffuse_color,
  bool             output_specular_color,
  bool             output_weights,
  bool             output_morphs,
  bool             opengl_formatting,
  bool             write_textfile_version,
  FILE            *out); 
static void Copy_String (char *src, char *dst);

/*___________________
|
| Constants
|__________________*/

// If vertex_format_flags has specified VERTEXFORMAT_TEXCOORDS, then tex coords are required from the LWO file!
#define OBJECT_REQUIRES_TEXCOORDS (vertex_format_flags & gx3d_VERTEXFORMAT_TEXCOORDS)

/*____________________________________________________________________
|
| Function: GX3D_Object_To_GX3DBIN_File
|                       
| Input: Called from gx3d_WriteGX3DBINFile()                                                                 
| Output: Writes data to a GX3DBIN File.
|___________________________________________________________________*/

void GX3D_Object_To_GX3DBIN_File (
  char       *filename, 
  gx3dObject *g_object, 
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
  int n;
  FILE *out;
  gx3dBinFileHeader header;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (g_object);
  DEBUG_ASSERT (filename);

/*____________________________________________________________________
|
| Open output file
|___________________________________________________________________*/

  out = fopen (filename, "wb");
  if (out) {

/*____________________________________________________________________
|
| Write output file header
|___________________________________________________________________*/

    // Count the number of layers in the gx3d object
    n = 0;
	  Count_Layers (g_object->layer, &n);      
    // Set values in header
    header.bound_box          = g_object->bound_box;
    header.bound_sphere       = g_object->bound_sphere;
    header.num_layers         = n;
    header.has_texcoords      = output_texcoords;
    header.has_vertex_normals = output_vertex_normals;
    header.has_diffuse        = output_diffuse_color;
    header.has_specular       = output_specular_color;
    header.has_weights        = output_weights;
    header.has_skeleton       = g_object->skeleton AND output_skeleton;
    // Write header to file
    fwrite (&header, sizeof(gx3dBinFileHeader), 1, out);

/*____________________________________________________________________
|
| Write out data for each layer
|___________________________________________________________________*/

    Process_Layers (g_object->layer, 
                    g_object->vertex_format,
                    output_texcoords,
                    output_vertex_normals,
                    output_diffuse_color,
                    output_specular_color,
                    output_weights,
                    output_morphs,
                    opengl_formatting,
                    write_textfile_version,
                    out);
  
/*____________________________________________________________________
|
| Write out skeleton?
|___________________________________________________________________*/

    if (g_object->skeleton AND output_skeleton) {



    }
    
/*____________________________________________________________________
|
| Close output file
|___________________________________________________________________*/

    fclose (out);
  }
}

/*____________________________________________________________________
|
| Function: Count_Layers
| 
| Input: Called from GX3D_Object_To_GX3DBIN_File()
| Output: Counts the number of layers starting with the input layer.
|___________________________________________________________________*/

static void Count_Layers (gx3dObjectLayer *layer, int *n)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (n);

/*____________________________________________________________________
|
| Count layers
|___________________________________________________________________*/

	for (; layer; layer=layer->next) {
  	(*n)++;
		// Look at child layers
		if (layer->child)
			Count_Layers (layer->child, n);
  }
}

/*____________________________________________________________________
|
| Function: Process_Layers
| 
| Input: Called from GX3D_Object_To_GX3DBIN_File()
| Output: Writes layer data to output file starting with the input layer.
|___________________________________________________________________*/

static void Process_Layers (
  gx3dObjectLayer *layer, 
  unsigned         vertex_format, 
  bool             output_texcoords,
  bool             output_vertex_normals,
  bool             output_diffuse_color,
  bool             output_specular_color,
  bool             output_weights,
  bool             output_morphs,
  bool             opengl_formatting,
  bool             write_textfile_version,
  FILE            *out ) 
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (out);

/*____________________________________________________________________
|
| Process layers
|___________________________________________________________________*/

	for (; layer; layer=layer->next) {
    Process_Geometry_Layer (layer, 
                            vertex_format, 
                            output_texcoords,
                            output_vertex_normals, 
                            output_diffuse_color,
                            output_specular_color,
                            output_weights,
                            output_morphs,
                            opengl_formatting,
                            write_textfile_version,
                            out);
		// Process child layers
		if (layer->child)
        Process_Layers (layer, 
                        vertex_format, 
                        output_texcoords,
                        output_vertex_normals,
                        output_diffuse_color,
                        output_specular_color,
                        output_weights,
                        output_morphs,
                        opengl_formatting,
                        write_textfile_version,
                        out);
  }
}

/*____________________________________________________________________
|
| Function: Process_Geometry_Layer
| 
| Input: Called from Process_Layers()
| Output: Writes one geometry layer to output file.
|___________________________________________________________________*/

static void Process_Geometry_Layer (
  gx3dObjectLayer *layer, 
  unsigned         vertex_format, 
  bool             output_texcoords,
  bool             output_vertex_normals,
  bool             output_diffuse_color,
  bool             output_specular_color,
  bool             output_weights,
  bool             output_morphs,
  bool             opengl_formatting,
  bool             write_textfile_version,
  FILE            *out )
{
  int i, j;
	gx3dBinFileLayerHeader header;
  gx3dBinFileMorphHeader morph_header;
  char str[32];

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (layer);
  DEBUG_ASSERT (out);

/*____________________________________________________________________
|
| Write layer header
|___________________________________________________________________*/

  // Set values in header
  header.id                 = layer->id;
  header.parent_id          = layer->parent_id;
  header.has_parent         = (layer->has_parent != 0);
  header.has_name           = (layer->name != NULL);
  header.pivot              = layer->pivot;
  header.bound_box          = layer->bound_box;
  header.bound_sphere       = layer->bound_sphere;
  header.num_vertices       = layer->num_vertices;
  header.num_polygons       = layer->num_polygons;
  header.num_textures       = layer->num_textures;
  if (output_morphs)
    header.num_morphs       = layer->num_morphs;
  else
    header.num_morphs       = 0;
  // Write header to file
  fwrite (&header, sizeof(gx3dBinFileLayerHeader), 1, out);

/*____________________________________________________________________
|
| Write vertices
|___________________________________________________________________*/
  
  if (opengl_formatting) {
    gx3dVector *temp_vertex = (gx3dVector *) malloc (layer->num_vertices * sizeof(gx3dVector));
    for (i=0; i<layer->num_vertices; i++) {
      temp_vertex[i].x =  layer->vertex[i].x;
      temp_vertex[i].y =  layer->vertex[i].y;
      temp_vertex[i].z = -layer->vertex[i].z;
    }
    fwrite (temp_vertex, sizeof(gx3dVector), layer->num_vertices, out);
    free (temp_vertex);
  }
  else
    fwrite (layer->vertex,  sizeof(gx3dVector),  layer->num_vertices, out);

/*____________________________________________________________________
|
| Write polygons
|___________________________________________________________________*/

  if (opengl_formatting) {
    gx3dPolygon *temp_polygon = (gx3dPolygon *) malloc (layer->num_polygons * sizeof(gx3dPolygon));
    for (i=0; i<layer->num_polygons; i++) {
      temp_polygon[i].index[0] = layer->polygon[i].index[0];
      temp_polygon[i].index[1] = layer->polygon[i].index[2];
      temp_polygon[i].index[2] = layer->polygon[i].index[1];
    }
    fwrite (temp_polygon, sizeof(gx3dPolygon), layer->num_polygons, out);
    free (temp_polygon);
  }
  else
    fwrite (layer->polygon, sizeof(gx3dPolygon), layer->num_polygons, out);

/*____________________________________________________________________
|
| Write layer name?
|___________________________________________________________________*/

  if (layer->name) {
    debug_WriteFile ("Process_Geometry_Layer(): Writing layer name");
    Copy_String (layer->name, str);
    fwrite (str, sizeof(char), 32, out);
  }

/*____________________________________________________________________
|
| Write vertex normals?
|___________________________________________________________________*/

  if (output_vertex_normals) {
    debug_WriteFile ("Process_Geometry_Layer(): Writing vertex normals");
    if (opengl_formatting) {
      gx3dVector *temp_normal = (gx3dVector *) malloc (layer->num_vertices * sizeof(gx3dVector));
      for (i=0; i<layer->num_vertices; i++) {
        temp_normal[i].x =  layer->vertex_normal[i].x;
        temp_normal[i].y =  layer->vertex_normal[i].y;
        temp_normal[i].z = -layer->vertex_normal[i].z;
      }
      fwrite (temp_normal, sizeof(gx3dVector), layer->num_vertices, out);
      free (temp_normal);
    }
    else
      fwrite (layer->vertex_normal, sizeof(gx3dVector), layer->num_vertices, out);
  }

/*____________________________________________________________________
|
| Write diffuse color?
|___________________________________________________________________*/

  if ((vertex_format & gx3d_VERTEXFORMAT_DIFFUSE) AND output_diffuse_color) {
    debug_WriteFile ("Process_Geometry_Layer(): Writing diffuse colors");
    if (layer->diffuse == NULL)
      gxError ("Process_Geometry_Layer(): Missing diffuse color array");
    else
      fwrite (layer->diffuse, sizeof(gxColor), layer->num_vertices, out);
  }

/*____________________________________________________________________
|
| Write specular color?
|___________________________________________________________________*/

  if ((vertex_format & gx3d_VERTEXFORMAT_SPECULAR) AND output_specular_color) {
    debug_WriteFile ("Process_Geometry_Layer(): Writing specular colors");
    if (layer->specular == NULL)
      gxError ("Process_Geometry_Layer(): Missing specular color array");
    else
      fwrite (layer->specular, sizeof(gxColor), layer->num_vertices, out);
  }

/*____________________________________________________________________
|
| Write weights?
|___________________________________________________________________*/

  if ((vertex_format & gx3d_VERTEXFORMAT_WEIGHTS) AND output_weights) {
    debug_WriteFile ("Process_Geometry_Layer(): Writing weights");
    if (layer->weight == NULL)
      gxError ("Process_Geometry_Layer(): Missing weight array");
    else
      fwrite (layer->weight, sizeof(gx3dVertexWeight), layer->num_vertices, out);
  }

/*____________________________________________________________________
|
| Write texture data?
|___________________________________________________________________*/

  if ((vertex_format & gx3d_VERTEXFORMAT_TEXCOORDS) AND layer->num_textures AND output_texcoords) {
    debug_WriteFile ("Process_Geometry_Layer(): Writing texture coords");
    // Write texture coordinate sets
    for (i=0; (i<layer->num_textures) AND layer->tex_coords[i]; i++)
      fwrite (layer->tex_coords[i], sizeof(gx3dUVCoordinate), layer->num_vertices, out);
    
    // ADD CODE HERE: write texture filenames
  
  }

/*____________________________________________________________________
|
| Write morph data?
|___________________________________________________________________*/

  if ((vertex_format & gx3d_VERTEXFORMAT_MORPHS) AND layer->num_morphs AND output_morphs) {
    debug_WriteFile ("Process_Geometry_Layer(): Writing morphs");
    for (i=0; i<layer->num_morphs; i++) {
      // Create header
      Copy_String (layer->morph[i].name, morph_header.name);
      morph_header.num_entries = layer->morph[i].num_entries;
      // Write header
      fwrite (&morph_header, sizeof(gx3dBinFileMorphHeader), 1, out);
      // Write index array
      fwrite (layer->morph[i].index, sizeof(int), layer->morph[i].num_entries, out);
      // Write offset array
      if (opengl_formatting) {
        gx3dVector *temp_offset = (gx3dVector *) malloc (layer->morph[i].num_entries * sizeof(gx3dVector));
        for (j=0; j<layer->morph[i].num_entries; j++) {
          temp_offset[j].x =  layer->morph[i].offset[j].x;
          temp_offset[j].y =  layer->morph[i].offset[j].y;
          temp_offset[j].z = -layer->morph[i].offset[j].z;
        }
        fwrite (temp_offset, sizeof(gx3dVector), layer->morph[i].num_entries, out);
        free (temp_offset);
      }
      else
        fwrite (layer->morph[i].offset, sizeof(gx3dVector), layer->morph[i].num_entries, out);
    }
  }
}

/*____________________________________________________________________
|
| Function: Copy_String
| 
| Input: Called from Process_Geometry_Layer()
| Output: Copies up to 31 bytes from src string into dst string.  Null
|   terminates dst string.
|___________________________________________________________________*/

static void Copy_String (char *src, char *dst)
{

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (src);
  DEBUG_ASSERT (dst);

/*____________________________________________________________________
|
| Copy data
|___________________________________________________________________*/

  strncpy (dst, src, 31);
  dst[31] = 0;
}

/*____________________________________________________________________
|
| Function: GX3DBIN_File_TO_GX3D_Object
|                       
| Input: Called from gx3d_ReadLWO2File()                                                                 
| Output: Reads data from a GX3DBIN File and puts it in a gx3d object.
|       Returns true on success, else false.
|___________________________________________________________________*/

bool GX3DBIN_File_TO_GX3D_Object (
  char        *filename, 
  gx3dObject  *g_object, 
  unsigned     vertex_format_flags,
  unsigned     flags,
  void       (*free_layer) (gx3dObjectLayer *layer) )
{
  // ADD CODE HERE

  return (false);
}
