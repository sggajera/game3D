/*____________________________________________________________________
|
| File: d3d9_2d.h (Direct3D 2D functions)
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*____________________
|
|	Aliases
|___________________*/

#define Direct3D_Vertical_Retrace_Delay d3d9_Vertical_Retrace_Delay
#define Direct3D_Set_Palette            d3d9_Set_Palette
#define Direct3D_Get_Palette            d3d9_Get_Palette
#define Direct3D_Set_Active_Surface     d3d9_Set_Active_Surface
#define Direct3D_Set_Fore_Color         d3d9_Set_Fore_Color
#define Direct3D_Set_Logic_Op           d3d9_Set_Logic_Op
#define Direct3D_Allocate_Surface       d3d9_Allocate_Surface
#define Direct3D_Free_Surface           d3d9_Free_Surface
#define Direct3D_Get_Surface_Dimensions d3d9_Get_Surface_Dimensions
#define Direct3D_Clear_Surface          d3d9_Clear_Surface
#define Direct3D_Draw_Fill_Rectangle    d3d9_Draw_Fill_Rectangle
#define Direct3D_Draw_Pixel             d3d9_Draw_Pixel
#define Direct3D_Get_Pixel              d3d9_Get_Pixel
#define Direct3D_Draw_Line              d3d9_Draw_Line
#define Direct3D_Copy_Image             d3d9_Copy_Image
#define Direct3D_Copy_Image_ColorKey    d3d9_Copy_Image_ColorKey
#define Direct3D_Put_Image              d3d9_Put_Image
#define Direct3D_Get_Image              d3d9_Get_Image
#define Direct3D_Put_Bitmap             d3d9_Put_Bitmap
#define Direct3D_BltSurface             d3d9_BltSurface
// Cursor functions
#define Direct3D_Set_Image_Cursor       d3d9_Set_Image_Cursor
#define Direct3D_Set_Bitmap_Cursor      d3d9_Set_Bitmap_Cursor
#define Direct3D_Free_Cursor            d3d9_Free_Cursor
#define Direct3D_Set_Cursor_Position    d3d9_Set_Cursor_Position
#define Direct3D_Show_Cursor            d3d9_Show_Cursor

/*____________________
|
|	Functions
|___________________*/

void  d3d9_Vertical_Retrace_Delay ();
int		d3d9_Set_Palette (byte *rgb_palette, int start_color, int num_colors);
int		d3d9_Get_Palette (byte *rgb_palette, int start_color, int num_colors);
void  d3d9_Set_Active_Surface (byte *surface);
void	d3d9_Set_Fore_Color (byte r, byte g, byte b, byte a);
void	d3d9_Set_Logic_Op (int logic_op);
byte *d3d9_Allocate_Surface (int width, int height);
void	d3d9_Free_Surface (byte *surface);
int		d3d9_Get_Surface_Dimensions (byte *surface, int *width, int *height);
void	d3d9_Clear_Surface (byte r, byte g, byte b, byte *surface = 0);
int	  d3d9_Draw_Fill_Rectangle (int x1, int y1, int x2, int y2, byte *surface = 0);
int	  d3d9_Draw_Pixel (int x, int y, byte *surface = 0);
void	d3d9_Get_Pixel (int x, int y, byte *r, byte *g, byte *b, byte *surface = 0);
int	  d3d9_Draw_Line (int x1, int y1, int x2, int y2, byte *surface = 0);
int	  d3d9_Copy_Image (int srcx,	int srcy,	byte *srcsurface, int dstx, int dsty, byte *dstsurface,	int dx,	int dy);
int	  d3d9_Copy_Image_ColorKey (int srcx,	int srcy,	byte *srcsurface, int dstx, int dsty, byte *dstsurface,	int dx,	int dy,	byte r, byte g, byte b);
int	  d3d9_Put_Image (
	byte *image,
	int   image_dx,		// in pixels
	int   image_x,		// top-left coordinates in image
	int   image_y,
	int   x,					// top-left coordinates of destination
	int   y,
	int   dx,					// size in pixels of rectangle to transfer
	int   dy,
	int   or,					// boolean
	byte *surface = 0 );
int d3d9_Get_Image (
	byte *image,
	int   image_dx,		// in pixels
	int   image_x,		// top-left coordinates in image
	int   image_y,
	int   x,					// top-left coordinates of destination
	int   y,
	int   dx,					// size in pixels of rectangle to transfer
	int   dy,
	byte *surface = 0 );
int d3d9_Put_Bitmap (
	byte *bitmap,
	int   bitmap_dx,	// in bits
	int   bitmap_x,		// top-left coordinates in bitmap
	int   bitmap_y,
	int   x,					// top-left coordinates of destination
	int   y,
	int   dx,					// size in bits of rectangle to transfer
	int   dy,
	byte  r,					// color to draw bitmap as
	byte  g,
	byte  b,
	byte *surface = 0 );

// Cursor functions
void d3d9_Set_Image_Cursor (
	byte *image, 
	int		image_dx, 
	int		image_dy, 
	int		hot_x,                                  
	int		hot_y );
void d3d9_Set_Bitmap_Cursor (
	byte *cursor_bitmap, 
	byte *mask_bitmap,
	int   bitmap_dx,
	int   bitmap_dy,
	int		hot_x,
	int		hot_y,
  byte  cursor_color_r,
  byte  cursor_color_g,
  byte  cursor_color_b,
  byte  mask_color_r,
  byte  mask_color_g,
  byte  mask_color_b );
void d3d9_Free_Cursor ();
void d3d9_Set_Cursor_Position (unsigned x, unsigned y);
void d3d9_Show_Cursor (int flag);
