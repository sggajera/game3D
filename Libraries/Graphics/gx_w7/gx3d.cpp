/*____________________________________________________________________
|
| File: gx3d.cpp
|
| Description: Various functions for 3D graphics.
|
| Functions:  gx3d_BeginRender
|             gx3d_EndRender
|             gx3d_SetFillMode
|             gx3d_GetFillMode
|             gx3d_GetDriverInfo
|
|             gx3d_SetViewport
|             gx3d_GetViewport
|             gx3d_ClearViewport
|             gx3d_ClearViewportRectangle
|
|             gx3d_EnableClipping
|             gx3d_DisableClipping
|             gx3d_InitClipPlane
|             gx3d_FreeClipPlane
|             gx3d_EnableClipPlane
|             gx3d_DisableClipPlane
|             
|             gx3d_EnableZBuffer
|             gx3d_DisableZBuffer
|             gx3d_SetBackfaceRemoval
|
|             gx3d_EnableStencilBuffer
|             gx3d_DisableStencilBuffer
|             gx3d_SetStencilFailOp
|             gx3d_SetStencilZFailOp
|             gx3d_SetStencilPassOp
|             gx3d_SetStencilComparison
|             gx3d_SetStencilReferenceValue
|             gx3d_SetStencilMask
|             gx3d_SetStencilWriteMask
|
|             gx3d_EnableLighting
|             gx3d_DisableLighting
|             gx3d_SetAmbientLight
|             gx3d_EnableSpecularLighting
|             gx3d_DisableSpecularLighting
|             gx3d_EnableVertexLighting
|             gx3d_DisableVertexLighting
|             gx3d_InitLight
|             gx3d_UpdateLight
|             gx3d_FreeLight
|             gx3d_EnableLight
|             gx3d_DisableLight
|
|             gx3d_EnableFog
|             gx3d_DisableFog
|             gx3d_SetFogColor
|             gx3d_SetLinearPixelFog
|             gx3d_SetExpPixelFog
|             gx3d_SetExp2PixelFog
|             gx3d_SetLinearVertexFog
|
|             gx3d_SetMaterial
|             gx3d_GetMaterial
|                                      
|             gx3d_EnableAlphaBlending
|             gx3d_DisableAlphaBlending
|             gx3d_SetAlphaBlendFactor
|
|             gx3d_EnableAntialiasing
|             gx3d_DisableAntiasliasing
|  
| Notes: 
|   Z-Buffer
|     This code assumes use of a zbuffer for rendering.  The system will
|     attempt to create the deepest zbuffer possible in order to get the
|     best quality rendering.
|
|   Viewport
|     Only one viewport is allowed (currently).
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

/*____________________________________________________________________
|
| Function: gx3d_BeginRender
|
| Output: Returns true on success, else false on any error.
|___________________________________________________________________*/

int gx3d_BeginRender ()
{
  int rc;

  // Make sure this function is supported
  DEBUG_ASSERT (gx_Video.begin_render);

  rc = (*gx_Video.begin_render) ();

  return (rc);
}

/*____________________________________________________________________
|
| Function: gx3d_EndRender
|
| Output: 
|___________________________________________________________________*/

void gx3d_EndRender ()
{
  // Make sure this function is supported
  DEBUG_ASSERT (gx_Video.end_render);

  (*gx_Video.end_render) ();
}

/*____________________________________________________________________
|
| Function: gx3d_SetFillMode
|
| Output: Sets the render fill mode to one of the following:
|     gx3d_FILL_MODE_POINT          
|     gx3d_FILL_MODE_WIREFRAME      
|     gx3d_FILL_MODE_SMOOTH_SHADED  
|     gx3d_FILL_MODE_GOURAUD_SHADED   
|___________________________________________________________________*/

inline void gx3d_SetFillMode (int fill_mode)
{
  if (gx_Video.set_fill_mode)
    (*gx_Video.set_fill_mode) (fill_mode);
  gx3d_Fill_mode = fill_mode;
}

/*____________________________________________________________________
|
| Function: gx3d_GetFillMode
|
| Output: Returns the render fill mode which is one of the following:
|     gx3d_FILL_MODE_POINT          
|     gx3d_FILL_MODE_WIREFRAME      
|     gx3d_FILL_MODE_SMOOTH_SHADED  
|     gx3d_FILL_MODE_GOURAUD_SHADED   
|___________________________________________________________________*/

inline int gx3d_GetFillMode ()
{
  return (gx3d_Fill_mode);
}

/*____________________________________________________________________
|
| Function: gx3d_GetDriverInfo
|
| Output: Returns info about the 3D driver.
|___________________________________________________________________*/

void gx3d_GetDriverInfo (gx3dDriverInfo *info)
{
  if (gx_Video.get_driver_info)
    (*gx_Video.get_driver_info) (&(info->max_texture_dx),
                                 &(info->max_texture_dy),
                                 &(info->max_active_lights),
                                 &(info->max_user_clip_planes),
                                 &(info->max_simultaneous_texture_stages),
                                 &(info->max_texture_stages),
                                 &(info->max_texture_repeat),
                                 &(info->num_stencil_bits),
                                 &(info->stencil_ops),
                                 &(info->max_vertex_blend_matrices),
                                 &(info->max_vertex_streams),
                                 &(info->max_vertex_index) );
}

/*____________________________________________________________________
|
| Function: gx3d_SetViewport
|
| Output: Defines the onscreen window dimensions of a render target
|   surface onto which a 3D volume projects.
|___________________________________________________________________*/

void gx3d_SetViewport (gxRectangle *win)
{
  // Make sure this function is supported
  DEBUG_ASSERT (gx_Video.set_viewport);

  (*gx_Video.set_viewport) (win->xleft, win->ytop, win->xright, win->ybottom);
  memcpy (&gx3d_Viewport, win, sizeof(gxRectangle));
}

/*____________________________________________________________________
|
| Function: gx3d_GetViewport
|
| Output: Returns the current onscreen viewport.
|___________________________________________________________________*/

void gx3d_GetViewport (gxRectangle *win)
{
  *win = gx3d_Viewport;
}

/*____________________________________________________________________
|
| Function: gx3d_ClearViewport
|
| Output: Clears the current onscreen viewport. Can optionally clear
|   the viewport surface to a color, the zbuffer to a value and the
|   stencil buffer to a value.
|___________________________________________________________________*/

#define _RED_   surface_color.r
#define _GREEN_ surface_color.g
#define _BLUE_  surface_color.b
#define _ALPHA_ surface_color.a

inline void gx3d_ClearViewport (
  unsigned flags,           // surface? zbuffer? stencil?
  gxColor  surface_color,   // new color to write to surface
  float    z_value,         // new z buffer value, 0.0 (nearest) to 1.0 (farthest)
  unsigned stencil_value )  // new stencil buffer value
{
  if (gx_Video.clear_viewport_rectangle)
    (*gx_Video.clear_viewport_rectangle) ((int *)&gx3d_Viewport, flags, _RED_, _GREEN_, _BLUE_, _ALPHA_, z_value, stencil_value);
}

#undef _RED_  
#undef _GREEN_
#undef _BLUE_ 
#undef _ALPHA_

/*____________________________________________________________________
|
| Function: gx3d_ClearViewportRectangle
|
| Output: Clears a rectangle on the current onscreen viewport. Can
|   optionally clear the viewport surface to a color, the zbuffer to a
|   value and the stencil buffer to a value.
|___________________________________________________________________*/

#define _RED_   surface_color.r
#define _GREEN_ surface_color.g
#define _BLUE_  surface_color.b
#define _ALPHA_ surface_color.a

inline void gx3d_ClearViewportRectangle (
  gxRectangle *rect,        // screen-relative rectangle
  unsigned flags,           // surface? zbuffer? stencil?
  gxColor  surface_color,   // new color to write to surface
  float    z_value,         // new z buffer value, 0.0 (nearest) to 1.0 (farthest)
  unsigned stencil_value )  // new stencil buffer value
{
  if (gx_Video.clear_viewport_rectangle)
    (*gx_Video.clear_viewport_rectangle) ((int *)rect, flags, _RED_, _GREEN_, _BLUE_, _ALPHA_, z_value, stencil_value);
}

#undef _RED_  
#undef _GREEN_
#undef _BLUE_ 
#undef _ALPHA_

/*___________________________________________________________________
|
|	Function: gx3d_EnableClipping
| 
| Output: Enables clipping to view frustum.  Normally this
|   should be on and should only be turned off when drawing objects
|   known to be completely within the view frustum.  
|
|   The default is clipping is enabled.
|___________________________________________________________________*/

inline void gx3d_EnableClipping (void)
{
  if (gx_Video.enable_clipping)
    (*gx_Video.enable_clipping) (TRUE);
}

/*___________________________________________________________________
|
|	Function: gx3d_DisableClipping
| 
| Output: Disables clipping to view frustum.  Normally this
|   should be on and should only be turned off when drawing objects
|   known to be completely within the view frustum.  
|
|   The default is clipping is enabled.
|___________________________________________________________________*/

inline void gx3d_DisableClipping (void)
{
  if (gx_Video.enable_clipping)
    (*gx_Video.enable_clipping) (FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_InitClipPlane
|
| Output: Adds clipping plane but does not enable it.  Returns a handle
|   to the plane or zero on any error.  
|
|   Parameters represent the equation of a plane Ax + Bx + Cx + D = 0 in 
|   world coordinates.  A point with world-space coordinates (x,y,z) is 
|   visible in the half space of the plane if Ax + Bx + Cx + D >= 0.  
|   Points that exist on or behind the plane are clipped from the scene.
|
|   The plane must be enabled with gx3d_EnableClipPlane() before actual
|   clipping occurs.    
|
|   A max of 32 clipping planes can be in existence at any one time.
|   Attempting to create more will cause this function to return 0.
|___________________________________________________________________*/

inline gx3dClipPlane gx3d_InitClipPlane (float a, float b, float c, float d)
{
  int plane = 0;

  if (gx_Video.init_clip_plane)
    plane = (*gx_Video.init_clip_plane) (a, b, c, d);

  return ((gx3dClipPlane)plane);
}

/*____________________________________________________________________
|
| Function: gx3d_FreeClipPlane
|
| Output: Frees a clipping plane previously created with gx3d_InitClipPlane().
|___________________________________________________________________*/

inline void gx3d_FreeClipPlane (gx3dClipPlane plane)
{
  if (plane)
    if (gx_Video.free_clip_plane)
      (*gx_Video.free_clip_plane) (plane);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableClipPlane
|
| Output: Enables a clipping plane previously created with gx3d_InitClipPlane().
|___________________________________________________________________*/

inline void gx3d_EnableClipPlane (gx3dClipPlane plane)
{
  if (plane)
    if (gx_Video.enable_clip_plane)
      (*gx_Video.enable_clip_plane) (plane, TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableClipPlane
|
| Output: Disables a clipping plane previously created with gx3d_InitClipPlane().
|___________________________________________________________________*/

inline void gx3d_DisableClipPlane (gx3dClipPlane plane)
{
  if (plane)
    if (gx_Video.enable_clip_plane)
      (*gx_Video.enable_clip_plane) (plane, FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableZBuffer
|
| Output: Enables zbuffering.
|___________________________________________________________________*/

inline void gx3d_EnableZBuffer (void)
{
  if (gx_Video.enable_zbuffer)
    (*gx_Video.enable_zbuffer) (TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableZBuffer
|
| Output: Disables zbuffering.
|___________________________________________________________________*/

inline void gx3d_DisableZBuffer (void)
{
  if (gx_Video.enable_zbuffer)
    (*gx_Video.enable_zbuffer) (FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_SetBackfaceRemoval
|
| Output: Enables/disables backface removal.  Default is enabled.
|___________________________________________________________________*/

inline void gx3d_SetBackfaceRemoval (int flag)
{
  if (gx_Video.enable_backface_removal)
    (*gx_Video.enable_backface_removal) (flag);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableStencilBuffer
|
| Output: Enables stencil buffer processing.
|___________________________________________________________________*/

inline void gx3d_EnableStencilBuffer (void)
{
  if (gx_Video.enable_stencil_buffer)
    (*gx_Video.enable_stencil_buffer) (TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableStencilBuffer
|
| Output: Disables stencil buffer processing.
|___________________________________________________________________*/

inline void gx3d_DisableStencilBuffer (void)
{
  if (gx_Video.enable_stencil_buffer)
    (*gx_Video.enable_stencil_buffer) (FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_SetStencilFailOp
|
| Output: Sets the stencil operation to perform if the stencil test
|     fails.  Operation can be one of the following:
|
|       gx3d_STENCILOP_DECR     
|       gx3d_STENCILOP_DECRSAT  
|       gx3d_STENCILOP_INCR     
|       gx3d_STENCILOP_INCRSAT  
|       gx3d_STENCILOP_INVERT   
|       gx3d_STENCILOP_KEEP   (default)
|       gx3d_STENCILOP_REPLACE  
|       gx3d_STENCILOP_ZERO     
|___________________________________________________________________*/

inline void gx3d_SetStencilFailOp (int stencil_op)
{
  if (gx_Video.set_stencil_fail_op)
    (*gx_Video.set_stencil_fail_op) (stencil_op);
}

/*____________________________________________________________________
|
| Function: gx3d_SetStencilZFailOp
|
| Output: Sets the stencil operation to perform if the stencil test
|     passes and the depth test fails. Operation can be one of the 
|     following:
|
|       gx3d_STENCILOP_DECR     
|       gx3d_STENCILOP_DECRSAT  
|       gx3d_STENCILOP_INCR     
|       gx3d_STENCILOP_INCRSAT  
|       gx3d_STENCILOP_INVERT   
|       gx3d_STENCILOP_KEEP   (default)
|       gx3d_STENCILOP_REPLACE  
|       gx3d_STENCILOP_ZERO     
|___________________________________________________________________*/

inline void gx3d_SetStencilZFailOp (int stencil_op) 
{
  if (gx_Video.set_stencil_zfail_op)
    (*gx_Video.set_stencil_zfail_op) (stencil_op);
}

/*____________________________________________________________________
|
| Function: gx3d_SetStencilPassOp
|
| Output: Sets the stencil operation to perform if both the stencil test
|     passes and the depth test passes. Operation can be one of the 
|     following:
|
|       gx3d_STENCILOP_DECR     
|       gx3d_STENCILOP_DECRSAT  
|       gx3d_STENCILOP_INCR     
|       gx3d_STENCILOP_INCRSAT  
|       gx3d_STENCILOP_INVERT   
|       gx3d_STENCILOP_KEEP   (default)
|       gx3d_STENCILOP_REPLACE  
|       gx3d_STENCILOP_ZERO     
|___________________________________________________________________*/

inline void gx3d_SetStencilPassOp (int stencil_op)
{
  if (gx_Video.set_stencil_pass_op)
    (*gx_Video.set_stencil_pass_op) (stencil_op);
}

/*____________________________________________________________________
|
| Function: gx3d_SetStencilComparison
|
| Output: Sets the stencil comparison function. The comparison function
|     compares the reference value to a stencil buffer entry and applies
|     only to the bits in the reference value and stencil buffer entry
|     that are set in the stencil mask.  If the comparison is true, the
|     stencil test passes.
|
|     Can be one of the following:
|
|       gx3d_STENCILFUNC_NEVER        
|       gx3d_STENCILFUNC_LESS         
|       gx3d_STENCILFUNC_EQUAL        
|       gx3d_STENCILFUNC_LESSEQUAL    
|       gx3d_STENCILFUNC_GREATER      
|       gx3d_STENCILFUNC_NOTEQUAL     
|       gx3d_STENCILFUNC_GREATEREQUAL 
|       gx3d_STENCILFUNC_ALWAYS   (default)       
|___________________________________________________________________*/

inline void gx3d_SetStencilComparison (int stencil_function)
{
  if (gx_Video.set_stencil_comparison)
    (*gx_Video.set_stencil_comparison) (stencil_function);
}

/*____________________________________________________________________
|
| Function: gx3d_SetStencilReferenceValue
|
| Output: Sets the integer reference value for the stencil test.  The
|     default value is 0.
|___________________________________________________________________*/

inline void gx3d_SetStencilReferenceValue (unsigned reference_value)
{
  if (gx_Video.set_stencil_reference_value)
    (*gx_Video.set_stencil_reference_value) (reference_value);
}

/*____________________________________________________________________
|
| Function: gx3d_SetStencilMask
|
| Output: Sets the mask to apply to the reference value and each stencil
|     buffer entry to determine the significant bits for the stencil test.  
|     The default mask is 0xFFFFFFFF.
|___________________________________________________________________*/

inline void gx3d_SetStencilMask (unsigned mask)
{
  if (gx_Video.set_stencil_mask)
    (*gx_Video.set_stencil_mask) (mask);
}

/*____________________________________________________________________
|
| Function: gx3d_SetStencilWriteMask
|
| Output: Sets the mask to apply to values written into the stencil 
|     buffer.  The default is 0xFFFFFFFF. 
|___________________________________________________________________*/

inline void gx3d_SetStencilWriteMask (unsigned mask)
{
  if (gx_Video.set_stencil_write_mask)
    (*gx_Video.set_stencil_write_mask) (mask);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableLighting
|
| Output: Enables lighting.
|___________________________________________________________________*/

inline void gx3d_EnableLighting (void)
{
  if (gx_Video.enable_lighting)
    (*gx_Video.enable_lighting) (TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableLighting
|
| Output: Disables lighting.
|___________________________________________________________________*/

inline void gx3d_DisableLighting (void)
{
  if (gx_Video.enable_lighting)
    (*gx_Video.enable_lighting) (FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_SetAmbientLight
|
| Output: Sets the ambient light.
|___________________________________________________________________*/

inline void gx3d_SetAmbientLight (gx3dColor color)
{
  if (gx_Video.set_ambient_light)
    (*gx_Video.set_ambient_light) ((float *)&color);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableSpecularLightning
|
| Output: Enables specular highlights.
|___________________________________________________________________*/

inline void gx3d_EnableSpecularLighting (void) 
{
  if (gx_Video.enable_specular_lighting)
    (*gx_Video.enable_specular_lighting) (TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableSpecularLighting
|
| Output: Disables specular highlights.
|___________________________________________________________________*/

inline void gx3d_DisableSpecularLighting (void)
{
  if (gx_Video.enable_specular_lighting)
    (*gx_Video.enable_specular_lighting) (FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableVertexLightning
|
| Output: Enables vertex color lighting info.  If enabled (the default),
|   and specifying a diffuse vertex color, the output alpha value will 
|   equal to the diffuse alpha value of the vertex.
|___________________________________________________________________*/

inline void gx3d_EnableVertexLighting (void)
{
  if (gx_Video.enable_vertex_lighting)
    (*gx_Video.enable_vertex_lighting) (TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableVertexLighting
|
| Output: Disables vertex color lighting info.  If disabled, the output
|   alpha value will be equal to the alpha component of the diffuse
|   material, clamped to a range of 0-255.
|___________________________________________________________________*/

inline void gx3d_DisableVertexLighting (void)
{
  if (gx_Video.enable_vertex_lighting)
    (*gx_Video.enable_vertex_lighting) (FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_InitLight
|
| Output: Creates a light in a disabled state.  Returns a handle to the
|   light or zero on any error.  
|
|   A max of 8 lights can be in existence at any one time.
|   Attempting to create more will cause this function to return 0.
|___________________________________________________________________*/

gx3dLight gx3d_InitLight (gx3dLightData *data)
{
  gx3dLight light = 0;

  switch (data->light_type) {
    case gx3d_LIGHT_TYPE_POINT:
      if (gx_Video.init_point_light)
        light = (*gx_Video.init_point_light) (data->point.src.x, 
                                              data->point.src.y, 
                                              data->point.src.z, 
                                              data->point.range, 
                                              data->point.constant_attenuation,
                                              data->point.linear_attenuation,
                                              data->point.quadratic_attenuation,
                                              (float *)&data->point.ambient_color,
                                              (float *)&data->point.diffuse_color,
                                              (float *)&data->point.specular_color );
      break;
    case gx3d_LIGHT_TYPE_SPOT:
      if (gx_Video.init_spot_light)
        light = (*gx_Video.init_spot_light) (data->spot.src.x, 
                                             data->spot.src.y,
                                             data->spot.src.z,
                                             data->spot.dst.x,
                                             data->spot.dst.y,
                                             data->spot.dst.z,
                                             data->spot.range,
                                             data->spot.constant_attenuation,
                                             data->spot.linear_attenuation,
                                             data->spot.quadratic_attenuation, 
                                             data->spot.inner_cone_angle,     
                                             data->spot.outer_cone_angle,     
                                             data->spot.falloff,              
                                             (float *)&data->spot.ambient_color,
                                             (float *)&data->spot.diffuse_color,
                                             (float *)&data->spot.specular_color );
      break;
    case gx3d_LIGHT_TYPE_DIRECTION:
      if (gx_Video.init_direction_light)
        light = (*gx_Video.init_direction_light) (data->direction.dst.x, 
                                                  data->direction.dst.y,
                                                  data->direction.dst.z,
                                                  (float *)&data->direction.ambient_color,
                                                  (float *)&data->direction.diffuse_color,
                                                  (float *)&data->direction.specular_color );
      break;
  }

  return (light);
}

/*____________________________________________________________________
|
| Function: gx3d_UpdateLight
|
| Output: Updates parameters for a light.
|___________________________________________________________________*/

void gx3d_UpdateLight (gx3dLight light, gx3dLightData *data)
{
  switch (data->light_type) {
    case gx3d_LIGHT_TYPE_POINT:
      if (gx_Video.update_point_light)
        (*gx_Video.update_point_light) (light,
                                        data->point.src.x, 
                                        data->point.src.y, 
                                        data->point.src.z, 
                                        data->point.range, 
                                        data->point.constant_attenuation,
                                        data->point.linear_attenuation,
                                        data->point.quadratic_attenuation,
                                        (float *)&data->point.ambient_color,
                                        (float *)&data->point.diffuse_color,
                                        (float *)&data->point.specular_color );
      break;
    case gx3d_LIGHT_TYPE_SPOT:
      if (gx_Video.update_spot_light)
        (*gx_Video.update_spot_light) (light,
                                       data->spot.src.x, 
                                       data->spot.src.y,
                                       data->spot.src.z,
                                       data->spot.dst.x,
                                       data->spot.dst.y,
                                       data->spot.dst.z,
                                       data->spot.range,
                                       data->spot.constant_attenuation,
                                       data->spot.linear_attenuation,
                                       data->spot.quadratic_attenuation, 
                                       data->spot.inner_cone_angle,     
                                       data->spot.outer_cone_angle,     
                                       data->spot.falloff,              
                                       (float *)&data->spot.ambient_color,
                                       (float *)&data->spot.diffuse_color,
                                       (float *)&data->spot.specular_color );
      break;
    case gx3d_LIGHT_TYPE_DIRECTION:
      if (gx_Video.update_direction_light)
        (*gx_Video.update_direction_light) (light,
                                            data->direction.dst.x, 
                                            data->direction.dst.y,
                                            data->direction.dst.z,
                                            (float *)&data->direction.ambient_color,
                                            (float *)&data->direction.diffuse_color,
                                            (float *)&data->direction.specular_color );
      break;
  }
}

/*____________________________________________________________________
|
| Function: gx3d_FreeLight
|
| Output: Destroys a light.
|___________________________________________________________________*/

inline void gx3d_FreeLight (gx3dLight light)
{
  if (gx_Video.free_light)
    (*gx_Video.free_light) (light);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableLight
|
| Output: Enables a light.
|___________________________________________________________________*/

inline void gx3d_EnableLight (gx3dLight light)
{
  if (gx_Video.enable_light)
    (*gx_Video.enable_light) (light, TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableLight
|
| Output: Disables a light.
|___________________________________________________________________*/

inline void gx3d_DisableLight (gx3dLight light)
{
  if (gx_Video.enable_light)
    (*gx_Video.enable_light) (light, FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableFog
|
| Output: Enables fog using the previously set formula and color.  If 
|   no previously set fog formula and fog color, does nothing.
|___________________________________________________________________*/

inline void gx3d_EnableFog (void)
{
  if (gx_Video.enable_fog)
    (*gx_Video.enable_fog) (TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableFog
|
| Output: Disables fog.
|___________________________________________________________________*/

inline void gx3d_DisableFog (void)
{
  if (gx_Video.enable_fog)
    (*gx_Video.enable_fog) (FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_SetFogColor
|
| Output: Sets the fog color.
|___________________________________________________________________*/

inline void gx3d_SetFogColor (byte r, byte g, byte b)
{
  if (gx_Video.set_fog_color) 
    (*gx_Video.set_fog_color) (r, g, b);
}

/*____________________________________________________________________
|
| Function: gx3d_SetLinearPixelFog
|
| Output: Sets the fog formula to a linear pixel fog.
|___________________________________________________________________*/

inline void gx3d_SetLinearPixelFog (float start_distance, float end_distance)
{
  if (gx_Video.set_linear_pixel_fog) 
    (*gx_Video.set_linear_pixel_fog) (start_distance, end_distance);
}

/*____________________________________________________________________
|
| Function: gx3d_SetExpPixelFog
|
| Output: Sets the fog formula to an exponential pixel fog.
|___________________________________________________________________*/

inline void gx3d_SetExpPixelFog (
  float density ) // 0-1
{
  if (gx_Video.set_exp_pixel_fog) 
    (*gx_Video.set_exp_pixel_fog) (density);
}

/*____________________________________________________________________
|
| Function: gx3d_SetExp2PixelFog
|
| Output: Sets the fog formula to an exponential squared pixel fog.
|___________________________________________________________________*/

inline void gx3d_SetExp2PixelFog (
  float density ) // 0-1
{
  if (gx_Video.set_exp2_pixel_fog) 
    (*gx_Video.set_exp2_pixel_fog) (density);
}

/*____________________________________________________________________
|
| Function: gx3d_SetLinearVertexFog
|
| Output: Sets the fog formula to an exponential pixel fog.
|___________________________________________________________________*/

inline void gx3d_SetLinearVertexFog (
  float start_distance, 
  float end_distance, 
  int   ranged_based )  // boolean
{
  if (gx_Video.set_linear_vertex_fog) 
    (*gx_Video.set_linear_vertex_fog) (start_distance, end_distance, ranged_based);
}

/*____________________________________________________________________
|
| Function: gx3d_SetMaterial
|
| Output: Sets the current render material.
|___________________________________________________________________*/

inline void gx3d_SetMaterial (gx3dMaterialData *data)
{
  if (gx_Video.set_material)
    (*gx_Video.set_material) ((float *)&data->ambient_color,
                              (float *)&data->diffuse_color,
                              (float *)&data->specular_color,
                              (float *)&data->emissive_color,
                              data->specular_sharpness );
}
 
/*____________________________________________________________________
|
| Function: gx3d_GetMaterial
|
| Output: Gets the current render material.
|___________________________________________________________________*/

inline void gx3d_GetMaterial (gx3dMaterialData *data)
{
  if (gx_Video.get_material)
    (*gx_Video.get_material) ((float *)&data->ambient_color,
                              (float *)&data->diffuse_color,
                              (float *)&data->specular_color,
                              (float *)&data->emissive_color,
                              &data->specular_sharpness );
}

/*____________________________________________________________________
|
| Function: gx3d_EnableAlphaBlending
|
| Output: Enables alpha blending.
|___________________________________________________________________*/

inline void gx3d_EnableAlphaBlending (void)
{
  if (gx_Video.enable_alpha_blending)
    (*gx_Video.enable_alpha_blending) (TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableAlphaBlending
|
| Output: Enables alpha blending.
|___________________________________________________________________*/

inline void gx3d_DisableAlphaBlending (void)
{
  if (gx_Video.enable_alpha_blending)
    (*gx_Video.enable_alpha_blending) (FALSE);
}

/*____________________________________________________________________
|
| Function: gx3d_SetAlphaBlendFactor
|
| Output: Sets the alpha blending src and dst blend factors.
|___________________________________________________________________*/

inline void gx3d_SetAlphaBlendFactor (int src_blend_factor, int dst_blend_factor)
{
  if (gx_Video.set_alpha_blend_factor)
    (*gx_Video.set_alpha_blend_factor) (src_blend_factor, dst_blend_factor);
}

/*____________________________________________________________________
|
| Function: gx3d_AlphaTestingAvailable
|
| Output: Returns true if alpha testing is supported.
|___________________________________________________________________*/

inline int gx3d_AlphaTestingAvailable (void)
{ 
  int rc = FALSE;

  if (gx_Video.alpha_testing_available)
    rc = (*gx_Video.alpha_testing_available) ();

  return (rc);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableAlphaTesting
|
| Output: Enables alpha testing, if supported.  Caller can use
|     gx3d_AlphaTestingAvailable() to first determine if alpha testing
|     is supported.  
|___________________________________________________________________*/

inline void gx3d_EnableAlphaTesting (byte reference_value)
{
  if (gx_Video.enable_alpha_testing)
    (*gx_Video.enable_alpha_testing) (TRUE, reference_value);
}
    
/*____________________________________________________________________
|
| Function: gx3d_DisableAlphaTesting
|
| Output: Disables alpha testing.
|___________________________________________________________________*/

inline void gx3d_DisableAlphaTesting (void)
{
  if (gx_Video.enable_alpha_testing)
    (*gx_Video.enable_alpha_testing) (FALSE, 0);
}

/*____________________________________________________________________
|
| Function: gx3d_EnableAntialiasing
|
| Output: Enables antialiasing, if available.
|___________________________________________________________________*/

inline void gx3d_EnableAntialiasing (void)
{
  if (gx_Video.enable_antialiasing)
    (*gx_Video.enable_antialiasing) (TRUE);
}

/*____________________________________________________________________
|
| Function: gx3d_DisableAntialiasing
|
| Output: Disables antialiasing, if on.
|___________________________________________________________________*/

inline void gx3d_DisableAntialiasing (void)
{
  if (gx_Video.enable_antialiasing)
    (*gx_Video.enable_antialiasing) (FALSE);
}
