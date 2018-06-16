/*____________________________________________________________________
|
| File: virtual.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

void virtual_Init (int page);
void virtual_DrawPixel (int x, int y);
void virtual_GetPixel (int x, int y, byte *r, byte *g, byte *b);
void virtual_DrawLine (int x1, int y1, int x2, int y2);
void virtual_DrawFillRectangle (int x1, int y1, int x2, int y2);
void virtual_PutImage (
  byte *image,
  int   image_dx,
  int   image_dy,
  int   image_x,
  int   image_y,
  int   x,
  int   y,
  int   dx,
  int   dy,
  int   or_image );     /* boolean */
void virtual_GetImage (
  byte *image,
  int   image_dx,
  int   image_dy,
  int   image_x,
  int   image_y,
  int   x,
  int   y,
  int   dx,
  int   dy );
void virtual_PutBitmap (
  byte *bitmap,
  int   bitmap_dx,
  int   bitmap_dy,
  int   bitmap_x,
  int   bitmap_y,
  int   x,
  int   y,
  int   dx,
  int   dy,
  byte  r,
  byte  g,
  byte  b );
