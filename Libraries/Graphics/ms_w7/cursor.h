/*____________________________________________________________________
|
| File: cursor.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Type definitions
|__________________*/

typedef struct {
  int     type;               // system, bitmap or sprite             
  gxColor color, mask_color;  // for system, bitmap cursors           
  int     x, y;               // hotspot                              
  int     dx, dy;             // size of cursor                       
  byte   *data1;              // screen mask bitmap, sprite or NULL   
  byte   *data2;              // cursor mask bitmap or NULL           
} CursorInfo;

/*___________________
|
| Defines
|__________________*/

// Cursor types 
#define CURSOR_TYPE_SYSTEM 1  // predefined bitmap    
#define CURSOR_TYPE_BITMAP 2  // user-defined bitmap  
#define CURSOR_TYPE_SPRITE 3  // user-defined sprite  

/*___________________
|
| Function prototypes
|__________________*/

void Cursor_Init (CursorInfo *cursor_def);
int  Cursor_Visible (void);
void Cursor_Update (int x, int y);
void Cursor_Show (void);
void Cursor_Hide (void);
