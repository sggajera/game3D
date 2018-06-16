/*____________________________________________________________________
|
| File: dqueue.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _DQUEUE_H_
#define _DQUEUE_H_

/*___________________
|
| Type definitions
|__________________*/

typedef void *DQueue;

/*___________________
|
| Functions
|__________________*/

DQueue dqueue_Init   (unsigned item_size);
void   dqueue_Free   (DQueue queue);
void   dqueue_Copy   (DQueue dst, DQueue src);
bool   dqueue_Empty  (DQueue queue);
void   dqueue_Flush  (DQueue queue);
int    dqueue_Size   (DQueue queue);
void   dqueue_Add    (DQueue queue, byte *item);
bool   dqueue_Remove (DQueue queue, byte *item);
bool   dqueue_Remove (DQueue queue);
void	 dqueue_Sort   (DQueue queue, int (*compare_func)(const void *a, const void *b));

#endif
