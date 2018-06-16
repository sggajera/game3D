/*____________________________________________________________________
|
| File: events.h
| Description: Event constants
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _EVENTS_H_
#define _EVENTS_H_

/*___________________
|
| Type definitions
|__________________*/

typedef struct {
	unsigned type;
	int			 keycode;   // for keypress
  int      x, y;      // for mouse
  unsigned timestamp;	// system time in milliseconds
} EventQueueEntry;

/*___________________
|
| Constants
|__________________*/

// Event type constants
#define evTYPE_WINDOW_ACTIVE				0x1	// windows msg loop generated event
#define evTYPE_WINDOW_INACTIVE			0x2	// windows msg loop generated event
#define evTYPE_WINDOW_CLOSE				  0x4	// windows msg loop generated event

#define evTYPE_KEY_PRESS						0x8
#define evTYPE_RAW_KEY_PRESS  			0x10
#define evTYPE_RAW_KEY_RELEASE      0x20

#define evTYPE_MOUSE_LEFT_PRESS			0x40
#define evTYPE_MOUSE_LEFT_RELEASE		0x80
#define evTYPE_MOUSE_RIGHT_PRESS		0x100
#define evTYPE_MOUSE_RIGHT_RELEASE	0x200
#define evTYPE_MOUSE_WHEEL_BACKWARD 0x400
#define evTYPE_MOUSE_WHEEL_FORWARD  0x800

// Key constants
#define evKY_SPACE				0x20
#define evKY_UP_ARROW			(256+0)				
#define evKY_DOWN_ARROW		(256+1)
#define evKY_LEFT_ARROW		(256+2)
#define evKY_RIGHT_ARROW	(256+3)
#define evKY_PAGE_UP			(256+4)
#define evKY_PAGE_DOWN		(256+5)
#define evKY_ENTER				(256+6)
#define evKY_HOME					(256+7)
#define evKY_END					(256+8)
#define evKY_INSERT				(256+9)
#define evKY_DELETE				(256+10)
#define evKY_PAUSE				(256+11)
#define evKY_ALT					(256+12)  
#define evKY_CONTROL			(256+13)  
#define evKY_SHIFT				(256+14)    
#define evKY_ESC					(256+15)
#define evKY_BACKSPACE		(256+16)
#define evKY_TAB					(256+17)
#define evKY_F1						(256+18)
#define evKY_F2						(256+19)
#define evKY_F3						(256+20)
#define evKY_F4						(256+21)
#define evKY_F5						(256+22)
#define evKY_F6						(256+23)
#define evKY_F7						(256+24)
#define evKY_F8						(256+25)
#define evKY_F9						(256+26)
#define evKY_F10					(256+27)    
#define evKY_F11					(256+28)	
#define evKY_F12					(256+29)        

#endif
