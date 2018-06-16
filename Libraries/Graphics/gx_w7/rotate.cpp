/*____________________________________________________________________
|
| File: rotate.cpp
|
| Description: Graphics functions for pixmap rotation.
|
| Functions:    gxRotateImage
|               gxRotateSprite
|               gxRotateBitmap
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include "dp.h"

/*____________________________________________________________________
|
| Function: gxRotateImage
|
| Input: An image to rotate.
| Output: Creates a new rotated image from original.  Returns pointer to
|       image.  Use free() to free up the memory when finished using
|       this image.
|
| Description: Currently, only rotations of 90 degrees are supported.
|___________________________________________________________________*/

byte *gxRotateImage (byte *image, float degrees)
{
  int x, y, old_dx, old_dy, new_dx, new_dy;
  unsigned *p, size;
  byte *new_image, *pold, *pnew;
  gxBound box;

/*____________________________________________________________________
|
| Error checking on input parameters.
|___________________________________________________________________*/

  if (degrees != 90)
    return (NULL);

/*____________________________________________________________________
|
| Get dimensions of old and new (rotated) images.
|___________________________________________________________________*/

  p = (unsigned *) image;
  old_dx = *p;
  old_dy = *(p+1);
  new_dx = old_dy;
  new_dy = old_dx;

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
| Rotate the image.
|___________________________________________________________________*/

  /* Set pointer to start of old image data */
  pold = image + (2*sizeof(unsigned));
  /* Set pointer to start of new image data */
  pnew = new_image + (2*sizeof(unsigned));

  for (y=0; y<new_dy; y++)
    for (x=0; x<new_dx; x++) {
      memcpy (pnew, &pold[(old_dy-x-1)*old_dx*gx_Pixel_size + y*gx_Pixel_size], gx_Pixel_size);
      pnew += gx_Pixel_size;
    }

//      *pnew++ = pold[(old_dy-x-1)*old_dx+y];

/*____________________________________________________________________
|
| Return pointer to rotated image.
|___________________________________________________________________*/

  return (new_image);
}

/*____________________________________________________________________
|
| Function: gxRotateSprite
|
| Input: A sprite to rotate.
| Output: Creates a new rotated sprite from original.  Returns pointer to
|       sprite.  Use free() to free up the memory when finished using
|       this sprite.
|
| Description: Currently, only rotations of 90 degrees are supported.
|___________________________________________________________________*/

byte *gxRotateSprite (byte *sprite, float degrees)
{
  int x, y, old_dx, old_dy, new_dx, new_dy, old_mask_dx, new_mask_dx;
  unsigned *p, size;
  byte bit, *new_sprite, *pold, *pnew, *poldmask, *pnewmask;
  gxBound box;

  byte bitmask [8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
  };

/*____________________________________________________________________
|
| Error checking on input parameters.
|___________________________________________________________________*/

  if (degrees != 90)
    return (NULL);

/*____________________________________________________________________
|
| Get dimensions of old and new (rotated) sprites.
|___________________________________________________________________*/

  p = (unsigned *) sprite;
  old_dx = *p;
  old_dy = *(p+1);
  new_dx = old_dy;
  new_dy = old_dx;

/*____________________________________________________________________
|
| Allocate memory for the new sprite and set its width & height.
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
| Rotate the sprite.
|___________________________________________________________________*/

  /* Init variables */
  pold = sprite     + (2*sizeof(unsigned));
  pnew = new_sprite + (2*sizeof(unsigned));
  poldmask = pold + (old_dx * old_dy);
  pnewmask = pnew + (new_dx * new_dy);
  old_mask_dx = (old_dx+7)/8;
  new_mask_dx = (new_dx+7)/8;

  for (y=0; y<new_dy; y++)
    for (x=0; x<new_dx; x++) {
      memcpy (pnew, &pold[(old_dy-x-1)*old_dx*gx_Pixel_size + y*gx_Pixel_size], gx_Pixel_size);
      pnew += gx_Pixel_size;
//      *pnew++ = pold[(old_dy-x-1)*old_dx+y];
      bit = poldmask[(old_dy-x-1)*old_mask_dx+(y/8)] & bitmask[y%8];
      if (bit)
        pnewmask[y*new_mask_dx+(x/8)] |= bitmask[x%8];
    }
    
/*____________________________________________________________________
|
| Return pointer to rotated sprite.
|___________________________________________________________________*/

  return (new_sprite);
}

/*____________________________________________________________________
|
| Function: gxRotateBitmap
|
| Input: A bitmap to rotate.
| Output: Creates a new rotated bitmap from original.  Returns pointer to
|       bitmap.  Use free() to free up the memory when finished using
|       this bitmap.
|
| Description: Currently, only rotations of 90 degrees are supported.
|___________________________________________________________________*/

byte *gxRotateBitmap (byte *bitmap, float degrees)
{
  int x, y, old_dx, old_dy, new_dx, new_dy, old_mask_dx, new_mask_dx;
  unsigned *p, size;
  byte bit, *new_bitmap, *poldmask, *pnewmask;
  gxBound box;

  byte bitmask [8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
  };

/*____________________________________________________________________
|
| Error checking on input parameters.
|___________________________________________________________________*/

  if (degrees != 90)
    return (NULL);

/*____________________________________________________________________
|
| Get dimensions of old and new (rotated) bitmaps.
|___________________________________________________________________*/

  p = (unsigned *) bitmap;
  old_dx = *p;
  old_dy = *(p+1);
  new_dx = old_dy;
  new_dy = old_dx;

/*____________________________________________________________________
|
| Allocate memory for the new bitmap and set its width & height.
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
| Rotate the bitmap.
|___________________________________________________________________*/

  /* Init variables */
  poldmask = bitmap     + (2*sizeof(unsigned));
  pnewmask = new_bitmap + (2*sizeof(unsigned));
  old_mask_dx = (old_dx+7)/8;
  new_mask_dx = (new_dx+7)/8;

  for (y=0; y<new_dy; y++)
    for (x=0; x<new_dx; x++) {
      bit = poldmask[(old_dy-x-1)*old_mask_dx+(y/8)] & bitmask[y%8];
      if (bit)
        pnewmask[y*new_mask_dx+(x/8)] |= bitmask[x%8];
    }

/*____________________________________________________________________
|
| Return pointer to rotated bitmap.
|___________________________________________________________________*/

  return (new_bitmap);
}
