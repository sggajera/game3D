/*____________________________________________________________________
|
| File: gx3d_lwo2.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

int LWO2_File_To_GX3D_Object  (
  char        *filename, 
  gx3dObject  *g_object, 
  unsigned     vertex_format_flags,
  unsigned     flags,
  void       (*free_layer) (gx3dObjectLayer *layer) );

void GX3D_Object_To_LWO2_File (gx3dObject *g_object, char *filename);
