/*___________________________________________________________________
|
|	File: dinput9.cpp
|
|	Description: Functions that interface to DirectInput8.
|
| Functions:  dinput9_Init
|							 Keyboard_Init
|               Keyboard_Thread
|                Translate_Key
|							 Mouse_Init
|								Mouse_Thread
|							dinput9_Free
|							dinput9_Restore
|							dinput9_Get_Event
|							dinput9_Flush_Events
|							dinput9_Mouse_Hide
|							dinput9_Mouse_Show
|							dinput9_Mouse_Get_Coords
|							dinput9_Mouse_Set_Coords
|							dinput9_Mouse_Get_Status
|							dinput9_Mouse_Set_Image_Cursor
|							dinput9_Mouse_Set_Bitmap_Cursor
|              Adjust_Cursor_Size
|              Set_Cursor
|							dinput9_Mouse_Confine
|             dinput9_Mouse_Get_Movement
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define DIRECTINPUT_VERSION 0x0800

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
#include <malloc.h>
#include <process.h>
#include <ddraw.h>
#include <dinput.h>

#include <defines.h>
#include <clib.h>
#include <events.h>
#include <win_support.h>

#include "d3d9_2d.h"
#include "d3d9_3d.h"

#include "dinput9.h"

/*____________________
|
|	Type definitions
|___________________*/

typedef struct _inputdeviceinfo_ {
  DIDEVICEINSTANCE desc;
  struct _inputdeviceinfo_ *next;
} InputDeviceInfo;

/*____________________
|
| Function prototypes
|___________________*/

static int	Keyboard_Init ();
static DWORD WINAPI Keyboard_Thread (LPVOID pParam);
static int  Translate_Key (DWORD di_key);
static int	Mouse_Init ();
static DWORD WINAPI Mouse_Thread (LPVOID pParam);	
static int Adjust_Cursor_Size (int dx, int dy, int *cursor_dx, int *cursor_dy);
static void Set_Cursor ();

/*____________________
|
| Macros
|___________________*/

typedef unsigned (WINAPI *PBEGINTHREADEX_THREADFUNC)(LPVOID lpThreadParameter);
typedef unsigned *PBEGINTHREADEX_THREADID;

#define READY_EVENT_INDEX	0
#define QUIT_EVENT_INDEX	1

#define KEYBOARD_BUFFER_SIZE	64
#define MOUSE_BUFFER_SIZE			64

#define CURSOR_TYPE_IMAGE   1
#define CURSOR_TYPE_BITMAP  2

#define FREE_CURSOR_SURFACE Direct3D_Free_Cursor ();

#define FREE_SAVED_CURSOR                                 \
  {                                                       \
    if (dimouse_cursor_type == CURSOR_TYPE_IMAGE) {       \
      if (dimouse_cursor_image) {                         \
        free (dimouse_cursor_image);                      \
        dimouse_cursor_image = NULL;                      \
      }                                                   \
      dimouse_cursor_type = 0;                            \
    }                                                     \
    else if (dimouse_cursor_type == CURSOR_TYPE_BITMAP) { \
      if (dimouse_cursor_bitmap) {                        \
        free (dimouse_cursor_bitmap);                     \
        dimouse_cursor_bitmap = NULL;                     \
      }                                                   \
      if (dimouse_cursor_bitmask) {                       \
        free (dimouse_cursor_bitmask);                    \
        dimouse_cursor_bitmask = NULL;                    \
      }                                                   \
      dimouse_cursor_type = 0;                            \
    }                                                     \
  }

/*____________________
|
|	Global variables
|___________________*/
																				
static struct {
	DWORD di_key;
	int   key;
} dikeyboard_translate_table [] = {
	{ DIK_A,						'a'								},
	{ DIK_B,						'b'								},
	{ DIK_C,						'c'								},
	{ DIK_D,						'd'								},
	{ DIK_E,						'e'								},
	{ DIK_F,						'f'								},
	{ DIK_G,						'g'								},
	{ DIK_H,						'h'								},
	{ DIK_I,						'i'								},
	{ DIK_J,						'j'								},
	{ DIK_K,						'k'								},
	{ DIK_L,						'l'								},
	{ DIK_M,						'm'								},
	{ DIK_N,						'n'								},
	{ DIK_O,						'o'								},
	{ DIK_P,						'p'								},
	{ DIK_Q,						'q'								},
	{ DIK_R,						'r'								},
	{ DIK_S,						's'								},
	{ DIK_T,						't'								},
	{ DIK_U,						'u'								},
	{ DIK_V,						'v'								},
	{ DIK_W,						'w'								},
	{ DIK_X,						'x'								},
	{ DIK_Y,						'y'								},
	{ DIK_Z,						'z'								},
	{ DIK_0,						'0'								},
	{ DIK_1,						'1'								},
	{ DIK_2,						'2'								},
	{ DIK_3,						'3'								},
	{ DIK_4,						'4'								},
	{ DIK_5,						'5'								},
	{ DIK_6,						'6'								},
	{ DIK_7,						'7'								},
	{ DIK_8,						'8'								},
	{ DIK_9,						'9'								},
	{ DIK_NUMPAD0,			'0'								},
	{ DIK_NUMPAD1,			'1'								},
	{ DIK_NUMPAD2,			'2'								},
	{ DIK_NUMPAD3,			'3'								},
	{ DIK_NUMPAD4,			'4'								},
	{ DIK_NUMPAD5,			'5'								},
	{ DIK_NUMPAD6,			'6'								},
	{ DIK_NUMPAD7,			'7'								},
	{ DIK_NUMPAD8,			'8'								},
	{ DIK_NUMPAD9,			'9'								},
	{ DIK_SPACE,				' '								},
	{ DIK_MINUS,				'-'								},
	{ DIK_EQUALS,				'='								},
	{ DIK_PERIOD,				'.'								},
	{ DIK_COMMA,				','								},
	{ DIK_SLASH,				'/'								},
	{ DIK_ADD,					'+'								},
	{ DIK_SUBTRACT,			'-'								},
	{ DIK_DECIMAL,			'.'								},
	{ DIK_SEMICOLON,		';'								},
	{ DIK_LBRACKET,			'['								},
	{ DIK_RBRACKET,			']'								},
	{ DIK_DIVIDE,				'/'								},
	{ DIK_MULTIPLY,			'*'								},
	{ DIK_BACKSLASH,	  '\\'							},
	{ DIK_APOSTROPHE,	  '\''							},
	{ DIK_UP,						evKY_UP_ARROW			},
	{ DIK_DOWN,					evKY_DOWN_ARROW		},
	{ DIK_LEFT,					evKY_LEFT_ARROW		},
	{ DIK_RIGHT,				evKY_RIGHT_ARROW	},
	{ DIK_PGUP,					evKY_PAGE_UP			},
	{	DIK_PGDN,					evKY_PAGE_DOWN		},
	{ DIK_RETURN,				evKY_ENTER				},
	{ DIK_HOME,					evKY_HOME					},
	{ DIK_END,					evKY_END					},
	{ DIK_INSERT,				evKY_INSERT				},
	{ DIK_DELETE,				evKY_DELETE				},
	{ DIK_PAUSE,				evKY_PAUSE				},
	{ DIK_LMENU,				evKY_ALT					},
	{ DIK_RMENU,				evKY_ALT					},
	{ DIK_LCONTROL,			evKY_CONTROL			},
	{ DIK_RCONTROL,			evKY_CONTROL			},
	{ DIK_LSHIFT,				evKY_SHIFT				},
	{ DIK_RSHIFT,				evKY_SHIFT				},
	{ DIK_NUMPADENTER,	evKY_ENTER				},
	{ DIK_ESCAPE,				evKY_ESC					},
	{ DIK_BACK,					evKY_BACKSPACE		},
	{ DIK_TAB,					evKY_TAB					},
	{ DIK_F1,						evKY_F1						},
	{ DIK_F2,						evKY_F2						},
	{ DIK_F3,						evKY_F3						},
	{ DIK_F4,						evKY_F4						},
	{ DIK_F5,						evKY_F5						},
	{ DIK_F6,						evKY_F6						},
	{ DIK_F7,						evKY_F7						},
	{ DIK_F8,						evKY_F8						},
	{ DIK_F9,						evKY_F9						},
	{ DIK_F10,					evKY_F10					},
	{ DIK_F11,					evKY_F11					},
	{ DIK_F12,					evKY_F12					},
	{ 0,								0									}	// last entry must be 0,0
};

// Pointer to DirectInput interface
static LPDIRECTINPUT8 dinput8;

static LPDIRECTINPUTDEVICE8 dimouse;
static HANDLE 				      dimouse_thread;
static CRITICAL_SECTION     dimouse_critsection;
static HANDLE               dimouse_event[2];
static int								  dimouse_visible;

static int								  dimouse_x;
static int								  dimouse_y;
static int								  dimouse_last_x;
static int								  dimouse_last_y;
static int								  dimouse_min_x;
static int								  dimouse_min_y;
static int								  dimouse_max_x;
static int								  dimouse_max_y;
static int								  dimouse_rel_x;
static int								  dimouse_rel_y;

static LPDIRECTINPUTDEVICE8 dikeyboard;
static HANDLE     				  dikeyboard_thread;
static CRITICAL_SECTION		  dikeyboard_critsection;
static HANDLE               dikeyboard_event[2];

// Info about current cursor, if any
static int   dimouse_cursor_type;
static byte *dimouse_cursor_image;
static byte *dimouse_cursor_bitmap;
static byte *dimouse_cursor_bitmask;
static int   dimouse_cursor_dx;
static int   dimouse_cursor_dy;
static int   dimouse_cursor_hotx;
static int   dimouse_cursor_hoty;
static int   dimouse_cursor_bitmap_color_r;
static int   dimouse_cursor_bitmap_color_g;
static int   dimouse_cursor_bitmap_color_b;
static int   dimouse_cursor_bitmask_color_r;
static int   dimouse_cursor_bitmask_color_g;
static int   dimouse_cursor_bitmask_color_b;

/*___________________________________________________________________
|
|	Function: dinput9_Init
| 
|	Input: Called from ____
|
| Output: Initializes DirectInput and keyboard and mouse devices.  
|					Returns true on success or false on any error.
|
| Description: Mouse cursor begins hidden. 
|
| Note: Currently, keyboard device does nothing, later may want to add
|				a function to get immediate keyboard data.
|___________________________________________________________________*/

int dinput9_Init (int use_keyboard, int use_mouse)
{
	int error;
	int initialized = FALSE;

/*___________________________________________________________________
|
| Init globals
|___________________________________________________________________*/

  dinput8     = 0;
  dikeyboard  = 0;
  dimouse     = 0;

  // Init mouse stuff
	dimouse_thread										= 0;
	dimouse_event[READY_EVENT_INDEX]	= 0;
	dimouse_event[QUIT_EVENT_INDEX]		= 0;
	dimouse_visible										= FALSE;
	dimouse_x													= 0;
	dimouse_y													= 0;
	dimouse_last_x										= -100000;
	dimouse_last_y										= -100000;
  dimouse_min_x											= 0;
	dimouse_min_y											= 0;
	dimouse_max_x											= 0;
	dimouse_max_y											= 0;
  dimouse_cursor_type               = 0;

  // Init keyboard stuff
	dikeyboard_thread										= 0;
	dikeyboard_event[READY_EVENT_INDEX]	= 0;
	dikeyboard_event[QUIT_EVENT_INDEX]	= 0;

/*___________________________________________________________________
|
| Init DirectInput
|___________________________________________________________________*/
  
  // Get a ptr to DirectInput interface
	if (DirectInput8Create (win_Get_Instance_Handle(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&dinput8, NULL) == DI_OK) {
		error = FALSE;
		// Initialize keyboard?
		if (use_keyboard)
			error = (NOT Keyboard_Init ());
		// Initialize mouse?
		if (use_mouse)
			error	= (NOT Mouse_Init ());
		
		// All devices initialized ok?
		if (NOT error) {
			// Acquire keyboard?
			if (dikeyboard)
				dikeyboard->Acquire ();
        // Start keyboard thread processing
        ResumeThread (dikeyboard_thread);
			// Acquire mouse?
			if (dimouse) {
				dimouse->Acquire ();
				// Start mouse thread processing
				ResumeThread (dimouse_thread);
			}
			initialized = TRUE;
		}
	}

	// If any error, free all interfaces
	if (NOT initialized)
		dinput9_Free ();
  
  return (initialized);
}

/*___________________________________________________________________
|
|	Function: Keyboard_Init
| 
|	Input: Called from dinput9_Init()
|
| Output: Initializes DirectInput keyboard device.  Returns true on 
|					success or false on any error.
|___________________________________________________________________*/

static int Keyboard_Init ()
{
	DIDEVCAPS caps;
  DIPROPDWORD prop;
	DWORD dwThreadID = 0;
	int initialized = FALSE;

	// Has DirectInput been initialized?
	if (dinput8) {
		// Create a keyboard device
		if (dinput8->CreateDevice (GUID_SysKeyboard, &dikeyboard, NULL) == DI_OK) {
			// Get device capabilities
			ZeroMemory (&caps, sizeof(caps));
			caps.dwSize = sizeof(caps);
			if (dikeyboard->GetCapabilities (&caps) == DI_OK) {
				// Is device physically attached?
				if (caps.dwFlags & DIDC_ATTACHED) {
					// Set data format
					if (dikeyboard->SetDataFormat (&c_dfDIKeyboard) == DI_OK) {
						// Set cooperative level
						if (dikeyboard->SetCooperativeLevel (win_Get_Window_Handle(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) == DI_OK) {
							// Set buffer size
							ZeroMemory (&prop, sizeof(prop));
							prop.diph.dwSize = sizeof(prop);
							prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
							prop.diph.dwObj = 0;
							prop.diph.dwHow = DIPH_DEVICE;
							prop.dwData = KEYBOARD_BUFFER_SIZE;
							if (dikeyboard->SetProperty (DIPROP_BUFFERSIZE, &prop.diph) == DI_OK) {
								// Create 2 events
								dikeyboard_event[READY_EVENT_INDEX] = CreateEvent (0, 0, 0, 0);
								dikeyboard_event[QUIT_EVENT_INDEX]  = CreateEvent (0, 0, 0, 0);
                // Create a critical section
                InitializeCriticalSection (&dikeyboard_critsection);
                // Were events created successfully?
								if (dikeyboard_event[READY_EVENT_INDEX] AND dikeyboard_event[QUIT_EVENT_INDEX]) {
									// Init event notification
									if (dikeyboard->SetEventNotification (dikeyboard_event[READY_EVENT_INDEX]) == DI_OK) {
										// Create the thread
                    dikeyboard_thread = (HANDLE)_beginthreadex (NULL,
                                                                1024,
                                                                (PBEGINTHREADEX_THREADFUNC)Keyboard_Thread,
                                                                NULL,
                                                                CREATE_SUSPENDED,
                                                                (PBEGINTHREADEX_THREADID)&dwThreadID);
                    if (dikeyboard_thread) {
                      SetThreadPriority (dikeyboard_thread, THREAD_PRIORITY_NORMAL);
											initialized = TRUE;
                    }
									}
								}
							}
						}
					}
				}
			}
		}
	}

  return (initialized);
}

/*___________________________________________________________________
|
|	Function: Keyboard_Thread
| 
|	Input: Called from Keyboard_Init()
| Output: This function is a seperate worker thread to process keyboard
|					input.
|___________________________________________________________________*/

static DWORD WINAPI Keyboard_Thread (LPVOID pParam)
{
  static int key;
  static DIDEVICEOBJECTDATA data;
	static DWORD event, num_events;
	static HRESULT hres;
	static BOOL buffer_empty;

  // Block until one of the 2 events becomes signaled
  while ((event = WaitForMultipleObjects (2, dikeyboard_event, FALSE, INFINITE)) != WAIT_FAILED) {							
		// If quit event signaled, then quit out of this thread function
		if ((event-WAIT_OBJECT_0) == QUIT_EVENT_INDEX) {
      ResetEvent (dikeyboard_event[QUIT_EVENT_INDEX]);
			return 0;
		}
    else 
      ResetEvent (dikeyboard_event[READY_EVENT_INDEX]);

		// Lock critical section to synch use of shared variables
		EnterCriticalSection (&dikeyboard_critsection);

		buffer_empty = FALSE;
		// Keep processing events until buffer is empty
		while (NOT buffer_empty) {
			num_events = 1;
			// Make sure device is acquired
			hres = dikeyboard->Acquire ();
			// Is device acquired?
			if ((hres == DI_OK) OR (hres == S_FALSE)) {
				// Retrieve 1 event from buffer
				hres = dikeyboard->GetDeviceData (sizeof(DIDEVICEOBJECTDATA), &data, &num_events, 0);
				if ((hres == DI_OK) AND (num_events == 1)) {
					static EventQueueEntry qentry;
          // Is this a valid key
          key = Translate_Key (data.dwOfs);
          if (key) {
            // Key press
            if (data.dwData & 0x80) {
              qentry.type      = evTYPE_RAW_KEY_PRESS;
              qentry.keycode   = key;
              qentry.timestamp = (unsigned)(data.dwTimeStamp);
              win_EventQueue_Add (&qentry);
            }
            // Key release
            else {
              qentry.type      = evTYPE_RAW_KEY_RELEASE;
              qentry.keycode   = key;
						  qentry.timestamp = (unsigned)(data.dwTimeStamp);
              win_EventQueue_Add (&qentry);
            }
          }
				}
				else
					buffer_empty = TRUE;
			} 
    } // while (NOT buffer_empty)

		LeaveCriticalSection (&dikeyboard_critsection);
	}

  return (0);
}

/*___________________________________________________________________
|
|	Function: Translate_Key
| 
|	Input: Called from Keyboard_Thread
|
| Output: Takes a DirectInput keycode and translates it according to the
|       translation table in this file.
|___________________________________________________________________*/

static int Translate_Key (DWORD di_key)
{
  int i;

  for (i=0; dikeyboard_translate_table[i].key; i++)
    if (dikeyboard_translate_table[i].di_key == di_key)
      return (dikeyboard_translate_table[i].key);

  // Key not found!
  return (0);
}

/*___________________________________________________________________
|
|	Function: Mouse_Init
| 
|	Input: Called from dinput9_Init()
|
| Output: Initializes DirectInput mouse device.  Returns true on  
|					success or false on any error.
|___________________________________________________________________*/

static int Mouse_Init ()
{
  int screen_dx, screen_dy;
	DIDEVCAPS caps;
	DIPROPDWORD prop;
	DWORD dwThreadID = 0;
	int initialized = FALSE;

  // Default 16x16 mouse cursor
	static byte screen_mask [] = {  
  0x60, 0x00,   // 0110000000000000
  0x70, 0x00,   // 0111000000000000
  0x78, 0x00,   // 0111100000000000
  0x7C, 0x00,   // 0111110000000000
  0x7E, 0x00,   // 0111111000000000
  0x7F, 0x00,   // 0111111100000000
  0x7F, 0x80,   // 0111111110000000
  0x7F, 0xC0,   // 0111111111000000
  0x7F, 0xE0,   // 0111111111100000
  0x7F, 0xC0,   // 0111111111000000
  0x7F, 0x00,   // 0111111100000000
  0x77, 0x80,   // 0111011110000000
  0x67, 0x80,   // 0110011110000000
  0x03, 0xC0,   // 0000001111000000
  0x03, 0xC0,   // 0000001111000000
  0x01, 0xC0    // 0000000111000000
  };
	static byte cursor_mask [] = {
  0x00, 0x00,   // 0000000000000000
  0x20, 0x00,   // 0010000000000000
  0x30, 0x00,   // 0011000000000000
  0x38, 0x00,   // 0011100000000000
  0x3C, 0x00,   // 0011110000000000
  0x3E, 0x00,   // 0011111000000000
  0x3F, 0x00,   // 0011111100000000
  0x3F, 0x80,   // 0011111110000000
  0x3F, 0xC0,   // 0011111111000000
  0x3E, 0x00,   // 0011111000000000
  0x36, 0x00,   // 0011011000000000
  0x23, 0x00,   // 0010001100000000
  0x03, 0x00,   // 0000001100000000
  0x01, 0x80,   // 0000000110000000
  0x01, 0x80,   // 0000000110000000
  0x00, 0x00    // 0000000000000000
	};

	// Has DirectInput been initialized?
	if (dinput8) {
		// Create a mouse device
		if (dinput8->CreateDevice (GUID_SysMouse, &dimouse, NULL) == DI_OK) {
			// Get device capabilities
			ZeroMemory (&caps, sizeof(caps));
			caps.dwSize = sizeof(caps);
			if (dimouse->GetCapabilities (&caps) == DI_OK) {
				// Is device physically attached?
				if (caps.dwFlags & DIDC_ATTACHED) {
					// Set data format to a device that has up to 4 buttons and 3 axis
					if (dimouse->SetDataFormat (&c_dfDIMouse) == DI_OK) {
						// Set cooperative level
						if (dimouse->SetCooperativeLevel (win_Get_Window_Handle(), DISCL_EXCLUSIVE | DISCL_FOREGROUND) == DI_OK) {
							// Set buffer size
							ZeroMemory (&prop, sizeof(prop));
							prop.diph.dwSize = sizeof(prop);
							prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
							prop.diph.dwObj = 0;
							prop.diph.dwHow = DIPH_DEVICE;
							prop.dwData = MOUSE_BUFFER_SIZE;
							if (dimouse->SetProperty (DIPROP_BUFFERSIZE, &prop.diph) == DI_OK) {
								// Create 2 events
								dimouse_event[READY_EVENT_INDEX] = CreateEvent (0, 0, 0, 0);
								dimouse_event[QUIT_EVENT_INDEX]  = CreateEvent (0, 0, 0, 0);
                // Create a critical section
                InitializeCriticalSection (&dimouse_critsection);
                // Were events created successfully?
								if (dimouse_event[READY_EVENT_INDEX] AND dimouse_event[QUIT_EVENT_INDEX]) {
									// Init event notification
									if (dimouse->SetEventNotification (dimouse_event[READY_EVENT_INDEX]) == DI_OK) {
										// Set default cursor
                    if (Direct3D_GetPixelSize() == 2)
											dinput9_Mouse_Set_Bitmap_Cursor (cursor_mask, screen_mask, 16, 16, 1, 2, 255, 255, 255, 10, 10, 10);
                    else 
											dinput9_Mouse_Set_Bitmap_Cursor (cursor_mask, screen_mask, 16, 16, 1, 2, 255, 255, 255, 1, 1, 1);
										// Set mouse screen limits
                    Direct3D_GetScreenDimensions (&screen_dx, &screen_dy, NULL);
                    dimouse_min_x = 0;
										dimouse_min_y = 0;
										dimouse_max_x = screen_dx - 1;
										dimouse_max_y = screen_dy - 1;
										// Create the thread
                    dimouse_thread = (HANDLE)_beginthreadex (NULL,
                                                             1024,
                                                             (PBEGINTHREADEX_THREADFUNC)Mouse_Thread,
                                                             NULL,
                                                             CREATE_SUSPENDED,
                                                             (PBEGINTHREADEX_THREADID)&dwThreadID);
                    if (dimouse_thread) {
                      SetThreadPriority (dimouse_thread, THREAD_PRIORITY_TIME_CRITICAL); // was HIGHEST, changed 2/17/2013
											initialized = TRUE;
                    }
									}
								}
							}
						}
					}
				}
			}
		}
	}

  return (initialized);
}

/*___________________________________________________________________
|
|	Function: Mouse_Thread
| 
|	Input: Called from Mouse_Init()
| Output: This function is a seperate worker thread to process mouse 
|					movement and input.
|___________________________________________________________________*/

static DWORD WINAPI Mouse_Thread (LPVOID pParam)
{
	static DIDEVICEOBJECTDATA data;
	static DWORD event, num_events;
	static HRESULT hres;
	static BOOL buffer_empty;

  // Block until one of the 2 events becomes signaled
  while ((event = WaitForMultipleObjects (2, dimouse_event, FALSE, INFINITE)) != WAIT_FAILED) {							
		// If quit event signaled, then quit out of this thread function
		if ((event-WAIT_OBJECT_0) == QUIT_EVENT_INDEX) {
      ResetEvent (dimouse_event[QUIT_EVENT_INDEX]);
			return 0;
		}
    else 
      ResetEvent (dimouse_event[READY_EVENT_INDEX]);

		// Lock critical section to synch use of shared variables
		EnterCriticalSection (&dimouse_critsection);

		buffer_empty = FALSE;
		// Keep processing events until mouse buffer is empty
		while (NOT buffer_empty) {
			num_events = 1;
			// Make sure mouse is acquired
			hres = dimouse->Acquire ();
			// Is device acquired?
			if ((hres == DI_OK) OR (hres == S_FALSE)) {
				// Retrieve 1 event from mouse buffer
				hres = dimouse->GetDeviceData (sizeof(DIDEVICEOBJECTDATA), &data, &num_events, 0);
				if ((hres == DI_OK) AND (num_events == 1)) {
					static EventQueueEntry qentry;
					switch (data.dwOfs) {
						case DIMOFS_X:
							// Update mouse x coordinate
              dimouse_rel_x += data.dwData;
							dimouse_x += data.dwData;
							// Verify mouse coordinates, so don't go offscreen
							if (dimouse_x < dimouse_min_x)
								dimouse_x = dimouse_min_x;
							else if (dimouse_x > dimouse_max_x)	
								dimouse_x = dimouse_max_x;
							break;
						case DIMOFS_Y:
							// Update mouse y coordinate
              dimouse_rel_y += data.dwData;
              dimouse_y += data.dwData;
							// Verify mouse coordinates, so don't go offscreen
							if (dimouse_y < dimouse_min_y)
								dimouse_y = dimouse_min_y;
							else if (dimouse_y > dimouse_max_y)
								dimouse_y = dimouse_max_y;
							break;
            // Process mouse wheel movement
            case DIMOFS_Z:
              if (data.dwData) {
                if ((int)(data.dwData) < 0) 
                  qentry.type = evTYPE_MOUSE_WHEEL_BACKWARD;
                else
                  qentry.type = evTYPE_MOUSE_WHEEL_FORWARD;
                qentry.timestamp = (unsigned)(data.dwTimeStamp);
                win_EventQueue_Add (&qentry);
              }
							break;
						case DIMOFS_BUTTON0:
							// Put a left button event in mouse click queue
							if (data.dwData & 0x80)
								qentry.type = evTYPE_MOUSE_LEFT_PRESS;
						  else
								qentry.type = evTYPE_MOUSE_LEFT_RELEASE;
							qentry.x = dimouse_x;
							qentry.y = dimouse_y;		
							qentry.timestamp = (unsigned)(data.dwTimeStamp);
              win_EventQueue_Add (&qentry);
							break;
						case DIMOFS_BUTTON1:
							// Put a right button event in mouse click queue
							if (data.dwData & 0x80)
								qentry.type = evTYPE_MOUSE_RIGHT_PRESS;
							else
								qentry.type = evTYPE_MOUSE_RIGHT_RELEASE;
							qentry.x = dimouse_x;
							qentry.y = dimouse_y;
							qentry.timestamp = (unsigned)(data.dwTimeStamp);
              win_EventQueue_Add (&qentry);
							break;
					}
				}
				else
					buffer_empty = TRUE;
			} 
		} // while (NOT buffer_empty)

		// Update mouse cursor on screen?
		if (dimouse_visible) 
			// Has cursor been moved since last drawn position?
			if ((dimouse_x != dimouse_last_x) OR (dimouse_y != dimouse_last_y)) {
        Direct3D_Set_Cursor_Position ((unsigned)dimouse_x, (unsigned)dimouse_y);
				// Save last drawn cursor position
				dimouse_last_x = dimouse_x;
				dimouse_last_y = dimouse_y;
			}

		LeaveCriticalSection (&dimouse_critsection);
	}

  return (0);
}

/*___________________________________________________________________
|
|	Function: dinput9_Free
| 
|	Input: Called from ____
| Output: Releases all memory used by DirectInput.
|___________________________________________________________________*/

void dinput9_Free ()
{
	// Has DirectInput been initialized?
	if (dinput8) {

/*___________________________________________________________________
|
| Release the keyboard device
|___________________________________________________________________*/

		if (dikeyboard) { 
			EnterCriticalSection (&dikeyboard_critsection);
      dikeyboard->Unacquire ();
			// Send keyboard thread the quit event
			if (dikeyboard_event[QUIT_EVENT_INDEX]) {
				SetEvent (dikeyboard_event[QUIT_EVENT_INDEX]);
        // Wait for thread to terminate
        WaitForSingleObject (dikeyboard_thread, INFINITE);
        CloseHandle (dikeyboard_thread);
        dikeyboard_thread = 0;
			}
			dikeyboard->Release ();
			dikeyboard = 0;
      // Delete events
			if (dikeyboard_event[READY_EVENT_INDEX]) {
				CloseHandle (dikeyboard_event[READY_EVENT_INDEX]);
				dikeyboard_event[READY_EVENT_INDEX] = 0;
			}
			if (dikeyboard_event[QUIT_EVENT_INDEX]) {
				CloseHandle (dikeyboard_event[QUIT_EVENT_INDEX]);
				dikeyboard_event[QUIT_EVENT_INDEX] = 0;
			}
      LeaveCriticalSection (&dikeyboard_critsection);
      DeleteCriticalSection (&dikeyboard_critsection);
    }

/*___________________________________________________________________
|
| Release the mouse device
|___________________________________________________________________*/

		if (dimouse) {
			// Start by hiding mouse cursor
			dinput9_Mouse_Hide ();

			EnterCriticalSection (&dimouse_critsection);
			dimouse->Unacquire ();
			// Send mouse thread the quit event
			if (dimouse_event[QUIT_EVENT_INDEX]) {
				SetEvent (dimouse_event[QUIT_EVENT_INDEX]);
        // Wait for thread to terminate
        WaitForSingleObject (dimouse_thread, INFINITE);
        CloseHandle (dimouse_thread);
        dimouse_thread = 0;
			}
			dimouse->Release ();
			dimouse = 0;
      // Delete events
			if (dimouse_event[READY_EVENT_INDEX]) {
				CloseHandle (dimouse_event[READY_EVENT_INDEX]);
				dimouse_event[READY_EVENT_INDEX] = 0;
			}
			if (dimouse_event[QUIT_EVENT_INDEX]) {
				CloseHandle (dimouse_event[QUIT_EVENT_INDEX]);
				dimouse_event[QUIT_EVENT_INDEX] = 0;
			}
			// Free memory for mouse cursor-related surfaces
      FREE_CURSOR_SURFACE
      FREE_SAVED_CURSOR
			LeaveCriticalSection (&dimouse_critsection);
      DeleteCriticalSection (&dimouse_critsection);
		}

/*___________________________________________________________________
|
| Release the DirectInput object
|___________________________________________________________________*/

		dinput8->Release ();
		dinput8 = 0;
	}
}

/*___________________________________________________________________
|
|	Function: dinput9_Restore
| 
|	Input: Called from ____
| Output: Attempts to reacquire any unacquired devices.  Returns true
|					if all devices acquired, else flase if 1 or more not acquired.
|___________________________________________________________________*/

int dinput9_Restore ()
{
	HRESULT hres;
	int restored = TRUE;

	if (dinput8) {
		// Acquire keyboard 
		if (dikeyboard) {
			hres = dikeyboard->Acquire ();
			if ((hres != DI_OK) AND (hres != S_FALSE))
				restored = FALSE;
		}
		// Acquire mouse
		if (dimouse) {
			EnterCriticalSection (&dimouse_critsection);
			hres = dimouse->Acquire ();
			LeaveCriticalSection (&dimouse_critsection);
			if ((hres != DI_OK) AND (hres != S_FALSE))
				restored = FALSE;
      if (restored) {
        // Restore cursor?
        if (dimouse_cursor_type)
          Set_Cursor ();
      }
		}
  }

	return (restored);
}

/*___________________________________________________________________
|
|	Function: dinput9_Get_Event
| 
|	Input: Called from ____
| Output: Returns true if an event is ready.  If ready, returns info
|					about the event in callers variables.
|___________________________________________________________________*/

int dinput9_Get_Event (
	unsigned *type,				// type of event
	int			 *keycode,		// only valid for key events
	int			 *x,					// only valid for mouse events
	int			 *y, 
	unsigned *timestamp)	// system time in milliseconds
{
	int event_ready;
	EventQueueEntry qentry;

	// Get an event from event queue
  event_ready = win_EventQueue_Remove (&qentry);

	// If event was retrieved, put event info in callers variables
	if (event_ready) {
		*type			 = qentry.type;
		*keycode	 = qentry.keycode;
		*x				 = qentry.x;
		*y				 = qentry.y;
	  *timestamp = qentry.timestamp;
	}

	return (event_ready);
}

/*___________________________________________________________________
|
|	Function: dinput9_Flush_Events
| 
|	Input: Called from ____
| Output: Flushes all events contained in the eventmask from the event queue.
|___________________________________________________________________*/

void dinput9_Flush_Events (unsigned event_type_mask)
{
  win_EventQueue_Flush (event_type_mask);
}

/*___________________________________________________________________
|
|	Function: dinput9_Mouse_Hide
| 
|	Input: Called from ____
| Output: If mouse cursor is currently visible, hides it.
|___________________________________________________________________*/

void dinput9_Mouse_Hide ()
{
	if (dimouse AND dimouse_visible) {
		EnterCriticalSection (&dimouse_critsection);
		if ((dimouse_last_x != -1) OR (dimouse_last_y != -1))
      Direct3D_Show_Cursor (FALSE);
		dimouse_last_x = -1;
		dimouse_last_y = -1;
		dimouse_visible = FALSE;
		LeaveCriticalSection (&dimouse_critsection);
	}
}						

/*___________________________________________________________________
|
|	Function: dinput9_Mouse_Show
| 
|	Input: Called from ____
| Output: If mouse cursor is currently not visible, makes it visible.
|___________________________________________________________________*/

void dinput9_Mouse_Show ()
{
  if (dimouse AND (NOT dimouse_visible)) {
		EnterCriticalSection (&dimouse_critsection);
    Direct3D_Show_Cursor (TRUE);
    dimouse_last_x = dimouse_x;
		dimouse_last_y = dimouse_y;
		dimouse_visible = TRUE;
		LeaveCriticalSection (&dimouse_critsection);
	}
}

/*___________________________________________________________________
|
|	Function: dinput9_Mouse_Get_Coords
| 
|	Input: Called from ____
| Output: Returns current mouse coordinates.
|___________________________________________________________________*/

void dinput9_Mouse_Get_Coords (int *x, int *y)
{
	EnterCriticalSection (&dimouse_critsection);
	*x = dimouse_x;
	*y = dimouse_y;
	LeaveCriticalSection (&dimouse_critsection);
}

/*___________________________________________________________________
|
|	Function: dinput9_Mouse_Set_Coords
| 
|	Input: Called from ____
| Output: Sets new mouse coordinates and updates mouse cursor if visible.
|___________________________________________________________________*/

void dinput9_Mouse_Set_Coords (int x, int y)
{
	// Make sure new coordinates are valid
	if ((x >= dimouse_min_x) AND (x <= dimouse_max_x) AND
		  (y >= dimouse_min_y) AND (y <= dimouse_max_y)) {
		EnterCriticalSection (&dimouse_critsection);
		// Set new mouse coords
		dimouse_x = x;
		dimouse_y = y;
		// Update mouse cursor on screen?
    if (dimouse_visible) 
      // Has cursor been moved since last drawn position?
      if ((dimouse_x != dimouse_last_x) OR (dimouse_y != dimouse_last_y)) {
        Direct3D_Set_Cursor_Position ((unsigned)dimouse_x, (unsigned)dimouse_y);
        // Save last drawn cursor position
				dimouse_last_x = dimouse_x;
				dimouse_last_y = dimouse_y;
			}

		LeaveCriticalSection (&dimouse_critsection);
	}
}

/*___________________________________________________________________
|
|	Function: dinput9_Mouse_Get_Status
| 
|	Input: Called from ____
| Output: Returns current mouse coordinates and button info.  If a button
|					is currently being pressed, returns 1 in the variable for 
|					that button, else 0.
|___________________________________________________________________*/

void dinput9_Mouse_Get_Status (int *x, int *y, int *left, int *right)
{
	static HRESULT hres;
	static DIMOUSESTATE state;

	EnterCriticalSection (&dimouse_critsection);      
	*x		 = dimouse_x;
	*y		 = dimouse_y;
	*left  = 0;
	*right = 0;
	// Make sure mouse is acquired
	hres = dimouse->Acquire ();
	if ((hres == DI_OK) OR (hres == S_FALSE)) 
		if (dimouse->GetDeviceState (sizeof(DIMOUSESTATE), (void *)&state) == DI_OK) {
			*left  = (state.rgbButtons[0] & 0x80) >> 7;
			*right = (state.rgbButtons[1] & 0x80) >> 7;
		}
		
	LeaveCriticalSection (&dimouse_critsection);
}

/*___________________________________________________________________
|
|	Function: dinput9_Mouse_Set_Image_Cursor
| 
|	Input: Called from ____
| Output: Sets a new mouse cursor using an image.
|___________________________________________________________________*/

void dinput9_Mouse_Set_Image_Cursor (
	byte *image, 
	int   image_dx,
	int   image_dy,
	int		hot_x,
	int		hot_y )
{
  int size, cursor_dx, cursor_dy, x, y;

  // Save info about this cursor
  FREE_SAVED_CURSOR
  Adjust_Cursor_Size (image_dx, image_dy, &cursor_dx, &cursor_dy);
  size = cursor_dx * cursor_dy;
  dimouse_cursor_image = (byte *) calloc (size, Direct3D_GetPixelSize());
  if (dimouse_cursor_image) {
    for (y=0; y<image_dy; y++)
      for (x=0; x<image_dx; x++)
        dimouse_cursor_image[y*cursor_dx+x] = image[y*image_dx+x];
    dimouse_cursor_dx   = cursor_dx;
    dimouse_cursor_dy   = cursor_dy;
    dimouse_cursor_hotx = hot_x;
    dimouse_cursor_hoty = hot_y;
    dimouse_cursor_type = CURSOR_TYPE_IMAGE;
  }

  // Set this cursor
  Set_Cursor ();
}

/*___________________________________________________________________
|
|	Function: dinput9_Mouse_Set_Bitmap_Cursor
| 
|	Input: Called from ____
| Output: Sets a new mouse cursor using a bitmap.
|___________________________________________________________________*/

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
	byte 	mask_color_b )
{
  int size, cursor_dx, cursor_dy, x, y;

  // Save info about this cursor
  FREE_SAVED_CURSOR
  Adjust_Cursor_Size (bitmap_dx, bitmap_dy, &cursor_dx, &cursor_dy);
  size = cursor_dx/8 * cursor_dy;
  dimouse_cursor_bitmap  = (byte *) calloc (size, sizeof(byte));
  dimouse_cursor_bitmask = (byte *) calloc (size, sizeof(byte));
  if ((dimouse_cursor_bitmap == NULL) OR (dimouse_cursor_bitmask == NULL)) {
    if (dimouse_cursor_bitmap) {
      free (dimouse_cursor_bitmap);
      dimouse_cursor_bitmap = NULL;
    }
    if (dimouse_cursor_bitmap) {
      free (dimouse_cursor_bitmap);
      dimouse_cursor_bitmap = NULL;
    }
  }
  else {
    for (y=0; y<bitmap_dy; y++)
      for (x=0; x<(bitmap_dx+7)/8; x++) {
        dimouse_cursor_bitmap [y*(cursor_dx/8)+x] = cursor_bitmap[y*(bitmap_dx/8)+x];
        dimouse_cursor_bitmask[y*(cursor_dx/8)+x] = mask_bitmap  [y*(bitmap_dx/8)+x];
      }
    dimouse_cursor_dx              = cursor_dx;
    dimouse_cursor_dy              = cursor_dy;
    dimouse_cursor_hotx            = hot_x;
    dimouse_cursor_hoty            = hot_y;
    dimouse_cursor_bitmap_color_r  = cursor_color_r;
    dimouse_cursor_bitmap_color_g  = cursor_color_g;
    dimouse_cursor_bitmap_color_b  = cursor_color_b;
    dimouse_cursor_bitmask_color_r = mask_color_r;
    dimouse_cursor_bitmask_color_g = mask_color_g;
    dimouse_cursor_bitmask_color_b = mask_color_b;
    dimouse_cursor_type            = CURSOR_TYPE_BITMAP;
  }

  // Set this cursor
  Set_Cursor ();
}

/*__________________________________________________________________
|
|	Function: Adjust_Cursor_Size
| 
|	Input: Called from dinput9_Mouse_Set_Image_Cursor(), 
|                    dinput9_Mouse_Set_Bitmap_Cursor()
| Output:	Adjusts cursor size if necessary.  Returns true if valid size
|   has been chosen for the cursor, else false if no valid size chosen.
|___________________________________________________________________*/

static int Adjust_Cursor_Size (int dx, int dy, int *cursor_dx, int *cursor_dy)
{
  int i, valid;
  static int valid_size [] = {
    1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
    0 // last entry in this table must be zero
  };

  // Init cursor width,height
  *cursor_dx = 0;
  *cursor_dy = 0;

  // Set cursor width
  for (i=0; valid_size[i]; i++)
    if (dx <= valid_size[i]) {
      *cursor_dx = valid_size[i];
      break;
    }

  // Set cursor height
  for (i=0; valid_size[i]; i++)
    if (dy <= valid_size[i]) {
      *cursor_dy = valid_size[i];
      break;
    }

  //// Make the cursor square
  //if (*cursor_dx < *cursor_dy)
  //  *cursor_dx = *cursor_dy;
  //else if (*cursor_dy < *cursor_dx)
  //  *cursor_dy = *cursor_dx;

  if (*cursor_dx AND *cursor_dy)
    valid = TRUE;
  else
    valid = FALSE;

  return (valid);
}

/*___________________________________________________________________
|
|	Function: Set_Cursor
| 
|	Input: Called from dinput9_Mouse_Set_Image_Cursor(), 
|                    dinput9_Mouse_Set_Bitmap_Cursor(),
|                    dinput9_Restore()
| Output: Sets a new mouse cursor.
|___________________________________________________________________*/

static void Set_Cursor ()
{
  int hidden = FALSE;

  // Hide mouse cursor if needed
	if (dimouse_visible) {
	  dinput9_Mouse_Hide ();
		hidden = TRUE;
	}

  // Set new cursor
	EnterCriticalSection (&dimouse_critsection);
  if (dimouse_cursor_type == CURSOR_TYPE_IMAGE)
    Direct3D_Set_Image_Cursor (dimouse_cursor_image, 
                               dimouse_cursor_dx, 
                               dimouse_cursor_dy, 
                               dimouse_cursor_hotx, 
                               dimouse_cursor_hoty);
  else if (dimouse_cursor_type == CURSOR_TYPE_BITMAP)
    Direct3D_Set_Bitmap_Cursor (dimouse_cursor_bitmap, 
                                dimouse_cursor_bitmask, 
                                dimouse_cursor_dx, 
                                dimouse_cursor_dy, 
                                dimouse_cursor_hotx, 
                                dimouse_cursor_hoty, 
                                dimouse_cursor_bitmap_color_r, 
                                dimouse_cursor_bitmap_color_g, 
                                dimouse_cursor_bitmap_color_b, 
                                dimouse_cursor_bitmask_color_r, 
                                dimouse_cursor_bitmask_color_g, 
                                dimouse_cursor_bitmask_color_b);
  LeaveCriticalSection (&dimouse_critsection);
  Direct3D_Set_Cursor_Position ((unsigned)dimouse_x, (unsigned)dimouse_y);

  // Unhide mouse cursor if needed
  if (hidden)
  	dinput9_Mouse_Show ();
}

/*___________________________________________________________________
|
|	Function: dinput9_Mouse_Confine
| 
|	Input: Called from ____
| Output: Confines mouse cursor to a rectangle on the screen.
|___________________________________________________________________*/

void dinput9_Mouse_Confine (int left, int top, int right, int bottom)
{
  int screen_dx, screen_dy;

  // Make sure new coordinates are valid
  Direct3D_GetScreenDimensions (&screen_dx, &screen_dy, NULL);
  if ((left >= 0) AND (right  < screen_dx) AND
		  (top  >= 0) AND (bottom < screen_dy)) {
		EnterCriticalSection (&dimouse_critsection);
		// Set new mouse limits
		dimouse_min_x = left;
		dimouse_min_y = top;
		dimouse_max_x = right;
		dimouse_max_y = bottom;
		// Set new mouse coords		
		if (dimouse_x < dimouse_min_x)
			dimouse_x = dimouse_min_x;
		else if (dimouse_x > dimouse_max_x)
			dimouse_x = dimouse_max_x;
		if (dimouse_y < dimouse_min_y)
			dimouse_y = dimouse_min_y;
		else if (dimouse_y > dimouse_max_y)
			dimouse_y = dimouse_max_y;
		// Update mouse cursor on screen?
		if (dimouse_visible) 
      // Has cursor been moved since last drawn position?
			if ((dimouse_x != dimouse_last_x) OR (dimouse_y != dimouse_last_y)) {
        Direct3D_Set_Cursor_Position ((unsigned)dimouse_x, (unsigned)dimouse_y);
				// Save last drawn cursor position
				dimouse_last_x = dimouse_x;
				dimouse_last_y = dimouse_y;
			}
		LeaveCriticalSection (&dimouse_critsection);
	}
}

/*___________________________________________________________________
|
|	Function: dinput9_Mouse_Get_Movement
| 
|	Input: Called from ____
| Output: Gets the amount of mouse movement since the last call to this
|   function.
|___________________________________________________________________*/

void dinput9_Mouse_Get_Movement (int *x, int *y)
{
  EnterCriticalSection (&dimouse_critsection);
  *x = dimouse_rel_x;
  *y = dimouse_rel_y;
  dimouse_rel_x = 0;
  dimouse_rel_y = 0;
	LeaveCriticalSection (&dimouse_critsection);
}
