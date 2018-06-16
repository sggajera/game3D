/*____________________________________________________________________
|
| File: hashtable.cpp
|
| Description: A general-purpose library to implement an array-based
|   hashtable using open addressing.  All functions are thread-safe.
|
|   The hash table stores an integer value in each slot.  Also, stored
|   is a key associated with the value.  An example of using this would
|   be to have a key such as: struct {float x,y,z} and an integer value
|   associated with each key.
|
| Functions:  hashtable_Init
|             hashtable_Free
|             hashtable_Empty
|             hashtable_Full
|             hashtable_Flush        
|             hashtable_Size
|             hashtable_Write
|             hashtable_Read
|             hashtable_Dump
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|
| DEBUG_ASSERTED!
|___________________________________________________________________*/

#define KEEP_STATS  // keeps track of stats
#define PRINT_STATS // when table is freed, writes stats to debug file, if stats have been kept

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

#include "hashtable.h"

/*___________________
|
| Type definitions
|__________________*/

struct HashTableData {
  int          num_buckets; 
  int          num_slots;
  int          max_count;   // max # entries (num_buckets * num_slots)
  int          count;       // # entries filled in
  unsigned     key_size;
  int         *values; 
  byte        *keys;        // key buffer
  HashFunction hash;        // User-defined callback hash function
  // Stats
  int          reads;
  int          compares;
  int          overflows;
};

/*___________________
|
| Macros
|__________________*/

#define HASHTABLE ((HashTableData *)ht)

/*____________________________________________________________________
|
| Function: hashtable_Init
|
| Output: Initializes a hash table.  Returns true if initialized, else
|   false on any error.
|___________________________________________________________________*/

HashTable hashtable_Init (int num_buckets, int num_slots, unsigned key_size, HashFunction hash)
{
  bool error;
  HashTableData *htdata = 0;

  // Validate input params
  DEBUG_ASSERT (num_buckets > 0)
  DEBUG_ASSERT (num_slots > 0)
  DEBUG_ASSERT (hash)

  error = false;
  // Create hash table
  htdata = (HashTableData *) malloc (sizeof(HashTableData));
  if (htdata) {
    // Init members
    htdata->num_buckets = num_buckets;
    htdata->num_slots   = num_slots;
    htdata->max_count   = num_buckets * num_slots;
    htdata->count       = 0;
    htdata->key_size    = key_size;
    htdata->hash        = hash;
    // Init stats
    htdata->reads       = 0;
    htdata->compares    = 0;
    htdata->overflows   = 0;
    // Create array of slots
    htdata->values = (int *) malloc (num_buckets * num_slots * sizeof(int));
    if (htdata->values == 0)
      error = true;
    else
      // Set all buckets to empty
      hashtable_Flush ((HashTable)htdata);
    // Create array of keys
    htdata->keys = (byte *) malloc (num_buckets * num_slots * key_size * sizeof(byte));
    if (htdata->keys == 0)
      error = true;
  }

  // On any error, free hash table
  if (error) {
    hashtable_Free ((HashTable)htdata);
    htdata = 0;
    DEBUG_ERROR ("hashtable_Init(): Error creating hash table")
  }

  return ((HashTable)htdata);
}

/*____________________________________________________________________
|
| Function: hashtable_Free
|                                      
| Output: Frees all resources used by hash table.
|___________________________________________________________________*/

void hashtable_Free (HashTable ht)
{
  // Validate input params
  DEBUG_ASSERT (ht)

  if (ht) {
#ifdef PRINT_STATS
    hashtable_PrintStats (ht);
#endif
    if (HASHTABLE->values) {
      free (HASHTABLE->values);
      HASHTABLE->values = 0;
    }
    if (HASHTABLE->keys) {
      free (HASHTABLE->keys);
      HASHTABLE->keys = 0;
    }
    free (HASHTABLE);
  }
}

/*____________________________________________________________________
|
| Function: hashtable_Empty
|
| Output: Returns true if hash table is empty.
|___________________________________________________________________*/

bool hashtable_Empty (HashTable ht)
{
  // Validate input params
  DEBUG_ASSERT (ht)

  return (HASHTABLE->count == 0);
}

/*____________________________________________________________________
|
| Function: hashtable_Full
|
| Output: Returns true if hash table is full.
|___________________________________________________________________*/

bool hashtable_Full (HashTable ht)
{
  // Validate input params
  DEBUG_ASSERT (ht)

  return (HASHTABLE->count == HASHTABLE->max_count);
}

/*____________________________________________________________________
|
| Function: hashtable_Flush
|
| Output: Flushes all data in hash table.
|___________________________________________________________________*/

void hashtable_Flush (HashTable ht)
{
  int i;

  // Validate input params
  DEBUG_ASSERT (ht)

  // Set all bucket values to empty
  for (i=0; i<HASHTABLE->max_count; i++)
    HASHTABLE->values[i] = hashtable_EMPTY_SLOT;
  HASHTABLE->count = 0;
}

/*____________________________________________________________________
|
| Function: hashtable_Size
|
| Output: Returns # of items in hash table.
|___________________________________________________________________*/

int hashtable_Size (HashTable ht)
{
  // Validate input params
  DEBUG_ASSERT (ht)

  return (HASHTABLE->count);
}

/*____________________________________________________________________
|
| Function: hashtable_PrintStats
|
| Output: Prints status about usage of hash table.
|___________________________________________________________________*/

void hashtable_PrintStats (HashTable ht)
{
  // Validate input params
  DEBUG_ASSERT (ht)

#ifdef _DEBUG // Only do this in debug builds
#ifdef KEEP_STATS
  char str[128];
  // Print out stats
  DEBUG_WRITE ("_______________ HashTable Stats ______________")
  sprintf (str, "%d BUCKETS, %d SLOTS", HASHTABLE->num_buckets, HASHTABLE->num_slots);
  DEBUG_WRITE (str)
  sprintf (str, "Number of reads: %d", HASHTABLE->reads);
  DEBUG_WRITE (str)
  sprintf (str, "Total comparisions: %d", HASHTABLE->compares);
  DEBUG_WRITE (str)
  sprintf (str, "Total overflows: %d", HASHTABLE->overflows);
  DEBUG_WRITE (str)
  if (HASHTABLE->reads) {
    sprintf (str, "Average # of collisions per read: %f", (float)HASHTABLE->compares/(float)HASHTABLE->reads);
    DEBUG_WRITE (str)
    sprintf (str, "Average # of overflows per read: %f", (float)HASHTABLE->overflows/(float)HASHTABLE->reads);
    DEBUG_WRITE (str)
  }
  DEBUG_WRITE ("")
#endif
#endif
}

/*____________________________________________________________________
|
| Function: hashtable_Write
|
| Output: Adds an entry to the hashtable.  Returns true if added, else 
|   false.
|___________________________________________________________________*/

bool hashtable_Write (HashTable ht, int value, void *key)
{
  int bucket, slot;
  bool added = false;

  // Validate input params
  DEBUG_ASSERT (ht)
  DEBUG_ASSERT (key)

  // Is there room in table to add an entry?
  if (NOT hashtable_Full (HASHTABLE)) {
    // Get bucket slot for this key
    bucket = HASHTABLE->hash (key) % HASHTABLE->num_buckets;
    slot   = bucket * HASHTABLE->num_slots;
    // Add to table
    for (; NOT added; slot=(slot+1)%HASHTABLE->max_count) {
      if (HASHTABLE->values[slot] == hashtable_EMPTY_SLOT) {
        memcpy ((byte *)&(HASHTABLE->keys[slot*HASHTABLE->key_size]), (byte *)key, HASHTABLE->key_size);
        HASHTABLE->values[slot] = value;
        HASHTABLE->count++;
        added = true;
      }
    }
  }

  return (added);
}

/*____________________________________________________________________
|
| Function: hashtable_Read
|
| Output: Reads a value from the hashtable given a key.  Assumes key
|   is in the table.  If not, returns hashtable_EMPTY_SLOT.
|___________________________________________________________________*/

int hashtable_Read (HashTable ht, void *key)
{
  int nbuckets, bucket, slot, nslots;
  bool found;
  int value = hashtable_EMPTY_SLOT;  // in case can't find

  // Validate input params
  DEBUG_ASSERT (ht)
  DEBUG_ASSERT (key)

  if (NOT hashtable_Empty (HASHTABLE)) {
    HASHTABLE->reads++;
    // Get bucket slot for this key
    bucket = HASHTABLE->hash (key) % HASHTABLE->num_buckets;
    slot   = bucket * HASHTABLE->num_slots;
    // Read from table
    found = false;
    for (nbuckets=0; (NOT found) AND (nbuckets < HASHTABLE->num_buckets); nbuckets++) {
      // Try reading from this bucket
      for (nslots=0; (NOT found) AND (nslots < HASHTABLE->num_slots); nslots++) {
        if (HASHTABLE->values[slot+nslots] != hashtable_EMPTY_SLOT) {
          HASHTABLE->compares++;
          if (!memcmp ((byte *)&(HASHTABLE->keys[(slot+nslots)*HASHTABLE->key_size]), (byte *)key, HASHTABLE->key_size)) {
            value = HASHTABLE->values[slot+nslots];
            found = true;
          }
        }
        else 
          found = true; // no x,y,z in table! (empty slots in this bucket!)
      }
      slot = (slot + HASHTABLE->num_slots) % HASHTABLE->max_count;
      if (NOT found)
        HASHTABLE->overflows++;
    }
  }

  return (value);
}
/*____________________________________________________________________
|
| Function: hashtable_Dump
|
| Output: Write to debug file all entries in table.
|___________________________________________________________________*/

struct Int3 {
  int x, y, z;
};

void hashtable_Dump (HashTable ht)
{
  int i;
  char str[200];
  Int3 *ip;

  // Validate input params
  DEBUG_ASSERT (ht)

  DEBUG_WRITE ("")
  DEBUG_WRITE ("Dumping Hash Table...")
  if (hashtable_Empty (HASHTABLE))
    DEBUG_WRITE ("--hash table empty--")
  else {
    for (i=0; i<HASHTABLE->max_count; i++) 
      if (HASHTABLE->values[i] != hashtable_EMPTY_SLOT) {
        ip = (Int3 *)(HASHTABLE->keys);
        sprintf (str, "%d at %d,%d,%d", HASHTABLE->values[i], ip[i].x, ip[i].y, ip[i].z);
        DEBUG_WRITE (str)
      }
  }
}
