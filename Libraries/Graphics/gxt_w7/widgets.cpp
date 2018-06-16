/*____________________________________________________________________
|
| File: widgets.cpp
|
| Description: Functions for widgets
|
| Functions: Widget_HSliderBar_Init (int version)
|            Widget_HSliderBar_Init (float version)
|            Widget_Free
|            Widget_FreeAll
|            Widget_Update 
|             HSliderBar_Update
|              HSliderBar_Update_Handle
|             VSliderBar_Update
|            WidgetUpdateAll
|            Widget_Draw
|             HSliderBar_Draw
|             VSliderBar_Draw
|            Widget_DrawAll
|            Widget_ClearAllFocus
|             
|             Init_Widget_Font
|             Free_Widget_Font
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>
#include "dp.h"
#include <rom8x8.h>

//#include "widgets.h"

/*___________________
|
| Constants
|__________________*/

// Add widget to linked list and loads widget font, if necessary
#define ADD_TO_WIDGETLIST(_widget_)     \
  {                                     \
    if (widgetlist == 0) {              \
      widgetlist = _widget_;            \
      Init_Widget_Font ();              \
    }                                   \
    else {                              \
      _widget_->next = widgetlist;      \
      widgetlist->previous = _widget_;  \
      widgetlist = _widget_;            \
    }                                   \
  }

// Removes widget from linked list and frees widget font, if necessary
#define REMOVE_FROM_WIDGETLIST(_widget_)              \
  {                                                   \
    if (_widget_->previous)                           \
      _widget_->previous->next = _widget_->next;      \
    else                                              \
      widgetlist = _widget_->next;                    \
    if (_widget_->next)                               \
      _widget_->next->previous = _widget_->previous;  \
    if (widgetlist == 0)                              \
      Free_Widget_Font ();                            \
  }

/*___________________
|
| Type definitions
|__________________*/

// Type of widget
enum WidgetType { WIDGET_TYPE_HSLIDERBAR, WIDGET_TYPE_VSLIDERBAR };

// Type of value tied to widget
enum ValueType { VALUE_TYPE_INT, VALUE_TYPE_FLOAT };

// Value tied to widget
struct Value {
  union {
    struct { 
      int *value, min, max; 
    } i;
    struct {
      float *value, min, max;
    } f;
  };
};

// Data for a widget
struct WidgetData {
  WidgetType  type;
  char       *title;              // title of widget, if any
  int         screen_x, screen_y; // top left coords of widget window
  gxRectangle window;
  bool        has_focus;          // true if mouse is focused on the widget
  int         focus_offset;       // moust offset from center of handle when widget gains focus
  union {
    // Horizontal slider bar
    struct {
      ValueType value_type;       // type of value
      Value     value;            // value tied to widget
      int       bar_dx;           // width of slider bar, in pixels
      int       bar_dy;           // height of slider bar, in pixels
      int       handle;           // position of handle, in pixels from left (from 0 to bar_dx-4)
      int       handle_dx, handle_dy;
    } hsliderbar;
    // Vertical slider bar
    struct {
      ValueType value_type;       // type of value
      Value     value;            // value tied to widget
      int       bar_dx;           // width of slider bar, in pixels
      int       bar_dy;           // height of slider bar, in pixels
      int       handle;           // position of handle, in pixels from bottom
      int       handle_dx, handle_dy;
    } vsliderbar;
    // Other types of widgets...
  };
  // Used to create a doubly linked list
  WidgetData *next, *previous;
};

/*___________________
|
| Function prototypes
|__________________*/

static void HSliderBar_Draw (WidgetData *wid);
static void VSliderBar_Draw (WidgetData *wid);
static bool HSliderBar_Update (WidgetData *wid, bool event_ready, evEvent *event);
static bool VSliderBar_Update (WidgetData *wid, bool event_ready, evEvent *event);
static void HSliderBar_Update_Handle (WidgetData *wid, int mx, int my);
static void Init_Widget_Font ();
static void Free_Widget_Font ();

/*___________________
|
| Global variables
|__________________*/

static WidgetData *widgetlist  = 0;           // doubly linked list of widgets
static gxFont     *widget_font = 0;

/*____________________________________________________________________
|
| Function: Widget_HSliderBar_Init (int version)
|
| Input: Called from ____.
| Output: Initialize a horizontal slider bar widget for an integer.
|___________________________________________________________________*/
 
Widget Widget_HSliderBar_Init (char *title, int *value, int min, int max, int screen_x, int screen_y, int bar_dx, int bar_dy)
{
  float f;
  bool error = false;
  WidgetData *wid = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (value)
  DEBUG_ASSERT (screen_x >= 0)
  DEBUG_ASSERT (screen_y >- 0)
  DEBUG_ASSERT (bar_dx > 0)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Make sure bar_dy is an odd # so handle_dx will be odd
  bar_dy += (bar_dy % 2);

  // Allocate memory for widget
  wid = (WidgetData *) calloc (1, sizeof(WidgetData));
  if (wid == 0) {
    error = true;
    DEBUG_ERROR ("Widget_HSliderBar_Init(): Memory not allocated for widget")
  }
  else {
    // Allocate memory for widget title?
    if (title) {
      wid->title = (char *) calloc (strlen(title)+1, sizeof(char));
      if (wid->title == 0) {
        error = true;
        DEBUG_ERROR ("Widget_HSliderBar_Init(): Memory not allocated for widget title")
      }
      else
        strcpy (wid->title, title);
    }
    if (NOT error) {
      wid->type                     = WIDGET_TYPE_HSLIDERBAR;
      wid->screen_x                 = screen_x;
      wid->screen_y                 = screen_y;
      wid->hsliderbar.value_type    = VALUE_TYPE_INT;
      wid->hsliderbar.value.i.value = value;
      wid->hsliderbar.value.i.min   = min;
      wid->hsliderbar.value.i.max   = max;
      wid->hsliderbar.bar_dx        = bar_dx;
      wid->hsliderbar.bar_dy        = bar_dy;
      // Compute widget window size and position
      wid->window.xleft   = screen_x;
      wid->window.ytop    = screen_y;
      wid->window.xright  = screen_x + bar_dx - 1 + 2;
      wid->window.ybottom = screen_y + bar_dy - 1 + 2;
      // Compute handle size
      wid->hsliderbar.handle_dx = wid->hsliderbar.bar_dy;
      wid->hsliderbar.handle_dy = wid->hsliderbar.bar_dy * 3;
      // Compute initial handle position (from 0 to bar_dx-4)
      f = (float)(*wid->hsliderbar.value.i.value - wid->hsliderbar.value.i.min) /
          (float)( wid->hsliderbar.value.i.max   - wid->hsliderbar.value.i.min);
      wid->hsliderbar.handle = (int)((wid->hsliderbar.bar_dx - 2) * f);
    }
  }

  // On any error, free the widget
  if (error) {
    Widget_Free ((Widget)wid);
    wid = 0;
  }
  else 
    ADD_TO_WIDGETLIST (wid)

  return ((Widget)wid);
}

/*____________________________________________________________________
|
| Function: Widget_HSliderBar_Init (float version)
|
| Input: Called from ____.
| Output: Initialize a horizontal slider bar widget for a float.
|___________________________________________________________________*/
 
Widget Widget_HSliderBar_Init (char *title, float *value, float min, float max, int screen_x, int screen_y, int bar_dx, int bar_dy)
{
  float f;
  bool error = false;
  WidgetData *wid = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (value)
  DEBUG_ASSERT (screen_x >= 0)
  DEBUG_ASSERT (screen_y >- 0)
  DEBUG_ASSERT (bar_dx > 0)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Allocate memory for widget
  wid = (WidgetData *) calloc (1, sizeof(WidgetData));
  if (wid == 0) {
    error = true;
    DEBUG_ERROR ("Widget_HSliderBar_Init(): Memory not allocated for widget")
  }
  else {
    // Allocate memory for widget title?
    if (title) {
      wid->title = (char *) calloc (strlen(title)+1, sizeof(char));
      if (wid->title == 0) {
        error = true;
        DEBUG_ERROR ("Widget_HSliderBar_Init(): Memory not allocated for widget title")
      }
      else
        strcpy (wid->title, title);
    }
    if (NOT error) {
      wid->type                     = WIDGET_TYPE_HSLIDERBAR;
      wid->screen_x                 = screen_x;
      wid->screen_y                 = screen_y;
      wid->hsliderbar.value_type    = VALUE_TYPE_FLOAT;
      wid->hsliderbar.value.f.value = value;
      wid->hsliderbar.value.f.min   = min;
      wid->hsliderbar.value.f.max   = max;
      wid->hsliderbar.bar_dx        = bar_dx;
      wid->hsliderbar.bar_dy        = bar_dy;
      // Compute widget window size and position
      wid->window.xleft   = screen_x;
      wid->window.ytop    = screen_y;
      wid->window.xright  = screen_x + bar_dx - 1 + 2;
      wid->window.ybottom = screen_y + bar_dy - 1 + 2;
      // Compute handle size
      wid->hsliderbar.handle_dx = wid->hsliderbar.bar_dy;
      wid->hsliderbar.handle_dy = wid->hsliderbar.bar_dy * 3;
      // Compute initial handle position (from 0 to bar_dx-4)
      f = (*wid->hsliderbar.value.f.value - wid->hsliderbar.value.f.min) /
          ( wid->hsliderbar.value.f.max   - wid->hsliderbar.value.f.min);
      wid->hsliderbar.handle = (int)((wid->hsliderbar.bar_dx - 4) * f);
    }
  }

  if (error) {
    Widget_Free ((Widget)wid);
    wid = 0;
  }
  else
    ADD_TO_WIDGETLIST (wid)

  return ((Widget)wid);
}

/*____________________________________________________________________
|
| Function: Widget_Free
|
| Input: Called from ____.
| Output: Frees a widget.
|___________________________________________________________________*/
 
void Widget_Free (Widget w)
{
	WidgetData *wid = (WidgetData *)w;

  if (wid) {
    // Remove from list of widgets
    REMOVE_FROM_WIDGETLIST (wid)
    // Free memory for title?
    if (wid->title)
      free (wid->title);
    // Free memory for widget
    free (wid);
  }
}

/*___________________________________________________________________
|
|	Function: Widget_FreeAll
| 
|	Output: Frees all widgets, if any.
|___________________________________________________________________*/

void Widget_FreeAll ()
{
  // Free any remaining widgets
  while (widgetlist) 
    Widget_Free ((Widget)widgetlist);
}

/*____________________________________________________________________
|
| Function: Widget_Update
|
| Input: Called from ____.
| Output: Updates widget.  Returns true if event, if any, is used by 
|   the widget else false if the widget does not use the event.
|___________________________________________________________________*/
 
bool Widget_Update (Widget w, bool event_ready, evEvent *event)
{
  bool processed_event = false;
	WidgetData *wid = (WidgetData *)w;

  DEBUG_ASSERT (wid)

  // Is there an event or does the widget have focus?
  if (event_ready OR wid->has_focus)
    switch (wid->type) {
      case WIDGET_TYPE_HSLIDERBAR: processed_event = HSliderBar_Update (wid, event_ready, event);
                                   break;
      case WIDGET_TYPE_VSLIDERBAR: processed_event = VSliderBar_Update (wid, event_ready, event);
                                   break;
    }

  return (processed_event);
}

/*____________________________________________________________________
|
| Function: HSliderBar_Update
|
| Input: Called from Widget_Update()
| Output: Updates widget horizontal slider bar.  Returns true if event,
|   if any, is used by the widget.
|___________________________________________________________________*/
 
static bool HSliderBar_Update (WidgetData *wid, bool event_ready, evEvent *event)
{
  int x1, y1, x2, y2, hx1, hy1, hx2, hy2;
  bool processed_event = false;

  // Get screen coords for widget
  x1 = wid->window.xleft + 2;
  y1 = wid->window.ytop + 2 + (wid->hsliderbar.bar_dy / 2) - (wid->hsliderbar.handle_dy /2);
  x2 = wid->window.xright - 2;
  y2 = y1 + wid->hsliderbar.handle_dy - 1;

  // Get screen coords for handle in current position
  hx1 = wid->window.xleft + 2 + wid->hsliderbar.handle - (wid->hsliderbar.handle_dx / 2);
  hy1 = y1;
  hx2 = hx1 + wid->hsliderbar.handle_dx - 1;
  hy2 = y2;

  // Process event?
  if (event_ready) {
    // Gain focus?
    if ((NOT wid->has_focus) AND (event->type == evTYPE_MOUSE_LEFT_PRESS)) {
      // Is click inside widget?
      if (msMouseInBox (x1, y1, x2, y2, event->x, event->y)) {
        // Is click inside handle?
        if (msMouseInBox (hx1, hy1, hx2, hy2, event->x, event->y)) 
          wid->focus_offset = event->x - (hx1+(hx2-hx1)/2);
        else 
          wid->focus_offset = 0;
        HSliderBar_Update_Handle (wid, event->x, event->y);
        wid->has_focus = true;
        processed_event = true;
      }
    }
    // Release focus?
    else if (wid->has_focus AND (event->type == evTYPE_MOUSE_LEFT_RELEASE)) {
      HSliderBar_Update_Handle (wid, event->x, event->y);
      wid->has_focus = false;
      processed_event = true;
    }
  }
  // Process focus with no new event?
  else if (wid->has_focus) {
    msGetMouseCoords (&x1, &y1);
    HSliderBar_Update_Handle (wid, x1, y1);
  }

  return (processed_event);
}

/*____________________________________________________________________
|
| Function: HSliderBar_Update_Handle
|
| Input: Called from HSliderBar_Update()
| Output: Updates handle position based on current mouse position (mx, my)
|   and previously saved focus position.  Also updates value.
|___________________________________________________________________*/
 
static void HSliderBar_Update_Handle (WidgetData *wid, int mx, int my)
{
  int left, right, lastx, diff;
  float t;

  // Get edges of bar, in screen coords
  left  = wid->window.xleft  + 2;
  right = wid->window.xright - 2;

  // Compute last mouse position, in screen coords
  lastx = wid->window.xleft + 2 + wid->hsliderbar.handle + wid->focus_offset;
  
  // Compute difference between last and current mouse x position
  diff = mx - lastx;

  // Compute new handle position
  wid->hsliderbar.handle += diff;
  if (wid->hsliderbar.handle < 0)
    wid->hsliderbar.handle = 0;
  else if (wid->hsliderbar.handle > wid->hsliderbar.bar_dx-4)
    wid->hsliderbar.handle = wid->hsliderbar.bar_dx-4;

  // Compute new value based on handle position
  t = wid->hsliderbar.handle / (float)(wid->hsliderbar.bar_dx - 4);
  if (wid->hsliderbar.value_type == VALUE_TYPE_INT)
    *wid->hsliderbar.value.i.value = (int) gx3d_Lerp ((float)(wid->hsliderbar.value.i.min), (float)(wid->hsliderbar.value.i.max), t);
  else // float
    *wid->hsliderbar.value.f.value = gx3d_Lerp (wid->hsliderbar.value.f.min, wid->hsliderbar.value.f.max, t);
}

/*____________________________________________________________________
|
| Function: VSliderBar_Update
|
| Input: Called from Widget_Update()
| Output: Updates widget vertical slider bar.  Returns true if event,
|   if any, is used by the widget.
|___________________________________________________________________*/
 
static bool VSliderBar_Update (WidgetData *wid, bool event_ready, evEvent *event)
{
  bool processed_event = false;

  // ADD CODE HERE

  return (processed_event);
}

/*____________________________________________________________________
|
| Function: Widget_UpdateAll
|
| Input: Called from ____.
| Output: Updates all widgets.  Returns true if event, if any, is used by 
|   any widget.
|___________________________________________________________________*/
 
bool Widget_UpdateAll (bool event_ready, evEvent *event)
{
  bool processed_event = false;
  WidgetData *wid;

  for (wid=widgetlist; wid; wid=wid->next) {
    processed_event = Widget_Update ((Widget)wid, event_ready, event);
    if (processed_event)
      break;
  }

  return (processed_event);
}

/*____________________________________________________________________
|
| Function: Widget_Draw
|
| Input: Called from Widget_Update()
| Output: Draws widgethorizontal slider bar.
|___________________________________________________________________*/
 
void Widget_Draw (Widget w)
{  
	WidgetData *wid = (WidgetData *)w;

  DEBUG_ASSERT (wid)

  switch (wid->type) {
    case WIDGET_TYPE_HSLIDERBAR: HSliderBar_Draw (wid);
                                 break;
    case WIDGET_TYPE_VSLIDERBAR: VSliderBar_Draw (wid);
                                 break;
  }
}
  
/*____________________________________________________________________
|
| Function: HSliderBar_Draw
|
| Input: Called from Widget_Draw()
| Output: Draws widget horizontal slider bar.
|___________________________________________________________________*/
 
static void HSliderBar_Draw (WidgetData *wid)
{
  int x, y;
  char str[80];
  gxState state;
  gxRectangle screen;
  gxColor color_black, color_red, color_green, color_blue;

  // Init variables
  screen.xleft   = 0;
  screen.ytop    = 0;
  screen.xright  = gxGetScreenWidth() - 1;
  screen.ybottom = gxGetScreenHeight() - 1;

  color_black.index = 0;
  color_red.index = 0;
  color_red.r = 255;
  color_green.index = 0;
  color_green.g = 255;
  color_blue.index = 0;
  color_blue.b = 255;

  // Save graphics state
  gxSaveState (&state);

  gxSetWindow (&wid->window);
  gxSetClip   (&screen);
  gxSetClipping (true);

//  msHideMouse ();
  // Draw value
  gxSetFont (widget_font);
  if (wid->hsliderbar.value_type == VALUE_TYPE_INT)
    sprintf (str, "%d", *wid->hsliderbar.value.i.value);
  else // float
    sprintf (str, "%.3f", *wid->hsliderbar.value.f.value);
  x = gxGetMaxX() + 5;
  y = (gxGetMaxY() / 2) - (gxGetFontHeight() / 2);
  gxSetColor (color_black);
  gxDrawText (str, x+1, y+1);
  gxSetColor (color_green);
  gxDrawText (str, x, y);

  // Draw title
  if (wid->title) {
    x = (gxGetMaxX() / 2) - (gxGetStringWidth (wid->title) / 2);
    y = 2 + (wid->hsliderbar.bar_dy / 2) - (wid->hsliderbar.handle_dy /2) - 3 - gxGetFontHeight();
    gxSetColor (color_black);
    gxDrawText (wid->title, x+1, y+1);
    gxSetColor (color_green);
    gxDrawText (wid->title, x, y);
  }
  // Draw black background for bar
  gxSetColor (color_black);
  gxDrawFillRectangle (0, 0, wid->hsliderbar.bar_dx+1, wid->hsliderbar.bar_dy+1);
  // Draw border of bar
  gxSetColor (color_green);
  gxDrawRectangle (1, 1, wid->hsliderbar.bar_dx, wid->hsliderbar.bar_dy);
  // Draw filled portion of bar
//  gxSetColor (color_red);
  gxSetColor (color_green);
  gxDrawFillRectangle (3, 3, 2+wid->hsliderbar.handle-2, gxGetMaxY()-3);
  // Draw handle
  x = 2 + wid->hsliderbar.handle - (wid->hsliderbar.handle_dx / 2);
  y = 2 + (wid->hsliderbar.bar_dy / 2) - (wid->hsliderbar.handle_dy /2);
  gxSetColor (color_black);
  gxDrawRectangle (x-1, y-1, x+wid->hsliderbar.handle_dx, y+wid->hsliderbar.handle_dy);
//  if (wid->has_focus)
//    gxSetColor (color_red);
//  else
    gxSetColor (color_green);
  gxDrawFillRectangle (x, y, x+wid->hsliderbar.handle_dx-1, y+wid->hsliderbar.handle_dy-1);

//  msShowMouse ();

  // Restore graphics state
  gxRestoreState (&state);
}

/*____________________________________________________________________
|
| Function: VSliderBar_Draw
|
| Input: Called from Widget_Draw()
| Output: Draws widget vertical slider bar.
|___________________________________________________________________*/
 
static void VSliderBar_Draw (WidgetData *wid)
{
  // ADD CODE HERE
}

/*____________________________________________________________________
|
| Function: Widget_DrawAll
|
| Input: Called from ____.
| Output: Draws all widgets.
|___________________________________________________________________*/
 
void Widget_DrawAll ()
{
  WidgetData *wid;

  for (wid=widgetlist; wid; wid=wid->next) 
    Widget_Draw ((Widget)wid);
}

/*____________________________________________________________________
|
| Function: Widget_ClearAllFocus
|
| Input: Called from ____.
| Output: Removes focus from any widget that has focus, if any.
|___________________________________________________________________*/

void Widget_ClearAllFocus ()
{
  WidgetData *wid;

  for (wid=widgetlist; wid; wid=wid->next) 
    wid->has_focus = false;
}

/*____________________________________________________________________
|
| Function: Init_Widget_Font
|
| Input: Called from ADD_TO_WIDGETLIST macro
| Output: Loads font to use with widgets (8x8 rom font)
|___________________________________________________________________*/

static void Init_Widget_Font ()
{
  // Load 8x8 font if not already loaded
  if (widget_font == 0) {
    widget_font = gxLoadFontData (gxFONT_TYPE_GX, (byte *)font_data_rom8x8, sizeof(font_data_rom8x8));
    if (widget_font == 0)
      DEBUG_ERROR ("Init_Widget_Font(): Error loading font")
  }
}

/*____________________________________________________________________
|
| Function: Free_Widget_Font
|
| Input: Called from REMOVE_FROM_WIDGETLIST macro
| Output: Frees widget font loaded by call to Init_Widget_Font()
|___________________________________________________________________*/

static void Free_Widget_Font ()
{
  if (widget_font) {
    gxFreeFont (widget_font);
    widget_font = 0;
  }
}
