/*____________________________________________________________________
|
| File: stack.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _STACK_H_
#define _STACK_H_

/*___________________
|
| Type definitions
|__________________*/

typedef void *Stack;
  
// Callback function to return true if item is of item_type
typedef int (*IdentifyStackItemType) (void *item, void *item_type);

/*___________________
|
| Functions
|__________________*/

Stack stack_Init       (int max_items, unsigned item_size);
void  stack_Free       (Stack Stack);
bool  stack_Empty      (Stack Stack);
bool  stack_Full       (Stack Stack);
void  stack_Flush      (Stack Stack);
int   stack_Size       (Stack Stack);
bool  stack_Push       (Stack Stack, void *item);
bool  stack_Pop        (Stack Stack, void *item); 
bool  stack_Pop        (Stack Stack);
bool  stack_Push_Unique (Stack Stack, void *item, void *item_type, IdentifyStackItemType identify);
void  stack_Pop_Selected_Entries (Stack Stack, void *item_types, IdentifyStackItemType identify);

#endif
