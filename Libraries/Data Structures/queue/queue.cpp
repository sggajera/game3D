/*____________________________________________________________________
|
| File: queue.cpp
|
| Description: A general-purpose library to implement an array-based
|   queue.  All functions are thread-safe.
|
| Functions:  queue_Init
|             queue_Free
|             queue_Empty
|             queue_Full
|             queue_Flush        
|             queue_Size
|             queue_Add
|             queue_Remove    
|             queue_Remove
|             queue_Add_Unique
|             queue_Remove_Selected_Entries
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

#include "queue.h"

/*___________________
|
| Type definitions
|__________________*/

struct QueueData {
  int      front, rear, max_items, count;
  byte    *data;
  unsigned item_size;
};

/*___________________
|
| Macros
|__________________*/

#define QUEUE ((QueueData *)queue)

#define QUEUE_DATA(_index_) (QUEUE->data[(_index_)*QUEUE->item_size])
#define NEXT(_i_)           (((_i_)+1)%QUEUE->max_items)
#define PREVIOUS(_i_)       (((_i_)-1+QUEUE->max_items)%QUEUE->max_items)

/*____________________________________________________________________
|
| Function: queue_Init
|
| Output: Initializes a queue.  Returns true if queue initialized, else
|   false on any error.
|___________________________________________________________________*/

Queue queue_Init (int max_items, unsigned item_size)
{
  QueueData *qdata = 0;

  DEBUG_ASSERT (max_items > 0)
  DEBUG_ASSERT (item_size > 0)

  qdata = new QueueData;
  if (qdata) {
    qdata->front     = 0;
    qdata->rear      = 0;
    qdata->count     = 0;
    qdata->max_items = max_items;
    qdata->item_size = item_size;
    qdata->data      = new byte [max_items * item_size];
    if (qdata->data == NULL) {
      delete qdata;
      qdata = 0;
    }
  }

  return ((Queue)qdata);
}

/*____________________________________________________________________
|
| Function: queue_Free
|
| Output: Frees all resources used by queue.
|___________________________________________________________________*/

void queue_Free (Queue queue)
{
  DEBUG_ASSERT (queue)

  if (queue) {
    if (QUEUE->data) {
      delete [] QUEUE->data;
      QUEUE->data = 0;
    }
    delete QUEUE;
  }
}

/*____________________________________________________________________
|
| Function: queue_Empty
|
| Output: Returns true if queue is empty.
|___________________________________________________________________*/

bool queue_Empty (Queue queue)
{
  DEBUG_ASSERT (queue)

  return (QUEUE->count == 0);
}

/*____________________________________________________________________
|
| Function: queue_Full
|
| Output: Returns true if queue is full.
|___________________________________________________________________*/

bool queue_Full (Queue queue)
{
  DEBUG_ASSERT (queue)

  return (QUEUE->count == QUEUE->max_items);
}

/*____________________________________________________________________
|
| Function: queue_Flush
|
| Output: Flushes all data in queue.
|___________________________________________________________________*/

void queue_Flush (Queue queue)
{
  DEBUG_ASSERT (queue)

  QUEUE->front = 0;
  QUEUE->rear  = 0;
  QUEUE->count = 0;
}

/*____________________________________________________________________
|
| Function: queue_Size
|
| Output: Returns # of items in queue.
|___________________________________________________________________*/

int queue_Size (Queue queue)
{
  DEBUG_ASSERT (queue)

  return (QUEUE->count);
}

/*____________________________________________________________________
|
| Function: queue_Add
|
| Output: Adds bytes to queue.  Returns true if added, else false.
|___________________________________________________________________*/

bool queue_Add (Queue queue, void *item)
{
  bool added = false;

  DEBUG_ASSERT (queue)
  DEBUG_ASSERT (item)

  // Does buffer have room
  if (QUEUE->count != QUEUE->max_items) {
    memcpy (&QUEUE_DATA(QUEUE->rear), item, QUEUE->item_size);
    QUEUE->rear = NEXT(QUEUE->rear);
    QUEUE->count++;
    added = true;
  }
  
  return (added);
}

/*____________________________________________________________________
|
| Function: queue_Remove
|
| Output: Removes bytes from queue.  Returns true if removed, else false.
|___________________________________________________________________*/

bool queue_Remove (Queue queue, void *item)
{
  bool removed = false;

  DEBUG_ASSERT (queue)
  DEBUG_ASSERT (item)

  // Does buffer have data available
  if (QUEUE->count) {
    memcpy (item, &QUEUE_DATA(QUEUE->front), QUEUE->item_size);
    QUEUE->front = NEXT(QUEUE->front);
    QUEUE->count--;
    removed = true;
  }
  
  return (removed);
}

/*____________________________________________________________________
|
| Function: queue_Remove
|
| Output: Removes bytes from queue.  Returns true if removed, else false.
|___________________________________________________________________*/

bool queue_Remove (Queue queue)
{
  bool removed = false;

  DEBUG_ASSERT (queue)

  // Does buffer have data available
  if (QUEUE->count) {
    QUEUE->front = NEXT(QUEUE->front);
    QUEUE->count--;
    removed = true;
  }
  
  return (removed);
}

/*____________________________________________________________________
|
| Function: queue_Add_Unique
|
| Output: If queue not full and no other item with same type in queue, 
|   adds item.
|___________________________________________________________________*/

bool Queue_Add_Unique (Queue queue, void *item, void *item_type, IdentifyQueueItemType identify)
{
  int i;
  bool found;
  bool added = false;

  DEBUG_ASSERT (queue)
  DEBUG_ASSERT (item)
  DEBUG_ASSERT (item_type)
  DEBUG_ASSERT (identify)

  // Does buffer have room
  if (QUEUE->count != QUEUE->max_items) {
    for (i=QUEUE->front, found=false; (i != QUEUE->rear) AND (NOT found); i=NEXT(i))
      if (identify (&QUEUE_DATA(i), item_type))
        found = true;
    if (NOT found)
      added = queue_Add (queue, item);
  }

  return (added);
}

/*____________________________________________________________________
|
| Function: queue_Remove_Selected_Entries
|
| Output: Removes all entries in queue with same type.
|___________________________________________________________________*/

void queue_Remove_Selected_Entries (Queue queue, void *item_types, IdentifyQueueItemType identify)
{
	int i, tfront;
		
  DEBUG_ASSERT (queue)
  DEBUG_ASSERT (item_types)
  DEBUG_ASSERT (identify)

  if (QUEUE->count) {
    for (tfront=QUEUE->front; tfront != QUEUE->rear; ) 
      // Is this item a type of item to remove?
      if (identify (&QUEUE_DATA(tfront), item_types)) {
        for (i=tfront; i != QUEUE->rear; i=NEXT(i)) 
				  memcpy (&QUEUE_DATA(i), &QUEUE_DATA(NEXT(i)), QUEUE->item_size);
        QUEUE->rear = PREVIOUS(QUEUE->rear);
        QUEUE->count--;
      }
      else
        tfront = NEXT(tfront);
  }
}
