/*____________________________________________________________________
|
| File: dx9.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _DX9_H_
#define _DX9_H_

// screen resolution - same constants as in gx_w7.h
#define dx9_RESOLUTION_640x480    ((unsigned)0x1)       // 4:3 unless otherwise noted
#define dx9_RESOLUTION_800x600    ((unsigned)0x2)
#define dx9_RESOLUTION_1024x768   ((unsigned)0x4)
#define dx9_RESOLUTION_1152x864   ((unsigned)0x8)
#define dx9_RESOLUTION_1280x960   ((unsigned)0x10)
#define dx9_RESOLUTION_1280x1024  ((unsigned)0x20)      // 5:4
#define dx9_RESOLUTION_1400x1050  ((unsigned)0x40)
#define dx9_RESOLUTION_1440x1080  ((unsigned)0x80)
#define dx9_RESOLUTION_1600x1200  ((unsigned)0x100)
#define dx9_RESOLUTION_1152x720   ((unsigned)0x200)     // widescreen 8:5
#define dx9_RESOLUTION_1280x800   ((unsigned)0x400)     // widescreen 8:5
#define dx9_RESOLUTION_1440x900   ((unsigned)0x800)     // widescreen 8:5
#define dx9_RESOLUTION_1680x1050  ((unsigned)0x1000)    // widescreen 8:5
#define dx9_RESOLUTION_1920x1200  ((unsigned)0x2000)    // widescreen 8:5
#define dx9_RESOLUTION_2048x1280  ((unsigned)0x4000)    // widescreen 8:5
#define dx9_RESOLUTION_1280x720   ((unsigned)0x8000)    // widescreen 16:9
#define dx9_RESOLUTION_1600x900   ((unsigned)0x10000)   // widescreen 16:9
#define dx9_RESOLUTION_1920x1080  ((unsigned)0x20000)   // widescreen 16:9
#define dx9_RESOLUTION_2048x1152  ((unsigned)0x40000)   // widescreen 16:9
#define dx9_RESOLUTION_2560x1440  ((unsigned)0x80000)   // widescreen 16:9
#define dx9_RESOLUTION_2560x1600  ((unsigned)0x100000)  // widescreen 16:10

// screen bit depth - same constants as in gx_w7.h
#define dx9_BITDEPTH_16           ((unsigned)0x1)
//#define dx9_BITDEPTH_24           ((unsigned)0x2)   // not supported
#define dx9_BITDEPTH_32           ((unsigned)0x4)

// logic ops - same constants as in gx_w7.h
#define dx9_SET         0    
#define dx9_AND         1   
#define dx9_OR          2
#define dx9_XOR         3
#define dx9_ADD         4
#define dx9_SUBTRACT    5
#define dx9_OP_SHL      6
#define dx9_OP_SHR      7
#define dx9_OP_MULTIPLY 8

// texture addressing modes - same constants as in gx_w7.h
#define dx9_TEXTURE_ADDRESSMODE_WRAP        1
#define dx9_TEXTURE_ADDRESSMODE_MIRROR      2
#define dx9_TEXTURE_ADDRESSMODE_CLAMP       3
#define dx9_TEXTURE_ADDRESSMODE_BORDER      4
#define dx9_TEXTURE_ADDRESSMODE_MIRRORONCE  5

// texture dimensions - same constants as in gx_w7.h
#define dx9_TEXTURE_DIMENSION_U 0x1
#define dx9_TEXTURE_DIMENSION_V 0x2
#define dx9_TEXTURE_DIMENSION_W 0x4

// texture filtering types - same constants as in gx_w7.h
#define dx9_TEXTURE_FILTERTYPE_POINT       1
#define dx9_TEXTURE_FILTERTYPE_LINEAR      2
#define dx9_TEXTURE_FILTERTYPE_ANISOTROPIC 3

// fill modes - same constants as in gx_w7.h
#define dx9_FILL_MODE_POINT           1
#define dx9_FILL_MODE_WIREFRAME       2
#define dx9_FILL_MODE_SMOOTH_SHADED   3
#define dx9_FILL_MODE_GOURAUD_SHADED  4  

// Alpha-blending factors - same constants as in gx_w7.h
#define dx9_ALPHABLENDFACTOR_ZERO        1   // blend factor is (0,0,0,0)
#define dx9_ALPHABLENDFACTOR_ONE         2   // blend factor is (1,1,1,1)
#define dx9_ALPHABLENDFACTOR_SRCCOLOR    3   // blend factor is (s[r],s[g],s[b],s[a])
#define dx9_ALPHABLENDFACTOR_DSTCOLOR    4   // blend factor is (d[r],d[g],d[b],d[a])
#define dx9_ALPHABLENDFACTOR_SRCALPHA    5   // blend factor is (s[a],s[a],s[a],s[a])
#define dx9_ALPHABLENDFACTOR_DSTALPHA    6   // blend factor is (d[a],d[a],d[a],d[a])
#define dx9_ALPHABLENDFACTOR_INVSRCCOLOR 7   // blend factor is (1-s[r],1-s[g],1-s[b],1-s[a])
#define dx9_ALPHABLENDFACTOR_INVDSTCOLOR 8   // blend factor is (1-d[r],1-d[g],1-d[b],1-d[a])
#define dx9_ALHPABLENDFACTOR_INVSRCALPHA 9   // blend factor is (1-s[a],1-s[a],1-s[a],1-s[a])
#define dx9_ALHPABLENDFACTOR_INVDSTALPHA 10  // blend factor is (1-d[a],1-d[a],1-d[a],1-d[a])
#define dx9_ALHPABLENDFACTOR_SRCALPHASAT 11  // blend factor is (f,f,f,1), where f = min(s[a], 1-d[a])

// Stencil operations - same constants as in gx_w7.h
#define dx9_STENCILOP_DECR    0x1
#define dx9_STENCILOP_DECRSAT 0x2
#define dx9_STENCILOP_INCR    0x4
#define dx9_STENCILOP_INCRSAT 0x8
#define dx9_STENCILOP_INVERT  0x10
#define dx9_STENCILOP_KEEP    0x20
#define dx9_STENCILOP_REPLACE 0x40
#define dx9_STENCILOP_ZERO    0x80

// Stencil comparison functions - same constans as in gx_w7.h
#define dx9_STENCILFUNC_NEVER         1
#define dx9_STENCILFUNC_LESS          2
#define dx9_STENCILFUNC_EQUAL         3
#define dx9_STENCILFUNC_LESSEQUAL     4
#define dx9_STENCILFUNC_GREATER       5
#define dx9_STENCILFUNC_NOTEQUAL      6
#define dx9_STENCILFUNC_GREATEREQUAL  7
#define dx9_STENCILFUNC_ALWAYS        8

/*___________________
|
| Texture color blending operations - same constants as in gx_w7.h
|__________________*/

// Disables output from this texture stage and all stages with a higher index. To disable texture 
//  mapping, set this as the color operation for the first texture stage (stage 0). Alpha operations 
//  cannot be disabled when color operations are enabled. Setting the alpha operation to DISABLE 
//  when color blending is enabled causes undefined behavior. 
#define dx9_TEXTURE_COLOROP_DISABLE                   0

// Use this texture stage's first color or alpha argument, unmodified, as the output. This operation 
//  affects the color argument when used with the COLOROP texture-stage state, and the alpha argument
//  when used with ALPHAOP. 
//  S(RGBA) = ARG1   
#define dx9_TEXTURE_COLOROP_SELECTARG1                1

// Use this texture stage's second color or alpha argument, unmodified, as the output. This operation 
//  affects the color argument when used with the COLOROP texture stage state, and the alpha argument 
//  when used with ALPHAOP. 
//  S(RGBA) = ARG2
#define dx9_TEXTURE_COLOROP_SELECTARG2                2

// Multiply the components of the arguments together
//  S(RGBA) = ARG1 * ARG2
#define dx9_TEXTURE_COLOROP_MODULATE                  3

// Multiply the components of the arguments, and shift the products to the left 1 bit (effectively 
//  multiplying them by 2) for brightening. 
//  S(RGBA) = (ARG1 * ARG2) << 1
#define dx9_TEXTURE_COLOROP_MODULATE2X                4

// Multiply the components of the arguments, and shift the products to the left 2 bits (effectively 
//  multiplying them by 4) for brightening. 
//  S(RGBA) = (ARG1 * ARG2) << 2
#define dx9_TEXTURE_COLOROP_MODULATE4X                5

// Add the components of the arguments
//  S(RGBA) = ARG1 + ARG2
#define dx9_TEXTURE_COLOROP_ADD                       6

// Add the components of the arguments with a -0.5 bias, making the effective range from -0.5 to 0.5
//  S(RGBA) = ARG1 + ARG2
#define dx9_TEXTURE_COLOROP_ADDSIGNED                 7

// Add the components with a -0.5 bias and shift left 1 bit 
//  S(RGBA) = ARG1 + ARG2 - 0.5
#define dx9_TEXTURE_COLOROP_ADDSIGNED2X               8

// Subtract the components of the second arg from the first
//  S(RGBA) = ARG1 - ARG2
#define dx9_TEXTURE_COLOROP_SUBTRACT                  9

// Add the first and second args, then subtract their product from their sum
//  S(RGBA) = ARG1 + ARG2 - ARG1 * ARG2
//          = ARG1 + ARG2 * (1 - ARG1)
#define dx9_TEXTURE_COLOROP_ADDSMOOTH                 10

// Linearly blend this stage using the interpolated alpha from each vertex
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define dx9_TEXTURE_COLOROP_BLENDDIFFUSEALPHA         11

// Linearly blend this stage using the interpolated alpha from this textures stage
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define dx9_TEXTURE_COLOROP_BLENDTEXTUREALPHA         12

// Linearly blend this stage using the interpolated alpha from a scalar alpha
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define dx9_TEXTURE_COLOROP_BLENDFACTORALPHA          13

// Linearly blend a texture stage that uses a premultiplied alpha
//  S(RGBA) = ARG1 + ARG2 * (1 - Alpha)
#define dx9_TEXTURE_COLOROP_BLENDTEXTUREALPHAPM       14

// Linearly blend this stage using the interpolated alpha from the previous texture stage
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define dx9_TEXTURE_COLOROP_BLENDCURRENTALPHA         15         

// Modulate this texture stage with the next texture stage
#define dx9_TEXTURE_COLOROP_PREMODULATE               16  

// Modulate the color of the second arg, using the alpha of the first arg, then add result to arg1
//  S(RGBA) = ARG1(RGB) + ARG1(A) * ARG2(RGB)
#define dx9_TEXTURE_COLOROP_MODULATEALPHA_ADDCOLOR    17

// Modulate the args, then add the alpha of arg1
//  S(RGBA) = ARG1(RGB) * ARG2(RGB) + ARG1(A)
#define dx9_TEXTURE_COLOROP_MODULATECOLOR_ADDALPHA    18

// Similar to MODULATEALPHA_ADDCOLOR but use the inverse of the alpha of arg1
//  S(RGBA) = (1 - ARG1(A)) * ARG2(RGB) + ARG1(RGB)
#define dx9_TEXTURE_COLOROP_MODULATEINVALPHA_ADDCOLOR 19

// Similar to MODULATEALPHA_ADDALPHA but use the inverse of the color of arg1
//  S(RGBA) = (1 - ARG1(RGB)) * ARG2(RGB) + ARG1(A)
#define dx9_TEXTURE_COLOROP_MODULATEINVCOLOR_ADDALPHA 20

// Perform per-pixel bump mapping, using the environment map in the next texture stage (w/out luminance)
#define dx9_TEXTURE_COLOROP_BUMPENVMAP                21

// Perform per-pixel bump mapping, using the environment map in the next texture stage (with luminance)
#define dx9_TEXTURE_COLOROP_BUMPENVMAPLUMINANCE       22

// Modulate the components of each arg (as signed), add their products, then replicate the sum to 
//  all color channels, including alpha
//  S(RGBA) = ARG1(R) * ARG2(R) + ARG1(G) * ARG2(G) + ARG1(B) * ARG2(B)
#define dx9_TEXTURE_COLOROP_DOTPRODUCT3               23

// Performs a multiply-accumulate operation.  It takes the last two arguments, multiplies them together,
//  and adds them to the remaining input/source argument, and places that into the result.
//  S(RGBA) = ARG1 + ARG2 * ARG3
#define dx9_TEXTURE_COLOROP_MULTIPLYADD               24

// Linearly interpolates between the 2nd and 3rd source arguments by a proportion specified in the 1st
//  source argument.
//  S(RGBA) = (ARG1) * ARG2 + (1-ARG1) * ARG3
#define dx9_TEXTURE_COLOROP_LERP                      25

/*___________________
|
| Texture alpha blending operations - same constants as in gx_w7.h
|__________________*/

// Disables output from this texture stage and all stages with a higher index. To disable texture 
//  mapping, set this as the color operation for the first texture stage (stage 0). Alpha operations 
//  cannot be disabled when color operations are enabled. Setting the alpha operation to DISABLE 
//  when color blending is enabled causes undefined behavior. 
#define dx9_TEXTURE_ALPHAOP_DISABLE                   0

// Use this texture stage's first color or alpha argument, unmodified, as the output. This operation 
//  affects the color argument when used with the COLOROP texture-stage state, and the alpha argument
//  when used with ALPHAOP. 
//  S(RGBA) = ARG1   
#define dx9_TEXTURE_ALPHAOP_SELECTARG1                1

// Use this texture stage's second color or alpha argument, unmodified, as the output. This operation 
//  affects the color argument when used with the COLOROP texture stage state, and the alpha argument 
//  when used with ALPHAOP. 
//  S(RGBA) = ARG2
#define dx9_TEXTURE_ALPHAOP_SELECTARG2                2

// Multiply the components of the arguments together
//  S(RGBA) = ARG1 * ARG2
#define dx9_TEXTURE_ALPHAOP_MODULATE                  3

// Multiply the components of the arguments, and shift the products to the left 1 bit (effectively 
//  multiplying them by 2) for brightening. 
//  S(RGBA) = (ARG1 * ARG2) << 1
#define dx9_TEXTURE_ALPHAOP_MODULATE2X                4

// Multiply the components of the arguments, and shift the products to the left 2 bits (effectively 
//  multiplying them by 4) for brightening. 
//  S(RGBA) = (ARG1 * ARG2) << 2
#define dx9_TEXTURE_ALPHAOP_MODULATE4X                5

// Add the components of the arguments
//  S(RGBA) = ARG1 + ARG2
#define dx9_TEXTURE_ALPHAOP_ADD                       6

// Add the components of the arguments with a -0.5 bias, making the effective range from -0.5 to 0.5
//  S(RGBA) = ARG1 + ARG2
#define dx9_TEXTURE_ALPHAOP_ADDSIGNED                 7

// Add the components with a -0.5 bias and shift left 1 bit 
//  S(RGBA) = ARG1 + ARG2 - 0.5
#define dx9_TEXTURE_ALPHAOP_ADDSIGNED2X               8

// Subtract the components of the second arg from the first
//  S(RGBA) = ARG1 - ARG2
#define dx9_TEXTURE_ALPHAOP_SUBTRACT                  9

// Add the first and second args, then subtract their product from their sum
//  S(RGBA) = ARG1 + ARG2 - ARG1 * ARG2
//          = ARG1 + ARG2 * (1 - ARG1)
#define dx9_TEXTURE_ALPHAOP_ADDSMOOTH                 10

// Linearly blend this stage using the interpolated alpha from each vertex
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define dx9_TEXTURE_ALPHAOP_BLENDDIFFUSEALPHA         11

// Linearly blend this stage using the interpolated alpha from this textures stage
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define dx9_TEXTURE_ALPHAOP_BLENDTEXTUREALPHA         12

// Linearly blend this stage using the interpolated alpha from a scalar alpha
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define dx9_TEXTURE_ALPHAOP_BLENDFACTORALPHA          13

// Linearly blend a texture stage that uses a premultiplied alpha
//  S(RGBA) = ARG1 + ARG2 * (1 - Alpha)
#define dx9_TEXTURE_ALPHAOP_BLENDTEXTUREALPHAPM       14

// Linearly blend this stage using the interpolated alpha from the previous texture stage
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define dx9_TEXTURE_ALPHAOP_BLENDCURRENTALPHA         15         

// Modulate this texture stage with the next texture stage
#define dx9_TEXTURE_ALPHAOP_PREMODULATE               16  

// Modulate the components of each arg (as signed), add their products, then replicate the sum to 
//  all color channels, including alpha
//  S(RGBA) = ARG1(R) * ARG2(R) + ARG1(G) * ARG2(G) + ARG1(B) * ARG2(B)
#define dx9_TEXTURE_ALPHAOP_DOTPRODUCT3               17

// Performs a multiply-accumulate operation.  It takes the last two arguments, multiplies them together,
//  and adds them to the remaining input/source argument, and places that into the result.
//  S(RGBA) = ARG1 + ARG2 * ARG3
#define dx9_TEXTURE_ALPHAOP_MULTIPLYADD               18

// Linearly interpolates between the 2nd and 3rd source arguments by a proportion specified in the 1st
//  source argument.
//  S(RGBA) = (ARG1) * ARG2 + (1-ARG1) * ARG3
#define dx9_TEXTURE_ALPHAOP_LERP                      19

/*___________________
|
| Texture stage blending arguments - same constants as in gx_w7.h
|__________________*/

// The texture argument is the result of the previous blending stage. In the first texture stage 
//  (stage 0), this argument is equivalent to D3DTA_DIFFUSE. If the previous blending stage uses 
//  a bump-map texture (the D3DTOP_BUMPENVMAP operation), the system chooses the texture from the 
//  stage before the bump-map texture. (If s represents the current texture stage and s - 1 contains 
//  a bump-map texture, this argument becomes the result output by texture stage s - 2.) 
#define dx9_TEXTURE_ARG_CURRENT  0

// The texture argument is the diffuse color interpolated from vertex components during Gouraud shading. 
//  If the vertex does not contain a diffuse color, the default color is 0xFFFFFFFF. 
#define dx9_TEXTURE_ARG_DIFFUSE  1

// The texture argument is the texture color for this texture stage. This is valid only for the 
//  first color and alpha arguments in a stage (the color ARG1 and alpha ARG1)
#define dx9_TEXTURE_ARG_TEXTURE  2

// The texture argument is the texture factor set in a previous call to dx9_SetTextureColorFactor()
#define dx9_TEXTURE_ARG_TFACTOR  3

// The texture argument is the specular color interpolated from vertex components during Gouraud 
//  shading. If the vertex does not contain a diffuse color, the default color is 0xFFFFFFFF. 
#define dx9_TEXTURE_ARG_SPECULAR 4

/*___________________
|
| 2D functions
|__________________*/

int  dx9_GetUserFormat (
  unsigned  acceptable_resolutions, // bit mask of acceptable resolutions
  unsigned  acceptable_bitdepths,   // bit mask of acceptable bitdepths
  unsigned *selected_resolution,
  unsigned *selected_bitdepth,
  int       enable_hardware_acceleration );
int  dx9_Init (
  unsigned resolution, 
  unsigned bitdepth, 
  unsigned stencil_depth_requested,
  int      num_pages_requested, 
  int      enable_hardware_acceleration );
void dx9_Free (void);
void dx9_VertRetraceDelay (void);
int  dx9_RestoreDirectX (void);
void dx9_GetRGBFormat (
  unsigned *redmask, 
  unsigned *greenmask, 
  unsigned *bluemask,
  int      *low_redbit,
  int      *low_greenbit,
  int      *low_bluebit,
  int      *num_redbits,
  int      *num_greenbits,
  int      *num_bluebits );
int  dx9_SetActivePage (int page);
void dx9_FlipVisualPage (void);
void dx9_SetForeColor (byte r, byte g, byte b, byte a);
void dx9_SetLogicOp (int logic_op);
void dx9_DrawPixel (int x, int y);
void dx9_GetPixel (int x, int y, byte *r, byte *g, byte *b);
void dx9_DrawLine (int x1, int y1, int x2, int y2);
void dx9_DrawFillRectangle (int x1, int y1, int x2, int y2);
void dx9_PutImage (
  byte *image,
  int   image_dx,
  int   image_dy,
  int   image_x,
  int   image_y,
  int   x,
  int   y,
  int   dx,
  int   dy,
  int   or_image );	/* boolean */
void dx9_GetImage (
  byte *image,
  int   image_dx,
  int   image_dy,
  int   image_x,
  int   image_y,
  int   x,
  int   y,
  int   dx,
  int   dy );
void dx9_CopyImage (
	int srcx,	
	int srcy,	
	int src_pg, 
	int dstx, 
	int dsty, 
	int dst_pg,	
	int dx,	
	int dy );
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
  byte b );
void dx9_PutBitmap (
  byte *bitmap,
  int   bitmap_dx,
	int	  bitmap_dy,
  int   bitmap_x,
  int   bitmap_y,
  int   x,
  int   y,
  int   dx,
  int   dy,
  byte  r,
  byte  g,
  byte  b );
int  dx9_CreateVirtualPage (int dx, int dy, int create_in_vram);
void dx9_FreeVirtualPage (int page);

/*___________________
|
| 3D functions
|__________________*/

int  dx9_BeginRender (void);
int  dx9_EndRender (void);
void dx9_SetFillMode (int fill_mode);
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
  unsigned *max_vertex_index );
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
  float **texture_coord,          // 0-4 arrays of texture coords (group of 2 float - u,v)
  float **X_texture_coord,
  float **texture_coord_w,        // 0-4 arrays of texture w coords (1 float - w)
  float **X_texture_coord_w,
  byte   *weight,      
  byte  **X_weight,
  void  **driver_data );          // address of a pointer to driver-specific data
void dx9_UnregisterObject (void *driver_data);
void dx9_DrawObject (void *driver_data);
void dx9_OptimizeObject (void *driver_data);

int  dx9_SetViewport (int left, int top, int right, int bottom);
void dx9_ClearViewportRectangle (
  int      *rect, 
  unsigned  flags,  // 0x1=surface, 0x2=zbuffer, 0x4=stencil
  byte      r,
  byte      g,
  byte      b,
  byte      a,
  float     zval,
  unsigned  stencilval );
void dx9_EnableClipping (int flag);
unsigned dx9_InitClipPlane (float a, float b, float c, float d);
void dx9_FreeClipPlane (unsigned plane);
void dx9_EnableClipPlane (unsigned plane, int flag);
int  dx9_SetWorldMatrix (void *m);
int  dx9_GetWorldMatrix (void *m);
int  dx9_SetViewMatrix (void *m);
int  dx9_GetViewMatrix (void *m);
int  dx9_SetProjectionMatrix (void *m);
int  dx9_GetProjectionMatrix (void *m);
int  dx9_EnableTextureMatrix (int stage, int dimension, int flag);
int  dx9_SetTextureMatrix (int stage, void *m);
int  dx9_GetTextureMatrix (int stage, void *m);

void dx9_EnableZBuffer (int flag);
void dx9_EnableBackfaceRemoval (int flag);

void dx9_EnableStencilBuffer (int flag);
void dx9_SetStencilFailOp (int stencil_op);
void dx9_SetStencilZFailOp (int stencil_op);
void dx9_SetStencilPassOp (int stencil_op);
void dx9_SetStencilComparison (int stencil_function);
void dx9_SetStencilReferenceValue (unsigned reference_value);
void dx9_SetStencilMask (unsigned mask);
void dx9_SetStencilWriteMask (unsigned mask);

void dx9_EnableLighting (int flag);
unsigned dx9_InitPointLight (
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
void dx9_UpdatePointLight (
  unsigned  light,
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
unsigned dx9_InitSpotLight (
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
void dx9_UpdateSpotLight (
  unsigned  light,
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
unsigned dx9_InitDirectionLight (
  float     dst_x,
  float     dst_y,
  float     dst_z, 
  float    *ambient_color_rgba,
  float    *diffuse_color_rgba,
  float    *specular_color_rgba );
void dx9_UpdateDirectionLight (
  unsigned  light,
  float     dst_x,
  float     dst_y,
  float     dst_z, 
  float    *ambient_color_rgba,
  float    *diffuse_color_rgba,
  float    *specular_color_rgba );
void dx9_FreeLight (unsigned light);
void dx9_EnableLight (unsigned light, int flag);
void dx9_SetAmbientLight (float *rgba);
void dx9_EnableSpecularLighting (int flag);
void dx9_EnableVertexLighting (int flag);

void dx9_EnableFog (int flag);
void dx9_SetFogColor (byte r, byte g, byte b);
void dx9_SetLinearPixelFog (float start_distance, float end_distance);
void dx9_SetExpPixelFog (float density);   // 0-1
void dx9_SetExp2PixelFog (float density);  // 0-1
void dx9_SetLinearVertexFog (float start_distance, float end_distance, int ranged_based);

void dx9_SetMaterial (
  float    *ambient_color_rgba,
  float    *diffuse_color_rgba,
  float    *specular_color_rgba,
  float    *emissive_color_rgba,
  float     specular_sharpness );
void dx9_GetMaterial (
  float    *ambient_color_rgba,
  float    *diffuse_color_rgba,
  float    *specular_color_rgba,
  float    *emissive_color_rgba,
  float    *specular_sharpness );

byte *dx9_InitTexture (
  int       num_mip_levels,
  byte    **image, 
  byte    **alphamap,
  int       dx, 
  int       dy, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size);
byte *dx9_InitVolumeTexture (
  int       num_levels,
  int       num_slices,
  byte    **image, 
  byte    **alphamap,
  int       dx, 
  int       dy, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size);
byte *dx9_InitCubemapTexture (
  byte    **image, 
  byte    **alphamap,
  int       dimensions, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size );
unsigned dx9_InitDynamicTexture (
  int       dx, 
  int       dy, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size );
unsigned dx9_InitDynamicCubemapTexture (
  int       dimensions, 
  int       num_color_bits,
  int       num_alpha_bits,
  unsigned *size );
void dx9_FreeTexture (byte *texture);
void dx9_FreeDynamicTexture (unsigned texture);
void dx9_SetTexture (int stage, byte *texture);
void dx9_SetDynamicTexture (int stage, unsigned texture);
void dx9_SetTextureAddressingMode (int stage, int dimension, int addressing_mode);
void dx9_SetTextureBorderColor (int stage, byte r, byte g, byte b, byte a);
void dx9_SetTextureFiltering (
  int stage, 
  int filter_type, 
  int anisotropy_level ); // amount of filtering (1-100), 1=min, 100=max
void dx9_SetTextureCoordinates (
  int stage,              // texture blending stage 0-7
  int coordinate_stage ); // set of texture coordinates in the object 0-7 (-1=cubemap)
void dx9_SetTextureCoordinateWrapping (
  int coordinate_stage, // set of texture coordinates in the object 0-7
  int wrap_s,           // boolean (u dimension)
  int wrap_t,           // boolean (v dimension)
  int wrap_r,           // boolean
  int wrap_q );         // boolean
void dx9_SetTextureFactor (byte r, byte g, byte b, byte a);
void dx9_PreLoadTexture (byte *texture);
void dx9_EvictAllTextures (void);
void dx9_EnableRenderToTexture (unsigned texture, int face);
void dx9_SetTextureColorOp (int stage, int texture_colorop, int texture_arg1, int texture_arg2);
void dx9_SetTextureAlphaOp (int stage, int texture_alphaop, int texture_arg1, int texture_arg2);
void dx9_SetTextureColorFactor (float *rgba);
void dx9_EnableCubemapTextureReflections (int flag);

void dx9_EnableAlphaBlending (int flag);
void dx9_SetAlphaBlendFactor (int src_blend_factor, int dst_blend_factor);
int  dx9_AlphaTestingAvailable (void);
void dx9_EnableAlphaTesting (int flag, byte reference_value);

/*___________________
|
| Event functions
|__________________*/

int  dx9_StartEvents (int use_keyboad, int use_mouse);
void dx9_StopEvents (void);
void dx9_FlushEvents (void);
int	 dx9_GetEvent (unsigned *type, int *keycode, int *x, int *y);

/*___________________
|
| Mouse functions
|__________________*/

void 	dx9_MouseFlushBuffer (void);
void 	dx9_MouseHide (void);
void 	dx9_MouseShow (void);
void 	dx9_MouseConfine (int left, int top, int right, int bottom);
int  	dx9_MouseGetStatus (int *x, int *y, int *button);
void 	dx9_MouseSetCoords (int x, int y);
void 	dx9_MouseGetCoords (int *x, int *y);
void 	dx9_MouseGetMovement (int *x, int *y);
void  dx9_MouseSetBitmapCursor (
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
void	dx9_MouseSetImageCursor (
	byte *image, 
	int		image_dx, 
	int		image_dy, 
	int		hot_x,                                  
	int		hot_y );

#endif
