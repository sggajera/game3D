/*____________________________________________________________________
|
| File: texture.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Constants
|__________________*/

// texture types
#define TEXTURE_TYPE_SQUARE           1
#define TEXTURE_TYPE_DYNAMIC_SQUARE   2
#define TEXTURE_TYPE_VOLUME           3
#define TEXTURE_TYPE_CUBEMAP          4
#define TEXTURE_TYPE_DYNAMIC_CUBEMAP  5

/*___________________
|
| Type definitions
|__________________*/

typedef struct _texture_ {
  int   type;                 // type of texture
  int   reference_count;      // # objects using this texture
  char *image_filename;       // associated image file, 0=none
  char *alpha_filename;       // associated alpha file, 0=none

  int   num_slices;           // 1 or more (only applies to volume textures)
  int   num_mip_levels;       // 1 or more
  int   dx;                   // width of texture
  int   dy;                   // height of texture
  int   num_color_bits;
  int   num_alpha_bits; 

  struct _texture_ *next;     // use to create a linked list
  struct _texture_ *previous; // use to create a double-linked list

  unsigned size;              // # bytes driver uses for this texture
  
  // driver-specific data (either a static texture or dynamic)
  byte     *driver_data_static_texture;
  unsigned  driver_data_dynamic_texture;
} Texture;

/*___________________
|
| Function prototypes
|__________________*/

// Init texture manager
void Texture_Init ();

// Free texture manager     
void Texture_Free ();

// Restore texture manager after a context switch
void Texture_Restore ();

// Returns combined size of all currently loaded textures
unsigned Texture_Get_Allocation_Size ();

// Load a texture from data
Texture *Texture_Add_Data (
  int    num_mip_levels,
  byte **image, 
  byte **alphamap,
  int    dx, 
  int    dy, 
  int    num_color_bits,
  int    num_alpha_bits );

// Load a volume texture from data
Texture *Texture_Add_Data_Volume (
  int    num_levels,
  int    num_slices,
  byte **image, 
  byte **alphamap,
  int    dx, 
  int    dy, 
  int    num_color_bits,
  int    num_alpha_bits );

// Load a texture from cubemap data
Texture *Texture_Add_Data_Cubemap (
  byte **image,       // array of 6 image buffers or NULL if creating empty cubemap
  byte **alphamap,    // array of 6 bytemap buffers or NULL
  int    dimensions,
  int    num_color_bits,
  int    num_alpha_bits );

// Load a texture from a file
Texture *Texture_Add_File (
  int    num_mip_levels,
  char **image_filename,  // 0-terminated list of filenames
  char **alpha_filename,  // 0-terminated list of filenames 
  int    dx, 
  int    dy,
  int    num_color_bits,
  int    num_alpha_bits );

// Load a volume texture from a file
Texture *Texture_Add_File_Volume (
  int    num_slices, 
  int    num_mip_levels, 
  char **image_filename,  // list of filenames (# in list is num_filenames * num_mipmaps)
  char **alpha_filename,  // list of filenames or NULL
  int    dx, 
  int    dy, 
  int    num_color_bits, 
  int    num_alpha_bits );

// Load a cubemap texture from a file
Texture *Texture_Add_File_Cubemap (
  char *image_filename, 
  char *alpha_filename, 
  int   dimensions, 
  int   num_color_bits, 
  int   num_alpha_bits );

// Adds a reference to the texture (increments its reference count by 1)
void Texture_AddRef (Texture *texture);

// Release a texture
void Texture_Release (Texture *texture);

// Release all textures
void Texture_Release_All ();

// Set the active drawing texture at stage 0-7
void Texture_Set_Active (int stage, Texture *texture);

// Get the active drawing texture at stage 0-7
Texture *Texture_Get_Active (int stage);

// Get the filenames, if any, associated with a texture
void Texture_Get_Associated_Filenames (
  Texture  *texture, 
  char    **texture_image_file,
  char    **texture_alpha_file );

// Preload a texture
void Texture_Preload (Texture *texture);

// Allows caller to modify/render to a texture
void Texture_Begin_Modify (Texture *texture, int face);

// Ends modify/render to texture
void Texture_End_Modify ();
