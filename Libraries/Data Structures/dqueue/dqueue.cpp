/*____________________________________________________________________
|
| File: dqueue.cpp
|
| Description: A general-purpose library to implement a linked list
|   based (dynamic sized) queue.  All functions are thread-safe.
|
| Functions:  dqueue_Init
|             dqueue_Free
|             dqueue_Copy
|             dqueue_Empty
|             dqueue_Flush
|             dqueue_Size
|             dqueue_Add
|             dqueue_Remove
|             dqueue_Remove
|							dqueue_Sort
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

#include "dqueue.h"

/*___________________
|
| Type definitions
|__________________*/

struct Node {
  byte *item;
  Node *next;
};

struct DQueueData {
  Node    *front, *rear;
  int      count;
  unsigned item_size;
};

/*___________________
|
| Macros
|__________________*/

#define QUEUE ((DQueueData *)queue)

/*____________________________________________________________________
|
| Function: dqueue_Init
|
| Output: Initializes a queue.  Returns true if initialized, else false 
|   on any error.
|___________________________________________________________________*/

DQueue dqueue_Init (unsigned item_size)
{
  DQueueData *qdata = 0;

  DEBUG_ASSERT (item_size > 0)

  qdata = (DQueueData *) malloc (sizeof(DQueueData));
  if (qdata) {
    qdata->front     = 0;
    qdata->rear      = 0;
    qdata->count     = 0;
    qdata->item_size = item_size;
  }

  return ((DQueue)qdata);
}

/*____________________________________________________________________
|
| Function: dqueue_Free
|
| Output: Frees a queue.
|___________________________________________________________________*/

void dqueue_Free (DQueue queue)
{
  DEBUG_ASSERT (queue)

  if (queue) {
    dqueue_Flush (queue);
    free (QUEUE);
  }
}

/*____________________________________________________________________
|
| Function: dqueue_Copy
|
| Output: Performs deep copy from src queue to dst queue.
|___________________________________________________________________*/

void dqueue_Copy (DQueue dst, DQueue src)
{
  Node *np;

  DEBUG_ASSERT (dst)
  DEBUG_ASSERT (src)

  if (dst AND src) {
    dqueue_Flush (dst);
    for (np=((DQueueData *)src)->front; np; np=np->next)
      dqueue_Add (dst, np->item);
  }
}

/*____________________________________________________________________
|
| Function: dqueue_Empty
|
| Output: Returns true if queue is empty.
|___________________________________________________________________*/

bool dqueue_Empty (DQueue queue)
{
  DEBUG_ASSERT (queue)

  return (QUEUE->count == 0);
}

/*____________________________________________________________________
|
| Function: dqueue_Flush
|
| Output: Flushes all data in queue.
|___________________________________________________________________*/

void dqueue_Flush (DQueue queue)
{
  Node *temp;

  DEBUG_ASSERT (queue)

  while (QUEUE->front) {
    temp = QUEUE->front;
    QUEUE->front = QUEUE->front->next;
    free (temp->item);
    free (temp);
  }
  QUEUE->rear  = 0;
  QUEUE->count = 0;
}

/*____________________________________________________________________
|
| Function: dqueue_Size
|
| Output: Returns # of items in queue.
|___________________________________________________________________*/

int dqueue_Size (DQueue queue)
{
  DEBUG_ASSERT (queue)

  return (QUEUE->count);
}

/*____________________________________________________________________
|
| Function: dqueue_Add
|
| Output: Adds an item to the rear of the queue.
|___________________________________________________________________*/

void dqueue_Add (DQueue queue, byte *item)
{
  Node *newnode;

  DEBUG_ASSERT (queue)
  DEBUG_ASSERT (item)

  newnode = (Node *) malloc (sizeof(Node));
  newnode->item = (byte *) malloc (QUEUE->item_size);
  memcpy ((void *)(newnode->item), (void *)item, QUEUE->item_size);
  newnode->next = 0;

  if (QUEUE->front == 0)
    QUEUE->front = newnode;
  else 
    QUEUE->rear->next = newnode;
  QUEUE->rear = newnode;
  QUEUE->count++;
}

/*____________________________________________________________________
|
| Function: dqueue_Remove
|
| Output: Removes an item from the front of queue.  Returns true if 
|   removed, else false.
|___________________________________________________________________*/

bool dqueue_Remove (DQueue queue, byte *item)
{
  Node *temp;
  bool removed = false;

  DEBUG_ASSERT (queue)
  DEBUG_ASSERT (item)

  if (QUEUE->count) {
    memcpy ((void *)item, (void *)(QUEUE->front->item), QUEUE->item_size);
    temp = QUEUE->front;
    QUEUE->front = QUEUE->front->next;
    if (QUEUE->front == 0)
      QUEUE->rear = 0;
    free (temp->item);
    free (temp);
    QUEUE->count--;
    removed = true;
  }

  return (removed);
}

/*____________________________________________________________________
|
| Function: dqueue_Remove
|
| Output: Removes an item from the front of queue.  Returns true if 
|   removed, else false.
|___________________________________________________________________*/

bool dqueue_Remove (DQueue queue)
{
  Node *temp;
  bool removed = false;

  DEBUG_ASSERT (queue)

  if (QUEUE->count) {
    temp = QUEUE->front;
    QUEUE->front = QUEUE->front->next;
    if (QUEUE->front == 0)
      QUEUE->rear = 0;
    free (temp->item);
    free (temp);
    QUEUE->count--;
    removed = true;
  }

  return (removed);
}

/*____________________________________________________________________
|
| Function: dqueue_Sort
|
| Output: Sorts queue according to method of caller-defined comparison
|		function.  The comparison function should compare two queue elements
|		elements a and b and return;
|
|	  -1 if a < b
|	   0 if a == b
|	   1 if a > b
|
|		Elements are bubble sorted in ascending order, with the least cost
|		elements being put at the start of the queue.
|___________________________________________________________________*/

void dqueue_Sort (DQueue queue, int (*compare_func)(const void *a, const void *b))
{
  bool switched;
  Node *temp, **qpp;

  if (NOT dqueue_Empty(queue)) {
    // Keep sorting while any switches
    do {
      switched = false;
      // Go through entire queue
      for (qpp=&(QUEUE->front); (*qpp)->next; qpp=&((*qpp)->next))
				// switch 2 nodes?
				if ((compare_func)((*qpp)->item, (*qpp)->next->item) >= 1) {
					temp = *qpp;
					*qpp = temp->next;
					temp->next = (*qpp)->next;
					(*qpp)->next = temp;
					switched = true;
				}
    } while (switched);

    // Set new queue rear, in case rear has changed
		QUEUE->rear = *qpp;
  }
}
