/*___________________________________________________________________
|
|	File: dx9.cpp
|
|	Description: Interface to DirectX 9.
|
| Functions:	dx9_GetUserFormat
|             dx9_Init
|							dx9_Free
|             dx9_VertRetraceDelay
|							dx9_Restore
|							dx9_SetActivePage
|							dx9_FlipVisualPage
|             dx9_SetForeColor
|							dx9_SetLogicOp
|							dx9_DrawPixel
|             dx9_GetPixel
|							dx9_DrawLine
|							dx9_DrawFillRectangle
|							dx9_PutImage
|							dx9_GetImage
|							dx9_CopyImage
|             dx9_CopyImageColorKey
|             dx9_PutBitmap
|             dx9_CreateVirtualPage
|             dx9_FreeVirtualPage
|
|             dx9_BeginRender
|             dx9_EndRender   
|             dx9_SetFillMode
|             dx9_GetDriverInfo
|             dx9_RegisterObject
|             dx9_UnregisterObject
|             dx9_DrawObject
|             dx9_OptimizeObject
|             dx9_SetViewport
|             dx9_ClearViewportRectangle
|             dx9_EnableClipping
|             dx9_InitClipPlane
|             dx9_FreeClipPlane
|             dx9_EnableClipPlane
|             dx9_SetWorldMatrix
|             dx9_GetWorldMatrix
|             dx9_SetViewMatrix
|             dx9_GetViewMatrix
|             dx9_SetProjectionMatrix
|             dx9_GetProjectionMatrix
|             dx9_EnableTextureMatrix
|             dx9_SetTextureMatrix
|             dx9_GetTextureMatrix
|             dx9_EnableZBuffer
|             dx9_EnableBackfaceRemoval
|
|             dx9_EnableStencilBuffer
|             dx9_SetStencilFailOp
|             dx9_SetStencilZFailOp
|             dx9_SetStencilPassOp
|             dx9_SetStencilComparison
|             dx9_SetStencilReferenceValue
|             dx9_SetStencilMask
|             dx9_SetStencilWriteMask
|
|             dx9_EnableLighting
|             dx9_InitPointLight
|              Init_Light
|             dx9_InitSpotLight
|             dx9_InitDirectionLight
|             dx9_FreeLight
|             dx9_EnableLight
|             dx9_SetAmbientLight
|             dx9_EnableSpecularLighting
|             dx9_EnableVertexLighting
|
|             dx9_EnableFog
|             dx9_SetFogColor
|             dx9_SetLinearPixelFog
|             dx9_SetExpPixelFog
|             dx9_SetExp2PixelFog
|             dx9_SetLinearVertexFog
|
|             dx9_SetMaterial
|             dx9_GetMaterial
|
|             dx9_InitTexture
|             dx9_InitVolumeTexture
|             dx9_InitCubemapTexture
|             dx9_InitDynamicTexture
|             dx9_InitDynamicCubemapTexture
|              Init_Dynamic_Texture
|               Add_Dynamic_Texture_To_Page_List
|             dx9_FreeTexture
|             dx9_FreeDynamicTexture
|              Free_Dynamic_Texture
|               Remove_Dynamic_Texture_From_Page_List
|             dx9_SetTexture
|             dx9_SetDynamicTexture
|             dx9_SetTextureAddressingMode
|             dx9_SetTextureBorderColor
|             dx9_SetTextureFiltering
|             dx9_SetTextureCoordinates
|             dx9_SetTextureCoordinateWrapping
|             dx9_SetTextureFactor
|             dx9_PreLoadTexture
|             dx9_EvictAllTextures
|             dx9_EnableRenderToTexture
|             dx9_SetTextureColorOp
|             dx9_SetTextureAlphaOp
|             dx9_SetTextureColorFactor
|             dx9_EnableCubemapTextureReflections
|
|             dx9_EnableAlphaBlending
|             dx9_SetAlphaBlendFactor
|             dx9_AlphaTestingAvailable
|             dx9_EnableAlphaTesting
|
|							dx9_StartEvents
|							dx9_StopEvents
|							dx9_FlushEvents
|							dx9_GetEvent
|
|							dx9_MouseFlushBuffer
|							dx9_MouseHide
|							dx9_MouseShow
|							dx9_MouseConfine
|							dx9_MouseGetStatus
|							dx9_MouseSetCoords
|							dx9_MouseGetCoords
|							dx9_MouseGetMovement
|             dx9_MouseSetBitmapCursor
|							dx9_MouseSetImageCursor
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*____________________
|
| Include files
|___________________*/

#include <first_header.h>

#include <windows.h>
#include <winbase.h>

#include <stdio.h>
#include <string.h>

#include <defines.h>
#include <events.h>
#include <clib.h>

#include "dinput9.h"
#include "d3d9_2d.h"
#include "d3d9_3d.h"

#include "dx9.h"

/*____________________
|
|	Type definitions
|___________________*/

typedef d3d9_Object dx9_Object;

typedef enum {none = 0, point = 1, spot = 2, direction = 3} LightType;

typedef struct {
  LightType type; // 0 = none
  int       on;   // boolean
  float     src_x;
  float     src_y;
  float     src_z;
  float     dst_x;
  float     dst_y;
  float     dst_z;
  float     range;
  float     constant_attenuation;
  float     linear_attenuation;
  float     quadratic_attenuation; 
  float     inner_cone_angle;
  float     outer_cone_angle;
  float     falloff;         
  float     ambient_color_rgba[4];
  float     diffuse_color_rgba[4];
  float     specular_color_rgba[4];
} Light;

/*____________________
|
|	Function prototypes
|___________________*/

static int Init_Light (int index, Light *light);
static int Init_Dynamic_Texture (
  int index,          // index into dynamic texture list to put data
  int type,
  int dx, 
  int dy,
  int num_color_bits,    
  int num_alpha_bits, 
  unsigned *size );
static void Add_Dynamic_Texture_To_Page_List (
  int dynamic_texture,  // index into dynamic texture list
  int type );           // type of texture (square or cubemap)
static void Free_Dynamic_Texture (unsigned index);
static void Remove_Dynamic_Texture_From_Page_List (
  int dynamic_texture,  // index into dynamic texture list
  int type );           // type of texture (square or cubemap)

/*____________________
|
|	Constants
|___________________*/

#define MAX_EVENTS	96

#define THE_PAGE    (dx9_Page_list[dx9_Active_page].surface)

/*____________________
|
|	Page list
|___________________*/

#define MAX_PAGES                 64
#define PAGE_TYPE_SCREEN          1
#define PAGE_TYPE_VIRTUAL         2
#define PAGE_TYPE_DYNAMIC_TEXTURE 3

static struct {
  int   type;     
  int   dx, dy;           // dimensions
  byte *surface;          // ptr to surface, if page type is virtual
} dx9_Page_list [MAX_PAGES];

/*____________________
|
|	Dynamic Textures list
|___________________*/

#define MAX_DYNAMIC_TEXTURES          8
#define DYNAMIC_TEXTURE_TYPE_SQUARE   1
#define DYNAMIC_TEXTURE_TYPE_CUBEMAP  2

static struct {
  int   type;           // square or cubemap
  int   dx, dy;         // dimensions
  int   num_color_bits;
  int   num_alpha_bits;
  byte *texture;        // texture ptr
  byte *surface[6];     // surface, 6 surfaces if cubemap
  int   page[6];        // corresponding entry in page list, 6 entries if cubemap - (NOT USED! 2/26/02)
} dx9_Dynamic_Texture_list [MAX_DYNAMIC_TEXTURES];

/*____________________
|
|	Light list
|___________________*/

#define MAX_3D_LIGHTS 8

static Light dx9_Light_list [MAX_3D_LIGHTS];

/*____________________
|
|	Clip list
|___________________*/

#define MAX_3D_CLIP_PLANES 32

static unsigned dx9_Clip_plane_list [MAX_3D_CLIP_PLANES];

/*____________________
|
|	Global variables
|___________________*/

static int  dx9_Keyboard_enabled;
static int  dx9_Mouse_enabled;

static int  direct3d_initialized     = FALSE;
static int  direct_input_initialized = FALSE;

//static int  dx9_Screen_width, dx9_Screen_height;  // not used! (2/26/02)
static int  dx9_Active_page;

// Table of info about video modes
static struct {
  unsigned resolution, width, height, depth;
} Mode_info [] = {
  { dx9_RESOLUTION_640x480,    640,  480, 16 }, // 16-bit modes
  { dx9_RESOLUTION_800x600,    800,  600, 16 },
  { dx9_RESOLUTION_1024x768,  1024,  768, 16 },
  { dx9_RESOLUTION_1152x864,  1152,  864, 16 },
  { dx9_RESOLUTION_1280x960,  1280,  960, 16 },
  { dx9_RESOLUTION_1280x1024, 1280, 1024, 16 },
  { dx9_RESOLUTION_1400x1050, 1400, 1050, 16 },
  { dx9_RESOLUTION_1440x1080, 1440, 1080, 16 },
  { dx9_RESOLUTION_1600x1200, 1600, 1200, 16 },
  { dx9_RESOLUTION_1152x720,  1152,  720, 16 },
  { dx9_RESOLUTION_1280x800,  1280,  800, 16 },
  { dx9_RESOLUTION_1440x900,  1440,  900, 16 },
  { dx9_RESOLUTION_1680x1050, 1680, 1050, 16 },
  { dx9_RESOLUTION_1920x1200, 1920, 1200, 16 }, 
  { dx9_RESOLUTION_2048x1280, 2048, 1280, 16 },  
  { dx9_RESOLUTION_1280x720,  1280,  720, 16 }, 
  { dx9_RESOLUTION_1600x900,  1600,  900, 16 }, 
  { dx9_RESOLUTION_1920x1080, 1920, 1080, 16 }, 
  { dx9_RESOLUTION_2048x1152, 2048, 1152, 16 },
  { dx9_RESOLUTION_2560x1440, 2560, 1440, 16 },
  { dx9_RESOLUTION_2560x1600, 2560, 1600, 16 },

  { dx9_RESOLUTION_640x480,    640,  480, 24 }, // 24-bit modes
  { dx9_RESOLUTION_800x600,    800,  600, 24 },
  { dx9_RESOLUTION_1024x768,  1024,  768, 24 },
  { dx9_RESOLUTION_1152x864,  1152,  864, 24 },
  { dx9_RESOLUTION_1280x960,  1280,  960, 24 },
  { dx9_RESOLUTION_1280x1024, 1280, 1024, 24 },
  { dx9_RESOLUTION_1400x1050, 1400, 1050, 24 },
  { dx9_RESOLUTION_1440x1080, 1440, 1080, 24 },
  { dx9_RESOLUTION_1600x1200, 1600, 1200, 24 },
  { dx9_RESOLUTION_1152x720,  1152,  720, 24 },
  { dx9_RESOLUTION_1280x800,  1280,  800, 24 },
  { dx9_RESOLUTION_1440x900,  1440,  900, 24 },
  { dx9_RESOLUTION_1680x1050, 1680, 1050, 24 },
  { dx9_RESOLUTION_1920x1200, 1920, 1200, 24 }, 
  { dx9_RESOLUTION_2048x1280, 2048, 1280, 24 },  
  { dx9_RESOLUTION_1280x720,  1280,  720, 24 }, 
  { dx9_RESOLUTION_1600x900,  1600,  900, 24 }, 
  { dx9_RESOLUTION_1920x1080, 1920, 1080, 24 }, 
  { dx9_RESOLUTION_2048x1152, 2048, 1152, 24 },
  { dx9_RESOLUTION_2560x1440, 2560, 1440, 24 },
  { dx9_RESOLUTION_2560x1600, 2560, 1600, 24 },
                                           
  { dx9_RESOLUTION_640x480,    640,  480, 32 }, // 32-bit modes
  { dx9_RESOLUTION_800x600,    800,  600, 32 },
  { dx9_RESOLUTION_1024x768,  1024,  768, 32 },
  { dx9_RESOLUTION_1152x864,  1152,  864, 32 },
  { dx9_RESOLUTION_1280x960,  1280,  960, 32 },
  { dx9_RESOLUTION_1280x1024, 1280, 1024, 32 },
  { dx9_RESOLUTION_1400x1050, 1400, 1050, 32 },
  { dx9_RESOLUTION_1440x1080, 1440, 1080, 32 },
  { dx9_RESOLUTION_1600x1200, 1600, 1200, 32 },
  { dx9_RESOLUTION_1152x720,  1152,  720, 32 },
  { dx9_RESOLUTION_1280x800,  1280,  800, 32 },
  { dx9_RESOLUTION_1440x900,  1440,  900, 32 },
  { dx9_RESOLUTION_1680x1050, 1680, 1050, 32 },
  { dx9_RESOLUTION_1920x1200, 1920, 1200, 32 }, 
  { dx9_RESOLUTION_2048x1280, 2048, 1280, 32 },  
  { dx9_RESOLUTION_1280x720,  1280,  720, 32 }, 
  { dx9_RESOLUTION_1600x900,  1600,  900, 32 }, 
  { dx9_RESOLUTION_1920x1080, 1920, 1080, 32 }, 
  { dx9_RESOLUTION_2048x1152, 2048, 1152, 32 },
  { dx9_RESOLUTION_2560x1440, 2560, 1440, 32 },
  { dx9_RESOLUTION_2560x1600, 2560, 1600, 32 },
  { 0,                           0,    0,  0 }
};

/*___________________________________________________________________
|
|	Function: dx9_GetUserFormat
| 
| Output:	Returns true if user selects a video format, or 0 on any error.
|   If user selects a mode, returns true.
|___________________________________________________________________*/

int dx9_GetUserFormat (
  unsigned  acceptable_resolutions, // bit mask of acceptable resolutions
  unsigned  acceptable_bitdepths,   // bit mask of acceptable bitdepths
  unsigned *selected_resolution,
  unsigned *selected_bitdepth,
  int       enable_hardware_acceleration )
{
	int i, found, width, height, depth;
  int format_selected = FALSE;

  // Query for hardware support
  if (Direct3D_QueryHardware (acceptable_resolutions, acceptable_bitdepths, enable_hardware_acceleration)) {
    // Allow user to select a video mode
    if (Direct3D_UserSelectMode (&width, &height, &depth)) {
      // Get the correct mode #, (will always find it, should be no error here!)
      for (i=0, found=FALSE; (NOT found) AND Mode_info[i].resolution; i++) { 
        if ((width  == (int)Mode_info[i].width)  AND
            (height == (int)Mode_info[i].height) AND
            (depth  == (int)Mode_info[i].depth)) {
          *selected_resolution = Mode_info[i].resolution;
          // change bit depth to a constant
          switch (Mode_info[i].depth) {
            case 16: *selected_bitdepth = dx9_BITDEPTH_16;
                     break;
            case 32: *selected_bitdepth = dx9_BITDEPTH_32;
                     break;
          }
          found = TRUE;
        }
      }
      // Should always be found, but double check here
      if (found)
        format_selected = TRUE;
      direct3d_initialized = TRUE;
    }
    else
      dx9_Free ();
  }

	return (format_selected);
}

/*___________________________________________________________________
|
|	Function: dx9_Init
| 
|	Input:
| Output:	Returns # of pages available or 0 on any error.
|___________________________________________________________________*/

int dx9_Init (
  unsigned resolution, 
  unsigned bitdepth, 
  unsigned stencil_depth_requested,
  int      num_pages_requested, 
  int      enable_hardware_acceleration )
{
	int i, found, width, height, depth;
	int num_pages_available = 0;


/*___________________________________________________________________
|
| Initialize D3D and query for hardware support?
|___________________________________________________________________*/

  // Init the Direct3D driver
  if (NOT direct3d_initialized) 
    // Query for hardware support
    if (Direct3D_QueryHardware (0xFFFFFFFF, 0xFFFFFFFF, enable_hardware_acceleration)) 
      direct3d_initialized = TRUE;
  
/*___________________________________________________________________
|
| Set the requested video mode
|___________________________________________________________________*/

  if (direct3d_initialized) {
  
    // Convert bitdepth from a constant to 8,16,24,32
    switch (bitdepth) {
      case dx9_BITDEPTH_16: bitdepth = 16;
                            break;
      case dx9_BITDEPTH_32: bitdepth = 32;
                            break;
    }

    // Get info about video mode
    for (i=0, found=FALSE; (NOT found) AND Mode_info[i].resolution; i++) 
      if ((resolution == Mode_info[i].resolution) AND (bitdepth == Mode_info[i].depth)) {
        width  = Mode_info[i].width;
        height = Mode_info[i].height;
        depth  = Mode_info[i].depth;
        found = TRUE;
      }
    // Set the video mode
    num_pages_available = Direct3D_SetMode (width, height, depth, stencil_depth_requested, num_pages_requested);

/*___________________________________________________________________
|
| Init global variables
|___________________________________________________________________*/

    if (num_pages_available) {
      // Init globals
	    dx9_Keyboard_enabled = FALSE;
	    dx9_Mouse_enabled		 = FALSE;
//      dx9_Screen_width     = width;
//      dx9_Screen_height    = height;
      dx9_Active_page      = 0;

      // Init page list
      for (i=0; i<MAX_PAGES; i++) {
        if (i < num_pages_available) {
          dx9_Page_list[i].type    = PAGE_TYPE_SCREEN;
          dx9_Page_list[i].dx      = width;
          dx9_Page_list[i].dy      = height;
          dx9_Page_list[i].surface = NULL;
        }
        else
          dx9_Page_list[i].type = 0;
      }

/*___________________________________________________________________
|
| Init 3D state
|___________________________________________________________________*/

      // Initialize clip plane list
      for (i=0; i<MAX_3D_CLIP_PLANES; i++)
        dx9_Clip_plane_list[i] = 0;
      // Initialize light list
      for (i=0; i<MAX_3D_LIGHTS; i++)
        dx9_Light_list[i].type = none;
      // Initialize dynamic texture list
      for (i=0; i<MAX_DYNAMIC_TEXTURES; i++)
        dx9_Dynamic_Texture_list[i].type = 0;

      // Set viewport to full screen
      Direct3D_SetViewport (0, 0, width-1, height-1);
      // Disable zbuffering
      Direct3D_EnableZBuffer (FALSE);
      // Disable lighting
      Direct3D_EnableLighting (FALSE);
    }
  }

	return (num_pages_available);
}

/*___________________________________________________________________
|
|	Function: dx9_Free
| 
| Output:	Frees up resources created by dx9_Init()
|___________________________________________________________________*/

void dx9_Free (void)
{
	Direct3D_Free ();
  direct3d_initialized = FALSE;
}

/*___________________________________________________________________
|
|	Function: dx9_VertRetraceDelay
| 
| Output:	Waits until the start of the next vertical retrace and then 
|   returns.
|___________________________________________________________________*/

void dx9_VertRetraceDelay (void)
{
  Direct3D_Vertical_Retrace_Delay ();
}

/*___________________________________________________________________
|
|	Function: dx9_RestoreDirectX
| 
|	Input:
| Output:	Restores Windows OS resources, vram surfaces, input interfaces,
|					etc.
|___________________________________________________________________*/

int dx9_RestoreDirectX (void) 
{
  int i;
  unsigned size;
  int restored = FALSE;

/*___________________________________________________________________
|
| Release memory for virtual pages
|___________________________________________________________________*/

  for (i=0; i<MAX_PAGES; i++) 
    if (dx9_Page_list[i].type == PAGE_TYPE_VIRTUAL) 
      // Free previously created interface
      Direct3D_Free_Surface (dx9_Page_list[i].surface);
  
/*___________________________________________________________________
|
| Release memory for dynamic textures
|___________________________________________________________________*/

  for (i=0; i<MAX_DYNAMIC_TEXTURES; i++)
    // Is this texture index active?
    if (dx9_Dynamic_Texture_list[i].type) 
      // Free the memory for it but don't set type = 0
      Free_Dynamic_Texture (i);

/*___________________________________________________________________
|
| Restore Direct3D
|___________________________________________________________________*/

  if (Direct3D_Restore ()) {

/*___________________________________________________________________
|
| Restore virtual pages
|___________________________________________________________________*/
  
    // Restore virtual pages
    for (i=0; i<MAX_PAGES; i++) 
      if (dx9_Page_list[i].type == PAGE_TYPE_VIRTUAL) {
        // Recreate the interface
        dx9_Page_list[i].surface = Direct3D_Allocate_Surface (dx9_Page_list[i].dx, dx9_Page_list[i].dy);
        // Any error?
        if (dx9_Page_list[i].surface == NULL) {
          dx9_Page_list[i].type = 0;
          DEBUG_ERROR ("dx9_RestoreDirectX(): ERROR restoring a virtual page")
        }
      }
    
/*___________________________________________________________________
|
| Restore dynamic texture
|___________________________________________________________________*/

  for (i=0; i<MAX_DYNAMIC_TEXTURES; i++)
    // Is this texture index active?
    if (dx9_Dynamic_Texture_list[i].type) 
      // Restore the texture
      if (NOT Init_Dynamic_Texture (i, 
                                    dx9_Dynamic_Texture_list[i].type, 
                                    dx9_Dynamic_Texture_list[i].dx,
                                    dx9_Dynamic_Texture_list[i].dy,
                                    dx9_Dynamic_Texture_list[i].num_color_bits,
                                    dx9_Dynamic_Texture_list[i].num_alpha_bits,
                                    &size)) {


        dx9_Dynamic_Texture_list[i].type = 0;
        DEBUG_ERROR ("dx9_RestoreDirectX(): ERROR restoring a dynamic texture")
      }  

/*___________________________________________________________________
|
| Restore lights
|___________________________________________________________________*/

    for (i=0; i<MAX_3D_LIGHTS; i++)
      if (dx9_Light_list[i].type)
        Init_Light (i, &(dx9_Light_list[i]));

/*___________________________________________________________________
|
| Restore DirectInput
|___________________________________________________________________*/

    DirectInput_Restore (); // Do this after restoring D3D

    restored = TRUE;
  } // if (Direct3D_Restore ...

  return (restored);
}

/*___________________________________________________________________
|
|	Function: dx9_GetRGBFormat
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_GetRGBFormat (
  unsigned *redmask, 
  unsigned *greenmask, 
  unsigned *bluemask,
  int      *low_redbit,
  int      *low_greenbit,
  int      *low_bluebit,
  int      *num_redbits,
  int      *num_greenbits,
  int      *num_bluebits )
{
  Direct3D_Get_RGB_Format (redmask, greenmask, bluemask, 
                           low_redbit, low_greenbit, low_bluebit,
                           num_redbits, num_greenbits, num_bluebits );
}

/*___________________________________________________________________
|
|	Function: dx9_SetActivePage
| 
|	Input:
| Output:	
|___________________________________________________________________*/

int dx9_SetActivePage (int page)	
{
  int set = FALSE;

  if (Direct3D_Set_Active_Page (dx9_Page_list[page].surface, FALSE)) {
    dx9_Active_page = page;
    set = TRUE;
  }

  return (set);
}

/*___________________________________________________________________
|
|	Function: dx9_FlipVisualPage
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_FlipVisualPage (void)
{
  Direct3D_Flip_Visual_Page ();
}

/*___________________________________________________________________
|
|	Function: dx9_SetForeColor
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_SetForeColor (byte r, byte g, byte b, byte a)
{
	Direct3D_Set_Fore_Color (r, g, b, a);
}

/*___________________________________________________________________
|
|	Function: dx9_SetLogicOp
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_SetLogicOp (int logic_op)
{
	Direct3D_Set_Logic_Op (logic_op);
}

/*___________________________________________________________________
|
|	Function: dx9_DrawPixel
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_DrawPixel (int x, int y)
{
	Direct3D_Draw_Pixel (x, y, THE_PAGE);
}

/*___________________________________________________________________
|
|	Function: dx9_GetPixel
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_GetPixel (int x, int y, byte *r, byte *g, byte *b)
{
	Direct3D_Get_Pixel (x, y, r, g, b, THE_PAGE);
}

/*___________________________________________________________________
|
|	Function: dx9_DrawLine
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_DrawLine (int x1, int y1, int x2, int y2)
{
	Direct3D_Draw_Line (x1, y1, x2, y2, THE_PAGE);
}

/*___________________________________________________________________
|
|	Function: dx9_DrawFillRectangle
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_DrawFillRectangle (int x1, int y1, int x2, int y2)
{
	Direct3D_Draw_Fill_Rectangle (x1, y1, x2, y2, THE_PAGE);
}

/*___________________________________________________________________
|
|	Function: dx9_PutImage
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_PutImage (
	byte *image,
	int   image_dx,
	int		image_dy,
	int		image_x,
	int		image_y,
	int		x,
	int		y,
	int		dx,
	int		dy,
	int		or_image )	// boolean
{
	Direct3D_Put_Image (image, image_dx, image_x,	image_y, x,	y, dx, dy, or_image, THE_PAGE);
}

/*___________________________________________________________________
|
|	Function: dx9_GetImage
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_GetImage (
	byte *image,
	int   image_dx,
	int		image_dy,
	int		image_x,
	int		image_y,
	int		x,
	int		y,
	int		dx,
	int		dy )
{
  Direct3D_Get_Image (image, image_dx, image_x,	image_y, x,	y, dx, dy, THE_PAGE);
}

/*___________________________________________________________________
|
|	Function: dx9_CopyImage
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_CopyImage (
	int srcx,	
	int srcy,	
	int src_pg, 
	int dstx, 
	int dsty, 
	int dst_pg,	
	int dx,	
	int dy )
{
  Direct3D_Copy_Image (srcx, srcy, dx9_Page_list[src_pg].surface, dstx, dsty, dx9_Page_list[dst_pg].surface, dx, dy);
}

/*___________________________________________________________________
|
|	Function: dx9_CopyImageColorKey
| 
|	Input:
| Output:	Copies an image from one page to another (possibly the same page).
|   Any source pixels of the key color are not copied, leaving the 
|   destination pixels untouched.
|___________________________________________________________________*/

void dx9_CopyImageColorKey (
	int  srcx,	
	int  srcy,	
	int  src_pg, 
	int  dstx, 
	int  dsty, 
	int  dst_pg,	
	int  dx,	
	int  dy,
  byte r,
  byte g,
  byte b )
{
  Direct3D_Copy_Image_ColorKey (srcx, srcy, dx9_Page_list[src_pg].surface, dstx, dsty, dx9_Page_list[dst_pg].surface, dx, dy, r, g, b);
}

/*___________________________________________________________________
|
|	Function: dx9_PutBitmap
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_PutBitmap (
	byte *bitmap,
	int   bitmap_dx,  // in pixels
	int		bitmap_dy,
	int   bitmap_x,	  // top-left coordinates in bitmap
	int   bitmap_y,
	int   x,					// top-left coordinates of destination
	int   y,
	int   dx,				  // size in pixels of rectangle to transfer
	int   dy,
	byte  r, 			    // color to draw bitmap as
  byte  g,
  byte  b )
{
	Direct3D_Put_Bitmap (bitmap, bitmap_dx, bitmap_x, bitmap_y, x, y, dx, dy, r, g, b, THE_PAGE);
}

/*___________________________________________________________________
|
|	Function: dx9_CreateVirtualPage
| 
|	Input:
| Output:	Creates a DirectX virtual buffer.  Returns page # if successful, 
|         else -1 on any error.
|___________________________________________________________________*/

int dx9_CreateVirtualPage (int dx, int dy, int create_in_vram)
{
  int i;
  int page = -1;

  // Look for an empty entry in page list
  for (i=0; dx9_Page_list[i].type AND (i<MAX_PAGES); i++);

  // Is their an entry available?
  if (i < MAX_PAGES) {
    // Create the virtual page
    dx9_Page_list[i].surface = Direct3D_Allocate_Surface (dx, dy);
    if (dx9_Page_list[i].surface) {
      dx9_Page_list[i].type = PAGE_TYPE_VIRTUAL;
      dx9_Page_list[i].dx   = dx;
      dx9_Page_list[i].dy   = dy;
      page = i;
    }
  }

  return (page);
}

/*___________________________________________________________________
|
|	Function: dx9_FreeVirtualPage
| 
|	Input:
| Output:	Frees memory allocated by dx9_CreateVirtualPage().
|___________________________________________________________________*/

void dx9_FreeVirtualPage (int page)
{
  // Is this a valid page number
  if ((page >= 0) AND (page < MAX_PAGES)) {
    // Make sure this isn't the currently active page
    if (page != dx9_Active_page) {
      // Is this a virtual page?
      if (dx9_Page_list[page].type == PAGE_TYPE_VIRTUAL) {
        // Free this page
        Direct3D_Free_Surface (dx9_Page_list[page].surface);
        dx9_Page_list[page].surface = NULL;
        dx9_Page_list[page].type   = 0;
      }
    }
  }
}

/*___________________________________________________________________
|
|	Function: dx9_BeginRender
| 
|	Input:
| Output: Begins 3D rendering.
|___________________________________________________________________*/

int dx9_BeginRender (void)
{
  return (Direct3D_BeginRender ());
}

/*___________________________________________________________________
|
|	Function: dx9_EndRender
| 
|	Input:
| Output: Ends 3D rendering.
|___________________________________________________________________*/

int dx9_EndRender (void)
{
  return (Direct3D_EndRender ());
}

/*___________________________________________________________________
|
|	Function: dx9_SetFillMode
| 
|	Input:
| Output: Sets the render fill mode to one of:
|
|     dx9_FILL_MODE_POINT           
|     dx9_FILL_MODE_WIREFRAME       
|     dx9_FILL_MODE_SMOOTH_SHADED   
|     dx9_FILL_MODE_GOURAUD_SHADED    
|___________________________________________________________________*/

void dx9_SetFillMode (int fill_mode)
{
  Direct3D_SetFillMode (fill_mode);
}

/*___________________________________________________________________
|
|	Function: dx9_GetDriverInfo
| 
| Output: Returns info about the 3D capabilities of the driver.
|___________________________________________________________________*/

void dx9_GetDriverInfo (
  unsigned *max_texture_dx,
  unsigned *max_texture_dy,
  int      *max_active_lights,
  int      *max_user_clip_planes,
  int      *max_simultaneous_texture_stages,
  int      *max_texture_stages,
  int      *max_texture_repeat,
  int      *num_stencil_bits,
  unsigned *stencil_ops, 
  int      *max_vertex_blend_matrices,
  int      *max_vertex_streams,
  unsigned *max_vertex_index )
{
  Direct3D_GetDriverInfo (max_texture_dx,
                          max_texture_dy,
                          max_active_lights,
                          max_user_clip_planes,
                          max_simultaneous_texture_stages,
                          max_texture_stages,
                          max_texture_repeat,
                          num_stencil_bits,
                          stencil_ops,
                          max_vertex_blend_matrices,
                          max_vertex_streams,
                          max_vertex_index );
}

/*___________________________________________________________________
|
|	Function: dx9_RegisterObject
| 
| Output: Registers an object.  An object must be registered before it
|   can be used in rendering.
|___________________________________________________________________*/

void dx9_RegisterObject (
  word   *surface,                // array of indeces (group of 3 unsigned short)
  int    *num_surfaces,           // # of indeces
  float  *vertex,                 // array of vertices (group of 3 float - x,y,z)
  float **X_vertex,               // array of vertices 
  int    *num_vertices,           // # of vertices
  float  *vertex_normal,          // array of vertex normals (group of 3 float - x,y,z)
  float **X_vertex_normal,        // array of vertex normals
  byte   *vertex_color_diffuse,   // array of diffuse color in 0xAABBGGRR format, or 0 if none
  byte   *vertex_color_specular,  // array of specular color in 0xAABBGGRR format, or 0 if none
  float **texture_coord,          // 0-8arrays of texture coords (group of 2 float - u,v)
  float **X_texture_coord,
  float **texture_coord_w,        // 0-8 arrays of texture w coords (1 float - w)
  float **X_texture_coord_w,
  byte   *weight,      
  byte  **X_weight,
  void  **driver_data )           // address of a pointer to driver-specific data
{
  int i;
  dx9_Object *object;
  int error = FALSE;

/*___________________________________________________________________
|
| Create object struct
|___________________________________________________________________*/

  object = (dx9_Object *) calloc (1, sizeof(dx9_Object));

  if (object == NULL) 
    error = TRUE;
  else {

/*___________________________________________________________________
|
| Put data into it
|___________________________________________________________________*/

    object->surface           = surface;
    object->num_surfaces      = num_surfaces;
    object->vertex            = vertex;
    object->X_vertex          = X_vertex;
    object->num_vertices      = num_vertices;
    object->vertex_normal     = vertex_normal;
    object->X_vertex_normal   = X_vertex_normal;
    object->texture_coord     = texture_coord;
    object->X_texture_coord   = X_texture_coord;
    object->texture_coord_w   = texture_coord_w;
    object->X_texture_coord_w = X_texture_coord_w;
    object->weight            = weight;
    object->X_weight          = X_weight;
    
/*___________________________________________________________________
|
| Allocate memory for diffuse color array?
|___________________________________________________________________*/
     
    if (vertex_color_diffuse) {
      object->vertex_color_diffuse = (byte *) calloc (*num_vertices, 4 * sizeof(byte));
      if (object->vertex_color_diffuse == NULL) 
        error = TRUE;
      else {
        // Change format of colors, putting into dx9 format
        for (i=0; i<*num_vertices; i++) {
          object->vertex_color_diffuse[i*4+0] = vertex_color_diffuse[i*4+2];  // set new b
          object->vertex_color_diffuse[i*4+1] = vertex_color_diffuse[i*4+1];  // set new g
          object->vertex_color_diffuse[i*4+2] = vertex_color_diffuse[i*4+0];  // set new r
          object->vertex_color_diffuse[i*4+3] = vertex_color_diffuse[i*4+3];  // set new a
        }
      }
    }
    
/*___________________________________________________________________
|
| Allocate memory for specular color array?
|___________________________________________________________________*/
     
    if (vertex_color_specular) {
      object->vertex_color_specular = (byte *) calloc (*num_vertices, 4 * sizeof(byte));
      if (object->vertex_color_specular == NULL) 
        error = TRUE;
      else {
        // Change format of colors, putting into dx9 format
        for (i=0; i<*num_vertices; i++) {
          object->vertex_color_specular[i*4+0] = vertex_color_specular[i*4+2];  // set new b
          object->vertex_color_specular[i*4+1] = vertex_color_specular[i*4+1];  // set new g
          object->vertex_color_specular[i*4+2] = vertex_color_specular[i*4+0];  // set new r
          object->vertex_color_specular[i*4+3] = vertex_color_specular[i*4+3];  // set new a
        }
      }
    }

/*___________________________________________________________________
|
| Initialize D3D specific stuff
|___________________________________________________________________*/

    if (NOT error)
      Direct3D_InitObject ((d3d9_Object *)object);
  } // else

/*___________________________________________________________________
|
| Return this object to caller
|___________________________________________________________________*/

  if (NOT error)
    // Save address of this object in callers variable
    *driver_data = (void *)object;
  else
    // On error, release all memory and resources
    dx9_UnregisterObject ((void *)object);
}

/*___________________________________________________________________
|
|	Function: dx9_UnregisterObject
| 
| Output: Unregisters an object.  An object must be unregistered before 
|   the caller can free it.
|___________________________________________________________________*/

void dx9_UnregisterObject (void *driver_data)
{
  dx9_Object *object = (dx9_Object *)driver_data;

  if (object) {
    // Free diffuse, specular arrays, if any
    if (object->vertex_color_diffuse)
      free (object->vertex_color_diffuse);
    if (object->vertex_color_specular)
      free (object->vertex_color_specular);
    // Free any D3D specific stuff
    Direct3D_FreeObject ((d3d9_Object *)object);
    // Free memory for the object struct
    free (object);
  }
}

/*___________________________________________________________________
|
|	Function: dx9_DrawObject
| 
| Output: Draws a (registered) object.
|___________________________________________________________________*/

void dx9_DrawObject (void *driver_data)
{
  if (driver_data)
    Direct3D_DrawObject ((d3d9_Object *)driver_data);
}

/*___________________________________________________________________
|
|	Function: dx9_OptimizeObject
| 
|	Input:
| Output: Optimizes a (registered) object.
|___________________________________________________________________*/

void dx9_OptimizeObject (void *driver_data)
{
}

/*___________________________________________________________________
|
|	Function: dx9_SetViewport
| 
|	Input:
| Output: Sets the onscreen viewport for 3D rendering.
|___________________________________________________________________*/

int dx9_SetViewport (int left, int top, int right, int bottom)
{
  return (Direct3D_SetViewport (left, top, right, bottom));
}

/*___________________________________________________________________
|
|	Function: dx9_ClearViewportRectangle
| 
|	Input:
| Output: Clears a rectangle in the 3D viewport to a color, and optionally
|   clears the zbuffer and/or stencil buffer.
|___________________________________________________________________*/

void dx9_ClearViewportRectangle (
  int      *rect, 
  unsigned  flags,  // 0x1=surface, 0x2=zbuffer, 0x4=stencil
  byte      r,
  byte      g,
  byte      b,
  byte      a,
  float     zval,
  unsigned  stencilval )
{
  Direct3D_ClearViewportRectangle (rect, flags, r, g, b, a, zval, stencilval);
}

/*___________________________________________________________________
|
|	Function: dx9_EnableClipping
| 
|	Input:
| Output: Enables/disables clipping to view frustrum.  Normally this
|   should be on and should only be turned off when drawing objects
|   known to be completely within the view frustrum.  
|___________________________________________________________________*/

void dx9_EnableClipping (int flag)
{
  Direct3D_EnableClipping (flag);
}

/*___________________________________________________________________
|
|	Function: dx9_InitClipPlane
| 
|	Input:
| Output: Initializes a clip plane in a disabled state.  Returns 0 on
|   error, else a positive integer handle to the plane.
|___________________________________________________________________*/

unsigned dx9_InitClipPlane (float a, float b, float c, float d)
{
  int i;
  unsigned plane = 0;

  // Find a free slot in the clip plane list
  for (i=0; i<MAX_3D_CLIP_PLANES; i++) 
    if (dx9_Clip_plane_list[i] == 0)
      break;

  // Free slot found?
  if (i < MAX_3D_CLIP_PLANES) {
    // Init this clip plane
    if (Direct3D_InitClipPlane (i, a, b, c, d)) {
      // Mark this as in use
      dx9_Clip_plane_list[i] = 1;
      plane = i+1;
    }     
  }

  return (plane);
}

/*___________________________________________________________________
|
|	Function: dx9_FreeClipPlane
| 
|	Input:
| Output: Frees a clip plane.  Returns true on success, else false on
|   any error.
|___________________________________________________________________*/

void dx9_FreeClipPlane (unsigned plane)
{
  // Is this a valid index into the clip plane list?
  if ((plane > 0) AND (plane <= MAX_3D_CLIP_PLANES))
    // Is this plane active?
    if (dx9_Clip_plane_list[plane-1] == 1) {
      // Free the plane
      Direct3D_EnableClipPlane (plane-1, FALSE);
      dx9_Clip_plane_list[plane-1] = 0;
    }
}

/*___________________________________________________________________
|
|	Function: dx9_EnableClipPlane
| 
|	Input:
| Output: Enables or disables the clip plane.  
|___________________________________________________________________*/

void dx9_EnableClipPlane (unsigned plane, int flag)
{
  // Is this a valid index into the clip plane list?
  if ((plane > 0) AND (plane <= MAX_3D_CLIP_PLANES))
    // Is this plane active?
    if (dx9_Clip_plane_list[plane-1] == 1) 
      // Enable or disable the plane
      Direct3D_EnableClipPlane (plane-1, flag);
}

/*___________________________________________________________________
|
|	Function: dx9_SetWorldMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int dx9_SetWorldMatrix (void *m)
{
  return (Direct3D_SetWorldMatrix (0, m));
}

/*___________________________________________________________________
|
|	Function: dx9_GetWorldMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int dx9_GetWorldMatrix (void *m)
{
  return (Direct3D_GetWorldMatrix (0, m));
}

/*___________________________________________________________________
|
|	Function: dx9_SetViewMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int dx9_SetViewMatrix (void *m)
{
  return (Direct3D_SetViewMatrix (m));
}

/*___________________________________________________________________
|
|	Function: dx9_GetViewMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int dx9_GetViewMatrix (void *m)
{
  return (Direct3D_GetViewMatrix (m));
}

/*___________________________________________________________________
|
|	Function: dx9_SetProjectionMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int dx9_SetProjectionMatrix (void *m)
{
  return (Direct3D_SetProjectionMatrix (m));
}

/*___________________________________________________________________
|
|	Function: dx9_GetProjectionMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int dx9_GetProjectionMatrix (void *m)
{
  return (Direct3D_GetProjectionMatrix (m));
}

/*___________________________________________________________________
|
|	Function: dx9_EnableTextureMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int dx9_EnableTextureMatrix (int stage, int dimension, int flag)
{
  return (Direct3D_EnableTextureMatrix (stage, dimension, flag));
}

/*___________________________________________________________________
|
|	Function: dx9_SetTextureMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int dx9_SetTextureMatrix (int stage, void *m)
{
  return (Direct3D_SetTextureMatrix (stage, m));
}

/*___________________________________________________________________
|
|	Function: dx9_GetTextureMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int dx9_GetTextureMatrix (int stage, void *m)
{
  return (Direct3D_GetTextureMatrix (stage, m));
}

/*___________________________________________________________________
|
|	Function: dx9_EnableZBuffer
| 
|	Input:
| Output:
|___________________________________________________________________*/

void dx9_EnableZBuffer (int flag)
{
  Direct3D_EnableZBuffer (flag);
}

/*___________________________________________________________________
|
|	Function: dx9_EnableBackfaceRemoval
| 
|	Input:
| Output:
|___________________________________________________________________*/

void dx9_EnableBackfaceRemoval (int flag)
{
  Direct3D_EnableBackfaceRemoval (flag);
}

/*____________________________________________________________________
|
| Function: dx9_EnableStencilBuffer
|
| Output: Enables stencil buffer processing.
|___________________________________________________________________*/

void dx9_EnableStencilBuffer (int flag)
{
  Direct3D_EnableStencilBuffer (flag);
}

/*____________________________________________________________________
|
| Function: dx9_SetStencilFailOp
|
| Output: Sets the stencil operation to perform if the stencil test
|     fails.  Operation can be one of the following:
|
|       dx9_STENCILOP_DECR     
|       dx9_STENCILOP_DECRSAT  
|       dx9_STENCILOP_INCR     
|       dx9_STENCILOP_INCRSAT  
|       dx9_STENCILOP_INVERT   
|       dx9_STENCILOP_KEEP   (default)
|       dx9_STENCILOP_REPLACE  
|       dx9_STENCILOP_ZERO     
|___________________________________________________________________*/

void dx9_SetStencilFailOp (int stencil_op)
{
  Direct3D_SetStencilFailOp (stencil_op);
}

/*____________________________________________________________________
|
| Function: dx9_SetStencilZFailOp
|
| Output: Sets the stencil operation to perform if the stencil test
|     passes and the depth test fails. Operation can be one of the 
|     following:
|
|       dx9_STENCILOP_DECR     
|       dx9_STENCILOP_DECRSAT  
|       dx9_STENCILOP_INCR     
|       dx9_STENCILOP_INCRSAT  
|       dx9_STENCILOP_INVERT   
|       dx9_STENCILOP_KEEP   (default)
|       dx9_STENCILOP_REPLACE  
|       dx9_STENCILOP_ZERO     
|___________________________________________________________________*/

void dx9_SetStencilZFailOp (int stencil_op) 
{
  Direct3D_SetStencilZFailOp (stencil_op);
}

/*____________________________________________________________________
|
| Function: dx9_SetStencilPassOp
|
| Output: Sets the stencil operation to perform if both the stencil test
|     passes and the depth test passes. Operation can be one of the 
|     following:
|
|       dx9_STENCILOP_DECR     
|       dx9_STENCILOP_DECRSAT  
|       dx9_STENCILOP_INCR     
|       dx9_STENCILOP_INCRSAT  
|       dx9_STENCILOP_INVERT   
|       dx9_STENCILOP_KEEP   (default)
|       dx9_STENCILOP_REPLACE  
|       dx9_STENCILOP_ZERO     
|___________________________________________________________________*/

void dx9_SetStencilPassOp (int stencil_op)
{
  Direct3D_SetStencilPassOp (stencil_op);
}

/*____________________________________________________________________
|
| Function: dx9_SetStencilComparison
|
| Output: Sets the stencil comparison function. The comparison function
|     compares the reference value to a stencil buffer entry and applies
|     only to the bits in the reference value and stencil buffer entry
|     that are set in the stencil mask.  If the comparison is true, the
|     stencil test passes.
|
|     Can be one of the following:
|
|       dx9_STENCILFUNC_NEVER        
|       dx9_STENCILFUNC_LESS         
|       dx9_STENCILFUNC_EQUAL        
|       dx9_STENCILFUNC_LESSEQUAL    
|       dx9_STENCILFUNC_GREATER      
|       dx9_STENCILFUNC_NOTEQUAL     
|       dx9_STENCILFUNC_GREATEREQUAL 
|       dx9_STENCILFUNC_ALWAYS   (default)       
|___________________________________________________________________*/

void dx9_SetStencilComparison (int stencil_function)
{
  Direct3D_SetStencilComparison (stencil_function);
}

/*____________________________________________________________________
|
| Function: dx9_SetStencilReferenceValue
|
| Output: Sets the integer reference value for the stencil test.  The
|     default value is 0.
|___________________________________________________________________*/

void dx9_SetStencilReferenceValue (unsigned reference_value)
{
  Direct3D_SetStencilReferenceValue (reference_value);
}

/*____________________________________________________________________
|
| Function: dx9_SetStencilMask
|
| Output: Sets the mask to apply to the reference value and each stencil
|     buffer entry to determine the significant bits for the stencil test.  
|     The default mask is 0xFFFFFFFF.
|___________________________________________________________________*/

void dx9_SetStencilMask (unsigned mask)
{
  Direct3D_SetStencilMask (mask);
}

/*____________________________________________________________________
|
| Function: dx9_SetStencilWriteMask
|
| Output: Sets the mask to apply to values written into the stencil 
|     buffer.  The default is 0xFFFFFFFF. 
|___________________________________________________________________*/

void dx9_SetStencilWriteMask (unsigned mask)
{
  Direct3D_SetStencilWriteMask (mask);
}

/*___________________________________________________________________
|
|	Function: dx9_EnableLighting
| 
|	Input:
| Output:
|___________________________________________________________________*/

void dx9_EnableLighting (int flag)
{
  Direct3D_EnableLighting (flag);
}

/*___________________________________________________________________
|
|	Function: dx9_InitPointLight
| 
|	Input:
| Output: Initializes a light in a disabled state.  Returns 0 on error,
|   else a positive integer handle to the light.
|___________________________________________________________________*/

unsigned dx9_InitPointLight (
  float  src_x,
  float  src_y,
  float  src_z,
  float  range,
  float  constant_attenuation,
  float  linear_attenuation,
  float  quadratic_attenuation, 
  float *ambient_color_rgba,
  float *diffuse_color_rgba,
  float *specular_color_rgba )
{
  int i;
  unsigned light = 0;

  // Find a free slot in the light list
  for (i=0; i<MAX_3D_LIGHTS; i++) 
    if (dx9_Light_list[i].type == 0)
      break;

  // Free slot found?
  if (i < MAX_3D_LIGHTS) {
    // Save data in light list
    dx9_Light_list[i].type                  = point;
    dx9_Light_list[i].on                    = FALSE;  // begins 'off' by default
    dx9_Light_list[i].src_x                 = src_x;
    dx9_Light_list[i].src_y                 = src_y;
    dx9_Light_list[i].src_z                 = src_z;
    dx9_Light_list[i].range                 = range;
    dx9_Light_list[i].constant_attenuation  = constant_attenuation;
    dx9_Light_list[i].linear_attenuation    = linear_attenuation;
    dx9_Light_list[i].quadratic_attenuation = quadratic_attenuation;
    memcpy (&(dx9_Light_list[i].ambient_color_rgba), ambient_color_rgba, 4*sizeof(float));
    memcpy (&(dx9_Light_list[i].diffuse_color_rgba), diffuse_color_rgba, 4*sizeof(float));
    memcpy (&(dx9_Light_list[i].specular_color_rgba), specular_color_rgba, 4*sizeof(float));
    // Initialize this light
    if (Init_Light (i, &dx9_Light_list[i]))
      light = i+1;
  }

  // On any error, release this slot in the light list
  if (light == 0)
    dx9_Light_list[i].type = none;

  return (light);
}

/*___________________________________________________________________
|
|	Function: Init_Light
| 
|	Input: Called from dx9_InitPointLight(), dx9_InitSpotLight(),
|   dx9_InitDirectionLight()
| Output: Initializes a light.  Returns true on success, else false on 
|   any error.
|___________________________________________________________________*/
  
static int Init_Light (int index, Light *light)
{
  int initialized = FALSE;

  switch (light->type) {
    case point: 
      initialized = Direct3D_InitPointLight (index, light->src_x, light->src_y, light->src_z, light->range,
                                             light->constant_attenuation, light->linear_attenuation, light->quadratic_attenuation,
                                             light->ambient_color_rgba, light->diffuse_color_rgba, light->specular_color_rgba);
      break;
    case spot:
      initialized = Direct3D_InitSpotLight (index, light->src_x, light->src_y, light->src_z, light->dst_x, light->dst_y, light->dst_z, light->range,
                                            light->constant_attenuation, light->linear_attenuation, light->quadratic_attenuation,
                                            light->inner_cone_angle, light->outer_cone_angle, light->falloff,
                                            light->ambient_color_rgba, light->diffuse_color_rgba, light->specular_color_rgba); 
      break;
    case direction:
      initialized = Direct3D_InitDirectionLight (index, light->dst_x, light->dst_y, light->dst_z, 
                                                 light->ambient_color_rgba, light->diffuse_color_rgba, light->specular_color_rgba);
      break;
  }

  if (initialized)
    Direct3D_EnableLight (index, light->on);

  return (initialized);
}

/*___________________________________________________________________
|
|	Function: dx9_UpdatePointLight
| 
| Output: Updates parameters for a light.
|___________________________________________________________________*/

void dx9_UpdatePointLight (
  unsigned light,
  float    src_x,
  float    src_y,
  float    src_z,
  float    range,
  float    constant_attenuation,
  float    linear_attenuation,
  float    quadratic_attenuation, 
  float   *ambient_color_rgba,
  float   *diffuse_color_rgba,
  float   *specular_color_rgba )
{
  // Compute index into light list
  int i = light - 1;

  // Is this a valid index into the light list?
  if ((light >= 0) AND (light < MAX_3D_LIGHTS))
    // Is this light active?
    if (dx9_Light_list[i].type == point) {
      // Save data in light list
      dx9_Light_list[i].src_x                 = src_x;
      dx9_Light_list[i].src_y                 = src_y;
      dx9_Light_list[i].src_z                 = src_z;
      dx9_Light_list[i].range                 = range;
      dx9_Light_list[i].constant_attenuation  = constant_attenuation;
      dx9_Light_list[i].linear_attenuation    = linear_attenuation;
      dx9_Light_list[i].quadratic_attenuation = quadratic_attenuation;
      memcpy (&(dx9_Light_list[i].ambient_color_rgba), ambient_color_rgba, 4*sizeof(float));
      memcpy (&(dx9_Light_list[i].diffuse_color_rgba), diffuse_color_rgba, 4*sizeof(float));
      memcpy (&(dx9_Light_list[i].specular_color_rgba), specular_color_rgba, 4*sizeof(float));
      // Initialize this light
      Init_Light (i, &dx9_Light_list[i]);
    }
}

/*___________________________________________________________________
|
|	Function: dx9_InitSpotLight
| 
| Output: Initializes a light in a disabled state.  Returns 0 on error,
|   else a positive integer handle to the light.
|___________________________________________________________________*/

unsigned dx9_InitSpotLight (
  float  src_x,
  float  src_y,
  float  src_z,
  float  dst_x,
  float  dst_y,
  float  dst_z,
  float  range,
  float  constant_attenuation,
  float  linear_attenuation,
  float  quadratic_attenuation,  
  float  inner_cone_angle,      // 0 - outer_cone_angle
  float  outer_cone_angle,      // 0 - 180
  float  falloff,               // intensity change from inner cone to outer cone
  float *ambient_color_rgba,
  float *diffuse_color_rgba,
  float *specular_color_rgba )
{
  int i;
  unsigned light = 0;

  // Find a free slot in the light list
  for (i=0; i<MAX_3D_LIGHTS; i++) 
    if (dx9_Light_list[i].type == 0)
      break;

  // Free slot found?
  if (i < MAX_3D_LIGHTS) {
    // Save data in light list
    dx9_Light_list[i].type                  = spot;
    dx9_Light_list[i].on                    = FALSE;  // begins 'off' by default
    dx9_Light_list[i].src_x                 = src_x;
    dx9_Light_list[i].src_y                 = src_y;
    dx9_Light_list[i].src_z                 = src_z;
    dx9_Light_list[i].dst_x                 = dst_x;
    dx9_Light_list[i].dst_y                 = dst_y;
    dx9_Light_list[i].dst_z                 = dst_z;
    dx9_Light_list[i].range                 = range;
    dx9_Light_list[i].constant_attenuation  = constant_attenuation;
    dx9_Light_list[i].linear_attenuation    = linear_attenuation;
    dx9_Light_list[i].quadratic_attenuation = quadratic_attenuation;
    dx9_Light_list[i].inner_cone_angle      = inner_cone_angle;
    dx9_Light_list[i].outer_cone_angle      = outer_cone_angle;
    dx9_Light_list[i].falloff               = falloff;
    memcpy (&(dx9_Light_list[i].ambient_color_rgba), ambient_color_rgba, 4*sizeof(float));
    memcpy (&(dx9_Light_list[i].diffuse_color_rgba), diffuse_color_rgba, 4*sizeof(float));
    memcpy (&(dx9_Light_list[i].specular_color_rgba), specular_color_rgba, 4*sizeof(float));
    // Initialize this light
    if (Init_Light (i, &dx9_Light_list[i]))
      light = i+1;
  }

  // On any error, release this slot in the light list
  if (light == 0)
    dx9_Light_list[i].type = none;

  return (light);
}

/*___________________________________________________________________
|
|	Function: dx9_UpdateSpotLight
| 
| Output: Updates parameters for a light.
|___________________________________________________________________*/

void dx9_UpdateSpotLight (
  unsigned light,
  float    src_x,
  float    src_y,
  float    src_z,
  float    dst_x,
  float    dst_y,
  float    dst_z,
  float    range,
  float    constant_attenuation,
  float    linear_attenuation,
  float    quadratic_attenuation,  
  float    inner_cone_angle,      // 0 - outer_cone_angle
  float    outer_cone_angle,      // 0 - 180
  float    falloff,               // intensity change from inner cone to outer cone
  float   *ambient_color_rgba,
  float   *diffuse_color_rgba,
  float   *specular_color_rgba )
{
  // Compute index into light list
  int i = light - 1;

  // Is this a valid index into the light list?
  if ((light >= 0) AND (light < MAX_3D_LIGHTS))
    // Is this light active?
    if (dx9_Light_list[i].type == spot) {
      // Save data in light list
      dx9_Light_list[i].src_x                 = src_x;
      dx9_Light_list[i].src_y                 = src_y;
      dx9_Light_list[i].src_z                 = src_z;
      dx9_Light_list[i].dst_x                 = dst_x;
      dx9_Light_list[i].dst_y                 = dst_y;
      dx9_Light_list[i].dst_z                 = dst_z;
      dx9_Light_list[i].range                 = range;
      dx9_Light_list[i].constant_attenuation  = constant_attenuation;
      dx9_Light_list[i].linear_attenuation    = linear_attenuation;
      dx9_Light_list[i].quadratic_attenuation = quadratic_attenuation;
      dx9_Light_list[i].inner_cone_angle      = inner_cone_angle;
      dx9_Light_list[i].outer_cone_angle      = outer_cone_angle;
      dx9_Light_list[i].falloff               = falloff;
      memcpy (&(dx9_Light_list[i].ambient_color_rgba), ambient_color_rgba, 4*sizeof(float));
      memcpy (&(dx9_Light_list[i].diffuse_color_rgba), diffuse_color_rgba, 4*sizeof(float));
      memcpy (&(dx9_Light_list[i].specular_color_rgba), specular_color_rgba, 4*sizeof(float));
      // Initialize this light
      Init_Light (i, &dx9_Light_list[i]);
    }
}

/*___________________________________________________________________
|
|	Function: dx9_InitDirectionLight
| 
| Output: Initializes a light in a disabled state.  Returns 0 on error,
|   else a positive integer handle to the light.
|___________________________________________________________________*/

unsigned dx9_InitDirectionLight (
  float  dst_x,
  float  dst_y,
  float  dst_z, 
  float *ambient_color_rgba,
  float *diffuse_color_rgba,
  float *specular_color_rgba )
{
  int i;
  unsigned light = 0;

  // Find a free slot in the light list
  for (i=0; i<MAX_3D_LIGHTS; i++) 
    if (dx9_Light_list[i].type == 0)
      break;

  // Free slot found?
  if (i < MAX_3D_LIGHTS) {
    // Save data in light list
    dx9_Light_list[i].type  = direction;
    dx9_Light_list[i].on    = FALSE;  // begins 'off' by default
    dx9_Light_list[i].dst_x = dst_x;
    dx9_Light_list[i].dst_y = dst_y;
    dx9_Light_list[i].dst_z = dst_z;
    memcpy (&(dx9_Light_list[i].ambient_color_rgba), ambient_color_rgba, 4*sizeof(float));
    memcpy (&(dx9_Light_list[i].diffuse_color_rgba), diffuse_color_rgba, 4*sizeof(float));
    memcpy (&(dx9_Light_list[i].specular_color_rgba), specular_color_rgba, 4*sizeof(float));
    // Initialize this light
    if (Init_Light (i, &dx9_Light_list[i]))
      light = i+1;
  }

  // On any error, release this slot in the light list
  if (light == 0)
    dx9_Light_list[i].type = none;

  return (light);
}

/*___________________________________________________________________
|
|	Function: dx9_UpdateDirectionLight
| 
| Output: Updates parameters for a light.
|___________________________________________________________________*/

void dx9_UpdateDirectionLight (
  unsigned light,
  float    dst_x,
  float    dst_y,
  float    dst_z, 
  float   *ambient_color_rgba,
  float   *diffuse_color_rgba,
  float   *specular_color_rgba )
{
  // Compute index into light list
  int i = light - 1;

  // Is this a valid index into the light list?
  if ((light >= 0) AND (light < MAX_3D_LIGHTS))
    // Is this light active?
    if (dx9_Light_list[i].type == direction) {
      // Save data in light list
      dx9_Light_list[i].dst_x = dst_x;
      dx9_Light_list[i].dst_y = dst_y;
      dx9_Light_list[i].dst_z = dst_z;
      memcpy (&(dx9_Light_list[i].ambient_color_rgba), ambient_color_rgba, 4*sizeof(float));
      memcpy (&(dx9_Light_list[i].diffuse_color_rgba), diffuse_color_rgba, 4*sizeof(float));
      memcpy (&(dx9_Light_list[i].specular_color_rgba), specular_color_rgba, 4*sizeof(float));
      // Initialize this light
      Init_Light (i, &dx9_Light_list[i]);
    }
}

/*___________________________________________________________________
|
|	Function: dx9_FreeLight
| 
| Output: Disables and frees a light from the light list.
|___________________________________________________________________*/

void dx9_FreeLight (unsigned light)
{
  // Is this a valid index into the light list?
  if ((light > 0) AND (light <= MAX_3D_LIGHTS))
    // Is this plane active?
    if (dx9_Light_list[light-1].type) {
      // Free the light
      Direct3D_EnableLight (light-1, FALSE);
      dx9_Light_list[light-1].type = none;
    }
}

/*___________________________________________________________________
|
|	Function: dx9_EnableLight
| 
| Output: Enables/disables a light.
|___________________________________________________________________*/

void dx9_EnableLight (unsigned light, int flag)
{
  // Is this a valid index into the light list?
  if ((light > 0) AND (light <= MAX_3D_LIGHTS))
    // Is this light active?
    if (dx9_Light_list[light-1].type) {
      dx9_Light_list[light-1].on = flag;
      Direct3D_EnableLight (light-1, flag);
    }
}

/*___________________________________________________________________
|
|	Function: dx9_SetAmbientLight
| 
| Output: Sets the ambient light.
|___________________________________________________________________*/

void dx9_SetAmbientLight (float *rgba)
{
  Direct3D_SetAmbientLight (rgba);
}

/*___________________________________________________________________
|
|	Function: dx9_EnableSpecularLighting
| 
|	Input:
| Output: Enables/disables specular lighting.
|___________________________________________________________________*/

void dx9_EnableSpecularLighting (int flag)
{
  Direct3D_EnableSpecularLighting (flag);
}

/*___________________________________________________________________
|
|	Function: dx9_EnableVertexLighting
| 
|	Input:
| Output: Enables/disables vertex lighting.
|___________________________________________________________________*/

void dx9_EnableVertexLighting (int flag)
{
  Direct3D_EnableVertexLighting (flag);
}

/*___________________________________________________________________
|
|	Function: dx9_EnableFog
| 
|	Input:
| Output: Enables/disables fog.
|___________________________________________________________________*/

void dx9_EnableFog (int flag)
{
  Direct3D_EnableFog (flag);
}

/*___________________________________________________________________
|
|	Function: dx9_SetFogColor
| 
|	Input:
| Output: Sets the fog color.
|___________________________________________________________________*/

void dx9_SetFogColor (byte r, byte g, byte b)
{
  Direct3D_SetFogColor (r, g, b);
}

/*___________________________________________________________________
|
|	Function: dx9_SetLinearPixelFog
| 
|	Input:
| Output: Sets the fog formula.
|___________________________________________________________________*/

void dx9_SetLinearPixelFog (float start_distance, float end_distance)
{
  Direct3D_SetLinearPixelFog (start_distance, end_distance);
}

/*___________________________________________________________________
|
|	Function: dx9_SetExpPixelFog
| 
|	Input:
| Output: Sets the fog formula.
|___________________________________________________________________*/

void dx9_SetExpPixelFog (float density)
{
  Direct3D_SetExpPixelFog (density);
}

/*___________________________________________________________________
|
|	Function: dx9_SetExp2PixelFog
| 
|	Input:
| Output: Sets the fog formula.
|___________________________________________________________________*/

void dx9_SetExp2PixelFog (float density)
{
  Direct3D_SetExp2PixelFog (density);
}

/*___________________________________________________________________
|
|	Function: dx9_SetLinearVertexFog
| 
|	Input:
| Output: Sets the fog formula.
|___________________________________________________________________*/

void dx9_SetLinearVertexFog (float start_distance, float end_distance, int ranged_based)
{
  Direct3D_SetLinearVertexFog (start_distance, end_distance, ranged_based);
}

/*___________________________________________________________________
|
|	Function: dx9_SetMaterial
| 
|	Input:
| Output: Sets the current render material.
|___________________________________________________________________*/

void dx9_SetMaterial (
  float *ambient_color_rgba,
  float *diffuse_color_rgba,
  float *specular_color_rgba,
  float *emissive_color_rgba,
  float  specular_sharpness )
{
  Direct3D_SetMaterial (ambient_color_rgba, diffuse_color_rgba, specular_color_rgba, emissive_color_rgba, specular_sharpness);
}

/*___________________________________________________________________
|
|	Function: dx9_GetMaterial
| 
|	Input:
| Output: Gets the current render material.
|___________________________________________________________________*/

void dx9_GetMaterial (
  float *ambient_color_rgba,
  float *diffuse_color_rgba,
  float *specular_color_rgba,
  float *emissive_color_rgba,
  float *specular_sharpness )
{
  Direct3D_GetMaterial (ambient_color_rgba, diffuse_color_rgba, specular_color_rgba, emissive_color_rgba, specular_sharpness);
}

/*___________________________________________________________________
|
|	Function: dx9_InitTexture
| 
| Output: Initializes a static 3D texture from an image buffer.  Returns  
|   a handle to the texture or 0 on any error.
|
| Description: Texture dimensions should always be a power of 2 and 
|   should be square.  Max size should be 256x256.
|___________________________________________________________________*/

byte *dx9_InitTexture (
  int       num_mip_levels,
  byte    **image, 
  byte    **alphamap,
  int       dx, 
  int       dy, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size )
{
  return (Direct3D_InitTexture (num_mip_levels, image, alphamap, dx, dy, num_color_bits, num_alpha_bits, size));
}

/*___________________________________________________________________
|
|	Function: dx9_InitVolumeTexture
| 
| Output: Initializes a static 3D volume texture from an image buffer.
|   Returns a handle to the texture or 0 on any error.
|
| Description: Texture dimensions should always be a power of 2 and 
|   should be square.  Max size should be 256x256.
|___________________________________________________________________*/

byte *dx9_InitVolumeTexture (
  int       num_levels,
  int       num_slices,
  byte    **image, 
  byte    **alphamap,
  int       dx, 
  int       dy, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size)
{
  return (Direct3D_InitVolumeTexture (num_levels, num_slices, image, alphamap, dx, dy, num_color_bits, num_alpha_bits, size));
}

/*___________________________________________________________________
|
|	Function: dx9_InitCubemapTexture
| 
| Output: Initializes a static 3D texture from an image buffer.  Returns  
|   a handle to the texture or 0 on any error.
|
| Description: Texture dimensions should always be a power of 2 and 
|   should be square.  Max size should be 256x256.
|___________________________________________________________________*/

byte *dx9_InitCubemapTexture (
  byte    **image, 
  byte    **alphamap,
  int       dimensions, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size )
{
  return (Direct3D_InitCubemapTexture (image, alphamap, dimensions, num_color_bits, num_alpha_bits, size));
}

/*____________________________________________________________________
|
| Function: dx9_InitDynamicTexture
|
| Output: Initializes a square dynamic texture.  Returns positive integer
|   handle to the texture or 0 on any error.
|___________________________________________________________________*/

unsigned dx9_InitDynamicTexture (
  int       dx, 
  int       dy, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size )
{
  int i;
  unsigned texture = 0;

  // Find a free slot in the list
  for (i=0; dx9_Dynamic_Texture_list[i].type AND (i<MAX_DYNAMIC_TEXTURES); i++);

  // Free slot found?
  if (i < MAX_DYNAMIC_TEXTURES) {
    if (Init_Dynamic_Texture (i, DYNAMIC_TEXTURE_TYPE_SQUARE, dx, dy, num_color_bits, num_alpha_bits, size))
      texture = i+1;
    else
      DEBUG_ERROR ("dx9_InitDynamicTexture(): ERROR creating a dynamic texture")
  }

  return (texture);
}

/*____________________________________________________________________
|
| Function: dx9_InitDynamicCubemapTexture
|
| Output: Initializes a cubemap dynamic texture.  Returns positive 
|   integer handle to the texture or 0 on any error.
|___________________________________________________________________*/

unsigned dx9_InitDynamicCubemapTexture (
  int       dimensions, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size )
{
  int i;
  unsigned texture = 0;

  // Find a free slot in the list
  for (i=0; dx9_Dynamic_Texture_list[i].type AND (i<MAX_DYNAMIC_TEXTURES); i++);

  // Free slot found?
  if (i < MAX_DYNAMIC_TEXTURES) {
    if (Init_Dynamic_Texture (i, DYNAMIC_TEXTURE_TYPE_CUBEMAP, dimensions, dimensions, num_color_bits, num_alpha_bits, size))
      texture = i+1;
    else
      DEBUG_ERROR ("dx9_InitDynamicCubemapTexture(): ERROR creating a dynamic texture")
  }

  return (texture);
}
  
/*____________________________________________________________________
|
| Function: Init_Dynamic_Texture
|
| Input: Called from dx9_InitDynamicTexture(), 
|                    dx9_InitDynamicCubemapTexture(),
|                    dx9_Restore()
|
|   Returns true if created, else false on any error.
|___________________________________________________________________*/

static int Init_Dynamic_Texture (
  int index,          // index into dynamic texture list to put data
  int type,
  int dx, 
  int dy,
  int num_color_bits,    
  int num_alpha_bits, 
  unsigned *size )
{
  int i, n;
  byte *texture;
  int created = FALSE;

  // See how many empty entries are in page list
  for (i=0,n=0; (i<MAX_PAGES) AND (n<6); i++)
    if (dx9_Page_list[i].type == 0)
      n++;

  switch (type) {
    case DYNAMIC_TEXTURE_TYPE_SQUARE:
      // Make sure there is space in page list
      if (n >= 1) {
        // Create the texture
        texture = Direct3D_InitTexture (1, NULL, NULL, dx, dy, num_color_bits, num_alpha_bits, size);
        // Was texture created?
        if (texture) {
          dx9_Dynamic_Texture_list[index].type           = type;
          dx9_Dynamic_Texture_list[index].dx             = dx;
          dx9_Dynamic_Texture_list[index].dy             = dy;
          dx9_Dynamic_Texture_list[index].num_color_bits = num_color_bits;
          dx9_Dynamic_Texture_list[index].num_alpha_bits = num_alpha_bits;
          dx9_Dynamic_Texture_list[index].texture        = texture;
          // Get surface interface
          dx9_Dynamic_Texture_list[index].surface[0] = Direct3D_GetTextureSurface (texture);
          created = TRUE;
        }
      }
      break;
    case DYNAMIC_TEXTURE_TYPE_CUBEMAP:
      // Make sure there is space in page list
      if (n >= 6) {
        // Create the texture
        texture = Direct3D_InitCubemapTexture (NULL, NULL, dx, num_color_bits, num_alpha_bits, size);
        // Was texture created?
        if (texture) {
          dx9_Dynamic_Texture_list[index].type           = type;
          dx9_Dynamic_Texture_list[index].dx             = dx;
          dx9_Dynamic_Texture_list[index].dy             = dx;
          dx9_Dynamic_Texture_list[index].num_color_bits = num_color_bits;
          dx9_Dynamic_Texture_list[index].num_alpha_bits = num_alpha_bits;
          dx9_Dynamic_Texture_list[index].texture        = texture;
          // Get surface interfaces
          for (i=0; i<6; i++)
            dx9_Dynamic_Texture_list[index].surface[i] = Direct3D_GetTextureCubemapSurface (texture, i);
          created = TRUE;
        }   
      }
      break;
  }

  // Add this info to page list
//  if (created)
//    Add_Dynamic_Texture_To_Page_List (index, type);

  return (created);
}

/*____________________________________________________________________
|
| Function: Add_Dynamic_Texture_To_Page_List
|
| Input: Called from Init_Dynamic_Texture()
| Output: Adds an entry into page list for this dynamic texture.  If 
|   the texture is a cubemap, adds 6 entries to page list. 
|___________________________________________________________________*/

static void Add_Dynamic_Texture_To_Page_List (
  int dynamic_texture,  // index into dynamic texture list
  int type )            // type of texture (square or cubemap)
{
  int i, n;

  switch (type) {

/*____________________________________________________________________
|
| Add entry for a square texture
|___________________________________________________________________*/

    case DYNAMIC_TEXTURE_TYPE_SQUARE:
      // Look for an empty entry in page list
      for (i=0; dx9_Page_list[i].type; i++);
      // Set info in page list
      dx9_Page_list[i].type    = PAGE_TYPE_DYNAMIC_TEXTURE;
      dx9_Page_list[i].dx      = dx9_Dynamic_Texture_list[dynamic_texture].dx;
      dx9_Page_list[i].dy      = dx9_Dynamic_Texture_list[dynamic_texture].dy;
      dx9_Page_list[i].surface = dx9_Dynamic_Texture_list[dynamic_texture].surface[0];
      // Set info in dynamic texture
      dx9_Dynamic_Texture_list[dynamic_texture].page[0] = i;
      break;

/*____________________________________________________________________
|
| Add entry for a cubemap texture
|___________________________________________________________________*/

    case DYNAMIC_TEXTURE_TYPE_CUBEMAP:
      for (n=0; n<6; n++) {
        // Look for an empty entry in page list
        for (i=0; dx9_Page_list[i].type; i++);
        // Set info in page list
        dx9_Page_list[i].type    = PAGE_TYPE_DYNAMIC_TEXTURE;
        dx9_Page_list[i].dx      = dx9_Dynamic_Texture_list[dynamic_texture].dx;
        dx9_Page_list[i].dy      = dx9_Dynamic_Texture_list[dynamic_texture].dy;
        dx9_Page_list[i].surface = dx9_Dynamic_Texture_list[dynamic_texture].surface[n];
        // Set info in dynamic texture
        dx9_Dynamic_Texture_list[dynamic_texture].page[n] = i;
      }
      break;
  }
}

/*____________________________________________________________________
|
| Function: dx9_FreeTexture
|
| Output: Frees a 3D texture created by dx9_InitTexture().
|___________________________________________________________________*/

void dx9_FreeTexture (byte *texture)
{
  Direct3D_FreeTexture (texture);
}

/*____________________________________________________________________
|
| Function: dx9_FreeDynamicTexture
|
| Output: Frees a dynamic texture.
|___________________________________________________________________*/

void dx9_FreeDynamicTexture (unsigned texture)
{
  // Is this a valid index into the dynamic texture list?
  if ((texture > 0) AND (texture <= MAX_DYNAMIC_TEXTURES))
    // Is this texture index active?
    if (dx9_Dynamic_Texture_list[texture-1].type) {
      Free_Dynamic_Texture (texture-1);
      dx9_Dynamic_Texture_list[texture-1].type = 0;
    }
}

/*____________________________________________________________________
|
| Function: Free_Dynamic_Texture
|
| Output: Called from dx9_FreeDynamicTexture(), dx9_Restore()
|___________________________________________________________________*/

static void Free_Dynamic_Texture (unsigned index)
{
  int i;

  switch (dx9_Dynamic_Texture_list[index].type) {
    case DYNAMIC_TEXTURE_TYPE_SQUARE:
      // Free previously created interfaces
      Direct3D_Free_Surface (dx9_Dynamic_Texture_list[index].surface[0]);
      Direct3D_FreeTexture (dx9_Dynamic_Texture_list[index].texture);
      break;
    case DYNAMIC_TEXTURE_TYPE_CUBEMAP:
      // Free previously created interfaces
      for (i=0; i<6; i++)
        Direct3D_Free_Surface (dx9_Dynamic_Texture_list[index].surface[i]);
      Direct3D_FreeTexture (dx9_Dynamic_Texture_list[index].texture);
      break;
  }

  // Free this info from page list
//  Remove_Dynamic_Texture_From_Page_List (index, dx9_Dynamic_Texture_list[index].type);
}

/*____________________________________________________________________
|
| Function: Remove_Dynamic_Texture_From_Page_List
|
| Input: Called from Free_Dynamic_Texture()
| Output: Removes entries in page list for this dynamic texture.
|___________________________________________________________________*/

static void Remove_Dynamic_Texture_From_Page_List (
  int dynamic_texture,  // index into dynamic texture list
  int type )            // type of texture (square or cubemap)
{
  int num_faces, face;

  if (type == DYNAMIC_TEXTURE_TYPE_SQUARE)
    num_faces = 1;
  else if (type == DYNAMIC_TEXTURE_TYPE_CUBEMAP)
    num_faces = 6;

  for (face=0; face<num_faces; face++) {
    dx9_Page_list[dx9_Dynamic_Texture_list[dynamic_texture].page[face]].type    = 0;
    dx9_Page_list[dx9_Dynamic_Texture_list[dynamic_texture].page[face]].surface = NULL;
  }
}

/*____________________________________________________________________
|
| Function: dx9_SetTexture
|
| Output: Sets the current render texture.
|___________________________________________________________________*/

void dx9_SetTexture (int stage, byte *texture)
{
  Direct3D_SetTexture (stage, texture);
}

/*____________________________________________________________________
|
| Function: dx9_SetDynamicTexture
|
| Output: Sets the current render texture.
|___________________________________________________________________*/

void dx9_SetDynamicTexture (int stage, unsigned texture)
{
  // Is this a valid index into the dynamic texture list?
  if ((texture > 0) AND (texture <= MAX_DYNAMIC_TEXTURES))
    // Is this texture index active?
    if (dx9_Dynamic_Texture_list[texture-1].type) 
      // Set this as the current render texture
      Direct3D_SetTexture (stage, dx9_Dynamic_Texture_list[texture-1].texture);
}

/*____________________________________________________________________
|
| Function: dx9_SetTextureAddressingMode
|
| Output: Sets addressing mode of a texture stage (0-7).
|   Addressing mode can be one of these types:
|
|     dx9_TEXTURE_ADDRESSMODE_WRAP   
|     dx9_TEXTURE_ADDRESSMODE_MIRROR 
|     dx9_TEXTURE_ADDRESSMODE_CLAMP  
|     dx9_TEXTURE_ADDRESSMODE_BORDER 
|     dx9_TEXTURE_ADDRESSMODE_MIRRORONCE 
|
|  Dimensions can be one or both of these flags:
|
|     vx3d_TEXTURE_DIMENSION_U
|     vx3d_TEXTURE_DIMENSION_V
|     vx3d_TEXTURE_DIMENSION_W
|___________________________________________________________________*/

void dx9_SetTextureAddressingMode (int stage, int dimension, int addressing_mode)
{
  Direct3D_SetTextureAddressingMode (stage, dimension, addressing_mode);
}

/*____________________________________________________________________
|
| Function: dx9_SetTextureBorderColor
|
| Output: Sets border color for a texture stage (0-7).
|___________________________________________________________________*/

void dx9_SetTextureBorderColor (int stage, byte r, byte g, byte b, byte a)
{
  Direct3D_SetTextureBorderColor (stage, r, g, b, a);
}

/*____________________________________________________________________
|
| Function: dx9_SetTextureFiltering
|
| Output: Sets border color for a texture stage (0-7).  Filtering can
|   be one of these types:
|
|     dx9_TEXTURE_FILTERTYPE_POINT       
|     dx9_TEXTURE_FILTERTYPE_LINEAR      
|     dx9_TEXTURE_FILTERTYPE_ANISOTROPIC 
|
| Description: If anisotropic filtering is supported, the anisotropy_level
|   defines the amount of filtering desired from 1 (lowest) to 100 (highest).
|   A low value will render more quickly.  A high value will produce the best
|   quality.
|___________________________________________________________________*/

void dx9_SetTextureFiltering (
  int stage, 
  int filter_type, 
  int anisotropy_level )  // amount of filtering (1-100), 1=min, 100=max
{
  Direct3D_SetTextureFiltering (stage, filter_type, anisotropy_level);
}

/*___________________________________________________________________
|
|	Function: dx9_SetTextureCoordinates
| 
|	Input:
| Output: Sets the set (0-7) of texture coordinates in the object to 
|     use for this texture stage. (-1=cubemap)
|___________________________________________________________________*/

void dx9_SetTextureCoordinates (
  int stage,              // texture blending stage 0-7
  int coordinate_stage )  // set of texture coordinates in the object 0-7 (-1=cubemap)
{
  Direct3D_SetTextureCoordinates (stage, coordinate_stage);
}

/*____________________________________________________________________
|
| Function: dx9_SetTextureCoordinateWrapping
|
| Output: Sets texture wrapping for a set of texture coordinates in an
|   object.
|___________________________________________________________________*/

void dx9_SetTextureCoordinateWrapping (
  int coordinate_stage, // set of texture coordinates in the object 0-7
  int wrap_s,           // boolean (u dimension)
  int wrap_t,           // boolean (v dimension)
  int wrap_r,           // boolean
  int wrap_q )          // boolean
{
  Direct3D_SetTextureCoordinateWrapping (coordinate_stage, wrap_s, wrap_t, wrap_r, wrap_q);
}

/*____________________________________________________________________
|
| Function: dx9_SetTextureFactor
|
| Output: Sets texture factor used by some texture blending operations.
|___________________________________________________________________*/

void dx9_SetTextureFactor (byte r, byte g, byte b, byte a)
{
  Direct3D_SetTextureFactor (r, g, b, a);
}

/*____________________________________________________________________
|
| Function: dx9_PreLoadTexture
|
| Output: Manually loads a texture into video memory.
|___________________________________________________________________*/

void dx9_PreLoadTexture (byte *texture)
{
  Direct3D_PreLoadManagedTexture (texture);
}

/*____________________________________________________________________
|
| Function: dx9_EvictAllTextures
|
| Output: Evicts all textures from texture video memory.
|___________________________________________________________________*/

void dx9_EvictAllTextures ()
{
  Direct3D_EvictManagedTextures ();
}

/*____________________________________________________________________
|
| Function: dx9_EnableRenderToTexture
|
| Output: Enables a texture as the rendering target so caller can render
|   to it.  If texture == 0, sets rendering target back to screen.
|   If the texture is a cubemap, render target will be set to face in 
|   the cubemap according to:
|
|     0 = right
|     1 = left
|     2 = top
|     3 = bottom
|     4 = front
|     5 = back
|___________________________________________________________________*/

void dx9_EnableRenderToTexture (unsigned texture, int face)
{
  // Set render target to screen?
  if (texture == 0) 
    Direct3D_Set_Active_Page (dx9_Page_list[dx9_Active_page].surface, FALSE);
  // Set render target to square texture?
  else if (dx9_Dynamic_Texture_list[texture-1].type == DYNAMIC_TEXTURE_TYPE_SQUARE)
    Direct3D_Set_Active_Page (dx9_Dynamic_Texture_list[texture-1].surface[0], TRUE);
  // Set render target to cubemap texture face?
  else if (dx9_Dynamic_Texture_list[texture-1].type == DYNAMIC_TEXTURE_TYPE_CUBEMAP) 
    Direct3D_Set_Active_Page (dx9_Dynamic_Texture_list[texture-1].surface[face], TRUE);

//  // Set render target to square texture?
//  if (dx9_Dynamic_Texture_list[texture-1].type == DYNAMIC_TEXTURE_TYPE_SQUARE) {
//    if (Direct3D_Set_Active_Page (dx9_Dynamic_Texture_list[texture-1].surface[0], TRUE))
//      dx9_Active_page = dx9_Dynamic_Texture_list[texture-1].page[0];
//  }
//  // Set render target to cubemap texture face?
//  else if (dx9_Dynamic_Texture_list[texture-1].type == DYNAMIC_TEXTURE_TYPE_CUBEMAP) {
//    if (Direct3D_Set_Active_Page (dx9_Dynamic_Texture_list[texture-1].surface[face], TRUE)) 
//      dx9_Active_page = dx9_Dynamic_Texture_list[texture-1].page[face];
//  }
}

/*____________________________________________________________________
|
| Function: dx9_SetTextureColorOp
|
| Output: Sets the texture blending color operation:
|     default for stage 0 is MODULATE, all other stages is DISABLE
|     default arg1 is TEXTURE
|     default arg2 is CURRENT
|___________________________________________________________________*/

void dx9_SetTextureColorOp (int stage, int texture_colorop, int texture_arg1, int texture_arg2)
{
  Direct3D_SetTextureColorOp (stage, texture_colorop, texture_arg1, texture_arg2);
}

/*____________________________________________________________________
|
| Function: dx9_SetTextureAlphaOp
|
| Output: Sets the texture blending alpha operation.
|     default for stage 0 is SELECTARG1, all other stages is DISABLE
|     default arg1 is TEXTURE
|     default arg2 is CURRENT
|___________________________________________________________________*/

void dx9_SetTextureAlphaOp (int stage, int texture_alphaop, int texture_arg1, int texture_arg2)
{
  Direct3D_SetTextureAlphaOp (stage, texture_alphaop, texture_arg1, texture_arg2);
}

/*____________________________________________________________________
|
| Function: dx9_SetTextureColorFactor
|
| Output: Sets the texture blending color factor.  This is the color used
|     for multiple texture blending w/ TFACTOR blending arg or 
|     BLENDFACTORALPHA operation.
|___________________________________________________________________*/

void dx9_SetTextureColorFactor (float *rgba)
{
  Direct3D_SetTextureColorFactor (rgba);
}

/*____________________________________________________________________
|
| Function: dx9_EnableCubemapTextureReflections
|
| Output: Enables/disables correct cubemap reflection processing.
|___________________________________________________________________*/

void dx9_EnableCubemapTextureReflections (int flag)
{
  Direct3D_EnableCubemapTextureReflections (flag);
}

/*____________________________________________________________________
|
| Function: dx9_EnableAlphaBlending
|
| Output: Enables/disables alpha-blending.
|___________________________________________________________________*/

void dx9_EnableAlphaBlending (int flag)
{
  Direct3D_EnableAlphaBlending (flag);
}

/*____________________________________________________________________
|
| Function: dx9_SetAlphaBlendFactor
|
| Output: Sets the alpha blending factors:
|   pixel_color = (src_pixel * src_blend_factor) + 
|                 (dst_pixel * dst_blend_factor)
|___________________________________________________________________*/

void dx9_SetAlphaBlendFactor (int src_blend_factor, int dst_blend_factor)
{
  Direct3D_SetAlphaBlendFactor (src_blend_factor, dst_blend_factor);
}

/*____________________________________________________________________
|
| Function: dx9_AlphaTestingAvailable
|
| Output: Returns true if alpha testing is available using the greater
|     than or equal to alpha reference value test.
|___________________________________________________________________*/

int dx9_AlphaTestingAvailable (void)
{
  return (Direct3D_AlphaTestingAvailable ());
}

/*____________________________________________________________________
|
| Function: dx9_EnableAlphaTesting
|
| Output: Enables/disables alpha testing, if supported.  Reference value
|     should be 0 - 1.  With alpha testing enabled, a pixel will only be
|     written if its alpha value is greater than or equal to the alpha
|     value already at that screen pixel.
|___________________________________________________________________*/

void dx9_EnableAlphaTesting (int flag, byte reference_value)
{
  Direct3D_EnableAlphaTesting (flag, reference_value);
}

/*___________________________________________________________________
|
|	Function: dx9_StartEvents
| 
|	Input:
| Output:	
|___________________________________________________________________*/

int dx9_StartEvents (int use_keyboard, int use_mouse)
{
  int started = FALSE;

  if (use_keyboard OR use_mouse)
    if (DirectInput_Init (use_keyboard, use_mouse)) {
      direct_input_initialized = TRUE;
      started = TRUE;
    }

  return (started);
}

/*___________________________________________________________________
|
|	Function: dx9_StopEvents
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_StopEvents (void)
{
	DirectInput_Free ();
  direct_input_initialized = FALSE;
}

/*___________________________________________________________________
|
|	Function: dx9_FlushEvents
| 
|	Input:
| Output:	Flushes all keyboard and mouse events from event queue.  Does
|					not flush windows events.
|___________________________________________________________________*/

void dx9_FlushEvents (void)
{
  DirectInput_Flush_Events (evTYPE_KEY_PRESS |
                            evTYPE_RAW_KEY_PRESS |
                            evTYPE_RAW_KEY_RELEASE |
														evTYPE_MOUSE_LEFT_PRESS | 
														evTYPE_MOUSE_LEFT_RELEASE |
														evTYPE_MOUSE_RIGHT_PRESS | 
														evTYPE_MOUSE_RIGHT_RELEASE );
}

/*____________________________________________________________________
|
| Function: dx9_GetEvent
|
| Outputs: Returns info about an event is one is ready.  Returns true
|						if event was ready else false. 
|___________________________________________________________________*/

int dx9_GetEvent (unsigned *type, int *keycode, int *x, int *y)
{
  unsigned timestamp;
	int input_ready = FALSE;

  input_ready = DirectInput_Get_Event (type, keycode, x, y, &timestamp);

	return (input_ready);
}

/*___________________________________________________________________
|
|	Function: dx9_MouseFlushBuffer
| 
|	Input:
| Output:	Flushes all mouse events from event queue.
|___________________________________________________________________*/

void dx9_MouseFlushBuffer (void)
{
	DirectInput_Flush_Events (evTYPE_MOUSE_LEFT_PRESS | 
														evTYPE_MOUSE_LEFT_RELEASE |
														evTYPE_MOUSE_RIGHT_PRESS | 
														evTYPE_MOUSE_RIGHT_RELEASE);
}

/*___________________________________________________________________
|
|	Function: dx9_MouseHide
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_MouseHide (void)
{
	DirectInput_Mouse_Hide ();
}

/*___________________________________________________________________
|
|	Function: dx9_MouseShow
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_MouseShow (void)
{
	DirectInput_Mouse_Show ();
}

/*___________________________________________________________________
|
|	Function: dx9_MouseConfine
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_MouseConfine (int left, int top, int right, int bottom)
{
	DirectInput_Mouse_Confine (left, top, right, bottom);
}

/*___________________________________________________________________
|
|	Function: dx9_MouseGetStatus
| 
|	Input:
| Output:	Returns the current state of the mouse.  Assumes mouse has
|					been activated with dx9_StartEvents().
|___________________________________________________________________*/

int dx9_MouseGetStatus (int *x, int *y, int *button)
{
	int left, right;

	DirectInput_Mouse_Get_Status (x, y, &left, &right);
	*button = 0;
	if (left)
		*button |= 0x1;
	if (right)
		*button |= 0x2;

	return (TRUE);
}

/*___________________________________________________________________
|
|	Function: dx9_MouseSetCoords
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_MouseSetCoords (int x, int y)
{
	DirectInput_Mouse_Set_Coords (x, y);
}

/*___________________________________________________________________
|
|	Function: dx9_MouseGetCoords
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_MouseGetCoords (int *x, int *y)
{
	DirectInput_Mouse_Get_Coords (x, y);
}

/*___________________________________________________________________
|
|	Function: dx9_MouseGetMovement
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_MouseGetMovement (int *x, int *y)
{
  DirectInput_Mouse_Get_Movement (x, y);
}
	
/*___________________________________________________________________
|
|	Function: dx9_MouseSetBitmapCursor
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_MouseSetBitmapCursor (
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
  DirectInput_Mouse_Set_Bitmap_Cursor (cursor_bitmap, mask_bitmap, bitmap_dx, bitmap_dy,
																			 hot_x, hot_y, 
                                       cursor_color_r, cursor_color_g, cursor_color_b,
                                       mask_color_r, mask_color_g, mask_color_b );
}

/*___________________________________________________________________
|
|	Function: dx9_MouseSetImageCursor
| 
|	Input:
| Output:	
|___________________________________________________________________*/

void dx9_MouseSetImageCursor (
	byte *image, 
	int   image_dx,
	int   image_dy,
	int		hot_x,
	int		hot_y )
{
	DirectInput_Mouse_Set_Image_Cursor (image, image_dx, image_dy, hot_x, hot_y);
}
