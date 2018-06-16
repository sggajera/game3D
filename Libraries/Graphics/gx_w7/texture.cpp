/*____________________________________________________________________
|
| File: texture.cpp
|
| Description: Functions to manage textures.
|
| Functions:  Texture_Init
|             Texture_Free
|             Texture_Restore
|             Texture_Get_Allocation_Size
|             Texture_Add_Data
|             Texture_Add_Data_Volume
|             Texture_Add_Data_Cubemap
|             Texture_Add_File
|             Texture_Add_File_Volume
|             Texture_Add_File_Cubemap
|             Texture_AddRef
|             Texture_Release
|             Texture_Release_All
|             Texture_Set_Active
|             Texture_Get_Active
|             Texture_Get_Associated_Filenames
|             Texture_Preload
|             Texture_Begin_Modify
|             Texture_End_Modify
|             Texture_Add_Reference
|             Texture_Subtract_Reference
|
| Description: There are three ways to load textures using the gx library.
|   First way is to use gx3d_InitTexture...() routines to do a low-level
|   load.  Second, use gx3d_InitTexture_File() to do a high-level managed
|   load which will screen out duplicates.  Third is to call a 3D object
|   file read function like gx3d_ReadLWO2File(), which will also do a
|   high-level managed load.
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

#include "dp.h"

#include "texture.h"

/*___________________
|
| Defines
|__________________*/

#define FREE_TEXTURE(_tex_)                                                     \
  switch (_tex_->type) {                                                        \
    case TEXTURE_TYPE_SQUARE:                                                   \
    case TEXTURE_TYPE_VOLUME:                                                   \
    case TEXTURE_TYPE_CUBEMAP:                                                  \
      if (gx_Video.free_texture)                                                \
        (*gx_Video.free_texture) (_tex_->driver_data_static_texture);           \
      break;                                                                    \
    case TEXTURE_TYPE_DYNAMIC_SQUARE:                                           \
    case TEXTURE_TYPE_DYNAMIC_CUBEMAP:                                          \
      if (gx_Video.free_dynamic_texture)                                        \
        (*gx_Video.free_dynamic_texture) (_tex_->driver_data_dynamic_texture);  \
      break;                                                                    \
  }

#define SET_TEXTURE_TO_NONE(_stage_)        \
  {                                         \
    if (gx_Video.set_texture)               \
      (*gx_Video.set_texture) (_stage_, 0); \
  }

#define NUM_CUBE_FACES  6

/*___________________
|
| Global variables
|__________________*/

static Texture *texture_list;
static Texture *active_texture [gx3d_NUM_TEXTURE_STAGES];

#ifdef DEBUG
static char debug_str [256];
#endif

/*____________________________________________________________________
|
| Function: Texture_Init
|
| Output: Init texture manager.
|___________________________________________________________________*/

void Texture_Init ()
{
  int i;
  // Init globals
  texture_list = NULL;
  for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) {
    SET_TEXTURE_TO_NONE (i)
    active_texture[i] = NULL;
  }
}

/*____________________________________________________________________
|
| Function: Texture_Free
|
| Output: Free texture manager.
|___________________________________________________________________*/

void Texture_Free ()
{
  Texture_Release_All ();
}

/*____________________________________________________________________
|
| Function: Texture_Restore
|
| Output: Restores texture manager after a context switch.
|___________________________________________________________________*/

void Texture_Restore ()
{
  int i;
  // Init globals
  for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) {
    SET_TEXTURE_TO_NONE (i)
    active_texture[i] = NULL;
  }
}

/*____________________________________________________________________
|
| Function: Texture_Get_Allocation_Size
|
| Output: Returns combined size of all currently loaded textures.
|___________________________________________________________________*/

unsigned Texture_Get_Allocation_Size ()
{
  Texture *tp;
  unsigned size = 0;

  for (tp=texture_list; tp; tp=tp->next) 
    size += tp->size;

  return (size);
}

/*____________________________________________________________________
|
| Function: Texture_Add_Data
|
| Output: Load a texture from data.
|___________________________________________________________________*/

Texture *Texture_Add_Data (
  int    num_mip_levels,
  byte **image, 
  byte **alphamap,
  int    dx, 
  int    dy, 
  int    num_color_bits,
  int    num_alpha_bits )
{
  int registered;
  unsigned  size;
  byte     *static_texture;
  unsigned  dynamic_texture;
  Texture  *texture = NULL;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  registered      = FALSE;
  static_texture  = NULL;
  dynamic_texture = 0;

/*____________________________________________________________________
|
| Register this texture with driver
|___________________________________________________________________*/

  // Create a static texture?
  if (image OR alphamap) {
    static_texture = (*gx_Video.init_texture) (num_mip_levels, image, alphamap, dx, dy, num_color_bits, num_alpha_bits, &size);
    if (static_texture)
      registered = TRUE;
  }
  // Create a dynamic texture?
  else {
    dynamic_texture = (*gx_Video.init_dynamic_texture) (dx, dy, num_color_bits, num_alpha_bits, &size);
    if (dynamic_texture) 
      registered = TRUE;
  }

/*____________________________________________________________________
|
| Add texture to linked list of textures
|___________________________________________________________________*/

  // Was texture registered ok?
  if (registered) {
    // Create a texture node to save this texture info in
    texture = (Texture *) calloc (1, sizeof(Texture));
    if (texture) {
      // Fill texture node with values
      if (image OR alphamap) {
        texture->type = TEXTURE_TYPE_SQUARE;
        texture->driver_data_static_texture = static_texture;
      }
      else {
        texture->type = TEXTURE_TYPE_DYNAMIC_SQUARE;
        texture->driver_data_dynamic_texture = dynamic_texture;
      }
      texture->reference_count = 1;
      texture->num_mip_levels  = num_mip_levels;  // should be 1 for dynamic textures
      texture->dx              = dx;
      texture->dy              = dy;
      texture->num_color_bits  = num_color_bits;
      texture->num_alpha_bits  = num_alpha_bits;
      texture->size            = size;
      // Attach this new node to the start of the texture list
      if (texture_list == NULL)
        texture_list = texture;
      else {
        texture_list->previous = texture;
        texture->next = texture_list;
        texture_list = texture;
      }
    }
  }

  return (texture);
}

/*____________________________________________________________________
|
| Function: Texture_Add_Data_Volume
|
| Output: Load a volume texture from data.
|___________________________________________________________________*/

Texture *Texture_Add_Data_Volume (
  int    num_levels,  // # mip levels
  int    num_slices,
  byte **image, 
  byte **alphamap,
  int    dx, 
  int    dy, 
  int    num_color_bits,
  int    num_alpha_bits )
{
  int registered;
  unsigned  size;
  byte     *static_texture;
  Texture  *texture = NULL;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  registered      = FALSE;
  static_texture  = NULL;

/*____________________________________________________________________
|
| Register this texture with driver
|___________________________________________________________________*/

  // Create a static texture?
  if (image OR alphamap) {
    static_texture = (*gx_Video.init_volume_texture) (num_levels, num_slices, image, alphamap, dx, dy, num_color_bits, num_alpha_bits, &size);
    if (static_texture)
      registered = TRUE;
  }

/*____________________________________________________________________
|
| Add texture to linked list of textures
|___________________________________________________________________*/

  // Was texture registered ok?
  if (registered) {
    // Create a texture node to save this texture info in
    texture = (Texture *) calloc (1, sizeof(Texture));
    if (texture) {
      // Fill texture node with values
      texture->type = TEXTURE_TYPE_VOLUME;
      texture->driver_data_static_texture = static_texture;
      texture->reference_count = 1;
      texture->num_slices      = num_slices;
      texture->num_mip_levels  = num_levels;
      texture->dx              = dx;
      texture->dy              = dy;
      texture->num_color_bits  = num_color_bits;
      texture->num_alpha_bits  = num_alpha_bits;
      texture->size            = size;
      // Attach this new node to the start of the texture list
      if (texture_list == NULL)
        texture_list = texture;
      else {
        texture_list->previous = texture;
        texture->next = texture_list;
        texture_list = texture;
      }
    }
  }

  return (texture);
}

/*____________________________________________________________________
|
| Function: Texture_Add_Data_Cubemap
|
| Output: Load a texture from data.
|___________________________________________________________________*/

Texture *Texture_Add_Data_Cubemap (
  byte **image,       // array of 6 image buffers or NULL if creating empty cubemap
  byte **alphamap,    // array of 6 bytemap buffers or NULL
  int    dimensions,
  int    num_color_bits,
  int    num_alpha_bits )
{
  int registered;
  unsigned  size;
  byte     *static_texture;
  unsigned  dynamic_texture;
  Texture  *texture = NULL;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  registered      = FALSE;
  static_texture  = NULL;
  dynamic_texture = 0;

/*____________________________________________________________________
|
| Register this texture with driver
|___________________________________________________________________*/

  // Create a static texture?
  if (image OR alphamap) {
    static_texture = (*gx_Video.init_cubemap_texture) (image, alphamap, dimensions, num_color_bits, num_alpha_bits, &size);
    if (static_texture)
      registered = TRUE;
  }
  // Create a dynamic texture?
  else {
    dynamic_texture = (*gx_Video.init_dynamic_cubemap_texture) (dimensions, num_color_bits, num_alpha_bits, &size);
    if (dynamic_texture) 
      registered = TRUE;
  }
  
/*____________________________________________________________________
|
| Add texture to linked list of textures
|___________________________________________________________________*/

  // Was texture registered ok?
  if (registered) {
    // Create a texture node to save this texture info in
    texture = (Texture *) calloc (1, sizeof(Texture));
    if (texture) {
      // Fill texture node with values
      if (image OR alphamap) {
        texture->type = TEXTURE_TYPE_CUBEMAP;
        texture->driver_data_static_texture = static_texture;
      }
      else {
        texture->type = TEXTURE_TYPE_DYNAMIC_CUBEMAP;
        texture->driver_data_dynamic_texture = dynamic_texture;
      }
      texture->reference_count = 1;
      texture->num_mip_levels  = 1;
      texture->dx              = dimensions;
      texture->dy              = dimensions;
      texture->num_color_bits  = num_color_bits;
      texture->num_alpha_bits  = num_alpha_bits;
      texture->size            = size;
      // Attach this new node to the start of the texture list
      if (texture_list == NULL)
        texture_list = texture;
      else {
        texture_list->previous = texture;
        texture->next = texture_list;
        texture_list = texture;
      }
    }
  }

  return (texture);
}

/*____________________________________________________________________
|
| Function: Texture_Add_File
|
| Output: Load a texture from a file.  If a texture has already been
|   loaded from the same filename, this function will return a handle
|   to the previously created texture and will not create a new texture.
|___________________________________________________________________*/

Texture *Texture_Add_File (
  int    num_mip_levels,  // 1 or more
  char **image_filename,  // 0-terminated list of filenames
  char **alpha_filename,  // 0-terminated list of filenames 
  int    dx, 
  int    dy,
  int    num_color_bits,
  int    num_alpha_bits )
{
  int i, found, memory_allocation_error;
  gxBound box;
  gxPage vpage;
  gxState state;
  byte *image, **image_array, **alpha_array, **image_data, **bytemap_data;
  Texture *tp;
  Texture *texture = NULL;

#ifdef DEBUG
  debug_WriteFile ("Texture_Add_File():");
  for (i=0; i<num_mip_levels; i++) {
    sprintf (debug_str, "  texture:  %s", image_filename[i]);
    debug_WriteFile (debug_str);
    if (alpha_filename[i]) {
      sprintf (debug_str, "    alpha:  %s", alpha_filename[i]);
      debug_WriteFile (debug_str);
    }
  }  
  sprintf (debug_str, " mipmaps: %d", num_mip_levels);
  debug_WriteFile (debug_str);
#endif

/*____________________________________________________________________
|
| Search texture list for an instance of this texture that is already loaded
|___________________________________________________________________*/

  for (tp=texture_list, found=FALSE; tp AND (NOT found); tp=tp->next) {
    if (tp->image_filename) {
      if (!strcmp(image_filename[0], tp->image_filename)) {
        if ((alpha_filename[0] == NULL) AND (tp->alpha_filename == NULL))
          found = TRUE;
        else if (alpha_filename[0] AND tp->alpha_filename) {
          if (!strcmp(alpha_filename[0], tp->alpha_filename))
            found = TRUE;
        }
        if (found) {
          // Use this texture and incr its reference count instead of creating a new texture
          Texture_AddRef (tp);
          texture = tp;
        }
      }
    }
  }

/*____________________________________________________________________
|
| Create a new texture?
|___________________________________________________________________*/

  if (texture == NULL) {
    // Init variables
    image_array  = NULL;
    alpha_array  = NULL;
    image_data   = NULL;
    bytemap_data = NULL;
      
/*____________________________________________________________________
|
| Allocate memory
|___________________________________________________________________*/

    memory_allocation_error = FALSE;
    
    image_array = (byte **) calloc (num_mip_levels, sizeof(byte *));
    image_data  = (byte **) calloc (num_mip_levels, sizeof(byte *));
    if ((image_array == NULL) OR (image_data == NULL))
      memory_allocation_error = TRUE;
    if (alpha_filename[0]) {
      alpha_array  = (byte **) calloc (num_mip_levels, sizeof(byte *));
      bytemap_data = (byte **) calloc (num_mip_levels, sizeof(byte *));
      if ((alpha_array == NULL) OR (bytemap_data == NULL))
        memory_allocation_error = TRUE;
    }

    // Create a virtual page to load data from files into
    if (NOT memory_allocation_error) 
      if (NOT gxCreateVirtualPage (dx, dy, gxHINT_CREATE_IN_SYSTEM_MEMORY, &vpage))
        memory_allocation_error = TRUE;

/*____________________________________________________________________
|
| Build image/alpha pixmap arrays
|___________________________________________________________________*/

    if (NOT memory_allocation_error) {

      gxSaveState (&state);
      gxSetActivePage (vpage);
      
      // Build arrays
      box.x = 0;
      box.y = 0;
      for (i=0; i<num_mip_levels; i++) {
        // Create image pixmap from file
        gxGetBMPFileDimensions (image_filename[i], &box.w, &box.h, NULL);
        gxReadBMPFile (image_filename[i], 1);
        image_array[i] = gxCreateImage (box);
        image_data[i] = image_array[i] + (2 * sizeof(unsigned));
        // Create alpha bytemap from file?
        if (alpha_filename[i]) {
          gxReadBMPFile (alpha_filename[i], 1);
          image = gxCreateImage (box);
          alpha_array[i] = gxCreateBytemap (image);
          bytemap_data[i] = alpha_array[i] + (2 * sizeof(unsigned));
          free (image);
        }
      }

/*____________________________________________________________________
|
| Create the new texture and add it to the texture list
|___________________________________________________________________*/

      texture = Texture_Add_Data (num_mip_levels,
                                  image_data,
                                  bytemap_data,
                                  dx,
                                  dy,
                                  num_color_bits,
                                  num_alpha_bits );
      // Save the filenames used to create this texture
      if (texture) {
        texture->image_filename = (char *) calloc (strlen(image_filename[0])+1, sizeof(char));
        strcpy (texture->image_filename, image_filename[0]);
        if (alpha_filename[0]) {
          texture->alpha_filename = (char *) calloc (strlen(alpha_filename[0])+1, sizeof(char));
          strcpy (texture->alpha_filename, alpha_filename[0]);
        }
      }

      gxRestoreState (&state);
      gxFreeVirtualPage (vpage);
    } // if (NOT memory_allocation_error)
      
/*____________________________________________________________________
|
| Free memory
|___________________________________________________________________*/

    if (image_array) {
      for (i=0; i<num_mip_levels; i++) 
        if (image_array[i])
          free (image_array[i]);
      free (image_array);
    }
    if (alpha_array) {
      for (i=0; i<num_mip_levels; i++) 
        if (alpha_array[i])
          free (alpha_array[i]);
      free (alpha_array);
    }
    if (image_data)
      free (image_data);
    if (bytemap_data)
      free (bytemap_data);
  } // if (texture == NULL)

  return (texture);
}

/*____________________________________________________________________
|
| Function: Texture_Add_File_Volume
|
| Output: Loads a volume texture from a file.  If a texture has already
|   been loaded from the same filename, this function will return a handle
|   to the previously created texture and will not create a new texture.
|___________________________________________________________________*/

// Load a volume texture from a file
Texture *Texture_Add_File_Volume (
  int    num_levels,      // # of mip levels
  int    num_slices,
  char **image_filename,  // list of filenames
  char **alpha_filename,  // list of filenames or NULL
  int    dx, 
  int    dy, 
  int    num_color_bits, 
  int    num_alpha_bits )
{
  int i, found, memory_allocation_error, num_filenames;
  gxBound box;
  gxPage vpage;
  gxState state;
  byte *image, **image_array, **alpha_array, **image_data, **bytemap_data;
  Texture *tp;
  Texture *texture = NULL;

/*____________________________________________________________________
|
| Search texture list for an instance of this texture that is already loaded
|___________________________________________________________________*/

  for (tp=texture_list, found=FALSE; tp AND (NOT found); tp=tp->next) {
    if (tp->image_filename) {
      if (!strcmp(image_filename[0], tp->image_filename)) {
        if ((alpha_filename[0] == NULL) AND (tp->alpha_filename == NULL))
          found = TRUE;
        else if (alpha_filename AND tp->alpha_filename) {
          if (!strcmp(alpha_filename[0], tp->alpha_filename))
            found = TRUE;
        }
        if (found) {
          // Use this texture and incr its reference count instead of creating a new texture
          Texture_AddRef (tp);
          texture = tp;
        }
      }
    }
  }

/*____________________________________________________________________
|
| Create a new texture?
|___________________________________________________________________*/

  if (texture == NULL) {
    // Init variables
    image_array  = NULL;
    alpha_array  = NULL;
    image_data   = NULL;
    bytemap_data = NULL;

/*____________________________________________________________________
|
| Allocate memory
|___________________________________________________________________*/

    memory_allocation_error = FALSE;
    
    // Compute # of filenames
    for (num_filenames=0,i=0; i<num_levels; i++)
      num_filenames += (num_slices>>i);
    
    image_array = (byte **) calloc (num_filenames, sizeof(byte *));
    image_data  = (byte **) calloc (num_filenames, sizeof(byte *));
    if ((image_array == NULL) OR (image_data == NULL))
      memory_allocation_error = TRUE;
    if (alpha_filename) {
      alpha_array  = (byte **) calloc (num_filenames, sizeof(byte *));
      bytemap_data = (byte **) calloc (num_filenames, sizeof(byte *));
      if ((alpha_array == NULL) OR (bytemap_data == NULL))
        memory_allocation_error = TRUE;
    }

    // Create a virtual page to load data from files into
    if (NOT memory_allocation_error) 
      if (NOT gxCreateVirtualPage (dx, dy, gxHINT_CREATE_IN_SYSTEM_MEMORY, &vpage))
        memory_allocation_error = TRUE;

/*____________________________________________________________________
|
| Build image/alpha pixmap arrays
|___________________________________________________________________*/

    if (NOT memory_allocation_error) {

      gxSaveState (&state);
      gxSetActivePage (vpage);
      
      // Build arrays
      box.x = 0;
      box.y = 0;
      for (i=0; i<num_filenames; i++) {
        // Create image pixmap from file
        gxGetBMPFileDimensions (image_filename[i], &box.w, &box.h, NULL);
        gxReadBMPFile (image_filename[i], 1);
        image_array[i] = gxCreateImage (box);
        image_data[i] = image_array[i] + (2 * sizeof(unsigned));
        // Create alpha bytemap from file?
        if (alpha_filename) {
          gxReadBMPFile (alpha_filename[i], 1);
          image = gxCreateImage (box);
          alpha_array[i] = gxCreateBytemap (image);
          bytemap_data[i] = alpha_array[i] + (2 * sizeof(unsigned));
          free (image);
        }
      }

/*____________________________________________________________________
|
| Create the new texture and add it to the texture list
|___________________________________________________________________*/

      texture = Texture_Add_Data_Volume (num_levels,
                                         num_slices,
                                         image_data,
                                         bytemap_data,
                                         dx,
                                         dy,
                                         num_color_bits,
                                         num_alpha_bits );
      // Save the filenames used to create this texture
      if (texture) {
        texture->image_filename = (char *) calloc (strlen(image_filename[0])+1, sizeof(char));
        strcpy (texture->image_filename, image_filename[0]);
        if (alpha_filename) {
          texture->alpha_filename = (char *) calloc (strlen(alpha_filename[0])+1, sizeof(char));
          strcpy (texture->alpha_filename, alpha_filename[0]);
        }
      }

      gxRestoreState (&state);
      gxFreeVirtualPage (vpage);
    } // if (NOT memory_allocation_error)
      
/*____________________________________________________________________
|
| Free memory
|___________________________________________________________________*/

    if (image_array) {
      for (i=0; i<num_filenames; i++) 
        if (image_array[i])
          free (image_array[i]);
      free (image_array);
    }
    if (alpha_array) {
      for (i=0; i<num_filenames; i++) 
        if (alpha_array[i])
          free (alpha_array[i]);
      free (alpha_array);
    }
    if (image_data)
      free (image_data);
    if (bytemap_data)
      free (bytemap_data);
  } // if (texture == NULL)

  return (texture);
}

/*____________________________________________________________________
|
| Function: Texture_Add_File_Cubemap
|
| Output: Load a texture from a file.  If a texture has already been
|   loaded from the same filename, this function will return a handle
|   to the previously created texture and will not create a new texture.
|___________________________________________________________________*/

Texture *Texture_Add_File_Cubemap (
  char *image_filename,
  char *alpha_filename,
  int   dimensions, 
  int   num_color_bits,
  int   num_alpha_bits )
{
  int i, found, memory_allocation_error;
  gxBound box;
  gxPage vpage;
  gxState state;
  byte *image, **image_array, **image_data, **alpha_array, **bytemap_data;
  
  Texture *tp;
  Texture *texture = NULL;

#ifdef DEBUG
  debug_WriteFile ("Texture_Add_File_Cubemap():");
  sprintf (debug_str, "  texture: %s", image_filename);
  debug_WriteFile (debug_str);
  if (alpha_filename) {
    sprintf (debug_str, "  alpha: %s", alpha_filename);
    debug_WriteFile (debug_str);
  }
#endif

/*____________________________________________________________________
|
| Search texture list for an instance of this texture that is already loaded
|___________________________________________________________________*/

  for (tp=texture_list, found=FALSE; tp AND (NOT found); tp=tp->next) {
    if (tp->image_filename) {
      if (!strcmp(image_filename, tp->image_filename)) {
        if ((alpha_filename == NULL) AND (tp->alpha_filename == NULL))
          found = TRUE;
        else if (alpha_filename AND tp->alpha_filename) {
          if (!strcmp(alpha_filename, tp->alpha_filename))
            found = TRUE;
        }
        if (found) {
          // Use this texture and incr its reference count instead of creating a new texture
          Texture_AddRef (tp);
          texture = tp;
        }
      }
    }
  }

/*____________________________________________________________________
|
| Create a new texture?
|___________________________________________________________________*/

  if (texture == NULL) {
    // Init variables
    image_array  = NULL;
    alpha_array  = NULL;
    image_data   = NULL;
    bytemap_data = NULL;

/*____________________________________________________________________
|
| Allocate memory
|___________________________________________________________________*/

    memory_allocation_error = FALSE;
    
    image_array = (byte **) calloc (NUM_CUBEMAP_FACES, sizeof(byte *));
    image_data  = (byte **) calloc (NUM_CUBEMAP_FACES, sizeof(byte *));
    if ((image_array == NULL) OR (image_data == NULL))
      memory_allocation_error = TRUE;
    if (alpha_filename) {
      alpha_array  = (byte **) calloc (NUM_CUBEMAP_FACES, sizeof(byte *));
      bytemap_data = (byte **) calloc (NUM_CUBEMAP_FACES, sizeof(byte *));
      if ((alpha_array == NULL) OR (bytemap_data == NULL))
        memory_allocation_error = TRUE;
    }

    // Create a virtual page to load data from files into
    if (NOT memory_allocation_error) 
      if (NOT gxCreateVirtualPage (dimensions*6, dimensions, gxHINT_CREATE_IN_SYSTEM_MEMORY, &vpage))
        memory_allocation_error = TRUE;

/*____________________________________________________________________
|
| Build image/alpha pixmap arrays
|___________________________________________________________________*/

    if (NOT memory_allocation_error) {

      gxSaveState (&state);
      gxSetActivePage (vpage);
      box.y = 0;
      box.w = dimensions;
      box.h = dimensions;

      // Load image data from file
      gxReadBMPFile (image_filename, 1);
      // Build image array
      for (i=0; i<NUM_CUBEMAP_FACES; i++) {
        box.x = i * dimensions;
        image_array[i] = gxCreateImage (box);
        image_data[i] = image_array[i] + (2 * sizeof(unsigned));
      }

      // Load alpha data from file?
      if (alpha_filename) {
        // Load image data from file
        gxReadBMPFile (alpha_filename, 1);
        // Build alpha bytemap array from file
        for (i=0; i<NUM_CUBEMAP_FACES; i++) {
          box.x = i * dimensions;
          image = gxCreateImage (box);
          alpha_array[i] = gxCreateBytemap (image);
          bytemap_data[i] = alpha_array[i] +  (2 * sizeof(unsigned));
          free (image);
        }
      }

/*____________________________________________________________________
|
| Create the new texture and add it to the texture list
|___________________________________________________________________*/

      texture = Texture_Add_Data_Cubemap (image_data, bytemap_data, dimensions, num_color_bits, num_alpha_bits);

      // Save the filenames used to create this texture
      if (texture) {
        texture->image_filename = (char *) calloc (strlen(image_filename)+1, sizeof(char));
        strcpy (texture->image_filename, image_filename);
        if (alpha_filename) {
          texture->alpha_filename = (char *) calloc (strlen(alpha_filename)+1, sizeof(char));
          strcpy (texture->alpha_filename, alpha_filename);
        }
      }

      gxRestoreState (&state);
      gxFreeVirtualPage (vpage);
    } // if (NOT memory_allocation_error)

/*____________________________________________________________________
|
| Free memory
|___________________________________________________________________*/

    if (image_array) {
      for (i=0; i<NUM_CUBEMAP_FACES; i++) 
        if (image_array[i])
          free (image_array[i]);
      free (image_array);
    }
    if (alpha_array) {
      for (i=0; i<NUM_CUBEMAP_FACES; i++) 
        if (alpha_array[i])
          free (alpha_array[i]);
      free (alpha_array);
    }
    if (image_data)
      free (image_data);
    if (bytemap_data)
      free (bytemap_data);
  } // if (texture == NULL)

  return (texture);
}


/*____________________________________________________________________
|
| Function: Texture_AddRef
|
| Output: Adds a reference to a texture.
|___________________________________________________________________*/

void Texture_AddRef (Texture *texture)
{
  if (texture) 
    // Increment the number of uses of this texture
    texture->reference_count++;
}

/*____________________________________________________________________
|
| Function: Texture_Release
|
| Output: Release a texture.
|___________________________________________________________________*/

void Texture_Release (Texture *texture)
{
  int i;

  if (texture) {
    // Decrement the number of uses of this texture
    texture->reference_count--;
    // If no longer needed, delete this texture
    if (texture->reference_count == 0) {
      // If this is an active texture, deactivate it
      for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) 
        if (texture == active_texture[i]) {
          SET_TEXTURE_TO_NONE (i);
          active_texture[i] = NULL;
          break;
        }
      // Free memory for filename strings, if any
      if (texture->image_filename)
        free (texture->image_filename);
      if (texture->alpha_filename)
        free (texture->alpha_filename);
      // Free driver-specific data
      FREE_TEXTURE (texture);
      // Delete this texture node from the texture linked list
      if (texture == texture_list) {
        texture_list = texture->next;
        if (texture->next)
          texture->next->previous = NULL;
      }
      else {
        texture->previous->next = texture->next;
        if (texture->next)
          texture->next->previous = texture->previous;
      }
      // Free memory for this texture node
      free (texture);
    }
  }
}

/*____________________________________________________________________
|
| Function: Texture_Release_All
|
| Output: Release all textures.
|___________________________________________________________________*/

void Texture_Release_All ()
{
  int i;
  Texture *tp, *ttp;

  tp = texture_list;
  while (tp) {
    // Free memory for filename strings, if any
    if (tp->image_filename)
      free (tp->image_filename);
    if (tp->alpha_filename)
      free (tp->alpha_filename);
    // Free driver-specific data
    FREE_TEXTURE (tp);
    // Get pointer to next texture node in list
    ttp = tp->next;
    // Free memory for this texture node
    free (tp);
    tp = ttp;
  }

  // Reset globals
  texture_list = NULL;
  for (i=0; i<gx3d_NUM_TEXTURE_STAGES; i++) {
    SET_TEXTURE_TO_NONE (i);
    active_texture[i] = NULL;
  }
}

/*____________________________________________________________________
|
| Function: Texture_Set_Active
|
| Output: Set the active drawing texture at stage 0-7.
|___________________________________________________________________*/

void Texture_Set_Active (int stage, Texture *texture)
{
  if (texture) {
    // Make sure this isn't already the active texture
    if (texture != active_texture[stage]) {
      switch (texture->type) {                                                                
        case TEXTURE_TYPE_SQUARE: 
        case TEXTURE_TYPE_VOLUME:
        case TEXTURE_TYPE_CUBEMAP:                                                          
          if (gx_Video.set_texture)                                                         
            (*gx_Video.set_texture) (stage, texture->driver_data_static_texture);           
          break;                                                                            
        case TEXTURE_TYPE_DYNAMIC_SQUARE:                                                   
        case TEXTURE_TYPE_DYNAMIC_CUBEMAP:                                                  
          if (gx_Video.set_dynamic_texture)                                                 
            (*gx_Video.set_dynamic_texture) (stage, texture->driver_data_dynamic_texture);  
          break;                                                                            
      }                                                                                     
      active_texture[stage] = texture;
    }
  }
  else
    active_texture[stage] = 0;
}

/*____________________________________________________________________
|
| Function: Texture_Get_Active
|
| Output: Get the active drawing texture at stage 0-7.
|___________________________________________________________________*/

Texture *Texture_Get_Active (int stage)
{
  return (active_texture[stage]);
}

/*____________________________________________________________________
|
| Function: Texture_Get_Associated_Filenames 
|
| Output: Returns pointers to the image and alpha filenames associated 
|   with a texture, if any.  If none, returns NULL.
|___________________________________________________________________*/

void Texture_Get_Associated_Filenames (
  Texture  *texture, 
  char    **texture_image_file,
  char    **texture_alpha_file )
{
  int found;
  Texture *tp;

  // Init return values to not found
  *texture_image_file = NULL;
  *texture_alpha_file = NULL;

  // Look for this texture in the texture list
  for (tp=texture_list, found=FALSE; tp AND (NOT found); tp=tp->next) 
    if (tp == texture) {
      *texture_image_file = tp->image_filename;
      *texture_alpha_file = tp->alpha_filename;
      found = TRUE;
    }
}

/*____________________________________________________________________
|
| Function: Texture_Preload 
|
| Output: Preloads a texture into vram. Only works with static textures.
|___________________________________________________________________*/

void Texture_Preload (Texture *texture)
{
  if (texture) 
    // Texture type must be static
    if ((texture->type == TEXTURE_TYPE_SQUARE) OR
        (texture->type == TEXTURE_TYPE_VOLUME) OR 
        (texture->type == TEXTURE_TYPE_CUBEMAP))
      if (gx_Video.preload_texture)
        (*gx_Video.preload_texture) (texture->driver_data_static_texture);
}

/*____________________________________________________________________
|
| Function: Texture_Begin_Modify
|
| Output: Allows caller to modify/render to a texture. Only works with
|   dynamic textures.  
|___________________________________________________________________*/

void Texture_Begin_Modify (Texture *texture, int face)
{
  // Texture type must be dynamic
  if ((texture->type == TEXTURE_TYPE_DYNAMIC_SQUARE) OR (texture->type == TEXTURE_TYPE_DYNAMIC_CUBEMAP)) 
    if (gx_Video.enable_render_to_texture) 
      // Enable this texture as the render target
      (*gx_Video.enable_render_to_texture) (texture->driver_data_dynamic_texture, face);
}

/*____________________________________________________________________
|
| Function: Texture_Begin_Modify
|
| Output: Ends modify/render to texture.
|___________________________________________________________________*/

void Texture_End_Modify ()
{
  if (gx_Video.enable_render_to_texture)
    // Disable texture rendering and switch render target back to screen
    (*gx_Video.enable_render_to_texture) (0, 0);
}
