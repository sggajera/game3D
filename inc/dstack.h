/*____________________________________________________________________
|
| File: dstack.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _DSTACK_H_
#define _DSTACK_H_

/*___________________
|
| Type definitions
|__________________*/

typedef void *DStack;
  
/*___________________
|
| Functions
|__________________*/

DStack dstack_Init  (unsigned bytes_per_entry);
void   dstack_Free  (DStack stack);
void   dstack_Copy  (DStack stack);
bool   dstack_Empty (DStack stack);
void   dstack_Flush (DStack stack);
int    dstack_Size  (DStack stack);
void   dstack_Push  (DStack stack, void *item);
bool   dstack_Pop   (DStack stack, void *item);
bool   dstack_Pop   (DStack stack);

#endif
