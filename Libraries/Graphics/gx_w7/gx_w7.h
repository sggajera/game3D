/*____________________________________________________________________
|
| File: gx_w7.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _GX_W7_H_
#define _GX_W7_H_

/*___________________
|
| Constants
|__________________*/

// driver
#define gxDRIVER_DX9              9   // DirectX 9
#define gxDRIVER_DX9_SOFTWARE     109 // DirectX 9 with Software Rasterizer

// screen resolution
#define gxRESOLUTION_640x480    ((unsigned)0x1)     // 4:3 unless otherwise noted
#define gxRESOLUTION_800x600    ((unsigned)0x2)
#define gxRESOLUTION_1024x768   ((unsigned)0x4)
#define gxRESOLUTION_1152x864   ((unsigned)0x8)
#define gxRESOLUTION_1280x960   ((unsigned)0x10)
#define gxRESOLUTION_1280x1024  ((unsigned)0x20)    // 5:4
#define gxRESOLUTION_1400x1050  ((unsigned)0x40)
#define gxRESOLUTION_1440x1080  ((unsigned)0x80)
#define gxRESOLUTION_1600x1200  ((unsigned)0x100)
#define gxRESOLUTION_1152x720   ((unsigned)0x200)   // widescreen 8:5
#define gxRESOLUTION_1280x800   ((unsigned)0x400)   // widescreen 8:5
#define gxRESOLUTION_1440x900   ((unsigned)0x800)   // widescreen 8:5
#define gxRESOLUTION_1680x1050  ((unsigned)0x1000)  // widescreen 8:5
#define gxRESOLUTION_1920x1200  ((unsigned)0x2000)  // widescreen 8:5
#define gxRESOLUTION_2048x1280  ((unsigned)0x4000)  // widescreen 8:5
#define gxRESOLUTION_1280x720   ((unsigned)0x8000)  // widescreen 16:9
#define gxRESOLUTION_1600x900   ((unsigned)0x10000) // widescreen 16:9
#define gxRESOLUTION_1920x1080  ((unsigned)0x20000) // widescreen 16:9
#define gxRESOLUTION_2048x1152  ((unsigned)0x40000) // widescreen 16:9
#define gxRESOLUTION_2560x1440  ((unsigned)0x80000) // widescreen 16:9
#define gxRESOLUTION_2560x1600  ((unsigned)0x100000) // widescreen 16:10

// screen bitdepth
#define gxBITDEPTH_16           ((unsigned)0x1)
#define gxBITDEPTH_24           ((unsigned)0x2) // not available with DX9 driver
#define gxBITDEPTH_32           ((unsigned)0x4)

// stencil bitdepth - 3D only
#define gxSTENCILDEPTH_1    1
#define gxSTENCILDEPTH_8    8

// predefined colors 
#define gxBLACK             0
#define gxBLUE              1
#define gxGREEN             2
#define gxCYAN              3
#define gxRED               4
#define gxMAGENTA           5
#define gxBROWN             6
#define gxGRAY              7
#define gxDARKGRAY          8
#define gxLIGHTBLUE         9
#define gxLIGHTGREEN        10
#define gxLIGHTCYAN         11
#define gxLIGHTRED          12
#define gxLIGHTMAGENTA      13
#define gxYELLOW            14
#define gxWHITE             15

// line widths 
#define gxLINE_WIDTH_SQUARE_1     1
#define gxLINE_WIDTH_SQUARE_2     2
#define gxLINE_WIDTH_SQUARE_3     3
#define gxLINE_WIDTH_SQUARE_4     4
#define gxLINE_WIDTH_SQUARE_5     5
#define gxLINE_WIDTH_SQUARE_6     6
#define gxLINE_WIDTH_SQUARE_7     7
#define gxLINE_WIDTH_CIRCLE_3     8
#define gxLINE_WIDTH_CIRCLE_5     9
#define gxLINE_WIDTH_CIRCLE_7     10
#define gxLINE_WIDTH_CIRCLE_9     11
#define gxLINE_WIDTH_CIRCLE_11    12
#define gxLINE_WIDTH_VERTICAL_2   13
#define gxLINE_WIDTH_VERTICAL_3   14
#define gxLINE_WIDTH_HORIZONTAL_2 15
#define gxLINE_WIDTH_HORIZONTAL_3 16
#define gxLINE_WIDTH_SPRAY_3      17
#define gxLINE_WIDTH_SPRAY_5      18

// predefined patterns  
#define gxPATTERN_SOLID 0

// logic ops 
#define gxSET       0  
#define gxAND       1  
#define gxOR        2
#define gxXOR       3
#define gxADD       4
#define gxSUBTRACT  5
#define gxSHL       6
#define gxSHR       7
#define gxMULTIPLY  8

// font types 
#define gxFONT_TYPE_GX         1  // files w/ .GXFONT extension       
#define gxFONT_TYPE_GEM        2
#define gxFONT_TYPE_METAWINDOW 3

// font spacing 
#define gxFONT_SPACING_FIXED        0
#define gxFONT_SPACING_PROPORTIONAL 1

// uniform palette spread 
#define gxPALETTE_SPREAD_LOW  5
#define gxPALETTE_SPREAD_HIGH 6

// special effects 
#define gxFADE_IN 0

// hints
#define gxHINT_CREATE_IN_SYSTEM_MEMORY  0x1
#define gxHINT_DONT_LET_DRIVER_MANAGE		0x2

// backbuffer macro - usage: gxSetActivePage (gxBACKBUFFER);
#define gxBACKBUFFER  ((gxGetVisualPage() + 1) % gxGetNumVRAMPages())

// Results (usually returned from testing functions)
enum gxRelation {
  gxRELATION_OUTSIDE,
  gxRELATION_INTERSECT,
  gxRELATION_INSIDE,
  gxRELATION_FRONT,
  gxRELATION_BACK,
  gxRELATION_EQUAL,
  gxRELATION_NOT_EQUAL,
  gxRELATION_PARALLEL,
  gxRELATION_PARALLEL_FRONT,
  gxRELATION_PARALLEL_BACK
};

// predefined mouse cursors 
/*
#define gxCURSOR_ARROW          0
#define gxCURSOR_CHECK          1
#define gxCURSOR_CROSS          2
#define gxCURSOR_BOX            3
#define gxCURSOR_FINGER         4
#define gxCURSOR_HAND           5
#define gxCURSOR_QUESTION       6
#define gxCURSOR_HOURGLASS      7
*/

// event types 
/*
#define gxEVENT_MOUSE_MOVE      0x01
#define gxEVENT_MOUSE_PRESS     0x02
#define gxEVENT_MOUSE_RELEASE   0x04
#define gxEVENT_KEY_PRESS       0x08
#define gxEVENT_KEY_RELEASE     0x10
*/

// String lengths
const int gx_ASCIIZ_STRING_LENGTH_SHORT = 16;
const int gx_ASCIIZ_STRING_LENGTH_LONG  = 32;

// 3D viewport clear operations
#define gx3d_CLEAR_SURFACE  0x1
#define gx3d_CLEAR_ZBUFFER  0x2
#define gx3d_CLEAR_STENCIL  0x4

// 3D lights
#define gx3d_LIGHT_TYPE_POINT     1
#define gx3d_LIGHT_TYPE_SPOT      2
#define gx3d_LIGHT_TYPE_DIRECTION 3

// texture addressing modes
#define gx3d_TEXTURE_ADDRESSMODE_WRAP   1
#define gx3d_TEXTURE_ADDRESSMODE_MIRROR 2
#define gx3d_TEXTURE_ADDRESSMODE_CLAMP  3
#define gx3d_TEXTURE_ADDRESSMODE_BORDER 4

// texture dimensions
#define gx3d_TEXTURE_DIMENSION_U  0x1
#define gx3d_TEXTURE_DIMENSION_V  0x2
#define gx3d_TEXTURE_DIMENSION_W  0x4

// texture filtering types
#define gx3d_TEXTURE_FILTERTYPE_POINT       1
#define gx3d_TEXTURE_FILTERTYPE_LINEAR      2
#define gx3d_TEXTURE_FILTERTYPE_TRILINEAR   3  
#define gx3d_TEXTURE_FILTERTYPE_ANISOTROPIC 4

// fill modes
#define gx3d_FILL_MODE_POINT          1
#define gx3d_FILL_MODE_WIREFRAME      2
#define gx3d_FILL_MODE_SMOOTH_SHADED  3
#define gx3d_FILL_MODE_GOURAUD_SHADED 4  

// camera orientation
#define gx3d_CAMERA_ORIENTATION_LOOKFROM_FIXED  1
#define gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED    2

// vertex format flags
#define gx3d_VERTEXFORMAT_WEIGHTS       0x1   // vertex data includes blending weight #1
#define gx3d_VERTEXFORMAT_DIFFUSE       0x2   // vertex data includes diffuse color
#define gx3d_VERTEXFORMAT_SPECULAR      0x4   // vertex data includes specular color
#define gx3d_VERTEXFORMAT_TEXCOORDS     0x8   // vertex data includes texcoords (0-8 sets)
#define gx3d_VERTEXFORMAT_MORPHS        0x10  // vertex data includes morphs (1-? sets)
//#define gx3d_VERTEXFORMAT_CUBEMAPCOORDS 0x10  // vertex data includes 1 set of cubemap (3D) coords
// default is NO diffuse, NO specular, 0-8 sets of texcoords
#define gx3d_VERTEXFORMAT_DEFAULT   (gx3d_VERTEXFORMAT_TEXCOORDS)
#define gx3d_VERTEXFORMAT_ALL           (0x1 | 0x2 | 0x4 | 0x8 | 0x10)

// misc 3D stuff
#define gx3d_MAX_ZBUFFER_VALUE  (1.0)
#define gx3d_NUM_TEXTURE_STAGES 8
#define gx3d_MAX_VERTEX_WEIGHTS 4
//#define gx3d_CUBEMAP_COORDS -1

// misc 3D flags
#define gx3d_DONT_COMBINE_LAYERS            0x1
#define gx3d_DONT_SET_TEXTURES              0x2
#define gx3d_DONT_SET_LOCAL_MATRIX          0x4
#define gx3d_SMOOTH_DISCONTINUOUS_VERTICES  0x8   // used by gx3d_ReadLWO2File(), gx3d_ComputeVertexNormals()
#define gx3d_DONT_GENERATE_MIPMAPS          0x10  // used by gx3d_InitTexture_File(), gx3d_InitParticleSystem(), gx3d_ReadLWOFile()
#define gx3d_MERGE_DUPLICATE_VERTICES       0x20  // used by gx3d_ReadLWO2File() - an n-squared algorithm - not good for large models!
#define gx3d_DONT_LOAD_TEXTURES             0x40  // used by gx3d_ReadLWO2File() - won't load texture files, just texcoords

// Alpha-blending factors, pixel_color = (src_pixel * src_blend_factor) + (dst_pixel * dst_blend_factor)
#define gx3d_ALPHABLENDFACTOR_ZERO        1   // blend factor is (0,0,0,0)
#define gx3d_ALPHABLENDFACTOR_ONE         2   // blend factor is (1,1,1,1)
#define gx3d_ALPHABLENDFACTOR_SRCCOLOR    3   // blend factor is (s[r],s[g],s[b],s[a])
#define gx3d_ALPHABLENDFACTOR_DSTCOLOR    4   // blend factor is (d[r],d[g],d[b],d[a])
#define gx3d_ALPHABLENDFACTOR_SRCALPHA    5   // blend factor is (s[a],s[a],s[a],s[a])
#define gx3d_ALPHABLENDFACTOR_DSTALPHA    6   // blend factor is (d[a],d[a],d[a],d[a])
#define gx3d_ALPHABLENDFACTOR_INVSRCCOLOR 7   // blend factor is (1-s[r],1-s[g],1-s[b],1-s[a])
#define gx3d_ALPHABLENDFACTOR_INVDSTCOLOR 8   // blend factor is (1-d[r],1-d[g],1-d[b],1-d[a])
#define gx3d_ALPHABLENDFACTOR_INVSRCALPHA 9   // blend factor is (1-s[a],1-s[a],1-s[a],1-s[a])
#define gx3d_ALPHABLENDFACTOR_INVDSTALPHA 10  // blend factor is (1-d[a],1-d[a],1-d[a],1-d[a])
#define gx3d_ALPHABLENDFACTOR_SRCALPHASAT 11  // blend factor is (f,f,f,1), where f = min(s[a], 1-d[a])

// Stencil operations
#define gx3d_STENCILOP_DECR     0x1
#define gx3d_STENCILOP_DECRSAT  0x2
#define gx3d_STENCILOP_INCR     0x4
#define gx3d_STENCILOP_INCRSAT  0x8
#define gx3d_STENCILOP_INVERT   0x10
#define gx3d_STENCILOP_KEEP     0x20
#define gx3d_STENCILOP_REPLACE  0x40
#define gx3d_STENCILOP_ZERO     0x80
                                
// Stencil comparison functions
#define gx3d_STENCILFUNC_NEVER        1
#define gx3d_STENCILFUNC_LESS         2
#define gx3d_STENCILFUNC_EQUAL        3
#define gx3d_STENCILFUNC_LESSEQUAL    4
#define gx3d_STENCILFUNC_GREATER      5
#define gx3d_STENCILFUNC_NOTEQUAL     6
#define gx3d_STENCILFUNC_GREATEREQUAL 7
#define gx3d_STENCILFUNC_ALWAYS       8

// Major coordinate axis
#define gx3d_X_AXIS 0x1
#define gx3d_Y_AXIS 0x2
#define gx3d_Z_AXIS 0x4

/*___________________
|
| Particle systems constants
|__________________*/

// Particle system emitter types
#define gx3d_PARTICLESYSTEM_EMITTER_TYPE_POINT      1 // a point at the origin
#define gx3d_PARTICLESYSTEM_EMITTER_TYPE_RECTANGLE  2 // a rectangle centered at the origin of the xz plane
#define gx3d_PARTICLESYSTEM_EMITTER_TYPE_CIRCLE     3 // a circle centered at the origin of the xz plane
#define gx3d_PARTICLESYSTEM_EMITTER_TYPE_CUBE       4 // a cube centered at the origin
#define gx3d_PARTICLESYSTEM_EMITTER_TYPE_SPHERE     5 // a sphere centered at the origin
#define gx3d_PARTICLESYSTEM_EMITTER_TYPE_CONE       6 // a cone centered at the origin and pointing up

// Particle system direction type
#define gx3d_PARTICLESYSTEM_DIRECTION_TYPE_FIXED    1 // fixed direction
#define gx3d_PARTICLESYSTEM_DIRECTION_TYPE_RANDOM   2 // completely random direction for starting value
#define gx3d_PARTICLESYSTEM_DIRECTION_TYPE_USER     3 // defined by a user function

// Particle system velocity type
#define gx3d_PARTICLESYSTEM_VELOCITY_TYPE_FIXED     1 // fixed velocity
#define gx3d_PARTICLESYSTEM_VELOCITY_TYPE_USER      2 // defined by a user function

// Particle system transparency type
#define gx3d_PARTICLESYSTEM_TRANSPARENCY_TYPE_FIXED 1 // fixed transparency
#define gx3d_PARTICLESYSTEM_TRANSPARENCY_TYPE_FADE  2 // fade from start transparency to zero over lifetime
#define gx3d_PARTICLESYSTEM_TRANSPARENCY_TYPE_USER  3 // defined by a user function

// Particle system size type
#define gx3d_PARTICLESYSTEM_SIZE_TYPE_FIXED             1
#define gx3d_PARTICLESYSTEM_SIZE_TYPE_TIME_VARIABLE     2 // size varies over max lifetime of a particle
#define gx3d_PARTICLESYSTEM_SIZE_TYPE_LIFETIME_VARIABLE 3 // size varies over lifetime of individual particle
#define gx3d_PARTICLESYSTEM_SIZE_TYPE_USER              4 // defined by a user function

/*___________________
|
| Texture color blending operations
|__________________*/

// Disables output from this texture stage and all stages with a higher index. To disable texture 
//  mapping, set this as the color operation for the first texture stage (stage 0). Alpha operations 
//  cannot be disabled when color operations are enabled. Setting the alpha operation to DISABLE 
//  when color blending is enabled causes undefined behavior. 
#define gx3d_TEXTURE_COLOROP_DISABLE                    0

// Use this texture stage's first color or alpha argument, unmodified, as the output. This operation 
//  affects the color argument when used with the COLOROP texture-stage state, and the alpha argument
//  when used with ALPHAOP. 
//  S(RGBA) = ARG1   
#define gx3d_TEXTURE_COLOROP_SELECTARG1                 1

// Use this texture stage's second color or alpha argument, unmodified, as the output. This operation 
//  affects the color argument when used with the COLOROP texture stage state, and the alpha argument 
//  when used with ALPHAOP. 
//  S(RGBA) = ARG2
#define gx3d_TEXTURE_COLOROP_SELECTARG2                 2

// Multiply the components of the arguments together
//  S(RGBA) = ARG1 * ARG2
#define gx3d_TEXTURE_COLOROP_MODULATE                   3

// Multiply the components of the arguments, and shift the products to the left 1 bit (effectively 
//  multiplying them by 2) for brightening. 
//  S(RGBA) = (ARG1 * ARG2) << 1
#define gx3d_TEXTURE_COLOROP_MODULATE2X                 4

// Multiply the components of the arguments, and shift the products to the left 2 bits (effectively 
//  multiplying them by 4) for brightening. 
//  S(RGBA) = (ARG1 * ARG2) << 2
#define gx3d_TEXTURE_COLOROP_MODULATE4X                 5

// Add the components of the arguments
//  S(RGBA) = ARG1 + ARG2
#define gx3d_TEXTURE_COLOROP_ADD                        6

// Add the components of the arguments with a -0.5 bias, making the effective range from -0.5 to 0.5
//  S(RGBA) = ARG1 + ARG2
#define gx3d_TEXTURE_COLOROP_ADDSIGNED                  7

// Add the components with a -0.5 bias and shift left 1 bit 
//  S(RGBA) = ARG1 + ARG2 - 0.5
#define gx3d_TEXTURE_COLOROP_ADDSIGNED2X                8

// Subtract the components of the second arg from the first
//  S(RGBA) = ARG1 - ARG2
#define gx3d_TEXTURE_COLOROP_SUBTRACT                   9

// Add the first and second args, then subtract their product from their sum
//  S(RGBA) = ARG1 + ARG2 - ARG1 * ARG2
//          = ARG1 + ARG2 * (1 - ARG1)
#define gx3d_TEXTURE_COLOROP_ADDSMOOTH                  10

// Linearly blend this stage using the interpolated alpha from each vertex
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define gx3d_TEXTURE_COLOROP_BLENDDIFFUSEALPHA          11

// Linearly blend this stage using the interpolated alpha from this textures stage
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define gx3d_TEXTURE_COLOROP_BLENDTEXTUREALPHA          12

// Linearly blend this stage using the interpolated alpha from a scalar alpha
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define gx3d_TEXTURE_COLOROP_BLENDFACTORALPHA           13

// Linearly blend a texture stage that uses a premultiplied alpha
//  S(RGBA) = ARG1 + ARG2 * (1 - Alpha)
#define gx3d_TEXTURE_COLOROP_BLENDTEXTUREALPHAPM        14

// Linearly blend this stage using the interpolated alpha from the previous texture stage
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define gx3d_TEXTURE_COLOROP_BLENDCURRENTALPHA          15         

// Modulate this texture stage with the next texture stage
#define gx3d_TEXTURE_COLOROP_PREMODULATE                16  

// Modulate the color of the second arg, using the alpha of the first arg, then add result to arg1
//  S(RGBA) = ARG1(RGB) + ARG1(A) * ARG2(RGB)
#define gx3d_TEXTURE_COLOROP_MODULATEALPHA_ADDCOLOR     17

// Modulate the args, then add the alpha of arg1
//  S(RGBA) = ARG1(RGB) * ARG2(RGB) + ARG1(A)
#define gx3d_TEXTURE_COLOROP_MODULATECOLOR_ADDALPHA     18

// Similar to MODULATEALPHA_ADDCOLOR but use the inverse of the alpha of arg1
//  S(RGBA) = (1 - ARG1(A)) * ARG2(RGB) + ARG1(RGB)
#define gx3d_TEXTURE_COLOROP_MODULATEINVALPHA_ADDCOLOR  19

// Similar to MODULATEALPHA_ADDALPHA but use the inverse of the color of arg1
//  S(RGBA) = (1 - ARG1(RGB)) * ARG2(RGB) + ARG1(A)
#define gx3d_TEXTURE_COLOROP_MODULATEINVCOLOR_ADDALPHA  20

// Perform per-pixel bump mapping, using the environment map in the next texture stage (w/out luminance)
#define gx3d_TEXTURE_COLOROP_BUMPENVMAP                 21

// Perform per-pixel bump mapping, using the environment map in the next texture stage (with luminance)
#define gx3d_TEXTURE_COLOROP_BUMPENVMAPLUMINANCE        22

// Modulate the components of each arg (as signed), add their products, then replicate the sum to 
//  all color channels, including alpha
//  S(RGBA) = ARG1(R) * ARG2(R) + ARG1(G) * ARG2(G) + ARG1(B) * ARG2(B)
#define gx3d_TEXTURE_COLOROP_DOTPRODUCT3                23

/*___________________
|
| Texture alpha blending operations
|__________________*/

// Disables output from this texture stage and all stages with a higher index. To disable texture 
//  mapping, set this as the color operation for the first texture stage (stage 0). Alpha operations 
//  cannot be disabled when color operations are enabled. Setting the alpha operation to DISABLE 
//  when color blending is enabled causes undefined behavior. 
#define gx3d_TEXTURE_ALPHAOP_DISABLE                    0

// Use this texture stage's first color or alpha argument, unmodified, as the output. This operation 
//  affects the color argument when used with the COLOROP texture-stage state, and the alpha argument
//  when used with ALPHAOP. 
//  S(RGBA) = ARG1   
#define gx3d_TEXTURE_ALPHAOP_SELECTARG1                 1

// Use this texture stage's second color or alpha argument, unmodified, as the output. This operation 
//  affects the color argument when used with the COLOROP texture stage state, and the alpha argument 
//  when used with ALPHAOP. 
//  S(RGBA) = ARG2
#define gx3d_TEXTURE_ALPHAOP_SELECTARG2                 2

// Multiply the components of the arguments together
//  S(RGBA) = ARG1 * ARG2
#define gx3d_TEXTURE_ALPHAOP_MODULATE                   3

// Multiply the components of the arguments, and shift the products to the left 1 bit (effectively 
//  multiplying them by 2) for brightening. 
//  S(RGBA) = (ARG1 * ARG2) << 1
#define gx3d_TEXTURE_ALPHAOP_MODULATE2X                 4

// Multiply the components of the arguments, and shift the products to the left 2 bits (effectively 
//  multiplying them by 4) for brightening. 
//  S(RGBA) = (ARG1 * ARG2) << 2
#define gx3d_TEXTURE_ALPHAOP_MODULATE4X                 5

// Add the components of the arguments
//  S(RGBA) = ARG1 + ARG2
#define gx3d_TEXTURE_ALPHAOP_ADD                        6

// Add the components of the arguments with a -0.5 bias, making the effective range from -0.5 to 0.5
//  S(RGBA) = ARG1 + ARG2
#define gx3d_TEXTURE_ALPHAOP_ADDSIGNED                  7

// Add the components with a -0.5 bias and shift left 1 bit 
//  S(RGBA) = ARG1 + ARG2 - 0.5
#define gx3d_TEXTURE_ALPHAOP_ADDSIGNED2X                8

// Subtract the components of the second arg from the first
//  S(RGBA) = ARG1 - ARG2
#define gx3d_TEXTURE_ALPHAOP_SUBTRACT                   9

// Add the first and second args, then subtract their product from their sum
//  S(RGBA) = ARG1 + ARG2 - ARG1 * ARG2
//          = ARG1 + ARG2 * (1 - ARG1)
#define gx3d_TEXTURE_ALPHAOP_ADDSMOOTH                  10

// Linearly blend this stage using the interpolated alpha from each vertex
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define gx3d_TEXTURE_ALPHAOP_BLENDDIFFUSEALPHA          11

// Linearly blend this stage using the interpolated alpha from this textures stage
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define gx3d_TEXTURE_ALPHAOP_BLENDTEXTUREALPHA          12

// Linearly blend this stage using the interpolated alpha from a scalar alpha
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define gx3d_TEXTURE_ALPHAOP_BLENDFACTORALPHA           13

// Linearly blend a texture stage that uses a premultiplied alpha
//  S(RGBA) = ARG1 + ARG2 * (1 - Alpha)
#define gx3d_TEXTURE_ALPHAOP_BLENDTEXTUREALPHAPM        14

// Linearly blend this stage using the interpolated alpha from the previous texture stage
//  S(RGBA) = ARG1 * (Alpha) + ARG2 * (1 - Alpha)
#define gx3d_TEXTURE_ALPHAOP_BLENDCURRENTALPHA          15         

// Modulate this texture stage with the next texture stage
#define gx3d_TEXTURE_ALPHAOP_PREMODULATE                16  

// Modulate the components of each arg (as signed), add their products, then replicate the sum to 
//  all color channels, including alpha
//  S(RGBA) = ARG1(R) * ARG2(R) + ARG1(G) * ARG2(G) + ARG1(B) * ARG2(B)
#define gx3d_TEXTURE_ALPHAOP_DOTPRODUCT3                17

/*___________________
|
| Texture stage blending arguments
|__________________*/

// The texture argument is the result of the previous blending stage. In the first texture stage 
//  (stage 0), this argument is equivalent to D3DTA_DIFFUSE. If the previous blending stage uses 
//  a bump-map texture (the D3DTOP_BUMPENVMAP operation), the system chooses the texture from the 
//  stage before the bump-map texture. (If s represents the current texture stage and s - 1 contains 
//  a bump-map texture, this argument becomes the result output by texture stage s - 2.) 
#define gx3d_TEXTURE_ARG_CURRENT  0

// The texture argument is the diffuse color interpolated from vertex components during Gouraud shading. 
//  If the vertex does not contain a diffuse color, the default color is 0xFFFFFFFF. 
#define gx3d_TEXTURE_ARG_DIFFUSE  1

// The texture argument is the texture color for this texture stage. This is valid only for the 
//  first color and alpha arguments in a stage (the color ARG1 and alpha ARG1)
#define gx3d_TEXTURE_ARG_TEXTURE  2

// The texture argument is the texture factor set in a previous call to gx3d_SetTextureColorFactor()
#define gx3d_TEXTURE_ARG_TFACTOR  3

// The texture argument is the specular color interpolated from vertex components during Gouraud 
//  shading. If the vertex does not contain a diffuse color, the default color is 0xFFFFFFFF. 
#define gx3d_TEXTURE_ARG_SPECULAR 4

/*___________________
|
| Data types
|__________________*/

typedef unsigned gxPage;
typedef int      gxPattern;

struct gxColor {
  union {
    // 8-bit color
    union {
      unsigned index;   // 2 ways of referring to this 4-bytes (index or rgba)
      unsigned rgba;
    };
    // 16,24,32-bit color
    struct {
      byte r, g, b, a;  // each has a value 0-255
    };  // rgba
  };
};

struct gxPointF {
  float x, y;
};

struct gxBound {
  int x,y,w,h;
};
																									  
struct gxLine { 
  int x1,y1,x2,y2; 
};

struct gxRectangle {
  int xleft, ytop, xright, ybottom;
};

struct gxFontHeader {           // 80-byte struct               
  byte  id;                     // version #                    
  char  fontname[32];           // font name                    
  short minch;                  // min ASCII character          
  short maxch;                  // max ASCII character          
  short ascent;
  short descent;
  short maxwidth;               // width of widest character    
  short fwidth;                 // width of buffer (in bytes)   
  short fheight;                // height of font buffer        
  char  reserved[33];
};

struct gxFont {
  gxFontHeader header;
  byte         type;               // GXFONT, GEM or MetaFONT, GXIMAGE
  byte         spacing;            // fixed or proportional             
  short        inter_char_spacing; // pixel spacing between characters, 0-? 
  short        space_char_width;   // pixel width for space character   
  word        *cotptr;             // character offset table            
  byte        *cwtptr;             // character width table             
  byte        *bufptr;             // character buffer                  
};

struct gxState {
  gxPage      active_page;
  gxRectangle win;
  gxRectangle clip;
  int         clipping;
  gxColor     color;
  int         line_width;
  int         line_style[4];
  int         logic_op;
  gxPattern   fill_pattern;
  gxFont     *font;
};

/*___________________
|
| 3D Data types
|__________________*/

struct gx3dDriverInfo {
  unsigned max_texture_dx;
  unsigned max_texture_dy;
  int      max_active_lights;
  int      max_user_clip_planes;
  int      max_simultaneous_texture_stages; // max simultaneous texture blending stages
  int      max_texture_stages;              // max texture blending stages
  int      max_texture_repeat;
  int      num_stencil_bits;
  unsigned stencil_ops; 
  int      max_vertex_blend_matrices;
  int      max_vertex_streams;
  unsigned max_vertex_index;
};

struct gx3dVector {
  float x, y, z;
};

struct gx3dVector4D {
  float x, y, z, w;
};

struct gx3dRectangle {
  float xleft, ytop, xright, ybottom;
};

struct gx3dLine {
  gx3dVector start, end;
};

struct gx3dRay {
  gx3dVector origin;
  gx3dVector direction; // normalized
};

struct gx3dPlane {
  gx3dVector n; // a, b, c make up the plane normal
  float      d; // di is the scalar part, where ax+by+cz-d=0;
};

// Standard trajectory - normalized direction vector and velocity
struct gx3dTrajectory {
  gx3dVector direction; // normalized
  float      velocity;
};

// Projected trajectory - a single vector encoding both direction and velocity information
struct gx3dProjectedTrajectory {
  gx3dVector direction; // direction normal * distance to project
};

struct gx3dMatrix {
  float _00, _01, _02, _03;
  float _10, _11, _12, _13;
  float _20, _21, _22, _23;
  float _30, _31, _32, _33;
};

struct gx3dQuaternion {
  float x, y, z, w;
};

struct gx3dCompressedQuaternion { // Compressed/decompressed using functions in QUANTIZE.CPP
  unsigned short x, y, z, w;   
};

typedef unsigned  gx3dClipPlane;
typedef unsigned  gx3dLight;
typedef void     *gx3dTexture;

struct gx3dColor {
  float r, g, b, a;     // typically use values 0-1
};

struct gx3dLightData {
  int light_type;
  union {
    // Point light
    struct {
      gx3dColor  diffuse_color;
      gx3dColor  specular_color;
      gx3dColor  ambient_color;
      gx3dVector src;
      float      range;
      float      constant_attenuation;
      float      linear_attenuation;
      float      quadratic_attenuation; 
    } point;
    // Spot light
    struct {
      gx3dColor  diffuse_color;
      gx3dColor  specular_color;
      gx3dColor  ambient_color;
      gx3dVector src;
      gx3dVector dst;
      float      range;
      float      falloff;
      float      constant_attenuation;
      float      linear_attenuation;
      float      quadratic_attenuation; 
      float      inner_cone_angle;      // 0 - outer_cone_angle
      float      outer_cone_angle;      // 0 - 180
    } spot;
    // Directional light
    struct {
      gx3dColor  diffuse_color;
      gx3dColor  specular_color;
      gx3dColor  ambient_color;
      gx3dVector dst;
    } direction;
  };
};

struct gx3dMaterialData {
	gx3dColor ambient_color;
	gx3dColor diffuse_color;
	gx3dColor specular_color;        
  gx3dColor emissive_color;
  float     specular_sharpness; // 0-1 (0=disabled)
};

enum gx3dTexCoordSet {
  gx3d_TEXCOORD_CUBEMAP = -1, // texture coords dynamically computed using camera reflection vector
  gx3d_TEXCOORD_SET0    =  0, // set of texture coords in object->layer->tex_coords[0]
  gx3d_TEXCOORD_SET1    =  1, // set of texture coords in object->layer->tex_coords[1]
  gx3d_TEXCOORD_SET2    =  2, // ...
  gx3d_TEXCOORD_SET3    =  3,
  gx3d_TEXCOORD_SET4    =  4,
  gx3d_TEXCOORD_SET5    =  5,
  gx3d_TEXCOORD_SET6    =  6,
  gx3d_TEXCOORD_SET7    =  7
};

struct gx3dBox {
  // bounding box coords are relative to local coordinate origin
  gx3dVector  min;    // min coord for bounding box (lower left nearest corner of box)
  gx3dVector  max;    // max coord for bounding box (upper right farthest corner of box)
};

struct gx3dSphere {
  gx3dVector  center; // relative to local coordinate origin
  float       radius; // radius of bounding sphere
};

struct gx3dFrustumOrientation {
  unsigned inside_near   : 1; // 1 = inside view Frustum, 0 = not inside (outside or possibly intersecting)
  unsigned inside_far    : 1;
  unsigned inside_left   : 1;
  unsigned inside_right  : 1;
  unsigned inside_top    : 1;
  unsigned inside_bottom : 1;
};

struct gx3dPolygon {
  word index[3];
};

struct gx3dUVCoordinate {
  float u, v;
};

struct gx3dVertexWeight {
  float value[gx3d_MAX_VERTEX_WEIGHTS];         // weights
  byte  matrix_index[gx3d_MAX_VERTEX_WEIGHTS];  // indexes into matrix_palette of up to 256 matrices (0-255)
  byte  num_weights;                            // 0 - (MAX_VERTEX_WEIGHTS-1)
};

struct gx3dTransform {
  bool       dirty;
  gx3dMatrix local_matrix;
  gx3dMatrix composite_matrix;
};

/*
typedef struct {
  int        dirty;   // boolean
  gx3dMatrix local_quaternion;
  gx3dMatrix composite_quaternion;
} gx3dBoneTransform;
*/

// Frustum
const int gx3d_NUM_FRUSTUM_PLANES = 6;

enum gx3dFrustumPlanes {
  gx3d_FRUSTUM_PLANE_NEAR   = 0,
  gx3d_FRUSTUM_PLANE_FAR    = 1,
  gx3d_FRUSTUM_PLANE_LEFT   = 2,
  gx3d_FRUSTUM_PLANE_RIGHT  = 3,
  gx3d_FRUSTUM_PLANE_TOP    = 4,
  gx3d_FRUSTUM_PLANE_BOTTOM = 5
};

struct gx3dViewFrustum {
  gx3dPlane     plane[gx3d_NUM_FRUSTUM_PLANES];
  gx3dRectangle view_plane; // in view space
};

// A view Frustum transformed into world space - useful for clipping AABB to a view Frustum
struct gx3dWorldFrustum {
  gx3dPlane     plane[gx3d_NUM_FRUSTUM_PLANES];
  // indexes (0-5) into the 6 floats of a gx3dBox - used to store the main diagonal of a AAB box most closely aligned with the plane normal
  struct {
    int minx, miny, minz, maxx, maxy, maxz;
  } box_diagonal [gx3d_NUM_FRUSTUM_PLANES];
};

// Matrix palette
struct gx3dPaletteMatrix {
  gx3dMatrix m;
  char      *weightmap_name;	// corresponding name of a weightmap used in the layer
};

/*___________________
|
| Particle systems format
|__________________*/

typedef void *gx3dParticleSystem;

// Emitter description - type and geometry, centered about its own local coordinate system
struct gx3dEmitter {
  int type;
  // Rectangle (dx, dz), Cube (dx, dy, dz)
  struct {
    float dx, dy, dz;
  };
  // Circle (on the xz plane), Sphere
  float radius;
  // Cone (points up)
  float height;
};

struct gx3dParticleSystemData {
  gx3dEmitter   emitter;          
  bool          attached_particles; // true = particles always attached to emitter even if emitter moves
  // particle direction
  int           direction_type;   
  gx3dVector    direction;      // start direction
  void        (*update_direction) (unsigned particle_age, gx3dVector *direction);  
  // particle velocity
  int           velocity_type;  
  float         min_velocity;   // in world units (typically feet) per second
  float         max_velocity; 
  void        (*update_velocity) (unsigned particle_age, float *velocity);
  // particle transparency
  int           transparency_type;
  float         start_transparency; // 0-1.0
  float         end_transparency;   // 0-1.0
  void        (*update_transparency) (unsigned particle_age, float *transparency);
  // particle size
  int           size_type;
  float         start_size;     // particle size in world units (all particles are square)
  float         end_size;
  void        (*update_size) (unsigned particle_age, float *size);
  // max particles / max particle lifetime in seconds = # particles emitted per second
  int           max_particles;
  float         min_lifespan;   // in seconds
  float         max_lifespan;   // in seconds
};

/*___________________
|
| gx3d Motion Skeleton format
|__________________*/

// One entry in a bone array
struct gx3dMotionSkeletonBone {             
  gx3dMatrix pre, post;
  char name[gx_ASCIIZ_STRING_LENGTH_LONG];  
  unsigned char parent;                     // 0xFF = root, else index into bone array
};

// Array of bones
struct gx3dMotionSkeleton {
  int                     num_bones;
  gx3dMotionSkeletonBone *bones;
  // used to create doubly-linked list
  gx3dMotionSkeleton     *next, *previous;
};

/*___________________
|
| gx3d bone pose formats
|__________________*/

struct gx3dLocalBonePose {
  gx3dQuaternion q;         // normalized rotation quaternion
};

struct gx3dLocalPose {
  gx3dMotionSkeleton *skeleton;
  gx3dVector          root_translate;
  gx3dLocalBonePose  *bone_pose;      // array (array size is skeleton->num_bones)
//  unsigned char dirty;              // boolean (1=bone pose data has changed recently)
};

struct gx3dGlobalBonePose {
  gx3dTransform transform;
};

struct gx3dGlobalPose {
  gx3dMotionSkeleton *skeleton;
  gx3dGlobalBonePose *bone_pose;      // array (array size is skeleton->num_bones)
//  unsigned char dirty;              // boolean (1=bone pose data has changed recently)
};

/*___________________
|
| gx3d Motion format
|__________________*/

// channels
const int      gx3dMotionMetadata_MAX_CHANNELS = 6;
const unsigned gx3dMotionMetadataChannel_POS_X = 0x1;
const unsigned gx3dMotionMetadataChannel_POS_Y = 0x2;
const unsigned gx3dMotionMetadataChannel_POS_Z = 0x4;
const unsigned gx3dMotionMetadataChannel_ROT_X = 0x8;
const unsigned gx3dMotionMetadataChannel_ROT_Y = 0x10;
const unsigned gx3dMotionMetadataChannel_ROT_Z = 0x20;
//const unsigned gx3dMotionMetadataChannel_SCALE_X  = 0x40;
//const unsigned gx3dMotionMetadataChannel_SCALE_Y  = 0x80;
//const unsigned gx3dMotionMetadataChannel_SCALE_Z  = 0x100;

// Used to index into gx3dMotionMetadata.channel array
enum gx3dMotionMetadataChannelIndex {
  gx3dMotionMetadataChannelIndex_POS_X = 0,
  gx3dMotionMetadataChannelIndex_POS_Y = 1,
  gx3dMotionMetadataChannelIndex_POS_Z = 2,
  gx3dMotionMetadataChannelIndex_ROT_X = 3,
  gx3dMotionMetadataChannelIndex_ROT_Y = 4,
  gx3dMotionMetadataChannelIndex_ROT_Z = 5
};

// Used by function gx3d_ReadLWSFile() to request metadata loaded from LWS file along with animation 
struct gx3dMotionMetadataRequest {
  char      name[gx_ASCIIZ_STRING_LENGTH_SHORT];  // ASCIIZ string (15 chars max)
  unsigned  channels_requested;                   // bitmask
};

// A single time-value pair key
struct gx3dMotionMetadataKey {
  float time;
  float value;
};

// A channel to hold an array of keys
struct gx3dMotionMetadataChannel {
  int  nkeys;                       // 0 = no data in this channel
  gx3dMotionMetadataKey *keys;
};

// Additional data included with motion (animation) data
struct gx3dMotionMetadata {
  char                      name[gx_ASCIIZ_STRING_LENGTH_SHORT];      // ASCIIZ string (15 chars max)
  unsigned                  channels_present;                         // bitmask (channels that are present in the channel array)
  unsigned                  duration;                                 // total duration of metadata (in milliseconds) - same as duration of the parent motion (copied here for convenience), if 0, then no data
  gx3dMotionMetadataChannel channel[gx3dMotionMetadata_MAX_CHANNELS]; // arrays of channel data (pos xyz, rot xyz)
  // used to create a linked list (temporarily)
  gx3dMotionMetadata       *next;           
};

struct gx3dMotionBone {
  char                      name          [gx_ASCIIZ_STRING_LENGTH_LONG];
  char                      weightmap_name[gx_ASCIIZ_STRING_LENGTH_LONG]; // used to find palette matrix to point to
  gx3dVector                pivot;
  gx3dQuaternion            qrotation;        // pre * (bone rest position) * post, this is the 'bind' pose from Modeler
  bool                      active;           // inactive bones use qrotation instead of rot keys
  int                       nkeys;            // # in pos/rot arrays
  gx3dVector               *pos_key;          // position keyframe data (root bone only)
  gx3dCompressedQuaternion *rot_key;          // rotation keyframe data
  // used to create array
  unsigned char             parent;           // 0xFF = root, else index into array of gx3dMotionBone
};

struct gx3dMotion {
  gx3dMotionSkeleton  *skeleton;
  gx3dLocalPose       *output_local_pose;   // 0=doesn't output to anything
  char                 name[gx_ASCIIZ_STRING_LENGTH_LONG];
//  int                  object_layer_id; 
  gx3dVector           position;				    // offset from object origin (in lwo file)
	gx3dVector			     rotation;				    // about object origin (in lwo file)
	int                  keys_per_second; 
  int                  max_nkeys;           // max # keys of any active bone
  unsigned             duration;            // total duration of motion (in milliseconds)
  int                  num_bones;
  gx3dMotionBone      *bones;               // array of bones
  // metadata
  int                  num_metadata;        // 0-? (num of elements in metadata array)
  gx3dMotionMetadata  *metadata;            // additional data, if any (in an array)
  // used to create doubly-linked list
  gx3dMotion          *next, *previous;
  // used for resource management
//  int                  reference_count;
};

/*___________________
|
| gx3d Skeleton format
|__________________*/

struct gx3dSkeletonBone {
  char							*name;                    // name of bone (should be unique)
  gx3dVector				 pivot;                   // bone pivot point (relative to the local coordinate origin)
  gx3dVector				 direction;               // normalized direction bone begins pointing
  int								 start_point, end_point;	// index into skeleton vertex array - used to build hierarchy
  gx3dTransform			 transform;
  int								 num_nonlocal_matrices;		// 0-?
	gx3dMatrix			 **nonlocal_matrices;				// array of pointers, if any, to palette matrices in object layers
	// used to create a tree
	gx3dSkeletonBone  *child;                   // use to create a hierarchy
  gx3dSkeletonBone  *next;                    // use to create a linked list
};

struct gx3dSkeleton {
  int								origin_point;	  // origin of root of skeleton (index into vertex array)
  int								num_vertices;
  gx3dVector			 *vertex;
  int								num_bones;
  gx3dSkeletonBone *bones;			    // array of bones
  gx3dTransform			root_transform; 
  bool							attached;				// to the owning gx3dObject
};

/*___________________
|
| gx3d Morph format
|__________________*/

typedef int gx3dMorphIndex; // an index into the array of gx3dVertexMorph

struct gx3dVertexMorph {
  char       *name;
  int         num_entries;  // # of index-offset pairs
  int        *index;        // array of indeces into vertex array
  gx3dVector *offset;       // array of vertex offsets
  float       amount;       // 0-1, 0=disabled
};

/*___________________
|
| gx3d Object format
|__________________*/

struct gx3dObjectLayer {
  int                id;            // unique ID for this layer (unique w/in a gx3dObject)
  int                parent_id;     // parent ID (valid only if has_parent is true)
  int                has_parent;    // boolean
  char              *name;          // optional layer name (not necessarily unique w/in a gx3dObject)
  gx3dVector         pivot;         // usually the local coord origin (0,0,0) but not always
  gx3dBox            bound_box;
  gx3dSphere         bound_sphere;
  // vertex position
  gx3dVector        *vertex;
  gx3dVector        *X_vertex;      // transformed set of vertices, if any
  // vertex normal
  gx3dVector        *vertex_normal;
  gx3dVector        *X_vertex_normal;
  // vertex diffuse color
  gxColor           *diffuse;
  // vertex specular color
  gxColor           *specular;
  // vertex texture coordinates
	gx3dUVCoordinate  *tex_coords  [gx3d_NUM_TEXTURE_STAGES]; 
  gx3dUVCoordinate  *X_tex_coords[gx3d_NUM_TEXTURE_STAGES];
  // w-component of vertex texture coordinates (only used to address volume textures)
  float             *tex_coords_w   [gx3d_NUM_TEXTURE_STAGES];
  float             *X_tex_coords_w [gx3d_NUM_TEXTURE_STAGES];
  // vertex weights
  gx3dVertexWeight  *weight;
  gx3dVertexWeight  *X_weight;
  // vertex morphs
  int                num_morphs;        // # morphs in the morph array
  int                num_active_morphs; // # morphs currently active (with amount != 0)
  gx3dVertexMorph   *morph;             // array of morphs, if any
  gx3dVector        *composite_morph;   // composite array for all morph offsets, same size of vertex array
  bool               morphs_dirty;      // true if composite array needs to be updated

  gx3dPolygon       *polygon;
  gx3dVector        *polygon_normal;
  int                num_vertices;
  int                num_polygons;
  int                num_textures;
  gx3dTexture        texture[gx3d_NUM_TEXTURE_STAGES];
  gx3dTransform      transform;

  gx3dPaletteMatrix *matrix_palette;      // array of matrices that affect weightmaps
  int                num_matrix_palette;  // # entries in matrix palette (0-?)

  gx3dObjectLayer   *child;         // use to create a hierarchy
  gx3dObjectLayer   *next;          // use to create a linked list
  // pointer to driver-specific data
  void              *driver_data;
};
  
struct gx3dObject {
  char              *name;            // optional (if loaded from a file, same as filename minus extension)
  unsigned           vertex_format;   // flags
  gx3dBox            bound_box;
  gx3dSphere         bound_sphere;
  gx3dTransform      transform;
  gx3dSkeleton      *skeleton;        // internal skeleton, if any
  gx3dObjectLayer   *layer;           // linked list of layers
  // Used to create a doubly linked list
  gx3dObject			  *next, *previous;
};

/*___________________
|
| gx3d Motion Blend Tree formats
|__________________*/

struct gx3dBlendMask {
  gx3dMotionSkeleton *skeleton;
  float              *values;   // array of 0-1 values, array size is skeleton->num_bones
};

enum gx3dBlendNodeType {
  gx3d_BLENDNODE_TYPE_SINGLE,   // one motion in track 0
  gx3d_BLENDNODE_TYPE_LERP2,    // track0 * blendvalue0 LERP track1 * (1-blendvalue0)
  gx3d_BLENDNODE_TYPE_LERP3,    // track0 * blendvalue0 LERP track1 * blendvalue1 LERP track2 * (1-blendvalue0-blendvalue1)
  gx3d_BLENDNODE_TYPE_ADD       // track0 ADD track1 * blendvalue0 [additive blend is in track1]
};

const int gx3d_BLENDNODE_TRACKS = 3;
enum gx3dBlendNodeTrack {
  gx3d_BLENDNODE_TRACK_0 = 0,
  gx3d_BLENDNODE_TRACK_1 = 1,
  gx3d_BLENDNODE_TRACK_2 = 2
};

struct gx3dBlendNode {
  gx3dBlendNodeType    type;
  gx3dMotionSkeleton  *skeleton;   
  int                  num_tracks;                                // 1-3, depends on type
  gx3dLocalPose       *input_local_pose [gx3d_BLENDNODE_TRACKS];  // array of pointers to local poses (array size is num_tracks)
  gx3dLocalPose       *output_local_pose;                         // last node in tree automatically outputs to tree local pose
  float                blend_value   [gx3d_BLENDNODE_TRACKS-1];   // array of blend values (used in Lerp2, Lerp3 and Add nodes)
  gx3dBlendMask       *blend_mask    [gx3d_BLENDNODE_TRACKS];     // array of pointers to blend masks 
  // used to create linked list
  gx3dBlendNode       *next;
//  float                local_clock   [gx3d_BLENDNODE_TRACKS]; // in seconds
//  float                playback_rate [gx3d_BLENDNODE_TRACKS];
//  int                  loop_rate     [gx3d_BLENDNODE_TRACKS]; // 0=non-looping, -1=loop forever, else=number of times to loop the motion
};

struct gx3dBlendTree {
  gx3dMotionSkeleton  *skeleton;  
  gx3dBlendNode       *nodes;                       // linked list of blend nodes  
  gx3dLocalPose       *local_pose; 
  gx3dGlobalPose      *global_pose;    
  gx3dObjectLayer     *target_objectlayer;          // target object layer (0=none)
  int                 *target_matrix_palette_index; // index into target matrix palette 0-? (-1 = no corresponding target matrix) (array size is skeleton->num_bones)
};

/*___________________
|
| gx3d Boxtree format
|__________________*/

enum gx3dBoxtreeType {
  gx3d_BOXTREE_TYPE_STATIC,
  gx3d_BOXTREE_TYPE_DYNAMIC
};

struct gx3dBoxtreeNode {
  int              num_polys;         // total # polygons contained in this subtree
  word            *poly_index;        // array of indexes into polygon arrays or 0 if not a leaf node (a splitting plane)
//  char             axis;              // 'x', 'y', 'z'
//  float            split;             // split position on given axis
  gx3dBox          box;               // bound box for this node (used if this is a nonterminal node)
  gx3dBoxtreeNode *left, *right;
};

struct gx3dBoxtree {
  gx3dBoxtreeType   type;             // static or dynamic
/***** I want to get rid of this dependency! ************/
//  gx3dObject       *object;           // parent object (may need this for dynamic boxtree)
/********************************************************/
  // Overall bound of boxtree
  gx3dBox           box;   
  // Geometry (all data in object space)
  int               num_polygons;     // # polygons in boxtree
  int               num_vertices;     // # vertices in boxtree
  gx3dObjectLayer **poly_layer;       // array of pointers to layers containing each polygon
  gx3dBox          *poly_box;         // AAAB for each polygon
  gx3dVector       *poly_box_center;  // center of box for each polygon
  gx3dPolygon      *poly;             // array of polygons
  union {
    struct {
      gx3dVector  **vertex;           // array of pointers to vertices
    } s;  // static
    struct {
      gx3dVector   *vertex;           // array of (transformed) vertices
      bool          dirty;            // true if updated needed
    } d;  // dynamic
  };
  // bsp tree
  gx3dBoxtreeNode *root;
};

/*___________________
|
| Macros
|__________________*/

#define gxTranslatePoint(pt,dx,dy)      \
  {                                     \
  pt.x = pt.x + dx;                     \
  pt.y = pt.y + dy;                     \
  }

#define gxTranslateLine(line,dx,dy)     \
  {                                     \
  line.x1 = line.x1 + dx;               \
  line.y1 = line.y1 + dy;               \
  line.x2 = line.x2 + dx;               \
  line.y2 = line.y2 + dy;               \
  }

#define gxTranslateRectangle(rect,dx,dy)  \
  {                                       \
  rect.xleft   = rect.xleft   + dx;       \
  rect.ytop    = rect.ytop    + dy;       \
  rect.xright  = rect.xright  + dx;       \
  rect.ybottom = rect.ybottom + dy;       \
  }

#define gxScalePoint(pt,sx,sy)  \
  {                             \
  pt.x = pt.x * sx;             \
  pt.y = pt.y * sy;             \
  }

#define gxScaleLine(line,sx,sy) \
  {                             \
  line.x1 = line.x1 * sx;       \
  line.y1 = line.y1 * sy;       \
  line.x2 = line.x2 * sx;       \
  line.y2 = line.y2 * sy;       \
  }

#define gxScaleRectangle(rect,sx,sy)    \
  {                                     \
  rect.xleft   = rect.xleft   * sx;     \
  rect.ytop    = rect.ytop    * sy;     \
  rect.xright  = rect.xright  * sx;     \
  rect.ybottom = rect.ybottom * sy;     \
  }

/*___________________
|
| Functions
|__________________*/

// GX_XP.CPP 
void gxError (char *str);               // internal use only 
void gxError_Filename (char *filename); // internal use only

int gxGetUserFormat (
  int       driver,             
  unsigned  acceptable_resolution,    // bit mask 
  unsigned  acceptable_bitdepth,      // bit mask
  unsigned *selected_resolution,      // selected resolution, if any
  unsigned *selected_bitdepth );      // selected depth, if any

int gxStartGraphics (
  unsigned resolution, 
  unsigned bitdepth, 
  unsigned stencildepth,
  int      num_pages, 
  int      driver );

void        gxStopGraphics (void);
void        gxSaveState    (gxState *state);
void        gxRestoreState (gxState *state);
inline int  gxGetScreenWidth (void);
inline int  gxGetScreenHeight (void);
inline int  gxGetBitDepth (void);

void gxGetRGBFormat (
  unsigned *redmask, 
  unsigned *greenmask, 
  unsigned *bluemask,
  int      *low_redbit, 
  int      *low_greenbit, 
  int      *low_bluebit,
  int      *num_redbits, 
  int      *num_greenbits, 
  int      *num_bluebits );

inline float  gxGetAspectRatio (void);
inline void   gxVertRetraceDelay (void);
int           gxRestoreDirectX (void);
inline int    gxGetNumVRAMPages (void);
void          gxFlipVisualActivePages (int wait_for_vsync);
void          gxSetActivePage (gxPage page);
inline gxPage gxGetActivePage (void);
void          gxSetVisualPage (gxPage page, int wait_for_vsync);
inline gxPage gxGetVisualPage (void);
int           gxCreateVirtualPage (int width, int height, unsigned hints, gxPage *page);
void          gxFreeVirtualPage (gxPage page);
int           gxGetPageWidth (gxPage page);
int           gxGetPageHeight (gxPage page);
void          gxClearPage (gxPage page, gxColor color);
void          gxCopyPage (int srcx, int srcy, gxPage srcpg, int dstx, int dsty, gxPage dstpg, int dx, int dy);
int           gxCopyPageColorKey (int srcx, int srcy, gxPage srcpg, int dstx, int dsty, gxPage dstpg, int dx, int dy, gxColor color);
inline void   gxSetColor (gxColor color);
inline gxColor gxGetColor (void);
inline void   gxSetLineWidth (int width);
inline int    gxGetLineWidth (void);
void          gxSetLineStyle (int seg1, int gap1, int seg2, int gap2);
void          gxGetLineStyle (int *seg1, int *gap1, int *seg2, int *gap2);
gxPattern     gxDefineBitmapPattern (byte *bitmap, gxColor fore_color, gxColor back_color, int transparent_background);
gxPattern     gxDefineImagePattern (byte *image);
void          gxFreePattern (gxPattern pattern);
void          gxSetFillPattern (gxPattern pattern);
inline gxPattern gxGetFillPattern (void);
inline void   gxSetLogicOp (int logic_op);
inline int    gxGetLogicOp (void);
inline void   gxSetWindow (gxRectangle *win);
inline void   gxGetWindow (gxRectangle *win);
inline void   gxSetClip   (gxRectangle *clip);
inline void   gxGetClip   (gxRectangle *clip);
inline void   gxSetClipping (int flag);
inline int    gxGetClipping (void);
inline void   gxClearWindow (void);
inline int    gxGetMaxX (void);
inline int    gxGetMaxY (void);

// CLIP.CPP 
int gxClipPoint (int x, int y);
int gxClipRectangle (int *xleft, int *ytop, int *xright, int *ybottom);
int gxClipLine (int *x1, int *y1, int *x2, int *y2);

// PRIM.CPP 
void    gxDrawPixel (int x, int y);
gxColor gxGetPixel (int x, int y);
void    gxDrawLine (int x1, int y1, int x2, int y2);
void    gxDrawRectangle (int x1, int y1, int x2, int y2);
void    gxDrawFillRectangle (int x1, int y1, int x2, int y2);
void    gxDrawPoly (int num_points, int *points);
void    gxDrawCircle (int ctrx, int ctry, int radius);
void    gxDrawFillCircle (int ctrx, int ctry, int radius);
void    gxDrawEllipse (int ctrx, int ctry, int xradius, int yradius);
void    gxDrawFillEllipse (int ctrx, int ctry, int xradius, int yradius);
void    gxDrawArc (int ctrx, int ctry, int radius, int start_angle, int end_angle);

// FLD_FILL.CPP 
void gxFloodFill (int seed_x, int seed_y, gxRectangle *bounds);

// FILLPOLY.CPP 
void gxDrawFillPoly (int num_points, int *points);

// PIXMAP.CPP 
unsigned gxImageSize (gxBound box);
unsigned gxSpriteSize (gxBound box);
unsigned gxBitmapSize (gxBound box);
unsigned gxBytemapSize (gxBound box);
void     gxGetImage (gxBound box, byte *image);
void     gxGetSprite (gxBound box, byte *sprite, gxColor *mask_color, gxColor *filter_color);
void     gxGetBitmap (gxBound box, byte *bitmap);
byte    *gxCreateImage (gxBound box);
byte    *gxCreateSprite (gxBound box, gxColor *mask_color, gxColor *filter_color);
byte    *gxCreateBitmap (gxBound box);
byte    *gxCreateBytemap (byte *image);
void     gxDrawImage (byte *image, int x, int y);
void     gxDrawSprite (byte *sprite, int x, int y);
void     gxDrawSpriteMask (byte *sprite, int x, int y, gxColor color);
void     gxDrawBitmap (byte *bitmap, int x, int y, gxColor color);
void     gxDrawPixelImage (byte *image, int x, int y, gxColor color);
gxColor  gxGetPixelImage (byte *image, int x, int y);

// SCALE.CPP 
byte *gxScaleImage  (byte *image,  float sx, float sy);
byte *gxScaleSprite (byte *sprite, float sx, float sy);
byte *gxScaleBitmap (byte *bitmap, float sx, float sy);

// ROTATE.CPP 
byte *gxRotateImage  (byte *image,  float degrees);
byte *gxRotateSprite (byte *sprite, float degrees);
byte *gxRotateBitmap (byte *bitmap, float degrees);

// EFFECT.CPP 
void gxDrawImageEffect (byte *image, int x, int y, int effect);
void gxDrawSpriteEffect (byte *sprite, int x, int y, int effect);
void gxDrawFillRectangleEffect (int x1, int y1, int x2, int y2, int effect);

// TEXT.CPP 
gxFont *gxLoadFont (int font_type, char *filename);
gxFont *gxLoadFontData (int font_type, byte *buff, unsigned buff_size);
int     gxSaveFont (char *filename);
void    gxFreeFont (gxFont *font);
void    gxSetFont (gxFont *font);
gxFont *gxGetFont (void);
gxFont *gxScaleFont (gxFont *font, int sx, int sy);
void    gxSetFontAttributes (int spacing, int inter_char_spacing);
void    gxGetFontAttributes (int *spacing, int *inter_char_spacing);
void    gxSetSpaceCharWidth (int width);
int     gxGetSpaceCharWidth (void);
int     gxGetFontWidth (void);
int     gxGetFontHeight (void);
int     gxGetStringWidth (char *str);
void    gxDrawText (char *str, int x, int y);
void    gxDrawTextOverwrite (char *str, int x, int y, gxColor back_color);
char   *gxGetString (
  char    *str,                   // string to fill                       
  int      max_str,               // max characters to put into str, not counting NULL 
  int      x,                     // position in window to echo input     
  int      y,
  gxColor  text_color,            // color to echo input in               
  gxColor *back_color,            // background color or NULL for no background color                    
  void   (*animate_func)(void) ); // ptr to callers animate function 

// PCX.CPP 
int gxReadPCXFile (char *filename, int set_palette);
int gxWritePCXFile (char *filename);
int gxGetPCXFileDimensions (char *filename, int *width, int *height);
int gxConvertPCXFile (
  char *infilename,
  char *outfilename,
  int   palette_spread );

// BMP.CPP 
int gxReadBMPFile (char *filename, int set_palette);
int gxWriteBMPFile (char *filename);
int gxGetBMPFileDimensions (char *filename, int *width, int *height, int *bits_per_pixel);

// PALETTE.CPP
void gxSetPalette (byte *rgb_palette, int num_colors);
int  gxGetPalette (byte *rgb_palette, int num_colors);
void gxSetPaletteEntry (int entry, byte *rgb_color);
int  gxGetPaletteEntry (int entry, byte *rgb_color);
void gxSetUniformPalette (byte *rgb_palette, int palette_spread);
void gxGetVGAPalette (byte *rgb_palette);
void gxCopyPalette (
  byte *src_rgb_palette,
  byte *dst_rgb_palette,
  int   start,
  int   num );

// RELATION.CPP
gxRelation gxRelation_Line_Line (
  gxPointF *p1, 
  gxPointF *p2, 
  gxPointF *p3, 
  gxPointF *p4, 
  gxPointF *intersection ); // NULL if no return value needed
gxRelation gxRelation_Point_Polygon (float x, float y, gxPointF *poly, int num_poly_points);
gxRelation gxRelation_Line_Triangle (gxPointF *p1, gxPointF *p2, gxPointF *triangle);
gxRelation gxRelation_Triangle_Triangle (gxPointF *triangle1, gxPointF *triangle2);

/*___________________
|
| 3D Functions
|__________________*/

// GX3D.CPP
int         gx3d_BeginRender ();
void        gx3d_EndRender ();
inline void gx3d_SetFillMode (int fill_mode);
inline int  gx3d_GetFillMode ();
void        gx3d_GetDriverInfo (gx3dDriverInfo *info);

void       gx3d_SetViewport (gxRectangle *win);
void       gx3d_GetViewport (gxRectangle *win);
inline void gx3d_ClearViewport (
  unsigned flags,           // surface? zbuffer? stencil?
  gxColor  surface_color,   // new color to write to surface
  float    z_value,         // new z buffer value, 0.0 (nearest) to 1.0 (farthest)
  unsigned stencil_value ); // new stencil buffer value
inline void gx3d_ClearViewportRectangle (
  gxRectangle *rect,        // screen-relative rectangle
  unsigned flags,           // surface? zbuffer? stencil?
  gxColor  surface_color,   // new color to write to surface
  float    z_value,         // new z buffer value, 0.0 (nearest) to 1.0 (farthest)
  unsigned stencil_value ); // new stencil buffer value

inline void          gx3d_EnableClipping (void);
inline void          gx3d_DisableClipping (void);
inline gx3dClipPlane gx3d_InitClipPlane (float a, float b, float c, float d);
inline void          gx3d_FreeClipPlane (gx3dClipPlane plane);
inline void          gx3d_EnableClipPlane (gx3dClipPlane plane);
inline void          gx3d_DisableClipPlane (gx3dClipPlane plane);

/***** ZBuffer functions *****/
inline void gx3d_EnableZBuffer (void);
inline void gx3d_DisableZBuffer (void);
inline void gx3d_SetZBias (int bias);
inline void gx3d_SetBackfaceRemoval (int flag);

/***** Stencil buffer functions *****/
inline void gx3d_EnableStencilBuffer (void);
inline void gx3d_DisableStencilBuffer (void);
inline void gx3d_SetStencilFailOp (int stencil_op);                   // default is KEEP
inline void gx3d_SetStencilZFailOp (int stencil_op);                  // default is KEEP
inline void gx3d_SetStencilPassOp (int stencil_op);                   // default is KEEP
inline void gx3d_SetStencilComparison (int stencil_function);         // default is ALWAYS
inline void gx3d_SetStencilReferenceValue (unsigned reference_value); // default is 0
inline void gx3d_SetStencilMask (unsigned mask);                      // default is 0xFFFFFFFF
inline void gx3d_SetStencilWriteMask (unsigned mask);                 // default is 0xFFFFFFFF

/***** Light functions *****/
// Enables lighting
inline void gx3d_EnableLighting (void);
// Disables lighting
inline void gx3d_DisableLighting (void);
// Set the ambient light
inline void gx3d_SetAmbientLight (gx3dColor color);
// Enable/disable specular highlights
inline void gx3d_EnableSpecularLighting (void);
inline void gx3d_DisableSpecularLighting (void);
// Enable/disable vertex lighting info
inline void gx3d_EnableVertexLighting (void);
inline void gx3d_DisableVertexLighting (void);
// Creates a light
gx3dLight   gx3d_InitLight (gx3dLightData *data);
// Changes light parameters
void        gx3d_UpdateLight (gx3dLight light, gx3dLightData *data);
// Frees a light
inline void gx3d_FreeLight (gx3dLight light);
// Enables a light
inline void gx3d_EnableLight (gx3dLight light);
// Disables a light
inline void gx3d_DisableLight (gx3dLight light);

/***** Fog Functions *****/
// Enable fog using the previously set formula
inline void gx3d_EnableFog (void);
// Disable fog
inline void gx3d_DisableFog (void);
// Set the fog color
inline void gx3d_SetFogColor (byte r, byte g, byte b);
// Set a linear pixel fog formula
inline void gx3d_SetLinearPixelFog (float start_distance, float end_distance);
// Set an EXP fog formula
inline void gx3d_SetExpPixelFog (float density);   // 0-1
// Set an EXP2 for formula
inline void gx3d_SetExp2PixelFog (float density);  // 0-1
// Set a linear vertex fog formula
inline void gx3d_SetLinearVertexFog (float start_distance, float end_distance, int ranged_based);

/***** Material functions *****/
inline void gx3d_SetMaterial (gx3dMaterialData *data);        
inline void gx3d_GetMaterial (gx3dMaterialData *data);

/***** Alpha-blending functions *****/
inline void gx3d_EnableAlphaBlending (void);
inline void gx3d_DisableAlphaBlending (void);
inline void gx3d_SetAlphaBlendFactor (int src_blend_factor, int dst_blend_factor);
inline int  gx3d_AlphaTestingAvailable (void);
inline void gx3d_EnableAlphaTesting (byte reference_value);
inline void gx3d_DisableAlphaTesting (void);

/***** Misc functions *****/
inline void gx3d_EnableAntialiasing (void);        
inline void gx3d_DisableAntialiasing (void);

// GX3D_TEXTURE.CPP
void gx3d_SetTextureDirectory (char *dir);
// Loads a texture from file/s
gx3dTexture gx3d_InitTexture_File (char *filename, char *alpha_filename, unsigned flags);
// Creates a texture
gx3dTexture gx3d_InitTexture_Image (
  int      num_mip_levels,            // 1 or more
  byte   **image,                     // array of 1 or more images
  gxColor *transparent_color,
  int      texture_bits_per_pixel );  // 16 or 24
gx3dTexture gx3d_InitTexture_Sprite (
  int    num_mip_levels,              // 1 or more
  byte **sprite,                      // array of 1 or more sprites
  int    texture_bits_per_pixel );    // 16 or 24
gx3dTexture gx3d_InitTexture_Image_Bitmap (
  int    num_mip_levels,              // 1 or more
  byte **image,                       // array of 1 or more images
  byte **bitmap,                      // array of 1 or more bitmaps
  int    texture_bits_per_pixel );    // 16 or 24
gx3dTexture gx3d_InitTexture_Image_Bytemap (
  int    num_mip_levels,              // 1 or more
  byte **image,                       // array of 1 or more images
  byte **bytemap,                     // array of 1 or more bytemaps
  int    texture_bits_per_pixel );    // 16 or 24
gx3dTexture gx3d_InitTexture_Bytemap (
  int    num_mip_levels,              // 1 or more
  byte **bytemap );                   // array of 1 or more bytemaps
// Load a volume texture from files
gx3dTexture gx3d_InitTexture_File_Volume (char ***filenames, char ***alpha_filenames);
// Load a cubemap texture from a file
gx3dTexture gx3d_InitTexture_File_Cubemap (char *filename, char *alpha_filename);
// Create a renderable square texture
gx3dTexture gx3d_InitRenderTexture (int dimensions);
// Create a renderable cube map texture
gx3dTexture gx3d_InitRenderTexture_Cubemap (int dimensions);
// Frees a texture
inline void  gx3d_FreeTexture (gx3dTexture texture);
// Frees all textures
inline void gx3d_FreeAllTextures (void);
// Sets current render texture
inline void gx3d_SetTexture (int stage, gx3dTexture texture);
// Gets current render texture
inline gx3dTexture gx3d_GetTexture (int stage);
// Returns combined size of all currently loaded textures
inline unsigned gx3d_GetTextureAllocationSize (void);
// Sets addressing mode of a texture stage (wrap/mirror/clamp/border)
inline void gx3d_SetTextureAddressingMode (int stage, int dimension, int addressing_mode);
// Sets border color of a texture stage
inline void gx3d_SetTextureBorderColor (int stage, gxColor color);
// Sets filtering algorithm of a texture stage
inline void gx3d_SetTextureFiltering (
  int stage, 
  int filter_type, 
  int anisotropy_level ); // amount of filtering (1-100), 1=min, 100=max
inline void gx3d_SetTextureCoordinates (
  int             stage,            // texture blending stage
  gx3dTexCoordSet coordinate_set ); // set of texture coordinates in the object or gx3d_TEXCOORD_CUBEMAP
// Sets texture wrapping for a set of texture coordinates in an object
inline void gx3d_SetTextureWrapping (
  int coordinate_stage, // set of texture coordinates in the object
  int wrap_s,           // boolean (u dimension)
  int wrap_t,           // boolean (v dimension)
  int wrap_r,           // boolean
  int wrap_q );         // boolean
// Set the texture factor used by some texture blending operations
inline void gx3d_SetTextureFactor (byte r, byte g, byte b, byte a);
// Preloads a texture into vram
inline void gx3d_PreLoadTexture (gx3dTexture texture);
// Flushes all textures out of texture memory
inline void gx3d_EvictAllTextures (void);
// Sets texture blending color operation
//  default for stage 0 is MODULATE, all other stages is DISABLE
//  default arg1 is TEXTURE
//  default arg2 is CURRENT
inline void gx3d_SetTextureColorOp (int stage, int texture_colorop, int texture_arg1, int texture_arg2);
// Sets texture blending alpha operation
//  default for stage 0 is SELECTARG1, all other stages is DISABLE
//  default arg1 is TEXTURE
//  default arg2 is CURRENT
inline void gx3d_SetTextureAlphaOp (int stage, int texture_alphaop, int texture_arg1, int texture_arg2);
// Sets texture blending color factor
inline void gx3d_SetTextureColorFactor (gx3dColor color);
void        gx3d_EnableCubemapTexturing (int stage);
void        gx3d_DisableCubemapTexturing (int stage);
// Allows caller to modify/render to a renderable texture
inline void gx3d_BeginModifyTexture (gx3dTexture texture, int face);
inline void gx3d_EndModifyTexture (void);
inline void gx3d_AddRefTexture (gx3dTexture texture);

// GX3D_MATH.CPP
void gx3d_ComputeViewMatrix (
  gx3dMatrix *m,            // the resulting matrix
  gx3dVector *from,         // the eye point (in world coordinates)
  gx3dVector *to,           // the look at point (in world coordinates)
  gx3dVector *world_up );   // the normalized world up vector, typically (0,1,0)
void gx3d_ComputeProjectionMatrix (
  gx3dMatrix *m,            // the resulting matrix
  float       fov,          // field of view in degrees
  float       near_plane,
  float       far_plane );
void gx3d_ComputeProjectionMatrixHV (
  gx3dMatrix *m,            // the resulting matrix
  float       hfov,         // horizontal field of view in degrees (0.1 - 179.9)
  float       vfov,         // vertical field of view in degrees (0.1 - 179.9)
  float       near_plane,   // in world z units
  float       far_plane );  // in world z units

void      gx3d_SetWorldMatrix      (gx3dMatrix *m);
void      gx3d_GetWorldMatrix      (gx3dMatrix *m);
void      gx3d_SetViewMatrix       (gx3dMatrix *m);
void      gx3d_SetViewMatrix (
  gx3dVector *from,       // the eye point
  gx3dVector *to,         // the look at point
  gx3dVector *world_up ); // the world up vector, typically (0,1,0)
void      gx3d_GetViewMatrix       (gx3dMatrix *m);
void      gx3d_SetProjectionMatrix (gx3dMatrix *m);
void      gx3d_SetProjectionMatrix (
  float fov,          // field of view in degrees
  float near_plane,   // in world z units
  float far_plane );  // in world z units
void      gx3d_SetProjectionMatrix (
  float hfov,         // horizontal field of view in degrees (0.1 - 179.9)
  float vfov,         // vertical field of view in degrees (0.1 - 179.9)
  float near_plane,   // in world z units
  float far_plane );  // in world z units
void      gx3d_GetProjectionMatrix (gx3dMatrix *m);

void      gx3d_GetViewFrustum (gx3dViewFrustum *vf);
void      gx3d_GetWorldFrustum (gx3dViewFrustum *vf, gx3dWorldFrustum *wf);

inline void gx3d_GetIdentityMatrix         (gx3dMatrix *m);
void      gx3d_GetTransposeMatrix        (gx3dMatrix *m, gx3dMatrix *mresult);
void      gx3d_OrthogonalizeMatrix (gx3dMatrix *m);
void      gx3d_GetTranslateMatrix        (gx3dMatrix *m, float tx, float ty, float tz);
void      gx3d_GetTranslateMatrixInverse (gx3dMatrix *m, float tx, float ty, float tz);
void      gx3d_GetScaleMatrix            (gx3dMatrix *m, float sx, float sy, float sz);
void      gx3d_GetScaleMatrixInverse     (gx3dMatrix *m, float sx, float sy, float sz);
void      gx3d_GetRotateXMatrix          (gx3dMatrix *m, float degrees);
void      gx3d_GetRotateXMatrixInverse   (gx3dMatrix *m, float degrees);
void      gx3d_GetRotateYMatrix          (gx3dMatrix *m, float degrees);
void      gx3d_GetRotateYMatrixInverse   (gx3dMatrix *m, float degrees);
void      gx3d_GetRotateZMatrix          (gx3dMatrix *m, float degrees);
void      gx3d_GetRotateZMatrixInverse   (gx3dMatrix *m, float degrees);
void      gx3d_GetRotateMatrix (gx3dMatrix *m, gx3dVector *axis, float degrees);

void        gx3d_EnableTextureMatrix   (int stage);
void        gx3d_EnableTextureMatrix3D (int state);
void        gx3d_DisableTextureMatrix  (int stage);
void        gx3d_SetTextureMatrix      (int stage, gx3dMatrix *m);
void        gx3d_GetTextureMatrix      (int stage, gx3dMatrix *m);
void        gx3d_GetTranslateTextureMatrix        (gx3dMatrix *m, float tx, float ty);
void        gx3d_GetTranslateTextureMatrixInverse (gx3dMatrix *m, float tx, float ty);
void        gx3d_GetScaleTextureMatrix            (gx3dMatrix *m, float sx, float sy);
void        gx3d_GetScaleTextureMatrixInverse     (gx3dMatrix *m, float sx, float sy);
inline void gx3d_GetRotateTextureMatrix           (gx3dMatrix *m, float degrees);
inline void gx3d_GetRotateTextureMatrixInverse    (gx3dMatrix *m, float degrees);

void         gx3d_MultiplyMatrix (gx3dMatrix *m1, gx3dMatrix *m2, gx3dMatrix *mresult);
void         gx3d_MultiplyScalarMatrix (float s, gx3dMatrix *m, gx3dMatrix *mresult);
void         gx3d_MultiplyVectorMatrix (gx3dVector *v, gx3dMatrix *m, gx3dVector *vresult);
void         gx3d_MultiplyNormalVectorMatrix (gx3dVector *v, gx3dMatrix *m, gx3dVector *vresult);
void         gx3d_MultiplyVector4DMatrix (gx3dVector4D *v, gx3dMatrix *m, gx3dVector4D *vresult);
inline void  gx3d_MultiplyScalarVector (float s, gx3dVector *v, gx3dVector *vresult);

inline void  gx3d_NormalizeVector (gx3dVector *v, gx3dVector *normal);
inline void  gx3d_NormalizeVector (gx3dVector *v, gx3dVector *normal, float *magnitude);
inline float gx3d_VectorMagnitude (gx3dVector *v);
inline float gx3d_VectorDotProduct (gx3dVector *v1, gx3dVector *v2);
inline float gx3d_AngleBetweenVectors (gx3dVector *v1, gx3dVector *v2);
inline float gx3d_AngleBetweenUnitVectors (gx3dVector *v1, gx3dVector *v2);
inline void  gx3d_VectorCrossProduct (gx3dVector *v1, gx3dVector *v2, gx3dVector *vresult);
inline void  gx3d_AddVector (gx3dVector *v1, gx3dVector *v2, gx3dVector *vresult);
inline void  gx3d_SubtractVector (gx3dVector *v1, gx3dVector *v2, gx3dVector *vresult);
inline void  gx3d_NegateVector (gx3dVector *v, gx3dVector *vresult);
void         gx3d_ProjectVectorOntoVector (gx3dVector *v, gx3dVector *n, gx3dVector *v_parallel, gx3dVector *v_perpindicular);
void         gx3d_ProjectVectorOntoVector (gx3dVector *v, gx3dVector *n, gx3dVector *v_parallel);
void         gx3d_ProjectVectorOntoUnitVector (gx3dVector *v, gx3dVector *n, gx3dVector *v_parallel, gx3dVector *v_perpindicular);
void         gx3d_ProjectVectorOntoUnitVector (gx3dVector *v, gx3dVector *n, gx3dVector *v_parallel);
int          gx3d_SurfaceNormal (gx3dVector *p1, gx3dVector *p2, gx3dVector *p3, gx3dVector *normal);
inline float gx3d_Lerp (float start, float end, float t); // t is between 0-1
void         gx3d_LerpVector (gx3dVector *start, gx3dVector *end, float t, gx3dVector *vresult);  // t is between 0-1 normally
inline float gx3d_Clamp (float value, float low, float high);

void         gx3d_GetPlane (gx3dVector *p1, gx3dVector *p2, gx3dVector *p3, gx3dPlane *plane);
void         gx3d_GetPlane (gx3dVector *point, gx3dVector *normal, gx3dPlane *plane);

void         gx3d_GetBillboardRotateXYMatrix (gx3dMatrix *m, gx3dVector *billboard_normal, gx3dVector *view_normal);
void         gx3d_GetBillboardRotateXMatrix  (gx3dMatrix *m, gx3dVector *billboard_normal, gx3dVector *view_normal);
void         gx3d_GetBillboardRotateYMatrix  (gx3dMatrix *m, gx3dVector *billboard_normal, gx3dVector *view_normal);

void         gx3d_HeadingToXZVector (float heading, gx3dVector *v);
void         gx3d_HeadingToYZVector (float heading, gx3dVector *v);
void         gx3d_XZVectorToHeading (gx3dVector *v, float *heading);
void         gx3d_YZVectorToHeading (gx3dVector *v, float *heading);

// GX3D_CAMERA.CPP
void         gx3d_CameraSetPosition (gx3dVector *from, gx3dVector *to, gx3dVector *world_up, int orientation);
void         gx3d_CameraGetCurrentPosition (gx3dVector *from, gx3dVector *to, gx3dVector *world_up);
int          gx3d_CameraGetCurrentOrientation ();
inline float gx3d_CameraGetCurrentDistance (void);
void         gx3d_CameraGetCurrentRotation (float *x_axis_rotate_degrees, float *y_axis_rotate_degrees);
void         gx3d_CameraRotate (float x_axis_rotate_degrees, float y_axis_rotate_degrees);
void         gx3d_CameraScale (float scale);
inline void  gx3d_CameraSetViewMatrix (void);

// GX3D_OBJECT.CPP
gx3dObject *gx3d_CreateObject    (void);
gx3dObjectLayer *gx3d_CreateObjectLayer (gx3dObject *object);
void        gx3d_FreeObject      (gx3dObject *object);
void				gx3d_FreeAllObjects  ();
gx3dObject *gx3d_CopyObject      (gx3dObject *object);
void        gx3d_SetObjectName   (gx3dObject *object, char *name);
void        gx3d_OptimizeObject  (gx3dObject *object);
void        gx3d_GetObjectInfo   (gx3dObject *object, int *num_layers, int *num_vertices, int *num_polygons);
void        gx3d_DrawObject      (gx3dObject *object, unsigned flags = 0);			// available flags: gx3d_DONT_SET_TEXTURES, gx3d_DONT_SET_LOCAL_MATRIX
void        gx3d_DrawObjectLayer (gx3dObjectLayer *layer, unsigned flags = 0);	// available flags: gx3d_DONT_SET_TEXTURES, gx3d_DONT_SET_LOCAL_MATRIX
void				gx3d_Object_UpdateTransforms (gx3dObject *object);
gx3dObjectLayer *gx3d_GetObjectLayer (gx3dObject *object, char *name);
void        gx3d_SetObjectMatrix (gx3dObject *object, gx3dMatrix *m);
void        gx3d_SetObjectLayerMatrix (gx3dObject *object, gx3dObjectLayer *layer, gx3dMatrix *m);

void        gx3d_TwistXObject (gx3dObject *object, float twist_rate);
void        gx3d_TwistYObject (gx3dObject *object, float twist_rate);
void        gx3d_TwistZObject (gx3dObject *object, float twist_rate);

void        gx3d_TransformObject (gx3dObject *object, gx3dMatrix *m);
void        gx3d_TransformObjectLayer (gx3dObjectLayer *layer, gx3dMatrix *m);

void        gx3d_CombineObjects (gx3dObject *dst_obj, gx3dObject *src_obj);

void        gx3d_ReadLWO2File  (
  char        *filename, 
  gx3dObject **object, 
  unsigned     vertex_format, // use gx3d_VERTEXFORMAT_... flags
  unsigned     flags );       // available flags: gx3d_DONT_COMBINE_LAYERS, gx3d_DONT_GENERATE_MIPMAPS
void        gx3d_WriteLWO2File (char *filename, gx3dObject *object);

void        gx3d_WriteGX3DBINFile (
  char       *filename, 
  gx3dObject *object, 
  bool        output_texcoords,
  bool        output_vertex_normals, 
  bool        output_diffuse_color,
  bool        output_specular_color,
  bool        output_weights,
  bool        output_morphs,
  bool        output_skeleton,
  bool        opengl_formatting, 
  bool        write_textfile_version );
void        gx3d_ReadGX3DBINFile  (
  char        *filename, 
  gx3dObject **object, 
  unsigned     vertex_format, // use gx3d_VERTEXFORMAT_... flags
  unsigned     flags );       // available flags: gx3d_DONT_COMBINE_LAYERS, gx3d_DONT_GENERATE_MIPMAPS

gxRelation gx3d_ObjectBoundBoxVisible (gx3dObject *object);
gxRelation gx3d_ObjectBoundSphereVisible (gx3dObject *object);

void gx3d_MakeDoubleSidedObject (gx3dObject *object);
void gx3d_MakeDoubleSidedObjectLayer (gx3dObjectLayer *layer);
void gx3d_ComputeVertexNormals (gx3dObject *object, unsigned flags);
void gx3d_ComputeObjectBounds (gx3dObject *object);

// Returns morph index or -1 if not found
gx3dMorphIndex gx3d_GetMorph (gx3dObjectLayer *layer, char *morph_name);
// Set the amount of a morph (0-1), 0=disable the morph
void gx3d_SetMorphAmount (gx3dObjectLayer *layer, gx3dMorphIndex morph_index, float amount);
void gx3d_SetMorphAmount (gx3dObjectLayer *layer, char *morph_name, float amount);
void gx3d_SetMorphAmount (gx3dObject *object, char *morph_name, float amount);

// GX3D_SKELETON.CPP
gx3dSkeleton *gx3d_Skeleton_Init (
  int         num_vertices,
  gx3dVector *vertices,       // array of vertices
  int         origin_point,   // origin of root of skeleton (index into vertices array)
  int         num_bones );
void gx3d_Skeleton_AddBone (
  gx3dObject	*object,
  char        *name,          // name of bone (should be unique)
  gx3dVector  *pivot,         // bone pivot point (relative to the local coordinate origin)
  gx3dVector  *direction,     // normalized direction bone begins pointing
  int          start_point,		// indexes into skeleton vertex array - used to build hierarchy
  int          end_point );
void gx3d_Skeleton_Free (gx3dSkeleton *skel);
gx3dSkeleton		 *gx3d_Skeleton_Copy (gx3dSkeleton *skel);

gx3dSkeletonBone *gx3d_Skeleton_GetBone (gx3dObject *object, char *name);
void							gx3d_Skeleton_SetMatrix (gx3dObject *object, gx3dMatrix *m);
void							gx3d_Skeleton_SetBoneMatrix (gx3dSkeletonBone *bone, gx3dMatrix *m);
void							gx3d_Skeleton_UpdateTransforms (gx3dObject *object);
void							gx3d_Skeleton_Attach (gx3dObject *object);
void							gx3d_Skeleton_Detach (gx3dObject *object);

// GX3D_MOTIONSKELETON.CPP
gx3dMotionSkeleton *gx3d_MotionSkeleton_Init ();
gx3dMotionSkeleton *gx3d_MotionSkeleton_Read_LWS_File (char *filename);
gx3dMotionSkeleton *gx3d_MotionSkeleton_Read_GX3DSKEL_File (char *filename);
void                gx3d_MotionSkeleton_Free (gx3dMotionSkeleton *skeleton);
void                gx3d_MotionSkeleton_Free_All ();
void                gx3d_MotionSkeleton_Print (gx3dMotionSkeleton *skeleton, char *outputfilename);
void                gx3d_MotionSkeleton_Write_GX3DSKEL_File (gx3dMotionSkeleton *skeleton, char *filename);
bool                gx3d_MotionSkeleton_GetBoneIndex (gx3dMotionSkeleton *skeleton, char *bone_name, int *bone_index);

// GX3D_LOCALPOSE.CPP
gx3dLocalPose *gx3d_LocalPose_Init (gx3dMotionSkeleton *skeleton);
void           gx3d_LocalPose_Free (gx3dLocalPose *pose);

// GX3D_GLOBALPOSE.CPP
gx3dGlobalPose *gx3d_GlobalPose_Init (gx3dMotionSkeleton *skeleton);
void            gx3d_GlobalPose_Free (gx3dGlobalPose *pose);

// GX3D_BLENDMASK.CPP
gx3dBlendMask *gx3d_BlendMask_Init (gx3dMotionSkeleton *skeleton, float initial_value);
void           gx3d_BlendMask_Free (gx3dBlendMask *blendmask);
void           gx3d_BlendMask_Set_All   (gx3dBlendMask *blendmask, float value);
void           gx3d_BlendMask_Set_Bone  (gx3dBlendMask *blendmask, char *bone_name, float value);
void           gx3d_BlendMask_Set_Bone  (gx3dBlendMask *blendmask, int bone_index, float value);
void           gx3d_BlendMask_Set_Chain (gx3dBlendMask *blendmask, char *bone_name, float value);
void           gx3d_BlendMask_Set_Chain (gx3dBlendMask *blendmask, int bone_index, float value);

// GX3DBLENDNODE.CPP                                        
gx3dBlendNode        *gx3d_BlendNode_Init             (gx3dMotionSkeleton *skeleton, gx3dBlendNodeType type);
void                  gx3d_BlendNode_Free             (gx3dBlendNode *blendnode);
gx3dLocalPose        *gx3d_BlendNode_Get_Input        (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track);
void                  gx3d_BlendNode_Set_Output       (gx3dBlendNode *blendnode, gx3dLocalPose *pose);
void                  gx3d_BlendNode_Set_Output       (gx3dBlendNode *src_blendnode, gx3dBlendNode *dst_blendnode, gx3dBlendNodeTrack dst_track);
void                  gx3d_BlendNode_Set_BlendMask    (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track, gx3dBlendMask *blendmask); // blendmask=0 to disable
void                  gx3d_BlendNode_Set_BlendValue   (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track, float value);
//void                  gx3d_BlendNode_Set_LocalClock   (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track, float value);
//void                  gx3d_BlendNode_Set_PlaybackRate (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track, float value);
//void                  gx3d_BlendNode_Set_LoopRate     (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track, int ntimes);  // 0=none, -1=forever
//inline float          gx3d_BlendNode_Get_LocalClock   (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track);
//inline float          gx3d_BlendNode_Get_PlaybackRate (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track);
//inline int            gx3d_BlendNode_Get_LooRate      (gx3dBlendNode *blendnode, gx3dBlendNodeTrack track); 
void                  gx3d_BlendNode_Update           (gx3dBlendNode *blendnode);

// GX3DBLENDTREE.CPP
gx3dBlendTree *gx3d_BlendTree_Init             (gx3dMotionSkeleton *skeleton);
void           gx3d_BlendTree_Free             (gx3dBlendTree *blendtree);
void           gx3d_BlendTree_Add_Node         (gx3dBlendTree *blendtree, gx3dBlendNode *blendnode);
void           gx3d_BlendTree_Remove_Node      (gx3dBlendTree *blendtree);
void           gx3d_BlendTree_Remove_All_Nodes (gx3dBlendTree *blendtree);
void           gx3d_BlendTree_Set_Output       (gx3dBlendTree *blendtree, gx3dObjectLayer *objectlayer);
void           gx3d_BlendTree_Update           (gx3dBlendTree *blendtree, gx3dVector *new_position = 0);

// GX3D_MOTION.CPP
gx3dMotion *gx3d_Motion_Init (gx3dMotionSkeleton *skeleton);                                            
gx3dMotion *gx3d_Motion_Read_LWS_File (gx3dMotionSkeleton *skeleton, char *filename, int fps, gx3dMotionMetadataRequest *metadata_requested, int num_metadata_requested, bool load_all_metadata);
gx3dMotion *gx3d_Motion_Read_GX3DANI_File (gx3dMotionSkeleton *skeleton, char *filename);
gx3dMotion *gx3d_Motion_Copy (gx3dMotion *motion);
gx3dMotion *gx3d_Motion_Compute_Difference (gx3dMotion *reference_motion, gx3dMotion *source_motion);
void        gx3d_Motion_Free (gx3dMotion *motion);
void        gx3d_Motion_Free_All ();
void        gx3d_Motion_Set_Output (gx3dMotion *motion, gx3dBlendNode *blendnode, gx3dBlendNodeTrack track);
bool        gx3d_Motion_Update (gx3dMotion *motion, float local_time, bool repeat);
void        gx3d_Motion_Write_GX3DANI_File (gx3dMotion *motion, char *filename, bool opengl_formatting);
gx3dMotionMetadata *gx3d_Motion_GetMetadata (gx3dMotion *motion, char *name);
bool        gx3d_MotionMetadata_GetSample (gx3dMotionMetadata *metadata, gx3dMotionMetadataChannelIndex channel_index, float local_time, bool repeat, float *sample);
gx3dMotionMetadata *gx3d_MotionMetadata_Copy (gx3dMotionMetadata *metadata);
void        gx3d_Motion_Print (gx3dMotion *motion, char *outputfilename);

// LWO2_PRINT.CPP
void gx3d_PrintLWO2File (char *filename, char *outputfilename, bool verbose = true);

// LWS_PRINT.CPP
void gx3d_PrintLWSFile (char *filename, char *outputfilename);

// GX3D_PARTICLESYSTEM.CPP
gx3dParticleSystem gx3d_InitParticleSystem (
  gx3dParticleSystemData *particle_system_data,
  char                   *image_texture_filename,
  char                   *alpha_texture_filename,
  unsigned                flags );
void gx3d_FreeParticleSystem      (gx3dParticleSystem particle_system);
void gx3d_UpdateParticleSystem    (gx3dParticleSystem particle_system, unsigned elapsed_time);
void gx3d_SetParticleSystemMatrix (gx3dParticleSystem particle_system, gx3dMatrix *m);
void gx3d_DrawParticleSystem      (gx3dParticleSystem particle_system, gx3dVector *view_normal, bool wireframe);

// GX3D_QUATERNION.CPP
void         gx3d_GetAxisAngleQuaternion (gx3dVector *axis, float angle, gx3dQuaternion *q);
void         gx3d_GetQuaternionAxisAngle (gx3dQuaternion *q, gx3dVector *axis, float *angle);
void         gx3d_GetMatrixQuaternion (gx3dMatrix *m, gx3dQuaternion *q);
void         gx3d_GetEulerQuaternion (float roll, float pitch, float yaw, gx3dQuaternion *q);
void         gx3d_GetQuaternionMatrix (gx3dQuaternion *q, gx3dMatrix *m);
void         gx3d_MultiplyQuaternion (gx3dQuaternion *q1, gx3dQuaternion *q2, gx3dQuaternion *qresult);
inline float gx3d_QuaternionDotProduct (gx3dQuaternion *q1, gx3dQuaternion *q2);
void         gx3d_GetLerpQuaternion (gx3dQuaternion *from, gx3dQuaternion *to, float amount, gx3dQuaternion *qresult);
void         gx3d_GetSlerpQuaternion (gx3dQuaternion *from, gx3dQuaternion *to, float amount, gx3dQuaternion *qresult);
inline void  gx3d_GetIdentityQuaternion (gx3dQuaternion *q);
int          gx3d_GetInverseQuaternion (gx3dQuaternion *q, gx3dQuaternion *qinverse);
void         gx3d_GetConjugateQuaternion (gx3dQuaternion *q, gx3dQuaternion *qconjugate);
inline void  gx3d_NormalizeQuaternion (gx3dQuaternion *q);
inline void  gx3d_NormalizeQuaternion (gx3dQuaternion *q, gx3dQuaternion *qnormal);
void         gx3d_MultiplyVectorQuaternion (gx3dVector *v, gx3dQuaternion *q, gx3dVector *vresult);
void         gx3d_ScaleQuaternion (gx3dQuaternion *q, float amount, gx3dQuaternion *qresult); // doesn't scale the w component
void         gx3d_SubtractQuaternion (gx3dQuaternion *q1, gx3dQuaternion *q2, gx3dQuaternion *qresult);

// GX3D_RELATION.CPP
       gxRelation gx3d_Relation_Point_Plane (gx3dVector *point, gx3dPlane *plane, float proximity);
inline gxRelation gx3d_Relation_Line_Plane (gx3dLine *line, gx3dPlane *plane);
inline gxRelation gx3d_Relation_Ray_Plane (gx3dRay *ray, gx3dPlane *plane);
inline gxRelation gx3d_Relation_Ray_Plane (gx3dRay *ray, float ray_length, gx3dPlane *plane);
       gxRelation gx3d_Relation_Sphere_Plane (gx3dSphere *sphere, gx3dPlane *plane);
       gxRelation gx3d_Relation_Box_Plane (gx3dBox *box, gx3dPlane *plane);
       gxRelation gx3d_Relation_Box_Plane (gx3dBox *box, gx3dMatrix *box_transform, gx3dPlane *plane);
       gxRelation gx3d_Relation_Triangle_Plane (gx3dVector *vertices, gx3dPlane *plane);
inline gxRelation gx3d_Relation_Point_Sphere (gx3dVector *point, gx3dSphere *sphere);
       gxRelation gx3d_Relation_Line_Sphere (gx3dLine *line, gx3dSphere *sphere);
inline gxRelation gx3d_Relation_Ray_Sphere (gx3dRay *ray, gx3dSphere *sphere);
inline gxRelation gx3d_Relation_Ray_Sphere (gx3dRay *ray, float ray_length, gx3dSphere *sphere);
       gxRelation gx3d_Relation_Sphere_Sphere (gx3dSphere *s1, gx3dSphere *s2, bool exact);
       gxRelation gx3d_Relation_Box_Sphere (gx3dBox *box, gx3dSphere *sphere);
       gxRelation gx3d_Relation_Triangle_Sphere (gx3dVector *vertices, gx3dSphere *sphere);
inline gxRelation gx3d_Relation_Point_Box (gx3dVector *point, gx3dBox *box);
inline gxRelation gx3d_Relation_Ray_Box (gx3dRay *ray, gx3dBox *box);
inline gxRelation gx3d_Relation_Ray_Box (gx3dRay *ray, float ray_length, gx3dBox *box);
inline gxRelation gx3d_Relation_Box_Box (gx3dBox *box1, gx3dBox *box2);
       gxRelation gx3d_Relation_Triangle_Box (gx3dVector *vertices, gx3dBox *box);
inline gxRelation gx3d_Relation_Ray_Triangle (gx3dRay *ray, gx3dVector *vertices);
inline gxRelation gx3d_Relation_Ray_Triangle (gx3dRay *ray, float ray_length, gx3dVector *vertices);
inline gxRelation gx3d_Relation_Ray_TriangleFront (gx3dRay *ray, gx3dVector *vertices);
inline gxRelation gx3d_Relation_Ray_TriangleFront (gx3dRay *ray, float ray_length, gx3dVector *vertices);
       gxRelation gx3d_Relation_Triangle_Triangle (gx3dVector *vertices1, gx3dVector *vertices2);
       gxRelation gx3d_Relation_Point_Frustum (gx3dVector *point);
       gxRelation gx3d_Relation_Point_Frustum (gx3dVector *point, gx3dViewFrustum *vf);
       gxRelation gx3d_Relation_Sphere_Frustum (gx3dSphere *sphere);
       gxRelation gx3d_Relation_Sphere_Frustum (gx3dSphere *sphere, gx3dViewFrustum *vf);
       gxRelation gx3d_Relation_Sphere_Frustum (gx3dSphere *sphere, gx3dFrustumOrientation *orientation);
       gxRelation gx3d_Relation_Sphere_Frustum (gx3dSphere *sphere, gx3dViewFrustum *vf, gx3dFrustumOrientation *orientation);
       gxRelation gx3d_Relation_Box_Frustum (gx3dBox *box, gx3dMatrix *box_transform);
       gxRelation gx3d_Relation_Box_Frustum (gx3dBox *box, gx3dWorldFrustum *wf, gx3dFrustumOrientation *orientation);

// GX3D_NEAREST.CPP
void gx3d_Nearest_Point_Line     (gx3dVector *point, gx3dLine *line, gx3dVector *nearest_point);
void gx3d_Nearest_Point_Ray      (gx3dVector *point, gx3dRay *ray, gx3dVector *nearest_point);
void gx3d_Nearest_Point_Ray      (gx3dVector *point, gx3dRay *ray, float ray_length, gx3dVector *nearest_point);
void gx3d_Nearest_Point_Plane    (gx3dVector *point, gx3dPlane *plane, gx3dVector *nearest_point);
void gx3d_Nearest_Point_Sphere   (gx3dVector *point, gx3dSphere *sphere, gx3dVector *nearest_point);
void gx3d_Nearest_Point_Box      (gx3dVector *point, gx3dBox *box, gx3dVector *nearest_point);
void gx3d_Nearest_Point_Triangle (gx3dVector *point, gx3dVector *vertices, gx3dVector *nearest_point);

// GX3D_DISTANCE.CPP
inline float gx3d_Distance_Point_Point        (gx3dVector *p1, gx3dVector *p2);
inline float gx3d_DistanceSquared_Point_Point (gx3dVector *p1, gx3dVector *p2);
       float gx3d_Distance_Point_Line         (gx3dVector *point, gx3dLine *line);
       float gx3d_Distance_Point_Ray          (gx3dVector *point, gx3dRay *ray);
       float gx3d_Distance_Point_Ray          (gx3dVector *point, gx3dRay *ray, float ray_length);
inline float gx3d_Distance_Point_Plane        (gx3dVector *point, gx3dPlane *plane);
inline float gx3d_Distance_Point_Sphere       (gx3dVector *point, gx3dSphere *sphere);
       float gx3d_Distance_Point_Box          (gx3dVector *point, gx3dBox *box);
       float gx3d_Distance_Point_Triangle     (gx3dVector *point, gx3dVector *vertices);

// GX3D_INTERSECT.CPP
gxRelation gx3d_Intersect_Rect_Rect (gx3dRectangle *r1, gx3dRectangle *r2, gx3dRectangle *intersection_rect);
gxRelation gx3d_Intersect_Line_Line (gx3dLine *l1, gx3dLine *l2, float proximity, gx3dVector *intersection);
gxRelation gx3d_Intersect_Line_Plane (gx3dLine *line, gx3dPlane *plane, gx3dVector *intersection);
gxRelation gx3d_Intersect_Ray_Ray (gx3dRay *r1, gx3dRay *r2, float proximity, gx3dVector *intersection);
gxRelation gx3d_Intersect_Ray_Ray (
  gx3dRay    *r1, 
  gx3dRay    *r2, 
  float       r2_length, 
  float       proximity, 
  gx3dVector *intersection );
gxRelation gx3d_Intersect_Ray_Ray (
  gx3dRay    *r1, 
  float       r1_length,
  gx3dRay    *r2, 
  float       r2_length, 
  float       proximity, 
  gx3dVector *intersection );
gxRelation gx3d_Intersect_Ray_Plane (
  gx3dRay    *ray, 
  gx3dPlane  *plane, 
  float      *distance, 
  gx3dVector *intersection );
gxRelation gx3d_Intersect_Ray_Plane (
  gx3dRay    *ray, 
  float       ray_length,
  gx3dPlane  *plane, 
  float      *distance, 
  gx3dVector *intersection );
gxRelation gx3d_Intersect_Ray_Sphere (
  gx3dRay    *ray, 
  gx3dSphere *sphere, 
  float      *distance, 
  gx3dVector *intersection );
gxRelation gx3d_Intersect_Ray_Sphere (
  gx3dRay    *ray, 
  float       ray_length,
  gx3dSphere *sphere, 
  float      *distance, 
  gx3dVector *intersection );
gxRelation gx3d_Intersect_Ray_Box (gx3dRay *ray, gx3dBox *box, float *distance, gx3dVector *intersection);
gxRelation gx3d_Intersect_Ray_Box (gx3dRay *ray, float ray_length, gx3dBox *box, float *distance, gx3dVector *intersection);
gxRelation gx3d_Intersect_Ray_Triangle (
  gx3dRay    *ray, 
  gx3dVector *vertices, 
  float      *distance,
  gx3dVector *intersection,
  float      *barycentric_u,
  float      *barycentric_v );
gxRelation gx3d_Intersect_Ray_Triangle (
  gx3dRay    *ray, 
  float       ray_length,
  gx3dVector *vertices,
  float      *distance,
  gx3dVector *intersection,
  float      *barycentric_u,
  float      *barycentric_v );
gxRelation gx3d_Intersect_Ray_TriangleFront (
  gx3dRay    *ray, 
  gx3dVector *vertices,
  float      *distance,
  gx3dVector *intersection,
  float      *barycentric_u,
  float      *barycentric_v );
gxRelation gx3d_Intersect_Ray_TriangleFront (
  gx3dRay    *ray, 
  float       ray_length,
  gx3dVector *vertices,
  float      *distance,
  gx3dVector *intersection,
  float      *barycentric_u,
  float      *barycentric_v );
gxRelation gx3d_Intersect_Box_Box (gx3dBox *box1, gx3dBox *box2, gx3dBox *intersection_box);

// GX3D_COLLIDE.CPP
gxRelation gx3d_Collide_Sphere_StaticPlane (
  gx3dSphere     *sphere,
  gx3dTrajectory *trajectory,
  float           dtime,                        // period of time sphere moves
  gx3dPlane      *plane,
  float          *parametric_collision_time );  // NULL if not needed
gxRelation gx3d_Collide_Sphere_StaticPlane (
  gx3dSphere              *sphere,
  gx3dProjectedTrajectory *ptrajectory,
  gx3dPlane               *plane,
  float                   *parametric_collision_time ); // NULL if not needed
gxRelation gx3d_Collide_Sphere_StaticSphere (
  gx3dSphere     *sphere,
  gx3dTrajectory *trajectory1,
  float           dtime,                        // period of time spheres move
  gx3dSphere     *static_sphere,
  float          *parametric_collision_time );  // NULL if not needed
gxRelation gx3d_Collide_Sphere_StaticSphere (
  gx3dSphere              *sphere,
  gx3dProjectedTrajectory *ptrajectory1,
  gx3dSphere              *static_sphere,
  float                   *parametric_collision_time ); // NULL if not needed
gxRelation gx3d_Collide_Sphere_Sphere (
  gx3dSphere     *sphere1,
  gx3dTrajectory *trajectory1,
  float           dtime,                        // period of time spheres move
  gx3dSphere     *sphere2,
  gx3dTrajectory *trajectory2,
  float          *parametric_collision_time );  // NULL if not needed
gxRelation gx3d_Collide_Sphere_Sphere (
  gx3dSphere              *sphere1,
  gx3dProjectedTrajectory *ptrajectory1,
  gx3dSphere              *sphere2,
  gx3dProjectedTrajectory *ptrajectory2,
  float                   *parametric_collision_time ); // NULL if not needed
gxRelation gx3d_Collide_Box_StaticPlane (
  gx3dBox        *box,
  gx3dTrajectory *trajectory,
  float           dtime,                        // period of time box moves
  gx3dPlane      *plane,
  float          *parametric_collision_time );  // NULL if not needed
gxRelation gx3d_Collide_Box_StaticPlane (
  gx3dBox                 *box,
  gx3dProjectedTrajectory *trajectory,
  gx3dPlane               *plane,
  float                   *parametric_collision_time ); // NULL if not needed
gxRelation gx3d_Collide_Box_StaticBox (
  gx3dBox        *box,
  gx3dTrajectory *trajectory,
  float           dtime,                        // period of time box moves
  gx3dBox        *static_box,
  float          *parametric_collision_time );  // NULL if not needed
gxRelation gx3d_Collide_Box_StaticBox (
  gx3dBox                 *box,
  gx3dProjectedTrajectory *ptrajectory,
  gx3dBox                 *static_box,
  float                   *parametric_collision_time ); // NULL if not needed
gxRelation gx3d_Collide_Box_Box (
  gx3dBox        *box1,
  gx3dTrajectory *trajectory1,
  float           dtime,                        // period of time box moves
  gx3dBox        *box2,
  gx3dTrajectory *trajectory2,
  float          *parametric_collision_time );  // NULL if not needed
gxRelation gx3d_Collide_Box_Box (
  gx3dBox                 *box1,
  gx3dProjectedTrajectory *ptrajectory1,
  gx3dBox                 *box2,
  gx3dProjectedTrajectory *ptrajectory2,
  float                   *parametric_collision_time ); // NULL if not needed

// GX3D_BV.CPP
void gx3d_GetBoundBox       (gx3dBox *box, gx3dVector *vertices, int num_vertices);
void gx3d_GetBoundBox       (gx3dBox *box, gx3dVector **vertices, int num_vertices);
void gx3d_GetBoundBox       (gx3dBox *new_box, gx3dBox *box1, gx3dBox *box2);
void gx3d_EncloseBoundBox   (gx3dBox *box, gx3dVector *vertices, int num_vertices);
void gx3d_EncloseBoundBox   (gx3dBox *box, gx3dVector **vertices, int num_vertices);
void gx3d_EncloseBoundBox   (gx3dBox *box, gx3dBox *box_to_enclose);
void gx3d_GetBoundBoxCenter (gx3dBox *box, gx3dVector *center);
void gx3d_TransformBoundBox (gx3dBox *box, gx3dMatrix *m, gx3dBox *new_box);

void gx3d_GetBoundSphere        (gx3dSphere *sphere, gx3dVector *vertices, int num_vertices);
void gx3d_GetBoundSphere        (gx3dSphere *sphere, gx3dVector *vertices, int num_vertices, gx3dBox *bound_box);
void gx3d_GetBoundSphere        (gx3dSphere *new_sphere, gx3dSphere *sphere1, gx3dSphere *sphere2);
void gx3d_GetOptimalBoundSphere (gx3dSphere *sphere, gx3dVector *vertices, int num_vertices);
void gx3d_EncloseBoundSphere    (gx3dSphere *sphere, gx3dVector *vertices, int num_vertices);
void gx3d_EncloseBoundSphere    (gx3dSphere *sphere, gx3dSphere *sphere_to_enclose);

// GX3D_BOXTREE.CPP
gx3dBoxtree *gx3d_Boxtree_Init (gx3dObject *object, gx3dBoxtreeType type);
void         gx3d_Boxtree_Free (gx3dBoxtree *boxtree);
// Sets dirty bit of a dynamic boxtree to true
void         gx3d_Boxtree_SetDirty (gx3dBoxtree *boxtree);
void         gx3d_Boxtree_Update (gx3dBoxtree *boxtree);
gxRelation   gx3d_Boxtree_Intersect_Ray (
  gx3dBoxtree *boxtree,         
  gx3dRay     *ray,
  float        ray_length,     
  float       *distance,        // optional
  gx3dVector  *intersection,    // optional
  char       **name );          // optional

#endif
