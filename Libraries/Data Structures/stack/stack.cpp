/*____________________________________________________________________
|
| File: stack.cpp
|
| Description: A general-purpose library to implement an array-based
|   stack.  All functions are thread-safe.
|
| Functions:  stack_Init
|             stack_Free
|             stack_Empty
|             stack_Full
|             stack_Flush        
|             stack_Size
|             stack_Push
|             stack_Pop
|             stack_Pop
|             stack_Add_Unique
|             stack_Remove_Selected_Entries
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|
| DEBUG_ASSERTED!
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <windows.h>
#include <winbase.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include <defines.h>
#include <clib.h>

#include "stack.h"

/*___________________
|
| Type definitions
|__________________*/

struct StackData {
  int      top, max_items, count;
  byte    *data;
  unsigned item_size;
};

/*___________________
|
| Macros
|__________________*/

#define STACK ((StackData *)stack)

#define STACK_DATA(_index_) (STACK->data[(_index_)*STACK->item_size])

/*____________________________________________________________________
|
| Function: stack_Init
|
| Output: Initializes a stack.  Returns true if stack initialized, else
|   false on any error.
|___________________________________________________________________*/

Stack stack_Init (int max_items, unsigned item_size)
{
  StackData *sdata = 0;

  DEBUG_ASSERT (max_items > 0)
  DEBUG_ASSERT (item_size > 0)

  sdata = new StackData;
  if (sdata) {
    sdata->top       = 0;
    sdata->count     = 0;
    sdata->max_items = max_items;
    sdata->item_size = item_size;
    sdata->data      = new byte [max_items * item_size];
    if (sdata->data == NULL) {
      delete sdata;
      sdata = 0;
    }
  }

  return ((Stack)sdata);
}

/*____________________________________________________________________
|
| Function: stack_Free
|
| Output: Frees all resources used by stack.
|___________________________________________________________________*/

void stack_Free (Stack stack)
{
  DEBUG_ASSERT (stack)

  if (stack) {
    if (STACK->data) {
      delete [] STACK->data;
      STACK->data = 0;
    }
    delete STACK;
  }
}

/*____________________________________________________________________
|
| Function: stack_Empty
|
| Output: Returns true if stack is empty.
|___________________________________________________________________*/

bool stack_Empty (Stack stack)
{
  DEBUG_ASSERT (stack)

  return (STACK->count == 0);
}

/*____________________________________________________________________
|
| Function: stack_Full
|
| Output: Returns true if stack is full.
|___________________________________________________________________*/

bool stack_Full (Stack stack)
{
  DEBUG_ASSERT (stack)

  return (STACK->count == STACK->max_items);
}

/*____________________________________________________________________
|
| Function: stack_Flush
|
| Output: Flushes all data in stack.
|___________________________________________________________________*/

void stack_Flush (Stack stack)
{
  DEBUG_ASSERT (stack)

  STACK->top   = 0;
  STACK->count = 0;
}

/*____________________________________________________________________
|
| Function: stack_Size
|
| Output: Returns # of items in stack.
|___________________________________________________________________*/

int stack_Size (Stack stack)
{
  DEBUG_ASSERT (stack)

  return (STACK->count);
}

/*____________________________________________________________________
|
| Function: stack_Push
|
| Output: Adds item to stack.  Returns true if added, else false.
|___________________________________________________________________*/

bool stack_Push (Stack stack, void *item)
{
  bool pushed = false;

  DEBUG_ASSERT (stack)
  DEBUG_ASSERT (item)

  // Does buffer have room
  if (STACK->count != STACK->max_items) {
    memcpy (&STACK_DATA(STACK->top), item, STACK->item_size);
    STACK->top++;
    STACK->count++;
    pushed = true;
  }
  
  return (pushed);
}

/*____________________________________________________________________
|
| Function: stack_Pop
|
| Output: Removes bytes from stack.  Returns true if removed, else false.
|___________________________________________________________________*/

bool stack_Pop (Stack stack, void *item)
{
  bool popped = false;

  DEBUG_ASSERT (stack)
  DEBUG_ASSERT (item)

  // Does buffer have data available
  if (STACK->count) {
    STACK->top--;
    memcpy (item, &STACK_DATA(STACK->top), STACK->item_size);
    STACK->count--;
    popped = true;
  }
  
  return (popped);
}

/*____________________________________________________________________
|
| Function: stack_Pop
|
| Output: Removes bytes from stack.  Returns true if removed, else false.
|___________________________________________________________________*/

bool stack_Pop (Stack stack)
{
  bool popped = false;

  DEBUG_ASSERT (stack)

  // Does buffer have data available
  if (STACK->count) {
    STACK->top--;
    STACK->count--;
    popped = true;
  }
  
  return (popped);
}

/*____________________________________________________________________
|
| Function: stack_Push_Unique
|
| Output: If stack not full and no other item with same type in stack, 
|   adds item.
|___________________________________________________________________*/

bool stack_Push_Unique (Stack stack, void *item, void *item_type, IdentifyStackItemType identify)
{
  int i;
  bool found;
  bool pushed = false;

  DEBUG_ASSERT (stack)
  DEBUG_ASSERT (item)
  DEBUG_ASSERT (item_type)
  DEBUG_ASSERT (identify)

  // Does buffer have room
  if (STACK->count != STACK->max_items) {
    for (i=STACK->top, found=false; (i >= 0) AND (NOT found); i--)
      if (identify (&STACK_DATA(i), item_type))
        found = true;
    if (NOT found)
      pushed = stack_Push (stack, item);
  }

  return (pushed);
}

/*____________________________________________________________________
|
| Function: stack_Pop_Selected_Entries
|
| Output: Removes all entries in stack with same type.
|___________________________________________________________________*/

void stack_Pop_Selected_Entries (Stack stack, void *item_types, IdentifyStackItemType identify)
{
	int i, j;

  DEBUG_ASSERT (stack)
  DEBUG_ASSERT (item_types)
  DEBUG_ASSERT (identify)

  for (i=0; i<=STACK->top; ) {
    // Is this item a type of item to remove?
    if (identify (&STACK_DATA(i), item_types)) {
      for (j=i; j<STACK->top; j++)
        memcpy (&STACK_DATA(j), &STACK_DATA(j+1), STACK->item_size);
      STACK->top--;
    }
    else
      i++;
  }
}
