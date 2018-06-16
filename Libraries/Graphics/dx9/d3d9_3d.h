/*____________________________________________________________________
|
| File: d3d9_3d.h (Direct3D 3D functions)
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*____________________
|
|	Aliases
|___________________*/

#define Direct3D_QueryHardware                    d3d9_QueryHardware
#define Direct3D_UserSelectMode                   d3d9_UserSelectMode
#define Direct3D_SetMode                          d3d9_SetMode
#define Direct3D_Restore                          d3d9_Restore
#define Direct3D_Free                             d3d9_Free
#define Direct3D_GetScreenDimensions              d3d9_GetScreenDimensions
#define Direct3D_GetPixelSize                     d3d9_GetPixelSize
#define Direct3D_BeginRender                      d3d9_BeginRender
#define Direct3D_EndRender                        d3d9_EndRender   
#define Direct3D_SetFillMode                      d3d9_SetFillMode
#define Direct3D_GetDriverInfo                    d3d9_GetDriverInfo
#define Direct3D_InitObject                       d3d9_InitObject
#define Direct3D_FreeObject                       d3d9_FreeObject
#define Direct3D_DrawObject                       d3d9_DrawObject
#define Direct3D_SetViewport                      d3d9_SetViewport
#define Direct3D_ClearViewportRectangle           d3d9_ClearViewportRectangle
#define Direct3D_EnableClipping                   d3d9_EnableClipping
#define Direct3D_InitClipPlane                    d3d9_InitClipPlane
#define Direct3D_EnableClipPlane                  d3d9_EnableClipPlane
#define Direct3D_SetWorldMatrix                   d3d9_SetWorldMatrix
#define Direct3D_GetWorldMatrix                   d3d9_GetWorldMatrix
#define Direct3D_SetViewMatrix                    d3d9_SetViewMatrix 
#define Direct3D_GetViewMatrix                    d3d9_GetViewMatrix 
#define Direct3D_SetProjectionMatrix              d3d9_SetProjectionMatrix
#define Direct3D_GetProjectionMatrix              d3d9_GetProjectionMatrix
#define Direct3D_EnableTextureMatrix              d3d9_EnableTextureMatrix
#define Direct3D_SetTextureMatrix                 d3d9_SetTextureMatrix
#define Direct3D_GetTextureMatrix                 d3d9_GetTextureMatrix
#define Direct3D_EnableZBuffer                    d3d9_EnableZBuffer
#define Direct3D_EnableBackfaceRemoval            d3d9_EnableBackfaceRemoval
#define Direct3D_EnableStencilBuffer              d3d9_EnableStencilBuffer
#define Direct3D_SetStencilFailOp                 d3d9_SetStencilFailOp
#define Direct3D_SetStencilZFailOp                d3d9_SetStencilZFailOp
#define Direct3D_SetStencilPassOp                 d3d9_SetStencilPassOp
#define Direct3D_SetStencilComparison             d3d9_SetStencilComparison
#define Direct3D_SetStencilReferenceValue         d3d9_SetStencilReferenceValue
#define Direct3D_SetStencilMask                   d3d9_SetStencilMask
#define Direct3D_SetStencilWriteMask              d3d9_SetStencilWriteMask
#define Direct3D_EnableLighting                   d3d9_EnableLighting
#define Direct3D_InitPointLight                   d3d9_InitPointLight
#define Direct3D_InitSpotLight                    d3d9_InitSpotLight
#define Direct3D_InitDirectionLight               d3d9_InitDirectionLight
#define Direct3D_EnableLight                      d3d9_EnableLight
#define Direct3D_SetAmbientLight                  d3d9_SetAmbientLight
#define Direct3D_EnableSpecularLighting           d3d9_EnableSpecularLighting
#define Direct3D_EnableVertexLighting             d3d9_EnableVertexLighting
#define Direct3D_EnableFog                        d3d9_EnableFog
#define Direct3D_SetFogColor                      d3d9_SetFogColor
#define Direct3D_SetLinearPixelFog                d3d9_SetLinearPixelFog
#define Direct3D_SetExpPixelFog                   d3d9_SetExpPixelFog
#define Direct3D_SetExp2PixelFog                  d3d9_SetExp2PixelFog
#define Direct3D_SetLinearVertexFog               d3d9_SetLinearVertexFog
#define Direct3D_SetMaterial                      d3d9_SetMaterial
#define Direct3D_GetMaterial                      d3d9_GetMaterial

#define Direct3D_InitTexture                      d3d9_InitTexture
#define Direct3D_InitVolumeTexture                d3d9_InitVolumeTexture
#define Direct3D_InitCubemapTexture               d3d9_InitCubemapTexture
#define Direct3D_FreeTexture                      d3d9_FreeTexture
#define Direct3D_SetTexture                       d3d9_SetTexture
#define Direct3D_SetTextureAddressingMode         d3d9_SetTextureAddressingMode
#define Direct3D_SetTextureBorderColor            d3d9_SetTextureBorderColor
#define Direct3D_SetTextureFiltering              d3d9_SetTextureFiltering
#define Direct3D_SetTextureCoordinates            d3d9_SetTextureCoordinates
#define Direct3D_SetTextureCoordinateWrapping     d3d9_SetTextureCoordinateWrapping
#define Direct3D_SetTextureFactor                 d3d9_SetTextureFactor
#define Direct3D_PreLoadManagedTexture            d3d9_PreLoadManagedTexture
#define Direct3D_EvictManagedTextures             d3d9_EvictManagedTextures
#define Direct3D_SetTextureColorOp                d3d9_SetTextureColorOp
#define Direct3D_SetTextureAlphaOp                d3d9_SetTextureAlphaOp
#define Direct3D_SetTextureColorFactor            d3d9_SetTextureColorFactor
#define Direct3D_EnableCubemapTextureReflections  d3d9_EnableCubemapTextureReflections
#define Direct3D_GetTextureSurface                d3d9_GetTextureSurface
#define Direct3D_GetTextureCubemapSurface         d3d9_GetTextureCubemapSurface
 
#define Direct3D_EnableAlphaBlending              d3d9_EnableAlphaBlending
#define Direct3D_SetAlphaBlendFactor              d3d9_SetAlphaBlendFactor
#define Direct3D_AlphaTestingAvailable            d3d9_AlphaTestingAvailable
#define Direct3D_EnableAlphaTesting               d3d9_EnableAlphaTesting

#define Direct3D_Get_RGB_Format                   d3d9_GetRGBFormat
#define Direct3D_Set_Active_Page                  d3d9_SetActivePage
#define Direct3D_Flip_Visual_Page                 d3d9_FlipVisualPage

/*____________________
|
|	Constants
|___________________*/

// same as in dx9.h
#define RESOLUTION_640x480    ((unsigned)0x1)     // 4:3 unless otherwise noted
#define RESOLUTION_800x600    ((unsigned)0x2)
#define RESOLUTION_1024x768   ((unsigned)0x4)
#define RESOLUTION_1152x864   ((unsigned)0x8)
#define RESOLUTION_1280x960   ((unsigned)0x10)
#define RESOLUTION_1280x1024  ((unsigned)0x20)    // 5:4
#define RESOLUTION_1400x1050  ((unsigned)0x40)
#define RESOLUTION_1440x1080  ((unsigned)0x80)
#define RESOLUTION_1600x1200  ((unsigned)0x100)
#define RESOLUTION_1152x720   ((unsigned)0x200)   // widescreen 8:5
#define RESOLUTION_1280x800   ((unsigned)0x400)   // widescreen 8:5
#define RESOLUTION_1440x900   ((unsigned)0x800)   // widescreen 8:5
#define RESOLUTION_1680x1050  ((unsigned)0x1000)  // widescreen 8:5
#define RESOLUTION_1920x1200  ((unsigned)0x2000)  // widescreen 8:5
#define RESOLUTION_2048x1280  ((unsigned)0x4000)  // widescreen 8:5
#define RESOLUTION_1280x720   ((unsigned)0x8000)  // widescreen 16:9
#define RESOLUTION_1600x900   ((unsigned)0x10000) // widescreen 16:9
#define RESOLUTION_1920x1080  ((unsigned)0x20000) // widescreen 16:9
#define RESOLUTION_2048x1152  ((unsigned)0x40000) // widescreen 16:9
#define RESOLUTION_2560x1440  ((unsigned)0x80000) // widescreen 16:9
#define RESOLUTION_2560x1600  ((unsigned)0x100000) // widescreen 16:10

// same as in dx9.h
#define BITDEPTH_16 0x1
//#define BITDEPTH_24 0x2   // not supported
#define BITDEPTH_32 0x4

// texture addressing modes - same constants as in dx9.h
#define TEXTURE_ADDRESSMODE_WRAP   1
#define TEXTURE_ADDRESSMODE_MIRROR 2
#define TEXTURE_ADDRESSMODE_CLAMP  3
#define TEXTURE_ADDRESSMODE_BORDER 4

// texture dimensions - same constants as in dx9.h
#define TEXTURE_DIMENSION_U 0x1
#define TEXTURE_DIMENSION_V 0x2
#define TEXTURE_DIMENSION_W 0x4

// texture filtering types - same constants as in dx9.h
#define TEXTURE_FILTERTYPE_POINT       1
#define TEXTURE_FILTERTYPE_LINEAR      2
#define TEXTURE_FILTERTYPE_TRILINEAR   3
#define TEXTURE_FILTERTYPE_ANISOTROPIC 4

// fill modes - same constants as in dx9.h
#define FILL_MODE_POINT           1
#define FILL_MODE_WIREFRAME       2
#define FILL_MODE_SMOOTH_SHADED   3
#define FILL_MODE_GOURAUD_SHADED  4  

// Alpha-blending factors - same constants as in dx9.h
#define ALPHABLENDFACTOR_ZERO        1   // blend factor is (0,0,0,0)
#define ALPHABLENDFACTOR_ONE         2   // blend factor is (1,1,1,1)
#define ALPHABLENDFACTOR_SRCCOLOR    3   // blend factor is (s[r],s[g],s[b],s[a])
#define ALPHABLENDFACTOR_DSTCOLOR    4   // blend factor is (d[r],d[g],d[b],d[a])
#define ALPHABLENDFACTOR_SRCALPHA    5   // blend factor is (s[a],s[a],s[a],s[a])
#define ALPHABLENDFACTOR_DSTALPHA    6   // blend factor is (d[a],d[a],d[a],d[a])
#define ALPHABLENDFACTOR_INVSRCCOLOR 7   // blend factor is (1-s[r],1-s[g],1-s[b],1-s[a])
#define ALPHABLENDFACTOR_INVDSTCOLOR 8   // blend factor is (1-d[r],1-d[g],1-d[b],1-d[a])
#define ALHPABLENDFACTOR_INVSRCALPHA 9   // blend factor is (1-s[a],1-s[a],1-s[a],1-s[a])
#define ALHPABLENDFACTOR_INVDSTALPHA 10  // blend factor is (1-d[a],1-d[a],1-d[a],1-d[a])
#define ALHPABLENDFACTOR_SRCALPHASAT 11  // blend factor is (f,f,f,1), where f = min(s[a], 1-d[a])

// Stencil operations - same constants as in dx9.h
#define STENCILOP_DECR    0x1
#define STENCILOP_DECRSAT 0x2
#define STENCILOP_INCR    0x4
#define STENCILOP_INCRSAT 0x8
#define STENCILOP_INVERT  0x10
#define STENCILOP_KEEP    0x20
#define STENCILOP_REPLACE 0x40
#define STENCILOP_ZERO    0x80
                          
// Stencil comparison functions - same constans as in dx9.h
#define STENCILFUNC_NEVER         1
#define STENCILFUNC_LESS          2
#define STENCILFUNC_EQUAL         3
#define STENCILFUNC_LESSEQUAL     4
#define STENCILFUNC_GREATER       5
#define STENCILFUNC_NOTEQUAL      6
#define STENCILFUNC_GREATEREQUAL  7
#define STENCILFUNC_ALWAYS        8

// Texture color blending operations - same constants as in dx9.h
#define TEXTURE_COLOROP_DISABLE                   0
#define TEXTURE_COLOROP_SELECTARG1                1
#define TEXTURE_COLOROP_SELECTARG2                2
#define TEXTURE_COLOROP_MODULATE                  3
#define TEXTURE_COLOROP_MODULATE2X                4
#define TEXTURE_COLOROP_MODULATE4X                5
#define TEXTURE_COLOROP_ADD                       6
#define TEXTURE_COLOROP_ADDSIGNED                 7
#define TEXTURE_COLOROP_ADDSIGNED2X               8
#define TEXTURE_COLOROP_SUBTRACT                  9
#define TEXTURE_COLOROP_ADDSMOOTH                 10
#define TEXTURE_COLOROP_BLENDDIFFUSEALPHA         11
#define TEXTURE_COLOROP_BLENDTEXTUREALPHA         12
#define TEXTURE_COLOROP_BLENDFACTORALPHA          13
#define TEXTURE_COLOROP_BLENDTEXTUREALPHAPM       14
#define TEXTURE_COLOROP_BLENDCURRENTALPHA         15         
#define TEXTURE_COLOROP_PREMODULATE               16  
#define TEXTURE_COLOROP_MODULATEALPHA_ADDCOLOR    17
#define TEXTURE_COLOROP_MODULATECOLOR_ADDALPHA    18
#define TEXTURE_COLOROP_MODULATEINVALPHA_ADDCOLOR 19
#define TEXTURE_COLOROP_MODULATEINVCOLOR_ADDALPHA 20
#define TEXTURE_COLOROP_BUMPENVMAP                21
#define TEXTURE_COLOROP_BUMPENVMAPLUMINANCE       22
#define TEXTURE_COLOROP_DOTPRODUCT3               23
#define TEXTURE_COLOROP_MULTIPLYADD               24
#define TEXTURE_COLOROP_LERP                      25

// Texture alpha blending operations - same constants as in dx9.h
#define TEXTURE_ALPHAOP_DISABLE                   0
#define TEXTURE_ALPHAOP_SELECTARG1                1
#define TEXTURE_ALPHAOP_SELECTARG2                2
#define TEXTURE_ALPHAOP_MODULATE                  3
#define TEXTURE_ALPHAOP_MODULATE2X                4
#define TEXTURE_ALPHAOP_MODULATE4X                5
#define TEXTURE_ALPHAOP_ADD                       6
#define TEXTURE_ALPHAOP_ADDSIGNED                 7
#define TEXTURE_ALPHAOP_ADDSIGNED2X               8
#define TEXTURE_ALPHAOP_SUBTRACT                  9
#define TEXTURE_ALPHAOP_ADDSMOOTH                 10
#define TEXTURE_ALPHAOP_BLENDDIFFUSEALPHA         11
#define TEXTURE_ALPHAOP_BLENDTEXTUREALPHA         12
#define TEXTURE_ALPHAOP_BLENDFACTORALPHA          13
#define TEXTURE_ALPHAOP_BLENDTEXTUREALPHAPM       14
#define TEXTURE_ALPHAOP_BLENDCURRENTALPHA         15         
#define TEXTURE_ALPHAOP_PREMODULATE               16  
#define TEXTURE_ALPHAOP_DOTPRODUCT3               17
#define TEXTURE_ALPHAOP_MULTIPLYADD               18
#define TEXTURE_ALPHAOP_LERP                      19

// Texture stage blending arguments  - same constants as in dx9.h
#define TEXTURE_ARG_CURRENT  0
#define TEXTURE_ARG_DIFFUSE  1
#define TEXTURE_ARG_TEXTURE  2
#define TEXTURE_ARG_TFACTOR  3
#define TEXTURE_ARG_SPECULAR 4

#define NUM_TEXTURE_STAGES  8

/*____________________
|
|	Type definitions
|___________________*/

typedef struct {
  // polygon data
  int       *num_surfaces;          // # of indeces
  word      *surface;               // array of indeces (group of 3 unsigned short)
  // vertex data
  int       *num_vertices;          // # of vertices
  float     *vertex;                // array of vertices (group of 3 float - x,y,z)
  float    **X_vertex;              // array of transformed vertices - for special effects
  float     *vertex_normal;         // array of vertex normals (group of 3 float - x,y,z)
  float    **X_vertex_normal;       // array of vertex normals
  byte      *vertex_color_diffuse;  // array of diffuse color in 0xAARRGGBB format, or 0 if none
  byte      *vertex_color_specular; // array of specular color in 0xAARRGGBB format, or 0 if none
  byte      *weight;                // array of weights - group of 4 floats (values) + 4 bytes (matrix indeces) + 1 byte (num_weights)
  byte     **X_weight;              // array of transformed weights
  float    **texture_coord;         // 0-4 arrays of texture coords
  float    **X_texture_coord;       // 0-4 arrays of transformed texture coords
  float    **texture_coord_w;       // 0-4 arrays of texture w coords
  float    **X_texture_coord_w;     // 0-4 arrays of transformed texture w coords
  // d3d9-specific stuff
  void      *vertex_buffer;         // vertex buffer
  void      *index_buffer;          // index buffer
  unsigned   vertex_size;           // vertex size in bytes
  unsigned   fvf_code;              // D3D flexible format vertex flags
  unsigned   offset_weight;         // offsets into flexible vertexx format
  unsigned   offset_normal;
  unsigned   offset_diffuse;
  unsigned   offset_specular;
  unsigned   offset_texcoord  [NUM_TEXTURE_STAGES];
  unsigned   offset_texcoord_w[NUM_TEXTURE_STAGES];
} d3d9_Object;

/*____________________
|
|	Functions
|___________________*/

int  d3d9_QueryHardware (
  unsigned acceptable_resolutions,
  unsigned acceptable_bitdepths,
  int      enable_hardware_acceleration );
int	 d3d9_UserSelectMode (int *width, int *height, int *depth);
int	 d3d9_SetMode (int width, int height, int depth, unsigned stencil_depth_requested, int num_pages_requested);
int  d3d9_Restore ();
void d3d9_Free ();
void d3d9_GetScreenDimensions (int *width, int *height, int *depth);
int  d3d9_GetPixelSize ();
int  d3d9_BeginRender ();
int  d3d9_EndRender ();
void d3d9_SetFillMode (int fill_mode);
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
  unsigned *max_vertex_index );
void d3d9_InitObject (d3d9_Object *object);
void d3d9_FreeObject (d3d9_Object *object);
void d3d9_DrawObject (d3d9_Object *object);
int  d3d9_SetViewport (int left, int top, int right, int bottom);
void d3d9_ClearViewportRectangle (
  int      *rect, 
  unsigned  flags,  // 0x1=surface, 0x2=zbuffer, 0x4=stencil
  byte      r,
  byte      g,
  byte      b,
  byte      a,
  float     zval,
  unsigned  stencilval );
void d3d9_EnableClipping (int flag);
int  d3d9_InitClipPlane (unsigned index, float a, float b, float c, float d);
void d3d9_EnableClipPlane (
  unsigned plane, // 0-31
  int flag );     // boolean
     
int  d3d9_SetWorldMatrix (int index, void *m);
int  d3d9_GetWorldMatrix (int index, void *m);
int  d3d9_SetViewMatrix (void *m);
int  d3d9_GetViewMatrix (void *m);
int  d3d9_SetProjectionMatrix (void *m);
int  d3d9_GetProjectionMatrix (void *m); 
int  d3d9_EnableTextureMatrix (int stage, int dimension, int flag);
int  d3d9_SetTextureMatrix (int stage, void *m);
int  d3d9_GetTextureMatrix (int stage, void *m);
void d3d9_EnableZBuffer (int flag);   
void d3d9_EnableBackfaceRemoval (int flag);
void d3d9_EnableStencilBuffer (int flag);
void d3d9_SetStencilFailOp (int stencil_op);
void d3d9_SetStencilZFailOp (int stencil_op);
void d3d9_SetStencilPassOp (int stencil_op);
void d3d9_SetStencilComparison (int stencil_function);
void d3d9_SetStencilReferenceValue (unsigned reference_value);
void d3d9_SetStencilMask (unsigned mask);
void d3d9_SetStencilWriteMask (unsigned mask);
void d3d9_EnableLighting (int flag);
int  d3d9_InitPointLight (
  int       index,
  float     src_x,
  float     src_y,
  float     src_z,
  float     range,
  float     constant_attenuation,
  float     linear_attenuation,
  float     quadratic_attenuation, 
  float    *ambient_color_rgba,
  float    *diffuse_color_rgba,
  float    *specular_color_rgba );
int  d3d9_InitSpotLight (
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
  float    *specular_color_rgba );
int  d3d9_InitDirectionLight (
  int       index,
  float     dst_x,
  float     dst_y,
  float     dst_z, 
  float    *ambient_color_rgba,
  float    *diffuse_color_rgba,
  float    *specular_color_rgba );
 void d3d9_EnableLight (int index, int flag);
 void d3d9_SetAmbientLight (float *rgba);
 void d3d9_EnableSpecularLighting (int flag);
 void d3d9_EnableVertexLighting (int flag);

 void d3d9_EnableFog (int flag);
 void d3d9_SetFogColor (byte r, byte g, byte b);
 void d3d9_SetLinearPixelFog (float start_distance, float end_distance);
 void d3d9_SetExpPixelFog (float density);   // 0-1
 void d3d9_SetExp2PixelFog (float density);  // 0-1
 void d3d9_SetLinearVertexFog (float start_distance, float end_distance, int ranged_based);

void d3d9_SetMaterial (
  float    *ambient_color_rgba,
  float    *diffuse_color_rgba,
  float    *specular_color_rgba,
  float    *emissive_color_rgba,
  float     specular_sharpness );
void d3d9_GetMaterial (
  float    *ambient_color_rgba,
  float    *diffuse_color_rgba,
  float    *specular_color_rgba,
  float    *emissive_color_rgba,
  float    *specular_sharpness );

byte *d3d9_InitTexture (
  int       num_mip_levels, 
  byte    **image, 
  byte    **alphamap,
  int       dx, 
  int       dy,
  int       num_color_bits, // 0, 16, 24
  int       num_alpha_bits, // 0, 1, 8
  unsigned *size );
byte *d3d9_InitVolumeTexture (
  int       num_levels,
  int       num_slices, 
  byte    **image, 
  byte    **alphamap,
  int       dx, 
  int       dy,
  int       num_color_bits, // 0, 16, 24
  int       num_alpha_bits, // 0, 1, 8
  unsigned *size );
byte *d3d9_InitCubemapTexture (
  byte    **image, 
  byte    **alphamap,
  int       dimensions, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size);
void d3d9_FreeTexture (byte *texture);
void d3d9_SetTexture (int stage, byte *texture);
void d3d9_SetTextureAddressingMode (int stage, int dimension, int addressing_mode);
void d3d9_SetTextureBorderColor (int stage, byte r, byte g, byte b, byte a);
void d3d9_SetTextureFiltering (
  int stage, 
  int filter_type, 
  int anisotropy_level ); // amount of filtering (1-100), 1=min, 100=max
void d3d9_SetTextureCoordinates (
  int stage,              // texture blending stage 0-7
  int coordinate_stage ); // set of texture coordinates in the object 0-7 (-1=cubemap)
void d3d9_SetTextureCoordinateWrapping (
  int coordinate_stage, // set of texture coordinates in the object 0-7
  int wrap_s,           // boolean (u dimension)
  int wrap_t,           // boolean (v dimension)
  int wrap_r,           // boolean
  int wrap_q );         // boolean
void d3d9_SetTextureFactor (byte r, byte g, byte b, byte a);
void d3d9_PreLoadManagedTexture (byte *texture);
void d3d9_EvictManagedTextures ();
void d3d9_SetTextureColorOp (int stage, int texture_colorop, int texture_arg1, int texture_arg2);
void d3d9_SetTextureAlphaOp (int stage, int texture_alphaop, int texture_arg1, int texture_arg2);
void d3d9_SetTextureColorFactor (float *rgba);
void d3d9_EnableCubemapTextureReflections (int flag);
byte *d3d9_GetTextureSurface        (byte *texture);
byte *d3d9_GetTextureCubemapSurface (byte *texture, int face);

void d3d9_EnableAlphaBlending (int flag);
void d3d9_SetAlphaBlendFactor (int src_blend_factor, int dst_blend_factor);
int  d3d9_AlphaTestingAvailable ();
void d3d9_EnableAlphaTesting (int flag, byte reference_value);

void d3d9_SetCursorInfo (
  void (*lock_cursor) (),
  void (*unlock_cursor) (), 
  void (*invalidate_cursor_background) () );

void d3d9_GetRGBFormat (
  unsigned *redmask, 
  unsigned *greenmask, 
  unsigned *bluemask,
  int      *low_redbit,
  int      *low_greenbit,
  int      *low_bluebit,
  int      *num_redbits,
  int      *num_greenbits,
  int      *num_bluebits );
int  d3d9_SetActivePage (byte *surface, int page_is_a_texture);
void d3d9_FlipVisualPage ();
