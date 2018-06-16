/*____________________________________________________________________
|
| File: gx3d_gx3dbin.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
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
  bool        write_textfile_version );

bool GX3DBIN_File_TO_GX3D_Object (
  char        *filename, 
  gx3dObject  *g_object, 
  unsigned     vertex_format_flags,
  unsigned     flags,
  void       (*free_layer) (gx3dObjectLayer *layer) );
