/*____________________________________________________________________
|
| File: dp.h (gx_vista.h)
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define VERSION_STR     "1.0"

#ifdef _GX_W7_CPP_
#define GLOBAL /* */
#else
#define GLOBAL extern
#endif

//#define USING_DIRECTX_5
//#define USING_DIRECTX_8
#define USING_DIRECTX_9

/*____________________
|
| Include files
|___________________*/

#include <windows.h>
#include <winbase.h>  // for Beep(), Sleep()

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;

#include <stdio.h>
#include <stdlib.h>        
#include <conio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <time.h>

#include <defines.h>
#include <events.h>
#include <clib.h>
#include <win_support.h>
#ifdef USING_DIRECTX_5
#include <dx5.h>
#endif
#ifdef USING_DIRECTX_8
#include <dx8.h>
#endif
#ifdef USING_DIRECTX_9
#include <dx9.h>
#endif

#include "gx_w7.h"
#include "gx3d_globals.h"

/*____________________           
|
| Type definitions
|___________________*/

typedef struct {
  int      driver;      // see gx_vista.h 
  unsigned resolution;  // see gx_vista.h 
  unsigned bitdepth;    // see gx_vista.h
  unsigned redmask, greenmask, bluemask;
  int      low_redbit, low_greenbit, low_bluebit;
  int      num_redbits, num_greenbits, num_bluebits;

  void     (*free_driver) (void);
  void     (*vert_retrace_delay) (void);
  int      (*restore_directx) (void);
  int      (*create_virtual_page) (int width, int height, int create_in_vram);
  void     (*free_virtual_page) (int page);
  int      (*set_active_page) (int page);
  int      (*set_visual_page) (int page, int wait_for_vsync);
  void     (*flip_visual_page) (void);
  void     (*set_fore_color) (byte r, byte g, byte b, byte a); 
  void     (*set_logic_op) (int logic_op);
  void     (*draw_pixel) (int x, int y);
  void     (*get_pixel) (int x, int y, byte *r, byte *g, byte *b);
  void     (*draw_line) (int x1, int y1, int x2, int y2);
  void     (*draw_fill_rectangle) (int x1, int y1, int x2, int y2);
  // function for drawing a non-complex polygon where num_points <= 4 
  void     (*draw_fill_poly) (int num_points, int *points);
  // image functions 
  void     (*put_image) (
             byte *image,
             int   image_dx,
             int   image_dy,
             int   image_x,
             int   image_y,
             int   x,
             int   y,
             int   dx,
             int   dy,
             int   or_image );
  void     (*get_image) (
             byte *image,
             int   image_dx,
             int   image_dy,
             int   image_x,
             int   image_y,
             int   x,
             int   y,
             int   dx,
             int   dy );
  void     (*copy_image) (
             int src_x,
             int src_y,
             int src_page,
             int dst_x,
             int dst_y,
             int dst_page,
             int dx,
             int dy );
  void     (*copy_image_colorkey) (
             int  src_x,
             int  src_y,
             int  src_page,
             int  dst_x,
             int  dst_y,
             int  dst_page,
             int  dx,
             int  dy,
             byte r,
             byte g,
             byte b );
  void     (*put_bitmap) (
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
  // 3D functions
  int      (*begin_render) (void);
  int      (*end_render) (void);
  void     (*set_fill_mode) (int fill_mode);
  void     (*get_driver_info) (
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
  void     (*register_object) (
             word   *surface,         // array of indeces (group of 3 unsigned short)
             int    *num_polygons,    // # of indeces
             float  *vertex,          // array of vertices (group of 3 float - x,y,z)
             float **X_vertex,        // array of vertices
             int    *num_vertices,    // # of vertices
             float  *vertex_normal,   // array of vertex normals (group of 3 float - x,y,z)
             float **X_vertex_normal, // array of vertex normals
             byte   *diffuse,         // array of diffuse color in 0xAABBGGRR format
             byte   *specular,        // array of specular color in 0xAABBGGRR format
             float **texture_coord,   // 0,1 or 2 arrays of texture coords (group of 2 float - u,v)
             float **X_texture_coord,
             float **texture_coord_w,
             float **X_texture_coord_w,
             byte   *weight,          // array of weights (group of 2 float + 4 bytes)
             byte  **X_weight,
             void  **driver_data );   // address of a pointer to driver-specific data
  void     (*unregister_object) (void *driver_data);
  void     (*draw_object) (void *driver_data);
  void     (*optimize_object) (void *driver_data);
  int      (*set_viewport) (int left, int top, int right, int bottom);
  void     (*clear_viewport_rectangle)(
             int      *rect, 
             unsigned  flags, // 0x1=surface, 0x2=zbuffer, 0x4=stencil
             byte      surface_color_r, 
             byte      surface_color_g, 
             byte      surface_color_b, 
             byte      surface_color_a, 
             float     zval, 
             unsigned  stencilval );
  void     (*enable_clipping) (int flag);
  unsigned (*init_clip_plane) (float a, float b, float c, float d);
  void     (*free_clip_plane) (unsigned plane);
  void     (*enable_clip_plane) (unsigned plane, int flag);
  int      (*set_world_matrix) (void *m);
  int      (*get_world_matrix) (void *m);
  int      (*set_view_matrix) (void *m);
  int      (*get_view_matrix) (void *m);
  int      (*set_projection_matrix) (void *m);
  int      (*get_projection_matrix) (void *m);
  int      (*enable_texture_matrix) (int stage, int dimension, int flag);
  int      (*set_texture_matrix) (int stage, void *m);
  int      (*get_texture_matrix) (int stage, void *m);
  void     (*enable_zbuffer) (int flag);
  void     (*set_z_bias) (int bias);
  void     (*enable_backface_removal) (int flag);

  void     (*enable_stencil_buffer) (int flag);
  void     (*set_stencil_fail_op) (int stencil_op);
  void     (*set_stencil_zfail_op) (int stencil_op);
  void     (*set_stencil_pass_op) (int stencil_op);
  void     (*set_stencil_comparison) (int stencil_function);
  void     (*set_stencil_reference_value) (unsigned reference_value);
  void     (*set_stencil_mask) (unsigned mask);
  void     (*set_stencil_write_mask) (unsigned mask);

  void     (*enable_lighting) (int flag);
  unsigned (*init_point_light) (
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
  void     (*update_point_light) (
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
  unsigned (*init_spot_light) (
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
  void     (*update_spot_light) (
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
  unsigned (*init_direction_light) (
             float     dst_x,
             float     dst_y,
             float     dst_z, 
             float    *ambient_color_rgba,
             float    *diffuse_color_rgba,
             float    *specular_color_rgba );
  void     (*update_direction_light) (
             unsigned  light,
             float     dst_x,
             float     dst_y,
             float     dst_z, 
             float    *ambient_color_rgba,
             float    *diffuse_color_rgba,
             float    *specular_color_rgba );
  void     (*free_light) (unsigned light);
  void     (*enable_light) (unsigned light, int flag);
  void     (*set_ambient_light) (float *rgba);
  void     (*enable_specular_lighting) (int flag);
  void     (*set_material) (
             float *ambient_color_rgba,
             float *diffuse_color_rgba,
             float *specular_color_rgba,
             float *emissive_color_rgba,
             float  specular_sharpness );
  void     (*get_material) (
             float *ambient_color_rgba,  
             float *diffuse_color_rgba,
             float *specular_color_rgba,
             float *emissive_color_rgba,
             float *specular_sharpness );
  byte *   (*init_texture) (
             int       num_mip_levels,
             byte    **image, 
             byte    **alphamap,
             int       dx, 
             int       dy, 
             int       num_color_bits,
             int       num_alpha_bits, 
             unsigned *size );
  byte *   (*init_volume_texture) (
             int       num_levels,
             int       num_slices,
             byte    **image, 
             byte    **alphamap,
             int       dx, 
             int       dy, 
             int       num_color_bits,
             int       num_alpha_bits, 
             unsigned *size );
  byte *   (*init_cubemap_texture) (
             byte    **image, 
             byte    **alphamap, 
             int       dimensions, 
             int       num_color_bits, 
             int       num_alpha_bits,
             unsigned *size );
  unsigned (*init_dynamic_texture) (int dx, int dy, int num_color_bits, int num_alpha_bits, unsigned *size);
  unsigned (*init_dynamic_cubemap_texture) (int dimensions, int num_color_bits, int num_alpha_bits, unsigned *size);
  void     (*free_texture) (byte *texture);
  void     (*free_dynamic_texture) (unsigned texture);
  void     (*set_texture) (int stage, byte *texture);
  void     (*set_dynamic_texture) (int stage, unsigned texture);
  void     (*set_texture_addressing_mode) (int stage, int dimension, int addressing_mode);
  void     (*set_texture_border_color) (int stage, byte r, byte g, byte b, byte a);
  void     (*set_texture_filtering) (int stage, int filter_type, int anisotropy_level);
  void     (*set_texture_coordinates) (int stage, int coordinate_stage);
  void     (*enable_cubemap_texture_reflections) (int flag);
  void     (*set_texture_wrapping) (int coordinate_stage, int wrap_s, int wrap_t, int wrap_r, int wrap_q);
  void     (*set_texture_factor) (byte r, byte g, byte b, byte a);
  void     (*preload_texture) (byte *texture);
  void     (*evict_all_textures) (void);
  void     (*enable_render_to_texture) (unsigned texture, int face);
  void     (*set_texture_colorop) (int stage, int texture_colorop, int texture_arg1, int texture_arg2);
  void     (*set_texture_alphaop) (int stage, int texture_colorop, int texture_arg1, int texture_arg2);
  void     (*set_texture_color_factor) (float *rgba);
  void     (*enable_antialiasing) (int flag);
  void     (*enable_vertex_lighting) (int flag);
  void     (*enable_fog) (int flag);
  void     (*set_fog_color) (byte r, byte g, byte b);
  void     (*set_linear_pixel_fog) (float start_distance, float end_distance);
  void     (*set_exp_pixel_fog) (float density);
  void     (*set_exp2_pixel_fog) (float density);
  void     (*set_linear_vertex_fog) (float start_distance, float end_distance, int ranged_based);
  void     (*enable_alpha_blending) (int flag);
  void     (*set_alpha_blend_factor) (int src_blend_factor, int dst_blend_factor);
  int      (*alpha_testing_available) (void);
  void     (*enable_alpha_testing) (int flag, byte reference_value);

} gxVideoDriver;

typedef struct {
  int   type;                     // screen or virtual                    
  int   width, height;            // dimensions
  byte *buffer;                   // ptr to virtual buffer, if page type is virtual 
  int   driver_page;              // driver page # (not necessarily the same as gx page #)
} gxPageInfo;

typedef struct {
  int     type;                   // 0=not valid, 1=bitmap, 2=image pattern 
  int     dx, dy;                 // dimensions of pattern (in pixels)    
  int     bytes_per_row;  
  byte   *data;                   // ptr to a bitmap or image data        
  gxColor fore_color, back_color; // colors (bitmap patterns only) 
  int     transparent_background; // boolean, 1=background color is not drawn
} gxPatternInfo;

/*____________________
|
| Constants
|___________________*/

#define NUM_STYLE_ELEMENTS 4

#define NUM_CUBEMAP_FACES 6

// page types
#define PAGE_TYPE_SCREEN          1
#define PAGE_TYPE_DRIVER_VIRTUAL  2
#define PAGE_TYPE_VIRTUAL         3

#define MAX_PAGES         100
#define MAX_PATTERNS      32

// pattern types
#define PATTERN_TYPE_SOLID  1
#define PATTERN_TYPE_BITMAP 2
#define PATTERN_TYPE_IMAGE  3
#define PATTERN_TYPE_BITMAP_TRANSPARENT 4 // same as bitmap with transparent background color

#define NUM_INDEXED_COLORS  256
#define MAX_PIXEL_SIZE      4   // in 32-bit color mode

#define ERROR_FILE "GXERROR.TXT"

#define PAGE_WIDTH  (gx_Page_list[gx_Active_page].width)
#define PAGE_HEIGHT (gx_Page_list[gx_Active_page].height)

/*____________________
|
| Macros
|___________________*/

//#define BEEP {sound(5000);delay(20);nosound();delay(50);}
#define BEEP {Beep(5000,20);Sleep(50);}

#define sleep(_n_)  Sleep(_n_*1000)

#define ZERO_COLOR(_color_) _color_.index = 0

/*___________________
|
| Global variables
|__________________*/

// system 
GLOBAL gxVideoDriver gx_Video;
GLOBAL gxVideoDriver gx_Video_save;     // place to save gx_Video temporarily 
GLOBAL gxRectangle   gx_Screen;
GLOBAL int           gx_Num_pages;
GLOBAL float         gx_Aspect_ratio;
GLOBAL gxPageInfo    gx_Page_list [MAX_PAGES];
GLOBAL gxPatternInfo gx_Pattern_list [MAX_PATTERNS];
GLOBAL byte          gx_Current_palette [256*3];
GLOBAL int           gx_Pixel_size;     // # bytes per pixel in current video mode (1, 2, 3 or 4)

// state variables 
GLOBAL gxRectangle   gx_Window;
GLOBAL gxRectangle   gx_Clip;
GLOBAL int           gx_Clipping;
GLOBAL gxPage        gx_Active_page;
GLOBAL gxPage        gx_Visual_page;
GLOBAL gxColor       gx_Fore_color;
GLOBAL int           gx_Line_width;
GLOBAL int           gx_Line_style_enabled;
GLOBAL int           gx_Line_style [NUM_STYLE_ELEMENTS];
GLOBAL int           gx_Line_style_index;
GLOBAL int           gx_Line_style_count;
GLOBAL int           gx_Logic_op;
GLOBAL gxPattern     gx_Fill_pattern;             // 0 - MAX_PATTERNS-1 

/*___________________
|
| 3D globals - always valid
|__________________*/

GLOBAL gxRectangle   gx3d_Viewport;
GLOBAL gx3dMatrix    gx3d_View_matrix;

GLOBAL float         gx3d_Projection_hfov;        // horizontal field of view in degress (0.1 - 179.9)
GLOBAL float         gx3d_Projection_vfov;        // vertical field of view in degress (0.1 - 179.9)
GLOBAL float         gx3d_Projection_near_plane;  // in world z units
GLOBAL float         gx3d_Projection_far_plane;   // in world z units

GLOBAL int           gx3d_Fill_mode;

GLOBAL char          gx3d_Texture_Directory[256]; // directory to load textures from (disabled if nullstring)

/*___________________
|
| 3D globals - updated on demand (see gx3d_globals.cpp)
|__________________*/

// When these globals are changed, they are not immediately updated but instead their dirty flag is set
// Functions using these globals test the dirty flag and update if dirty
// These globals are not always updated since that would not be efficient

// (View * Projection) matrix
GLOBAL gx3dMatrix gx3d_View_projection_matrix;
GLOBAL bool       gx3d_View_projection_matrix_dirty;

// View frustrum data
GLOBAL gx3dViewFrustum gx3d_View_frustum;
GLOBAL bool            gx3d_View_frustum_dirty;

#ifdef DEBUG
static char debug_str [256];
#endif
