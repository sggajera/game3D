/*____________________________________________________________________
|
| File: gx_w7.cpp
|
| Description: Graphics library - Windows XP version.
|
| Functions:    gxError
|               gxError_Filename
|               gxGetUserFormat
|               gxStartGraphics
|                Start_DX9_Driver
|               gxStopGraphics
|               gxSaveState
|               gxRestoreState
|               gxGetScreenWidth
|               gxGetScreenHeight
|               gxGetBitDepth
|               gxGetRGBFormat
|               gxGetAspectRatio
|               gxVertRetraceDelay
|               gxRestoreDirectX
|               gxGetNumVRAMPages
|               gxFlipVisualActivePages
|               gxSetActivePage
|               gxGetActivePage
|               gxSetVisualPage
|               gxGetVisualPage
|
|               gxCreateVirtualPage
|               gxFreeVirtualPage
|               gxClearPage
|               gxCopyPage
|                Copy_Virtual_Page
|                 Rectangles_Overlap
|               gxCopyPageColorKey
|
|               gxSetColor
|               gxGetColor
|               gxSetLineWidth
|               gxGetLineWidth
|               gxSetLineStyle
|               gxGetLineStyle
|               gxDefineBitmapPattern
|               gxDefineImagePattern
|               gxFreePattern
|               gxSetFillPattern
|               gxGetFillPattern
|               gxSetLogicOp
|               gxGetLogicOp
|
|               gxSetWindow
|                Clip_Rectangle_To_Page
|               gxGetWindow
|               gxSetClip
|               gxGetClip
|               gxSetClipping
|               gxGetClipping
|               gxClearWindow
|               gxGetMaxX
|               gxGetMaxY
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define _GX_W7_CPP_

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include "dp.h"
#include "virtual.h"
#include "texture.h"

/*___________________
|
| Function Prototypes
|__________________*/

static int Start_DX9_Driver (
  int      num_pages_requested, 
  unsigned stencil_depth_requested, 
  int      enable_hardware_acceleration );

static void Copy_Virtual_Page (
  int srcx,
  int srcy,
  int srcpg,
  int dstx,
  int dsty,
  int dstpg,
  int dx,
  int dy );
static int  Rectangles_Overlap (int x1, int y1, int x2, int y2, int dx, int dy);

static int  Clip_Rectangle_To_Page (int *xleft, int *ytop, int *xright, int *ybottom);

/*___________________
|
| Constants
|__________________*/

#define HARDWARE_RASTERIZER 1
#define SOFTWARE_RASTERIZER 0

/*___________________
|
| Global variables
|__________________*/

static FILE *error_log_file = NULL;

/*____________________________________________________________________
|
| Function: gxError
|
| Input: Called from gx functions with a string describing the error.
| Output: Creates a file and writes: date and time of error, error msg.
|___________________________________________________________________*/

void gxError (char *str)
{
  time_t t;

  BEEP

  // Open output error file, creating it if needed
  if (error_log_file == NULL)
    error_log_file = fopen (ERROR_FILE, "wt");
  // If already opened before, open it for append
  else
    error_log_file = fopen (ERROR_FILE, "at");

  // Write out error msg
  if (error_log_file) {
    t = time (NULL);
    fprintf (error_log_file, "%s ", ctime(&t));
    fprintf (error_log_file, str);
    fprintf (error_log_file, "\n");
    fclose (error_log_file);
  }

  win_Abort_Program (str);
}

/*____________________________________________________________________
|
| Function: gxError_Filename
|
| Input: Called from gx functions with a filename.
| Output: Creates a file and writes: date and time of error, filename.
|___________________________________________________________________*/

void gxError_Filename (char *filename)
{
  int i;
  time_t t;

  BEEP

  // Open output error file, creating it if needed
  if (error_log_file == NULL)
    error_log_file = fopen (ERROR_FILE, "wt");
  // If already opened before, open it for append
  else
    error_log_file = fopen (ERROR_FILE, "at");

  // Write out error msg
  if (error_log_file) {
    t = time (NULL);
    fprintf (error_log_file, "%s ", ctime(&t));
    if (filename) {
      for (i=0; (i<256) AND filename[i]; i++) {
        if (isprint(filename[i]))
          fprintf (error_log_file, "%c", filename[i]);
        else
          fprintf (error_log_file, "?");
      }
    }
    else
      fprintf (error_log_file, "NULLSTRING");
    fprintf (error_log_file, "\n");
    fclose (error_log_file);
  }
}

/*____________________________________________________________________
|
| Function: gxGetUserFormat
|
| Output: Gets a gx video resolution from user.  Returns true if user
|   selected a mode, else false.  If user selected a mode, returns
|   resolution and depth in callers variables.
|___________________________________________________________________*/

int gxGetUserFormat (
  int       driver,             
  unsigned  acceptable_resolution,  // bit mask 
  unsigned  acceptable_bitdepth,    // bit mask
  unsigned *selected_resolution,    // selected resolution, if any
  unsigned *selected_bitdepth )     // selected depth, if any
{
  unsigned mode_selected = FALSE;

  switch (driver) {
#ifdef USING_DIRECTX_9
    case gxDRIVER_DX9: 
                          mode_selected = dx9_GetUserFormat (acceptable_resolution, 
                                                             acceptable_bitdepth,
                                                             selected_resolution,
                                                             selected_bitdepth,
                                                             1);
                          break;
    case gxDRIVER_DX9_SOFTWARE:
                          mode_selected = dx9_GetUserFormat (acceptable_resolution, 
                                                             acceptable_bitdepth,
                                                             selected_resolution,
                                                             selected_bitdepth,
                                                             0);
                          break;
#endif
  }

  return (mode_selected);
}

/*____________________________________________________________________
|
| Function: gxStartGraphics
|
| Output: Initializes graphics mode.  Returns number of pages available
|       or 0 on any error.  (Number of pages available can be different
|       than requested num_pages).
|___________________________________________________________________*/

int gxStartGraphics (
  unsigned resolution, 
  unsigned bitdepth, 
  unsigned stencildepth, 
  int      num_pages, 
  int      driver )
{
  int i, screen_dx, screen_dy;
  gxColor color;

  // Table of info about video modes
  static struct {
    unsigned resolution, width, height;
    float aspect_ratio;
  } Mode_info [] = {
    { gxRESOLUTION_640x480,    640,  480, 1 }, 
    { gxRESOLUTION_800x600,    800,  600, 1 },
    { gxRESOLUTION_1024x768,  1024,  768, 1 },
    { gxRESOLUTION_1152x864,  1152,  864, 1 },
    { gxRESOLUTION_1280x960,  1280,  960, 1 },
    { gxRESOLUTION_1280x1024, 1280, 1024, 1.066f },
    { gxRESOLUTION_1400x1050, 1400, 1050, 1 },
    { gxRESOLUTION_1440x1080, 1440, 1080, 1 },
    { gxRESOLUTION_1600x1200, 1600, 1200, 1 },
    { gxRESOLUTION_1152x720,  1152,  720, 1 },
    { gxRESOLUTION_1280x800,  1280,  800, 1 },
    { gxRESOLUTION_1440x900,  1440,  900, 1 },
    { gxRESOLUTION_1680x1050, 1680, 1050, 1 },
    { gxRESOLUTION_1920x1200, 1920, 1200, 1 }, 
    { gxRESOLUTION_2048x1280, 2048, 1280, 1 },  
    { gxRESOLUTION_1280x720,  1280,  720, 1 }, 
    { gxRESOLUTION_1600x900,  1600,  900, 1 }, 
    { gxRESOLUTION_1920x1080, 1920, 1080, 1 }, 
    { gxRESOLUTION_2048x1152, 2048, 1152, 1 },
    { gxRESOLUTION_2560x1440, 2560, 1440, 1 },
    { gxRESOLUTION_2560x1600, 2560, 1600, 1 },
    { 0,                         0,    0, 0 }
  };
  
/*____________________________________________________________________
|
| Compute info about the selected video mode.
|___________________________________________________________________*/

  memset (&gx_Video,      0, sizeof(gxVideoDriver));
  memset (&gx_Video_save, 0, sizeof(gxVideoDriver));

  // Set driver
  gx_Video.driver = driver;

  // Set mode stuff
  for (i=0; true; i++) // will always find it, should be no error here!
    if (resolution == Mode_info[i].resolution) {
      gx_Video.resolution = resolution;
      gx_Aspect_ratio     = Mode_info[i].aspect_ratio;
      screen_dx           = Mode_info[i].width;
      screen_dy           = Mode_info[i].height;
      break;
    }

  // Set bitdepth
  switch (bitdepth) {
    case gxBITDEPTH_16: gx_Video.bitdepth = bitdepth;
                        gx_Pixel_size     = 2;
                        break;
    case gxBITDEPTH_24: gx_Video.bitdepth = bitdepth;
                        gx_Pixel_size     = 3;
                        break;
    case gxBITDEPTH_32: gx_Video.bitdepth = bitdepth;
                        gx_Pixel_size     = 4;
                        break;

  }

  // Start the driver?
  if (gx_Video.resolution AND gx_Video.bitdepth) {
    switch (driver) {
      case gxDRIVER_DX9:    num_pages = Start_DX9_Driver (num_pages, stencildepth, HARDWARE_RASTERIZER);
                            break;
      case gxDRIVER_DX9_SOFTWARE: 
                            num_pages = Start_DX9_Driver (num_pages, stencildepth, SOFTWARE_RASTERIZER);
                            break;
    }
  }

  // Any errors?
  if (gx_Video.resolution == 0)
    gxError ("GX doesn't support the requested video driver\n");
  else if (num_pages == 0)
    gxError ("Error in GX initializing video mode\n");

/*____________________________________________________________________
|
| Init globals
|___________________________________________________________________*/

  if (num_pages) {
    // Save a copy of video driver info
    gx_Video_save = gx_Video;
    // Init page list
    for (i=0; i<MAX_PAGES; i++) {
      if (i < num_pages) {
        gx_Page_list[i].type        = PAGE_TYPE_SCREEN;
        gx_Page_list[i].width       = screen_dx;
        gx_Page_list[i].height      = screen_dy;
        gx_Page_list[i].buffer      = NULL;
        gx_Page_list[i].driver_page = i;
      }
      else
        gx_Page_list[i].type = 0;
    }

    // Init pattern list
    gx_Pattern_list[0].type = PATTERN_TYPE_SOLID;
    for (i=1; i<MAX_PATTERNS; i++)                      
      gx_Pattern_list[i].type = 0;

    gx_Num_pages      = num_pages;
    gx_Screen.xleft   = 0;
    gx_Screen.ytop    = 0;
    gx_Screen.xright  = screen_dx-1;
    gx_Screen.ybottom = screen_dy-1;
    // Default window/clip is entire screen
    gxSetWindow (&gx_Screen);
    gxSetClip (&gx_Screen);

    // Set some global variables
    gxSetClipping (FALSE);

    // Set default starting visual/active pages
    gx_Active_page = 0;
    gx_Visual_page = 0;
    gxSetVisualPage (0, TRUE);
    gxSetActivePage (0);

    color.index = 0;
    gxSetColor (color);
    gxSetLineWidth (gxLINE_WIDTH_SQUARE_1);
    gx_Line_style_enabled = FALSE;
    for (i=0; i<NUM_STYLE_ELEMENTS; i++)
      gx_Line_style[i] = 0;
    gxSetFillPattern (gxPATTERN_SOLID);
    gxSetLogicOp (gxSET);

    // Init 3D globals
    gx3d_SetFillMode (gx3d_FILL_MODE_GOURAUD_SHADED);
    gx3d_Texture_Directory[0] = 0;

    // Init support routines
    Texture_Init ();

    // Init random number generator
    random_Init ();
  }
  
  return (num_pages);
}

/*____________________________________________________________________
|                                                                              
| Function: Start_DX9_Driver
|
| Output: Initializes DirectX 9 driver.  Returns # pages available.
|___________________________________________________________________*/

static int Start_DX9_Driver (
  int      num_pages_requested, 
  unsigned stencil_depth_requested, 
  int      enable_hardware_acceleration )
{
  int num_pages = 0;

#ifdef USING_DIRECTX_9
  // Initialize driver
  num_pages = dx9_Init (gx_Video.resolution, gx_Video.bitdepth, stencil_depth_requested, num_pages_requested, enable_hardware_acceleration);

  // Set functions this driver has available 
  if (num_pages) {
    gx_Video.free_driver                  = dx9_Free;
    gx_Video.vert_retrace_delay           = dx9_VertRetraceDelay;
    gx_Video.restore_directx              = dx9_RestoreDirectX;
    gx_Video.create_virtual_page          = dx9_CreateVirtualPage;
    gx_Video.free_virtual_page            = dx9_FreeVirtualPage;
    gx_Video.set_active_page              = dx9_SetActivePage;
    gx_Video.set_visual_page              = NULL;
    gx_Video.flip_visual_page             = dx9_FlipVisualPage;
    gx_Video.set_fore_color               = dx9_SetForeColor;
    gx_Video.set_logic_op                 = dx9_SetLogicOp;
    gx_Video.draw_pixel                   = dx9_DrawPixel;
    gx_Video.get_pixel                    = dx9_GetPixel;
    gx_Video.draw_line                    = dx9_DrawLine;
    gx_Video.draw_fill_rectangle          = dx9_DrawFillRectangle;
    gx_Video.draw_fill_poly               = NULL;
    gx_Video.put_image                    = dx9_PutImage;
    gx_Video.get_image                    = dx9_GetImage;
    gx_Video.copy_image                   = dx9_CopyImage;
    gx_Video.copy_image_colorkey          = dx9_CopyImageColorKey;
    gx_Video.put_bitmap                   = dx9_PutBitmap;
    // Init 3D functions
    gx_Video.begin_render                 = dx9_BeginRender;
    gx_Video.end_render                   = dx9_EndRender;
    gx_Video.set_fill_mode                = dx9_SetFillMode;
    gx_Video.get_driver_info              = dx9_GetDriverInfo;
    gx_Video.register_object              = dx9_RegisterObject;
    gx_Video.unregister_object            = dx9_UnregisterObject;
    gx_Video.draw_object                  = dx9_DrawObject;
    gx_Video.optimize_object              = dx9_OptimizeObject;
    gx_Video.set_viewport                 = dx9_SetViewport;
    gx_Video.clear_viewport_rectangle     = dx9_ClearViewportRectangle;
    gx_Video.enable_clipping              = dx9_EnableClipping;
    gx_Video.init_clip_plane              = dx9_InitClipPlane;
    gx_Video.free_clip_plane              = dx9_FreeClipPlane;
    gx_Video.enable_clip_plane            = dx9_EnableClipPlane;
    gx_Video.set_world_matrix             = dx9_SetWorldMatrix;
    gx_Video.get_world_matrix             = dx9_GetWorldMatrix;
    gx_Video.set_view_matrix              = dx9_SetViewMatrix;
    gx_Video.get_view_matrix              = dx9_GetViewMatrix;
    gx_Video.set_projection_matrix        = dx9_SetProjectionMatrix;
    gx_Video.get_projection_matrix        = dx9_GetProjectionMatrix;
    gx_Video.enable_texture_matrix        = dx9_EnableTextureMatrix;
    gx_Video.set_texture_matrix           = dx9_SetTextureMatrix;
    gx_Video.get_texture_matrix           = dx9_GetTextureMatrix;
    gx_Video.enable_zbuffer               = dx9_EnableZBuffer;
    gx_Video.enable_backface_removal      = dx9_EnableBackfaceRemoval;
    gx_Video.enable_stencil_buffer        = dx9_EnableStencilBuffer;
    gx_Video.set_stencil_fail_op          = dx9_SetStencilFailOp;
    gx_Video.set_stencil_zfail_op         = dx9_SetStencilZFailOp;
    gx_Video.set_stencil_pass_op          = dx9_SetStencilPassOp;
    gx_Video.set_stencil_comparison       = dx9_SetStencilComparison;
    gx_Video.set_stencil_reference_value  = dx9_SetStencilReferenceValue;
    gx_Video.set_stencil_mask             = dx9_SetStencilMask;
    gx_Video.set_stencil_write_mask       = dx9_SetStencilWriteMask;
    gx_Video.enable_lighting              = dx9_EnableLighting;
    gx_Video.init_point_light             = dx9_InitPointLight;
    gx_Video.update_point_light           = dx9_UpdatePointLight;
    gx_Video.init_spot_light              = dx9_InitSpotLight;
    gx_Video.update_spot_light            = dx9_UpdateSpotLight;
    gx_Video.init_direction_light         = dx9_InitDirectionLight;
    gx_Video.update_direction_light       = dx9_UpdateDirectionLight;
    gx_Video.free_light                   = dx9_FreeLight;
    gx_Video.enable_light                 = dx9_EnableLight;
    gx_Video.set_ambient_light            = dx9_SetAmbientLight;
    gx_Video.enable_specular_lighting     = dx9_EnableSpecularLighting;
    gx_Video.set_material                 = dx9_SetMaterial;
    gx_Video.get_material                 = dx9_GetMaterial;
    gx_Video.init_texture                 = dx9_InitTexture;
    gx_Video.init_volume_texture          = dx9_InitVolumeTexture;
    gx_Video.init_cubemap_texture         = dx9_InitCubemapTexture;
    gx_Video.init_dynamic_texture         = dx9_InitDynamicTexture;
    gx_Video.init_dynamic_cubemap_texture = dx9_InitDynamicCubemapTexture;
    gx_Video.free_texture                 = dx9_FreeTexture;
    gx_Video.free_dynamic_texture         = dx9_FreeDynamicTexture;
    gx_Video.set_texture                  = dx9_SetTexture;
    gx_Video.set_dynamic_texture          = dx9_SetDynamicTexture;
    gx_Video.set_texture_addressing_mode  = dx9_SetTextureAddressingMode;
    gx_Video.set_texture_border_color     = dx9_SetTextureBorderColor;
    gx_Video.set_texture_filtering        = dx9_SetTextureFiltering;
    gx_Video.set_texture_coordinates      = dx9_SetTextureCoordinates;
    gx_Video.enable_cubemap_texture_reflections = dx9_EnableCubemapTextureReflections;
    gx_Video.set_texture_wrapping         = dx9_SetTextureCoordinateWrapping;
    gx_Video.set_texture_factor           = dx9_SetTextureFactor;
    gx_Video.preload_texture              = dx9_PreLoadTexture;
    gx_Video.evict_all_textures           = dx9_EvictAllTextures;
    gx_Video.enable_render_to_texture     = dx9_EnableRenderToTexture;
    gx_Video.enable_antialiasing          = NULL;
    gx_Video.set_texture_colorop          = dx9_SetTextureColorOp;
    gx_Video.set_texture_alphaop          = dx9_SetTextureAlphaOp;
    gx_Video.set_texture_color_factor     = dx9_SetTextureColorFactor;
    gx_Video.enable_vertex_lighting       = dx9_EnableVertexLighting;
    gx_Video.enable_fog                   = dx9_EnableFog;
    gx_Video.set_fog_color                = dx9_SetFogColor;
    gx_Video.set_linear_pixel_fog         = dx9_SetLinearPixelFog;
    gx_Video.set_exp_pixel_fog            = dx9_SetExpPixelFog;
    gx_Video.set_exp2_pixel_fog           = dx9_SetExp2PixelFog;
    gx_Video.set_linear_vertex_fog        = dx9_SetLinearVertexFog;
    gx_Video.enable_alpha_blending        = dx9_EnableAlphaBlending;
    gx_Video.set_alpha_blend_factor       = dx9_SetAlphaBlendFactor;
    gx_Video.alpha_testing_available      = dx9_AlphaTestingAvailable;
    gx_Video.enable_alpha_testing         = dx9_EnableAlphaTesting;
    // Save drivers RGB format
    dx9_GetRGBFormat (&(gx_Video.redmask),     &(gx_Video.greenmask),     &(gx_Video.bluemask),
                      &(gx_Video.low_redbit),  &(gx_Video.low_greenbit),  &(gx_Video.low_bluebit),
                      &(gx_Video.num_redbits), &(gx_Video.num_greenbits), &(gx_Video.num_bluebits));
  }                                  
#endif

  return (num_pages);
}

/*____________________________________________________________________
|
| Function: gxStopGraphics
|
| Output: Closes graphics mode.  This routine should not be called if
|       gxStartGraphics() was not successful.
|___________________________________________________________________*/

void gxStopGraphics (void)
{
  int i;

	// Free any loaded 3d objects
	gx3d_FreeAllObjects ();
  // Free all loaded motions
  gx3d_Motion_Free_All ();
  // Free all loaded skeletons
  gx3d_MotionSkeleton_Free_All ();
	
	// Free support routines - frees all textures
  Texture_Free ();
  
  // Set active page to first screen page
  gxSetActivePage (0);

  // Delete any virtual pages
  for (i=0; i<MAX_PAGES; i++) {
    if (gx_Page_list[i].type == PAGE_TYPE_DRIVER_VIRTUAL) 
      (*gx_Video.free_virtual_page) (i);
    else if (gx_Page_list[i].type == PAGE_TYPE_VIRTUAL) 
      free (gx_Page_list[i].buffer);
    gx_Page_list[i].type = 0;
  }

  // Delete any patterns
  for (i=1; i<MAX_PATTERNS; i++)
    if (gx_Pattern_list[i].type) {
      // Free this pattern
      free (gx_Pattern_list[i].data);
      gx_Pattern_list[i].type = 0;
    }

  // Stop driver processing
  if (gx_Video.free_driver)
    (*gx_Video.free_driver) ();
}

/*____________________________________________________________________
|
| Function: gxSaveState
|
| Output: Saves state of graphics in a user buffer.
|___________________________________________________________________*/

void gxSaveState (gxState *state)
{
  int i;

  state->active_page  = gx_Active_page;
  state->win          = gx_Window;
  state->clip         = gx_Clip;
  state->clipping     = gx_Clipping;
  state->color        = gx_Fore_color;
  state->line_width   = gx_Line_width;
  state->fill_pattern = gx_Fill_pattern;
  state->logic_op     = gx_Logic_op;
  for (i=0; i<NUM_STYLE_ELEMENTS; i++)
    state->line_style[i] = gx_Line_style[i];
  state->font         = gxGetFont ();
}

/*____________________________________________________________________
|
| Function: gxRestoreState
|
| Output: Restores graphics state from a user buffer.  Note the current
|       visual page is not changed.
|___________________________________________________________________*/

void gxRestoreState (gxState *state)
{
  gx_Window       = state->win;
  gx_Clip         = state->clip;
  gx_Clipping     = state->clipping;
  gx_Line_width   = state->line_width;
  gx_Fill_pattern = state->fill_pattern;

  gxSetActivePage (state->active_page);
  gxSetColor      (state->color);
  gxSetLogicOp    (state->logic_op);
  gxSetLineStyle  (state->line_style[0], state->line_style[1],
                   state->line_style[2], state->line_style[3]);
  gxSetFont       (state->font);
}

/*____________________________________________________________________
|
| Function: gxGetScreenWidth
|
| Output: Returns width in pixels of screen.
|___________________________________________________________________*/

inline int gxGetScreenWidth (void)
{
  return (gx_Screen.xright + 1);
}

/*____________________________________________________________________
|
| Function: gxGetScreenHeight
|
| Output: Returns height in pixels of screen.
|___________________________________________________________________*/

inline int gxGetScreenHeight (void)
{
  return (gx_Screen.ybottom + 1);
}

/*____________________________________________________________________
|
| Function: gxGetBitDepth
|
| Output: Returns bit depth (a constant) of screen.
|___________________________________________________________________*/

inline int gxGetBitDepth (void)
{
  return (gx_Video.bitdepth);
}

/*____________________________________________________________________
|
| Function: gxGetRGBFormat
|
| Output: Returns RGB format of screen.
|___________________________________________________________________*/

void gxGetRGBFormat (
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
  *redmask       = gx_Video.redmask;
  *greenmask     = gx_Video.greenmask;
  *bluemask      = gx_Video.bluemask;
  *low_redbit    = gx_Video.low_redbit;
  *low_greenbit  = gx_Video.low_greenbit;
  *low_bluebit   = gx_Video.low_bluebit;
  *num_redbits   = gx_Video.num_redbits;
  *num_greenbits = gx_Video.num_greenbits;
  *num_bluebits  = gx_Video.num_bluebits;
}

/*____________________________________________________________________
|
| Function: gxGetAspectRatio
|
| Output: Returns aspect ratio.  Value is the width of a pixel as
|       compared to the (height = 1).
|___________________________________________________________________*/

inline float gxGetAspectRatio (void)
{
  return (gx_Aspect_ratio);
}

/*____________________________________________________________________
|
| Function: gxVertRetraceDelay
|
| Output: Waits for start of next vertical retrace period.
|___________________________________________________________________*/

inline void gxVertRetraceDelay (void)
{
  if (gx_Video.vert_retrace_delay)
    (*gx_Video.vert_retrace_delay) ();
}

/*____________________________________________________________________
|
| Function: gxRestoreDirectX
|
| Output: Restores lost buffers, input devices (DirectX only).
|___________________________________________________________________*/

int gxRestoreDirectX (void)
{
  int restored = FALSE;

  if (gx_Video.restore_directx) {
    restored = (*gx_Video.restore_directx) ();
    if (restored) {
      // Reset to default starting visual/active pages
      gxSetVisualPage (0, TRUE);
      gxSetActivePage (0);
      // Restore texture manager
      Texture_Restore ();
    }
  }

  return (restored);
}

/*____________________________________________________________________
|
| Function: gxGetNumVRAMPages
|
| Output: Returns the number of vram pages available.
|___________________________________________________________________*/

inline int gxGetNumVRAMPages (void)
{
  return (gx_Num_pages);
}

/*____________________________________________________________________
|
| Function: gxFlipVisualActivePages
|
| Output: Sets visual page to next vram page.  Also sets active page
|   to first backbuffer page (page following new visual page).
|___________________________________________________________________*/

void gxFlipVisualActivePages (int wait_for_vsync)
{
  // Is there more than 1 vram page?
  if (gx_Num_pages > 1) {
    // Is flip function defined?
    if (gx_Video.flip_visual_page)
      (*gx_Video.flip_visual_page) ();
    // If not, manually flip visual/active pages
    else {
      gxSetVisualPage ((gxGetVisualPage () + 1) % gx_Num_pages, wait_for_vsync);
      gxSetActivePage ((gxGetVisualPage () + 1) % gx_Num_pages);
    }
  }
}

/*____________________________________________________________________
|
| Function: gxSetActivePage
|
| Output: Sets page for drawing operations.
|___________________________________________________________________*/

void gxSetActivePage (gxPage page)
{
  int set = FALSE;

  // Is this a valid page number?
  if ((page >= 0) AND (page < MAX_PAGES)) {
    // Is this a valid page?
    if (gx_Page_list[page].type) {
      // Is this a screen page or a driver-managed virtual page?
      if ((gx_Page_list[page].type == PAGE_TYPE_SCREEN) OR 
          (gx_Page_list[page].type == PAGE_TYPE_DRIVER_VIRTUAL)) {
        // Set the new active page
        set = (*gx_Video.set_active_page) (gx_Page_list[page].driver_page);
        if (set)
          // Was last page a virtual page?
          if (gx_Page_list[gx_Active_page].type == PAGE_TYPE_VIRTUAL)
            // Restore all original screen driver functions
            gx_Video = gx_Video_save;
      }
      // Must be a virtual page!
      else {
        virtual_Init (page);
        // Enable functions specific to virtual driver
        gx_Video.draw_pixel          = virtual_DrawPixel;
        gx_Video.get_pixel           = virtual_GetPixel;
        gx_Video.draw_line           = virtual_DrawLine;
        gx_Video.draw_fill_rectangle = virtual_DrawFillRectangle;
        gx_Video.draw_fill_poly      = NULL;
        gx_Video.put_image           = virtual_PutImage;
        gx_Video.get_image           = virtual_GetImage;
        gx_Video.put_bitmap          = virtual_PutBitmap;
        set = TRUE;
      }

      // Set new page in a global variable
      if (set)
        gx_Active_page = page;
    }
  }

  // Make sure page was set
  DEBUG_ASSERT (set);
}

/*____________________________________________________________________
|
| Function: gxGetActivePage
|
| Output: Returns current drawing page.
|___________________________________________________________________*/

inline gxPage gxGetActivePage (void)
{
  return (gx_Active_page);
}

/*____________________________________________________________________
|
| Function: gxSetVisualPage
|
| Output: Sets visual page.
|___________________________________________________________________*/

void gxSetVisualPage (gxPage page, int wait_for_vsync)
{
  int set = FALSE;

  // Is this a valid page number?
  if ((page >= 0) AND (page < MAX_PAGES)) 
    // Is this a screen page
    if (gx_Page_list[page].type == PAGE_TYPE_SCREEN) 
      // Set the new visual page?
      if (gx_Video.set_visual_page) {
        if ((*gx_Video.set_visual_page) (gx_Page_list[page].driver_page, wait_for_vsync)) {
          gx_Visual_page = page;
          set = TRUE;
        }
      }
      else {
        gx_Visual_page = page;
        set = TRUE;
      }

  // Make sure page was set
  DEBUG_ASSERT (set);
}

/*____________________________________________________________________
|
| Function: gxGetVisualPage
|
| Output: Returns visual page.
|___________________________________________________________________*/

inline gxPage gxGetVisualPage (void)
{
  return (gx_Visual_page);
}

/*____________________________________________________________________
|
| Function: gxCreateVirtualPage
|
| Output: Creates a virtual page in memory.  Returns true on success
|   else false on any error.  New page number returned in callers variable.
|___________________________________________________________________*/

int gxCreateVirtualPage (int width, int height, unsigned hints, gxPage *page)
{
  int i, create_in_vram, driver_page;
  int created = FALSE;

  // Try to create in vram?
  if (hints & gxHINT_CREATE_IN_SYSTEM_MEMORY)
    create_in_vram = FALSE;
  else
    create_in_vram = TRUE;
  
  // Look for an empty entry in page list 
  for (i=0; gx_Page_list[i].type AND (i<MAX_PAGES); i++);

  // Is their an entry available? 
  if (i < MAX_PAGES) {
    // Allow driver to create the virtual page? 
		if (NOT (hints & gxHINT_DONT_LET_DRIVER_MANAGE))
			if (gx_Video.create_virtual_page) {
				driver_page = (*gx_Video.create_virtual_page) (width, height, create_in_vram);
				if (driver_page != -1) {
					gx_Page_list[i].type        = PAGE_TYPE_DRIVER_VIRTUAL;
					gx_Page_list[i].width       = width;
					gx_Page_list[i].height      = height;
					gx_Page_list[i].buffer      = NULL;
					gx_Page_list[i].driver_page = driver_page;
					*page = i;
					created = TRUE;
				}
			}
    // Create the virtual page manually if not created by the driver 
    if (NOT created) {
      gx_Page_list[i].buffer = (byte *) calloc (width, height * gx_Pixel_size);
      if (gx_Page_list[i].buffer) {
        gx_Page_list[i].type   = PAGE_TYPE_VIRTUAL;
        gx_Page_list[i].width  = width;
        gx_Page_list[i].height = height;
        *page = i;
        created = TRUE;
      }
    }
  }

  return (created);
}

/*____________________________________________________________________
|
| Function: gxFreeVirtualPage
|
| Output: Frees memory associated with a virtual page.  Page cannot
|       be the currently active page.
|___________________________________________________________________*/

void gxFreeVirtualPage (gxPage page)
{
  // If freeing active page, set active page to 0 (first screen page)
  if (page == gx_Active_page)
    gxSetActivePage (0);
  
  // Is this a valid page number
  if ((page >= 0) AND (page < MAX_PAGES)) {
    // Is this a driver-managed virtual page?
    if (gx_Page_list[page].type == PAGE_TYPE_DRIVER_VIRTUAL) {
      (*gx_Video.free_virtual_page) (gx_Page_list[page].driver_page);
      gx_Page_list[page].type = 0;
    }
    // Is this a virtual page?
    else if (gx_Page_list[page].type == PAGE_TYPE_VIRTUAL) {
      // Free this page
      free (gx_Page_list[page].buffer);
      gx_Page_list[page].buffer = NULL;
      gx_Page_list[page].type   = 0;
    }
  }
}

/*____________________________________________________________________
|
| Function: gxGetPageWidth
|
| Output: Returns the width of a page or 0 on any error.
|___________________________________________________________________*/

int gxGetPageWidth (gxPage page)
{
  int width = 0;

  // Is this a valid page number?
  if ((page >= 0) AND (page < MAX_PAGES)) 
    // Is this a valid page?
    if (gx_Page_list[page].type)
      width = gx_Page_list[page].width;

  return (width);
}

/*____________________________________________________________________
|
| Function: gxGetPageHeight
|
| Output: Returns the height of a page or 0 on any error.
|___________________________________________________________________*/

int gxGetPageHeight (gxPage page)
{
  int height = 0;

  // Is this a valid page number?
  if ((page >= 0) AND (page < MAX_PAGES)) 
    // Is this a valid page?
    if (gx_Page_list[page].type)
      height = gx_Page_list[page].height;

  return (height);
}

/*____________________________________________________________________
|
| Function: gxClearPage
|
| Output: Clears a page to a color.
|___________________________________________________________________*/

void gxClearPage (gxPage page, gxColor color)
{
  gxRectangle rect;
  gxState state;

  gxSaveState (&state);
  gxSetActivePage (page);
  gxSetLogicOp (gxSET);
  gxSetColor (color);
  rect.xleft   = 0;
  rect.ytop    = 0;
  rect.xright  = gx_Page_list[page].width - 1;
  rect.ybottom = gx_Page_list[page].height - 1;
  gxSetWindow (&rect);
  gxSetClipping (FALSE);
  gxDrawFillRectangle (0, 0, rect.xright, rect.ybottom);
  gxRestoreState (&state);
}

/*____________________________________________________________________
|
| Function: gxCopyPage
|
| Output: Copies a rectangle from one page to another.
|___________________________________________________________________*/

void gxCopyPage (
  int      srcx,
  int      srcy,
  gxPage   srcpg,
  int      dstx,
  int      dsty,
  gxPage   dstpg,
  int      dx,
  int      dy ) 
//  gxColor *filter_color ) // this color will be transparent 
{
  // Are both valid page numbers?
  if ((srcpg < MAX_PAGES) AND (dstpg < MAX_PAGES))
    // Are both valid pages?
    if (gx_Page_list[srcpg].type AND gx_Page_list[dstpg].type) {
      // Are both driver-managed pages (screen and/or driver virtual)?
      if ((gx_Page_list[srcpg].type <= PAGE_TYPE_DRIVER_VIRTUAL) AND
          (gx_Page_list[dstpg].type <= PAGE_TYPE_DRIVER_VIRTUAL))
        (*gx_Video.copy_image)(srcx, srcy, gx_Page_list[srcpg].driver_page, dstx, dsty, gx_Page_list[dstpg].driver_page, dx, dy);
      // Are both virtual pages?
      else if ((gx_Page_list[srcpg].type == PAGE_TYPE_VIRTUAL) AND
               (gx_Page_list[dstpg].type == PAGE_TYPE_VIRTUAL))
        Copy_Virtual_Page (srcx, srcy, gx_Page_list[srcpg].driver_page, dstx, dsty, gx_Page_list[dstpg].driver_page, dx, dy);
      // Screen -> virtual page?
      else if ((gx_Page_list[srcpg].type == PAGE_TYPE_SCREEN) AND
               (gx_Page_list[dstpg].type == PAGE_TYPE_VIRTUAL)) {
        (*gx_Video.set_active_page) (gx_Page_list[srcpg].driver_page);
        (*gx_Video_save.get_image)(gx_Page_list[dstpg].buffer,
                                   gx_Page_list[dstpg].width, gx_Page_list[dstpg].height,
                                   dstx, dsty, srcx, srcy, dx, dy);
        (*gx_Video.set_active_page) (gx_Page_list[gx_Active_page].driver_page);
      }
      // Virtual -> screen page?
      else {
        (*gx_Video.set_active_page) (gx_Page_list[dstpg].driver_page);
        (*gx_Video_save.put_image)(gx_Page_list[srcpg].buffer,
                                   gx_Page_list[dstpg].width, gx_Page_list[dstpg].height,
                                   srcx, srcy, dstx, dsty, dx, dy, 0);
        (*gx_Video.set_active_page) (gx_Page_list[gx_Active_page].driver_page);
      }
    }
}

/*____________________________________________________________________
|
| Function: gxCopyPageColorKey
|
| Output: Copies a rectangle from one page to another.  Any pixels in
|     the source rectangle that are the same as color will not be copied,
|     leaving these areas of the destination transparent.  Returns true
|     on success, else false on any error.
|
|     This function only works for two VRAM pages.
|
|     This function currently only works for RGB color modes (not indexed 
|     color mode) that support color keying.
|___________________________________________________________________*/

int gxCopyPageColorKey (
  int     srcx, 
  int     srcy, 
  gxPage  srcpg, 
  int     dstx, 
  int     dsty, 
  gxPage  dstpg, 
  int     dx, 
  int     dy, 
  gxColor color )
{
  int copied = FALSE;

  // Are both valid page numbers?
  if ((srcpg < MAX_PAGES) AND (dstpg < MAX_PAGES))
    // Are both valid pages?
    if (gx_Page_list[srcpg].type AND gx_Page_list[dstpg].type)
      // Are both driver-managed pages (screen and/or driver virtual)?
      if ((gx_Page_list[srcpg].type <= PAGE_TYPE_DRIVER_VIRTUAL) AND
          (gx_Page_list[dstpg].type <= PAGE_TYPE_DRIVER_VIRTUAL)) {
        // Copy in true color mode
        if (gx_Video.copy_image_colorkey) {
          (*gx_Video.copy_image_colorkey)(srcx, srcy, gx_Page_list[srcpg].driver_page, 
                                          dstx, dsty, gx_Page_list[dstpg].driver_page, 
                                          dx, dy, color.r, color.g, color.b);
          copied = TRUE;
        }
      }
      
  return (copied);
}

/*____________________________________________________________________
|
| Function: Copy_Virtual_Page
|
| Input: Called from gxCopyPage()
| Output: Copies a rectangle from one virtual page to another or on
|       the same virtual page.
|___________________________________________________________________*/

static void Copy_Virtual_Page (
  int srcx,
  int srcy,
  int srcpg,
  int dstx,
  int dsty,
  int dstpg,
  int dx,
  int dy )
{
  int x, y, overlap, src_width, dst_width;
  byte *src, *dst;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  src_width = gx_Page_list[srcpg].width;
  dst_width = gx_Page_list[dstpg].width;

  // Get start of source, destination rectangles
  src = gx_Page_list[srcpg].buffer + (srcy * gx_Page_list[srcpg].width * gx_Pixel_size) + (srcx + gx_Pixel_size);
  dst = gx_Page_list[dstpg].buffer + (dsty * gx_Page_list[dstpg].width * gx_Pixel_size) + (dstx + gx_Pixel_size);

  // Are rectangles overlapping?
  if (srcpg == dstpg)
    overlap = Rectangles_Overlap (srcx, srcy, dstx, dsty, dx, dy);
  else
    overlap = FALSE;

/*____________________________________________________________________
|
| Copy (no overlap)
|___________________________________________________________________*/

  if (NOT overlap) {
    for (y=0; y<dy; y++) {
      memcpy (dst, src, dx * gx_Pixel_size);
      src += (src_width * gx_Pixel_size);
      dst += (dst_width * gx_Pixel_size);
    }
  }

/*____________________________________________________________________
|
| Copy (overlap)
|___________________________________________________________________*/

  else {
    // Pick special case of overlapping rectangles
    if ((srcx >= dstx) AND (srcy >= dsty)) {
      for (y=0; y<dy; y++) {
        memcpy (dst, src, dx * gx_Pixel_size);
        src += (src_width * gx_Pixel_size);
        dst += (dst_width * gx_Pixel_size);
      }
    }
    else if ((srcx <= dstx) AND (srcy <= dsty)) {
      src += ((dy-1) * src_width * gx_Pixel_size + (dx-1) * gx_Pixel_size);
      dst += ((dy-1) * dst_width * gx_Pixel_size + (dx-1) * gx_Pixel_size);
      for (y=0; y<dy; y++) {
        for (x=dx-1; x>=0; x--)
          memcpy (&dst[x], &src[x], gx_Pixel_size);
        src -= (src_width * gx_Pixel_size);
        dst -= (dst_width * gx_Pixel_size);
      }
    }
    else if ((srcx <= dstx) AND (srcy >= dsty)) {
      src += ((dx-1) * gx_Pixel_size);
      dst += ((dx-1) * gx_Pixel_size);
      for (y=0; y<dy; y++) {
        for (x=dx-1; x>=0; x--)
          memcpy (&dst[x], &src[x], gx_Pixel_size);
        src += (src_width * gx_Pixel_size);
        dst += (dst_width * gx_Pixel_size);
      }
    }
    else { // ((srcx >= dstx) AND (srcy <= dsty))
      src += ((dy-1) * src_width * gx_Pixel_size);
      dst += ((dy-1) * dst_width * gx_Pixel_size);
      for (y=0; y<dy; y++) {
        memcpy (dst, src, dx * gx_Pixel_size);
        src -= (src_width * gx_Pixel_size);
        dst -= (dst_width * gx_Pixel_size);
      }
    }
  }
}

/*____________________________________________________________________
|
| Function: Rectangles_Overlap
|
| Input: Called from Copy_Virtual_Page()
| Output: Returns true if rectangles overlap.  Rectangles have the same
|       width and height.
|___________________________________________________________________*/

static int Rectangles_Overlap (int x1, int y1, int x2, int y2, int dx, int dy)
{
  int x1right, y1bottom, x2right, y2bottom;
  int overlap = TRUE;

  x1right  = x1 + dx - 1;
  y1bottom = y1 + dy - 1;
  x2right  = x2 + dx - 1;
  y2bottom = y2 + dy - 1;

  if ((x2 > x1right) OR (x2right < x1) OR (y2 > y1bottom) OR (y2bottom < y1))
    overlap = FALSE;

  return (overlap);
}

/*____________________________________________________________________
|
| Function: gxSetColor
|
| Output: Sets color for drawing.
|___________________________________________________________________*/

inline void gxSetColor (gxColor color)
{
  (*gx_Video.set_fore_color) (color.r, color.g, color.b, color.a);

  gx_Fore_color = color;
}

/*____________________________________________________________________
|
| Function: gxGetColor
|
| Output: Returns current color.
|___________________________________________________________________*/

inline gxColor gxGetColor (void)
{
  return (gx_Fore_color);
}

/*____________________________________________________________________
|
| Function: gxSetLineWidth
|
| Output: Sets line width for line drawing.
|___________________________________________________________________*/

inline void gxSetLineWidth (int width)
{
  gx_Line_width = width;
}

/*____________________________________________________________________
|
| Function: gxGetLineWidth
|
| Output: Returns current line width.
|___________________________________________________________________*/

inline int gxGetLineWidth (void)
{
  return (gx_Line_width);
}

/*____________________________________________________________________
|
| Function: gxSetLineStyle
|
| Output: Sets line style for line drawing.
|___________________________________________________________________*/

void gxSetLineStyle (int seg1, int gap1, int seg2, int gap2)
{
  if ((seg1 == 0) AND (gap1 == 0) AND (seg2 == 0) AND (gap2 == 0))
    gx_Line_style_enabled = FALSE;
  else
    gx_Line_style_enabled = TRUE;
  gx_Line_style[0] = seg1;
  gx_Line_style[1] = gap1;
  gx_Line_style[2] = seg2;
  gx_Line_style[3] = gap2;
  gx_Line_style_index = 0;
  gx_Line_style_count = 0;
}

/*____________________________________________________________________
|
| Function: gxGetLineStyle
|
| Output: Returns line style for line drawing.
|___________________________________________________________________*/

void gxGetLineStyle (int *seg1, int *gap1, int *seg2, int *gap2)
{
  if (gx_Line_style_enabled) {
    *seg1 = gx_Line_style[0];
    *gap1 = gx_Line_style[1];
    *seg2 = gx_Line_style[2];
    *gap2 = gx_Line_style[3];
  }
  else {
    *seg1 = 0;
    *gap1 = 0;
    *seg2 = 0;
    *gap2 = 0;
  }
}

/*____________________________________________________________________
|
| Function: gxDefineBitmapPattern
|
| Output: Creates a bitmap fill pattern.  Returns pattern number or -1
|       on any error.  To enable this pattern for drawing, call
|       gxSetFillPattern().
|
| Description: The bitmap buffer is the same type as created by
|       gxCreateBitmap().  The first 4 bytes contain the width in
|       pixels of the pattern.  The next 4 bytes contain the height in
|       pixels of the pattern.  The rest of the buffer contain the
|       pattern.  The size of this is (width+7)/8 x height bytes.
|___________________________________________________________________*/

gxPattern gxDefineBitmapPattern (byte *bitmap, gxColor fore_color, gxColor back_color, int transparent_background)
{
  int i, dx, dy, bytes_per_row;
  int *p;
  gxPattern pattern = -1;

  // Look for an empty entry in pattern list
  for (i=0; gx_Pattern_list[i].type AND (i<MAX_PATTERNS); i++);

  // Is their an entry available?
  if (i < MAX_PATTERNS) {
    // Create the pattern
    p = (int *)bitmap;
    dx = *p;
    dy = *(p+1);
    bytes_per_row = (dx+7)/8;
    gx_Pattern_list[i].data = (byte *) malloc (bytes_per_row * dy);
    if (gx_Pattern_list[i].data != NULL) {
      memcpy (gx_Pattern_list[i].data, bitmap+(2*sizeof(int)), bytes_per_row * dy);
      gx_Pattern_list[i].type = PATTERN_TYPE_BITMAP;
      gx_Pattern_list[i].dx = dx;
      gx_Pattern_list[i].dy = dy;
      gx_Pattern_list[i].bytes_per_row = bytes_per_row;
      gx_Pattern_list[i].fore_color = fore_color;
      gx_Pattern_list[i].back_color = back_color;
      gx_Pattern_list[i].transparent_background = transparent_background;
      pattern = i;
    }
  }

  return (pattern);
}

/*____________________________________________________________________
|
| Function: gxDefineImagePattern
|
| Output: Creates an image fill pattern.  Returns pattern number or -1
|       on any error.  To enable this pattern for drawing, call
|       gxSetFillPattern().
|
| Description: The image buffer is the same type as created by
|       gxCreateImage().  The first 4 bytes contain the width in
|       pixels of the pattern.  The next 4 bytes contain the height in
|       pixels of the pattern.  The rest of the buffer contain the
|       pattern.  The size of this is width x height bytes.
|___________________________________________________________________*/

gxPattern gxDefineImagePattern (byte *image)
{
  int i, dx, dy;
  int *p;
  gxPattern pattern = -1;

  // Look for an empty entry in pattern list
  for (i=0; gx_Pattern_list[i].type AND (i<MAX_PATTERNS); i++);

  // Is their an entry available?
  if (i < MAX_PATTERNS) {
    // Create the pattern
    p = (int *)image;
    dx = *p;
    dy = *(p+1);
    gx_Pattern_list[i].data = (byte *) malloc (dx * dy * gx_Pixel_size);
    if (gx_Pattern_list[i].data != NULL) {
      memcpy (gx_Pattern_list[i].data, image+(2*sizeof(int)), dx * dy * gx_Pixel_size);
      gx_Pattern_list[i].type = PATTERN_TYPE_IMAGE;
      gx_Pattern_list[i].dx = dx;
      gx_Pattern_list[i].dy = dy;
      gx_Pattern_list[i].bytes_per_row = dx * gx_Pixel_size;
      pattern = i;
    }
  }

  return (pattern);
}

/*____________________________________________________________________
|
| Function: gxFreePattern
|
| Output: Frees memory associated with a pattern.
|___________________________________________________________________*/

void gxFreePattern (gxPattern pattern)
{
  // Is this a valid pattern number?
  if ((pattern > gxPATTERN_SOLID ) AND (pattern < MAX_PATTERNS))
    // Is this a valid pattern?
    if (gx_Pattern_list[pattern].type) {
      // Free this pattern
      free (gx_Pattern_list[pattern].data);
      gx_Pattern_list[pattern].type = 0;
      // If this is the current pattern, make the current pattern solid
      if (pattern == gx_Fill_pattern)
        gxSetFillPattern (gxPATTERN_SOLID);
    }
}

/*____________________________________________________________________
|
| Function: gxSetFillPattern
|
| Output: Sets current fill pattern.  If pattern is set to 1 this is
|       the predefined SOLID pattern.  If pattern is 2-MAX_PATTERNS, a
|       user defined pattern is set.  A user defined pattern is one
|       created with gxCreateFillPattern().
|___________________________________________________________________*/

void gxSetFillPattern (gxPattern pattern)
{
  // Is pattern # valid?
  if ((pattern >= gxPATTERN_SOLID) AND (pattern < MAX_PATTERNS))
    // Is pattern valid?
    if (gx_Pattern_list[pattern].type)
      gx_Fill_pattern = pattern;
}

/*____________________________________________________________________
|
| Function: gxGetFillPattern
|
| Output: Returns current fill pattern.
|___________________________________________________________________*/

inline gxPattern gxGetFillPattern (void)
{
  return (gx_Fill_pattern);
}

/*____________________________________________________________________
|
| Function: gxSetLogicOp
|
| Output: Sets current logic operation for drawing.
|___________________________________________________________________*/

inline void gxSetLogicOp (int logic_op)
{
  (*gx_Video.set_logic_op) (logic_op);
  gx_Logic_op = logic_op;
}

/*____________________________________________________________________
|
| Function: gxGetLogicOp
|
| Output: Returns logic operation.
|___________________________________________________________________*/

inline int gxGetLogicOp (void)
{
  return (gx_Logic_op);
}

/*____________________________________________________________________
|
| Function: gxSetWindow
|
| Output: Sets window for drawing.  Drawing will be window-relative.
|       To clip drawing to the window, call gxSetClip() also.
|___________________________________________________________________*/

inline void gxSetWindow (gxRectangle *win)
{
  gxRectangle box;

  box = *win;

  // Clip window against page
  if (Clip_Rectangle_To_Page (&box.xleft, &box.ytop, &box.xright, &box.ybottom))
    gx_Window = box;
}

/*____________________________________________________________________
|
| Function: Clip_Rectangle_To_Page
|
| Input: Called from gxSetWindow(), gxSetClip()
| Output: Clips input coordinates to active page.  Returns true if any
|       part of input rectangle is within page, else false if input
|       rectangle is entirely clipped.
|___________________________________________________________________*/

static int Clip_Rectangle_To_Page (int *xleft, int *ytop, int *xright, int *ybottom)
{
  int rc;
  gxRectangle save_clip;

  // Save current clip
  save_clip = gx_Clip;
  // Set clip to entire page
  gx_Clip.xleft   = 0;
  gx_Clip.ytop    = 0;
  gx_Clip.xright  = PAGE_WIDTH - 1;
  gx_Clip.ybottom = PAGE_HEIGHT - 1;

  rc = gxClipRectangle (xleft, ytop, xright, ybottom);

  // Restore clip
  gx_Clip = save_clip;

  return (rc);
}

/*____________________________________________________________________
|
| Function: gxGetWindow
|
| Output: Returns current window.
|___________________________________________________________________*/

inline void gxGetWindow (gxRectangle *win)
{
  *win = gx_Window;
}

/*____________________________________________________________________
|
| Function: gxSetClip
|
| Output: Sets clipping rectangle.  Input coords are screen relative.
|___________________________________________________________________*/

inline void gxSetClip (gxRectangle *clip)
{
  gxRectangle box;

  box = *clip;

  // Clip rectangle against page
  if (Clip_Rectangle_To_Page (&box.xleft, &box.ytop, &box.xright, &box.ybottom))
    gx_Clip = box;
}

/*____________________________________________________________________
|
| Function: gxGetClip
|
| Output: Returns current clip window.
|___________________________________________________________________*/

inline void gxGetClip (gxRectangle *clip)
{
  *clip = gx_Clip;
}

/*____________________________________________________________________
|
| Function: gxSetClipping
|
| Output: Sets clipping status to on or off.
|___________________________________________________________________*/

inline void gxSetClipping (int flag)
{
  gx_Clipping = flag;
}

/*____________________________________________________________________
|
| Function: gxGetClipping
|
| Output: Returns clipping status.
|___________________________________________________________________*/

inline int gxGetClipping (void)
{
  return (gx_Clipping);
}

/*____________________________________________________________________
|
| Function: gxClearWindow
|
| Output: Clears window with current color.
|___________________________________________________________________*/

inline void gxClearWindow (void)
{
  (*gx_Video.draw_fill_rectangle) (gx_Window.xleft,
                                   gx_Window.ytop,
                                   gx_Window.xright,
                                   gx_Window.ybottom);
}

/*____________________________________________________________________
|
| Function: gxGetMaxX
|
| Output: Returns max x coord of current window.
|___________________________________________________________________*/

inline int gxGetMaxX (void)
{
  return (gx_Window.xright - gx_Window.xleft);
}

/*____________________________________________________________________
|
| Function: gxGetMaxY
|
| Output: Returns max y coord of current window.
|___________________________________________________________________*/

inline int gxGetMaxY (void)
{
  return (gx_Window.ybottom - gx_Window.ytop);
}
