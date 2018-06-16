/*____________________________________________________________________
|
| File: d3d9_dp.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifdef _D3D9_2D_CPP_
#define GLOBAL /* */
#else
#define GLOBAL extern
#endif

/*____________________       
|
|	Type definitions
|___________________*/

typedef struct {
	int width, height, depth, rate;
  D3DFORMAT format;
  char *name;
} VideoModeInfo;

/*____________________
|
| Constants
|___________________*/

#define MAX_VIDEO_PAGES     32

// logic ops - same constants as in dx9.h
#define DD_OP_SET 	    0
#define DD_OP_AND	      1
#define DD_OP_OR	      2
#define DD_OP_XOR	      3
#define DD_OP_ADD       4
#define DD_OP_SUBTRACT  5
#define DD_OP_SHL       6
#define DD_OP_SHR       7
#define DD_OP_MULTIPLY  8

// screen size/depth
#define SCREEN_DX			d3d9_video_modes[d3d9_current_video_mode].width
#define SCREEN_DY			d3d9_video_modes[d3d9_current_video_mode].height
#define SCREEN_DEPTH	d3d9_video_modes[d3d9_current_video_mode].depth
#define SCREEN_FORMAT d3d9_video_modes[d3d9_current_video_mode].format

/*____________________
|
| Macros
|___________________*/

#define SURFACE_BUFFER ((byte *)(locked_rect.pBits))
#define SURFACE_PITCH  (locked_rect.Pitch)

/*____________________           
|
| Global variables
|___________________*/

GLOBAL LPDIRECT3DDEVICE9   d3ddevice9;              // pointer to the D3D device interface
GLOBAL LPDIRECT3DSURFACE9  d3dzbuffer9;             // zbuffer
GLOBAL LPDIRECT3DSURFACE9  d3dscreen9;
GLOBAL LPDIRECT3DSURFACE9  d3dcursor9;              // mouse cursor

GLOBAL VideoModeInfo      *d3d9_video_modes;        // array of available video modes
GLOBAL int                 d3d9_current_video_mode; // current mode (index into video_modes array)
GLOBAL int                 d3d9_current_logic_op;
GLOBAL DWORD               d3d9_current_color;
GLOBAL int                 d3d9_pixel_size;         // size of pixel in bytes

GLOBAL WORD	 d3d9_loREDbit,   d3d9_numREDbits;			// Info about pixel format for current video mode
GLOBAL WORD	 d3d9_loGREENbit, d3d9_numGREENbits;
GLOBAL WORD	 d3d9_loBLUEbit,  d3d9_numBLUEbits;
GLOBAL WORD  d3d9_loALPHAbit, d3d9_numALPHAbits;
GLOBAL DWORD d3d9_REDmask, d3d9_GREENmask, d3d9_BLUEmask, d3d9_ALPHAmask;
