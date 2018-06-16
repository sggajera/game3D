/*___________________________________________________________________
|
|	File: dinput9.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define DirectInput_Init                    dinput9_Init
#define DirectInput_Free                    dinput9_Free
#define DirectInput_Restore                 dinput9_Restore
#define DirectInput_Get_Event               dinput9_Get_Event
#define DirectInput_Flush_Events            dinput9_Flush_Events
#define DirectInput_Mouse_Hide              dinput9_Mouse_Hide
#define DirectInput_Mouse_Show              dinput9_Mouse_Show
#define DirectInput_Mouse_Get_Coords        dinput9_Mouse_Get_Coords
#define DirectInput_Mouse_Set_Coords        dinput9_Mouse_Set_Coords
#define DirectInput_Mouse_Get_Status        dinput9_Mouse_Get_Status
#define DirectInput_Mouse_Set_Image_Cursor  dinput9_Mouse_Set_Image_Cursor
#define DirectInput_Mouse_Set_Bitmap_Cursor dinput9_Mouse_Set_Bitmap_Cursor
#define DirectInput_Mouse_Confine           dinput9_Mouse_Confine
#define DirectInput_Mouse_Get_Movement      dinput9_Mouse_Get_Movement

int  dinput9_Init (int use_keyboard, int use_mouse);
void dinput9_Free ();
int  dinput9_Restore ();
int  dinput9_Get_Event (
	unsigned *type,				// type of event
	int			 *keycode,		// only valid for key events
	int			 *x,					// only valid for mouse events
	int			 *y, 
	unsigned *timestamp);	// system time in milliseconds
void dinput9_Flush_Events (unsigned event_type_mask);
void dinput9_Mouse_Hide ();
void dinput9_Mouse_Show ();
void dinput9_Mouse_Get_Coords (int *x, int *y);
void dinput9_Mouse_Set_Coords (int x, int y);
void dinput9_Mouse_Get_Status (int *x, int *y, int *left, int *right);
void dinput9_Mouse_Set_Image_Cursor (
	byte *image, 
	int   image_dx,
	int   image_dy,
	int		hot_x,
	int		hot_y );
void dinput9_Mouse_Set_Bitmap_Cursor (
	byte *cursor_bitmap, 
	byte *mask_bitmap,
	int   bitmap_dx,
	int   bitmap_dy,
	int		hot_x,
	int		hot_y,
	byte 	cursor_color_r,
	byte 	cursor_color_g,
	byte 	cursor_color_b,
	byte 	mask_color_r,
	byte 	mask_color_g,
	byte 	mask_color_b );
void dinput9_Mouse_Confine (int left, int top, int right, int bottom);
void dinput9_Mouse_Get_Movement (int *x, int *y);
