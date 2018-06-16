/*____________________________________________________________________
|
| File: queue.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _QUEUE_H_
#define _QUEUE_H_

/*___________________
|
| Type definitions
|__________________*/

typedef void *Queue;
  
// Callback function to return true if item is of item_type
typedef int (*IdentifyQueueItemType) (void *item, void *item_type);

/*___________________
|
| Functions
|__________________*/

Queue queue_Init       (int max_items, unsigned item_size);
void  queue_Free       (Queue queue);
bool  queue_Empty      (Queue queue);
bool  queue_Full       (Queue queue);
void  queue_Flush      (Queue queue);
int   queue_Size       (Queue queue);
bool  queue_Add        (Queue queue, void *item);
bool  queue_Remove     (Queue queue, void *item); 
bool  queue_Remove     (Queue queue);
bool  queue_Add_Unique (Queue queue, void *item, void *item_type, IdentifyQueueItemType identify);
void  queue_Remove_Selected_Entries (Queue queue, void *item_types, IdentifyQueueItemType identify);

#endif
