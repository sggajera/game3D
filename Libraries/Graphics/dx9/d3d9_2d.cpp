/*___________________________________________________________________
|
|	File: d3d9_2d.cpp
|
|	Description: Functions that interface to DirectDraw7.
|                                 
| Functions:	d3d9_Vertical_Retrace_Delay
|							d3d9_Set_Active_Surface
|							d3d9_Set_Fore_Color
|							 RGB_To_Pixel
|              RGBA_To_Pixel
|							d3d9_Se_Logic_Op               
|							d3d9_Allocate_Surface
|							d3d9_Free_Surface               
|							d3d9_Get_Surface_Dimensions     
|							d3d9_Clear_Surface
|							d3d9_Draw_Fill_Rectangle
|							 Draw_Fill_Rectangle_AND
|							 Draw_Fill_Rectangle_OR
|							 Draw_Fill_Rectangle_XOR
|							 Draw_Fill_Rectangle_ADD
|							 Draw_Fill_Rectangle_SUBTRACT
|              Draw_Fill_Rectangle_SHL
|              Draw_Fill_Rectangle_SHR
|              Draw_Fill_Rectangle_MULTIPLY
|							d3d9_Draw_Pixel	
|							d3d9_Get_Pixel
|							 Pixel_To_RGB
|							d3d9_Draw_Line
|							 Draw_Line
|							d3d9_Copy_Image
|             d3d9_Copy_Image_ColorKey
|							d3d9_Put_Image
|							 Put_Image
|							d3d9_Get_Image
|							 Get_Image				
|							d3d9_Put_Bitmap
|							 Put_Bitmap
|
|             d3d9_Set_Image_Cursor
|             d3d9_Set_Bitmap_Cursor
|              Adjust_Cursor_Size
|              Allocate_Cursor_Surface
|             d3d9_Free_Cursor
|             d3d9_Set_Cursor_Position
|             d3d9_Show_Cursor
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/
                    
#define _D3D9_2D_CPP_

#define DIRECT3D_VERSION 0x0900

/*____________________
|
| Include files
|___________________*/

#include <first_header.h>

#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>

#include <stdio.h>
#include <process.h>   
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <malloc.h>
#include <process.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9tex.h>

#include <defines.h>
#include <clib.h>
#include <events.h>
#include <win_support.h>

#include "d3d9_dp.h"
#include "d3d9_2d.h"

/*____________________
|
| Function prototypes
|___________________*/

static DWORD RGB_To_Pixel (byte r, byte g, byte b);	
DWORD d3d9_RGBA_To_Pixel (byte r, byte g, byte b, byte a);
static void	 Draw_Fill_Rectangle_AND      (byte *surfdata, int surfpitch, int dx, int dy);
static void	 Draw_Fill_Rectangle_OR       (byte *surfdata, int surfpitch, int dx, int dy);
static void	 Draw_Fill_Rectangle_XOR      (byte *surfdata, int surfpitch, int dx, int dy);
static void	 Draw_Fill_Rectangle_ADD      (byte *surfdata, int surfpitch, int dx, int dy);
static void	 Draw_Fill_Rectangle_SUBTRACT (byte *surfdata, int surfpitch, int dx, int dy);
static void	 Draw_Fill_Rectangle_SHL      (byte *surfdata, int surfpitch, int dx, int dy);
static void	 Draw_Fill_Rectangle_SHR      (byte *surfdata, int surfpitch, int dx, int dy);
static void	 Draw_Fill_Rectangle_MULTIPLY (byte *surfdata, int surfpitch, int dx, int dy);
void d3d9_Pixel_To_RGB (DWORD pixel, byte *r, byte *g, byte *b);
static void	 Draw_Line (int x1, int y1, int x2, int y2, byte *surfdata, int surfpitch);
static void Put_Image (
	byte *image,
	int   image_dx,
	int   image_x,
	int   image_y,
	int   x,
	int   y,
	int   dx,
	int   dy,
	int   or,
	byte *surfdata, 
	int   surfpitch ); 
static void	Get_Image (
	byte *image,
	int   image_dx,
	int   image_x,
	int   image_y,
	int   x,
	int   y,
	int   dx,
	int   dy,
	byte *surfdata, 
	int   surfpitch ); 
static void	Put_Bitmap (
	byte  *bitmap,
	int    bitmap_dx,		// in bits
	int    bitmap_x,
	int    bitmap_y,
	int    x,
	int    y,
	int    dx,					// size in bits of rectangle to transfer
	int    dy,
	DWORD  color,
	byte  *surfdata, 
	int    surfpitch ); 
static int Adjust_Cursor_Size (int dx, int dy, int *cursor_dx, int *cursor_dy);
static LPDIRECT3DSURFACE9 Allocate_Cursor_Surface (int dx, int dy);

/*____________________
|
| Macros
|___________________*/

#define THE_SURFACE ((LPDIRECT3DSURFACE9)surface)

#define FREE_CURSOR_SURFACE   \
  {                           \
    if (d3dcursor9) {         \
      d3dcursor9->Release (); \
      d3dcursor9 = 0;         \
    }                         \
  }

#define DRAW_PIXEL																	          \
  switch (d3d9_current_logic_op) {										        \
		case DD_OP_SET:																            \
			for (j=0; j<d3d9_pixel_size; j++)							          \
				surfdata[j] = ((byte *)&d3d9_current_color)[j];	      \
			break;																				          \
		case DD_OP_AND:																            \
			for (j=0; j<d3d9_pixel_size; j++)   						        \
				surfdata[j] &= ((byte *)&d3d9_current_color)[j];	    \
			break;																				          \
		case DD_OP_OR:																	          \
			for (j=0; j<d3d9_pixel_size; j++)							          \
				surfdata[j] |= ((byte *)&d3d9_current_color)[j];	    \
			break;																				          \
		case DD_OP_XOR:																            \
			for (j=0; j<d3d9_pixel_size; j++)							          \
				surfdata[j] ^= ((byte *)&d3d9_current_color)[j];	    \
			break;																				          \
		case DD_OP_ADD:																            \
			for (j=0; j<d3d9_pixel_size; j++)							          \
				surfdata[j] += ((byte *)&d3d9_current_color)[j];	    \
			break;																				          \
		case DD_OP_SUBTRACT:	  												          \
			for (j=0; j<d3d9_pixel_size; j++)							          \
				surfdata[j] -= ((byte *)&d3d9_current_color)[j];	    \
			break;																				          \
    case DD_OP_SHL:                                           \
      for (j=0; j<d3d9_pixel_size; j++)                       \
        surfdata[j] <<= ((byte *)&d3d9_current_color)[j];     \
      break;                                                  \
    case DD_OP_SHR:                                           \
      for (j=0; j<d3d9_pixel_size; j++)                       \
        surfdata[j] >>= ((byte *)&d3d9_current_color)[j];     \
      break;                                                  \
    case DD_OP_MULTIPLY:                                      \
      factor = (float)((d3d9_current_color & d3d9_REDmask) >> d3d9_loREDbit) / (float)100; \
      for (j=0; j<d3d9_pixel_size; j++)                       \
				surfdata[j] = (byte)((float)(surfdata[j]) * factor);  \
      break;                                                  \
	}

/*____________________
|
| Global variables
|___________________*/

// declared in d3d9_3d.cpp
extern int d3d_app_window_xleft;
extern int d3d_app_window_ytop;

/*___________________________________________________________________
|
|	Function: d3d9_Vertical_Retrace_Delay
| 
| Output: Waits for the start of the next vertical retrace period then
|   returns.
|___________________________________________________________________*/

void d3d9_Vertical_Retrace_Delay ()
{
  D3DRASTER_STATUS status;

  // If already in the retrace period, wait for next retrace
  do {
    d3ddevice9->GetRasterStatus (0, &status);
  } while (status.InVBlank);

  // Wait for the start of the next retrace
  do {
    d3ddevice9->GetRasterStatus (0, &status);
  } while (NOT status.InVBlank);
}

/*___________________________________________________________________
|
|	Function: d3d9_Set_Active_Surface
| 
|	Input: Called from ____
| Output: Sets the render surface.  If surface = 0, sets to screen, 
|   else sets to the surface given.
|___________________________________________________________________*/

void d3d9_Set_Active_Surface (byte *surface)
{
  HRESULT hres;

  // Set active surface to screen
  if (surface == 0) {
    hres = d3ddevice9->SetRenderTarget (0, d3dscreen9);
    if (hres == D3D_OK)
      hres = d3ddevice9->SetDepthStencilSurface (d3dzbuffer9);
  }
  // Set active surface to callers surface
  else
    hres = d3ddevice9->SetRenderTarget (0, (LPDIRECT3DSURFACE9)surface);

#ifdef _DEBUG
  if (hres != D3D_OK)
    DEBUG_ERROR ("d3d9_Set_Active_Surface(): ERROR calling d3ddevice9->SetRenderTarget()")
#endif
}

/*___________________________________________________________________
|
|	Function: d3d9_Set_Fore_Color
| 
|	Input: Called from ____
| Output: Sets the foreground drawing color to the rgb value.
|___________________________________________________________________*/

void d3d9_Set_Fore_Color (byte r, byte g, byte b, byte a)
{
  d3d9_current_color = d3d9_RGBA_To_Pixel (r, g, b, a);
}

/*___________________________________________________________________
|
|	Function: RGB_To_Pixel
| 
|	Input: Called from d3d9_Set_Fore_Color(), d3d9_Clear_Surface(),
|										 d3d9_Put_Bitmap()	
| Output: Converts separate RGB values into a single pixel with the same
|					pixel format as the current video mode.
|___________________________________________________________________*/

static DWORD RGB_To_Pixel (byte r, byte g, byte b)
{
	DWORD rgb_pixel = 0;

	// Convert a rgb color to a 16-bit rgb value
	if (d3d9_pixel_size == 2) 
    rgb_pixel = ((DWORD)r >> (8 - d3d9_numREDbits  ) << d3d9_loREDbit  ) |
                ((DWORD)g >> (8 - d3d9_numGREENbits) << d3d9_loGREENbit) |
                ((DWORD)b >> (8 - d3d9_numBLUEbits ) << d3d9_loBLUEbit );
	// Convert rgb color to a 24-bit rgb value (or 32-bit rgb value)
	else if ((d3d9_pixel_size == 3) OR (d3d9_pixel_size == 4))
		rgb_pixel = ((DWORD)r << d3d9_loREDbit) | ((DWORD)g << d3d9_loGREENbit) | ((DWORD) b <<d3d9_loBLUEbit);

	return (rgb_pixel);
}

/*___________________________________________________________________
|
|	Function: d3d9_RGBA_To_Pixel
| 
|	Input: Called from d3d9_SetTextureBorderColor()
| Output: Converts separate RGBA values into a single pixel with the same
|					pixel format as the current video mode.
|___________________________________________________________________*/

DWORD d3d9_RGBA_To_Pixel (byte r, byte g, byte b, byte a)
{
	DWORD rgba_pixel = 0;

	// Convert a rgba color to a 16-bit rgba value
	if (d3d9_pixel_size == 2) 
    rgba_pixel = ((DWORD)r >> (8 - d3d9_numREDbits  ) << d3d9_loREDbit  ) |
                 ((DWORD)g >> (8 - d3d9_numGREENbits) << d3d9_loGREENbit) |
                 ((DWORD)b >> (8 - d3d9_numBLUEbits ) << d3d9_loBLUEbit ) |
                 ((DWORD)a >> (8 - d3d9_numALPHAbits) << d3d9_loALPHAbit);
	// Convert rgba color to a 24-bit rgba value (or 32-bit rgba value)
	else if ((d3d9_pixel_size == 3) OR (d3d9_pixel_size == 4))
		rgba_pixel = ((DWORD)r << d3d9_loREDbit)   | 
                 ((DWORD)g << d3d9_loGREENbit) | 
                 ((DWORD)b << d3d9_loBLUEbit)  |
                 ((DWORD)a << d3d9_loALPHAbit);

	return (rgba_pixel);
}

/*___________________________________________________________________
|
|	Function: d3d9_Set_Logic_Op
| 
|	Input: Called from ____
| Output: Sets a new current logic operation for drawing.
|___________________________________________________________________*/

void d3d9_Set_Logic_Op (int logic_op)
{
	// Verify the new logic op is valid
	if ((logic_op == DD_OP_SET)      OR 
      (logic_op == DD_OP_AND)      OR 
      (logic_op == DD_OP_OR)       OR 
      (logic_op == DD_OP_XOR)      OR
      (logic_op == DD_OP_ADD)      OR
      (logic_op == DD_OP_SUBTRACT) OR
      (logic_op == DD_OP_SHL)      OR
      (logic_op == DD_OP_SHR)      OR
      (logic_op == DD_OP_MULTIPLY))
		d3d9_current_logic_op = logic_op;
}

/*___________________________________________________________________
|
|	Function: d3d9_Allocate_Surface
| 
|	Input: Called from ____
| Output: Creates a Direct3D renderable surface in video memory. The
|					surface created will have the same pixel depth as the current
|					video mode.  Returns a pointer to the surface or NULL on any error.
|___________________________________________________________________*/

byte *d3d9_Allocate_Surface (int width, int height)
{
  HRESULT hres;
  LPDIRECT3DSURFACE9 surface = 0;

  // A video mode must be active
	if (d3d9_current_video_mode != -1) {
    // Create the surface
    hres = d3ddevice9->CreateRenderTarget ((unsigned)width, (unsigned)height, SCREEN_FORMAT, D3DMULTISAMPLE_NONE, 0, TRUE, &surface, 0);
#ifdef _DEBUG  
    if (hres != D3D_OK)
      DEBUG_ERROR ("d3d9_Allocate_Surface(): ERROR calling d3ddevice9->CreateRenderTarget()")
#endif
  }

  return ((byte *)surface);
}

/*___________________________________________________________________
|
|	Function: d3d9_Free_Surface
| 
|	Input: Called from ____
| Output: Frees a surface created with _____
|___________________________________________________________________*/

void d3d9_Free_Surface (byte *surface)
{
  if (surface)
		((LPDIRECT3DSURFACE9)surface)->Release ();
}

/*___________________________________________________________________
|
|	Function: d3d9_Get_Surface_Dimensions
| 
|	Input: Called from ____
| Output: Returns in callers variables the dimensions of a surface.
|					Returns true if successful, else false on any error.
|___________________________________________________________________*/

int d3d9_Get_Surface_Dimensions (byte *surface, int *width, int *height)
{
	D3DSURFACE_DESC desc;
	int success = FALSE;

	if (surface) {
		ZeroMemory (&desc, sizeof(desc));
    if (((LPDIRECT3DSURFACE9)surface)->GetDesc (&desc) == D3D_OK) {
			*width  = (int)(desc.Width);
			*height = (int)(desc.Height);
			success = TRUE;
		}
	}

	return (success);
}

/*___________________________________________________________________
|
|	Function: d3d9_Clear_Surface
| 
|	Input: Called from d3d9_Set_Mode(), d3d9_Restore()
| Output:	Fills the rectangular area of a page to a rgb color.  Returns 
|					true if successful, else false on any error.
|___________________________________________________________________*/

void d3d9_Clear_Surface (byte r, byte g, byte b, byte *surface)
{
	int save_logic_op;
	DWORD save_color;

	save_logic_op		      = d3d9_current_logic_op;
	save_color			      = d3d9_current_color;
	d3d9_current_logic_op = DD_OP_SET;
	d3d9_current_color		= RGB_To_Pixel (r, g, b);

  d3d9_Draw_Fill_Rectangle (0, 0, SCREEN_DX-1, SCREEN_DY-1, surface);
	
  d3d9_current_logic_op = save_logic_op;
	d3d9_current_color		 = save_color;
}

/*___________________________________________________________________
|
|	Function: d3d9_Draw_Fill_Rectangle
| 
|	Input: Called from ____
| Output:	Fills the rectangular area of a surface with current color
|					using current logic operation.  Returns true if successful, 
|					else false on any error.
|___________________________________________________________________*/

int d3d9_Draw_Fill_Rectangle (int x1, int y1, int x2, int y2, byte *surface)
{
	int t;
  byte r, g, b;
  RECT rect;
  D3DRECT d3drect;
  D3DLOCKED_RECT locked_rect;
  D3DCOLOR color;
  LPDIRECT3DSURFACE9 tsurf;
	byte *surfdata;
  bool clearing_screen;
	int drawn = FALSE;

	// Init surface variable to active page? 
  if (surface == NULL) {
		surface = (byte *)d3dscreen9;
    clearing_screen = true;
  }
  else
    clearing_screen = false;
		
	if (surface) {
		// Build a rectangle describing the area to be drawn
		if (x2 < x1) {
			t = x2;
			x2 = x1;
			x1 = t;
		}
		if (y2 < y1) {
			t = y2;
			y2 = y1;
			y1 = t;
		}
		rect.left   = x1;
		rect.top    = y1;
		rect.right  = x2 + 1;	// Adjust since right,bottom is not inclusive
		rect.bottom = y2 + 1;

    d3drect.x1 = rect.left;
    d3drect.y1 = rect.top;
    d3drect.x2 = rect.right;
    d3drect.y2 = rect.bottom;

		// Draw the rectangle using replace operation?
		if (d3d9_current_logic_op == DD_OP_SET) {
      // Set color to draw
      d3d9_Pixel_To_RGB (d3d9_current_color, &r, &g, &b);
      color = (D3DCOLOR)r << 16 | (D3DCOLOR)g << 8 | (D3DCOLOR)b;
      // Clear the surface rectangle
      if (clearing_screen) {
			  if (d3ddevice9->Clear (1, &d3drect, D3DCLEAR_TARGET, color, 0, 0) == D3D_OK)
				  drawn = TRUE;
      }
      else {
        d3ddevice9->GetRenderTarget (0, &tsurf);
        d3ddevice9->SetRenderTarget (0, THE_SURFACE);
			  if (d3ddevice9->Clear (1, &d3drect, D3DCLEAR_TARGET, color, 0, 0) == D3D_OK)
				  drawn = TRUE;
        d3ddevice9->SetRenderTarget (0, tsurf);
      }
    }
		// Draw the rectangle using a logic operation?
		else {
			// Lock the surface area before accessing it
			if (THE_SURFACE->LockRect (&locked_rect, &rect, D3DLOCK_NOSYSLOCK) == D3D_OK) {
				// Draw the rectangle on the surface
				surfdata = SURFACE_BUFFER + (y1 * SURFACE_PITCH + x1 * d3d9_pixel_size);
				switch (d3d9_current_logic_op) {
					case DD_OP_AND: Draw_Fill_Rectangle_AND (surfdata, locked_rect.Pitch, x2-x1+1, y2-y1+1);
													break;
					case DD_OP_OR:	Draw_Fill_Rectangle_OR (surfdata, locked_rect.Pitch, x2-x1+1, y2-y1+1);
													break;
					case DD_OP_XOR: Draw_Fill_Rectangle_XOR (surfdata, locked_rect.Pitch, x2-x1+1, y2-y1+1);
													break;
          case DD_OP_ADD: Draw_Fill_Rectangle_ADD (surfdata, locked_rect.Pitch, x2-x1+1, y2-y1+1);
                          break;
          case DD_OP_SUBTRACT:
                          Draw_Fill_Rectangle_SUBTRACT (surfdata, locked_rect.Pitch, x2-x1+1, y2-y1+1);
                          break;
          case DD_OP_SHL: Draw_Fill_Rectangle_SHL (surfdata, locked_rect.Pitch, x2-x1+1, y2-y1+1);
                          break;
          case DD_OP_SHR: Draw_Fill_Rectangle_SHR (surfdata, locked_rect.Pitch, x2-x1+1, y2-y1+1);
                          break;
          case DD_OP_MULTIPLY: 
                          Draw_Fill_Rectangle_MULTIPLY (surfdata, locked_rect.Pitch, x2-x1+1, y2-y1+1);
                          break;
				}
		  	// Unlock the surface
	  		THE_SURFACE->UnlockRect ();
  			drawn = TRUE;
      }
		}
	}

  return (drawn);	
}

/*___________________________________________________________________
|
|	Function: Draw_Fill_Rectangle_AND
| 
|	Input: Called from d3d9_Draw_Fill_Rectangle()
| Output:	Fills the rectangular area of a surface with current color
|					using AND logic operation.  
|___________________________________________________________________*/

static void Draw_Fill_Rectangle_AND (byte *surfdata, int surfpitch, int dx, int dy)
{
	int x, y;

	switch (d3d9_pixel_size) {
		case 2: dx *= 2;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=2) {
								surfdata[x]   &= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] &= ((byte *)&d3d9_current_color)[1];
							}
							surfdata += surfpitch;
						}
				  	break;
    case 3: dx *= 3;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=3) {
								surfdata[x]   &= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] &= ((byte *)&d3d9_current_color)[1];
								surfdata[x+2] &= ((byte *)&d3d9_current_color)[2];
							}
							surfdata += surfpitch;
						}
            break;
    case 4: dx *= 4;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=4) 
                *(unsigned *)(surfdata+x) &= (unsigned)d3d9_current_color;
							surfdata += surfpitch;
						}
            break;
	}
}

/*___________________________________________________________________
|
|	Function: Draw_Fill_Rectangle_OR
| 
|	Input: Called from d3d9_Draw_Fill_Rectangle()
| Output:	Fills the rectangular area of a surface with current color
|					using OR logic operation.  
|___________________________________________________________________*/

static void Draw_Fill_Rectangle_OR (byte *surfdata, int surfpitch, int dx, int dy)
{
	int x, y;

	switch (d3d9_pixel_size) {
		case 2:	dx *= 2;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=2) {
								surfdata[x]   |= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] |= ((byte *)&d3d9_current_color)[1];
							}
							surfdata += surfpitch;
						}
				  	break;
    case 3: dx *= 3;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=3) {
								surfdata[x]   |= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] |= ((byte *)&d3d9_current_color)[1];
								surfdata[x+2] |= ((byte *)&d3d9_current_color)[2];
							}
							surfdata += surfpitch;
						}
            break;
    case 4: dx *= 4;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=4) 
                *(unsigned *)(surfdata+x) |= (unsigned)d3d9_current_color;
							surfdata += surfpitch;
						}
            break;
	}
}

/*___________________________________________________________________
|
|	Function: Draw_Fill_Rectangle_XOR
| 
|	Input: Called from d3d9_Draw_Fill_Rectangle()
| Output:	Fills the rectangular area of a surface with current color
|					using XOR logic operation.  
|___________________________________________________________________*/

static void Draw_Fill_Rectangle_XOR (byte *surfdata, int surfpitch, int dx, int dy)
{
	int x, y;

	switch (d3d9_pixel_size) {
		case 2:	dx *= 2;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=2) {
								surfdata[x]   ^= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] ^= ((byte *)&d3d9_current_color)[1];
							}
							surfdata += surfpitch;
						}
				  	break;
    case 3: dx *= 3;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=3) {
								surfdata[x]   ^= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] ^= ((byte *)&d3d9_current_color)[1];
								surfdata[x+2] ^= ((byte *)&d3d9_current_color)[2];
							}
							surfdata += surfpitch;
						}
            break;
    case 4: dx *= 4;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=4) 
                *(unsigned *)(surfdata+x) ^= (unsigned)d3d9_current_color;
							surfdata += surfpitch;
						}
            break;
	}
}

/*___________________________________________________________________
|
|	Function: Draw_Fill_Rectangle_ADD
| 
|	Input: Called from d3d9_Draw_Fill_Rectangle()
| Output:	Fills the rectangular area of a surface with current color
|					using ADD operation.  
|___________________________________________________________________*/

#define ADD_BYTE(_b1_,_b2_)           \
  {                                   \
    tmp = (int)(_b1_) + (int)(_b2_);  \
    if (tmp > 255)                    \
      (_b1_) = 255;                   \
    else                              \
      (_b1_) = (byte)tmp;             \
  }

static void Draw_Fill_Rectangle_ADD (byte *surfdata, int surfpitch, int dx, int dy)
{
	int x, y, tmp;

	switch (d3d9_pixel_size) {
		case 2:	dx *= 2;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=2) {
                ADD_BYTE (surfdata[x],   ((byte *)&d3d9_current_color)[0])
							  ADD_BYTE (surfdata[x+1], ((byte *)&d3d9_current_color)[1])
							}
							surfdata += surfpitch;
						}
				  	break;
    case 3: dx *= 3;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=3) {
								ADD_BYTE (surfdata[x],   ((byte *)&d3d9_current_color)[0])
								ADD_BYTE (surfdata[x+1], ((byte *)&d3d9_current_color)[1])
								ADD_BYTE (surfdata[x+2], ((byte *)&d3d9_current_color)[2])
							}
							surfdata += surfpitch;
						}
            break;
    case 4: dx *= 4;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=4) {
								ADD_BYTE (surfdata[x],   ((byte *)&d3d9_current_color)[0])
								ADD_BYTE (surfdata[x+1], ((byte *)&d3d9_current_color)[1])
								ADD_BYTE (surfdata[x+2], ((byte *)&d3d9_current_color)[2])
                ADD_BYTE (surfdata[x+3], ((byte *)&d3d9_current_color)[3])
							}
							surfdata += surfpitch;
						}
            break;
	}
}

#undef ADD_BYTE 

/*___________________________________________________________________
|
|	Function: Draw_Fill_Rectangle_SUBTRACT
| 
|	Input: Called from d3d9_Draw_Fill_Rectangle()
| Output:	Fills the rectangular area of a surface with current color
|					using SUBTRACT operation.  
|___________________________________________________________________*/

#define SUBTRACT_BYTE(_b1_,_b2_)      \
  {                                   \
    tmp = (int)(_b1_) - (int)(_b2_);  \
    if (tmp < 0)                      \
      (_b1_) = 0;                     \
    else                              \
      (_b1_) = (byte)tmp;             \
  }

static void Draw_Fill_Rectangle_SUBTRACT (byte *surfdata, int surfpitch, int dx, int dy)
{
	int x, y, tmp;

	switch (d3d9_pixel_size) {
		case 2:	dx *= 2;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=2) {
                SUBTRACT_BYTE (surfdata[x],   ((byte *)&d3d9_current_color)[0])
								SUBTRACT_BYTE (surfdata[x+1], ((byte *)&d3d9_current_color)[1])
							}
							surfdata += surfpitch;
						}
				  	break;
    case 3: dx *= 3;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=3) {
								SUBTRACT_BYTE (surfdata[x],   ((byte *)&d3d9_current_color)[0])
								SUBTRACT_BYTE (surfdata[x+1], ((byte *)&d3d9_current_color)[1])
								SUBTRACT_BYTE (surfdata[x+2], ((byte *)&d3d9_current_color)[2])
							}
							surfdata += surfpitch;
						}
            break;
    case 4: dx *= 4;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=4) {
								SUBTRACT_BYTE (surfdata[x],   ((byte *)&d3d9_current_color)[0])
								SUBTRACT_BYTE (surfdata[x+1], ((byte *)&d3d9_current_color)[1])
								SUBTRACT_BYTE (surfdata[x+2], ((byte *)&d3d9_current_color)[2])
                SUBTRACT_BYTE (surfdata[x+3], ((byte *)&d3d9_current_color)[3])
							}
							surfdata += surfpitch;
						}
            break;
	}
}

#undef SUBTRACT_BYTE

/*___________________________________________________________________
|
|	Function: Draw_Fill_Rectangle_SHL
| 
|	Input: Called from d3d9_Draw_Fill_Rectangle()
| Output:	Fills the rectangular area of a surface with current color
|					using Shift Left logic operation.  
|___________________________________________________________________*/

static void Draw_Fill_Rectangle_SHL (byte *surfdata, int surfpitch, int dx, int dy)
{
	int x, y;

	switch (d3d9_pixel_size) {
		case 2:	dx *= 2;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=2) {
								surfdata[x]   <<= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] <<= ((byte *)&d3d9_current_color)[1];
							}
							surfdata += surfpitch;
						}
				  	break;
    case 3: dx *= 3;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=3) {
								surfdata[x]   <<= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] <<= ((byte *)&d3d9_current_color)[1];
								surfdata[x+2] <<= ((byte *)&d3d9_current_color)[2];
							}
							surfdata += surfpitch;
						}
            break;
    case 4: dx *= 4;
						for (y=0; y<dy; y++) {
              for (x=0; x<dx; x+=4) {
								surfdata[x]   <<= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] <<= ((byte *)&d3d9_current_color)[1];
								surfdata[x+2] <<= ((byte *)&d3d9_current_color)[2];
								surfdata[x+3] <<= ((byte *)&d3d9_current_color)[3];
							}
							surfdata += surfpitch;
						}
            break;
	}
}

/*___________________________________________________________________
|
|	Function: Draw_Fill_Rectangle_SHR
| 
|	Input: Called from d3d9_Draw_Fill_Rectangle()
| Output:	Fills the rectangular area of a surface with current color
|					using Shift Right logic operation.  
|___________________________________________________________________*/

static void Draw_Fill_Rectangle_SHR (byte *surfdata, int surfpitch, int dx, int dy)
{
	int x, y;

	switch (d3d9_pixel_size) {
		case 2:	dx *= 2;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=2) {
								surfdata[x]   >>= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] >>= ((byte *)&d3d9_current_color)[1];
							}
							surfdata += surfpitch;
						}
				  	break;
    case 3: dx *= 3;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=3) {
								surfdata[x]   >>= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] >>= ((byte *)&d3d9_current_color)[1];
								surfdata[x+2] >>= ((byte *)&d3d9_current_color)[2];
							}
							surfdata += surfpitch;
						}
            break;
    case 4: dx *= 4;
						for (y=0; y<dy; y++) {
              for (x=0; x<dx; x+=4) {
								surfdata[x]   >>= ((byte *)&d3d9_current_color)[0];
								surfdata[x+1] >>= ((byte *)&d3d9_current_color)[1];
								surfdata[x+2] >>= ((byte *)&d3d9_current_color)[2];
								surfdata[x+3] >>= ((byte *)&d3d9_current_color)[3];
							}
							surfdata += surfpitch;
						}
            break;
	}
}

/*___________________________________________________________________
|
|	Function: Draw_Fill_Rectangle_MULTIPLY
| 
|	Input: Called from d3d9_Draw_Fill_Rectangle()
| Output:	Fills the rectangular area of a surface with current color
|					using a multiply operation.  
|
| Note: The current color red component is used as a multiplier.  The 
|   red value is divided by 100 and that fraction is used to mutiply
|   each pixel by.  For example if the red color = 10, then every pixel
|   in the rectangle would be multiplied by .1.
|
|   For 16-bit color this operation has no use.
|___________________________________________________________________*/

static void Draw_Fill_Rectangle_MULTIPLY (byte *surfdata, int surfpitch, int dx, int dy)
{
	int x, y;
  float factor = (float)((d3d9_current_color & d3d9_REDmask) >> d3d9_loREDbit) / (float)100;

	switch (d3d9_pixel_size) {
		case 2:	dx *= 2;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=2) {
                surfdata[x]   = (byte)((float)(surfdata[x])   * factor);
                surfdata[x+1] = (byte)((float)(surfdata[x+1]) * factor);
							}
							surfdata += surfpitch;
						}
				  	break;
    case 3: dx *= 3;
						for (y=0; y<dy; y++) {
							for (x=0; x<dx; x+=3) {
                surfdata[x]   = (byte)((float)(surfdata[x])   * factor);
                surfdata[x+1] = (byte)((float)(surfdata[x+1]) * factor);
                surfdata[x+2] = (byte)((float)(surfdata[x+2]) * factor);
							}
							surfdata += surfpitch;
						}
            break;
    case 4: dx *= 4;
						for (y=0; y<dy; y++) {
              for (x=0; x<dx; x+=4) {
                surfdata[x]   = (byte)((float)(surfdata[x])   * factor);
                surfdata[x+1] = (byte)((float)(surfdata[x+1]) * factor);
                surfdata[x+2] = (byte)((float)(surfdata[x+2]) * factor);
                surfdata[x+3] = (byte)((float)(surfdata[x+3]) * factor);
							}
							surfdata += surfpitch;
						}
            break;
	}
}

/*___________________________________________________________________
|
|	Function: d3d9_Draw_Pixel
| 
|	Input: Called from ____
| Output:	Draws a pixel on a surface using the current color.  Returns
|					true if successful, else false on any error.
|___________________________________________________________________*/

int d3d9_Draw_Pixel (int x, int y, byte *surface)
{
  D3DLOCKED_RECT locked_rect;
  byte *surfdata;
	int drawn = FALSE;

	// Init surface variable to active page? 
	if (surface == NULL) 
		surface = (byte *)d3dscreen9;
		
	if (surface) {
		// Lock the surface area before accessing it
		if (THE_SURFACE->LockRect (&locked_rect, NULL, D3DLOCK_NOSYSLOCK) == D3D_OK) {
			// Write the pixel to the surface
			surfdata = SURFACE_BUFFER + (y * SURFACE_PITCH + x * d3d9_pixel_size);
      if (d3d9_current_logic_op == DD_OP_SET)
        memcpy ((void *)surfdata, (void *)&d3d9_current_color, d3d9_pixel_size);
      else {
        int j;
        float factor = (float)((d3d9_current_color & d3d9_REDmask) >> d3d9_loREDbit) / (float)100;
        DRAW_PIXEL
      }
			// Unlock the surface
			THE_SURFACE->UnlockRect ();
			drawn = TRUE;
		}
	}

	return (drawn);	
}

/*___________________________________________________________________
|
|	Function: d3d9_Get_Pixel
| 
|	Input: Called from ____
| Output:	Returns the value of a pixel in callers variable.
|___________________________________________________________________*/

void d3d9_Get_Pixel (int x, int y, byte *r, byte *g, byte *b, byte *surface)
{
	DWORD pixel;
  D3DLOCKED_RECT locked_rect;
	byte *surfdata;

	// Init surface variable to active page? 
	if (surface == NULL) 
		surface = (byte *)d3dscreen9;
		
	if (surface) {
		// Lock the surface area before accessing it
		if (THE_SURFACE->LockRect (&locked_rect, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY) == D3D_OK) {
			// Read the pixel from the surface
			surfdata = SURFACE_BUFFER + (y * locked_rect.Pitch + x * d3d9_pixel_size);
      memcpy ((void *)&pixel, (void *)surfdata, d3d9_pixel_size);
			// Unlock the surface
			THE_SURFACE->UnlockRect ();
			// Convert the pixel to separate rgb values
			d3d9_Pixel_To_RGB (pixel, r, g, b);
		}
	}
}

/*___________________________________________________________________
|
|	Function: d3d9_Pixel_To_RGB
| 
|	Input: Called from d3d9_Get_Pixel()
| Output: Converts a pixel to separate RGB values.
|___________________________________________________________________*/

void d3d9_Pixel_To_RGB (DWORD pixel, byte *r, byte *g, byte *b)
{
	// Convert 16,24, or 32-bit rgb pixel to separate rgb values
	*r = (byte)((pixel & d3d9_REDmask)   >> d3d9_loREDbit  ) << (8 - d3d9_numREDbits);
	*g = (byte)((pixel & d3d9_GREENmask) >> d3d9_loGREENbit) << (8 - d3d9_numGREENbits);
	*b = (byte)((pixel & d3d9_BLUEmask)  >> d3d9_loBLUEbit ) << (8 - d3d9_numBLUEbits);
}

/*___________________________________________________________________
|
|	Function: d3d9_Draw_Line
| 
|	Input: Called from ____
| Output:	Draws a line on a surface with current color.  Returns true 
|					if successful, else false on any error.
|___________________________________________________________________*/

int d3d9_Draw_Line (int x1, int y1, int x2, int y2, byte *surface)
{
  D3DLOCKED_RECT locked_rect;
	int drawn = FALSE;

	// Init surface variable to active page? 
	if (surface == NULL) 
		surface = (byte *)d3dscreen9;
		
	if (surface) {
		// Lock the surface area before accessing it
		if (THE_SURFACE->LockRect (&locked_rect, NULL, D3DLOCK_NOSYSLOCK) == D3D_OK) {
			Draw_Line (x1, y1, x2, y2, SURFACE_BUFFER, locked_rect.Pitch);
			// Unlock the surface
			THE_SURFACE->UnlockRect ();
			drawn = TRUE;
		}
	}

  return (drawn);
}

/*___________________________________________________________________
|
|	Function: Draw_Line
| 
|	Input: Called from d3d9_Draw_Line()
| Output:	Draws a 1-pixel wide line on a surface with current color 
|					using current logic opeation.  Returns true if successful, 
|					else false on any error.
|___________________________________________________________________*/

void Draw_Line (int x1, int y1, int x2, int y2, byte *surfdata, int surfpitch)
{
  int i, j, t, error, dx, dy, dx2, dy2, dxy, yinc;
  float factor;

/*____________________________________________________________________
|
| Check for special cases of horizontal and vertical lines
|___________________________________________________________________*/

  // Draw a horizontal line? 
  if (y1 == y2) {
		// Draw left to right 
		if (x2 < x1) {
			t = x1;
			x1 = x2;
			x2 = t;
		}
    // Get address of first pixel 
    surfdata += ((y1 * surfpitch) + (x1 * d3d9_pixel_size));
		// Draw the line (1 special case possible)
		for (dx=x2-x1+1; dx; dx--) {
			DRAW_PIXEL
			surfdata += d3d9_pixel_size;
		}
	}
	// Draw a vertical line? 
  else if (x1 == x2) {
		// Draw top top to bottom
		if (y2 < y1) {
			t = y1;
			y1 = y2;
			y2 = t;
		}
    // Get address of first pixel 
    surfdata += ((y1 * surfpitch) + (x1 * d3d9_pixel_size));
		// Draw the line
		for (dy=y2-y1+1; dy; dy--) {
			DRAW_PIXEL
			surfdata += surfpitch;
		}
	}
	// Draw a diagonal line? 
  else {

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

    // Draw left to right 
    if (x2 < x1) {
      t = x1;
      x1 = x2;
      x2 = t;
      t = y1;
      y1 = y2;
      y2 = t;
    }
																									 
    dx   = x2 - x1;
    dx2  = dx * 2;

    if (y2 < y1) {
      dy   = y1 - y2;
      yinc = -1 * surfpitch;
    }
    else {
      dy   = y2 - y1;
      yinc = 1 * surfpitch;
    }
    dy2 = dy * 2;

    // Get address of first pixel 
    surfdata += ((y1 * surfpitch) + (x1 * d3d9_pixel_size));

/*____________________________________________________________________
|
| Draw diagonal line
|___________________________________________________________________*/

    if (dx >= dy) {
      error = dy2 - dx;
      dxy   = dy2 - dx2;
      for (i=0; i<=dx; i++) {
				// Draw pixel 
				DRAW_PIXEL
				// Increment to next pixel 
				surfdata += d3d9_pixel_size;
				if (error < 0)
					error += dy2;
				else {
					// Increment to next row 
					surfdata += yinc;
					error += dxy;
				}
      }
    }
    else {
      error = dx2 - dy;
      dxy   = dx2 - dy2;
      for (i=0; i<=dy; i++) {
				// Draw pixel 
				DRAW_PIXEL
				// Increment to next row 
				surfdata += yinc;
				if (error < 0)
					error += dx2;
				else {
					// Increment to next pixel 
					surfdata += d3d9_pixel_size;
					error += dxy;
				}
      }
    }
  }
}

#undef DRAW_PIXEL

/*___________________________________________________________________
|
|	Function: d3d9_Copy_Image
| 
|	Input: Called from ____
| Output:	Copies the rectangular area of pixels from one surface to 
|					another.  Returns true if successful, else false on any error.
|
|					No error checking or clipping is performed on the rectangle 
|					boundaries.  Caller must insure that all parameters are correct.
|___________________________________________________________________*/

int d3d9_Copy_Image (
	int		srcx,
	int		srcy,
	byte *srcsurface,
	int		dstx,
	int		dsty,
	byte *dstsurface,
	int		dx,
	int		dy )
{
	RECT srcrect, dstrect;
  int copied = FALSE;

/*___________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Init source surface to active page? 
	if (srcsurface == NULL) 
		srcsurface = (byte *)d3dscreen9;
  // Init destination surface to active page? 
	if (dstsurface == NULL) 
		dstsurface = (byte *)d3dscreen9;

/*___________________________________________________________________
|
| Copy the rectangle
|___________________________________________________________________*/

	if (srcsurface AND dstsurface) {
    // Set source rectangle to transfer
		srcrect.left   = srcx;
		srcrect.top    = srcy;
		srcrect.right  = srcx + dx - 1;
		srcrect.bottom = srcy + dy - 1;
		// Adjust rectangle since right,bottom is not inclusive
		srcrect.right++;
		srcrect.bottom++;

    // Set destination rectangle
    dstrect.left   = dstx;
    dstrect.top    = dsty;
    dstrect.right  = dstx + dx - 1;
    dstrect.bottom = dsty + dy - 1;
		// Adjust rectangle since right,bottom is not inclusive
		dstrect.right++;
		dstrect.bottom++;

    // Copy the rectangle
    if (D3DXLoadSurfaceFromSurface ((LPDIRECT3DSURFACE9)dstsurface, 0, &dstrect,
                                    (LPDIRECT3DSURFACE9)srcsurface, 0, &srcrect,
                                    D3DX_FILTER_NONE, 0) == D3D_OK)
	  	copied = TRUE;
  }

	return (copied);
}

/*___________________________________________________________________
|
|	Function: d3d9_Copy_Image_ColorKey
| 
|	Input: Called from ____
| Output:	Copies the rectangular area of pixels from one surface to 
|					another.  Any pixels in source image that are the key color
|         are not copied.
|___________________________________________________________________*/

int d3d9_Copy_Image_ColorKey (
	int		srcx,
	int		srcy,
	byte *srcsurface,
	int		dstx,
	int		dsty,
	byte *dstsurface,
	int		dx,
	int		dy,
  byte  r,    // key color (if or'ing image from src to dst)
  byte  g,
  byte  b )
{
	RECT srcrect, dstrect;
  D3DCOLOR color;
  int copied = FALSE;

/*___________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Init source surface to active page? 
	if (srcsurface == NULL) 
		srcsurface = (byte *)d3dscreen9;
  // Init destination surface to active page? 
	if (dstsurface == NULL) 
		dstsurface = (byte *)d3dscreen9;

/*___________________________________________________________________
|
| Copy the rectangle
|___________________________________________________________________*/

	if (srcsurface AND dstsurface) {
    // Set source rectangle to transfer
		srcrect.left   = srcx;
		srcrect.top    = srcy;
		srcrect.right  = srcx + dx - 1;
		srcrect.bottom = srcy + dy - 1;
		// Adjust rectangle since right,bottom is not inclusive
		srcrect.right++;
		srcrect.bottom++;

    // Set destination rectangle
    dstrect.left   = dstx;
    dstrect.top    = dsty;
    dstrect.right  = dstx + dx - 1;
    dstrect.bottom = dsty + dy - 1;
		// Adjust rectangle since right,bottom is not inclusive
		dstrect.right++;
		dstrect.bottom++;

    // Set the color key (a 32-bit ARGB color)
    color = D3DCOLOR_ARGB((unsigned)255, (unsigned)r, (unsigned)g, (unsigned)b);

    // Copy the rectangle
    if (D3DXLoadSurfaceFromSurface ((LPDIRECT3DSURFACE9)dstsurface, 0, &dstrect,
                                    (LPDIRECT3DSURFACE9)srcsurface, 0, &srcrect,
                                    D3DX_FILTER_NONE, color) == D3D_OK)
	  	copied = TRUE;
  }

	return (copied);
}

/*___________________________________________________________________
|
|	Function: d3d9_Put_Image
| 
|	Input: Called from ____
| Output:	Copies the rectangular area of an image in system memory to 
|					a surface.  If or operation specified, copies only non-zero
|					pixels.  Returns true if successful, else false on any error.
|
|					No error checking or clipping is performed on the rectangle 
|					boundaries.  Caller must insure that all parameters are correct.
|___________________________________________________________________*/

int d3d9_Put_Image (
	byte *image,
	int   image_dx,		// in pixels
	int   image_x,		// top-left coordinates in image
	int   image_y,
	int   x,					// top-left coordinates of destination
	int   y,
	int   dx,					// size in pixels of rectangle to transfer
	int   dy,
	int   or,					// boolean
	byte *surface )
{
  D3DLOCKED_RECT locked_rect;
  int copied = FALSE;

	// Init surface variable to active page? 
	if (surface == NULL) 
		surface = (byte *)d3dscreen9;
		
	if (surface) {
		// Lock the surface area before accessing it
		if (THE_SURFACE->LockRect (&locked_rect, NULL, D3DLOCK_NOSYSLOCK) == D3D_OK) {
			Put_Image (image, image_dx, image_x, image_y, x, y, dx, dy, or, SURFACE_BUFFER, SURFACE_PITCH);
			// Unlock the surface
			THE_SURFACE->UnlockRect ();
			copied = TRUE;
		}
    else
      DEBUG_ERROR ("d3d9_Put_Image(): error locking surface")
	}
  else
    DEBUG_ERROR ("d3d9_Put_Image(): error, surface is NULL")

  return (copied);
}

/*___________________________________________________________________
|
|	Function: Put_Image
| 
|	Input: Called from d3d9_Put_Image()
| Output:	Copies the rectangular area of an image in system memory to 
|					a surface.  If or operation specified, copies only non-zero
|					pixels. 
|
|					No error checking or clipping is performed on the rectangle 
|					boundaries.  Caller must insure that all parameters are correct.
|___________________________________________________________________*/

static void Put_Image (
	byte  *image,
	int    image_dx,
	int    image_x,
	int    image_y,
	int    x,
	int    y,
	int    dx,
	int    dy,
	int    or,
	byte  *surfdata, 
	int    surfpitch )
{
	int image_dx_bytes, dx_bytes;
	byte *imagedataptr, *surfdataptr, b1, b2, b3, b4;

/*___________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

	image_dx_bytes = image_dx * d3d9_pixel_size;
	dx_bytes = dx * d3d9_pixel_size;

	// Get address of first pixel in surface
  surfdata += ((y * surfpitch) + (x * d3d9_pixel_size));
	// Get address of first pixel in image
	image += (image_y * image_dx_bytes) + (image_x * d3d9_pixel_size);

/*___________________________________________________________________
|
| Copy the image to the surface
|___________________________________________________________________*/

	// Copy the data using a replace operation
	if (NOT or) {
		for (y=0; y<dy; y++) {
			surfdataptr  = surfdata;
			imagedataptr = image;
			// Copy 1 pixel at a time
			for (x=0; x<dx_bytes; x++)
				*surfdataptr++ = *imagedataptr++;
			surfdata  += surfpitch;
			image			+= image_dx_bytes;
		}
	}
	// Copy the data using an OR operation
	else {
		switch (d3d9_pixel_size) {
			case 2:	for (y=0; y<dy; y++) {
								surfdataptr  = surfdata;
								imagedataptr = image;
								// Copy 1 pixel at a time
								for (x=0; x<dx; x++) {
									b1 = *imagedataptr++;
									b2 = *imagedataptr++;
									if (b1 OR b2) {
										*(surfdataptr  ) = b1;
										*(surfdataptr+1) = b2;
									}
                  surfdataptr += 2;
								}
								surfdata  += surfpitch;
								image			+= image_dx_bytes;
							}
							break;	
			case 3:	for (y=0; y<dy; y++) {
								surfdataptr  = surfdata;
								imagedataptr = image;
								// Copy 1 pixel at a time
								for (x=0; x<dx; x++) {
									b1 = *imagedataptr++;
									b2 = *imagedataptr++;
									b3 = *imagedataptr++;
									if (b1 OR b2 OR b3) {
										*(surfdataptr  ) = b1;
										*(surfdataptr+1) = b2;
										*(surfdataptr+2) = b3;
									}
                  surfdataptr += 3;
								}
								surfdata  += surfpitch;
								image			+= image_dx_bytes;
							}
							break;	
			case 4:	for (y=0; y<dy; y++) {
								surfdataptr  = surfdata;
								imagedataptr = image;
								// Copy 1 pixel at a time
								for (x=0; x<dx; x++) {
									b1 = *imagedataptr++;
									b2 = *imagedataptr++;
									b3 = *imagedataptr++;
                  b4 = *imagedataptr++;
									if (b1 OR b2 OR b3 OR b4) {
										*(surfdataptr  ) = b1;
										*(surfdataptr+1) = b2;
										*(surfdataptr+2) = b3;
                    *(surfdataptr+3) = b4;
									}
                  surfdataptr += 4;
								}
								surfdata  += surfpitch;
								image			+= image_dx_bytes;
							}
							break;	
		}
	}
}

/*___________________________________________________________________
|
|	Function: d3d9_Get_Image
| 
|	Input: Called from ____
| Output:	Copies the rectangular area of a surface to an image buffer in
|					system memory.  Returns true if successful, else false on any 
|					error.
|
|					No error checking or clipping is performed on the rectangle 
|					boundaries.  Caller must insure that all parameters are correct.
|___________________________________________________________________*/

int d3d9_Get_Image (
	byte *image,
	int   image_dx,		// in pixels
	int   image_x,		// top-left coordinates in image
	int   image_y,
	int   x,					// top-left coordinates of destination
	int   y,
	int   dx,					// size in pixels of rectangle to transfer
	int   dy,
	byte *surface )
{
  D3DLOCKED_RECT locked_rect;
  int copied = FALSE;

  // Init surface variable to active page? 
  if (surface == NULL)
    surface = (byte *)d3dscreen9;

	if (surface) {
		// Lock the surface area before accessing it
		if (THE_SURFACE->LockRect (&locked_rect, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY) == D3D_OK) {
      Get_Image (image, image_dx, image_x, image_y, x, y, dx, dy, SURFACE_BUFFER, SURFACE_PITCH);
			// Unlock the surface
			THE_SURFACE->UnlockRect ();
			copied = TRUE;
		}
    else
      DEBUG_ERROR ("d3d9_Get_Image(): error locking surface")
	}
  else
    DEBUG_ERROR ("d3d9_Get_Image(): error, surface is NULL")

  return (copied);
}

/*___________________________________________________________________
|
|	Function: Get_Image
| 
|	Input: Called from d3d9_Get_Image()
| Output:	Copies the rectangular area of a surface to an image buffer in
|					system memory.  Returns true if successful, else false on any 
|					error.
|
|					No error checking or clipping is performed on the rectangle 
|					boundaries.  Caller must insure that all parameters are correct.
|___________________________________________________________________*/

static void Get_Image (
	byte  *image,
	int    image_dx,
	int    image_x,
	int    image_y,
	int    x,
	int    y,
	int    dx,
	int    dy,
	byte  *surfdata, 
	int    surfpitch ) 
{
	int image_dx_bytes, dx_bytes;
	byte *imagedataptr, *surfdataptr;

/*___________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

	image_dx_bytes = image_dx * d3d9_pixel_size;
	dx_bytes = dx * d3d9_pixel_size;

	// Get address of first pixel in surface
  surfdata += ((y * surfpitch) + (x * d3d9_pixel_size));
	// Get address of first pixel in image
	image += (image_y * image_dx_bytes) + (image_x * d3d9_pixel_size);

/*___________________________________________________________________
|
| Copy the vram memory to the image
|___________________________________________________________________*/

	// Copy the data
	for (y=0; y<dy; y++) {
		surfdataptr  = surfdata;
		imagedataptr = image;
		// Copy 1 pixel at a time
    for (x=0; x<dx_bytes; x++) 
			*imagedataptr++ = *surfdataptr++;
		surfdata += surfpitch;
		image    += image_dx_bytes;
	}
}

/*___________________________________________________________________
|
|	Function: d3d9_Put_Bitmap
| 
|	Input: Called from ____
| Output:	Copies the rectangular area of a bitmap in system memory to 
|					a surface.  Returns true if successful, else false on any error.
|
|					No error checking or clipping is performed on the rectangle 
|					boundaries.  Caller must insure that all parameters are correct.
|___________________________________________________________________*/

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
	byte *surface )
{
  D3DLOCKED_RECT locked_rect;
  int copied = FALSE;

	// Init surface variable to active page? 
	if (surface == NULL) 
		surface = (byte *)d3dscreen9;
		
	if (surface) {
		// Lock the surface before accessing it
		if (THE_SURFACE->LockRect (&locked_rect, NULL, D3DLOCK_NOSYSLOCK) == D3D_OK) {
      Put_Bitmap (bitmap, bitmap_dx, bitmap_x, bitmap_y, x, y, dx, dy, RGB_To_Pixel(r,g,b), SURFACE_BUFFER, SURFACE_PITCH);
			// Unlock the surface
			THE_SURFACE->UnlockRect ();
			copied = TRUE;
		}
    else
      DEBUG_ERROR ("d3d9_Put_Bitmap(): error locking surface")
	}
  else
    DEBUG_ERROR ("d3d9_Put_Bitmap(): error, surface is NULL")

  return (copied);
}

/*___________________________________________________________________
|
|	Function: Put_Bitmap
| 
|	Input: Called from d3d9_Put_Bitmap()
| Output:	Copies the rectangular area of a bitmap in system memory to 
|					a surface. 
|
|					No error checking or clipping is performed on the rectangle 
|					boundaries.  Caller must insure that all parameters are correct.
|
|	Description: bitmap buffer consists of one bit per pixel.  Only '1'
|					bits are drawn to the surface in the color.  '0' bits are
|					ignored and not drawn.
|___________________________________________________________________*/

#define DRAW_BITMAP_PIXEL					\
	switch (d3d9_pixel_size) {			\
		case 2: surfdata[x*2]		= b1;	\
						surfdata[x*2+1] = b2;	\
						break;								\
		case 3:	surfdata[x*3]   = b1;	\
						surfdata[x*3+1] = b2;	\
						surfdata[x*3+2] = b3;	\
						break;								\
		case 4:	surfdata[x*4]   = b1;	\
						surfdata[x*4+1] = b2;	\
						surfdata[x*4+2] = b3;	\
            surfdata[x*4+3] = b4; \
						break;								\
  }

static void Put_Bitmap (
	byte  *bitmap,
	int    bitmap_dx,		// in bits
	int    bitmap_x,
	int    bitmap_y,
	int    x,
	int    y,
	int    dx,					// size in bits of rectangle to transfer
	int    dy,
	DWORD  color,
	byte  *surfdata, 
	int    surfpitch )
{
	int i, bitmap_dx_bytes, dx_bytes, n, shift_left;
	byte b1, b2, b3, b4;
  dword mask, bits;

/*___________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

	// Get # bytes per row in the bitmap
	bitmap_dx_bytes = (bitmap_dx+7)/8;
	// Get # bytes per row in destination surface to write
	dx_bytes = dx * d3d9_pixel_size;

  // Compute shift left alignment
  shift_left = bitmap_x % 8;

	// Get each color byte in color (in case > 1 byte per pixel)
	b1 = ((byte *)&color)[0];
	b2 = ((byte *)&color)[1];
	b3 = ((byte *)&color)[2];
  b4 = ((byte *)&color)[3];

	// Get address of first pixel in surface
  surfdata += ((y * surfpitch) + (x * d3d9_pixel_size));
	// Get address of first pixel in bitmap
	bitmap += (bitmap_y * bitmap_dx_bytes) + (bitmap_x / 8);

/*___________________________________________________________________
|
| Copy bitmap to surface
|___________________________________________________________________*/
				
	// Transfer the data
	if (shift_left == 0) {
		for (y=0; y<dy; y++) {
			n = 0;
			for (x=0; x<dx;) {
				// Get 16 bits (max) to process
				bits = (bitmap[n] << 8) | bitmap[n+1];
				mask = 0x8000;
				for (i=0; (i<16) AND (x<dx); x++,i++) {
					if (bits & mask)
						DRAW_BITMAP_PIXEL
					mask = mask >> 1;
				}
				n += 2;
			}
			surfdata += surfpitch;
			bitmap	 += bitmap_dx_bytes;
		}
	}
	else {
		for (y=0; y<dy; y++) {
			n = 0;
			for (x=0; x<dx; ) {
				// Get 16 bits (max) to process
				bits = (bitmap[n] << 16) | (bitmap[n+1] << 8) | bitmap[n+2];
				bits = bits >> (8 - shift_left);
				mask = 0x8000;
				for (i=0; (i<16) AND (x<dx); x++,i++) {
					if (bits & mask)
						DRAW_BITMAP_PIXEL
					mask = mask >> 1;
				}
				n += 2;
			}
			surfdata += surfpitch;
			bitmap   += bitmap_dx_bytes;
		}
	}
}

/*__________________________________________________________________
|
|	Function: d3d9_Set_Image_Cursor
| 
|	Input: Called from dinput8_Set_Image_Cursor()
| Output:	Sets the cursor to the input image.
|___________________________________________________________________*/

void d3d9_Set_Image_Cursor (
	byte *image, 
	int		image_dx, 
	int		image_dy, 
	int		hot_x,                                  
	int		hot_y )
{
  int x, y, cursor_dx, cursor_dy;
  DWORD dwpixel;
  byte *temp_image;

/*___________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Make sure cursor size will be a power of 2
  Adjust_Cursor_Size (image_dx, image_dy, &cursor_dx, &cursor_dy);

/*___________________________________________________________________
|
| Build new mouse cursor surface
|___________________________________________________________________*/
		
  // Free previous mouse cursor, if any
  FREE_CURSOR_SURFACE
	// Build new mouse cursor surfaces
  d3dcursor9 = Allocate_Cursor_Surface (cursor_dx, cursor_dy);
  if (d3dcursor9) {
    // Allocate space for a temp image
    temp_image = (byte *) calloc (cursor_dx * cursor_dy, d3d9_pixel_size);
    // On error, release cursor surface
    if (temp_image == NULL) 
      FREE_CURSOR_SURFACE
    else {
      // Set alpha as needed in the temp image
      for (y=0; y<image_dy; y++)
        for (x=0; x<image_dx; x++) {
          dwpixel = ((DWORD *)image)[y*image_dx+x];
          if (dwpixel)
            dwpixel |= 0xFF000000;
          ((DWORD *)temp_image)[y*cursor_dx+x] = dwpixel;
        }
			d3d9_Put_Image (temp_image, image_dx, 0, 0, 0, 0, image_dx, image_dy, 0, (byte *)d3dcursor9);
      free (temp_image);
    }
  }

/*___________________________________________________________________
|
| Set the new cursor
|___________________________________________________________________*/
    
  if (d3dcursor9) 
    if (d3ddevice9->SetCursorProperties ((unsigned)hot_x, (unsigned)hot_y, d3dcursor9) != D3D_OK) 
      FREE_CURSOR_SURFACE

#ifdef _DEBUG
  if (d3dcursor9 == 0) {
    char str[256];
    sprintf (str, "d3d9_Set_Image_Cursor(): ERROR creating %dx%d cursor", image_dx, image_dy);
    DEBUG_ERROR (str)
  }  
#endif
}

/*__________________________________________________________________
|
|	Function: d3d9_Set_Bitmap_Cursor
| 
|	Input: Called from dinput8_Set_Bitmap_Cursor()
| Output:	Sets the cursor to the input bitmap.
|___________________________________________________________________*/

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
  byte  mask_color_b )
{
  int cursor_dx, cursor_dy;
  D3DCOLOR pixel;
  byte *temp_image;

/*___________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  // Make sure cursor size will be a power of 2
  Adjust_Cursor_Size (bitmap_dx, bitmap_dy, &cursor_dx, &cursor_dy);

/*___________________________________________________________________
|
| Build new mouse cursor surface
|___________________________________________________________________*/

  // Free previous mouse cursor, if any
  FREE_CURSOR_SURFACE
	// Build new mouse cursor surfaces
  d3dcursor9 = Allocate_Cursor_Surface (cursor_dx, cursor_dy);
  if (d3dcursor9) {
    // Allocate space for a temp image
    temp_image = (byte *) calloc (cursor_dx * cursor_dy, d3d9_pixel_size);
    // On error, release cursor surface
    if (temp_image == NULL) 
      FREE_CURSOR_SURFACE
    else {
      // Set alpha as needed in the temp image
      pixel = D3DCOLOR_ARGB ((unsigned)255,(unsigned)mask_color_r,(unsigned)mask_color_g,(unsigned)mask_color_b);
      Put_Bitmap (mask_bitmap, bitmap_dx, 0, 0, 0, 0, bitmap_dx, bitmap_dy, pixel, temp_image, cursor_dx * d3d9_pixel_size);
      pixel = D3DCOLOR_ARGB ((unsigned)255,(unsigned)cursor_color_r,(unsigned)cursor_color_g,(unsigned)cursor_color_b);
      Put_Bitmap (cursor_bitmap, bitmap_dx, 0, 0, 0, 0, cursor_dx, cursor_dy, pixel, temp_image, cursor_dx * d3d9_pixel_size);
      Direct3D_Put_Image (temp_image, cursor_dx, 0, 0, 0, 0, bitmap_dx, bitmap_dy, 0, (byte *)d3dcursor9);
      free (temp_image);
    }
  }

/*___________________________________________________________________
|
| Set the new cursor
|___________________________________________________________________*/
    
  if (d3dcursor9) 
    if (d3ddevice9->SetCursorProperties ((unsigned)hot_x, (unsigned)hot_y, d3dcursor9) != D3D_OK) 
      FREE_CURSOR_SURFACE

#ifdef _DEBUG
  if (d3dcursor9 == 0) {
    char str[256];
    sprintf (str, "d3d9_Set_Bitmap_Cursor(): ERROR creating %dx%d cursor", bitmap_dx, bitmap_dy);
    DEBUG_ERROR (str)
  }  
#endif
}

/*__________________________________________________________________
|
|	Function: Adjust_Cursor_Size
| 
|	Input: Called from d3d9_Set_Image_Cursor(), d3d9_Set_Bitmap_Cursor()
| Output:	Adjusts cursor size if necessary.  Returns true if valid size
|   has been chosen for the cursor, else false if no valid size chosen.
|___________________________________________________________________*/

static int Adjust_Cursor_Size (int dx, int dy, int *cursor_dx, int *cursor_dy)
{
  int i, valid;
  static int valid_size [] = {
    1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
    0 // last entry in this table must be zero
  };

  // Init cursor width,height
  *cursor_dx = 0;
  *cursor_dy = 0;

  // Set cursor width
  for (i=0; valid_size[i]; i++)
    if (dx <= valid_size[i]) {
      *cursor_dx = valid_size[i];
      break;
    }

  // Set cursor height
  for (i=0; valid_size[i]; i++)
    if (dy <= valid_size[i]) {
      *cursor_dy = valid_size[i];
      break;
    }

  // Make the cursor square
  //if (*cursor_dx < *cursor_dy)
  //  *cursor_dx = *cursor_dy;
  //else if (*cursor_dy < *cursor_dx)
  //  *cursor_dy = *cursor_dx;

  if (*cursor_dx AND *cursor_dy)
    valid = TRUE;
  else
    valid = FALSE;

  return (valid);
}

/*__________________________________________________________________
|
|	Function: Allocate_Cursor_Surface
| 
|	Input: Called from d3d9_Set_Image_Cursor(), 
|                    d3d9_Set_Bitmap_Cursor()
|
| Output:	Allocates memory for a surface to be loaded with a mouse 
|   cursor.  Use d3d9_Free_Surface() or Release() to free this memory 
|   when done.
|___________________________________________________________________*/

static LPDIRECT3DSURFACE9 Allocate_Cursor_Surface (int dx, int dy)
{    
  HRESULT hres;
  LPDIRECT3DSURFACE9 surface = 0;

  hres = d3ddevice9->CreateRenderTarget ((unsigned)dx, (unsigned)dy, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &surface, 0);
#ifdef _DEBUG  
  if (hres != D3D_OK)
    DEBUG_ERROR ("d3d9_Allocate_Cursor_Surface(): ERROR calling d3ddevice9->CreateRenderTarget()")
#endif

  return (surface);
}

/*__________________________________________________________________
|
|	Function: d3d9_Free_Cursor
| 
|	Input: Called from dinput8.cpp
| Output:	Frees the current cursor, if any.
|___________________________________________________________________*/

void d3d9_Free_Cursor ()
{
  FREE_CURSOR_SURFACE
}

/*__________________________________________________________________
|
|	Function: d3d9_Set_Cursor_Position
| 
|	Input: Called from Mouse_Thread(), 
|                    dinput8_Mouse_Set_Coords(), 
|                    dinput8_Mouse_Confine()
|
| Output:	Sets mouse cursor position in screen coordinates.
|___________________________________________________________________*/

void d3d9_Set_Cursor_Position (unsigned x, unsigned y)
{
  d3ddevice9->SetCursorPosition (x+d3d_app_window_xleft, y+d3d_app_window_ytop, D3DCURSOR_IMMEDIATE_UPDATE);
}

/*__________________________________________________________________
|
|	Function: d3d9_Show_Cursor
| 
|	Input: Called from dinput8_Mouse_Hide(), dinput8_Mouse_Show()
| Output:	Shows or hides mouse cursor.
|___________________________________________________________________*/

void d3d9_Show_Cursor (int flag)
{
  d3ddevice9->ShowCursor (flag);
}
