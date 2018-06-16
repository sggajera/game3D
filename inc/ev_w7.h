/*____________________________________________________________________
|
| File: ev_w7.h
| Description: Event library.
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _EV_W7_H_
#define _EV_W7_H_

#include <events.h>

/*___________________
|
| Constants
|__________________*/

// drivers
#define evDRIVER_DX9  9

/*___________________
|
| Data types
|__________________*/

typedef struct {
  unsigned type;
  int      keycode;  // keypress info      
  int      x, y;     // mouse info           
} evEvent;

/*___________________
|
| Function prototypes
|__________________*/

void evStartEvents (unsigned eventmask, int mouse_auto_tracking, int driver);
void evStopEvents (void);
void evFlushEvents (void);
bool evGetEvent (evEvent *input);

#endif
