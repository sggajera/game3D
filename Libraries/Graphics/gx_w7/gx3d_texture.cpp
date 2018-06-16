/*____________________________________________________________________
|
| File: gx3d_texture.cpp
|
| Description: Functions for 3D textures.
|
| Functions:  gx3d_SetTextureDirectory
|             gx3d_InitTexture_File
|              Refactor_Pathname
|              Get_Next_Lower_Mipmap_File
|             gx3d_InitTexture_Image
|             gx3d_InitTexture_Sprite
|             gx3d_InitTexture_Image_Bitmap
|             gx3d_InitTexture_Image_Bytemap
|             gx3d_InitTexture_Bytemap
|              Mip_Data_Valid
|             gx3d_InitTexture_File_Volume
|             gx3d_InitTexture_File_Cubemap
|             gx3d_InitRenderTexture
|             gx3d_InitRenderTexture_Cubemap
|             gx3d_FreeTexture
|             gx3d_FreeAllTextures
|             gx3d_SetTexture
|             gx3d_GetTexture
|             gx3d_GetTextureAllocationSize
|             gx3d_SetTextureAddressingMode
|             gx3d_SetTextureBorderColor
|             gx3d_SetTextureFiltering
|             gx3d_SetTextureCoordinates
|             gx3d_SetCameraReflectionTextureCoordinates
|             gx3d_SetTextureWrapping
|             gx3d_SetTextureFactor
|             gx3d_PreLoadTexture
|             gx3d_EvictAllTextures
|             gx3d_SetTextureColorOp
|             gx3d_SetTextureAlphaOp
|             gx3d_SetTextureColorFactor
|             gx3d_EnableCubemapTexturing
|             gx3d_DisableCubemapTexturing
|             gx3d_BeginModifyTexture 
|             gx3d_EndModifyTexture 
|  
| Description: Texture dimensions should always be a power of 2 and 
|     must be square.  Max size may be limited to 256x256 depending
|     on the video hardware and is absolutely limited to 2048x2048.  
|     
|     When using multiple mip levels (more than 1), each mip level must
|     be half the width and height of the previous mip level.     
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <math.h>

#include "dp.h"
#include "img_clr.h"
#include "texture.h"

/*___________________
|
| Function Prototpyes
|__________________*/

static void Refactor_Pathname (char **pathname);
static void Get_Next_Lower_Mipmap_File (char *filename, char *buff);
static int  Mip_Data_Valid (int num_mip_levels, byte **pixmap1, byte **pixmap2);

/*___________________
|
| Constants
|__________________*/

#define MAX_MIPMAPS 15  // 1 more than max

/*____________________________________________________________________
|
| Function: gx3d_SetTextureDirectory
|
| Output: Sets directory to load textures from or NULL to clear the
|   saved texture directory.
|___________________________________________________________________*/

void gx3d_SetTextureDirectory (char *dir)
{
  if (dir == 0)
    gx3d_Texture_Directory[0] = 0;
  else
    strcpy(gx3d_Texture_Directory, dir);
}

/*____________________________________________________________________
|
| Function: gx3d_InitTexture_File
|
| Output: Init a texture from BMP file/s.  Returns a handle to the
|     texture or 0 on any error. Automatically generates mip levels
|     according to filename conventions.  Examples:
|
| fname_d128.bmp    // a filename with dimension=128, this function
|                   // will look for file fname_d64.bmp and so on
| fname_d128_fa.bmp // alpha format, will look for fname_d64_fa.bmp
|                   // and so on
|
|     If the filename does not indicate pre-generated mipmap files or
|     the pre-generated mipmap files don't exist, this function will
|     request the driver automatically create the mipmaps, down to 4x4
|     pixels.  The only disadvantage of this is that it can take more 
|     time than loading pre-generated mipmap files.
|___________________________________________________________________*/

gx3dTexture gx3d_InitTexture_File (char *filename, char *alpha_filename, unsigned flags)
{
  int i, j, n, dx, dy, bits, color_dx, color_dy, color_bits, alpha_dx, alpha_dy, alpha_bits, error;
  char str[256], str2[256], *color_files[MAX_MIPMAPS], *alpha_files[MAX_MIPMAPS];
  gx3dTexture texture = 0;
  
/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Clear mipmap files arrays
  memset (color_files, 0, MAX_MIPMAPS * sizeof(char *));
  memset (alpha_files, 0, MAX_MIPMAPS * sizeof(char *));

  alpha_bits  = 0;  // initialized only to avoid run time debug error

  error = FALSE;

/*____________________________________________________________________
|
| Use texture directory?
|___________________________________________________________________*/

  boolean refactored = false;
  // Use texture directory?
  if (gx3d_Texture_Directory[0] != 0) {
    if (filename)
      Refactor_Pathname(&filename);
    if (alpha_filename)
      Refactor_Pathname(&alpha_filename);
    refactored = true;
  }

/*____________________________________________________________________
|
| Validate input files
|___________________________________________________________________*/

  // Make sure filename is valid
  if (filename == NULL) {
    TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): param 1 (filenme) is NULL")
    error = TRUE;
  }

  if (NOT error) {
    // Make sure this is a BMP file
    if (gxGetBMPFileDimensions (filename, &color_dx, &color_dy, &color_bits) == 0) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): param 1 is not a BMP file")
      error = TRUE;
    }
    // Make sure image is square
    else if (color_dx != color_dy) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): param 1 BMP file doesn't have square dimensions")
      error = TRUE;
    }
  }

  if ((NOT error) AND alpha_filename) {
    // Make sure this is a BMP file
    if (gxGetBMPFileDimensions (alpha_filename, &alpha_dx, &alpha_dy, &alpha_bits) == 0) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): param 2 is not a BMP file")
      error = TRUE;
    }
    // Make sure image is square
    else if (alpha_dx != alpha_dy) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): param 2 BMP file doesn't have square dimensions")
      error = TRUE;
    }
    // Make sure this alpha file has same dimensions as image file
    else if (color_dx != alpha_dx) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): input files have different dimensions")
      error = TRUE;
    }
  }

/*____________________________________________________________________
|
| Create array of image filenames for all the mipmaps associated with 
| this texture file.
|___________________________________________________________________*/

  if (NOT error) {

    // Start with initial filename
    strcpy (str, filename);
    // Create an array of 1 or more filenames (mipmaps)
    for (i=0; (i<MAX_MIPMAPS) AND str[0] AND (NOT error); i++) {
      // Create an entry in color files array
      color_files[i] = (char *) calloc (strlen(str)+1, sizeof(char));
      if (color_files[i]) 
        // Copy a filename into the entry
        strcpy (color_files[i], str);
      // Get next lower mipmap, if any
      if (NOT (flags & gx3d_DONT_GENERATE_MIPMAPS)) {
        Get_Next_Lower_Mipmap_File (str, str2);
        strcpy (str, str2);
      }
      else
        str[0] = 0;
    }
  }

/*____________________________________________________________________
|
| Create array of alpha filenames for all the mipmaps associated with 
| this texture file.
|___________________________________________________________________*/

  if ((NOT error) AND alpha_filename) {

    // Start with initial filename
    strcpy (str, alpha_filename);
    // Create an array of 1 or more filenames (mipmaps)
    for (i=0; (i<MAX_MIPMAPS) AND str[0] AND (NOT error) AND color_files[i]; i++) {
      // Create an entry in color files array
      alpha_files[i] = (char *) calloc (strlen(str)+1, sizeof(char));
      if (alpha_files[i]) 
        // Copy a filename into the entry
        strcpy (alpha_files[i], str);
      // Get next lower mipmap, if any
      if (NOT (flags & gx3d_DONT_GENERATE_MIPMAPS)) {
        Get_Next_Lower_Mipmap_File (str, str2);
        strcpy (str, str2);
      }
      else
        str[0] = 0;
    }
  }

/*____________________________________________________________________
|
| Verify the mipmaps are all same bits per pixel, have square dimensions
| and are decreasing in size by a factor of 2
|___________________________________________________________________*/

  if (NOT error) {

    for (i=1; color_files[i] AND (NOT error); i++) {
      gxGetBMPFileDimensions (color_files[i], &dx, &dy, &bits);
      // Verify this mipmap is same bits per pixel as original
      if (bits != color_bits) {
        TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): color mipmap file not same bits per pixel")
        error = TRUE;
      }
      // Verify mipmap file has square dimensions
      else if (dx != dy) {
        TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): color mipmap file doesn't have square dimensions")
        error = TRUE;
      }
      // Verify mipmap file is power of 2 lower than previous
      else if (dx != (color_dx >> i)) {
        TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): color mipmap file not a power of 2 lower than previous file")
        error = TRUE;
      }
    }

    for (i=1; alpha_files[i] AND (NOT error); i++) {
      gxGetBMPFileDimensions (alpha_files[i], &dx, &dy, &bits);
      // Verify this mipmap is same bits per pixel as original
      if (bits != alpha_bits) {
        TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): alpha mipmap file not same bits per pixel")
        error = TRUE;
      }
      // Verify mipmap file has square dimensions
      else if (dx != dy) {
        TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): alpha mipmap file doesn't have square dimensions")
        error = TRUE;
      }
      // Verify mipmap file is power of 2 lower than previous
      else if (dx != (alpha_dx >> i)) {
        TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): alpha mipmap file not a power of 2 lower than previous file")
        error = TRUE;
      }
    }
  }

/*____________________________________________________________________
|
| Verify the same number of image and alpha mipmaps
|___________________________________________________________________*/

  for (i=0; color_files[i]; i++);
  for (j=0; alpha_files[j]; j++);
  if (j AND (i != j))
    error = TRUE;
  else
    n = i;  // # of mip levels to create

/*____________________________________________________________________
|
| If only 1 mip level and caller has not specified not to generate mipmaps
| then auto generate the mipmaps
|___________________________________________________________________*/

//  if ((n == 1) AND (NOT (flags & gx3d_DONT_GENERATE_MIPMAPS))) 
//    // Count # mip levels from toplevel down to 4x4
//    for (dx=color_dx; dx > 4; dx/=2, n++);
  
/*____________________________________________________________________
|
| Init the texture
|___________________________________________________________________*/

  if (NOT error)
    texture = (gx3dTexture) Texture_Add_File (n, color_files, alpha_files, color_dx, color_dy, color_bits, alpha_bits);

/*____________________________________________________________________
|
| Free memory allocated, if any
|___________________________________________________________________*/

  // Free any memory allocated for color/alpha files arrays
  for (i=0; i<MAX_MIPMAPS; i++) {
    if (color_files[i])
      free (color_files[i]);
    if (alpha_files[i])
      free (alpha_files[i]);
  }

  /*____________________________________________________________________
  |
  | Free memory for refactored pathnames?
  |___________________________________________________________________*/

  if (refactored) {
    if (filename)
      free(filename);
    if (alpha_filename)
      free(alpha_filename);
  }

  return (texture);
}

/*____________________________________________________________________
|
| Function: Refactor_Pathname
|
| Input: Called from ...
|
| Output: Takes pathname and strips off filename and concatenates it
|   with texture directory pathname, creating a new pathname.  Caller
|   must free memory for this newly created pathname when done with it.
|___________________________________________________________________*/

static void Refactor_Pathname(char **pathname)
{
  char *the_filename, *the_pathname;

  // Allocate memory for the filename
  the_filename = (char *)malloc(strlen(*pathname) + 1);
  if (the_filename == 0) {
    TERMINAL_ERROR("Error in Refactor_Pathname(): can't allocate memory for filename")
  }
  // Extract the filename (from the pathname)
  Extract_Filename(*pathname, the_filename);
  // Allocate memory for the entire pathname (directory + '/' + filename)
  the_pathname = (char *)malloc(strlen(gx3d_Texture_Directory) + strlen(the_filename) + 2);
  if (the_pathname == 0) {
    TERMINAL_ERROR("Error in Refactor_Pathname(): can't allocate memory for pathname")
  }
  // Build the new pathname
  strcpy(the_pathname, gx3d_Texture_Directory);
  strcat(the_pathname, "\\");
  strcat(the_pathname, the_filename);
  // Make filename pointer point to this new pathname
  *pathname = the_pathname;
  // Free memory
  free(the_filename);
}

/*____________________________________________________________________
|
| Function: Get_Next_Lower_Mipmap_File
|
| Input: Called from gx3d_InitTexture_File()
|
| Output: Looks for the next lower dimension mipmap file associated
|   with this filename and returns it in buff.  If no file by this
|   name exists, returns nullstring in buff.   
|___________________________________________________________________*/

static void Get_Next_Lower_Mipmap_File (char *filename, char *buff)
{
  int i;
  char *p, *p2;
    
  static char *dimension_substr [] = {
    "_d8192",
    "_d4096",
    "_d2048",
    "_d1024",
    "_d512",
    "_d256",
    "_d128",
    "_d64",
    "_d32",
    "_d16",
    "_d8",
    0
  };
    
  // Start with initial filename
  strcpy (buff, filename);
  // Try to generate a new mipmap filename, get start of filename portion of the pathname
  p = strrchr (buff, '\\');
  if (p)
    p++;
  else
    p = buff;
  // Look for dimension substring in filename
  p2 = NULL;
  for (i=0; dimension_substr[i]; i++) {
    p2 = strstr (p, dimension_substr[i]);
    if (p2)
      break;
  }
  // If found, replace this with next lower dimension
  if (p2 AND dimension_substr[i+1]) {
    // Remove old dimension substring
    strcpy (p2, p2+strlen(dimension_substr[i]));
    // Add new dimension substring
    strins (p2, 0, dimension_substr[i+1]);
    // Make sure this file exists
    if (NOT File_Exists (buff)) 
      strcpy (buff, "");
  }
  else
    strcpy (buff, "");
}

/*____________________________________________________________________
|
| Function: gx3d_InitTexture_Image
|
| Output: Init a texture from an image.  Returns a handle to the texture
|     or 0 on any error. 
|___________________________________________________________________*/

gx3dTexture gx3d_InitTexture_Image (
  int      num_mip_levels,          // 1 or more
  byte   **image,                   // array of 1 or more images
  gxColor *transparent_color,
  int      texture_bits_per_pixel ) // 16 or 24
{
  int i, j, dx, alphabits, size, ok;
  unsigned *p;
  gxColor image_color;
  byte **image_data, **alphamap;
  gx3dTexture texture = 0;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  image_data = NULL;
  alphamap   = NULL;

/*____________________________________________________________________
|
| Init the texture
|___________________________________________________________________*/

  // Verify the input data is correct
  if (Mip_Data_Valid (num_mip_levels, image, NULL)) {

    // Allocate memory for an array of pointers to image data
    image_data = (byte **) calloc (num_mip_levels, sizeof(byte *));
    if (image_data) {
      // Fill array with pointers to image data
      for (i=0; i<num_mip_levels; i++)
        image_data[i] = image[i] + (2 * sizeof(unsigned));
      // Get dimensions of first image
      p = (unsigned *) (image[0]);
      dx = *p;

      // Build alphamaps?
      ok = FALSE;
      if (transparent_color) {
        alphabits = 1;
        // Allocate memory for an array of alphamaps
        alphamap = (byte **) calloc (num_mip_levels, sizeof(byte *));
        if (alphamap) {
          for (i=0, size=dx*dx; i<num_mip_levels; i++, size/=4) {
            alphamap[i] = (byte *) malloc (size);
            if (NOT alphamap[i])
              break;
          }
          // Was memory for each alphamap created ok?
          if (i == (num_mip_levels+1)) {
            // Build each alphamap
            for (i=0, size=dx*dx; i<num_mip_levels; i++, size/=4) {
              // Set each alphamap pixel as appropriate
              for (j=0; j<size; j++) {
                image_color = Get_Image_Pixel_Color (image_data[i], j, 0, size);
                if (memcmp ((void *)&image_color, (void *)transparent_color, sizeof(gxColor)))
                  alphamap[i][j] = 0;
                else
                  alphamap[i][j] = 1;
              }
            }
            ok = TRUE;
          }
        }
      }
      else {
        alphabits = 0;
        alphamap = NULL;
        ok = TRUE;
      }

      // Init the texture  
      if (ok)
        texture = (gx3dTexture) Texture_Add_Data (num_mip_levels,
                                                  image_data, 
                                                  alphamap,
                                                  dx, dx,
                                                  texture_bits_per_pixel,
                                                  alphabits );
    }
  }

/*____________________________________________________________________
|
| Free allocated memory, if any
|___________________________________________________________________*/

  if (image_data)
    free (image_data);
  if (alphamap) 
    for (i=0; i<num_mip_levels; i++)
      if (alphamap[i])
        free (alphamap[i]);

  return (texture);
}

/*____________________________________________________________________
|
| Function: gx3d_InitTexture_Sprite
|
| Output: Init a texture w/ 1-bit alpha from a sprite.
|___________________________________________________________________*/

gx3dTexture gx3d_InitTexture_Sprite (
  int    num_mip_levels,          // 1 or more
  byte **sprite,                  // array of 1 or more sprites
  int    texture_bits_per_pixel ) // 16 or 24
{
  int i, x, y, dx, bytes_per_row, ok, size;
  unsigned *p;
  byte **sprite_data, **alphamap, *bitmap_data, *alphamap_data;
  byte bit_mask [8] = {
    0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1
  };
  gx3dTexture texture = 0;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  sprite_data = NULL;
  alphamap    = NULL;

/*____________________________________________________________________
|
| Init the texture
|___________________________________________________________________*/

  // Verify the input data is correct
  if (Mip_Data_Valid (num_mip_levels, sprite, NULL)) {

    // Allocate memory for an array of pointers to sprite data
    sprite_data = (byte **) calloc (num_mip_levels, sizeof(byte *));
    if (sprite_data) {
      // Fill array with pointers to sprite data
      for (i=0; i<num_mip_levels; i++)
        sprite_data[i] = sprite[i] + (2 * sizeof(unsigned));
      // Get dimensions of first sprite
      p = (unsigned *) (sprite[0]);
      dx = *p;

      // Build alphamaps
      ok = FALSE;
      // Allocate memory for alphamaps
      alphamap = (byte **) calloc (num_mip_levels, sizeof(byte *));
      if (alphamap) {
        for (i=0, size=dx*dx; i<num_mip_levels; i++, size/=4) {
          alphamap[i] = (byte *) malloc (size);
          if (NOT alphamap[i])
            break;
        }
        // Was memory for each alphamap created ok?
        if (i == (num_mip_levels+1)) {
          // Build each alphamap
          for (i=0, size=dx*dx; i<num_mip_levels; i++, size/=4) {
            // Set each alphamap pixel as appropriate
            bitmap_data = sprite_data[i] + (dx * dx * gx_Pixel_size);
            bytes_per_row = (dx+7)/8;
            alphamap_data = alphamap[i];
            for (y=0; y<dx; y++) {
              for (x=0; x<dx; x++) {
                if (bitmap_data[x/8] & bit_mask[x%8])
                  alphamap_data[x] = 0xFF;
                else
                  alphamap_data[x] = 0;
              }
              bitmap_data += bytes_per_row;
              alphamap_data += dx;
            }
          }
          ok = TRUE;
        }
      }

      // Init the texture  
      if (ok)
        texture = (gx3dTexture) Texture_Add_Data (num_mip_levels,
                                                  sprite_data, 
                                                  alphamap,
                                                  dx, dx,
                                                  texture_bits_per_pixel,
                                                  1 );
    }
  }

/*____________________________________________________________________
|
| Free allocated memory, if any
|___________________________________________________________________*/

  if (sprite_data)
    free (sprite_data);
  if (alphamap) 
    for (i=0; i<num_mip_levels; i++)
      if (alphamap[i])
        free (alphamap[i]);

  return (texture);
}

/*____________________________________________________________________
|
| Function: gx3d_InitTexture_Image_Bitmap
|
| Output: Init a texture w/ 1-bit alpha from image & bitmap.
|___________________________________________________________________*/

gx3dTexture gx3d_InitTexture_Image_Bitmap (
  int    num_mip_levels,          // 1 or more
  byte **image,                   // array of 1 or more images
  byte **bitmap,                  // array of 1 or more bitmaps
  int    texture_bits_per_pixel ) // 16 or 24
{
  int i, x, y, dx, bytes_per_row, ok, size, width;
  unsigned *p;
  byte **image_data, **alphamap, *bitmap_data, *alphamap_data;
  byte bit_mask [8] = {
    0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1
  };
  gx3dTexture texture = 0;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  image_data = NULL;
  alphamap   = NULL;

/*____________________________________________________________________
|
| Init the texture
|___________________________________________________________________*/

  // Verify the input data is correct
  if (Mip_Data_Valid (num_mip_levels, image, bitmap)) {

    // Allocate memory for an array of pointers to image data
    image_data = (byte **) calloc (num_mip_levels, sizeof(byte *));
    if (image_data) {
      // Fill array with pointers to image data
      for (i=0; i<num_mip_levels; i++)
        image_data[i] = image[i] + (2 * sizeof(unsigned));
      // Get dimensions of first image
      p = (unsigned *) (image[0]);
      dx = *p;

      // Build alphamaps
      ok = FALSE;
      // Allocate memory for alphamaps
      alphamap = (byte **) calloc (num_mip_levels, sizeof(byte *));
      if (alphamap) {
        for (i=0, size=dx*dx; i<num_mip_levels; i++, size/=4) {
          alphamap[i] = (byte *) malloc (size);
          if (NOT alphamap[i])
            break;
        }
        // Was memory for each alphamap created ok?
        if (i == num_mip_levels) {
          // Build each alphamap
          width = dx;
          for (i=0, size=dx*dx; i<num_mip_levels; i++, size/=4) {
            // Set each alphamap pixel as appropriate
            bitmap_data = bitmap[i] + (2*sizeof(unsigned));
            bytes_per_row = (width+7)/8;
            alphamap_data = alphamap[i];
            for (y=0; y<width; y++) {
              for (x=0; x<width; x++) {
                if (bitmap_data[x/8] & bit_mask[x%8])
                  alphamap_data[x] = 0xFF;
                else
                  alphamap_data[x] = 0;
              }
              bitmap_data += bytes_per_row;
              alphamap_data += width;
            }
            width /= 2;
          }
          ok = TRUE;
        }
      }

      // Init the texture  
      if (ok)
        texture = (gx3dTexture) Texture_Add_Data (num_mip_levels,
                                                  image_data, 
                                                  alphamap,
                                                  dx, dx,
                                                  texture_bits_per_pixel,
                                                  1 );
    }
  }

/*____________________________________________________________________
|
| Free allocated memory, if any
|___________________________________________________________________*/

  if (image_data)
    free (image_data);
  if (alphamap) 
    for (i=0; i<num_mip_levels; i++)
      if (alphamap[i])
        free (alphamap[i]);

  return (texture);
}

/*____________________________________________________________________
|
| Function: gx3d_InitTexture_Image_Bytemap
|
| Output: Init a texture w/ alpha (8 bits max) from an image & bytemap.
|___________________________________________________________________*/

gx3dTexture gx3d_InitTexture_Image_Bytemap (
  int    num_mip_levels,          // 1 or more
  byte **image,                   // array of 1 or more images
  byte **bytemap,                 // array of 1 or more bytemaps
  int    texture_bits_per_pixel ) // 16 or 24
{
  int i, dx;
  unsigned *p;
  byte **image_data, **bytemap_data;
  gx3dTexture texture = 0;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  image_data   = NULL;
  bytemap_data = NULL;

/*____________________________________________________________________
|
| Init the texture
|___________________________________________________________________*/

  // Verify the input data is correct
  if (Mip_Data_Valid (num_mip_levels, image, bytemap)) {

    // Allocate memory for an array of pointers to image data
    image_data = (byte **) calloc (num_mip_levels, sizeof(byte *));
    if (image_data) {
      // Fill array with pointers to image data
      for (i=0; i<num_mip_levels; i++)
        image_data[i] = image[i] + (2 * sizeof(unsigned));
      // Get dimensions of first image
      p = (unsigned *) (image[0]);
      dx = *p;

      // Allocate memory for an array of pointers to bytemap data
      bytemap_data = (byte **) calloc (num_mip_levels, sizeof(byte *));
      if (bytemap_data) {
        // Fill array with pointers to bytemap data
        for (i=0; i<num_mip_levels; i++)
          bytemap_data[i] = bytemap[i] + (2 * sizeof(unsigned));

        // Init the texture  
        texture = (gx3dTexture) Texture_Add_Data (num_mip_levels,
                                                  image_data, 
                                                  bytemap_data,
                                                  dx, dx,
                                                  texture_bits_per_pixel,
                                                  8 );
      }
    }
  }

/*____________________________________________________________________
|
| Free allocated memory, if any
|___________________________________________________________________*/

  if (image_data)
    free (image_data);
  if (bytemap_data)
    free (bytemap_data);

  return (texture);
}

/*____________________________________________________________________
|
| Function: gx3d_InitTexture_Bytemap
|
| Output: Init a bytemap texture.
|___________________________________________________________________*/

gx3dTexture gx3d_InitTexture_Bytemap (
  int    num_mip_levels,  // 1 or more
  byte **bytemap )        // array of 1 or more bytemaps
{
  int i, dx;
  unsigned *p;
  byte **bytemap_data;
  gx3dTexture texture = 0;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  bytemap_data = NULL;

/*____________________________________________________________________
|
| Init the texture
|___________________________________________________________________*/

  // Verify the input data is correct
  if (Mip_Data_Valid (num_mip_levels, NULL, bytemap)) {

    // Allocate memory for an array of pointers to bytemap data
    bytemap_data = (byte **) calloc (num_mip_levels, sizeof(byte *));
    if (bytemap_data) {
      // Fill array with pointers to bytemap data
      for (i=0; i<num_mip_levels; i++)
        bytemap_data[i] = bytemap[i] + (2 * sizeof(unsigned));
      // Get dimensions of first image
      p = (unsigned *) (bytemap[0]);
      dx = *p;

      // Init the texture  
      texture = (gx3dTexture) Texture_Add_Data (num_mip_levels,
                                                NULL, 
                                                bytemap_data,
                                                dx, dx,
                                                0,
                                                8 );
    }
  }

/*____________________________________________________________________
|
| Free allocated memory, if any
|___________________________________________________________________*/

  if (bytemap_data)
    free (bytemap_data);

  return (texture);
}

/*____________________________________________________________________
|
| Function: Mip_Data_Valid
|
| Input: Called from gx3d_InitTexture_Image(),
|                    gx3d_InitTexture_Sprite(),
|                    gx3d_InitTexture_Image_Bitmap(),
|                    gx3d_InitTexture_Image_Bytemap(),
|                    gx3d_InitTexture_Bytemap()
|
| Output: Returns true if input pixmaps are in correct mipmap format.
|
| Description: Input pixmap/s are in correct mipmap format if all 
|     mipmaps are a power of 2, max size is less than or equal to 
|     2048x2048, dimensions are square and each lower level mipmap
|     is half the width and height of the previous mipmap.
|
|     Also, if 2 pixmaps are being checked, each set must have the same
|     dimensions.
|___________________________________________________________________*/

static int Mip_Data_Valid (int num_mip_levels, byte **pixmap1, byte **pixmap2)
{
  int k, i, j, dx, dy, valid_size, last_dx, valid, max_dx_pixmap[2];
  unsigned *p;
  byte **pixmap;
  char buff[160];
  static int valid_texture_size [] = {
    1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
    0 // last entry in this table must be zero
  };

  // Assume valid 
  valid = TRUE;
  
  // Verify each set of mipmaps are valid
  for (i=0; i<2; i++) {
    if (i == 0)
      pixmap = pixmap1;
    else
      pixmap = pixmap2;
    if (pixmap) {

      last_dx = 0;
      for (j=0; (j<num_mip_levels) AND valid; j++) {
        // Get dimensions of pixmap
        p = (unsigned *) pixmap[j];
        dx = *p;
        dy = *(p+1);

        // Make sure this pixmap is square
        if (dx == dy) {
          // Make sure the dimensions are a power of 2 (and are <= 2048)
          valid_size = FALSE;
          for (k=0; valid_texture_size[k]; k++) 
            if (dx == valid_texture_size[k]) {
              valid_size = TRUE;
              break;
            }
          if (valid_size) {
            // Make sure each lower mip level is half the width and height of the previous level
            valid_size = FALSE;
            if (last_dx == 0)
              valid_size = TRUE;
            else if ((dx*2) == last_dx)
              valid_size = TRUE;

            if (NOT valid_size) {
              valid = FALSE;
              sprintf (buff, "Mip_Data_Valid(): error, image %d not half the dimensions of the previous mipmap level", j);
              TERMINAL_ERROR (buff)
            }
          }
          else {
            valid = FALSE;
            sprintf (buff, "Mip_Data_Valid(): error, image %d dimensions not a power of 2", j);
            TERMINAL_ERROR (buff)
          }
        }    
        else {
          valid = FALSE;
          sprintf (buff, "Mip_Data_Valid(): error, image %d is not square", j);
          TERMINAL_ERROR (buff)
        }
        // Save largest mipmap level
        if (last_dx == 0)
          max_dx_pixmap[i] = dx;

        last_dx = dx;
      }
    }
  }

  // If two mipmaps, verify each have same dimensions
  if (valid AND pixmap1 AND pixmap2)
    if (max_dx_pixmap[0] != max_dx_pixmap[1]) {
      valid = FALSE;
      TERMINAL_ERROR ("Mip_Data_Valid(): error, 2 mipmaps not the same dimensions")
    }

  return (valid);
}

/*____________________________________________________________________
|
| Function: gx3d_InitTexture_File_Volume
|
| Output: Init a volume texture from BMP file/s.  Returns a handle to 
|   the texture or 0 on any error.
|
| Description: A volume texture is created from a series of BMP files.
|   All BMP files must have the same dimensions.  The first BMP file
|   becomes the top of the volume chain.  The second BMP file becomes the
|   second texture in the volume chain and is addressed with w=1 and so on. 
|   Each array of filenames must end with a 0.
|
|   Mipmaps are supported and if used all textures in the volume chain
|   must have the same number of mipmaps.
|
|   For best results the number of slices in the volume should be a power
|   of two.  If not then some texture memory will be wasted.
|
|   The inputs are null-terminated arrays of pointers to null-terminated
|   arrays of filenames.  Each set of filenames represents all the slices
|   for a mip level.
|
| Note: Doesn't work with currently set texture directory.  Make sure
|   filenames and alpha_filenames are valid.
|___________________________________________________________________*/

gx3dTexture gx3d_InitTexture_File_Volume (char ***filenames, char ***alpha_filenames)
{
  int i, j, n, dx, dy, bits, color_dx, color_dy, color_bits, alpha_dx, alpha_dy, alpha_bits, error;
  int num_filenames, num_slices, num_miplevels;
  char **color_files, **alpha_files;
  gx3dTexture texture = 0;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Clear file arrays
  color_files = 0;
  alpha_files = 0;
  alpha_bits  = 0;  // initialized only to avoid run time debug error

  error = FALSE;                            

/*____________________________________________________________________
|
| Validate input files
|___________________________________________________________________*/

  // Make sure filename array exists
  if (filenames == NULL) {
    TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Volume(): param 1 (filenames) is NULL")
    error = TRUE;
  }
  // Make sure first set of filenames exist
  else if (filenames[0] == 0) {
    TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Volume(): first set of filenames is NULL")
    error = TRUE;
  }
  // Make sure if alpha_filenames the array has at least one array of filenames
  else if (alpha_filenames) {
    if (alpha_filenames[0] == NULL) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Volume(): no alpha filenames")
      error = TRUE;
    }
  }

  if (NOT error) {
    // Count the number of slices (filenames at top mip level)
    for (num_slices=0; filenames[0][num_slices]; num_slices++);
    // Count the number of mip levels
    for (num_miplevels=0; filenames[num_miplevels]; num_miplevels++);
    
    // Make sure first image file is a BMP file and get the dimensions of the image
    if (gxGetBMPFileDimensions (filenames[0][0], &color_dx, &color_dy, &color_bits) == 0) {
      gxError_Filename (filenames[0][0]);
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): first image file is not a BMP file")
      error = TRUE;
    }
    if (NOT error)
      if (alpha_filenames)
        // Make sure first alpha file is a BMP file and get the dimensions of the image
        if (gxGetBMPFileDimensions (alpha_filenames[0][0], &alpha_dx, &alpha_dy, &alpha_bits) == 0) {
          gxError_Filename (alpha_filenames[0][0]);
          TERMINAL_ERROR ("Error in gx3d_InitTexture_File(): first alpha file is not a BMP file")
          error = TRUE;
        }
    if (NOT error) {
      // Make sure dimensions are square
      if (color_dx != color_dy) {
        gxError_Filename (alpha_filenames[0][0]);
        TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Volume(): first BMP file doesn't have square dimensions")
        error = TRUE;
      }
    }
  }

  if (NOT error) {
    // Make sure each image file is a BMP file and has the correct dimensions
    for (i=0; (i<num_miplevels) AND (NOT error); i++)
      for (j=0; (j<(num_slices>>i)) AND (NOT error); j++) {
        if (gxGetBMPFileDimensions (filenames[i][j], &dx, &dy, &bits) == 0) {
          gxError_Filename (filenames[i][j]);
          TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Volume(): image file is not a BMP file")
          error = TRUE;
        }
        else if ((dx != (color_dx>>i)) OR (dy != (color_dy>>i))) {
          gxError_Filename (filenames[i][j]);
          TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Volume(): image file dimensions are bad: %s")
          error = TRUE;
        }
        else if (bits != color_bits) {
          gxError_Filename (filenames[i][j]);
          TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Volume(): image file bitdepth is bad: %s")
          error = TRUE;
        }
      }
    
    // Make sure each alpha file is a BMP file and has the correct dimensions
    if (alpha_filenames)
      for (i=0; (i<num_miplevels) AND (NOT error); i++)
        for (j=0; (j<(num_slices>>i)) AND (NOT error); j++) {
          if (gxGetBMPFileDimensions (alpha_filenames[i][j], &dx, &dy, &bits) == 0) {
            gxError_Filename (alpha_filenames[i][j]);
            TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Volume(): alpha file is not a BMP file")
            error = TRUE;
          }
          else if ((dx != (color_dx>>i)) OR (dy != (color_dy>>i))) {
            gxError_Filename (alpha_filenames[i][j]);
            TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Volume(): alpha file dimensions are bad: %s")
            error = TRUE;
          }
          else if (bits != alpha_bits) {
            gxError_Filename (alpha_filenames[i][j]);
            TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Volume(): alpha file bitdepth is bad: %s")
            error = TRUE;
          }
        }
  }  

/*____________________________________________________________________
|
| Create array of image filenames for all the files associated with 
| this texture.
|___________________________________________________________________*/

  if (NOT error) {

    // Compute # of filenames generated
    for (num_filenames=0,i=0; i<num_miplevels; i++)
      num_filenames += (num_slices>>i);

    // Allocate memory for array of filenames
    color_files = (char **) calloc (num_filenames, sizeof(char *));

    n = 0;
    for (i=0; (i<num_miplevels) AND (NOT error); i++) {
      for (j=0; (j<(num_slices>>i)) AND (NOT error); j++) {
        // Create an entry in color files array
        color_files[n] = (char *) calloc (strlen(filenames[i][j])+1, sizeof(char));
        if (color_files[n]) {
          // Copy a filename into the entry
          strcpy (color_files[n], filenames[i][j]);
          n++;
        }
      }
    }
  }

/*____________________________________________________________________
|
| Create array of alpha filenames for all the mipmaps associated with 
| this texture.
|___________________________________________________________________*/

  if (NOT error) {

    // Compute # of filenames generated (should be the same as above)
    for (num_filenames=0,i=0; i<num_miplevels; i++)
      num_filenames += (num_slices>>i);

    // Allocate memory for array of filenames
    alpha_files = (char **) calloc (num_filenames, sizeof(char *));

    n = 0;
    for (i=0; (i<num_miplevels) AND (NOT error); i++) {
      for (j=0; (j<(num_slices>>i)) AND (NOT error); j++) {
        // Create an entry in color files array
        alpha_files[n] = (char *) calloc (strlen(filenames[i][j])+1, sizeof(char));
        if (alpha_files[n]) {
          // Copy a filename into the entry
          strcpy (alpha_files[n], filenames[i][j]);
          n++;
        }
      }
    }
  }

/*____________________________________________________________________
|
| Init the texture
|___________________________________________________________________*/

  if (NOT error) 
    texture = (gx3dTexture) Texture_Add_File_Volume (num_miplevels, num_slices, color_files, alpha_files, color_dx, color_dy, color_bits, alpha_bits);

/*____________________________________________________________________
|
| Free memory allocated, if any
|___________________________________________________________________*/

  if (color_files) {
    // Compute # of filenames generated
    for (num_filenames=0,i=0; i<num_miplevels; i++)
      num_filenames += (num_slices>>i);
    // Free the memory for this array of filenames
    for (i=0; i<num_filenames; i++) 
      if (color_files[i])
        free (color_files[i]);
    free (color_files);
  }
  if (alpha_files) {
    // Compute # of filenames generated (should be the same as above)
    for (num_filenames=0,i=0; i<num_miplevels; i++)
      num_filenames += (num_slices>>i);
    // Free the memory for this array of filenames
    for (i=0; i<num_filenames; i++) 
      if (alpha_files[i])
        free (alpha_files[i]);
    free (alpha_files);
  }

  return (texture);
}

/*____________________________________________________________________
|
| Function: gx3d_InitTexture_File_Cubemap
|
| Output: Init a texture from a cubemap stored in a BMP file.  Returns 
|   a handle to the texture or 0 on any error. 
|
| Description: Texture dimensions should always be a power of 2 and 
|     must be square.  Max size may be limited to 256x256 depending
|     on the video hardware and is absolutely limited to 2048x2048.  
|
|     Cubemap textures should be stored as 6 square images in a row
|     in this order:  (the image size will be 6x wider than the height)
|        _ _ _ _ _ _ _ 
|       |0|1|2|3|4|5|6| 
|        - - - - - - -  
|
|     where:  0 = right
|             1 = left
|             2 = top
|             3 = bottom
|             4 = front
|             5 = back
|
|   Currently, mipmaps are not supported for cube textures although this
|   can be done.
|___________________________________________________________________*/

gx3dTexture gx3d_InitTexture_File_Cubemap (char *filename, char *alpha_filename)
{
  int color_dx, color_dy, color_bits, alpha_dx, alpha_dy, alpha_bits, error;
  gx3dTexture texture = 0;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  alpha_bits  = 0;  // initialized only to avoid run time debug error
  error = FALSE;

/*____________________________________________________________________
|
| Use texture directory?
|___________________________________________________________________*/

  boolean refactored = false;
  // Use texture directory?
  if (gx3d_Texture_Directory[0] != 0) {
    if (filename)
      Refactor_Pathname(&filename);
    if (alpha_filename)
      Refactor_Pathname(&alpha_filename);
    refactored = true;
  }

/*____________________________________________________________________
|
| Validate input files
|___________________________________________________________________*/

  // Make sure filename is valid
  if (filename == NULL) {
    TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Cubemap(): param 1 (filenme) is NULL")
    error = TRUE;
  }

  if (NOT error) {
    // Make sure this is a BMP file
    if (gxGetBMPFileDimensions (filename, &color_dx, &color_dy, &color_bits) == 0) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Cubemap(): param 1 is not a BMP file")
      error = TRUE;
    }
    // Make sure 6 images are square
    else if ((color_dx/6) != color_dy) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Cubemap(): BMP file doesn't have square dimensions")
      error = TRUE;
    }
  }

  if ((NOT error) AND alpha_filename) {
    // Make sure this is a BMP file
    if (gxGetBMPFileDimensions (alpha_filename, &alpha_dx, &alpha_dy, &alpha_bits) == 0) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Cubemap(): param 2 is not a BMP file")
      error = TRUE;
    }
    // Make sure 6 images are square
    else if ((alpha_dx/6) != alpha_dy) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Cubemap(): param 2 BMP file doesn't have square dimensions")
      error = TRUE;
    }
    // Make sure this alpha file has same dimensions as image file
    else if ((color_dx != alpha_dx) OR (color_dy != alpha_dy)) {
      TERMINAL_ERROR ("Error in gx3d_InitTexture_File_Cubemap(): input files have different dimensions")
      error = TRUE;
    }
  }

/*____________________________________________________________________
|
| Init the texture
|___________________________________________________________________*/

  if (NOT error)
    texture = (gx3dTexture) Texture_Add_File_Cubemap (filename, alpha_filename, color_dy, color_bits, alpha_bits);
  
/*____________________________________________________________________
|
| Free memory for refactored pathnames?
|___________________________________________________________________*/

  if (refactored) {
    if (filename)
      free(filename);
    if (alpha_filename)
      free(alpha_filename);
  }

  return (texture);
}

/*____________________________________________________________________
|
| Function: gx3d_InitRenderTexture
|
| Output: Init a renderable empty square texture.  Returns a handle
|   to the texture or 0 on any error. 
|
| Description: Texture dimensions should always be a power of 2 and 
|     must be square.  Max size may be limited to 256x256 depending
|     on the video hardware and is absolutely limited to 2048x2048.  
|___________________________________________________________________*/

gx3dTexture gx3d_InitRenderTexture (int dimensions) 
{
  int i, valid_size;
  static int valid_texture_size [] = {
    1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
    0 // last entry in this table must be zero
  };
  gx3dTexture texture = 0;

  // Make sure dimenion size is valid - must be a power of 2
  valid_size = FALSE;
  for (i=0; valid_texture_size[i]; i++) 
    if (dimensions == valid_texture_size[i]) {
      valid_size = TRUE;
      break;
    }

  // Init the texture?
  if (NOT valid_size)
    TERMINAL_ERROR ("Error in gx3d_InitRenderTexture(): param 1 (dimensions) is not a power of 2")
  else 
    // Create an empty renderable square texture
    texture = (gx3dTexture) Texture_Add_Data (1, NULL, NULL, dimensions, dimensions, 0, 0);

  return (texture);
}

/*____________________________________________________________________
|
| Function: gx3d_InitRenderTexture_Cubemap
|
| Output: Init a renderable empty cubemap texture.  Returns a handle
|   to the texture or 0 on any error. 
|
| Description: Texture dimensions should always be a power of 2 and 
|     must be square.  Max size may be limited to 256x256 depending
|     on the video hardware and is absolutely limited to 2048x2048.  
|___________________________________________________________________*/

gx3dTexture gx3d_InitRenderTexture_Cubemap (int dimensions) 
{
  int i, valid_size;
  static int valid_texture_size [] = {
    1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
    0 // last entry in this table must be zero
  };
  gx3dTexture texture = 0;

  // Make sure dimenion size is valid - must be a power of 2
  valid_size = FALSE;
  for (i=0; valid_texture_size[i]; i++) 
    if (dimensions == valid_texture_size[i]) {
      valid_size = TRUE;
      break;
    }

  // Init the texture?
  if (NOT valid_size)
    TERMINAL_ERROR ("Error in gx3d_InitRenderTexture_Cubemap(): param 1 (dimensions) is not a power of 2")
  else 
    // Create an empty renderable cubemap texture
    texture = (gx3dTexture) Texture_Add_Data_Cubemap (NULL, NULL, dimensions, 0, 0);

  return (texture);
}

/*____________________________________________________________________
|
| Function: gx3d_FreeTexture
|
| Output: Frees a 3D texture created by gx3d_InitTexture().
|___________________________________________________________________*/

inline void gx3d_FreeTexture (gx3dTexture texture)
{
  Texture_Release ((Texture *)texture);
}

/*____________________________________________________________________
|
| Function: gx3d_FreeAllTextures
|
| Output: Frees all 3D textures.
|___________________________________________________________________*/

inline void gx3d_FreeAllTextures (void)
{
  Texture_Release_All ();
}

/*____________________________________________________________________
|
| Function: gx3d_SetTexture
|
| Output: Sets the current render texture for a stage.
|___________________________________________________________________*/

inline void gx3d_SetTexture (int stage, gx3dTexture texture)
{
  Texture_Set_Active (stage, (Texture *)texture);
}

/*____________________________________________________________________
|
| Function: gx3d_GetTexture
|
| Output: Gets the current render texture for a stage.
|___________________________________________________________________*/

inline gx3dTexture gx3d_GetTexture (int stage)
{
  return ((gx3dTexture)(Texture_Get_Active (stage)));
}

/*____________________________________________________________________
|
| Function: gx3d_GetTextureAllocationSize
|
| Output: Returns combined size of all currently loaded textures.
|___________________________________________________________________*/
 
inline unsigned gx3d_GetTextureAllocationSize (void)
{
  return (Texture_Get_Allocation_Size ());
}

/*____________________________________________________________________
|
| Function: gx3d_SetTextureAddressingMode
|
| Output: Sets addressing mode for the UV dimensions of a texture stage 
|   (0-gx3d_NUM_TEXTURE_STAGES-1).
|   Addressing mode can be one of these types:
|
|     gx3d_TEXTURE_ADDRESSMODE_WRAP   
|     gx3d_TEXTURE_ADDRESSMODE_MIRROR 
|     gx3d_TEXTURE_ADDRESSMODE_CLAMP  
|     gx3d_TEXTURE_ADDRESSMODE_BORDER 
|
|  Dimensions can be one or both of these flags:
|
|     gx3d_TEXTURE_DIMENSION_U
|     gx3d_TEXTURE_DIMENSION_V
|     gx3d_TEXTURE_DIMENSION_W
|___________________________________________________________________*/

inline void gx3d_SetTextureAddressingMode (int stage, int dimension, int addressing_mode)
{
  if (gx_Video.set_texture_addressing_mode)
    (*gx_Video.set_texture_addressing_mode) (stage, dimension, addressing_mode);
}

/*____________________________________________________________________
|
| Function: gx3d_SetTextureBorderColor
|
| Output: Sets border color for a texture stage (0-gx3d_NUM_TEXTURE_STAGES-1).
|___________________________________________________________________*/

inline void gx3d_SetTextureBorderColor (int stage, gxColor color)
{
  if (gx_Video.set_texture_border_color)
    (*gx_Video.set_texture_border_color) (stage, color.r, color.g, color.b, color.a);
}

/*____________________________________________________________________
|
| Function: gx3d_SetTextureFiltering
|
| Output: Sets border color for a texture stage (0-gx3d_NUM_TEXTURE_STAGES-1).  
|   Filtering can be one of these types:
|
|     gx3d_TEXTURE_FILTERTYPE_POINT       
|     gx3d_TEXTURE_FILTERTYPE_LINEAR      
|     gx3d_TEXTURE_FILTERTYPE_TRILINEAR      
|     gx3d_TEXTURE_FILTERTYPE_ANISOTROPIC 
|
| Description: If anisotropic filtering is supported, the anisotropy_level
|   defines the amount of filtering desired from 1 (lowest) to 100 (highest).
|   A low value will render more quickly.  A high value will produce the best
|   quality.
|___________________________________________________________________*/

inline void gx3d_SetTextureFiltering (
  int stage, 
  int filter_type, 
  int anisotropy_level )  // amount of filtering (1-100), 1=min, 100=max
{
  if (gx_Video.set_texture_filtering)
    (*gx_Video.set_texture_filtering) (stage, filter_type, anisotropy_level);
}

/*____________________________________________________________________
|
| Function: gx3d_SetTextureCoordinates
|
| Output: Sets the set of texture coordinates of an object to use for 
|     this texture stage.
|___________________________________________________________________*/

inline void gx3d_SetTextureCoordinates (
  int             stage,            // texture blending stage
  gx3dTexCoordSet coordinate_set )  // set of texture coordinates in the object or gx3d_TEXCOORD_CUBEMAP
{
  if (gx_Video.set_texture_coordinates)
    (*gx_Video.set_texture_coordinates) (stage, (int)coordinate_set);
}
                          
/*____________________________________________________________________
|
| Function: gx3d_SetTextureWrapping
|
| Output: Sets texture wrapping for a set of texture coordinates in an
|   object.
|___________________________________________________________________*/

inline void gx3d_SetTextureWrapping (
  int coordinate_stage, // set of texture coordinates in the object
  int wrap_s,           // boolean (u dimension)
  int wrap_t,           // boolean (v dimension)
  int wrap_r,           // boolean
  int wrap_q )          // boolean
{
  if (gx_Video.set_texture_wrapping)
    (*gx_Video.set_texture_wrapping) (coordinate_stage, wrap_s, wrap_t, wrap_r, wrap_q);
}

/*____________________________________________________________________
|
| Function: gx3d_SetTextureWrapping
|
| Output: Sets the texture factor (a color with an alpha part) used by
|   some texture blending operations.
|___________________________________________________________________*/

inline void gx3d_SetTextureFactor (byte r, byte g, byte b, byte a)
{
  if (gx_Video.set_texture_factor)
    (*gx_Video.set_texture_factor) (r, g, b, a);
}

/*____________________________________________________________________
|
| Function: gx3d_PreLoadTexture
|
| Output: Preloads a texture into vram.
|___________________________________________________________________*/

inline void gx3d_PreLoadTexture (gx3dTexture texture)
{
  Texture_Preload ((Texture *)texture);
}

/*____________________________________________________________________
|
| Function: gx3d_EvictAllTextures
|
| Output: Evicts all textures from texture video memory.
|___________________________________________________________________*/

inline void gx3d_EvictAllTextures (void)
{
  if (gx_Video.evict_all_textures)
    (*gx_Video.evict_all_textures) ();
}

/*____________________________________________________________________
|
| Function: gx3d_SetTextureColorOp
|
| Output: Sets the texture blending color operation:
|     default for stage 0 is MODULATE, all other stages is DISABLE
|     default arg1 is TEXTURE
|     default arg2 is CURRENT
|___________________________________________________________________*/

inline void gx3d_SetTextureColorOp (int stage, int texture_colorop, int texture_arg1, int texture_arg2)
{
  if (gx_Video.set_texture_colorop)
    (*gx_Video.set_texture_colorop) (stage, texture_colorop, texture_arg1, texture_arg2);
}

/*____________________________________________________________________
|
| Function: gx3d_SetTextureAlphaOp
|
| Output: Sets the texture blending alpha operation.
|     default for stage 0 is SELECTARG1, all other stages is DISABLE
|     default arg1 is TEXTURE
|     default arg2 is CURRENT
|___________________________________________________________________*/

inline void gx3d_SetTextureAlphaOp (int stage, int texture_alphaop, int texture_arg1, int texture_arg2)
{
  if (gx_Video.set_texture_alphaop)
    (*gx_Video.set_texture_alphaop) (stage, texture_alphaop, texture_arg1, texture_arg2);
}

/*____________________________________________________________________
|
| Function: gx3d_SetTextureColorFactor
|
| Output: Sets the texture blending color factor.  This is the color used
|     for multiple texture blending w/ TFACTOR blending arg or 
|     BLENDFACTORALPHA operation.
|___________________________________________________________________*/

inline void gx3d_SetTextureColorFactor (gx3dColor color)
{
  if (gx_Video.set_texture_color_factor)
    (*gx_Video.set_texture_color_factor) ((float *)&color);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableCubemapTexturing
|
| Output: Enables cubemap texture processing for the texture stage.
|___________________________________________________________________*/

void gx3d_EnableCubemapTexturing (int stage)
{
  // Enable correct reflection vector processing
  if (gx_Video.enable_cubemap_texture_reflections)
    (*gx_Video.enable_cubemap_texture_reflections) (TRUE);

  // Enable use of auto generated cubemap texture coordinates
  gx3d_SetTextureCoordinates (stage, gx3d_TEXCOORD_CUBEMAP);

  // Set correct texture wrap mode
  gx3d_SetTextureAddressingMode (stage, gx3d_TEXTURE_DIMENSION_U | gx3d_TEXTURE_DIMENSION_V, gx3d_TEXTURE_ADDRESSMODE_MIRROR);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableCubemapTexturing
|
| Output: Disables cubemap texture processing for the texture stage.
|___________________________________________________________________*/

void gx3d_DisableCubemapTexturing (int stage)
{
  gx3dTexCoordSet coord_set [8] = {
    gx3d_TEXCOORD_SET0,
    gx3d_TEXCOORD_SET1,
    gx3d_TEXCOORD_SET2,
    gx3d_TEXCOORD_SET3,
    gx3d_TEXCOORD_SET4,
    gx3d_TEXCOORD_SET5,
    gx3d_TEXCOORD_SET6,
    gx3d_TEXCOORD_SET7,
  };
  
  // Disable reflection vector processing
  if (gx_Video.enable_cubemap_texture_reflections)
    (*gx_Video.enable_cubemap_texture_reflections) (FALSE);

  // Enable use of auto generated cubemap texture coordinates
  gx3d_SetTextureCoordinates (stage, coord_set[stage]);

  // Set correct texture wrap mode
  gx3d_SetTextureAddressingMode (stage, gx3d_TEXTURE_DIMENSION_U | gx3d_TEXTURE_DIMENSION_V, gx3d_TEXTURE_ADDRESSMODE_WRAP);
}

/*____________________________________________________________________
|
| Function: gx3d_BeginModifyTexture
|
| Output: Allows caller to modify/render to a renderable texture.  If
|   the texture is a cubemap, sets the face to render to according to:
|
|     0 = right
|     1 = left
|     2 = top
|     3 = bottom
|     4 = front
|     5 = back
|___________________________________________________________________*/

inline void gx3d_BeginModifyTexture (gx3dTexture texture, int face)
{
  Texture_Begin_Modify ((Texture *)texture, face);
}

/*____________________________________________________________________
|
| Function: gx3d_EndModifyTexture
|
| Output: Ends texture modify/render.
|___________________________________________________________________*/

inline void gx3d_EndModifyTexture (void)
{
  Texture_End_Modify ();
}
