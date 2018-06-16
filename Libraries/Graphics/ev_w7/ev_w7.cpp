/*____________________________________________________________________
|
| File: ev_w7.cpp
|
| Description: Contains functions for event processing.
|
| Functions:	evStartEvents
|             evStopEvents
|             evFlushEvents
|             evGetEvents       
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define USING_DIRECTX_9

//#define DEBUG

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <stdio.h>
#include <string.h>
#include <conio.h>

#include <defines.h>
#include <clib.h>
#include <gx_w7.h>
#include <ms_w7.h>
#ifdef USING_DIRECTX_9
#include <dx9.h>
#endif

#include "ev_w7.h"

/*___________________
|
| Globals
|__________________*/

static unsigned Event_mask = 0;
static unsigned Any_windows_event =     
  evTYPE_WINDOW_ACTIVE |
  evTYPE_WINDOW_INACTIVE |
	evTYPE_WINDOW_CLOSE;
static unsigned Any_mouse_event =                       
  evTYPE_MOUSE_LEFT_PRESS |
  evTYPE_MOUSE_LEFT_RELEASE |
  evTYPE_MOUSE_RIGHT_PRESS |
  evTYPE_MOUSE_RIGHT_RELEASE |
  evTYPE_MOUSE_WHEEL_BACKWARD |
  evTYPE_MOUSE_WHEEL_FORWARD;
static unsigned Any_raw_keyboard_event =    
  evTYPE_RAW_KEY_PRESS |
  evTYPE_RAW_KEY_RELEASE;

// Pointers to driver functions
static int  (*Start_events) (int use_keyboad, int use_mouse);
static void (*Stop_events) (void);
static void (*Flush_events) (void);
static int	(*Get_event) (unsigned *type, int *keycode, int *x, int *y);

static int using_mouse;

/*____________________________________________________________________
|
| Function: evStartEvents
|
| Outputs: Initializes input event queue and starts buffering requested
|       events. 
|___________________________________________________________________*/

void evStartEvents (unsigned eventmask, int mouse_auto_tracking, int driver)
{
  int mouse_driver;

  // Set driver functions
  switch (driver) {
#ifdef USING_DIRECTX_9
    case evDRIVER_DX9:  Start_events = dx9_StartEvents;
                        Stop_events  = dx9_StopEvents;
                        Flush_events = dx9_FlushEvents;
                        Get_event    = dx9_GetEvent;
                        mouse_driver = msDRIVER_DX9;
                        break;
#endif
  }

  // Build event mask containing events of interest - for sure interested in all windows events!
  Event_mask = Any_windows_event;
  // Add events user is interested in
  Event_mask |= eventmask;

  // Start event driver?
  (*Start_events) (Event_mask & Any_raw_keyboard_event, Event_mask & Any_mouse_event);

  // Start mouse driver?
  if (Event_mask & Any_mouse_event) {
    msStartMouse (mouse_auto_tracking, mouse_driver);
    using_mouse = TRUE;
  }
  else
    using_mouse = FALSE;
}

/*____________________________________________________________________
|
| Function: evStopEvents
|
| Outputs: Disables event queue processing.
|___________________________________________________________________*/

void evStopEvents (void)
{
  if (Event_mask) {
    if (using_mouse) {
      msStopMouse ();
      using_mouse = FALSE;
    }
    (*Stop_events) ();
    Event_mask = 0;
  }
}

/*____________________________________________________________________
|
| Function: evFlushEvents
|
| Outputs: Flushes event queue.
|___________________________________________________________________*/

void evFlushEvents (void)
{
  (*Flush_events) ();
}

/*____________________________________________________________________
|
| Function: evGetEvent
|
| Outputs: Returns info about an event if one is ready.  Returns true
|					if event was ready else false.  If both key presses and mouse
|         presses/releases are being buffered, key presses receive a
|         higher priority and are returned first, regardless of the
|         actual sequence of key/mouse actions.
|___________________________________________________________________*/

bool evGetEvent (evEvent *input)
{
  bool done;
  bool event_ready = false;

  done = false;
  // Get an event of interest if any events ready
  while (NOT done) {
    // Get an event, if one is ready
    if ((*Get_event) (&(input->type), &(input->keycode), &(input->x), &(input->y))) {
      // Is this event of interest?
      if (input->type & Event_mask) {
        event_ready = true;
        done = true;
      }
    }
    else
      // No events ready, so exit
      done = true;
  }

/*____________________________________________________________________
|
| Print event type to debug file
|___________________________________________________________________*/

#ifdef DEBUG
  if (event_ready) {
    switch (input->type) {
      case evTYPE_KEY_PRESS:
        DEBUG_WRITE ("evGetEvent(): evTYPE_KEY_PRESS\n")
        break;
      case evTYPE_RAW_KEY_PRESS:
        DEBUG_WRITE ("evGetEvent(): evTYPE_RAW_KEY_PRESS\n")
        break;
      case evTYPE_MOUSE_LEFT_PRESS:
        DEBUG_WRITE ("evGetEvent(): evTYPE_MOUSE_LEFT_PRESS\n")
        break;
      case evTYPE_MOUSE_LEFT_RELEASE:
        DEBUG_WRITE ("evGetEvent(): evTYPE_MOUSE_LEFT_RELEASE\n")
        break;
      case evTYPE_MOUSE_RIGHT_PRESS:
        DEBUG_WRITE ("evGetEvent(): evTYPE_MOUSE_RIGHT_PRESS\n")
        break;
      case evTYPE_MOUSE_RIGHT_RELEASE:
        DEBUG_WRITE ("evGetEvent(): evTYPE_MOUSE_RIGHT_RELEASE\n")
        break;
      case evTYPE_MOUSE_WHEEL_BACKWARD:
        DEBUG_WRITE ("evGetEvent(): evTYPE_MOUSE_WHEEL_BACKWARD\n")
        break;
      case evTYPE_MOUSE_WHEEL_FORWARD:
        DEBUG_WRITE ("evGetEvent(): evTYPE_MOUSE_WHEEL_FORWARD\n")
        break;
      case evTYPE_WINDOW_ACTIVE:
        DEBUG_WRITE ("evGetEvent(): evTYPE_WINDOW_ACTIVE\n")
        break;
      case evTYPE_WINDOW_INACTIVE:
        DEBUG_WRITE ("evGetEvent(): evTYPE_WINDOW_INACTIVE\n")
        break;
      case evTYPE_WINDOW_CLOSE:
        DEBUG_WRITE ("evGetEvent(): evTYPE_WINDOW_CLOSE\n")
        break;
      case evTYPE_RAW_KEY_RELEASE:
        DEBUG_WRITE ("evGetEvent(): evTYPE_RAW_KEY_PRESS\n")
        break;
      default:
        DEBUG_WRITE ("evGetEvent(): unknown event!\n")
        break;
    }
  }
#endif

  return (event_ready);
}
