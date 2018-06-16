/*____________________________________________________________________
|
| File: ms_w7.h
| Description: mouse library for use with GX_W7.
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _MS_W7_H_
#define _MS_W7_H_

/*___________________
|
| Function prototypes
|__________________*/

typedef int msCursor;

/*___________________
|
| Constants
|__________________*/

// drivers
#define msDRIVER_DX5              5   // DirectX 5
#define msDRIVER_DX8              8   // DirectX 8
#define msDRIVER_DX9              9   // DirectX 9

// tracking speed
#define msSPEED_VERY_SLOW   0
#define msSPEED_SLOW        1
#define msSPEED_NORMAL      2
#define msSPEED_FAST        3
#define msSPEED_VERY_FAST   4

// predefined cursors
#define msCURSOR_SMALL_ARROW      ((msCursor) 0 )
#define msCURSOR_MEDIUM_ARROW     ((msCursor) 1 )
#define msCURSOR_LARGE_ARROW      ((msCursor) 2 )
#define msCURSOR_SMALL_HAND       ((msCursor) 3 )
#define msCURSOR_LARGE_HAND       ((msCursor) 4 )
#define msCURSOR_SMALL_CROSSHAIR  ((msCursor) 5 )
#define msCURSOR_LARGE_CROSSHAIR  ((msCursor) 6 )
#define msCURSOR_SMALL_BUCKET     ((msCursor) 7 )
#define msCURSOR_LARGE_BUCKET     ((msCursor) 8 )
#define msCURSOR_SMALL_EYEDROPPER ((msCursor) 9 )
#define msCURSOR_LARGE_EYEDROPPER ((msCursor) 10)
#define msCURSOR_SMALL_STRETCHER  ((msCursor) 11)
#define msCURSOR_LARGE_STRETCHER  ((msCursor) 12)
#define msCURSOR_SMALL_WATCH      ((msCursor) 13)
#define msCURSOR_LARGE_WATCH      ((msCursor) 14)

// mouse actions
#define msMOVEMENT              0x1   // unsupported
#define msBUTTON_LEFT_PRESS     0x2
#define msBUTTON_LEFT_RELEASE   0x4
#define msBUTTON_RIGHT_PRESS    0x8
#define msBUTTON_RIGHT_RELEASE  0x10

/*___________________
|
| Function prototypes
|__________________*/

// ms_xp.cpp
int      msMouseExists (void);
int      msStartMouse (int cursor_auto_tracking, int driver);
void     msStopMouse (void);
void     msHideMouse (void);
void     msShowMouse (void);
void     msUpdateMouse (void);
void     msSetMouseSpeed (int speed);
void     msConfineMouse (int left, int top, int right, int bottom);
int      msGetMouseStatus (int *x, int *y, int *button);
void     msSetMouseCoords (int x, int y);
void     msGetMouseCoords (int *x, int *y);
void     msGetMouseMovement (int *x, int *y);
int      msMouseInBox (int left, int top, int right, int bottom, int x, int y);
                                        
void     msSetCursor (msCursor cursor, gxColor color, gxColor mask_color);
void     msGetCursor (msCursor *cursor, gxColor *color, gxColor *mask_color);
msCursor msDefineBitmapCursor (int hot_x, int hot_y, byte *screen_mask_bitmap, byte *cursor_mask_bitmap);
msCursor msDefineSpriteCursor (int hot_x, int hot_y, byte *sprite);

// cursor.cpp
void msCopyCursor (int page);
void msEraseCursor (void);

#endif
