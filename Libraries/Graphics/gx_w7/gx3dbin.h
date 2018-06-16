/*____________________________________________________________________
|
| File: gx3dbin.h
|
| Description: Data structures for GX3DBIN file format.
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Type definitions
|__________________*/

struct gx3dBinFileHeader {
  gx3dBox     bound_box;
  gx3dSphere  bound_sphere;
  int         num_layers;         // 1-?
  bool        has_texcoords;
  bool        has_vertex_normals;
  bool        has_diffuse;
  bool        has_specular;
  bool        has_weights;
  bool        has_skeleton;
};

struct gx3dBinFileLayerHeader {
  int         id;                 // unique ID for this layer (unique w/in a gx3dObject)
  int         parent_id;          // parent ID (valid only if has_parent is true)
  bool        has_parent;         
  bool        has_name;
  gx3dVector  pivot;              // usually the local coord origin (0,0,0) but not always
  gx3dBox     bound_box;
  gx3dSphere  bound_sphere;
  int         num_vertices;
  int         num_polygons;
  int         num_textures;       // 0-8
  int         num_morphs;         // 0-?
};

struct gx3dBinFileMorphHeader {
  char        name[32];
  int         num_entries;  // # of index-offset pairs
};
// followed by:
//int        *index;  // array of indeces into vertex array
//gx3dVector *offset; // array of vertex offsets
