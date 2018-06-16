/*____________________________________________________________________
|
| File: dstack.cpp
|
| Description: A general-purpose library to implement a linked list
|   based (dynamic sized) stack.  All functions are thread-safe.
|
| Functions:  dstack_Init
|             dstack_Free
|             dstack_Copy
|              Copy_Stack
|             dstack_Empty
|             dstack_Flush
|             dstack_Size
|             dstack_Push
|             dstack_Pop
|             dstack_Pop
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

#include "dstack.h"

/*___________________
|
| Type definitions
|__________________*/

struct Node {
  void *item;
  Node *next;
};

struct DStackData {
  Node    *top;
  int      count;
  unsigned item_size;
};

/*___________________
|
| Function prototypes
|__________________*/

static void Copy_Stack (DStackData *dst, Node *node);

/*___________________
|
| Macros
|__________________*/

#define STACK ((DStackData *)stack)

/*____________________________________________________________________
|
| Function: dstack_Init
|
| Output: Initializes a stack.  Returns true if stack initialized, else
|   false on any error.
|___________________________________________________________________*/

DStack dstack_Init (unsigned item_size)
{
  DStackData *sdata = 0;

  DEBUG_ASSERT (item_size > 0)

  sdata = new DStackData;
  if (sdata) {
    sdata->top       = 0;
    sdata->count     = 0;
    sdata->item_size = item_size;
  }

  return ((DStack)sdata);
}

/*____________________________________________________________________
|
| Function: dstack_Free
|
| Output: Frees all resources used by stack.
|___________________________________________________________________*/

void dstack_Free (DStack stack)
{
  DEBUG_ASSERT (stack)

  if (stack) {
    dstack_Flush (stack);
    delete STACK;
  }
}

/*____________________________________________________________________
|
| Function: dstack_Copy
|
| Output: Performs deep copy from src queue to dst stack.
|___________________________________________________________________*/

void dstack_Copy (DStack dst, DStack src)
{
  DEBUG_ASSERT (dst)
  DEBUG_ASSERT (src)

  if (dst AND src) {
    dstack_Flush (dst);
    Copy_Stack ((DStackData *)dst, ((DStackData *)src)->top);
  }
}

/*____________________________________________________________________
|
| Function: Copy_Stack
|
| Input: Called from dstack_Copy()
| Output: Recursively copies stack contents to dst stack.
|___________________________________________________________________*/

static void Copy_Stack (DStackData *dst, Node *node)
{
  DEBUG_ASSERT (dst)
  DEBUG_ASSERT (node)

  if (node) {
    Copy_Stack (dst, node->next);
    dstack_Push (dst, node->item);
  }
}

/*____________________________________________________________________
|
| Function: dstack_Empty
|
| Output: Returns true if stack is empty.
|___________________________________________________________________*/

bool dstack_Empty (DStack stack)
{
  DEBUG_ASSERT (stack)

  return (STACK->count == 0);
}

/*____________________________________________________________________
|
| Function: dstack_Flush
|
| Output: Flushes all data in stack.
|___________________________________________________________________*/

void dstack_Flush (DStack stack)
{
  Node *temp;

  DEBUG_ASSERT (stack)

  while (STACK->top) {
    temp = STACK->top;
    STACK->top = STACK->top->next;
    delete [] temp->item;
    delete temp;
  }
  STACK->count = 0;
}

/*____________________________________________________________________
|
| Function: dstack_Size
|
| Output: Returns # of items in stack.
|___________________________________________________________________*/

int dstack_Size (DStack stack)
{
  DEBUG_ASSERT (stack)

  return (STACK->count);
}

/*____________________________________________________________________
|
| Function: dstack_Add
|
| Output: Adds an entry to stack.
|___________________________________________________________________*/

void dstack_Push (DStack stack, void *item)
{
  Node *newnode;

  DEBUG_ASSERT (stack)
  DEBUG_ASSERT (item)

  newnode = new Node;
  newnode->item = new byte [STACK->item_size];
  memcpy (newnode->item, item, STACK->item_size);
  newnode->next = 0;

  newnode->next = STACK->top;
  STACK->top = newnode;
  STACK->count++;
}

/*____________________________________________________________________
|
| Function: dstack_Pop
|
| Output: Removes an entry from stack.  Returns true if removed, else 
|   false.
|___________________________________________________________________*/

bool dstack_Remove (DStack stack, void *item)
{
  Node *temp;
  bool removed = false;

  DEBUG_ASSERT (stack)
  DEBUG_ASSERT (item)

  if (STACK->count) {
    memcpy (item, STACK->top->item, STACK->item_size);
    temp = STACK->top;
    STACK->top = STACK->top->next;
    delete [] temp->item;
    delete temp;
    STACK->count--;
    removed = true;
  }

  return (removed);
}

/*____________________________________________________________________
|
| Function: dstack_Pop
|
| Output: Removes an entry from stack.  Returns true if removed, else 
|   false.
|___________________________________________________________________*/

bool dstack_Remove (DStack stack)
{
  Node *temp;
  bool removed = false;

  DEBUG_ASSERT (stack)

  if (STACK->count) {
    temp = STACK->top;
    STACK->top = STACK->top->next;
    delete [] temp->item;
    delete temp;
    STACK->count--;
    removed = true;
  }

  return (removed);
}
