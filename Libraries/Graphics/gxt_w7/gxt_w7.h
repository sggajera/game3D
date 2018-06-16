/*____________________________________________________________________
|
| File: gxt_w7.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Functions
|__________________*/

// SCRIPT_PSYS.CPP
// Creates a particle system from a script file
gx3dParticleSystem Script_ParticleSystem_Create (char *script_file);


// WIDGETS.CPP
typedef void *Widget;

// Initialize a horizontal slider bar widget for an integer
Widget Widget_HSliderBar_Init (char *title, int *value, int min, int max, int screen_x, int screen_y, int bar_dx, int bar_dy);
// Initialize a horizontal slider bar widget for a float
Widget Widget_HSliderBar_Init (char *title, float *value, float min, float max, int screen_x, int screen_y, int bar_dx, int bar_dy);
// Free a widget                            
void   Widget_Free (Widget w);
// Frees all widgets
void   Widget_FreeAll ();
// Updates widget, returns true if event is used by the widget
bool   Widget_Update (Widget w, bool event_ready, evEvent *event);
// Udates all widgets, returns true if event is used by any widget
bool   Widget_UpdateAll (bool event_ready, evEvent *event);
// Draws widget
void   Widget_Draw (Widget w);
// Draws all widgets
void   Widget_DrawAll ();
// Removes focus from any widget that has focus, if any
void   Widget_ClearAllFocus ();

