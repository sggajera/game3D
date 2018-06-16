/*____________________________________________________________________
|
| File: pixmap.cpp
|
| Description: Functions to manipulate images, sprites and bitmaps.
|
| Functions:    gxImageSize
|               gxSpriteSize
|               gxBitmapSize
|               gxGetImage
|               gxGetSprite
|               gxGetBitmap
|                Clip_Image_To_Page
|               gxCreateImage
|               gxCreateSprite
|               gxCreateBitmap
|               gxCreateBytemap
|               gxDrawImage
|               gxDrawSprite
|               gxDrawSpriteMask
|               gxDrawBitmap
|               gxDrawPixelImage
|               gxGetPixelImage
|
| Note: Adapted from TX16.LIB graphics library by permission of author.
|
| Note: A note on VX image formats
|   All images/sprites/bitmaps begin with:
|     4-byte unsigned width (in pixels)
|     4-byte unsigned height (in pixels)
|
|   Next comes the image portion (images/sprites)
|     for 8-bit color
|       1-byte per pixel
|     for 16-bit color
|       2-bytes per pixel, in a bit pattern corresponding to the driver
|     for 24-bit color
|       3-bytes per pixel, in a bit pattern corresponding to the driver
|     for 32-bit color
|       4-bytes per pixel, in a bit pattern corresponding to the driver
|
|   Next comes the bitmap portion (sprites/bitmaps)
|     1-bit per pixel
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
#include "bitmap.h"
#include "img_clr.h"

/*___________________
|
| Function Prototpyes
|__________________*/

static int Clip_Image_To_Page (
  gxBound  box,
  int     *scn_x,
  int     *scn_y,
  int     *img_x,
  int     *img_y,
  int     *dx,
  int     *dy );

/*____________________________________________________________________
|
| Function: gxImageSize
|
| Output: Calculates buffer size to store an image.  Returns 0 on error.
|___________________________________________________________________*/

unsigned gxImageSize (gxBound box)
{
  unsigned size;

  size = (unsigned)(box.w) * (unsigned)(box.h) * gx_Pixel_size + (2*sizeof(unsigned));

  return (size);
}

/*____________________________________________________________________
|
| Function: gxSpriteSize
|
| Output: Calculates buffer size to store a sprite.  Returns 0 on error.
|___________________________________________________________________*/

unsigned gxSpriteSize (gxBound box)
{
  unsigned img_size, mask_size;

  img_size = gxImageSize (box);
  mask_size = (unsigned)((box.w+7)/8) * (unsigned)(box.h);

  return (img_size + mask_size);
}

/*____________________________________________________________________
|
| Function: gxBitmapSize
|
| Output: Calculates buffer size to store a bitmap.  Returns 0 on error.
|___________________________________________________________________*/

unsigned gxBitmapSize (gxBound box)
{
  unsigned size;

  size = (unsigned)((box.w+7)/8) * (unsigned)(box.h) + (2*sizeof(unsigned));

  return (size);
}

/*____________________________________________________________________
|
| Function: gxBytemapSize
|
| Output: Calculates buffer size to store a bytemap.  Returns 0 on error.
|___________________________________________________________________*/

unsigned gxBytemapSize (gxBound box)
{
  unsigned size;

  size = (unsigned)(box.w) * (unsigned)(box.h) + (2*sizeof(unsigned));

  return (size);
}

/*____________________________________________________________________
|
| Function: gxGetImage
|
| Output: Gets an image from the currently active page into a buffer.
|       Returns true if image was captured.
|___________________________________________________________________*/

void gxGetImage (gxBound box, byte *image)
{
  int scn_x, scn_y, img_x, img_y, dx, dy;
  unsigned *p;
  byte *image_data;

  /* Put image dimensions in buffer */
  p = (unsigned *) image;
  *p = box.w;
  *(p+1) = box.h;

  /* Clip to page */
  if (Clip_Image_To_Page (box, &scn_x, &scn_y, &img_x, &img_y, &dx, &dy)) {
    /* Put image data in buffer */
    image_data = image + (2 * sizeof(unsigned));
    (*gx_Video.get_image)(image_data, box.w, box.h, img_x, img_y,
                          scn_x, scn_y, dx, dy);
  }
}

/*____________________________________________________________________
|
| Function: gxGetSprite
|
| Output: Gets a sprite from the currently active page into a buffer.
|       If mask_color is not NULL, then all pixels of the
|       mask_color are removed from the image portion of the sprite.
|
|       If filter_color is not NULL, then all pixels of the
|       filter_color are removed from the image and bitmap portions of
|       the sprite.
|___________________________________________________________________*/

void gxGetSprite (
  gxBound  box, 
  byte    *sprite, 
  gxColor *mask_color,    // NULL = no color
  gxColor *filter_color ) // NULL = no color
{
  int i, scn_x, scn_y, img_x, img_y, dx, dy, last_img_x, already_removed_this_color;
  int x, y, bytes_per_row;
  unsigned n, *p;
  gxColor image_color;
  byte *image_data, *mask_data;
  byte bit_mask [8] = {
    0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1
  };

  /* Put sprite dimensions in buffer */
  p = (unsigned *) sprite;
  *p = box.w;
  *(p+1) = box.h;

  /* Clip to page */
  if (Clip_Image_To_Page (box, &scn_x, &scn_y, &img_x, &img_y, &dx, &dy)) {
    /* Put image portion of sprite in buffer */
    image_data = sprite + (2 * sizeof(unsigned));
    (*gx_Video.get_image)(image_data, box.w, box.h, img_x, img_y, scn_x, scn_y, dx, dy);

    /* Build sprite mask */
    mask_data = image_data + ((unsigned)(box.w) * (unsigned)(box.h) * gx_Pixel_size);
    bytes_per_row = (box.w+7)/8;
    n = (img_y * box.w * gx_Pixel_size) + (img_x * gx_Pixel_size);
    last_img_x = img_x + dx - 1;
    for (y=0; y<dy; y++) {
      for (x=img_x,i=0; x<=last_img_x; x++,i++) {
        image_color = Get_Image_Pixel_Color (&image_data[n], i, 0, 0);
        if (image_color.index != mask_color->index)
          mask_data[x/8] |= bit_mask[x%8];
        else
          mask_data[x/8] &= ~bit_mask[x%8];
      }
      n += (box.w * gx_Pixel_size);
      mask_data += bytes_per_row;
    }

    /* Remove all pixels from image portion of sprite that are same as filter color */
    if (filter_color) {
      n = (img_y * box.w * gx_Pixel_size) + (img_x * gx_Pixel_size);
      for (y=0; y<dy; y++) {
        for (x=0; x<dx; x++) {
          image_color = Get_Image_Pixel_Color (&image_data[n], x, 0, 0);
          if (image_color.index == filter_color->index)
            memset (&image_data[n+(x*gx_Pixel_size)], 0, gx_Pixel_size);
        }
        n += (box.w * gx_Pixel_size);
      }
    }

    /* Remove all transparent colored pixels from image portion of sprite */
    already_removed_this_color = FALSE;
    if (mask_color AND filter_color)
      if (mask_color->index == filter_color->index)
        already_removed_this_color = TRUE;

    if (mask_color AND (NOT already_removed_this_color)) {
      n = img_y * box.w * gx_Pixel_size + (img_x * gx_Pixel_size);
      for (y=0; y<dy; y++) {
        for (x=0; x<dx; x++) {
          image_color = Get_Image_Pixel_Color (&image_data[n], x, 0, 0);
          if (image_color.index == mask_color->index)
            memset (&image_data[n+(x*gx_Pixel_size)], 0, gx_Pixel_size);
        }
        n += (box.w * gx_Pixel_size);
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: gxGetBitmap
|
| Output: Gets a bitmap from the currently active page into a buffer.
|       Returns true if bitmap was captured.
|___________________________________________________________________*/

void gxGetBitmap (gxBound box, byte *bitmap)
{
  int scn_x, scn_y, img_x, img_y, dx, dy, last_img_x;
  int x, y, bytes_per_row;
  unsigned *p;
  gxColor image_color;
  byte *bitmap_data, *scanline;
  byte bit_mask [8] = {
    0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1
  };

  /* Put bitmap dimensions in buffer */
  p = (unsigned *) bitmap;
  *p = box.w;
  *(p+1) = box.h;

  /* Clip to page */
  if (Clip_Image_To_Page (box, &scn_x, &scn_y, &img_x, &img_y, &dx, &dy)) {
    /* Create a temp buffer to hold one row of pixels */
    scanline = (byte *) malloc (box.w * gx_Pixel_size);

    if (scanline) {
      /* Build bitmap */
      bitmap_data = bitmap + (2 * sizeof(unsigned));
      bytes_per_row = (box.w+7)/8;
      last_img_x = img_x + dx - 1;
      for (y=0; y<dy; y++) {
        /* Get 1 row of pixels */
        (*gx_Video.get_image)(scanline, box.w, 1, img_x, 0, scn_x, scn_y+y, dx, 1);
        /* Build 1 row of bitmap data */
        for (x=img_x; x<=last_img_x; x++) {
          image_color = Get_Image_Pixel_Color (scanline, x, 0, 0);
          if (image_color.index)
            bitmap_data[x/8] |= bit_mask[x%8];
          else
            bitmap_data[x/8] &= ~bit_mask[x%8];
        }
        bitmap_data += bytes_per_row;
      }

      /* Free dynamic memory */
      free (scanline);
    }
  }
}

/*____________________________________________________________________
|
| Function: Clip_Image_To_Page
|
| Input: Called from gxGetImage(), gxGetSprite(), gxGetBitmap()
| Output: Clips box, representing an image to capture, to page.
|       Returns true if any part of image is within the page rectangle.
|___________________________________________________________________*/

static int Clip_Image_To_Page (
  gxBound  box,
  int     *scn_x,
  int     *scn_y,
  int     *img_x,
  int     *img_y,
  int     *dx,
  int     *dy )
{
  int xleft, ytop, xright, ybottom, visible;
  gxRectangle save_clip, rect;

  gxGetClip (&save_clip);
  rect.xleft   = 0;
  rect.ytop    = 0;
  rect.xright  = PAGE_WIDTH-1;
  rect.ybottom = PAGE_HEIGHT-1;
  gxSetClip (&rect);
  xleft   = box.x;
  ytop    = box.y;
  xright  = box.x + box.w - 1;
  ybottom = box.y + box.h - 1;
  visible = gxClipRectangle (&xleft, &ytop, &xright, &ybottom);
  gxSetClip (&save_clip);

  if (visible) {
    *scn_x = xleft;
    *scn_y = ytop;
    *img_x = xleft - box.x;
    *img_y = ytop - box.y;
    *dx    = xright - xleft + 1;
    *dy    = ybottom - ytop + 1;
  }

  return (visible);
}

/*____________________________________________________________________
|
| Function: gxCreateImage
|
| Output: Creates a dynamically allocated image located on the currently
|       active page.  Returns pointer to the image.  Use free() to
|       free up the memory when finished using this image.
|___________________________________________________________________*/

byte *gxCreateImage (gxBound box)
{
  unsigned size;
  byte *img = NULL;

  size = gxImageSize (box);
  img = (byte *) malloc (size * sizeof (byte));
  if (img != NULL)
    gxGetImage (box, img);

  return (img);
}

/*____________________________________________________________________
|
| Function: gxCreateSprite
|
| Output: Creates a dynamically allocated sprite located on the currently
|       active page.  Returns pointer to the sprite.  Use free() to
|       free up the memory when finished using this sprite.
|___________________________________________________________________*/

byte *gxCreateSprite (
  gxBound  box, 
  gxColor *mask_color,    // NULL = no color
  gxColor *filter_color ) // NULL = no color
{
  unsigned size;
  byte *spr = NULL;

  size = gxSpriteSize (box);
  spr = (byte *) malloc (size * sizeof (byte));
  if (spr != NULL)
    gxGetSprite (box, spr, mask_color, filter_color);

  return (spr);
}

/*____________________________________________________________________
|
| Function: gxCreateBitmap
|
| Output: Creates a dynamically allocated bitmap located on the currently
|       active page.  Returns pointer to the bitmap.  Use free() to
|       free up the memory when finished using this bitmap.
|___________________________________________________________________*/

byte *gxCreateBitmap (gxBound box)
{
  unsigned size;
  byte *bmp = NULL;

  size = gxBitmapSize (box);
  bmp = (byte *) malloc (size * sizeof (byte));
  if (bmp != NULL)
    gxGetBitmap (box, bmp);

  return (bmp);
}

/*____________________________________________________________________
|
| Function: gxCreateBytemap
|
| Output: Creates a dynamically allocated bytemap.  Returns a pointer 
|       to the bytemap.  Use free() to free up the memory when finished
|       using this bytemap.
|
| Description: A bytemap is a pixel map where each pixel is a single 
|       byte.  This can be used for a variety of purposes such as a 
|       lighting map.  
|
|       The bytemap is created by taking the each RGB component of a
|       color, adding them together and dividing by 3 to get an average
|       intensity value.  This intensity is the value stored in the bytemap.
|___________________________________________________________________*/

byte *gxCreateBytemap (byte *image)
{
  int dx, dy, x, y;
  unsigned *p, size;
  gxBound box;
  gxColor image_color;
  byte *image_data, *bytemap_data;
  byte *bmp = NULL;

/*____________________________________________________________________
|
| Get image width, height
|___________________________________________________________________*/

  p = (unsigned *) image;
  dx = *p;
  dy = *(p+1);
  
/*____________________________________________________________________
|
| Allocate memory for the bytemap
|___________________________________________________________________*/

  box.x = 0;
  box.y = 0;
  box.w = dx;
  box.h = dy;
  size = gxBytemapSize (box);
  bmp = (byte *) malloc (size * sizeof (byte));

/*____________________________________________________________________
|
| Build the bytemap
|___________________________________________________________________*/

  if (bmp != NULL) {
    // Put bytemap dimensions in buffer
    p = (unsigned *) bmp;
    *p     = dx;
    *(p+1) = dy;

    // Read through image and write to bytemap
    image_data   = image + (2 * sizeof(unsigned));
    bytemap_data = bmp   + (2 * sizeof(unsigned));
    for (y=0; y<dy; y++) 
      for (x=0; x<dx; x++) {
        // Get a pixel from the image
        image_color = Get_Image_Pixel_Color (image_data, x, y, dx);
        // Write the average of the RGB components to the bytemap
        *bytemap_data = (byte) (((int)(image_color.r) + (int)(image_color.g) + (int)(image_color.b)) / 3);
        bytemap_data++;
      }
  }

  return (bmp);
}

/*____________________________________________________________________
|
| Function: gxDrawImage
|
| Output: Draws an image in the current window on the active page.
|___________________________________________________________________*/

void gxDrawImage (byte *image, int x, int y)
{
  int dx, dy, xright, ybottom;
  int clip_x, clip_y, clip_dx, clip_dy;
  unsigned *p;
  int totally_clipped = FALSE;

/*____________________________________________________________________
|
| Get image width, height, and other variables.
|___________________________________________________________________*/

  p = (unsigned *) image;
  dx = *p;
  dy = *(p+1);

  clip_x  = 0;
  clip_y  = 0;
  clip_dx = dx;
  clip_dy = dy;

/*____________________________________________________________________
|
| Adjust for current window.
|___________________________________________________________________*/

  x += gx_Window.xleft;
  y += gx_Window.ytop;

/*____________________________________________________________________
|
| If clipping is on, clip the image.
|___________________________________________________________________*/

  if (gx_Clipping) {

    /* Get coordinates of lower right corner of image */
    ybottom = y + dy - 1;
    xright  = x + dx - 1;

    /* Is image completely clipped? */
    if ((xright  < gx_Clip.xleft) OR (x > gx_Clip.xright) OR
        (ybottom < gx_Clip.ytop)  OR (y > gx_Clip.ybottom))
      totally_clipped = TRUE;
    else {
      /* Clip against BOTTOM edge of clip */
      if (ybottom > gx_Clip.ybottom)
        clip_dy -= (ybottom - gx_Clip.ybottom);

      /* Clip against TOP edge of clip */
      if (y < gx_Clip.ytop) {
        clip_y = gx_Clip.ytop - y;
        clip_dy -= clip_y;
      }

      /* Clip against RIGHT edge of clip */
      if (xright > gx_Clip.xright)
        clip_dx -= (xright - gx_Clip.xright);

      /* Clip against LEFT edge of clip */
      if (x < gx_Clip.xleft) {
        clip_x = gx_Clip.xleft - x;
        clip_dx -= clip_x;
      }
    }
  }

/*____________________________________________________________________
|
| Draw image.
|___________________________________________________________________*/

  x += clip_x;
  y += clip_y;

  // Make sure image is contained entirely within page
  DEBUG_ASSERT (x >= 0);
  DEBUG_ASSERT (x < PAGE_WIDTH);
  DEBUG_ASSERT (y >= 0);
  DEBUG_ASSERT (y < PAGE_HEIGHT);
  DEBUG_ASSERT (x+clip_dx-1 <= PAGE_WIDTH-1);
  DEBUG_ASSERT (y+clip_dy-1 <= PAGE_HEIGHT-1);

  if (NOT totally_clipped)
    (*gx_Video.put_image)(image+2*sizeof(unsigned), dx, dy, clip_x, clip_y,
                          x, y, clip_dx, clip_dy, 0);
}

/*____________________________________________________________________
|
| Function: gxDrawSprite
|
| Output: Draws a sprite in the current window on the active page.
|___________________________________________________________________*/

void gxDrawSprite (byte *sprite, int x, int y)
{
  int dx, dy, xright, ybottom;
  int clip_x, clip_y, clip_dx, clip_dy;
  unsigned *p;
  byte *image, *bitmap;
  int totally_clipped = FALSE;

/*____________________________________________________________________
|
| Get sprite width, height, and other variables.
|___________________________________________________________________*/

  p = (unsigned *) sprite;
  dx = *p;
  dy = *(p+1);

  clip_x  = 0;
  clip_y  = 0;
  clip_dx = dx;
  clip_dy = dy;

/*____________________________________________________________________
|
| Adjust for current window.
|___________________________________________________________________*/

  x += gx_Window.xleft;
  y += gx_Window.ytop;

/*____________________________________________________________________
|
| If clipping is on, clip the sprite.
|___________________________________________________________________*/

  if (gx_Clipping) {

    /* Get coordinates of lower right corner of image */
    ybottom = y + dy - 1;
    xright  = x + dx - 1;

    /* Is image completely clipped? */
    if ((xright  < gx_Clip.xleft) OR (x > gx_Clip.xright) OR
        (ybottom < gx_Clip.ytop)  OR (y > gx_Clip.ybottom))
      totally_clipped = TRUE;
    else {
      /* Clip against BOTTOM edge of clip */
      if (ybottom > gx_Clip.ybottom)
        clip_dy -= (ybottom - gx_Clip.ybottom);

      /* Clip against TOP edge of clip */
      if (y < gx_Clip.ytop) {
        clip_y = gx_Clip.ytop - y;
        clip_dy -= clip_y;
      }

      /* Clip against RIGHT edge of clip */
      if (xright > gx_Clip.xright)
        clip_dx -= (xright - gx_Clip.xright);

      /* Clip against LEFT edge of clip */
      if (x < gx_Clip.xleft) {
        clip_x = gx_Clip.xleft - x;
        clip_dx -= clip_x;
      }
    }
  }

/*____________________________________________________________________
|
| Draw sprite.
|___________________________________________________________________*/

  x += clip_x;
  y += clip_y;

  // Make sure image is contained entirely within page
  DEBUG_ASSERT (x >= 0);
  DEBUG_ASSERT (x < PAGE_WIDTH);
  DEBUG_ASSERT (y >= 0);
  DEBUG_ASSERT (y < PAGE_HEIGHT);
  DEBUG_ASSERT (x+clip_dx-1 <= PAGE_WIDTH-1);
  DEBUG_ASSERT (y+clip_dy-1 <= PAGE_HEIGHT-1);

  if (NOT totally_clipped) {
    image  = sprite +  2 * sizeof(unsigned);
    bitmap = sprite + (2 * sizeof(unsigned)) + (dx * dy * gx_Pixel_size);
    (*gx_Video.put_bitmap)(bitmap, dx, dy, clip_x, clip_y, x, y, clip_dx, clip_dy, 0, 0, 0);
    (*gx_Video.put_image) (image,  dx, dy, clip_x, clip_y, x, y, clip_dx, clip_dy, 1);
  }
}

/*____________________________________________________________________
|
| Function: gxDrawSpriteMask
|
| Output: Draws the mask portion of a sprite in the current window on
|       the active page.
|___________________________________________________________________*/

void gxDrawSpriteMask (byte *sprite, int x, int y, gxColor color)
{
  int dx, dy;
  unsigned *p;
  byte *bitmap;

  /* Init variables */
  p = (unsigned *) sprite;
  dx = *p;
  dy = *(p+1);
  bitmap = sprite + (2 * sizeof(unsigned)) + (dx * dy * gx_Pixel_size);

  /* Draw the bitmap */
  Draw_Bitmap (bitmap, dx, dy, 0, 0, x, y, dx, dy, color);
}

/*____________________________________________________________________
|
| Function: gxDrawBitmap
|
| Output: Draws the bitmap in the current window on the active page.
|___________________________________________________________________*/

void gxDrawBitmap (byte *bitmap, int x, int y, gxColor color)
{
  int dx, dy;
  unsigned *p;

  /* Init variables */
  p = (unsigned *) bitmap;
  dx = *p;
  dy = *(p+1);

  /* Draw the bitmap */
  Draw_Bitmap (bitmap+(2*sizeof(unsigned)), dx, dy, 0, 0, x, y, dx, dy, color);
}

/*____________________________________________________________________
|
| Function: gxDrawPixelImage
|
| Output: Draws a pixel in an image.
|___________________________________________________________________*/

void gxDrawPixelImage (byte *image, int x, int y, gxColor color)
{
  int dx;

  dx = *(int *)image;
  Put_Image_Pixel_Color (image+2*sizeof(unsigned), x, y, dx, color);
}

/*____________________________________________________________________
|
| Function: gxGetPixelImage
|
| Output: Draws a pixel in an image.
|___________________________________________________________________*/

gxColor gxGetPixelImage (byte *image, int x, int y)
{
  int dx;
  gxColor color;

  dx = *(int *)image;
  color = Get_Image_Pixel_Color (image+2*sizeof(unsigned), x, y, dx);
  
  return (color);
}
