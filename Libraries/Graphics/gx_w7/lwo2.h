/*____________________________________________________________________
|
| File: lwo2.h
|
| Description: Hierarchy of LWO2 data is:
|   layer
|     polytag_list
|       surface
|         block          
|           vertexmap_name
|             vmap
|               clip
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Type definitions
|__________________*/

struct lwo2_Color {
  float r,g,b;
};

struct lwo2_Point {
  float x, y, z;  
};

struct lwo2_Polygon {
  int num_vertices; // 1, 2 or 3
  int index[3];     // index into vertex array
};

struct lwo2_BoundBox {
  lwo2_Point min;
  lwo2_Point max;
};

enum lwo2_VertexMapType {
  weight_map     = 1,
  uv_map         = 2,
  rgb_color_map  = 3,
  rgba_color_map = 4,
  morph_map      = 5,
};

struct lwo2_VertexMap {
  lwo2_VertexMapType  type;
  int                 weight_map_id;      // 0-? equal to the position in the linked list (first node=0, second node=1, etc.), weight maps only
  int                 dimension;          // vector length (example UV maps have dimension=2)
  int                 num_entries;        // # of index-value pairs
  char               *name;               // name (optional)
  int                *index_array;        // array of indeces into layer's vertex array
  float              *value_array;        // array of values
  lwo2_VertexMap     *next;               // used to create a linked list
}; 

enum lwo2_PolyTagType {
  lwo2_POLYTAGTYPE_SURFACE        = 1,
  lwo2_POLYTAGTYPE_BONE_NAME      = 2,
  lwo2_POLYTAGTYPE_BONE_WEIGHTMAP = 3,
};

// This struct associates polygons with surfaces, bone names or bone weight maps
struct lwo2_PolyTag {
  lwo2_PolyTagType  type;
  int              *polygon_array;    // array of indeces into layer's polygon array
  int              *tags_index_array; // array of 0-based indeces into tags_array in object
  lwo2_PolyTag     *next;
};

struct lwo2_Layer {
  int             number;         // unique ID for this layer
  int             hidden;         // boolean
  int             skeleton;       // boolean
  lwo2_Point      pivot;
  char           *name;           // name
  int            *parent;         // parent layer number
  int             num_vertices;
  lwo2_Point     *vertex_array;   // array of vertices
  lwo2_BoundBox  *bound;          // bounding box
  lwo2_VertexMap *vmap_list;      // linked list of vertex maps
  int             num_polygons;
  lwo2_Polygon   *polygon_array;  // array of polygons
  lwo2_PolyTag   *polytag_list;   // linked list of polygon tags
  // used by search routines
  int             processed;
  lwo2_Layer     *next;  
};

enum lwo2_BlockType {
  image_texture      = 1,
  procedural_texture = 2,
  gradient_texture   = 3,
};

// Specifies how color of the texture is derived for areas outside the image
enum lwo2_TextureWrapType {
  wrap_type_reset  = 0,               // areas outside are black
  wrap_type_repeat = 1,               // repeat (default)
  wrap_type_mirror = 2,               // mirror
  wrap_type_edge   = 3,               // edge (color taken from image's nearest edge pixel
};

struct lwo2_Block {
  lwo2_BlockType        type;
  int                   opacity_type; // 0=additive, 1=subtractive, 2=difference, 3=multiply, 4=divide, 5=alpha, 6=texture displacement
  int                  *clip_id;      // id of a clip in clip list of the mapped image
  lwo2_TextureWrapType  width_wrap;  
  lwo2_TextureWrapType  height_wrap;
  char                 *vertexmap_name; // same string as in a vertexmap node
  lwo2_Block           *next;
};

// This is the struct that defines a surface (material)
struct lwo2_Surface {
  char         *name;       // uniquely identifies this surface
  char         *source;     // name of a source surface, if any
  lwo2_Color    color;      // base color (default=0,0,0)
  lwo2_Block   *block_list; // linked list of blocks
  lwo2_Surface *next;
};

struct lwo2_Clip {
  int        id;          // unique non-zero integer for this clip
  char      *filename;    // texture filename
  lwo2_Clip *next;
};

struct lwo2_Object {
  int           num_tags;             // # tags in tags array
  char        **tags_array;           // array of tag strings
  lwo2_Layer   *layer_list;           // linked list of layers (optional)
  lwo2_Surface *surface_list;         // linked list of surfaces
  lwo2_Clip    *clip_list;            // linked list of clips
};

/*___________________
|
| Function prototypes
|__________________*/

lwo2_Object *lwo2_ReadObjectFile (char *filename);
void         lwo2_WriteObjectFile (char *filename, lwo2_Object *object);
void         lwo2_FreeObject (lwo2_Object *object);
