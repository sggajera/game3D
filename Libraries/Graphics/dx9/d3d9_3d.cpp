/*___________________________________________________________________
|
|	File: d3d9_3d.cpp
|
|	Description: Functions that interface to Direct3D9.
|                                 
| Functions:	d3d9_QueryHardware            
|							 Build_Video_Modes_Array      
|							  Compare_Modes               
|             d3d9_UserSelectMode           
|             d3d9_SetMode                  
|              Depth_Format_Available       
|              Attach_Depth_Buffer
|              Get_Pixel_Format_Data
|             d3d9_Restore    
|              Reset_D3D_Device
|						  d3d9_Free                     
|
|             d3d9_GetScreenDimensions      
|             d3d9_GetPixelSize             
|
|             d3d9_BeginRender              
|             d3d9_EndRender                
|             d3d9_SetFillMode              
|             d3d9_GetDriverInfo            
|             d3d9_InitObject
|             d3d9_FreeObject
|             d3d9_DrawObject
|              Draw_Object
|             d3d9_SetViewport              
|             d3d9_ClearViewportRectangle   
|             d3d9_EnableClipping
|             d3d9_InitClipPlane            
|             d3d9_EnableClipPlane          
|             d3d9_SetWorldMatrix           
|             d3d9_GetWorldMatrix           
|             d3d9_SetViewMatrix            
|             d3d9_GetViewMatrix            
|             d3d9_SetProjectionMatrix      
|             d3d9_GetProjectionMatrix      
|             d3d9_EnableTextureMatrix      
|             d3d9_SetTextureMatrix         
|             d3d9_GetTextureMatrix         
|             d3d9_EnableZBuffer            
|             d3d9_EnableBackfaceRemoval                 
|             d3d9_EnableStencilBuffer      
|             d3d9_SetStencilFailOp         
|             d3d9_SetStencilZFailOp        
|             d3d9_SetStencilPassOp         
|             d3d9_SetStencilComparison     
|             d3d9_SetStencilReferenceValue 
|             d3d9_SetStencilMask           
|             d3d9_SetStencilWriteMask      
|             d3d9_EnableLighting           
|             d3d9_InitPointLight           
|             d3d9_InitSpotLight            
|             d3d9_InitDirectionLight       
|             d3d9_EnableLight              
|             d3d9_SetAmbientLight          
|             d3d9_EnableSpecularLighting   
|             d3d9_EnableVertexLighting     
|             d3d9_EnableFog                
|             d3d9_SetFogColor              
|             d3d9_SetLinearPixelFog        
|             d3d9_SetExpPixelFog           
|             d3d9_SetExp2PixelFog          
|             d3d9_SetLinearVertexFog       
|             d3d9_SetMaterial              
|             d3d9_GetMaterial              
|             d3d9_InitTexture              
|             d3d9_InitVolumeTexture
|             d3d9_InitCubemapTexture
|              Copy_Pixels_To_Texture       
|             d3d9_FreeTexture                    
|             d3d9_SetTexture                     
|             d3d9_SetTextureAddressingMode       
|             d3d9_SetTextureBorderColor          
|             d3d9_SetTextureFiltering            
|             d3d9_SetTextureCoordinates          
|             d3d9_SetTextureCoordinateWrapping
|             d3d9_SetTextureFactor   
|             d3d9_PreLoadManagedTexture          
|             d3d9_EvictManagedTextures           
|             d3d9_SetTextureColorOp              
|             d3d9_SetTextureAlphaOp              
|             d3d9_SetTextureColorFactor          
|             d3d9_EnableCubemapTextureReflections
|             d3d9_GetTextureSurface
|             d3d9_GetTextureCubemapSurface           
|
|             d3d9_EnableAlphaBlending            
|             d3d9_SetAlphaBlendFactor            
|             d3d9_AlphaTestingAvailable          
|             d3d9_EnableAlphaTesting             
|
|             d3d9_GetRGBFormat                   
|             d3d9_SetActivePage                  
|             d3d9_FlipVisualPage                 
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define DIRECT3D_VERSION 0x0900

// This library doesn't need to use the multithreaded version of D3D but it avoids a lot of D3D debug output, so keep it for now
#ifdef _DEBUG
#define USE_MULTITHREADED_VERSION_OF_DIRECT3D
#endif

//#define ENABLE_CLOUD_SHADERS

/*____________________
|
| Include files
|___________________*/

#include <first_header.h>

#include <stdio.h>
#include <process.h>   
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <malloc.h>
#include <process.h>
#include <math.h>
#include <assert.h>

#include <windows.h>  // these may need to go first (above)
#include <mmsystem.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9shader.h>

#include <defines.h>
#include <clib.h>
#include <events.h>
#include <win_support.h>

#include "d3d9_dp.h"
#include "d3d9_3d.h"

/*____________________
|
| Type definitions
|___________________*/

typedef struct {
  D3DFORMAT  zbuffer_format;
  int       *restored;          // pointer to a boolean
  int       *reset_in_progress; // pointer to a boolean
} RestoreParams;

/*____________________
|
| Function prototypes
|___________________*/

static int Build_Video_Modes_Array (
  D3DDISPLAYMODE *mode_desc, 
  int             num_modes, 
  unsigned        acceptable_resolutions, 
  unsigned        acceptable_bitdepths,
  unsigned        adapter,
  D3DDEVTYPE      dev_type );
static int Compare_Video_Modes (const void *arg1, const void *arg2);
static int Depth_Format_Available (D3DFORMAT depth_format, D3DFORMAT adapter_format, D3DFORMAT backbuffer_format);
static int Attach_Depth_Buffer (D3DFORMAT depth_format);
static void Get_Pixel_Format_Data (
  D3DFORMAT texture_format,
  WORD     *low_red_bit,  // bits are numbered starting with 0 as low order bit
  WORD     *low_green_bit,
  WORD     *low_blue_bit,
  WORD     *low_alpha_bit,
  WORD     *num_red_bits,
  WORD     *num_green_bits,
  WORD     *num_blue_bits,
  WORD     *num_alpha_bits,
  DWORD    *red_mask,
  DWORD    *green_mask,
  DWORD    *blue_mask,
  DWORD    *alpha_mask, 
  int      *pixel_size );
static void Reset_D3D_Device (void *params);
static void Draw_Object (
  void     *vertex_buffer,
  void     *index_buffer, 
  int       num_vertices,
  unsigned  vertex_size,
  int       num_surfaces,
  DWORD     fvfcode );
static void Copy_Pixels_To_Texture (
	byte     *image,     // NULL if none
  byte     *alphamap,  // NULL if none
  int       dx,
  int       dy,
  byte     *surfdata,
  int       surfpitch, 
  D3DFORMAT texture_format );


// Defined in d3d9_2d.cpp
extern DWORD d3d9_RGBA_To_Pixel (byte r, byte g, byte b, byte a);	
extern void	d3d9_Pixel_To_RGB (DWORD pixel, byte *r, byte *g, byte *b);

/*____________________
|
|	Global variables
|___________________*/
																				
static LPDIRECT3D9  d3d9;                 // pointer to the D3D object interface
static D3DCAPS9     d3dcaps9;             // capabilities of the D3D object/device
                                          // screen page
static int          d3d_adapter;
static D3DDEVTYPE   d3d_device_type;
static int          d3d_num_video_modes;  // # modes in array
static int          d3d_num_stencil_bits;
static D3DFORMAT    d3d_zbuffer_format;
static int          d3d_num_vram_pages;

// Info about application window location, in screen-relative coords
int d3d_app_window_xleft;
int d3d_app_window_ytop;

#ifdef _DEBUG
static char debug_str[256];
#endif

/*___________________________________________________________________
|
|	Function: d3d9_QueryHardware
| 
|	Input: Called from dx9_GetUserFormat(), dx9_Init()
|
| Output: Queries the hardware for info on what modes are supported
|   by a particular adapter and device (hardware or software rasterizer).
|   Builds a global array of available video modes.  
|
|   If more than one video adapter exists in the system, pops up a 
|   dialog box to allow user to select an adapter.
|
|   Returns true on sucess, else false on any error.
|___________________________________________________________________*/

int d3d9_QueryHardware (
  unsigned acceptable_resolutions,
  unsigned acceptable_bitdepths,
  int      enable_hardware_acceleration )
{
  int i, j, n, num_adapters, num_modes;
  char **adapter_names;
  D3DADAPTER_IDENTIFIER9 *adapter_desc;
//  D3DFORMAT requested_format;
  D3DDISPLAYMODE *mode_desc;
	int query_ok = FALSE;

/*___________________________________________________________________
|
| Init globals
|___________________________________________________________________*/

  d3d9                   = NULL;
  d3ddevice9             = NULL;
  d3dscreen9             = NULL;
  d3dzbuffer9            = NULL;
  d3dcursor9             = NULL;
  d3d_adapter            = -1;
  d3d9_video_modes        = NULL;
  d3d_num_video_modes    = 0;
  d3d9_current_video_mode = -1;
  d3d_num_vram_pages     = 0;

  d3d9_pixel_size        = 0;
 	d3d9_current_logic_op  = DD_OP_SET;
  d3d9_current_color     = 0;

  // Set type of rendering device to use
  if (enable_hardware_acceleration)
    d3d_device_type = D3DDEVTYPE_HAL;
  else
    d3d_device_type = D3DDEVTYPE_REF;

/*___________________________________________________________________
|
| Create the D3D object
|___________________________________________________________________*/

  d3d9 = Direct3DCreate9 (D3D_SDK_VERSION);
  if (d3d9) {
    // Count the number of display adapters in the system
    num_adapters = d3d9->GetAdapterCount ();
    // Any adapters found?
    if (num_adapters) {

/*___________________________________________________________________
|
| Select adapter
|___________________________________________________________________*/

      // Set default adapter to use  
      d3d_adapter = D3DADAPTER_DEFAULT;
      
      // If more than 1, allow user to select an adapter
      if (num_adapters > 1) {
        // Allocate memory for array of adapter descriptions
        adapter_desc = (D3DADAPTER_IDENTIFIER9 *) calloc (num_adapters, sizeof(D3DADAPTER_IDENTIFIER9));
        // Allocate memory for array of adapter names    
        adapter_names = (char **) calloc (num_adapters, sizeof(char *));
        
        if (adapter_desc AND adapter_names) {
          // Build array of adapter descriptions
          for (i=0; i<num_adapters; i++)
            if (d3d9->GetAdapterIdentifier (i, 0, &adapter_desc[i]) != D3D_OK)
              strcpy (adapter_desc[i].Description, "--unknown adapter--");
          // Build array of adapter names
          for (i=0; i<num_adapters; i++)
            adapter_names[i] = adapter_desc[i].Description;
		      // Allow user to select an adapter from a list (-1 = none selected)
		      d3d_adapter = win_ListBox_Select ("Select an adapter", adapter_names, num_adapters);
        }

        // Free memory 
        if (adapter_desc)
          free (adapter_desc);
        if (adapter_names)
          free (adapter_names);
      }
  
/*___________________________________________________________________
|
| Build a list of supported modes for this adapter
|___________________________________________________________________*/
  
      // Count the number of modes available on the selected adapter
      num_modes = 0;
      if (d3d_adapter != -1) {
        // Count the number of 32-bit modes
        if (acceptable_bitdepths & BITDEPTH_32) {
          num_modes += d3d9->GetAdapterModeCount (d3d_adapter, D3DFMT_A8R8G8B8);
          num_modes += d3d9->GetAdapterModeCount (d3d_adapter, D3DFMT_X8R8G8B8);
        }
        // Count the number of 16-bit modes
        if (acceptable_bitdepths & BITDEPTH_16) {
          num_modes += d3d9->GetAdapterModeCount (d3d_adapter, D3DFMT_A1R5G5B5);
          num_modes += d3d9->GetAdapterModeCount (d3d_adapter, D3DFMT_X1R5G5B5);
          num_modes += d3d9->GetAdapterModeCount (d3d_adapter, D3DFMT_R5G6B5);
        }
      }

      // Any modes found?
      if (num_modes) {
        // Allocate memory for array of mode descriptions
        mode_desc = (D3DDISPLAYMODE *) calloc (num_modes, sizeof(D3DDISPLAYMODE));
        if (mode_desc) {
          // Set starting index into mode_desc array to write description into
          i = 0;
          // Add 32-bit mode descriptions
          if (acceptable_bitdepths & BITDEPTH_32) {
            n = d3d9->GetAdapterModeCount (d3d_adapter, D3DFMT_A8R8G8B8);
            for (j=0; j<n; j++, i++)
              d3d9->EnumAdapterModes (d3d_adapter, D3DFMT_A8R8G8B8, j, &mode_desc[i]);
            n = d3d9->GetAdapterModeCount (d3d_adapter, D3DFMT_X8R8G8B8);
            for (j=0; j<n; j++, i++)
              d3d9->EnumAdapterModes (d3d_adapter, D3DFMT_X8R8G8B8, j, &mode_desc[i]);
          }
          // Add 16-bit mode descriptions
          if (acceptable_bitdepths & BITDEPTH_16) {
            n = d3d9->GetAdapterModeCount (d3d_adapter, D3DFMT_A1R5G5B5);
            for (j=0; j<n; j++, i++)
              d3d9->EnumAdapterModes (d3d_adapter, D3DFMT_A1R5G5B5, j, &mode_desc[i]);
            n = d3d9->GetAdapterModeCount (d3d_adapter, D3DFMT_X1R5G5B5);
            for (j=0; j<n; j++, i++)
              d3d9->EnumAdapterModes (d3d_adapter, D3DFMT_X1R5G5B5, j, &mode_desc[i]);
            n = d3d9->GetAdapterModeCount (d3d_adapter, D3DFMT_R5G6B5);
            for (j=0; j<n; j++, i++)
              d3d9->EnumAdapterModes (d3d_adapter, D3DFMT_R5G6B5, j, &mode_desc[i]);
          }
          // Build global array of acceptable modes
          if (Build_Video_Modes_Array (mode_desc, num_modes, acceptable_resolutions, acceptable_bitdepths, d3d_adapter, d3d_device_type)) 
            query_ok = TRUE;
          // Free memory
          free (mode_desc);
        }
      }
    } // if (num_adapters)

/*___________________________________________________________________
|
| Cleanup
|___________________________________________________________________*/
  
    // Release the D3D object
    d3d9->Release ();
    d3d9 = NULL;
  } // if (d3d9)

  return (query_ok);
}

/*___________________________________________________________________
|
|	Function: Build_Video_Modes_Array
| 
|	Input: Called from d3d8_QueryHardware()
| Output: Builds global array of video modes.  Returns true on success,
|   else false on any error.
|___________________________________________________________________*/

static int Build_Video_Modes_Array (
  D3DDISPLAYMODE *mode_desc, 
  int             num_modes, 
  unsigned        acceptable_resolutions, 
  unsigned        acceptable_bitdepths,
  unsigned        adapter,
  D3DDEVTYPE      dev_type )
{
  int i, j, found, depth;
  unsigned resolution, bitdepth;
  D3DFORMAT buffer_format;
  char str[256], str2[80];

  // Table of info about video modes
  static struct {
    unsigned resolution, width, height;
  } Mode_info [] = {
    { RESOLUTION_640x480,    640,  480 },
    { RESOLUTION_800x600,    800,  600 },
    { RESOLUTION_1024x768,  1024,  768 },
    { RESOLUTION_1152x864,  1152,  864 },
    { RESOLUTION_1280x960,  1280,  960 },
    { RESOLUTION_1280x1024, 1280, 1024 },
    { RESOLUTION_1400x1050, 1400, 1050 },
    { RESOLUTION_1440x1080, 1440, 1080 },
    { RESOLUTION_1600x1200, 1600, 1200 },
    { RESOLUTION_1152x720,  1152,  720 },
    { RESOLUTION_1280x800,  1280,  800 },
    { RESOLUTION_1440x900,  1440,  900 },
    { RESOLUTION_1680x1050, 1680, 1050 },
    { RESOLUTION_1920x1200, 1920, 1200 }, 
    { RESOLUTION_2048x1280, 2048, 1280 },  
    { RESOLUTION_1280x720,  1280,  720 }, 
    { RESOLUTION_1600x900,  1600,  900 }, 
    { RESOLUTION_1920x1080, 1920, 1080 }, 
    { RESOLUTION_2048x1152, 2048, 1152 },
    { RESOLUTION_2560x1440, 2560, 1440 },
    { RESOLUTION_2560x1600, 2560, 1600 },
    { 0,                       0,    0 }
  };

/*___________________________________________________________________
|
| Allocate memory for modes array
|___________________________________________________________________*/

  d3d9_video_modes = 0;
  if (num_modes)
    d3d9_video_modes = (VideoModeInfo *) calloc (num_modes, sizeof(VideoModeInfo));
  if (d3d9_video_modes) {
    for (i=0; i<num_modes; i++) {

/*___________________________________________________________________
|
| Get resolution and bitdepth of this mode
|___________________________________________________________________*/

      resolution = 0; // Assume unknown resolution

      // Does it match a known resolution?
      for (j=0, found=FALSE; (NOT found) AND Mode_info[j].resolution; j++)  
        if ((mode_desc[i].Width  == (int)(Mode_info[j].width)) AND
            (mode_desc[i].Height == (int)(Mode_info[j].height))) {
          resolution = Mode_info[j].resolution;     
          found = TRUE;
        }
      
      // What is the bit depth of this mode? (only 6 formats are valid as a render target)
      switch (mode_desc[i].Format) {
        case D3DFMT_A8R8G8B8: depth = 32;
                              bitdepth = BITDEPTH_32;
                              buffer_format = D3DFMT_A8R8G8B8;
                              break;
        case D3DFMT_X8R8G8B8: depth = 32;
                              bitdepth = BITDEPTH_32;
                              buffer_format = D3DFMT_X8R8G8B8;
                              break;
        case D3DFMT_A1R5G5B5: depth = 16;
                              bitdepth = BITDEPTH_16;
                              buffer_format = D3DFMT_A1R5G5B5;
                              break;
        case D3DFMT_X1R5G5B5: depth = 16;
                              bitdepth = BITDEPTH_16;
                              buffer_format = D3DFMT_X1R5G5B5;
                              break;
        case D3DFMT_R5G6B5:   depth = 16;
                              bitdepth = BITDEPTH_16;
                              buffer_format = D3DFMT_R5G6B5;
                              break;
        default:              // Any other format not valid as a render target
                              bitdepth = 0;
                              break;
      }

/*___________________________________________________________________
|
| Make sure this mode can be used with the adapter and device type
|___________________________________________________________________*/

      if (resolution AND bitdepth) {
        if (d3d9->CheckDeviceType (adapter, dev_type, buffer_format, buffer_format, FALSE) == D3D_OK) {
    
/*___________________________________________________________________
|
| If mode is valid, add it to the list of available video modes
|___________________________________________________________________*/

          // Is this a valid mode?
          if ((resolution & acceptable_resolutions) AND (bitdepth & acceptable_bitdepths)) {
            // Save this mode in video modes array
            d3d9_video_modes[d3d_num_video_modes].width  = mode_desc[i].Width;
	  	      d3d9_video_modes[d3d_num_video_modes].height = mode_desc[i].Height;
		        d3d9_video_modes[d3d_num_video_modes].depth  = depth;
  		      d3d9_video_modes[d3d_num_video_modes].rate   = mode_desc[i].RefreshRate;
            d3d9_video_modes[d3d_num_video_modes].format = buffer_format;
            // Build a string id for this mode
            strcpy (str, "");
            strcat (str, itoa ((int)(mode_desc[i].Width), str2, 10));
            strcat (str, "x");
            strcat (str, itoa ((int)(mode_desc[i].Height), str2, 10));
            strcat (str, "x");
            strcat (str, itoa (depth, str2, 10));
            strcat (str, "bpp");
            // Show refresh rate in description if not 0
            if (mode_desc[i].RefreshRate) {
              strcat (str, "@");
              strcat (str, itoa ((int)(mode_desc[i].RefreshRate), str2, 10));
              strcat (str, "Hz");
            }
            // Show bit format info
            switch (mode_desc[i].Format) {
              case D3DFMT_A8R8G8B8: sprintf (str2, " (8/8/8/8)");
                                    break;
              case D3DFMT_X8R8G8B8: sprintf (str2, " (8/8/8)");
                                    break;
              case D3DFMT_A1R5G5B5: sprintf (str2, " (1/5/5/5)");
                                    break;
              case D3DFMT_X1R5G5B5: sprintf (str2, " (5/5/5)");
                                    break;
              case D3DFMT_R5G6B5:   sprintf (str2, " (5/6/5)");
                                    break;
            }
            strcat (str, str2);
            switch (resolution) {
              case RESOLUTION_1152x720:
              case RESOLUTION_1280x800:
              case RESOLUTION_1440x900:
              case RESOLUTION_1680x1050:
              case RESOLUTION_1920x1200:
              case RESOLUTION_2048x1280:  strcat (str, " wide 8:5");
                                          break;
              case RESOLUTION_1280x720:   
              case RESOLUTION_1600x900:  
              case RESOLUTION_1920x1080: 
              case RESOLUTION_2048x1152:
              case RESOLUTION_2560x1440:  strcat (str, " wide 16:9");
                                          break;
              case RESOLUTION_2560x1600:  strcat (str, " wide 16:10");
                                          break;
            }
//            d3d9_video_modes[d3d_num_video_modes].name = strdup (str);  // don't use strdup (which allocates memory) because Mem_Chek wouldn't see the allocation
            d3d9_video_modes[d3d_num_video_modes].name = (char *) malloc (strlen(str) + 1);
            if (d3d9_video_modes[d3d_num_video_modes].name == 0)
              DEBUG_ERROR ("Build_Video_Modes_Array(): can't allocate memory for video mode name")
            else {
              strcpy (d3d9_video_modes[d3d_num_video_modes].name, str);
              d3d_num_video_modes++;
            }
          }
        }
      }
    } // for

/*___________________________________________________________________
|
| Sort array of available video modes in order of increasing resolution/depth
|___________________________________________________________________*/

    if (d3d_num_video_modes)
      qsort (d3d9_video_modes, d3d_num_video_modes, sizeof(VideoModeInfo), Compare_Video_Modes);
  }

	return (d3d_num_video_modes != 0);
}

/*___________________________________________________________________
|
|	Function: Compare_Video_Modes
| 
|	Input: Called from Build_Video_Modes_Array(), via callback from qsort()
| Output: A comparison function for qsort() used to compare 2 video
|					modes.
|___________________________________________________________________*/

static int Compare_Video_Modes (const void *arg1, const void *arg2)
{
	VideoModeInfo *mode1 = (VideoModeInfo *)arg1;
	VideoModeInfo *mode2 = (VideoModeInfo *)arg2;

	int volume1 = mode1->width * mode1->height;
	int volume2 = mode2->width * mode2->height;

	if (volume1 < volume2)
		return -1;
	else if (volume1 > volume2)
		return 1;

	if (mode1->depth < mode2->depth)
		return -1;
	else if (mode1->depth > mode2->depth)
		return 1;
	
	return 0;
}

/*___________________________________________________________________
|
|	Function: d3d9_UserSelectMode
| 
|	Input: Called from dx9_GetUserFormat()
|
| Output: After d3d9_QueryHardware() has been called, this function 
|   allows user to select a video mode based on the list of available
|   video modes created in d3d9_QueryHardware().
|
|   Returns true on success and info about selected mode in callers
|   variables, else false if user cancels the dialog box without 
|   selecting anything or on any error.
|___________________________________________________________________*/

int	d3d9_UserSelectMode (int *width, int *height, int *depth)
{
  int i;
  char **mode_names;
  int selected_mode = -1;

  // Any modes available?
  if (d3d_num_video_modes) {
    // Allocate memory for array of mode names
    mode_names = (char **) calloc (d3d_num_video_modes, sizeof(char *));
    if (mode_names) {
      // Build array of mode names
      for (i=0; i<d3d_num_video_modes; i++) 
        mode_names[i] = d3d9_video_modes[i].name;
      // Allow user to select a mode from the list of available modes
      selected_mode = win_ListBox_Select ("Select a DX9 video mode", mode_names, d3d_num_video_modes);
      // Was a valid mode selected?
      if ((selected_mode >= 0) AND (selected_mode < d3d_num_video_modes)) {
        if (width)
          *width  = d3d9_video_modes[selected_mode].width;
        if (height)
          *height = d3d9_video_modes[selected_mode].height;
        if (depth)
          *depth  = d3d9_video_modes[selected_mode].depth;
      }
      else
        selected_mode = -1;
      
      // Free memory
      free (mode_names);
    }
  }

  return (selected_mode != -1);
}

/*___________________________________________________________________
|
|	Function: d3d9_SetMode
| 
|	Input: Called from dx9_Init()
|
| Output: Initializes video mode.  Returns # VRAM pages available or 0
|   on any error.  Caller should not request num_pages < 2 and this function
|   will not return a value of < 2 (must have at least 1 frontbuffer and
|   1 backbuffer).
|___________________________________________________________________*/

int	d3d9_SetMode (int width, int height, int depth, unsigned stencil_depth_requested, int num_pages_requested)
{
  int i, mode, initialized;
  DWORD flags;
  D3DPRESENT_PARAMETERS d3dpp;
  D3DFORMAT *depth_format;
  static D3DFORMAT non_stencil_depth_formats [] = {
    // non stencil formats
    D3DFMT_D32,
    D3DFMT_D24X8,
    D3DFMT_D24X4S4,
    D3DFMT_D24S8,
    D3DFMT_D16,
    D3DFMT_D16_LOCKABLE,
    D3DFMT_D15S1,
    (D3DFORMAT)0
  };
  static D3DFORMAT stencil_depth_formats [] = {
    // stencil formats
    D3DFMT_D24S8,
    D3DFMT_D24X4S4,
    D3DFMT_D15S1,
    // non stencil formats
    D3DFMT_D32,
    D3DFMT_D24X8,
    D3DFMT_D24X4S4,
    D3DFMT_D24S8,
    D3DFMT_D16,
    D3DFMT_D16_LOCKABLE,
    D3DFMT_D15S1,
    (D3DFORMAT)0
  };
#ifdef USE_MULTITHREADED_VERSION_OF_DIRECT3D
  DWORD flag_options [] = {
    D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
    D3DCREATE_MIXED_VERTEXPROCESSING    | D3DCREATE_MULTITHREADED,
    D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
    0
  };
#else
  DWORD flag_options [] = {
    D3DCREATE_HARDWARE_VERTEXPROCESSING,
    D3DCREATE_MIXED_VERTEXPROCESSING,
    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
    0
  };
#endif
  int num_pages_available = 0;

/*___________________________________________________________________
|
| Create the D3D object
|___________________________________________________________________*/

  d3d9 = Direct3DCreate9 (D3D_SDK_VERSION);
  if (d3d9) {
    // Get capabilities of the hardware exposed through the D3D object
    if (d3d9->GetDeviceCaps (d3d_adapter, d3d_device_type, &d3dcaps9) == D3D_OK) {

      /* CHECK HERE FOR ANY REQUIRED DEVICE CAPABILITIES */

/*___________________________________________________________________
|
| Find desired mode in mode list
|___________________________________________________________________*/

      mode = -1;
      for (i=0; i<d3d_num_video_modes; i++) {
        // Will this mode work?
        if ((d3d9_video_modes[i].width  == width)  AND
            (d3d9_video_modes[i].height == height) AND
            (d3d9_video_modes[i].depth  == depth)) {
          // Has a mode already been chosen?
          if (mode != -1) {
            // If newer mode is faster, choose it
            if (d3d9_video_modes[i].rate > d3d9_video_modes[mode].rate)
              mode = i;
          }
          else
            mode = i;
        }
      }
      // Was a matching mode found?
      if (mode != -1) {

/*___________________________________________________________________
|
| Create the D3D device (and set the video mode)
|___________________________________________________________________*/

/***** Probably need to modify this to auto created the depth/stencil buffer *****/
// d3dpp.EnableAutoDepthStencil = TRUE;
// d3dpp.AutoDepthStencilFormat = D3D_FMT_32; // or something similar

        // Describe the device needed
        ZeroMemory (&d3dpp, sizeof(d3dpp));
        d3dpp.BackBufferWidth        = d3d9_video_modes[mode].width;
        d3dpp.BackBufferHeight       = d3d9_video_modes[mode].height;
        d3dpp.BackBufferFormat       = d3d9_video_modes[mode].format;
        d3dpp.BackBufferCount        = num_pages_requested - 1;
        d3dpp.MultiSampleType        = D3DMULTISAMPLE_NONE;
        d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow          = NULL;
        d3dpp.Windowed               = FALSE;
        d3dpp.EnableAutoDepthStencil = FALSE;
        d3dpp.AutoDepthStencilFormat = (D3DFORMAT)0;
        d3dpp.Flags                  = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
        if (d3d9_video_modes[mode].rate)
          d3dpp.FullScreen_RefreshRateInHz = d3d9_video_modes[mode].rate;
        else
          d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;   // must be 0 for windowed mode
        d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // waits for next vsync (good!)

        // Create the device
        initialized = FALSE;
        for (i=0; flag_options[i] AND (NOT initialized); i++) {
          // Set creation behavior flags
          flags = flag_options[i];
          // Set number of desired back buffers
          d3dpp.BackBufferCount = num_pages_requested - 1;
          // Create the device with as many backbuffers as user needs or less (but not less than 1)
          for (; (d3dpp.BackBufferCount >= 1) AND (NOT initialized); d3dpp.BackBufferCount--) 
            if (d3d9->CreateDevice (d3d_adapter, d3d_device_type, win_Get_Window_Handle(), flags, &d3dpp, &d3ddevice9) == D3D_OK) {
              initialized = TRUE;
              break;
            }
        }
        
/*___________________________________________________________________
|
| Get a pointer to the backbuffer
|___________________________________________________________________*/

        if (initialized) {
          // Get interface to screen page (backbuffer)
          if (d3ddevice9->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &d3dscreen9) == D3D_OK) 
            // Save the current mode
            d3d9_current_video_mode = mode;
          else
            initialized = FALSE;
        }

/*___________________________________________________________________
|
| Create a depth buffer
|___________________________________________________________________*/

        // Initialize the best zbuffer/stencilbuffer available
        if (initialized) {
          // Try to init a z/stencil buffer or just a z buffer?
          if (stencil_depth_requested) 
            depth_format = stencil_depth_formats;
          else 
            depth_format = non_stencil_depth_formats;
          // Try all combinations starting from best to worst
          for (; *depth_format != 0; depth_format++) 
            if (Depth_Format_Available (*depth_format, d3d9_video_modes[mode].format, d3d9_video_modes[mode].format)) 
              // Create and attach a zbuffer
              if (Attach_Depth_Buffer (*depth_format))
                break;
          // Was the depth buffer created?
          if (NOT d3dzbuffer9) 
				    DEBUG_ERROR ("d3d9_SetMode(): ERROR ZBuffer not attached to backbuffer chain")
					else {
            // Save the format of the depth buffer
            d3d_zbuffer_format = *depth_format;
            // Save the current render surface pixel format
            Get_Pixel_Format_Data (d3d9_video_modes[d3d9_current_video_mode].format, 
                                   &d3d9_loREDbit,
                                   &d3d9_loGREENbit,
                                   &d3d9_loBLUEbit,
                                   &d3d9_loALPHAbit,
                                   &d3d9_numREDbits,
                                   &d3d9_numGREENbits,
                                   &d3d9_numBLUEbits,
                                   &d3d9_numALPHAbits,
                                   &d3d9_REDmask,
                                   &d3d9_GREENmask,
                                   &d3d9_BLUEmask,
                                   &d3d9_ALPHAmask,
                                   &d3d9_pixel_size);
            // Set # pages available in vram
            d3d_num_vram_pages  = 1 + d3dpp.BackBufferCount;
            num_pages_available = 1 + d3dpp.BackBufferCount;
          }
        }
      }
    }
  }

/*___________________________________________________________________
|
| Init some render attributes
|___________________________________________________________________*/

  if (num_pages_available) {
    // Enable z-buffering
    d3d9_EnableZBuffer (TRUE);
    // Set the viewport to entire screen
    d3d9_SetViewport (0, 0, SCREEN_DX-1, SCREEN_DY-1);

    // Get info about application window location
    int x1, y1, x2, y2;
    win_Get_Window_Rectangle (&x1, &y1, &x2, &y2);
    d3d_app_window_xleft = x1;
    d3d_app_window_ytop  = y1;
  }
  
/*___________________________________________________________________
|
| On any error, free resources
|___________________________________________________________________*/

  if (num_pages_available == 0) 
    d3d9_Free ();

  return (num_pages_available);
}

/*___________________________________________________________________
|
|	Function: Depth_Format_Available
| 
|	Input: Called from d3d9_SetMode()
| Output: Returns true if the depth format can be used with the adapter
|   and current backbuffer format.
|___________________________________________________________________*/

static int Depth_Format_Available (D3DFORMAT depth_format, D3DFORMAT adapter_format, D3DFORMAT backbuffer_format) 
{
  int available = FALSE;

  // Verify that the depth format exists
  if (d3d9->CheckDeviceFormat (d3d_adapter, 
                               d3d_device_type, 
                               adapter_format, 
                               D3DUSAGE_DEPTHSTENCIL, 
                               D3DRTYPE_SURFACE, 
                               depth_format) == D3D_OK)

    // Verify that the depth format is compatible
    if (d3d9->CheckDepthStencilMatch (d3d_adapter,
                                      d3d_device_type,
                                      adapter_format,
                                      backbuffer_format,
                                      depth_format) == D3D_OK)
      available = TRUE;

  return (available);
}

/*___________________________________________________________________
|
|	Function: Attach_Depth_Buffer
| 
|	Input: Called from d3d8_SetMode(), Reset_D3D_Device()
| Output: Creates a depth buffer and attaches it to the flipping chain.
|___________________________________________________________________*/

static int Attach_Depth_Buffer (D3DFORMAT depth_format)
{
  int attached = FALSE;

  // Create a z/stencil buffer
  if (d3ddevice9->CreateDepthStencilSurface (d3d9_video_modes[d3d9_current_video_mode].width,
                                             d3d9_video_modes[d3d9_current_video_mode].height,
                                             depth_format,
                                             D3DMULTISAMPLE_NONE,
                                             0,
                                             true,
                                             &d3dzbuffer9, 0) == D3D_OK)
    // Attach this depth buffer to the flipping chain
    if (d3ddevice9->SetDepthStencilSurface (d3dzbuffer9) == D3D_OK)
      attached = TRUE;

#ifdef _DEBUG
  if (NOT attached)
    DEBUG_ERROR ("Attach_Depth_Buffer(): ERROR depth buffer not attached")
#endif

  return (attached);
}

/*___________________________________________________________________
|
|	Function:	Get_Pixel_Format_Data
| 
|	Input: Called from d3d9_SetMode(), Copy_Pixels_To_Texture(),
|   d3d9_Get_Cursor_Pixel_Format()
| Output: Gets info about pixel format for and saves this info in class 
|   data variables.
|___________________________________________________________________*/

static void Get_Pixel_Format_Data (
  D3DFORMAT format,
  WORD     *low_red_bit,  // bits are numbered starting with 0 as low order bit
  WORD     *low_green_bit,
  WORD     *low_blue_bit,
  WORD     *low_alpha_bit,
  WORD     *num_red_bits,
  WORD     *num_green_bits,
  WORD     *num_blue_bits,
  WORD     *num_alpha_bits,
  DWORD    *red_mask,
  DWORD    *green_mask,
  DWORD    *blue_mask,
  DWORD    *alpha_mask, 
  int      *pixel_size )
{
  switch (format) {
    case D3DFMT_A8R8G8B8: *low_alpha_bit  = 24;
                          *low_red_bit    = 16;
                          *low_green_bit  = 8;
                          *low_blue_bit   = 0;

                          *num_alpha_bits = 8;
                          *num_green_bits = 8;
                          *num_blue_bits  = 8;
                          *num_red_bits   = 8;

                          *alpha_mask     = 0xFF000000;
                          *red_mask       = 0x00FF0000;
                          *green_mask     = 0x0000FF00;
                          *blue_mask      = 0x000000FF;

                          *pixel_size     = 4;
                          break;

    case D3DFMT_X8R8G8B8: *low_alpha_bit  = 0;  // none
                          *low_red_bit    = 16;
                          *low_green_bit  = 8;
                          *low_blue_bit   = 0;
                          
                          *num_alpha_bits = 0;
                          *num_red_bits   = 8;
                          *num_green_bits = 8;
                          *num_blue_bits  = 8;

                          *alpha_mask     = 0x00000000;
                          *red_mask       = 0x00FF0000;
                          *green_mask     = 0x0000FF00;
                          *blue_mask      = 0x000000FF;

                          *pixel_size     = 4;
                          break;

    case D3DFMT_X1R5G5B5: *low_alpha_bit  = 0;  // none
                          *low_red_bit    = 10;
                          *low_green_bit  = 5;
                          *low_blue_bit   = 0;

                          *num_alpha_bits = 0;
                          *num_red_bits   = 5;
                          *num_green_bits = 5;
                          *num_blue_bits  = 5;

                          *alpha_mask     = 0x00000000;
                          *red_mask       = 0x00007C00;
                          *green_mask     = 0x000003E0;
                          *blue_mask      = 0x0000001F;

                          *pixel_size     = 2;
                          break;

    case D3DFMT_A1R5G5B5: *low_alpha_bit  = 15;
                          *low_red_bit    = 10;
                          *low_green_bit  = 5;
                          *low_blue_bit   = 0;

                          *num_alpha_bits = 1;
                          *num_red_bits   = 5;
                          *num_green_bits = 5;
                          *num_blue_bits  = 5;

                          *alpha_mask     = 0x00008000;
                          *red_mask       = 0x00007C00;
                          *green_mask     = 0x000003E0;
                          *blue_mask      = 0x0000001F;

                          *pixel_size     = 2;
                          break;

    case D3DFMT_R5G6B5:   *low_alpha_bit  = 0;  // none
                          *low_red_bit    = 11;
                          *low_green_bit  = 5;
                          *low_blue_bit   = 0;

                          *num_alpha_bits = 0;
                          *num_red_bits   = 5;
                          *num_green_bits = 6;
                          *num_blue_bits  = 5;

                          *alpha_mask     = 0x00000000;
                          *red_mask       = 0x0000F800;
                          *green_mask     = 0x000007E0;
                          *blue_mask      = 0x0000001F;

                          *pixel_size     = 2;
                          break;
  }
}

/*___________________________________________________________________
|
|	Function: d3d9_Restore
| 
|	Input: Called from ____
| Output: Restores from a lost state.
|___________________________________________________________________*/

int d3d9_Restore ()
{
  RestoreParams p;
  static int restored, reset_in_progress;
  
  // Reset the device
  reset_in_progress = TRUE;
  p.zbuffer_format    = d3d_zbuffer_format;
  p.restored          = &restored;
  p.reset_in_progress = &reset_in_progress;
  win_CallbackQueue_Add (Reset_D3D_Device, (void *)&p, sizeof(RestoreParams));
  // Wait for the reset
  while (reset_in_progress);

#ifdef _DEBUG
  if (restored)
    DEBUG_WRITE ("d3d9_Restore(): device restored!")
  else
    DEBUG_ERROR ("d3d9_Restore(): ERROR device not restored")
#endif

  return (restored);
}

/*___________________________________________________________________
|
|	Function: Reset_D3D_Device
| 
|	Input: Called from d3d9_Restore()
| Output: Restores from a lost state.
|___________________________________________________________________*/

static void Reset_D3D_Device (void *params)
{
  D3DPRESENT_PARAMETERS d3dpp;
  RestoreParams *p = (RestoreParams *)params;

  *p->restored = TRUE;

  // Does D3D need restoring?
  if (d3ddevice9->TestCooperativeLevel () != D3D_OK) {
    // Busy wait until restore is allowed
    while (d3ddevice9->TestCooperativeLevel () == D3DERR_DEVICELOST);

    // Release interfaces
    if (d3dcursor9) {
      d3dcursor9->Release ();
      d3dcursor9 = 0;
    }
    if (d3dzbuffer9) {
      d3dzbuffer9->Release ();
      d3dzbuffer9 = 0;
    }
    if (d3dscreen9) {
      d3dscreen9->Release ();
      d3dscreen9 = 0;
    }
  //  if (d3ddevice8) {
  //    // Decrement reference count on last used vertex/index buffer, if any
  //    d3ddevice8->SetStreamSource (0, NULL, 0, 0);
  //    d3ddevice8->SetIndices (NULL);
  //  }

    // Reset the device
    ZeroMemory (&d3dpp, sizeof(d3dpp));
    d3dpp.BackBufferWidth        = d3d9_video_modes[d3d9_current_video_mode].width;
    d3dpp.BackBufferHeight       = d3d9_video_modes[d3d9_current_video_mode].height;
    d3dpp.BackBufferFormat       = d3d9_video_modes[d3d9_current_video_mode].format;
    d3dpp.BackBufferCount        = d3d_num_vram_pages - 1;
    d3dpp.MultiSampleType        = D3DMULTISAMPLE_NONE;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow          = NULL;
    d3dpp.Windowed               = FALSE;
    d3dpp.EnableAutoDepthStencil = FALSE;
    d3dpp.AutoDepthStencilFormat = (D3DFORMAT)0;
    d3dpp.Flags                  = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    if (d3d9_video_modes[d3d9_current_video_mode].rate)
      d3dpp.FullScreen_RefreshRateInHz = d3d9_video_modes[d3d9_current_video_mode].rate;
    else
      d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    
    if (d3ddevice9->Reset (&d3dpp) != D3D_OK) 
      DEBUG_ERROR ("Reset_D3D_Device(): Error calling d3ddevice9->Reset()")
    else {
      // Get interface to screen page (backbuffer)
      if (d3ddevice9->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &d3dscreen9) != D3D_OK)
        DEBUG_ERROR ("Reset_D3D_Device(): Error calling d3ddevice9->GetBackBuffer()")
      else
        // Create and attach a zbuffer
        Attach_Depth_Buffer (p->zbuffer_format);
    }

    // Was the device restored?
    if (d3ddevice9->TestCooperativeLevel () != D3D_OK) 
      *p->restored = FALSE;
  }

  *p->reset_in_progress = FALSE;
}

/*___________________________________________________________________
|
|	Function: d3d9_Free
| 
|	Input: Called from ____
| Output: Releases all global memory used by this file.
|___________________________________________________________________*/

void d3d9_Free ()
{
	int i;

  // Release cursor if not already done so via calldown from DirectInput
  if (d3dcursor9) {
    d3dcursor9->Release ();
    d3dcursor9 = 0;
  }
  // Release all D3D interface
  if (d3dzbuffer9) {
    d3dzbuffer9->Release ();
    d3dzbuffer9 = 0;
  }
  if (d3dscreen9) {
    d3dscreen9->Release ();
    d3dscreen9 = 0;
  }
  if (d3ddevice9) {
    // Decrement reference count on last used vertex/index buffer, if any
    d3ddevice9->SetStreamSource (0, NULL, 0, 0);
    d3ddevice9->SetIndices (NULL);
    // Release the D3D device
    d3ddevice9->Release ();
    d3ddevice9 = 0;
  }
	if (d3d9) {
		d3d9->Release ();
	  d3d9 = 0;
	}
										 
  // Free memory allocated for video mode array
  if (d3d9_video_modes) {
    for (i=0; i<d3d_num_video_modes; i++)
      if (d3d9_video_modes[i].name)
        free (d3d9_video_modes[i].name);
    free (d3d9_video_modes);
  }

	// Set global variables to uninitialized values
  d3d9_video_modes        = NULL;
  d3d_num_video_modes     = 0;
  d3d9_current_video_mode = -1;
}

/*___________________________________________________________________
|
|	Function: d3d9_GetScreenDimensions
| 
| Output: Returns width, height and depth of current video mode, or all
|   zeros if mode has not yet been set.
|___________________________________________________________________*/

void d3d9_GetScreenDimensions (int *width, int *height, int *depth)
{
	if (d3d9_current_video_mode != -1) {
    if (width)
  		*width  = SCREEN_DX;
    if (height)
  		*height = SCREEN_DY;
    if (depth)
      *depth  = SCREEN_DEPTH;
	}
	else {
    if (width)
      *width  = 0;
    if (height)
  	  *height = 0;
    if (depth)
      *depth  = 0;
	}
}

/*__________________________________________________________________
|
|	Function: d3d9_GetPixelSize
| 
|	Input: Called from ____
| Output:	Returns current pixel size.
|___________________________________________________________________*/

int d3d9_GetPixelSize ()
{
  return (d3d9_pixel_size);
}

/*__________________________________________________________________
|
|	Function: d3d9_BeginRender
| 
| Output: Begins a scene. Must be called before performing any rendering,
|   and d3d9_EndRender() must be called when rendering is complete.
|___________________________________________________________________*/

int d3d9_BeginRender ()
{  
  return (d3ddevice9->BeginScene () == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_EndRender
| 
| Output: Ends a scene that was begun by calling d3d9_BeginRender().
|___________________________________________________________________*/

int d3d9_EndRender ()
{
  return (d3ddevice9->EndScene () == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_SetFillMode
| 
| Output: Sets the render fill mode to one of:
|
|     FILL_MODE_POINT           
|     FILL_MODE_WIREFRAME       
|     FILL_MODE_SMOOTH_SHADED   
|     FILL_MODE_GOURAUD_SHADED    
|___________________________________________________________________*/

void d3d9_SetFillMode (int fill_mode)
{
  switch (fill_mode) {
    case FILL_MODE_POINT:
      d3ddevice9->SetRenderState (D3DRS_FILLMODE, D3DFILL_POINT);
      break;
    case FILL_MODE_WIREFRAME:
      d3ddevice9->SetRenderState (D3DRS_FILLMODE, D3DFILL_WIREFRAME);
      break;
    case FILL_MODE_SMOOTH_SHADED:
      d3ddevice9->SetRenderState (D3DRS_FILLMODE,  D3DFILL_SOLID);
      d3ddevice9->SetRenderState (D3DRS_SHADEMODE, D3DSHADE_FLAT);
      break;
    case FILL_MODE_GOURAUD_SHADED:
      d3ddevice9->SetRenderState (D3DRS_FILLMODE,  D3DFILL_SOLID);
      d3ddevice9->SetRenderState (D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
      break;
  }
}

/*___________________________________________________________________
|
|	Function: d3d9_GetDriverInfo
| 
| Output: Returns info about the 3D capabilities of the driver.
|___________________________________________________________________*/

void d3d9_GetDriverInfo (
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
  *max_texture_dx                  = (unsigned) d3dcaps9.MaxTextureWidth;
  *max_texture_dy                  = (unsigned) d3dcaps9.MaxTextureHeight;
  *max_active_lights               = (int)      d3dcaps9.MaxActiveLights;
  *max_user_clip_planes            = (int)      d3dcaps9.MaxUserClipPlanes;
  *max_simultaneous_texture_stages = (int)      d3dcaps9.MaxSimultaneousTextures;
  *max_texture_stages              = (int)      d3dcaps9.MaxTextureBlendStages;
  *max_texture_repeat              = (int)      d3dcaps9.MaxTextureRepeat;
  *num_stencil_bits                = d3d_num_stencil_bits;
  *stencil_ops                     = 0;
  if (d3d_num_stencil_bits) {
    if (d3dcaps9.StencilCaps & D3DSTENCILCAPS_DECR)
      *stencil_ops |= STENCILOP_DECR;
    if (d3dcaps9.StencilCaps & D3DSTENCILCAPS_DECRSAT)
      *stencil_ops |= STENCILOP_DECRSAT;
    if (d3dcaps9.StencilCaps & D3DSTENCILCAPS_INCR)
      *stencil_ops |= STENCILOP_INCR;
    if (d3dcaps9.StencilCaps & D3DSTENCILCAPS_INCRSAT)
      *stencil_ops |= STENCILOP_INCRSAT;
    if (d3dcaps9.StencilCaps & D3DSTENCILCAPS_INVERT)
      *stencil_ops |= STENCILOP_INVERT;
    if (d3dcaps9.StencilCaps & D3DSTENCILCAPS_KEEP)
      *stencil_ops |= STENCILOP_KEEP;
    if (d3dcaps9.StencilCaps & D3DSTENCILCAPS_REPLACE)
      *stencil_ops |= STENCILOP_REPLACE;
    if (d3dcaps9.StencilCaps & D3DSTENCILCAPS_ZERO)
      *stencil_ops |= STENCILOP_ZERO;
  }
  *max_vertex_blend_matrices       = (int)      d3dcaps9.MaxVertexBlendMatrices;
  *max_vertex_streams              = (int)      d3dcaps9.MaxStreams;
  *max_vertex_index                = (unsigned) d3dcaps9.MaxVertexIndex;
}

/*___________________________________________________________________
|
|	Function: d3d9_InitObject
| 
| Output: Sets up an object for drawing.
|___________________________________________________________________*/

void d3d9_InitObject (d3d9_Object *object)
{
  int i, j, index;
  unsigned size, host_stride;
  byte *buffer;
  LPDIRECT3DVERTEXBUFFER9 vb;
  LPDIRECT3DINDEXBUFFER9  ib;
  
/*___________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  object->offset_weight      = 0;
  object->offset_normal      = 0;
  object->offset_diffuse     = 0;
  object->offset_specular    = 0;
  for (i=0; i<NUM_TEXTURE_STAGES; i++) {
    object->offset_texcoord  [i] = 0;
    object->offset_texcoord_w[i] = 0;
  }

  object->fvf_code = D3DFVF_XYZ;
  size = 3 * sizeof(float);

//  if (object->weight) {
//    object->fvf_code |= (D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4);
//    object->offset_weight = size;
//    size += (3 * sizeof(float) + sizeof(unsigned));
//  }

  object->fvf_code |= D3DFVF_NORMAL;
  object->offset_normal = size;
  size += (3 * sizeof(float));

  if (object->vertex_color_diffuse) {
    object->fvf_code |= D3DFVF_DIFFUSE;
    object->offset_diffuse = size;
    size += sizeof(unsigned);
  }

  if (object->vertex_color_specular) {
    object->fvf_code |= D3DFVF_SPECULAR;
    object->offset_specular = size;
    size += sizeof(unsigned);
  }
  
  for (i=0; object->texture_coord[i] AND (i<NUM_TEXTURE_STAGES); i++) {
    object->offset_texcoord[i] = size;
    size += (2 * sizeof(float));
    if (object->texture_coord_w[i]) {
      object->offset_texcoord_w[i] = size;
      size += sizeof(float);
      object->fvf_code |= D3DFVF_TEXCOORDSIZE3(i);
    }
    else
      object->fvf_code |= D3DFVF_TEXCOORDSIZE2(i);
  }
  switch (i) {
    case 1: object->fvf_code |= D3DFVF_TEX1;
            break;
    case 2: object->fvf_code |= D3DFVF_TEX2;
            break;
    case 3: object->fvf_code |= D3DFVF_TEX3;
            break;
    case 4: object->fvf_code |= D3DFVF_TEX4;
            break;
    case 5: object->fvf_code |= D3DFVF_TEX5;
            break;
    case 6: object->fvf_code |= D3DFVF_TEX6;
            break;
    case 7: object->fvf_code |= D3DFVF_TEX7;
            break;
    case 8: object->fvf_code |= D3DFVF_TEX8;
            break;
  }

  object->vertex_size = size;

/*___________________________________________________________________
|
| Init vertex buffer
|___________________________________________________________________*/

  // Create the vertex buffer
  if (d3ddevice9->CreateVertexBuffer ((unsigned)(*object->num_vertices) * object->vertex_size, 
                                      D3DUSAGE_WRITEONLY,
                                      object->fvf_code,
                                      D3DPOOL_MANAGED,
                                      &vb, 0) != D3D_OK) 
    DEBUG_ERROR ("d3d9_InitObject(): Error calling d3ddevice9->CreateVertexBuffer()")
  else {
    // Lock the buffer
    if (vb->Lock (0, 0, (void **)&buffer, 0) != D3D_OK) {
      vb->Release ();
      vb = NULL;
      DEBUG_ERROR ("d3d9_InitObject(): Error calling vb->Lock()")
    }
    else {
      // Copy data into the buffer
      for (i=0; i<*object->num_vertices; i++) {
        index = i * object->vertex_size;
        // Copy vertex position
        memcpy (&buffer[index + 0], (void *)(&object->vertex[i * 3]), 3*sizeof(float));
        // Copy weight?
        if (object->offset_weight) {
          host_stride = 4 * sizeof(float) + 5 * sizeof(byte);
          size = 3 * sizeof(float);
          memcpy (&buffer[index + object->offset_weight + 0],                   (void *)(&object->weight[i * host_stride]), size);
          size = 4 * sizeof(byte);
          memcpy (&buffer[index + object->offset_weight + (3 * sizeof(float))], (void *)(&object->weight[i * host_stride + (4 * sizeof(float))]), size);
        }
        // Copy vertex normal                       
        memcpy (&buffer  [index + object->offset_normal],    (void *)(&object->vertex_normal[i * 3]),                           3*sizeof(float));
        // Copy diffuse color?
        if (object->offset_diffuse)
          memcpy (&buffer[index + object->offset_diffuse],   (void *)(&object->vertex_color_diffuse[i * sizeof(unsigned)]),     sizeof(unsigned));
        // Copy specular color?
        if (object->offset_specular)
          memcpy (&buffer[index + object->offset_specular],  (void *)(&object->vertex_color_specular[i * sizeof(unsigned)]),    sizeof(unsigned));
        // Copy tex coords?
        for (j=0; j<NUM_TEXTURE_STAGES; j++)
          if (object->offset_texcoord[j])
            memcpy (&buffer[index + object->offset_texcoord[j]], (void *)(&(object->texture_coord[j][i * 2])),                    2*sizeof(float));
        // Copy tex w coords?
        for (j=0; j<NUM_TEXTURE_STAGES; j++)
          if (object->offset_texcoord_w[j])
            memcpy (&buffer[index + object->offset_texcoord_w[j]], (void *)(&(object->texture_coord_w[j][i])),                sizeof(float));
      }
      // Unlock the buffer
      vb->Unlock ();
    }
  }

  object->vertex_buffer = vb;

/*___________________________________________________________________
|
| Init index buffer
|___________________________________________________________________*/

  // Create the index buffer
  if (d3ddevice9->CreateIndexBuffer (*object->num_surfaces * 3 * sizeof(word), 
                                     D3DUSAGE_WRITEONLY,
                                     D3DFMT_INDEX16,
                                     D3DPOOL_MANAGED,
                                     &ib, 0) != D3D_OK) 
    DEBUG_ERROR ("d3d9_InitObject(): Error calling d3ddevice9->CreateIndexBuffer()")
  else {
    // Lock the buffer
    if (ib->Lock (0, 0, (void **)&buffer, 0) != D3D_OK) {
      ib->Release ();
      ib = NULL;
      DEBUG_ERROR ("d3d9_InitObject(): Error calling ib->Lock()")
    }
    else {
      // Copy data into the buffer
      memcpy (buffer, (void *)(object->surface), *object->num_surfaces * 3 * sizeof(word));
      // Unlock the buffer
      ib->Unlock ();
    }
  }

  object->index_buffer = ib;
}

/*___________________________________________________________________
|
|	Function: d3d9_FreeObject
| 
| Output: Frees object resources created by routines in this file.
|___________________________________________________________________*/

void d3d9_FreeObject (d3d9_Object *object)
{
  // Free vertex buffer?
  if (object->vertex_buffer) {
    ((LPDIRECT3DVERTEXBUFFER9)(object->vertex_buffer))->Release ();
    object->vertex_buffer = 0;
  }
  // Free index buffer?
  if (object->index_buffer) {
    ((LPDIRECT3DINDEXBUFFER9)(object->index_buffer))->Release ();
    object->index_buffer = 0;
  }
}

extern bool  enable_pixel_shader1;
extern bool  enable_pixel_shader2;
extern bool  enable_pixel_shader_sun;
extern float param_clouddome_cover;
extern float param_clouddome_sunset;
extern float param_clouddome_density;
extern float param_clouddome_time;
extern int   param_elapsed_time;
extern float param_amount_sunlight;
extern bool  context_switch;

/*___________________________________________________________________
|
|	Function: Pixel_Shader_1_Enable
| 
| Output: Sets cloud dome pixel shader.
|___________________________________________________________________*/

static void Pixel_Shader_1_Enable ()
{
  ID3DXBuffer *shader = 0;
  static IDirect3DPixelShader9 *ps = 0;
  static IDirect3DVertexShader9 *vs = 0;
  static ID3DXConstantTable *ps_ct = 0;
  static D3DXHANDLE h0, h1, h2, h3, h4, h5, h6, h7;
  static D3DXCONSTANT_DESC d0, d1, d2, d3, d4, d5, d6, d7;
  static bool first_time = true;

  if (first_time OR context_switch) {
    if (context_switch) {
      if (ps_ct) {
        ps_ct->Release ();
        ps_ct = 0;
       }
      if (ps) {
        ps->Release ();
        ps = 0;
      }
    }
    // Load the shader from a file
    D3DXCompileShaderFromFile ("Pixel Shaders\\ps_clouddome.txt", 0, 0, "Main", "ps_2_0", 0, &shader, 0, &ps_ct);
    if (shader AND ps_ct) {
      DEBUG_WRITE ("")
      DEBUG_WRITE ("Creating pixel shader...")
      d3ddevice9->CreatePixelShader ((DWORD *)shader->GetBufferPointer(), &ps);
      shader->Release ();
      if (ps) {
        // Get handles
DEBUG_WRITE ("getting handles...")
        h0 = ps_ct->GetConstantByName (0, "InputImage0");
        h1 = ps_ct->GetConstantByName (0, "InputImage1");
        h2 = ps_ct->GetConstantByName (0, "InputImage2");
        h3 = ps_ct->GetConstantByName (0, "InputImage3");
        h4 = ps_ct->GetConstantByName (0, "Const_Float_0");
        h5 = ps_ct->GetConstantByName (0, "Const_Float_1");
        h6 = ps_ct->GetConstantByName (0, "Const_Float_2");
        h7 = ps_ct->GetConstantByName (0, "Const_Float_3");
        // Set constant descriptions
DEBUG_WRITE ("setting constant descriptions...")
        unsigned count;
        if (h0)
          ps_ct->GetConstantDesc (h0, &d0, &count);
        else
          DEBUG_WRITE ("no h0")
        if (h1)
          ps_ct->GetConstantDesc (h1, &d1, &count);
        else
          DEBUG_WRITE ("no h1")
        if (h2)
          ps_ct->GetConstantDesc (h2, &d2, &count);
        else
          DEBUG_WRITE ("no h2")
        if (h3)
          ps_ct->GetConstantDesc (h3, &d3, &count);
        else
          DEBUG_WRITE ("no h3")
        if (h4)
          ps_ct->GetConstantDesc (h4, &d4, &count);
        else
          DEBUG_WRITE ("no h4")
        if (h5)
          ps_ct->GetConstantDesc (h5, &d5, &count);
        else
          DEBUG_WRITE ("no h5")
        if (h6)
          ps_ct->GetConstantDesc (h6, &d6, &count);
        else
          DEBUG_WRITE ("no h6")
        if (h7)
          ps_ct->GetConstantDesc (h7, &d7, &count);
        else
          DEBUG_WRITE ("no h7")
//        ps_ct->SetDefaults (d3ddevice9);
      }
      else
        DEBUG_ERROR ("Pixel_Shader_1_Enable(): Error creating a pixel shader")
    }
    else
      DEBUG_ERROR ("Pixel_Shader_1_Enable(): Error compiling a pixel shader")
    first_time = false;
  }
  if (ps) {
    if (h4)
      ps_ct->SetFloat (d3ddevice9, h4, param_clouddome_cover);
    if (h5)
      ps_ct->SetFloat (d3ddevice9, h5, param_clouddome_sunset);
    if (h6)
      ps_ct->SetFloat (d3ddevice9, h6, param_clouddome_density);
    if (h7)
      ps_ct->SetFloat (d3ddevice9, h7, param_amount_sunlight);
    d3ddevice9->SetPixelShader (ps);
  }
  else
    d3ddevice9->SetPixelShader (0);
}

/*___________________________________________________________________
|
|	Function: Pixel_Shader_2_Enable
| 
| Output: Sets cloud dome pixel shader.
|___________________________________________________________________*/

static void Pixel_Shader_2_Enable ()
{
  ID3DXBuffer *shader = 0;
  static IDirect3DPixelShader9 *ps = 0;
  static IDirect3DVertexShader9 *vs = 0;
  static ID3DXConstantTable *ps_ct = 0;
  static D3DXHANDLE h0, h1, h2, h3, h4;
  static D3DXCONSTANT_DESC d0, d1, d2, d3, d4;
  static bool first_time = true;

  if (first_time OR context_switch) {
    if (context_switch) {
      if (ps_ct) {
        ps_ct->Release ();
        ps_ct = 0;
       }
      if (ps) {
        ps->Release ();
        ps = 0;
      }
    }
    // Load the shader from a file
    D3DXCompileShaderFromFile ("Pixel Shaders\\ps_skydome.txt", 0, 0, "Main", "ps_2_0", 0, &shader, 0, &ps_ct);
    if (shader AND ps_ct) {
      DEBUG_WRITE ("")
      DEBUG_WRITE ("Creating pixel shader...")
      d3ddevice9->CreatePixelShader ((DWORD *)shader->GetBufferPointer(), &ps);
      shader->Release ();
      if (ps) {
        // Get handles
DEBUG_WRITE ("getting handles...")
        h0 = ps_ct->GetConstantByName (0, "InputImage0");
        h1 = ps_ct->GetConstantByName (0, "InputImage1");
        h2 = ps_ct->GetConstantByName (0, "InputImage2");
        h3 = ps_ct->GetConstantByName (0, "Const_Float_0");
        h4 = ps_ct->GetConstantByName (0, "Const_Int_0");
        // Set constant descriptions
DEBUG_WRITE ("setting constant descriptions...")
        unsigned count;
        if (h0)
          ps_ct->GetConstantDesc (h0, &d0, &count);
        else
          DEBUG_WRITE ("no h0")
        if (h1)
          ps_ct->GetConstantDesc (h1, &d1, &count);
        else
          DEBUG_WRITE ("no h1")
        if (h2)
          ps_ct->GetConstantDesc (h2, &d2, &count);
        else
          DEBUG_WRITE ("no h2")
        if (h3)
          ps_ct->GetConstantDesc (h3, &d3, &count);
        else
          DEBUG_WRITE ("no h3")
        if (h4)
          ps_ct->GetConstantDesc (h4, &d4, &count);
        else
          DEBUG_WRITE ("no h4")
//        ps_ct->SetDefaults (d3ddevice9);
      }
      else
        DEBUG_ERROR ("Pixel_Shader_2_Enable(): Error creating a pixel shader")
    }
    else
      DEBUG_ERROR ("Pixel_Shader_2_Enable(): Error compiling a pixel shader")
    first_time = false;
  }
  if (ps) {
    if (h3)
      ps_ct->SetFloat (d3ddevice9, h3, param_amount_sunlight);
    if (h4)
      ps_ct->SetInt (d3ddevice9, h4, param_elapsed_time);
    d3ddevice9->SetPixelShader (ps);
  }
  else
    d3ddevice9->SetPixelShader (0);
}

/*___________________________________________________________________
|
|	Function: Pixel_Shader_Sun_Enable
| 
| Output: Sets cloud dome pixel shader.
|___________________________________________________________________*/

static void Pixel_Shader_Sun_Enable ()
{
  ID3DXBuffer *shader = 0;
  static IDirect3DPixelShader9 *ps = 0;
  static IDirect3DVertexShader9 *vs = 0;
  static ID3DXConstantTable *ps_ct = 0;
  static D3DXHANDLE h0, h1;
  static D3DXCONSTANT_DESC d0, d1;
  static bool first_time = true;

  if (first_time OR context_switch) {
    if (context_switch) {
      if (ps_ct) {
        ps_ct->Release ();
        ps_ct = 0;
       }
      if (ps) {
        ps->Release ();
        ps = 0;
      }
    }
    // Load the shader from a file
    D3DXCompileShaderFromFile ("Pixel Shaders\\ps_sun.txt", 0, 0, "Main", "ps_2_0", 0, &shader, 0, &ps_ct);
    if (shader AND ps_ct) {
      DEBUG_WRITE ("")
      DEBUG_WRITE ("Creating pixel shader...")
      d3ddevice9->CreatePixelShader ((DWORD *)shader->GetBufferPointer(), &ps);
      shader->Release ();
      if (ps) {
        // Get handles
DEBUG_WRITE ("getting handles...")
        h0 = ps_ct->GetConstantByName (0, "InputImage0");
        h1 = ps_ct->GetConstantByName (0, "Const_Float_0");
        // Set constant descriptions
DEBUG_WRITE ("setting constant descriptions...")
        unsigned count;
        if (h0)
          ps_ct->GetConstantDesc (h0, &d0, &count);
        else
          DEBUG_WRITE ("no h0")
        if (h1)
          ps_ct->GetConstantDesc (h1, &d1, &count);
        else
          DEBUG_WRITE ("no h1")
//        ps_ct->SetDefaults (d3ddevice9);
      }
      else
        DEBUG_ERROR ("Pixel_Shader_Sun_Enable(): Error creating a pixel shader")
    }
    else
      DEBUG_ERROR ("Pixel_Shader_Sun_Enable(): Error compiling a pixel shader")
    first_time = false;
  }
  if (ps) {
    if (h1)
      ps_ct->SetFloat (d3ddevice9, h1, param_clouddome_time);
    d3ddevice9->SetPixelShader (ps);
  }
  else
    d3ddevice9->SetPixelShader (0);
}

/*___________________________________________________________________
|
|	Function: d3d9_DrawObject
| 
| Output: Draws an object.
|___________________________________________________________________*/

void d3d9_DrawObject (d3d9_Object *object)
{
  int i, j, index;
  byte *buffer;
  bool found;

#ifdef ENABLE_CLOUD_SHADERS  
	// Set pixel shader
  if (enable_pixel_shader1)
    Pixel_Shader_1_Enable ();
  else if (enable_pixel_shader2)
    Pixel_Shader_2_Enable ();
  else if (enable_pixel_shader_sun) 
    Pixel_Shader_Sun_Enable ();
  else
#endif
    d3ddevice9->SetPixelShader (0);
  
  if (object) {
    // Is any dynamic data?
    found = false;
    if (*object->X_vertex OR *object->X_vertex_normal OR *object->X_weight)
      found = true;
    for (i=0; (i<NUM_TEXTURE_STAGES) AND (NOT found); i++)
      if (object->X_texture_coord[i] OR object->X_texture_coord_w[i]) 
        found = true;
    // If any dynamic data, write it into vertex buffer
    if (found) {
      // Lock the buffer
      if (((LPDIRECT3DVERTEXBUFFER9)object->vertex_buffer)->Lock (0, 0, (void **)&buffer, 0) == D3D_OK) {
        // Copy data into the buffer
        for (i=0; i<*object->num_vertices; i++) {
          index = i * object->vertex_size;
          // Copy vertex position?
          if (*object->X_vertex)
            memcpy (&buffer[index + 0], (void *)(&(*object->X_vertex)[i * 3]), 3*sizeof(float));
          // Copy weight?
          if (*object->X_weight)             
            memcpy (&buffer[index + object->offset_weight], (void *)(&(*object->X_weight)[i * (3*sizeof(float)+sizeof(unsigned))]), 3*sizeof(float)+sizeof(unsigned));
          // Copy vertex normal?
          if (*object->X_vertex_normal)
            memcpy (&buffer[index + object->offset_normal], (void *)(&(*object->X_vertex_normal)[i * 3]), 3*sizeof(float));
          // Copy tex coords?
          for (j=0; j<NUM_TEXTURE_STAGES; j++)
            if (object->X_texture_coord[j])
              memcpy (&buffer[index + object->offset_texcoord[j]], (void *)(&(object->X_texture_coord[j][i * 2])),                    2*sizeof(float));
          // Copy tex w coords?
          for (j=0; j<NUM_TEXTURE_STAGES; j++)
            if (object->X_texture_coord_w[j])
              memcpy (&buffer[index + object->offset_texcoord_w[j]], (void *)(&(object->X_texture_coord_w[j][i])),                sizeof(float));
        }
        // Unlock the buffer
        ((LPDIRECT3DVERTEXBUFFER9)object->vertex_buffer)->Unlock ();
      }
    }
    // Draw the object
    Draw_Object (object->vertex_buffer, 
                 object->index_buffer,
                *object->num_vertices,
                 object->vertex_size,
                *object->num_surfaces,
                 object->fvf_code);
  }
}

/*___________________________________________________________________
|
|	Function: Draw_Object
| 
| Output: Draws an object using previously created vertex/index buffers.
|___________________________________________________________________*/

static void Draw_Object (
  void     *vertex_buffer,
  void     *index_buffer, 
  int       num_vertices,
  unsigned  vertex_size,
  int       num_surfaces,
  DWORD     fvfcode )
{
  HRESULT hres1, hres2;

  // Set rendering stream 0 to this vertex buffer
  if (d3ddevice9->SetStreamSource (0, (LPDIRECT3DVERTEXBUFFER9)vertex_buffer, 0, vertex_size) != D3D_OK) 
    DEBUG_ERROR ("(d3d9.cpp) Draw_Object(): Error calling d3ddevice9->SetStreamSource()")
  else {
    // Set the FVF codes in the vertex shader
    hres1 = d3ddevice9->SetVertexShader (0);
    hres2 = d3ddevice9->SetFVF (fvfcode);
    if ((hres1 != D3D_OK) OR (hres2 != D3D_OK))
      DEBUG_ERROR ("(d3d9.cpp) Draw_Object(): Error setting FVF code")
    else {
      // Set the index buffer to use
      if (d3ddevice9->SetIndices ((LPDIRECT3DINDEXBUFFER9)index_buffer) != D3D_OK)
        DEBUG_ERROR ("(d3d9.cpp) Draw_Object(): Error calling d3ddevice9->SetIndices()")
      else {
        // Draw the object
        if (d3ddevice9->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, 0, 0, num_vertices, 0, num_surfaces) != D3D_OK)
          DEBUG_ERROR ("(d3d9.cpp) Draw_Object(): Error calling d3ddevice9->DrawIndexedPrimitive()")
      }
    }
  }
}

/*___________________________________________________________________
|
|	Function: d3d9_SetViewport
| 
| Output: Sets the viewport parameters for the device.
|___________________________________________________________________*/

int d3d9_SetViewport (int left, int top, int right, int bottom)
{
  D3DVIEWPORT9 viewport;

  viewport.X      = left;
  viewport.Y      = top;
  viewport.Width  = right - left + 1;
  viewport.Height = bottom - top + 1;
  viewport.MinZ   = 0;
  viewport.MaxZ   = 1;

  return (d3ddevice9->SetViewport (&viewport) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_ClearViewportRectangle
| 
| Output: Clears a rectangle in the viewport to a color, and optionally
|   clears the zbuffer and/or stencil buffer.
|___________________________________________________________________*/

void d3d9_ClearViewportRectangle (
  int      *rect, 
  unsigned  flags,  // 0x1=surface, 0x2=zbuffer, 0x4=stencil
  byte      r,
  byte      g,
  byte      b,
  byte      a,
  float     zval,
  unsigned  stencilval )
{
  int num_rt;
  D3DRECT rt, *rtp;
  DWORD   dwFlags;
  D3DCOLOR color;
  HRESULT hres;

  if (rect) {
    rt.x1 = rect[0];
    rt.y1 = rect[1];
    rt.x2 = rect[2] + 1;
    rt.y2 = rect[3] + 1;
    rtp = &rt;
    num_rt = 1;
  }
  else {
    rtp = NULL;
    num_rt = 0;
  }

  dwFlags = 0;
  // clear surface?
  if (flags & 0x1) 
    dwFlags |= D3DCLEAR_TARGET;
  // clear zbuffer?
  if (flags & 0x2)
    dwFlags |= D3DCLEAR_ZBUFFER;
  // clear stencilbuffer?
  if (flags & 0x4)
    dwFlags |= D3DCLEAR_STENCIL;

  color = (unsigned)a << 24 | (unsigned)r << 16 | (unsigned)g << 8 | (unsigned)b;

  hres = d3ddevice9->Clear (num_rt, rtp, dwFlags, color, zval, (DWORD)stencilval);
  
#ifdef _DEBUG
  if (hres != D3D_OK) {
    char str[80];
    DEBUG_ERROR ("d3d9_ClearViewportRectangle(): ERROR")
    sprintf (str, "error code %d", (int)hres);
    DEBUG_WRITE (str)
  }
#endif
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableClipping
| 
|	Input:
| Output: Enables/disables clipping to view frustrum.  Normally this
|   should be on and should only be turned off when drawing objects
|   known to be completely within the view frustrum.  
|
|   The default is clipping is enabled.
|___________________________________________________________________*/

void d3d9_EnableClipping (int flag)
{
  if (flag) 
    d3ddevice9->SetRenderState (D3DRS_CLIPPING, TRUE);
  else
    d3ddevice9->SetRenderState (D3DRS_CLIPPING, FALSE);
}

/*___________________________________________________________________
|
|	Function: d3d9_InitClipPlane
| 
|	Input: index = 0-31
| Output: Returns true if successful, else false on any error.
|___________________________________________________________________*/

int d3d9_InitClipPlane (unsigned index, float a, float b, float c, float d)
{
  float plane[4];

  plane[0] = a;
  plane[1] = b;
  plane[2] = c;
  plane[3] = d;

  return (d3ddevice9->SetClipPlane (index, plane) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableClipPlane
| 
|	Input: 
| Output: 
|___________________________________________________________________*/
 
void d3d9_EnableClipPlane (
  unsigned plane, // 0-31
  int      flag ) // boolean
{
  DWORD state, mask;

  // Get the current clip state
  d3ddevice9->GetRenderState (D3DRS_CLIPPLANEENABLE, &state);

  // Get bitmask for the plane
  for (mask=0x1; plane; mask<<=1, plane--);

  // Enable the plane
  if (flag)
    state |= mask;
  // Disable the plane?
  else
    state |= (~mask);

  d3ddevice9->SetRenderState (D3DRS_CLIPPLANEENABLE, state);
}

/*___________________________________________________________________
|
|	Function: d3d9_SetWorldMatrix
| 
| Output: Sets one of the 256 world matrices, identified by the index.
|___________________________________________________________________*/

int d3d9_SetWorldMatrix (int index, void *m)
{
  return (d3ddevice9->SetTransform (D3DTS_WORLDMATRIX(index), (D3DMATRIX *)m) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_GetWorldMatrix
| 
| Output: Returns contents of one of the 256 world matrices, identified
|   by the index
|___________________________________________________________________*/

int d3d9_GetWorldMatrix (int index, void *m)
{
  return (d3ddevice9->GetTransform (D3DTS_WORLDMATRIX(index), (D3DMATRIX *)m) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_SetViewMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int d3d9_SetViewMatrix (void *m)
{
  return (d3ddevice9->SetTransform (D3DTS_VIEW, (D3DMATRIX *)m) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_GetViewMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int d3d9_GetViewMatrix (void *m)
{
  return (d3ddevice9->GetTransform (D3DTS_VIEW, (D3DMATRIX *)m) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_SetProjectionMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int d3d9_SetProjectionMatrix (void *m)
{
  return (d3ddevice9->SetTransform (D3DTS_PROJECTION, (D3DMATRIX *)m) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_GetProjectionMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int d3d9_GetProjectionMatrix (void *m)
{
  return (d3ddevice9->GetTransform (D3DTS_PROJECTION, (D3DMATRIX *)m) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableTextureMatrix
| 
| Output: Enables/disables use of the texture matrix to transform
|     texture coordinates.
|___________________________________________________________________*/

int d3d9_EnableTextureMatrix (int stage, int dimension, int flag)
{
  HRESULT hres;
  DWORD value;

  if (flag AND (dimension >= 2) AND (dimension <= 4)) {
    switch (dimension) {
      case 2: value = D3DTTFF_COUNT2; break;
      case 3: value = D3DTTFF_COUNT3; break;
      case 4: value = D3DTTFF_COUNT4; break;
    }
    hres = d3ddevice9->SetTextureStageState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, value);
  }
  else
    hres = d3ddevice9->SetTextureStageState (stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

  return (hres == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_SetTextureMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int d3d9_SetTextureMatrix (int stage, void *m)
{
  D3DTRANSFORMSTATETYPE type[8] = {
    D3DTS_TEXTURE0,
    D3DTS_TEXTURE1,
    D3DTS_TEXTURE2,
    D3DTS_TEXTURE3,
    D3DTS_TEXTURE4,
    D3DTS_TEXTURE5,
    D3DTS_TEXTURE6,
    D3DTS_TEXTURE7
  };

  return (d3ddevice9->SetTransform (type[stage], (D3DMATRIX *)m) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_GetTextureMatrix
| 
|	Input:
| Output:
|___________________________________________________________________*/

int d3d9_GetTextureMatrix (int stage, void *m)
{
  D3DTRANSFORMSTATETYPE type[8] = {
    D3DTS_TEXTURE0,
    D3DTS_TEXTURE1,
    D3DTS_TEXTURE2,
    D3DTS_TEXTURE3,
    D3DTS_TEXTURE4,
    D3DTS_TEXTURE5,
    D3DTS_TEXTURE6,
    D3DTS_TEXTURE7
  };

  return (d3ddevice9->GetTransform (type[stage], (D3DMATRIX *)m) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableZBuffer
| 
|	Input:
| Output:
|___________________________________________________________________*/

void d3d9_EnableZBuffer (int flag)
{
  if (flag) 
    d3ddevice9->SetRenderState (D3DRS_ZENABLE, D3DZB_TRUE);
  else
    d3ddevice9->SetRenderState (D3DRS_ZENABLE, D3DZB_FALSE);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableBackfaceRemoval
| 
| Output: Enables/disables backface removal.  Default is enabled.
|___________________________________________________________________*/

void d3d9_EnableBackfaceRemoval (int flag)
{
  if (flag)
    d3ddevice9->SetRenderState (D3DRS_CULLMODE, D3DCULL_CCW);
  else
    d3ddevice9->SetRenderState (D3DRS_CULLMODE, D3DCULL_NONE);
}

/*____________________________________________________________________
|
| Function: d3d9_EnableStencilBuffer
|
| Output: Enables stencil buffer processing.
|___________________________________________________________________*/

void d3d9_EnableStencilBuffer (int flag)
{
  if (flag) 
    d3ddevice9->SetRenderState (D3DRS_STENCILENABLE, TRUE);
  else
    d3ddevice9->SetRenderState (D3DRS_STENCILENABLE, FALSE);
}

/*____________________________________________________________________
|
| Function: d3d9_SetStencilFailOp
|
| Output: Sets the stencil operation to perform if the stencil test
|     fails.  Operation can be one of the following:
|
|       STENCILOP_DECR     
|       STENCILOP_DECRSAT  
|       STENCILOP_INCR     
|       STENCILOP_INCRSAT  
|       STENCILOP_INVERT   
|       STENCILOP_KEEP   (default)
|       STENCILOP_REPLACE  
|       STENCILOP_ZERO     
|___________________________________________________________________*/

void d3d9_SetStencilFailOp (int stencil_op)
{
  DWORD op;
  
  switch (stencil_op) {
    case STENCILOP_DECR:    op = D3DSTENCILOP_DECR;
                            break;
    case STENCILOP_DECRSAT: op = D3DSTENCILOP_DECRSAT;
                            break;
    case STENCILOP_INCR:    op = D3DSTENCILOP_INCR;
                            break;
    case STENCILOP_INCRSAT: op = D3DSTENCILOP_INCRSAT;
                            break;
    case STENCILOP_INVERT:  op = D3DSTENCILOP_INVERT;
                            break;
    case STENCILOP_KEEP:    op = D3DSTENCILOP_KEEP;
                            break;
    case STENCILOP_REPLACE: op = D3DSTENCILOP_REPLACE;
                            break;
    case STENCILOP_ZERO:    op = D3DSTENCILOP_ZERO;
                            break;
  }
  
  d3ddevice9->SetRenderState (D3DRS_STENCILFAIL, op);
}

/*____________________________________________________________________
|
| Function: d3d9_SetStencilZFailOp
|
| Output: Sets the stencil operation to perform if the stencil test
|     passes and the depth test fails. Operation can be one of the 
|     following:
|
|       STENCILOP_DECR     
|       STENCILOP_DECRSAT  
|       STENCILOP_INCR     
|       STENCILOP_INCRSAT  
|       STENCILOP_INVERT   
|       STENCILOP_KEEP   (default)
|       STENCILOP_REPLACE  
|       STENCILOP_ZERO     
|___________________________________________________________________*/

void d3d9_SetStencilZFailOp (int stencil_op) 
{
  DWORD op;
  
  switch (stencil_op) {
    case STENCILOP_DECR:    op = D3DSTENCILOP_DECR;
                            break;
    case STENCILOP_DECRSAT: op = D3DSTENCILOP_DECRSAT;
                            break;
    case STENCILOP_INCR:    op = D3DSTENCILOP_INCR;
                            break;
    case STENCILOP_INCRSAT: op = D3DSTENCILOP_INCRSAT;
                            break;
    case STENCILOP_INVERT:  op = D3DSTENCILOP_INVERT;
                            break;
    case STENCILOP_KEEP:    op = D3DSTENCILOP_KEEP;
                            break;
    case STENCILOP_REPLACE: op = D3DSTENCILOP_REPLACE;
                            break;
    case STENCILOP_ZERO:    op = D3DSTENCILOP_ZERO;
                            break;
  }
  
  d3ddevice9->SetRenderState (D3DRS_STENCILZFAIL, op);
}

/*____________________________________________________________________
|
| Function: d3d9_SetStencilPassOp
|
| Output: Sets the stencil operation to perform if both the stencil test
|     passes and the depth test passes. Operation can be one of the 
|     following:
|
|       STENCILOP_DECR     
|       STENCILOP_DECRSAT  
|       STENCILOP_INCR     
|       STENCILOP_INCRSAT  
|       STENCILOP_INVERT   
|       STENCILOP_KEEP   (default)
|       STENCILOP_REPLACE  
|       STENCILOP_ZERO     
|___________________________________________________________________*/

void d3d9_SetStencilPassOp (int stencil_op)
{
  DWORD op;
  
  switch (stencil_op) {
    case STENCILOP_DECR:    op = D3DSTENCILOP_DECR;
                            break;
    case STENCILOP_DECRSAT: op = D3DSTENCILOP_DECRSAT;
                            break;
    case STENCILOP_INCR:    op = D3DSTENCILOP_INCR;
                            break;
    case STENCILOP_INCRSAT: op = D3DSTENCILOP_INCRSAT;
                            break;
    case STENCILOP_INVERT:  op = D3DSTENCILOP_INVERT;
                            break;
    case STENCILOP_KEEP:    op = D3DSTENCILOP_KEEP;
                            break;
    case STENCILOP_REPLACE: op = D3DSTENCILOP_REPLACE;
                            break;
    case STENCILOP_ZERO:    op = D3DSTENCILOP_ZERO;
                            break;
  }
  
  d3ddevice9->SetRenderState (D3DRS_STENCILPASS, op);
}

/*____________________________________________________________________
|
| Function: d3d9_SetStencilComparison
|
| Output: Sets the stencil comparison function. The comparison function
|     compares the reference value to a stencil buffer entry and applies
|     only to the bits in the reference value and stencil buffer entry
|     that are set in the stencil mask.  If the comparison is true, the
|     stencil test passes.
|
|     Can be one of the following:
|
|       STENCILFUNC_NEVER        
|       STENCILFUNC_LESS         
|       STENCILFUNC_EQUAL        
|       STENCILFUNC_LESSEQUAL    
|       STENCILFUNC_GREATER      
|       STENCILFUNC_NOTEQUAL     
|       STENCILFUNC_GREATEREQUAL 
|       STENCILFUNC_ALWAYS   (default)       
|___________________________________________________________________*/

void d3d9_SetStencilComparison (int stencil_function)
{
  DWORD compare;

  switch (stencil_function) {
    case STENCILFUNC_NEVER:         compare = D3DCMP_NEVER;
                                    break;
    case STENCILFUNC_LESS:          compare = D3DCMP_LESS;
                                    break;
    case STENCILFUNC_EQUAL:         compare = D3DCMP_EQUAL;
                                    break;
    case STENCILFUNC_LESSEQUAL:     compare = D3DCMP_LESSEQUAL;
                                    break;
    case STENCILFUNC_GREATER:       compare = D3DCMP_GREATER;
                                    break;
    case STENCILFUNC_NOTEQUAL:      compare = D3DCMP_NOTEQUAL;
                                    break;
    case STENCILFUNC_GREATEREQUAL:  compare = D3DCMP_GREATEREQUAL;
                                    break;
    case STENCILFUNC_ALWAYS:        compare = D3DCMP_ALWAYS;
                                    break;
  }
  
  d3ddevice9->SetRenderState (D3DRS_STENCILFUNC, compare);
}

/*____________________________________________________________________
|
| Function: d3d9_SetStencilReferenceValue
|
| Output: Sets the integer reference value for the stencil test.  The
|     default value is 0.
|___________________________________________________________________*/

void d3d9_SetStencilReferenceValue (unsigned reference_value)
{
  d3ddevice9->SetRenderState (D3DRS_STENCILREF, (DWORD)reference_value);
}

/*____________________________________________________________________
|
| Function: d3d9_SetStencilMask
|
| Output: Sets the mask to apply to the reference value and each stencil
|     buffer entry to determine the significant bits for the stencil test.  
|     The default mask is 0xFFFFFFFF.
|___________________________________________________________________*/

void d3d9_SetStencilMask (unsigned mask)
{
  d3ddevice9->SetRenderState (D3DRS_STENCILMASK, (DWORD)mask);
}

/*____________________________________________________________________
|
| Function: d3d9_SetStencilWriteMask
|
| Output: Sets the mask to apply to values written into the stencil 
|     buffer.  The default is 0xFFFFFFFF. 
|___________________________________________________________________*/

void d3d9_SetStencilWriteMask (unsigned mask)
{
  d3ddevice9->SetRenderState (D3DRS_STENCILWRITEMASK, (DWORD)mask);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableLighting
| 
|	Input:
| Output:
|___________________________________________________________________*/

void d3d9_EnableLighting (int flag)
{
  if (flag) 
    d3ddevice9->SetRenderState (D3DRS_LIGHTING, TRUE);
  else
    d3ddevice9->SetRenderState (D3DRS_LIGHTING, FALSE);
}

/*___________________________________________________________________
|
|	Function: d3d9_InitPointLight
| 
| Output: Returns true if light created, else false on any error.
|___________________________________________________________________*/

int d3d9_InitPointLight (
  int    index,
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
  D3DLIGHT9 light;

  memset (&light, 0, sizeof(D3DLIGHT9));
  light.Type           = D3DLIGHT_POINT;
  light.Diffuse.r    = diffuse_color_rgba[0];
  light.Diffuse.g    = diffuse_color_rgba[1];
  light.Diffuse.b    = diffuse_color_rgba[2];
  light.Diffuse.a    = diffuse_color_rgba[3];                         
  light.Specular.r   = specular_color_rgba[0];
  light.Specular.g   = specular_color_rgba[1];
  light.Specular.b   = specular_color_rgba[2];
  light.Specular.a   = specular_color_rgba[3];                         
  light.Ambient.r    = ambient_color_rgba[0];
  light.Ambient.g    = ambient_color_rgba[1];
  light.Ambient.b    = ambient_color_rgba[2];
  light.Ambient.a    = ambient_color_rgba[3];
  light.Position.x   = src_x;
  light.Position.y   = src_y;
  light.Position.z   = src_z;
  light.Range        = range;
  light.Attenuation0 = constant_attenuation;
  light.Attenuation1 = linear_attenuation;
  light.Attenuation2 = quadratic_attenuation;
  
  return (d3ddevice9->SetLight (index, &light) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_InitSpotLight
| 
| Output: Returns true if light created, else false on any error.
|___________________________________________________________________*/

int d3d9_InitSpotLight (
  int       index,                           
  float     src_x,
  float     src_y,
  float     src_z,
  float     dst_x,
  float     dst_y,
  float     dst_z,
  float     range,
  float     constant_attenuation,
  float     linear_attenuation,
  float     quadratic_attenuation, 
  float     inner_cone_angle,       // 0 - outer_cone_angle
  float     outer_cone_angle,       // 0 - 180
  float     falloff,                // intensity change from inner cone to outer cone
  float    *ambient_color_rgba,
  float    *diffuse_color_rgba,
  float    *specular_color_rgba )
{
  D3DLIGHT9 light;

  memset (&light, 0, sizeof(D3DLIGHT9));
  light.Type         = D3DLIGHT_POINT;
  light.Diffuse.r    = diffuse_color_rgba[0];
  light.Diffuse.g    = diffuse_color_rgba[1];
  light.Diffuse.b    = diffuse_color_rgba[2];
  light.Diffuse.a    = diffuse_color_rgba[3];                         
  light.Specular.r   = specular_color_rgba[0];
  light.Specular.g   = specular_color_rgba[1];
  light.Specular.b   = specular_color_rgba[2];
  light.Specular.a   = specular_color_rgba[3];                         
  light.Ambient.r    = ambient_color_rgba[0];
  light.Ambient.g    = ambient_color_rgba[1];
  light.Ambient.b    = ambient_color_rgba[2];
  light.Ambient.a    = ambient_color_rgba[3];
  light.Position.x   = src_x;
  light.Position.y   = src_y;
  light.Position.z   = src_z;
  light.Direction.x  = dst_x;
  light.Direction.y  = dst_y;
  light.Direction.z  = dst_z;
  light.Range        = range;
  light.Falloff      = falloff;
  light.Attenuation0 = constant_attenuation;
  light.Attenuation1 = linear_attenuation;
  light.Attenuation2 = quadratic_attenuation;
  light.Theta        = inner_cone_angle * DEGREES_TO_RADIANS;
  light.Phi          = outer_cone_angle * DEGREES_TO_RADIANS;   
  
  return (d3ddevice9->SetLight (index, &light) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_InitDirectionLight
| 
| Output: Returns true if light created, else false on any error.
|___________________________________________________________________*/

int d3d9_InitDirectionLight (
  int       index,
  float     dst_x,
  float     dst_y,
  float     dst_z, 
  float    *ambient_color_rgba,
  float    *diffuse_color_rgba,
  float    *specular_color_rgba )
{
  D3DLIGHT9 light;

  memset (&light, 0, sizeof(D3DLIGHT9));
  light.Type         = D3DLIGHT_DIRECTIONAL;
  light.Diffuse.r    = diffuse_color_rgba[0];
  light.Diffuse.g    = diffuse_color_rgba[1];
  light.Diffuse.b    = diffuse_color_rgba[2];
  light.Diffuse.a    = diffuse_color_rgba[3];                         
  light.Specular.r   = specular_color_rgba[0];
  light.Specular.g   = specular_color_rgba[1];
  light.Specular.b   = specular_color_rgba[2];
  light.Specular.a   = specular_color_rgba[3];                         
  light.Ambient.r    = ambient_color_rgba[0];
  light.Ambient.g    = ambient_color_rgba[1];
  light.Ambient.b    = ambient_color_rgba[2];
  light.Ambient.a    = ambient_color_rgba[3];
  light.Direction.x  = dst_x;
  light.Direction.y  = dst_y;
  light.Direction.z  = dst_z;

  return (d3ddevice9->SetLight (index, &light) == D3D_OK);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableLight
| 
| Output: Enables/disables a previously created light.
|___________________________________________________________________*/

void d3d9_EnableLight (int index, int flag)
{
  d3ddevice9->LightEnable (index, flag);
}

/*___________________________________________________________________
|
|	Function: d3d9_SetAmbientLight
| 
| Output: Sets the ambient light.
|___________________________________________________________________*/

void d3d9_SetAmbientLight (float *rgba)
{
  D3DCOLOR color = D3DCOLOR_ARGB((unsigned)(rgba[3]*255),(unsigned)(rgba[0]*255),(unsigned)(rgba[1]*255),(unsigned)(rgba[2]*255));

  d3ddevice9->SetRenderState (D3DRS_AMBIENT, color);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableSpecularLighting
| 
| Output: Enables/disables specular lighting.
|___________________________________________________________________*/

void d3d9_EnableSpecularLighting (int flag)
{
  d3ddevice9->SetRenderState (D3DRS_SPECULARENABLE, flag);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableVertexLighting
| 
| Output: Enables/disables vertex color lighting info.  If enabled
|   (the default) and specifying a diffuse vertex color, the output 
|   alpha value will equal to the diffuse alpha value of the vertex.
|
|   If disabled, the output alpha value will be equal to the alpha 
|   component of the diffuse material, clamped to a range of 0-255.
|___________________________________________________________________*/

void d3d9_EnableVertexLighting (int flag)
{
  d3ddevice9->SetRenderState (D3DRS_COLORVERTEX, flag);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableFog
| 
|	Input: 
| Output:
|___________________________________________________________________*/

void d3d9_EnableFog (int flag)
{
  d3ddevice9->SetRenderState (D3DRS_FOGENABLE, flag);
}

/*___________________________________________________________________
|
|	Function: d3d9_SetFogColor
| 
|	Input: 
| Output:
|___________________________________________________________________*/

void d3d9_SetFogColor (byte r, byte g, byte b)
{
  D3DCOLOR color;

  color = (D3DCOLOR)r << 16 | (D3DCOLOR)g << 8 | (D3DCOLOR)b;
  d3ddevice9->SetRenderState (D3DRS_FOGCOLOR, color);
}

/*___________________________________________________________________
|
|	Function: d3d9_SetLinearPixelFog
| 
|	Input: 
| Output:
|___________________________________________________________________*/

void d3d9_SetLinearPixelFog (float start_distance, float end_distance)
{
  d3ddevice9->SetRenderState (D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
  d3ddevice9->SetRenderState (D3DRS_FOGSTART, *(DWORD *)(&start_distance));
  d3ddevice9->SetRenderState (D3DRS_FOGEND,   *(DWORD *)(&end_distance));
}

/*___________________________________________________________________
|
|	Function: d3d9_SetExpPixelFog
| 
|	Input: 
| Output:
|___________________________________________________________________*/

void d3d9_SetExpPixelFog (float density)  // 0-1
{
  d3ddevice9->SetRenderState (D3DRS_FOGTABLEMODE, D3DFOG_EXP);
  d3ddevice9->SetRenderState (D3DRS_FOGDENSITY, *(DWORD *)(&density));
}

/*___________________________________________________________________
|
|	Function: d3d9_SetExp2PixelFog
| 
|	Input: 
| Output:
|___________________________________________________________________*/

void d3d9_SetExp2PixelFog (float density) // 0-1
{
  HRESULT hres;

  hres = d3ddevice9->SetRenderState (D3DRS_FOGTABLEMODE, D3DFOG_EXP2);

#ifdef _DEBUG
  if (hres != D3D_OK)
    DEBUG_ERROR ("d3d9_SetExp2PixelFog(): Error calling SetRenderState (FOGTABLEMODE)")
#endif

  hres = d3ddevice9->SetRenderState (D3DRS_FOGDENSITY, *(DWORD *)(&density));

#ifdef _DEBUG
  if (hres != D3D_OK)
    DEBUG_ERROR ("d3d9_SetExp2PixelFog(): Error calling SetRenderState (FOGDENSITY)")
#endif
}

/*___________________________________________________________________
|
|	Function: d3d9_SetLinearVertexFog
| 
|	Input: 
| Output:
|___________________________________________________________________*/

void d3d9_SetLinearVertexFog (float start_distance, float end_distance, int ranged_based)
{
  d3ddevice9->SetRenderState (D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
  d3ddevice9->SetRenderState (D3DRS_FOGSTART, *(DWORD *)(&start_distance));
  d3ddevice9->SetRenderState (D3DRS_FOGEND,   *(DWORD *)(&end_distance));
  // enable or disable range-based fog only if the hardware supports it
  if (d3dcaps9.RasterCaps & D3DPRASTERCAPS_FOGRANGE)
    d3ddevice9->SetRenderState (D3DRS_RANGEFOGENABLE, (DWORD)ranged_based);
}

/*___________________________________________________________________
|
|	Function: d3d9_SetMaterial
| 
| Output: Sets the current render material. 
|___________________________________________________________________*/

void d3d9_SetMaterial (
  float *ambient_color_rgba,
  float *diffuse_color_rgba,
  float *specular_color_rgba,
  float *emissive_color_rgba,
  float  specular_sharpness )
{
  D3DMATERIAL9 material;

  material.Ambient.r  = ambient_color_rgba[0];
  material.Ambient.g  = ambient_color_rgba[1];
  material.Ambient.b  = ambient_color_rgba[2];
  material.Ambient.a  = ambient_color_rgba[3];
  material.Diffuse.r  = diffuse_color_rgba[0];
  material.Diffuse.g  = diffuse_color_rgba[1];
  material.Diffuse.b  = diffuse_color_rgba[2];
  material.Diffuse.a  = diffuse_color_rgba[3];
  material.Specular.r = specular_color_rgba[0];
  material.Specular.g = specular_color_rgba[1];
  material.Specular.b = specular_color_rgba[2];
  material.Specular.a = specular_color_rgba[3];                      
  material.Emissive.r = emissive_color_rgba[0];
  material.Emissive.g = emissive_color_rgba[1];
  material.Emissive.b = emissive_color_rgba[2];
  material.Emissive.a = emissive_color_rgba[3];
  material.Power      = specular_sharpness;

  d3ddevice9->SetMaterial (&material);
}

/*___________________________________________________________________
|
|	Function: d3d9_GetMaterial
| 
| Output: Gets the current render material.
|___________________________________________________________________*/

void d3d9_GetMaterial (
  float *ambient_color_rgba,
  float *diffuse_color_rgba,
  float *specular_color_rgba,
  float *emissive_color_rgba,
  float *specular_sharpness )
{
  D3DMATERIAL9 material;

  d3ddevice9->GetMaterial (&material);

  ambient_color_rgba[0]  = material.Ambient.r;
  ambient_color_rgba[1]  = material.Ambient.g;
  ambient_color_rgba[2]  = material.Ambient.b;
  ambient_color_rgba[3]  = material.Ambient.a;
  diffuse_color_rgba[0]  = material.Diffuse.r;
  diffuse_color_rgba[1]  = material.Diffuse.g;
  diffuse_color_rgba[2]  = material.Diffuse.b;
  diffuse_color_rgba[3]  = material.Diffuse.a;
  specular_color_rgba[0] = material.Specular.r;
  specular_color_rgba[1] = material.Specular.g;
  specular_color_rgba[2] = material.Specular.b;
  specular_color_rgba[3] = material.Specular.a;
  emissive_color_rgba[0] = material.Emissive.r;
  emissive_color_rgba[1] = material.Emissive.g;
  emissive_color_rgba[2] = material.Emissive.b;
  emissive_color_rgba[3] = material.Emissive.a;
  *specular_sharpness    = material.Power;
}

/*___________________________________________________________________
|
|	Function: d3d9_InitTexture
| 
| Output: Creates a texture.
|
| Description: Creates a texture that can optionally use an alphamap.
|     Can also create an alpha only map.
|
|     Assumes input image/s (and alphamap/s, if any) are square, are a
|     a power of 2 and and <= max dimensions for a texture.
|___________________________________________________________________*/

#define TEXTURE_CREATE_ERROR(_str_) \
  {                                 \
    if (texture) {                  \
      texture->Release ();          \
      texture = NULL;               \
    }                               \
    DEBUG_WRITE (_str_)             \
  }

byte *d3d9_InitTexture (
  int       num_mip_levels, 
  byte    **image, 
  byte    **alphamap,
  int       dx, 
  int       dy,
  int       num_color_bits, // 0, 16, 24
  int       num_alpha_bits, // 0, 1, 8
  unsigned *size )          // size in memory taken up by this texture
{
  int i, mip_dx, mip_dy, ok;
  byte *the_image, *the_alphamap;
  DWORD usage;
  D3DPOOL pool;
  D3DFORMAT format;
  D3DLOCKED_RECT locked_rect;
  LPDIRECT3DSURFACE9 mip;
  LPDIRECT3DTEXTURE9 texture = NULL;

/*___________________________________________________________________
|
| Make sure input texture dimensions are acceptable
|___________________________________________________________________*/

  for (i=0, ok=TRUE; (i<num_mip_levels) AND ok; i++) {
    mip_dx = dx >> i;
    mip_dy = dy >> i;
    if ((mip_dx < 2)  OR
        (mip_dx > (int)(d3dcaps9.MaxTextureWidth))  OR
        (mip_dy < 2) OR
        (mip_dy > (int)(d3dcaps9.MaxTextureHeight))) {
      sprintf (debug_str, "d3d9_InitTexture(): ERROR, requested texture size (%dx%d) is not supported", mip_dx, mip_dy);
      TEXTURE_CREATE_ERROR (debug_str)
      ok = FALSE;
    }
  }

  // If creating a renderable texture, only 1 mip level is allowed
  if (image == NULL)
    num_mip_levels = 1;

/*___________________________________________________________________
|
| Pick a texture format to use
|___________________________________________________________________*/

  if (ok) {
    // For a renderable square texture, choose same format as screen
    if (image == NULL)
      format = SCREEN_FORMAT;
    // For a static square texture, choose best format
    else {
      // Choose best alpha format?
      if (num_alpha_bits != 0) {
        switch (SCREEN_FORMAT) {
          case D3DFMT_A8R8G8B8:
          case D3DFMT_X8R8G8B8: format = D3DFMT_A8R8G8B8;
                                break;
          case D3DFMT_X1R5G5B5: format = D3DFMT_A1R5G5B5;
                                break;
        }    
      }
      // Choose best non-alpha format?
      else {
        switch (SCREEN_FORMAT) {
          case D3DFMT_A8R8G8B8:
          case D3DFMT_X8R8G8B8: format = D3DFMT_X8R8G8B8;
                                break;
          case D3DFMT_X1R5G5B5: format = D3DFMT_X1R5G5B5;
                                break;
        }   
      }
    }

#ifdef _DEBUG
    {
      int r, g, b, a;
      switch (format) {
        case D3DFMT_A8R8G8B8: 
        case D3DFMT_X8R8G8B8: r = 8; g = 8; b = 8; a = 8; break;
        case D3DFMT_A1R5G5B5: r = 5; g = 5; b = 5; a = 1; break;
        case D3DFMT_R8G8B8:   r = 8; g = 8; b = 8; a = 0; break;
        case D3DFMT_X1R5G5B5: r = 5; g = 5; b = 5; a = 0; break;
      }
      sprintf (debug_str, "d3d9_InitTexture(): texture format chosen = %d/%d/%d/%d", r, g, b, a);
      DEBUG_WRITE (debug_str)
    }
#endif
  }

/*___________________________________________________________________
|
| Create the texture
|___________________________________________________________________*/

  if (ok) {
    // Set creation flags for a non-renderable texture
    if (image OR alphamap) {
      usage = 0;
      pool  = D3DPOOL_MANAGED;
    }
    // Set creation flags for a renderable texture
    else {
      usage = D3DUSAGE_RENDERTARGET;
      pool  = D3DPOOL_DEFAULT;
      // Make sure device supports renderable textures
      if (d3d9->CheckDeviceFormat (d3d_adapter, d3d_device_type, SCREEN_FORMAT, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, format) != D3D_OK) {
        TEXTURE_CREATE_ERROR ("d3d9_InitTexture(): ERROR device doesn't support renderable textures")
        ok = FALSE;
      }
    }
    if (ok) {
      // Create the texture
      if (d3ddevice9->CreateTexture (dx, dy, num_mip_levels, usage, format, pool, &texture, 0) != D3D_OK)
        TEXTURE_CREATE_ERROR ("d3d9_InitTexture(): ERROR creating a texture")
      else {
        // All mipmaps created ok?
        if (texture->GetLevelCount() != (DWORD)num_mip_levels) 
          TEXTURE_CREATE_ERROR ("d3d9_InitTexture(): ERROR, not enough mip levels created")
        else {
          *size = 0;
          // Load image data into texture?
          if (image OR alphamap) {
            // Load each mip level with data
            for (i=0; i<num_mip_levels; i++) {
              // Get interface to this mip level surface
              if (texture->GetSurfaceLevel (i, &mip) != D3D_OK) 
                TEXTURE_CREATE_ERROR ("d3d9_InitTexture(): ERROR getting access to a mip level")
              else {
                // Lock the surface
                if (mip->LockRect (&locked_rect, NULL, D3DLOCK_NOSYSLOCK) != D3D_OK) 
                  TEXTURE_CREATE_ERROR ("d3d9_InitTexture(): ERROR locking a mip level")
                else {
                  // Add to size
                  *size += ((dy>>i) * SURFACE_PITCH);
                  // Copy the data from the image to the texture
                  if (image)
                    the_image = image[i];
                  else
                    the_image = NULL;
                  if (alphamap)
                    the_alphamap = alphamap[i];
                  else
                    the_alphamap = NULL;
                  Copy_Pixels_To_Texture (the_image, the_alphamap, dx>>i, dy>>i, SURFACE_BUFFER, SURFACE_PITCH, format);
                  // Unlock the surface
	  		          mip->UnlockRect ();
                }
                mip->Release ();
              }
            } // for
          }
        }
      }
    }
  }

  return ((byte *)texture);
}

/*___________________________________________________________________
|
|	Function: d3d9_InitVolumeTexture
| 
| Output: Creates a volume texture.
|
| Description: Creates a texture that can optionally use an alphamap.
|     Can also create an alpha only map.
|
|     Assumes input image/s (and alphamap/s, if any) are square, are a
|     a power of 2 and and <= max dimensions for a texture.
|
|     Assumes num_slices is a power of 2.
|___________________________________________________________________*/

byte *d3d9_InitVolumeTexture (
  int       num_levels,
  int       num_slices, 
  byte    **image, 
  byte    **alphamap,
  int       dx, 
  int       dy,
  int       num_color_bits, // 0, 16, 24
  int       num_alpha_bits, // 0, 1, 8
  unsigned *size )          // size in memory taken up by this texture
{
  int i, j, n, mip_dx, mip_dy, ok, mip_levels_to_create;
  byte *the_image, *the_alphamap, *slice;
  D3DFORMAT format;
  D3DLOCKED_BOX locked_box;
  LPDIRECT3DVOLUMETEXTURE9 texture = NULL;

/*___________________________________________________________________
|
| Make sure input texture dimensions are acceptable
|___________________________________________________________________*/

  for (i=0, ok=TRUE; (i<num_levels) AND ok; i++) {
    mip_dx = dx >> i;
    mip_dy = dy >> i;
    if ((mip_dx < 2) OR
        (mip_dx > (int)(d3dcaps9.MaxTextureWidth)) OR
        (mip_dy < 2) OR
        (mip_dy > (int)(d3dcaps9.MaxTextureHeight))) {
      sprintf (debug_str, "d3d9_InitVolumeTexture(): ERROR, requested texture size (%dx%d) is not supported", mip_dx, mip_dy);
      TEXTURE_CREATE_ERROR (debug_str)
      ok = FALSE;
    }
  }

/*___________________________________________________________________
|
| If device doesn't support mipmapped volume textures, reduce to 1 mip level
|___________________________________________________________________*/

  if ((d3dcaps9.TextureCaps & D3DPTEXTURECAPS_MIPVOLUMEMAP) == 0) {
    mip_levels_to_create = 1;
    DEBUG_ERROR ("d3d9_InitVolumeTexture(): device doesn't support mipmapped volume textures")
  }
  else
    mip_levels_to_create = num_levels;

/*___________________________________________________________________
|
| Pick a texture format to use
|___________________________________________________________________*/

  if (ok) {
    // For a renderable square texture, choose same format as screen
    if (image == NULL)
      format = SCREEN_FORMAT;
    // For a static square texture, choose best format
    else {
      // Choose best alpha format?
      if (num_alpha_bits != 0) {
        switch (SCREEN_FORMAT) {
          case D3DFMT_A8R8G8B8:
          case D3DFMT_X8R8G8B8: format = D3DFMT_A8R8G8B8;
                                break;
          case D3DFMT_X1R5G5B5: format = D3DFMT_A1R5G5B5;
                                break;
        }    
      }
      // Choose best non-alpha format?
      else {
        switch (SCREEN_FORMAT) {
          case D3DFMT_A8R8G8B8:
          case D3DFMT_X8R8G8B8: format = D3DFMT_X8R8G8B8;
                                break;
          case D3DFMT_X1R5G5B5: format = D3DFMT_X1R5G5B5;
                                break;
        }   
      }
    }

#ifdef _DEBUG
    {
      int r, g, b, a;
      switch (format) {
        case D3DFMT_A8R8G8B8: 
        case D3DFMT_X8R8G8B8: r = 8; g = 8; b = 8; a = 8; break;
        case D3DFMT_A1R5G5B5: r = 5; g = 5; b = 5; a = 1; break;
        case D3DFMT_R8G8B8:   r = 8; g = 8; b = 8; a = 0; break;
        case D3DFMT_X1R5G5B5: r = 5; g = 5; b = 5; a = 0; break;
      }
      sprintf (debug_str, "d3d9_InitVolumeTexture(): texture format chosen = %d/%d/%d/%d", r, g, b, a);
      DEBUG_WRITE (debug_str)
    }
#endif
  }

/*___________________________________________________________________
|
| Create the texture
|___________________________________________________________________*/

  if (ok) {
    // Create the texture
    if (d3ddevice9->CreateVolumeTexture (dx, dy, num_slices, mip_levels_to_create, 0, format, D3DPOOL_MANAGED, &texture, 0) != D3D_OK)
      TEXTURE_CREATE_ERROR ("d3d9_InitVolumeTexture(): ERROR creating a texture")
    else {
      // All mipmaps created ok?
      if (texture->GetLevelCount() != (DWORD)mip_levels_to_create) 
        TEXTURE_CREATE_ERROR ("d3d9_InitVolumeTexture(): ERROR, not enough mip levels created")
      else {
        *size = 0;
        // Load image data into texture?
        if (image OR alphamap) {
          // Load each mip level with data
          n = 0;
          for (i=0; i<mip_levels_to_create; i++) {
            // Lock the mip level
            if (texture->LockBox (i, &locked_box, NULL, 0) != D3D_OK) 
              TEXTURE_CREATE_ERROR ("d3d9_InitVolumeTexture(): ERROR locking a mip level")
            else {
              // Load each slice with data
              for (j=0; (j<(num_slices>>i)) AND ok; j++) {
                // Add to size
                *size += ((dy>>i) * locked_box.SlicePitch);
                // Get pointer into texture to write this slice into
                slice = (byte *)(locked_box.pBits) + j * locked_box.SlicePitch;
                // Copy the data from the image to the texture
                if (image)
                  the_image = image[n];
                else
                  the_image = NULL;
                if (alphamap)
                  the_alphamap = alphamap[n];
                else
                  the_alphamap = NULL;
                Copy_Pixels_To_Texture (the_image, the_alphamap, dx>>i, dy>>i, slice, locked_box.RowPitch, format);
                n++;
              } // for
              texture->UnlockBox (i);
            }
          } // for
        }
      }
    }
  }

  return ((byte *)texture);
}

/*___________________________________________________________________
|
|	Function: d3d9_InitCubemapTexture
| 
| Output: Creates a cubic environment map texture.
|___________________________________________________________________*/

byte *d3d9_InitCubemapTexture (
  byte    **image,          // 6 faces or NULL to create an empty renderable cubemap
  byte    **alphamap,
  int       dimensions, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size )
{
  int i, ok;
  byte *the_image, *the_alphamap;
  DWORD usage;
  D3DPOOL pool;
  D3DFORMAT format;
  D3DLOCKED_RECT locked_rect;
  LPDIRECT3DSURFACE9 face;
  LPDIRECT3DCUBETEXTURE9 texture = NULL;

/*___________________________________________________________________
|
| Make sure input texture dimensions are acceptable
|___________________________________________________________________*/

  ok = TRUE;

  if ((dimensions < 2)                               OR
      (dimensions > (int)(d3dcaps9.MaxTextureWidth)) OR
      (dimensions < 2)                               OR
      (dimensions > (int)(d3dcaps9.MaxTextureHeight))) {
    sprintf (debug_str, "d3d9_InitCubemapTexture(): ERROR, requested texture size (%dx%d) is not supported", dimensions, dimensions);
    TEXTURE_CREATE_ERROR (debug_str)
    ok = FALSE;
  }

/*___________________________________________________________________
|
| Pick a texture format to use
|___________________________________________________________________*/

  if (ok) {
    // For a renderable cubemap, choose same format as screen
    if (image == NULL)
      format = SCREEN_FORMAT;
    // For a static cubemap, choose best format
    else {
      // Choose best alpha format?
      if (num_alpha_bits != 0) {
        switch (SCREEN_FORMAT) {
          case D3DFMT_A8R8G8B8:
          case D3DFMT_X8R8G8B8: format = D3DFMT_A8R8G8B8;
                                break;
          case D3DFMT_X1R5G5B5: format = D3DFMT_A1R5G5B5;
                                break;
        }    
      }
      // Choose best non-alpha format?
      else {
        switch (SCREEN_FORMAT) {
          case D3DFMT_A8R8G8B8:
          case D3DFMT_X8R8G8B8: format = D3DFMT_X8R8G8B8;
                                break;
          case D3DFMT_X1R5G5B5: format = D3DFMT_X1R5G5B5;
                                break;
        }   
      }
    }

#ifdef _DEBUG
    {
      int r, g, b, a;
      switch (format) {
        case D3DFMT_A8R8G8B8: 
        case D3DFMT_X8R8G8B8: r = 8; g = 8; b = 8; a = 8; break;
        case D3DFMT_A1R5G5B5: r = 5; g = 5; b = 5; a = 1; break;
        case D3DFMT_R8G8B8:   r = 8; g = 8; b = 8; a = 0; break;
        case D3DFMT_X1R5G5B5: r = 5; g = 5; b = 5; a = 0; break;
      }
      sprintf (debug_str, "d3d9_InitCubemapTexture(): texture format chosen = %d/%d/%d/%d", r, g, b, a);
      DEBUG_WRITE (debug_str)
    }
#endif
  }

/*___________________________________________________________________
|
| Create the texture
|___________________________________________________________________*/

  if (ok) {
    // Set creation flags for a non-renderable cubemap
    if (image OR alphamap) {
      usage = 0;
      pool  = D3DPOOL_MANAGED;
    }
    // Set creation flags for a renderable cubemap
    else {
      usage = D3DUSAGE_RENDERTARGET;
      pool  = D3DPOOL_DEFAULT;
      // Make sure device supports renderable cubemaps
      if (d3d9->CheckDeviceFormat (d3d_adapter, d3d_device_type, SCREEN_FORMAT, D3DUSAGE_RENDERTARGET, D3DRTYPE_CUBETEXTURE, format) != D3D_OK) {
        TEXTURE_CREATE_ERROR ("d3d9_InitCubemapTexture(): ERROR device doesn't support renderable cubemaps")
        ok = FALSE;
      }
    }
    if (ok) {
      // Create the texture
      if (d3ddevice9->CreateCubeTexture (dimensions, 1, usage, format, pool, &texture, 0) != D3D_OK)
        TEXTURE_CREATE_ERROR ("d3d9_InitCubemapTexture(): ERROR creating a texture")
      else {
        *size = 0;
        // Load image data into cubemap?
        if (image OR alphamap) {
          // Load each face with data?
          for (i=0; (i<6) AND ok; i++) {
            // Get interface to this face surface
            if (texture->GetCubeMapSurface ((D3DCUBEMAP_FACES)i, 0, &face) != D3D_OK) 
              TEXTURE_CREATE_ERROR ("d3d9_InitCubemapTexture(): ERROR getting access to a face")
            else {
              // Lock the surface
              if (face->LockRect (&locked_rect, NULL, D3DLOCK_NOSYSLOCK) != D3D_OK) 
                TEXTURE_CREATE_ERROR ("d3d9_InitCubemapTexture(): ERROR locking a face")
              else {
                // Add to size
                *size += (dimensions * SURFACE_PITCH);
                // Copy the data from the image to the texture
                if (image)
                  the_image = image[i];
                else
                  the_image = NULL;
                if (alphamap)
                  the_alphamap = alphamap[i];
                else
                  the_alphamap = NULL;
                Copy_Pixels_To_Texture (the_image, the_alphamap, dimensions, dimensions, SURFACE_BUFFER, SURFACE_PITCH, format);
                // Unlock the surface
	  		        face->UnlockRect ();
              }
              face->Release ();
            }
          } // for
        }
      }
    }
  }

  return ((byte *)texture);
}

/*___________________________________________________________________
|
|	Function: Copy_Pixels_To_Texture
| 
|	Input: Called from d3d9_InitTexture(), d3d9_InitCubemapTexture()
| Output: Fills a texture with data.
|___________________________________________________________________*/

static void Copy_Pixels_To_Texture (
	byte     *image,     // NULL if none
  byte     *alphamap,  // NULL if none
  int       dx,
  int       dy,
  byte     *surfdata,
  int       surfpitch, 
  D3DFORMAT texture_format )
{
  int i, x, y, dx_bytes, texel_size;
  unsigned src_color, dst_color;
  byte r, g, b, a, *surfdataptr;
  DWORD dummy;
  // Data about pixel format for texture
  WORD	loREDbit,   numREDbits;	
  WORD	loGREENbit, numGREENbits;
  WORD	loBLUEbit,  numBLUEbits;
  WORD  loALPHAbit, numALPHAbits;

/*___________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  Get_Pixel_Format_Data (texture_format, 
                         &loREDbit,
                         &loGREENbit,
                         &loBLUEbit,
                         &loALPHAbit,
                         &numREDbits,
                         &numGREENbits,
                         &numBLUEbits,
                         &numALPHAbits,
                         &dummy, &dummy, &dummy, &dummy,
                         &texel_size);
  
  dx_bytes = dx * d3d9_pixel_size;

/*___________________________________________________________________
|
| Copy an image/alphamap to the texture
|___________________________________________________________________*/

  if (image AND alphamap) {
  
    for (y=0; y<dy; y++) {
      surfdataptr = surfdata;

      // Copy 1 pixel at a time
      for (x=0; x<dx; x++) {
        // Get the source color
        memcpy (&src_color, &image[x*d3d9_pixel_size], d3d9_pixel_size);
        d3d9_Pixel_To_RGB (src_color, &r, &g, &b);
        a = alphamap[x];

        // Convert to the destination color/alpha
        dst_color = ((DWORD)r >> (8 - numREDbits  ) << loREDbit  ) |
                    ((DWORD)g >> (8 - numGREENbits) << loGREENbit) |
                    ((DWORD)b >> (8 - numBLUEbits ) << loBLUEbit ) |
                    ((DWORD)a >> (8 - numALPHAbits) << loALPHAbit);
        // Write this color to the texture (texel)
        for (i=0; i<texel_size; i++)
          *surfdataptr++ = ((byte *)&dst_color)[i];

      }  
      // Incr pointers
      surfdata += surfpitch;
      image    += dx_bytes;
      alphamap += dx;
    }
  }

/*___________________________________________________________________
|
| Copy an image to the texture
|___________________________________________________________________*/

  else if (image) {

    for (y=0; y<dy; y++) {
      surfdataptr = surfdata;

      // Copy 1 pixel at a time
      for (x=0; x<dx; x++) {
        // Get the source color
        memcpy (&src_color, &image[x*d3d9_pixel_size], d3d9_pixel_size);
        d3d9_Pixel_To_RGB (src_color, &r, &g, &b);

        // Convert to the destination color/alpha
        dst_color = ((DWORD)r >> (8 - numREDbits  ) << loREDbit  ) |
                    ((DWORD)g >> (8 - numGREENbits) << loGREENbit) |
                    ((DWORD)b >> (8 - numBLUEbits ) << loBLUEbit ); 
        // Write this color to the texture (texel)
        for (i=0; i<texel_size; i++)
          *surfdataptr++ = ((byte *)&dst_color)[i];
        
      }
      // Incr pointers
      surfdata += surfpitch;
      image    += dx_bytes;
    }
  }

/*___________________________________________________________________
|
| Copy an alphamap to the texture
|___________________________________________________________________*/

  else if (alphamap) {

    for (y=0; y<dy; y++) {
      surfdataptr = surfdata;

      // Copy 1 pixel at a time
      for (x=0; x<dx; x++) {
        // Get the source alpha
        a = alphamap[x];

        // Convert to the destination color/alpha
        dst_color = ((DWORD)a >> (8 - numALPHAbits) << loALPHAbit);
        // Write this color to the texture (texel)
        for (i=0; i<texel_size; i++)
          *surfdataptr++ = ((byte *)&dst_color)[i];
        
      }
      // Incr pointers
      surfdata += surfpitch;
      alphamap += dx;
    }
  }
}

/*___________________________________________________________________
|
|	Function: d3d9_FreeTexture
| 
| Output: Frees a texture.
|___________________________________________________________________*/

void d3d9_FreeTexture (byte *texture)
{
  if (texture)
		((LPDIRECT3DTEXTURE9)texture)->Release ();
}

/*___________________________________________________________________
|
|	Function: d3d9_SetTexture
| 
| Output: Sets the current render texture.
|___________________________________________________________________*/

void d3d9_SetTexture (int stage, byte *texture)
{
  HRESULT hres;

/*___________________________________________________________________
|
| Verify input parameters
|___________________________________________________________________*/

  assert ((stage >= 0) AND (stage < NUM_TEXTURE_STAGES));  

/*___________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  hres = d3ddevice9->SetTexture (stage, (LPDIRECT3DTEXTURE9)texture);

#ifdef _DEBUG
  if (hres != D3D_OK)
    DEBUG_ERROR ("d3d9_SetTexture(): ERROR")
#endif
}

/*___________________________________________________________________
|
|	Function: d3d9_SetTextureAddressingMode
| 
| Output: Sets the texture addressing mode for a texture stage.
|___________________________________________________________________*/

void d3d9_SetTextureAddressingMode (int stage, int dimension, int addressing_mode)
{
  D3DSAMPLERSTATETYPE type;
  D3DTEXTUREADDRESS mode[5] = {
    D3DTADDRESS_WRAP,
    D3DTADDRESS_MIRROR,
    D3DTADDRESS_CLAMP,
    D3DTADDRESS_BORDER,
    D3DTADDRESS_MIRRORONCE
  };

  // Are input parameters are correct?
  if ((stage >= 0)                 AND
      (stage < NUM_TEXTURE_STAGES) AND
      (addressing_mode >= 1)       AND
      (addressing_mode <= 5)       AND
      (dimension & (TEXTURE_DIMENSION_U | TEXTURE_DIMENSION_V))) {
    // Set the addressing mode for this texture stage
    if (dimension & TEXTURE_DIMENSION_U) {
      type = D3DSAMP_ADDRESSU;
      d3ddevice9->SetSamplerState (stage, type, mode[addressing_mode-1]);
    }
    if (dimension & TEXTURE_DIMENSION_V) {
      type = D3DSAMP_ADDRESSV;
      d3ddevice9->SetSamplerState (stage, type, mode[addressing_mode-1]);
    }
    if (dimension & TEXTURE_DIMENSION_W) {
      type = D3DSAMP_ADDRESSW;
      d3ddevice9->SetSamplerState (stage, type, mode[addressing_mode-1]);
    }
  }
}

/*___________________________________________________________________
|
|	Function: d3d9_SetTextureBorderColor
| 
| Output: Sets the texture border color.
|___________________________________________________________________*/

void d3d9_SetTextureBorderColor (int stage, byte r, byte g, byte b, byte a)
{
  HRESULT hres;

  hres = d3ddevice9->SetSamplerState (stage, D3DSAMP_BORDERCOLOR, (D3DCOLOR)d3d9_RGBA_To_Pixel (r,g,b,a));

#ifdef _DEBUG
  if (hres != D3D_OK)
    DEBUG_ERROR ("d3d9_SetTextureBorderColor(): ERROR")
#endif
}

/*___________________________________________________________________
|
|	Function: d3d9_SetTextureFiltering
| 
| Output: Sets the texture filtering algorithm for a texture stage.
|
| Description: If anisotropic filtering is supported, the anisotropy_level
|   defines the amount of filtering desired from 1 (lowest) to 100 (highest).
|   A low value will render more quickly.  A high value will produce the best
|   quality.
|___________________________________________________________________*/

void d3d9_SetTextureFiltering (
  int stage, 
  int filter_type, 
  int anisotropy_level )  // amount of filtering (1-100), 1=min, 100=max
{
  int filter_level;

  switch (filter_type) {
    case TEXTURE_FILTERTYPE_POINT:
      d3ddevice9->SetSamplerState (stage, D3DSAMP_MINFILTER, D3DTEXF_POINT);
      d3ddevice9->SetSamplerState (stage, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
      d3ddevice9->SetSamplerState (stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
      break;
    case TEXTURE_FILTERTYPE_LINEAR:      
      d3ddevice9->SetSamplerState (stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      d3ddevice9->SetSamplerState (stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      d3ddevice9->SetSamplerState (stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
      break;
    case TEXTURE_FILTERTYPE_TRILINEAR:
      d3ddevice9->SetSamplerState (stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
      d3ddevice9->SetSamplerState (stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
      d3ddevice9->SetSamplerState (stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
      break;
    case TEXTURE_FILTERTYPE_ANISOTROPIC:
      filter_level = d3dcaps9.MaxAnisotropy;
      // Is anisotropic filtering supported?
      if (filter_level > 1) {
        // Make sure anisotropy level is within correct bounds
        if (anisotropy_level < 1)
          anisotropy_level = 1;
        else if (anisotropy_level > 100)
          anisotropy_level = 100;
        // Compute the amount of filtering required
        filter_level = (int) ((float)filter_level * ((float)anisotropy_level/100.0));
        // Don't go less than 2 (a value of 1 will disable this filtering algorithm)
        if (filter_level < 2)
          filter_level = 2;
        // Enable anisotropic filtering
        d3ddevice9->SetSamplerState (stage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
        d3ddevice9->SetSamplerState (stage, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
        d3ddevice9->SetSamplerState (stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        d3ddevice9->SetSamplerState (stage, D3DSAMP_MAXANISOTROPY, filter_level);
      }
      // If anisotropic filtering not supported, use trilinear filtering
      else {
        d3ddevice9->SetSamplerState (stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        d3ddevice9->SetSamplerState (stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        d3ddevice9->SetSamplerState (stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
      }
      break;
  }
}

/*___________________________________________________________________
|
|	Function: d3d9_SetTextureCoordinates
| 
| Output: Sets the set (0-7) of texture coordinates in the object to 
|     use for this texture stage.
|___________________________________________________________________*/

void d3d9_SetTextureCoordinates (
  int stage,              // texture blending stage 0-7
  int coordinate_stage )  // set of texture coordinates in the object 0-7 (-1=cubemap)
{
  // Input parameters valid?
  if ((stage >= 0) AND (stage < NUM_TEXTURE_STAGES) AND (coordinate_stage >= -1) AND (coordinate_stage < NUM_TEXTURE_STAGES)) {
    if (coordinate_stage == -1) 
      d3ddevice9->SetTextureStageState (stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
    else 
      d3ddevice9->SetTextureStageState (stage, D3DTSS_TEXCOORDINDEX, coordinate_stage);
  }
}
                          
/*___________________________________________________________________
|
|	Function: d3d9_SetTextureCoordinateWrapping
| 
| Output: Sets the texture wrapping for a set of texture coordinates.
|     If texture wrapping is enabled it changes the way D3D interprets
|     the shortest distance between 2 points on a texture.  By enabling
|     texture wrapping in one direction, it causes the texture to be
|     treated as if it's wrapped around something.
|___________________________________________________________________*/

void d3d9_SetTextureCoordinateWrapping (
  int coordinate_stage, // set of texture coordinates in the object 0-7
  int wrap_s,           // boolean (u dimension)
  int wrap_t,           // boolean (v dimension)
  int wrap_r,           // boolean
  int wrap_q )          // boolean
{
  D3DRENDERSTATETYPE type[8] = {
    D3DRS_WRAP0,
    D3DRS_WRAP1,
    D3DRS_WRAP2,
    D3DRS_WRAP3,
    D3DRS_WRAP4,
    D3DRS_WRAP5,
    D3DRS_WRAP6,
    D3DRS_WRAP7
  };
  DWORD state;
  HRESULT hres;

#ifdef _DEBUG
  if ((coordinate_stage < 0) OR (coordinate_stage > NUM_TEXTURE_STAGES-1)) {
    char str[256];
    sprintf (str, "d3d9_SetTextureCoordinateWrapping (%d, ?, ?, ?): param 1 out of bounds (0-7)", coordinate_stage);
    DEBUG_ERROR (str)
    return;
  }
#endif  
  
  state = 0;
  if (wrap_s)
    state |= D3DWRAPCOORD_0;
  if (wrap_t)
    state |= D3DWRAPCOORD_1;
  if (wrap_r)
    state |= D3DWRAPCOORD_2;
  if (wrap_q)
    state |= D3DWRAPCOORD_3;
  
  hres = d3ddevice9->SetRenderState (type[coordinate_stage], state);

#ifdef _DEBUG
  if (hres != D3D_OK)
    DEBUG_ERROR ("d3d9_SetTextureCoordinateWrapping(): ERROR")
#endif
}

/*___________________________________________________________________
|
|	Function: d3d9_SetTextureFactor
| 
| Output: Sets the texture factor used by some texture blending operations.
|___________________________________________________________________*/

void d3d9_SetTextureFactor (byte r, byte g, byte b, byte a)
{
  D3DCOLOR color;

  color = (D3DCOLOR)a << 24 | (D3DCOLOR)r << 16 | (D3DCOLOR)g << 8 | (D3DCOLOR)b;
  d3ddevice9->SetRenderState (D3DRS_TEXTUREFACTOR, color);
}

/*___________________________________________________________________
|
|	Function: d3d9_PreLoadManagedTexture
| 
| Output: Manually puts a managed texture into video memory.
|___________________________________________________________________*/

void d3d9_PreLoadManagedTexture (byte *texture)
{
  if (texture)
    ((LPDIRECT3DTEXTURE9)texture)->PreLoad ();
}

/*___________________________________________________________________
|
|	Function: d3d9_EvictManagedTextures
| 
| Output: Evicts all managed textures from local or nonlocal video 
|   memory.
|___________________________________________________________________*/

void d3d9_EvictManagedTextures ()
{
  d3ddevice9->EvictManagedResources ();
}

/*____________________________________________________________________
|
| Function: d3d9_SetTextureColorOp
|
| Output: Sets the texture blending color operation:
|     default for stage 0 is MODULATE, all other stages is DISABLE
|     default arg1 is TEXTURE
|     default arg2 is CURRENT
|___________________________________________________________________*/

void d3d9_SetTextureColorOp (int stage, int texture_colorop, int texture_arg1, int texture_arg2)
{
  D3DTEXTUREOP op;
  DWORD arg1, arg2;
  
  switch (texture_colorop) {
    case TEXTURE_COLOROP_DISABLE:                   op = D3DTOP_DISABLE;                   break;
    case TEXTURE_COLOROP_SELECTARG1:                op = D3DTOP_SELECTARG1;                break;
    case TEXTURE_COLOROP_SELECTARG2:                op = D3DTOP_SELECTARG2;                break;
    case TEXTURE_COLOROP_MODULATE:                  op = D3DTOP_MODULATE;                  break;
    case TEXTURE_COLOROP_MODULATE2X:                op = D3DTOP_MODULATE2X;                break;
    case TEXTURE_COLOROP_MODULATE4X:                op = D3DTOP_MODULATE4X;                break;
    case TEXTURE_COLOROP_ADD:                       op = D3DTOP_ADD;                       break;
    case TEXTURE_COLOROP_ADDSIGNED:                 op = D3DTOP_ADDSIGNED;                 break;
    case TEXTURE_COLOROP_ADDSIGNED2X:               op = D3DTOP_ADDSIGNED2X;               break;
    case TEXTURE_COLOROP_SUBTRACT:                  op = D3DTOP_SUBTRACT;                  break;
    case TEXTURE_COLOROP_ADDSMOOTH:                 op = D3DTOP_ADDSMOOTH;                 break;
    case TEXTURE_COLOROP_BLENDDIFFUSEALPHA:         op = D3DTOP_BLENDDIFFUSEALPHA;         break;
    case TEXTURE_COLOROP_BLENDTEXTUREALPHA:         op = D3DTOP_BLENDTEXTUREALPHA;         break;
    case TEXTURE_COLOROP_BLENDFACTORALPHA:          op = D3DTOP_BLENDFACTORALPHA;          break;
    case TEXTURE_COLOROP_BLENDTEXTUREALPHAPM:       op = D3DTOP_BLENDTEXTUREALPHAPM;       break;
    case TEXTURE_COLOROP_BLENDCURRENTALPHA:         op = D3DTOP_BLENDCURRENTALPHA;         break;
    case TEXTURE_COLOROP_PREMODULATE:               op = D3DTOP_PREMODULATE;               break;
    case TEXTURE_COLOROP_MODULATEALPHA_ADDCOLOR:    op = D3DTOP_MODULATEALPHA_ADDCOLOR;    break;
    case TEXTURE_COLOROP_MODULATECOLOR_ADDALPHA:    op = D3DTOP_MODULATECOLOR_ADDALPHA;    break;
    case TEXTURE_COLOROP_MODULATEINVALPHA_ADDCOLOR: op = D3DTOP_MODULATEINVALPHA_ADDCOLOR; break;
    case TEXTURE_COLOROP_MODULATEINVCOLOR_ADDALPHA: op = D3DTOP_MODULATEINVCOLOR_ADDALPHA; break;
    case TEXTURE_COLOROP_BUMPENVMAP:                op = D3DTOP_BUMPENVMAP;                break;
    case TEXTURE_COLOROP_BUMPENVMAPLUMINANCE:       op = D3DTOP_BUMPENVMAPLUMINANCE;       break;
    case TEXTURE_COLOROP_DOTPRODUCT3:               op = D3DTOP_DOTPRODUCT3;               break;
    case TEXTURE_COLOROP_MULTIPLYADD:               op = D3DTOP_MULTIPLYADD;               break;
    case TEXTURE_COLOROP_LERP:                      op = D3DTOP_LERP;                      break;
  }
  switch (texture_arg1) {                              
    case TEXTURE_ARG_CURRENT:   arg1 = D3DTA_CURRENT;  break;
    case TEXTURE_ARG_DIFFUSE:   arg1 = D3DTA_DIFFUSE;  break;
    case TEXTURE_ARG_TEXTURE:   arg1 = D3DTA_TEXTURE;  break;
    case TEXTURE_ARG_TFACTOR:   arg1 = D3DTA_TFACTOR;  break;
    case TEXTURE_ARG_SPECULAR:  arg1 = D3DTA_SPECULAR; break;
  }
  switch (texture_arg2) {                              
    case TEXTURE_ARG_CURRENT:   arg2 = D3DTA_CURRENT;  break;
    case TEXTURE_ARG_DIFFUSE:   arg2 = D3DTA_DIFFUSE;  break;
    case TEXTURE_ARG_TEXTURE:   arg2 = D3DTA_TEXTURE;  break;
    case TEXTURE_ARG_TFACTOR:   arg2 = D3DTA_TFACTOR;  break;
    case TEXTURE_ARG_SPECULAR:  arg2 = D3DTA_SPECULAR; break;
  }

  d3ddevice9->SetTextureStageState (stage, D3DTSS_COLOROP,   op);
  d3ddevice9->SetTextureStageState (stage, D3DTSS_COLORARG1, arg1);
  d3ddevice9->SetTextureStageState (stage, D3DTSS_COLORARG2, arg2);
}

/*____________________________________________________________________
|
| Function: d3d9_SetTextureAlphaOp
|
| Output: Sets the texture blending alpha operation.
|     default for stage 0 is SELECTARG1, all other stages is DISABLE
|     default arg1 is TEXTURE
|     default arg2 is CURRENT
|___________________________________________________________________*/

void d3d9_SetTextureAlphaOp (int stage, int texture_alphaop, int texture_arg1, int texture_arg2)
{
  D3DTEXTUREOP op;
  DWORD arg1, arg2;
  
  switch (texture_alphaop) {                    
    case TEXTURE_ALPHAOP_DISABLE:             op = D3DTOP_DISABLE;             break;
    case TEXTURE_ALPHAOP_SELECTARG1:          op = D3DTOP_SELECTARG1;          break;
    case TEXTURE_ALPHAOP_SELECTARG2:          op = D3DTOP_SELECTARG2;          break;
    case TEXTURE_ALPHAOP_MODULATE:            op = D3DTOP_MODULATE;            break;
    case TEXTURE_ALPHAOP_MODULATE2X:          op = D3DTOP_MODULATE2X;          break;
    case TEXTURE_ALPHAOP_MODULATE4X:          op = D3DTOP_MODULATE4X;          break;
    case TEXTURE_ALPHAOP_ADD:                 op = D3DTOP_ADD;                 break;
    case TEXTURE_ALPHAOP_ADDSIGNED:           op = D3DTOP_ADDSIGNED;           break;
    case TEXTURE_ALPHAOP_ADDSIGNED2X:         op = D3DTOP_ADDSIGNED2X;         break;
    case TEXTURE_ALPHAOP_SUBTRACT:            op = D3DTOP_SUBTRACT;            break;
    case TEXTURE_ALPHAOP_ADDSMOOTH:           op = D3DTOP_ADDSMOOTH;           break;
    case TEXTURE_ALPHAOP_BLENDDIFFUSEALPHA:   op = D3DTOP_BLENDDIFFUSEALPHA;   break;
    case TEXTURE_ALPHAOP_BLENDTEXTUREALPHA:   op = D3DTOP_BLENDTEXTUREALPHA;   break;
    case TEXTURE_ALPHAOP_BLENDFACTORALPHA:    op = D3DTOP_BLENDFACTORALPHA;    break;
    case TEXTURE_ALPHAOP_BLENDTEXTUREALPHAPM: op = D3DTOP_BLENDTEXTUREALPHAPM; break;
    case TEXTURE_ALPHAOP_BLENDCURRENTALPHA:   op = D3DTOP_BLENDCURRENTALPHA;   break;
    case TEXTURE_ALPHAOP_PREMODULATE:         op = D3DTOP_PREMODULATE;         break;
    case TEXTURE_ALPHAOP_DOTPRODUCT3:         op = D3DTOP_DOTPRODUCT3;         break;
    case TEXTURE_ALPHAOP_MULTIPLYADD:         op = D3DTOP_MULTIPLYADD;         break;
    case TEXTURE_ALPHAOP_LERP:                op = D3DTOP_LERP;                break;
  }
  switch (texture_arg1) {                              
    case TEXTURE_ARG_CURRENT:   arg1 = D3DTA_CURRENT;  break;
    case TEXTURE_ARG_DIFFUSE:   arg1 = D3DTA_DIFFUSE;  break;
    case TEXTURE_ARG_TEXTURE:   arg1 = D3DTA_TEXTURE;  break;
    case TEXTURE_ARG_TFACTOR:   arg1 = D3DTA_TFACTOR;  break;
    case TEXTURE_ARG_SPECULAR:  arg1 = D3DTA_SPECULAR; break;
  }
  switch (texture_arg2) {                              
    case TEXTURE_ARG_CURRENT:   arg2 = D3DTA_CURRENT;  break;
    case TEXTURE_ARG_DIFFUSE:   arg2 = D3DTA_DIFFUSE;  break;
    case TEXTURE_ARG_TEXTURE:   arg2 = D3DTA_TEXTURE;  break;
    case TEXTURE_ARG_TFACTOR:   arg2 = D3DTA_TFACTOR;  break;
    case TEXTURE_ARG_SPECULAR:  arg2 = D3DTA_SPECULAR; break;
  }

  d3ddevice9->SetTextureStageState (stage, D3DTSS_ALPHAOP,   op);
  d3ddevice9->SetTextureStageState (stage, D3DTSS_ALPHAARG1, arg1);
  d3ddevice9->SetTextureStageState (stage, D3DTSS_ALPHAARG2, arg2);
}

/*____________________________________________________________________
|
| Function: d3d9_SetTextureColorFactor
|
| Output: Sets the texture blending color factor.  This is the color used
|     for multiple texture blending w/ TFACTOR blending arg or 
|     BLENDFACTORALPHA operation.
|___________________________________________________________________*/

void d3d9_SetTextureColorFactor (float *rgba)
{
  D3DCOLOR color = D3DCOLOR_ARGB((unsigned)(rgba[3]*255),(unsigned)(rgba[0]*255),(unsigned)(rgba[1]*255),(unsigned)(rgba[2]*255));

  d3ddevice9->SetRenderState (D3DRS_TEXTUREFACTOR, color);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableCubemapTextureReflections
| 
| Output: Enables/disables correct cubemap texture reflection processing.
|___________________________________________________________________*/

void d3d9_EnableCubemapTextureReflections (int flag)
{
  HRESULT hres;

  hres = d3ddevice9->SetRenderState (D3DRS_NORMALIZENORMALS, flag);

#ifdef _DEBUG
  if (hres != D3D_OK) 
    DEBUG_ERROR ("d3d9_EnableCubemapTextureReflections(): ERROR")
#endif
}

/*___________________________________________________________________
|
|	Function: d3d9_GetTextureSurface
| 
| Output: Returns the surface ptr for the texture.  Should be used
|   with renderable textures only.
|
|   This increases the reference count on the IDirect3DSurface9 interface.
|   Failure to call Release() when done using the interface results in 
|   a memory leak.
|___________________________________________________________________*/

byte *d3d9_GetTextureSurface (byte *texture)
{
  LPDIRECT3DSURFACE9 surf;

  // Get interface to this mip level surface
  if (((LPDIRECT3DTEXTURE9)texture)->GetSurfaceLevel (0, &surf) != D3D_OK) {
#ifdef _DEBUG
    DEBUG_ERROR ("d3d9_GetTextureSurface(): ERROR getting surface")
#endif
    surf = NULL;
  }

  return ((byte *)surf);
}

/*___________________________________________________________________
|
|	Function: d3d9_GetTextureCubemapSurface
| 
| Output: Returns the surface ptr for the cubemap texture face.  Should 
|   be used with renderable textures only.
|
|   This increases the reference count on the IDirect3DSurface9 interface.
|   Failure to call Release() when done using the interface results in 
|   a memory leak.
|___________________________________________________________________*/

byte *d3d9_GetTextureCubemapSurface (byte *texture, int face)
{
  LPDIRECT3DSURFACE9 surf;

  // Get interface to this mip level surface
  if (((LPDIRECT3DCUBETEXTURE9)texture)->GetCubeMapSurface ((D3DCUBEMAP_FACES)face, 0, &surf) != D3D_OK) {
#ifdef _DEBUG
    DEBUG_ERROR ("d3d9_GetTextureCubemapSurface(): ERROR getting surface")
#endif
    surf = NULL;
  }

  return ((byte *)surf);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableAlphaBlending
| 
| Output: Enables/disables alpha blending (if alpha-blending is 
|   supported).
|___________________________________________________________________*/

void d3d9_EnableAlphaBlending (int flag)
{
  HRESULT hres;

  hres = d3ddevice9->SetRenderState (D3DRS_ALPHABLENDENABLE, flag);

#ifdef _DEBUG
  if (hres != D3D_OK) 
    DEBUG_ERROR ("d3d9_EnableAlphaBlending(): ERROR")
#endif
}

/*___________________________________________________________________
|
|	Function: d3d9_SetAlphaBlendFactor
| 
| Output: Sets the alpha blending source and destination factors:
|   pixel_color = (src_pixel * src_blend_factor) + 
|                 (dst_pixel * dst_blend_factor)
|___________________________________________________________________*/

void d3d9_SetAlphaBlendFactor (int src_blend_factor, int dst_blend_factor)
{
  HRESULT hres;
  static DWORD caps [11] = {
    D3DPBLENDCAPS_ZERO,
    D3DPBLENDCAPS_ONE,
    D3DPBLENDCAPS_SRCCOLOR,
    D3DPBLENDCAPS_DESTCOLOR,
    D3DPBLENDCAPS_SRCALPHA,
    D3DPBLENDCAPS_DESTALPHA,
    D3DPBLENDCAPS_INVSRCCOLOR,
    D3DPBLENDCAPS_INVDESTCOLOR,
    D3DPBLENDCAPS_INVSRCALPHA,
    D3DPBLENDCAPS_INVDESTALPHA,
    D3DPBLENDCAPS_SRCALPHASAT
  };
  static DWORD factor [11] = {
    D3DBLEND_ZERO,
    D3DBLEND_ONE,
    D3DBLEND_SRCCOLOR,
    D3DBLEND_DESTCOLOR,
    D3DBLEND_SRCALPHA,
    D3DBLEND_DESTALPHA,
    D3DBLEND_INVSRCCOLOR,
    D3DBLEND_INVDESTCOLOR,
    D3DBLEND_INVSRCALPHA,
    D3DBLEND_INVDESTALPHA,  
    D3DBLEND_SRCALPHASAT
  };

  // Is this source blend factor supported?
  if (d3dcaps9.SrcBlendCaps & caps[src_blend_factor-1]) {
    // Set the source blend factor
    hres = d3ddevice9->SetRenderState (D3DRS_SRCBLEND, factor[src_blend_factor-1]);
#ifdef _DEBUG
    if (hres != D3D_OK) 
      DEBUG_ERROR ("d3d9_SetAlphaBlendFactor(): error setting src blend factor")
#endif
  }
#ifdef _DEBUG
  else
    DEBUG_ERROR ("d3d9_SetAlphaBlendFactor(): src blend factor not supported")
#endif

  // Is this destination blend factor supported?
  if (d3dcaps9.DestBlendCaps & caps[dst_blend_factor-1]) {
    // Set the destination blend factor
    hres = d3ddevice9->SetRenderState (D3DRS_DESTBLEND, factor[dst_blend_factor-1]);
#ifdef _DEBUG
    if (hres != D3D_OK) 
      DEBUG_ERROR ("d3d9_SetAlphaBlendFactor(): error setting dst blend factor")
#endif
  }
#ifdef _DEBUG
  else
    DEBUG_ERROR ("d3d9_SetAlphaBlendFactor(): dst blend factor not supported")
#endif
}

/*___________________________________________________________________
|
|	Function: d3d9_AlphaTestingAvailable
| 
| Output: Returns true if alpha testing is available using the greater
|     than or equal to alpha reference value test.
|___________________________________________________________________*/

int d3d9_AlphaTestingAvailable ()
{
  return (d3dcaps9.AlphaCmpCaps & D3DPCMPCAPS_GREATEREQUAL);
}

/*___________________________________________________________________
|
|	Function: d3d9_EnableAlphaTesting
| 
| Output: Enables/disables alpha testing, if supported.
|___________________________________________________________________*/

void d3d9_EnableAlphaTesting (int flag, byte reference_value)
{
  // Is alpha testing supported?
  if (d3d9_AlphaTestingAvailable ()) {
    // Enable it
    if (flag) {
      // Make sure reference value is in range 0 - 255
      if ((reference_value >= 0) AND (reference_value <= 255)) {
        d3ddevice9->SetRenderState (D3DRS_ALPHAREF, (unsigned long)reference_value);
        d3ddevice9->SetRenderState (D3DRS_ALPHATESTENABLE, TRUE);
        d3ddevice9->SetRenderState (D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
      }
#ifdef _DEBUG
      else
        DEBUG_ERROR ("d3d9_EnableAlphaTesting(): reference value out of range")
#endif
    }
    // Disable it
    else
      d3ddevice9->SetRenderState (D3DRS_ALPHATESTENABLE, FALSE);
  }
}

/*__________________________________________________________________
|
|	Function: d3d9_GetRGBFormat
| 
|	Input: Called from ____
| Output:	Returns rgb format info.
|___________________________________________________________________*/

void d3d9_GetRGBFormat (
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
  *redmask       = d3d9_REDmask;
  *greenmask     = d3d9_GREENmask;
  *bluemask      = d3d9_BLUEmask;
  *low_redbit    = d3d9_loREDbit;
  *low_greenbit  = d3d9_loGREENbit;
  *low_bluebit   = d3d9_loBLUEbit;
  *num_redbits   = d3d9_numREDbits;
  *num_greenbits = d3d9_numGREENbits;
  *num_bluebits  = d3d9_numBLUEbits;
} 

/*__________________________________________________________________
|
|	Function: d3d9_SetActivePage
| 
|	Input: Called from ____
| Output:	Sets the active drawing surface.  If surface is NULL, sets 
|   to backbuffer.
|___________________________________________________________________*/

int d3d9_SetActivePage (byte *surface, int page_is_a_texture)
{
  HRESULT hres;
  int set = FALSE;

  // Set active surface to screen (backbuffer)
  if (surface == 0) {
    hres = d3ddevice9->SetRenderTarget (0, d3dscreen9);
    if (hres == D3D_OK)
      hres = d3ddevice9->SetDepthStencilSurface (d3dzbuffer9);
  }
  // Set active surface to callers surface
  else
    hres = d3ddevice9->SetRenderTarget (0, (LPDIRECT3DSURFACE9)surface);

  if (hres == D3D_OK)
    set = TRUE;
#ifdef _DEBUG
  else
    DEBUG_ERROR ("d3d9_SetActivePage(): ERROR, page not set")
#endif

  return (set);
}

/*__________________________________________________________________
|
|	Function: d3d9_FlipVisualPage
| 
|	Input: Called from ____
| Output:	Sets visual page to next page in flip chain.
|
| Note: May want to look at the return value from this function.  If
|   it returns D3DERR_DEVICELOST, need to do a restore.
|___________________________________________________________________*/

void d3d9_FlipVisualPage ()
{
  d3ddevice9->Present (0, 0, 0, 0);
}
