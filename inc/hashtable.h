/*____________________________________________________________________
|
| File: hashtable.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

/*___________________
|
| Type definitions
|__________________*/

typedef void *HashTable;

// Callback function to return integer (in range 0 to num_buckets-1) given a key
typedef int (*HashFunction) (const void *key);
  
/*___________________
|
| Constants
|__________________*/

#define hashtable_EMPTY_SLOT -1

/*___________________
|
| Functions
|__________________*/

HashTable hashtable_Init  (int num_buckets, int num_slots, unsigned key_size, HashFunction hash);
void      hashtable_Free  (HashTable ht);
bool      hashtable_Empty (HashTable ht);
bool      hashtable_Full  (HashTable ht);
void      hashtable_Flush (HashTable ht);
int       hashtable_Size  (HashTable ht);
void      hashtable_PrintStats (HashTable ht);
bool      hashtable_Write (HashTable ht, int value, void *key);
int       hashtable_Read (HashTable ht, void *key);
void      hashtable_Dump (HashTable ht);

#endif
