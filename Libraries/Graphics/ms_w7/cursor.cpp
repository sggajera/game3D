/*____________________________________________________________________
|
| File: cursor.cpp
|
| Description: Functions to draw mouse cursor on screen using GX_XP
|       graphics library.
|
| Functions:    Cursor_Init
|                Compute_Save_Image
|               Cursor_Visible
|               Cursor_Update
|               Cursor_Show
|               Cursor_Hide
|
|               msCopyCursor
|               msEraseCursor
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

#include "cursor.h"

/*___________________
|
| Function prototypes
|__________________*/

static void Compute_Save_Image (
  int screen_x,
  int screen_y,
  int dx,
  int dy,
  gxBound *box );

/*___________________
|
| Defines
|__________________*/

#define SAVE_STATE                      \
  gxGetWindow (&win);                   \
  gxGetClip   (&clip);                  \
  clipping = gxGetClipping ();          \
  active_page = gxGetActivePage ();     \
  gxSetWindow (&screen_win);            \
  gxSetClip   (&screen_win);            \
  gxSetClipping (TRUE);                 \
  gxSetActivePage (gxGetVisualPage ());

#define RESTORE_STATE                   \
  gxSetWindow (&win);                   \
  gxSetClip   (&clip);                  \
  gxSetClipping (clipping);             \
  gxSetActivePage (active_page);

/*___________________
|
| Global variables
|__________________*/

static byte mouse_visible = FALSE;

static byte *save_image = NULL;         /* screen save buffer           */
static byte *temp_image = NULL;
static int save_x, save_y;              /* last drawn mouse position    */
static gxPage save_page, temp_page;
static int temp_x, temp_y;
static CursorInfo *cursor = NULL;
static int screen_dx, screen_dy;

static gxRectangle screen_win;

/*____________________________________________________________________
|
| Function: Cursor_Init
|
| Input: Called from msStartMouse(), msSetCursor()
| Output: Initializes mouse cursor, screen save buffer, etc.
|       Call this routine with NULL to free current cursor.  This frees
|       memory associated with the cursor.  This routine should only be
|       called when mouse cursor is not visible.
|___________________________________________________________________*/

void Cursor_Init (CursorInfo *cursor_def)
{
  gxBound box;

/*____________________________________________________________________
|
| Init globals
|___________________________________________________________________*/

  screen_dx          = gxGetScreenWidth ();
  screen_dy          = gxGetScreenHeight ();
  screen_win.xleft   = 0;
  screen_win.ytop    = 0;
  screen_win.xright  = screen_dx-1;
  screen_win.ybottom = screen_dy-1;

/*____________________________________________________________________
|
| Delete current cursor, if any
|___________________________________________________________________*/

  if (cursor) {
    /* Free memory for save buffers */
    if (save_image) {
      free (save_image);
      save_image = NULL;
    }
    if (temp_image) {
      free (temp_image);
      temp_image = NULL;
    }
    /* Set globals */
    cursor = NULL;
  }

/*____________________________________________________________________
|
| Create new cursor?
|___________________________________________________________________*/

  if (cursor_def) {
    cursor = cursor_def;
    box.x = 0;
    box.y = 0;
    box.w = cursor->dx;
    box.h = cursor->dy;
    save_image = (byte *) malloc (gxImageSize (box));
    temp_image = (byte *) malloc (gxImageSize (box));
    /* Error checking on malloc() */
    if ((save_image == NULL) OR (temp_image == NULL)) {
      if (save_image) {
        free (save_image);
        save_image = NULL;
      }
      if (temp_image) {
        free (temp_image);
        temp_image = NULL;
      }
      cursor = NULL;
    }
  }
}

/*____________________________________________________________________
|
| Function: Compute_Save_Image
|
| Input: Called from Cursor_Init()
| Output: Returns in box the position and size of the screen image
|       under the cursor.
|___________________________________________________________________*/

static void Compute_Save_Image (
  int screen_x,
  int screen_y,
  int dx,
  int dy,
  gxBound *box )
{
  if (screen_x < 0)
    screen_x = 0;
  else if ((screen_x + dx) > screen_dx)
    screen_x = screen_dx - dx;

  if (screen_y < 0)
    screen_y = 0;
  else if ((screen_y + dy) > screen_dy)
    screen_y = screen_dy - dy;

  box->x = screen_x;
  box->y = screen_y;
  box->w = dx;
  box->h = dy;
}

/*____________________________________________________________________
|
| Function: Cursor_Visible
|
| Output: Returns true if mouse cursor is currently visible.
|___________________________________________________________________*/

int Cursor_Visible (void)
{
  return (mouse_visible);
}

/*____________________________________________________________________
|
| Function: Cursor_Update
|
| Input: Called from msUpdateMouse()
| Output: Redraws new position of mouse cursor.
|___________________________________________________________________*/

void Cursor_Update (int x, int y)
{
  static gxBound box;
  static gxRectangle win, clip;
  static gxPage active_page;
  static int clipping;
  gxColor black;

  if (mouse_visible) {
    /* Init drawing area */
    SAVE_STATE
    /* Hide current cursor */
    gxSetActivePage (save_page);
    gxDrawImage (save_image, save_x, save_y);
    gxSetActivePage (gxGetVisualPage ());
    Compute_Save_Image (x-cursor->x, y-cursor->y, cursor->dx, cursor->dy, &box);
    gxGetImage (box, save_image);
    save_x    = box.x;
    save_y    = box.y;
    save_page = gxGetActivePage ();
    /* Draw cursor */
    if (cursor->type == CURSOR_TYPE_SPRITE)
      gxDrawSprite (cursor->data1, x-cursor->x, y-cursor->y);
    else {
      black.index = 0;
      gxDrawBitmap (cursor->data1, x-cursor->x, y-cursor->y, black);
      gxDrawBitmap (cursor->data2, x-cursor->x, y-cursor->y, cursor->color);
    }

    /* Restore original drawing state */
    RESTORE_STATE
  }
}

/*____________________________________________________________________
|
| Function: Cursor_Show
|
| Input: Called from msShowMouse()
| Output: If mouse cursor isn't visible, draws it.
|___________________________________________________________________*/

void Cursor_Show (void)
{
  int x, y, button;
  gxBound box;
  gxRectangle win, clip;
  gxPage active_page;
  int clipping;
  gxColor black;

  if (NOT mouse_visible) {
    /* Init drawing area */
    SAVE_STATE
    /* Save screen background */
    msGetMouseStatus (&x, &y, &button);
    Compute_Save_Image (x-cursor->x, y-cursor->y, cursor->dx, cursor->dy, &box);
    gxGetImage (box, save_image);
    save_x    = box.x;
    save_y    = box.y;
    save_page = gxGetActivePage ();
    /* Draw cursor */
    if (cursor->type == CURSOR_TYPE_SPRITE)
      gxDrawSprite (cursor->data1, x-cursor->x, y-cursor->y);
    else {
      black.index = 0;
      gxDrawBitmap (cursor->data1, x-cursor->x, y-cursor->y, black);
      gxDrawBitmap (cursor->data2, x-cursor->x, y-cursor->y, cursor->color);
    }
    /* Restore original drawing state */
    RESTORE_STATE

    mouse_visible = TRUE;
  }
}

/*____________________________________________________________________
|
| Function: Cursor_Hide
|
| Input: Called from msStopMouse(), msHideMouse()
| Output: If mouse cursor is visible, hides it.
|___________________________________________________________________*/

void Cursor_Hide (void)
{
  gxRectangle win, clip;
  gxPage active_page;
  int clipping;

  if (mouse_visible) {
    SAVE_STATE
    gxSetActivePage (save_page);
    gxDrawImage (save_image, save_x, save_y);
    RESTORE_STATE
    mouse_visible = FALSE;
  }
}

/*____________________________________________________________________
|
| Function: msCopyCursor
|
| Outputs: Draws cursor, if visible, on page at last position.  Saves
|       screen contents in a temp buffer.
|
| Description: Use msvCopyCursor() and msvEraseCursor() to enable a
|       flicker-free cursor when page swapping.  See LM32 library (file
|       LM_DISP.C, function Display()) for an example.
|___________________________________________________________________*/

void msCopyCursor (int page)
{
  int x, y, button;
  gxBound box;
  gxRectangle win, clip;
  gxPage active_page;
  int clipping;
  gxColor black;

  if (mouse_visible) {
    /* Init drawing area */
    SAVE_STATE
    gxSetActivePage (page);
    /* Save screen background */
    msGetMouseStatus (&x, &y, &button);
    Compute_Save_Image (x-cursor->x, y-cursor->y, cursor->dx, cursor->dy, &box);
    gxGetImage (box, temp_image);
    temp_x    = box.x;
    temp_y    = box.y;
    temp_page = page;
    /* Draw cursor */
    if (cursor->type == CURSOR_TYPE_SPRITE)
      gxDrawSprite (cursor->data1, x-cursor->x, y-cursor->y);
    else {
      black.index = 0;
      gxDrawBitmap (cursor->data1, x-cursor->x, y-cursor->y, black);
      gxDrawBitmap (cursor->data2, x-cursor->x, y-cursor->y, cursor->color);
    }
    /* Restore original drawing state */
    RESTORE_STATE
  }
}

/*____________________________________________________________________
|
| Function: msEraseCursor
|
| Outputs: Erases cursor from last page and sets save buffer to data
|       saved by msCopyCursor().
|___________________________________________________________________*/

void msEraseCursor (void)
{
  gxRectangle win, clip;
  gxPage active_page;
  int clipping;
  byte *tmp;

  if (mouse_visible) {
    /* Hide cursor */
    SAVE_STATE
    gxSetActivePage (save_page);
    gxDrawImage (save_image, save_x, save_y);
    RESTORE_STATE
    /* Swap save buffers */
    tmp        = save_image;
    save_image = temp_image;
    temp_image = tmp;
    save_x     = temp_x;
    save_y     = temp_y;
    save_page  = temp_page;
  }
}
