/*____________________________________________________________________
|
| File: scale.cpp
|
| Description: Graphics functions for pixmap scaling.
|
| Functions:    gxScaleImage
|               gxScaleSprite
|               gxScaleBitmap
|                Set_Scale_Array
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

#include "dp.h"

/*___________________
|
| Function prototypes
|__________________*/

static void Set_Scale_Array (
  int *array,           // pointer to an array
  int  num_src,         // number of pixels in source
  int  num_dest,        // number of pixels in destination
  int  direction );     // left-to-right or right-to-left

/*___________________
|
| Constants
|__________________*/

#define LEFT_TO_RIGHT   0
#define RIGHT_TO_LEFT   1

/*____________________________________________________________________
|
| Function: gxScaleImage
|
| Input: An image to scale.
| Output: Creates a new scaled image from original.  Returns pointer to
|       image.  Use free() to free up the memory when finished using
|       this image.
|___________________________________________________________________*/

byte *gxScaleImage (byte *image, float sx, float sy)
{
  int x, y, old_dx, old_dy, new_dx, new_dy, xdirection, ydirection;
  int *col, *row;
  unsigned *p, size;
  byte *new_image, *pnew, *pold;
  gxBound box;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (image);

/*____________________________________________________________________
|
| Get dimensions of old and new (scaled) images.
|___________________________________________________________________*/

  p = (unsigned *) image;
  old_dx = *p;
  old_dy = *(p+1);
  new_dx = (int)((float)old_dx * sx);
  if (new_dx < 0) {
    new_dx = -new_dx;
    xdirection = RIGHT_TO_LEFT;
  }
  else
    xdirection = LEFT_TO_RIGHT;
  new_dy = (int)((float)old_dy * sy);
  if (new_dy < 0) {
    new_dy = -new_dy;
    ydirection = RIGHT_TO_LEFT;
  }
  else
    ydirection = LEFT_TO_RIGHT;

/*____________________________________________________________________
|
| Allocate memory for the new image and set its width & height.
|___________________________________________________________________*/

  box.x = 0;
  box.y = 0;
  box.w = new_dx;
  box.h = new_dy;
  size = gxImageSize (box);
  new_image = (byte *) calloc (1, size * sizeof(byte));
  if (new_image == NULL)
    return (NULL);

  p = (unsigned *) new_image;
  *p     = new_dx;
  *(p+1) = new_dy;

/*____________________________________________________________________
|
| Allocate memory for scaling arrays.
|___________________________________________________________________*/

  col = (int *) malloc (new_dx * sizeof (int));
  if (col == NULL) {
    free (new_image);
    return (NULL);
  }
  row = (int *) malloc (new_dy * sizeof (int));
  if (row == NULL) {
    free (col);
    free (new_image);
    return (NULL);
  }

/*____________________________________________________________________
|
| Scale the image.
|___________________________________________________________________*/

  Set_Scale_Array (col, old_dx, new_dx, xdirection);
  Set_Scale_Array (row, old_dy, new_dy, ydirection);

  /* Set pointer to start of new image */
  pnew = new_image + (2*sizeof(unsigned));

  for (y=0; y<new_dy; y++) {
    /* Set pointer to row in old image */
    pold = image + (2*sizeof(unsigned)) + (row[y] * old_dx * gx_Pixel_size);
    /* If next row in new image will be same as last row */
    if (y AND (row[y] == row[y-1]))
      /* Duplicate one entire row in new image */
      memcpy (pnew, pnew-(new_dx*gx_Pixel_size), new_dx*gx_Pixel_size);
    else
      for (x=0; x<new_dx; x++) 
        memcpy (&pnew[x], &pold[col[x] * gx_Pixel_size], gx_Pixel_size);
//        pnew[x] = pold[col[x]];
    /* Increment pointer to next row in new image */
    pnew += (new_dx * gx_Pixel_size);
  }

/*____________________________________________________________________
|
| Return pointer to scaled image.
|___________________________________________________________________*/

  free (col);
  free (row);

  return (new_image);
}

/*____________________________________________________________________
|
| Function: gxScaleSprite
|
| Input: A sprite to scale.
| Output: Creates a new scaled sprite from original.  Returns pointer to
|       sprite.  Use free() to free up the memory when finished using
|       this sprite.
|___________________________________________________________________*/

byte *gxScaleSprite (byte *sprite, float sx, float sy)
{
  int x, y, old_dx, old_dy, old_mask_dx, new_dx, new_dy, new_mask_dx;
  int xdirection, ydirection;
  int *col, *row;
  unsigned *p, size, new_image_size, old_image_size;
  byte bit, *new_sprite, *pnew, *pold, *pnewmask, *poldmask;
  gxBound box;

  byte bitmask [8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
  };

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (sprite);

/*____________________________________________________________________
|
| Get dimensions of old and new (scaled) sprites
|___________________________________________________________________*/

  p = (unsigned *) sprite;
  old_dx = *p;
  old_dy = *(p+1);
  new_dx = (int)((float)old_dx * sx);
  if (new_dx < 0) {
    new_dx = -new_dx;
    xdirection = RIGHT_TO_LEFT;
  }
  else
    xdirection = LEFT_TO_RIGHT;
  new_dy = (int)((float)old_dy * sy);
  if (new_dy < 0) {
    new_dy = -new_dy;
    ydirection = RIGHT_TO_LEFT;
  }
  else
    ydirection = LEFT_TO_RIGHT;

/*____________________________________________________________________
|
| Allocate memory for the new sprite and set its width & height
|___________________________________________________________________*/

  box.x = 0;
  box.y = 0;
  box.w = new_dx;
  box.h = new_dy;
  size = gxSpriteSize (box);
  new_sprite = (byte *) calloc (1, size * sizeof(byte));
  if (new_sprite == NULL)
    return (NULL);

  p = (unsigned *) new_sprite;
  *p     = new_dx;
  *(p+1) = new_dy;

/*____________________________________________________________________
|
| Allocate memory for scaling arrays
|___________________________________________________________________*/

  col = (int *) malloc (new_dx * sizeof (int));
  if (col == NULL) {
    free (new_sprite);
    return (NULL);
  }
  row = (int *) malloc (new_dy * sizeof (int));
  if (row == NULL) {
    free (col);
    free (new_sprite);
    return (NULL);
  }

/*____________________________________________________________________
|
| Scale the sprite
|___________________________________________________________________*/

  Set_Scale_Array (col, old_dx, new_dx, xdirection);
  Set_Scale_Array (row, old_dy, new_dy, ydirection);

  /* Init variables */
  new_image_size = new_dx * new_dy;
  old_image_size = old_dx * old_dy;
  old_mask_dx = (old_dx+7)/8;
  new_mask_dx = (new_dx+7)/8;

  /* Set pointer to start of new sprite */
  pnew     = new_sprite + (2*sizeof(unsigned));
  pnewmask = pnew + new_image_size;

  for (y=0; y<new_dy; y++) {
    /* Set pointer to row in old image */
    pold     = sprite + (2*sizeof(unsigned)) + (row[y] * old_dx * gx_Pixel_size);
    poldmask = sprite + (2*sizeof(unsigned)) + old_image_size + (row[y] * old_mask_dx);
    /* If next row in new image will be same as last row */
    if (y AND (row[y] == row[y-1])) {
      /* Duplicate a row in new image */
      memcpy (pnew, pnew-(new_dx*gx_Pixel_size), new_dx*gx_Pixel_size);
      /* Duplicate a row in new mask */
      memcpy (pnewmask, pnewmask-new_mask_dx, new_mask_dx);
    }
    else
      for (x=0; x<new_dx; x++) {
        /* Create a new scaled row in new image */
        memcpy (&pnew[x], &pold[col[x] * gx_Pixel_size], gx_Pixel_size);
        /* Create a new scaled row in new mask */
        bit = poldmask[col[x]/8] & bitmask[col[x]%8];
        if (bit)
          pnewmask[x/8] |= bitmask[x%8];
      }
    /* Increment pointer to next row in new image */
    pnew     += (new_dx * gx_Pixel_size);
    pnewmask += new_mask_dx;
  }
  
/*____________________________________________________________________
|
| Return pointer to scaled sprite.
|___________________________________________________________________*/

  free (col);
  free (row);

  return (new_sprite);
}

/*____________________________________________________________________
|
| Function: gxScaleBitmap
|
| Input: A bitmap to scale.
| Output: Creates a new scaled bitmap from original.  Returns pointer to
|       bitmap.  Use free() to free up the memory when finished using
|       this bitmap.
|___________________________________________________________________*/

byte *gxScaleBitmap (byte *bitmap, float sx, float sy)
{
  int x, y, old_dx, old_dy, old_mask_dx, new_dx, new_dy, new_mask_dx;
  int xdirection, ydirection;
  int *col, *row;
  unsigned *p, size;
  byte bit, *new_bitmap, *pnewmask, *poldmask;
  gxBound box;

  byte bitmask [8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
  };

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (bitmap);

/*____________________________________________________________________
|
| Get dimensions of old and new (scaled) bitmaps
|___________________________________________________________________*/

  p = (unsigned *) bitmap;
  old_dx = *p;
  old_dy = *(p+1);
  new_dx = (int)((float)old_dx * sx);
  if (new_dx < 0) {
    new_dx = -new_dx;
    xdirection = RIGHT_TO_LEFT;
  }
  else
    xdirection = LEFT_TO_RIGHT;
  new_dy = (int)((float)old_dy * sy);
  if (new_dy < 0) {
    new_dy = -new_dy;
    ydirection = RIGHT_TO_LEFT;
  }
  else
    ydirection = LEFT_TO_RIGHT;

/*____________________________________________________________________
|
| Allocate memory for the new bitmap and set its width & height
|___________________________________________________________________*/

  box.x = 0;
  box.y = 0;
  box.w = new_dx;
  box.h = new_dy;
  size = gxBitmapSize (box);
  new_bitmap = (byte *) calloc (1, size * sizeof(byte));
  if (new_bitmap == NULL)
    return (NULL);

  p = (unsigned *) new_bitmap;
  *p     = new_dx;
  *(p+1) = new_dy;

/*____________________________________________________________________
|
| Allocate memory for scaling arrays
|___________________________________________________________________*/

  col = (int *) malloc (new_dx * sizeof (int));
  if (col == NULL) {
    free (new_bitmap);
    return (NULL);
  }
  row = (int *) malloc (new_dy * sizeof (int));
  if (row == NULL) {
    free (col);
    free (new_bitmap);
    return (NULL);
  }

/*____________________________________________________________________
|
| Scale the bitmap
|___________________________________________________________________*/

  Set_Scale_Array (col, old_dx, new_dx, xdirection);
  Set_Scale_Array (row, old_dy, new_dy, ydirection);

  /* Init variables */
  old_mask_dx = (old_dx+7)/8;
  new_mask_dx = (new_dx+7)/8;

  /* Set pointer to start of new bitmap */
  pnewmask = new_bitmap + (2*sizeof(unsigned));

  for (y=0; y<new_dy; y++) {
    /* Set pointer to row in old image */
    poldmask = bitmap + (2*sizeof(unsigned)) + (row[y] * old_mask_dx);
    /* If next row in new image will be same as last row */
    if (y AND (row[y] == row[y-1])) {
      /* Duplicate a row in new mask */
      memcpy (pnewmask, pnewmask-new_mask_dx, new_mask_dx);
    }
    else
      for (x=0; x<new_dx; x++) {
        /* Create a new scaled row in new mask */
        bit = poldmask[col[x]/8] & bitmask[col[x]%8];
        if (bit)
          pnewmask[x/8] |= bitmask[x%8];
      }
    /* Increment pointer to next row in new image */
    pnewmask += new_mask_dx;
  }

/*____________________________________________________________________
|
| Return pointer to scaled bitmap.
|___________________________________________________________________*/

  free (col);
  free (row);

  return (new_bitmap);
}

/*____________________________________________________________________
|
| Function: Set_Scale_Array
|
| Input: Called from gxScaleImage(), gxScaleSprite(), gxScaleBitmap()
| Output: An array used for scaling.
|___________________________________________________________________*/

static void Set_Scale_Array (
  int *array,           // pointer to an array
  int  num_src,         // number of pixels in source
  int  num_dest,        // number of pixels in destination
  int  direction )      // left-to-right or right-to-left
{
  int i, fac;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (array);
  DEBUG_ASSERT (num_src > 0);
  DEBUG_ASSERT (num_dest > 0);
  DEBUG_ASSERT ((direction == LEFT_TO_RIGHT) OR (direction == RIGHT_TO_LEFT));

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (direction == LEFT_TO_RIGHT)
    for (i=0,fac=0; i<num_dest; i++,fac+=num_src)
      array[i] = fac / num_dest;
  else /* RIGHT_TO_LEFT */
    for (i=0,fac=0; i<num_dest; i++,fac+=num_src)
      array[num_dest-1-i] = fac / num_dest;
}
