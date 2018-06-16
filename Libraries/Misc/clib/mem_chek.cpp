/*____________________________________________________________________
|
| File: mem_chek.c
| Object Module: mem_chek.obj
|
| Description: Contains C library functions to do run-time error checking
|	on dynamic memory allocation.
|
| Functions:  Mem_Chek_Start
|             Mem_Chek_Stop
|		          Mem_Chek_Blocks_Allocated
|		          Mem_Chek_Max_Blocks_Allocated
|             Mem_Chek_malloc
|		          Mem_Chek_calloc
|		          Mem_Chek_realloc
|		          Mem_Chek_free
|
| Note: This library writes (appends) any error messages to debug file.
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
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

#include <defines.h>

#include "clib.h"

#undef malloc
#undef calloc
#undef realloc
#undef free

/*___________________
|
| Type defintions
|__________________*/

struct Tombstone {
  size_t size;
  void  *address;
  int    line;
  char  *file;
  Tombstone *next;
};

/*___________________
|
| Global variables
|__________________*/

static CRITICAL_SECTION critsection;

static bool       using_mem_chek = false;
static Tombstone *mem_list = 0;
static int        blocks_allocated = 0;
static int        max_blocks_allocated = 0;

/*____________________________________________________________________
|
| Function: Mem_Chek_Start
|
| Output: Enables memory checking.  Should be done only once.
|___________________________________________________________________*/

void Mem_Chek_Start ()
{
  using_mem_chek       = true;
  mem_list             = 0;
  blocks_allocated     = 0;
  max_blocks_allocated = 0;
  InitializeCriticalSection (&critsection);
}

/*____________________________________________________________________
|
| Function: Mem_Chek_Stop
|
| Output: Frees all tombstones, Prints error message to debug file for
|   each memory block not already freed (still in list).
|___________________________________________________________________*/

void Mem_Chek_Stop ()
{
  void *p;
  char str[200];
  Tombstone *tsp;

  if (using_mem_chek) {
    // Go through list, if any
    while (mem_list) {
      // Detach tombstone from list
      tsp = mem_list;
      mem_list = mem_list->next;
      // Print error message
      sprintf (str, "Mem_Chek_Stop(): ERROR, memory leak (bytes: %d, line: %d, file: %s)", tsp->size, tsp->line, tsp->file);
      DEBUG_ERROR (str)
      // Free memory for tombstone and block
      if (tsp->file) 
        free (tsp->file);
      p = tsp->address;
      free (tsp);
      free (p);
    }
    using_mem_chek = false;
    DeleteCriticalSection (&critsection);
  }
}

/*____________________________________________________________________
|
| Function: Mem_Chek_Blocks_Allocated
|
| Output: Returns current number of memory blocks allocated by malloc(),
| 	calloc() and realloc()
|___________________________________________________________________*/

int Mem_Chek_Blocks_Allocated (void)
{
  return (blocks_allocated);
}

/*____________________________________________________________________
|
| Function: Mem_Chek_Max_Blocks_Allocated
|
| Output: Returns max number of memory blocks allocated by malloc(),
| 	calloc() and realloc()
|___________________________________________________________________*/

int Mem_Chek_Max_Blocks_Allocated (void)
{
  return (max_blocks_allocated);
}

/*____________________________________________________________________
|
| Function: Mem_Chek_malloc
|
| Output: Returns ptr to dynamic memory block.
|___________________________________________________________________*/

void *Mem_Chek_malloc (size_t size, int line, char *file)
{
  Tombstone *tsp;
  void *p = 0;

  // Allocate the memory
  p = malloc (size);
  if (p == 0) 
    DEBUG_ERROR ("Mem_Chek_malloc(): Error, can't allocate memory")

  // If using memory checks, create a tombstone
  if (p AND using_mem_chek) {
    tsp = (Tombstone *) calloc (1, sizeof(Tombstone));
    if (tsp == 0)
      DEBUG_ERROR ("Mem_Chek_malloc(): Error, can't allocate tombstone")
    else {
      tsp->size    = size;
      tsp->address = p;
      tsp->line    = line;
      if (file) {
        tsp->file = (char *) malloc (strlen(file)+1);
        if (tsp->file == 0)
          DEBUG_ERROR ("Mem_Chek_malloc(): Error, can't allocate char array")
        else
          strcpy (tsp->file, file);
      }
      // Add to start of list of tombstones
      EnterCriticalSection (&critsection);  
      if (mem_list == 0)
        mem_list = tsp;
      else {
        tsp->next = mem_list;
        mem_list = tsp;
      }
      blocks_allocated++;
      if (blocks_allocated > max_blocks_allocated)
        max_blocks_allocated = blocks_allocated;
      LeaveCriticalSection (&critsection);  
    }
  }

  return (p);
}

/*____________________________________________________________________
|
| Function: Mem_Chek_calloc
|
| Output: Returns ptr to dynamic memory block.
|___________________________________________________________________*/

void *Mem_Chek_calloc (size_t nitems, size_t size, int line, char *file)
{
  Tombstone *tsp;
  void *p = 0;

  // Allocate the memory
  p = calloc (nitems, size);
  if (p == 0) 
    DEBUG_ERROR ("Mem_Chek_calloc(): Error, can't allocate memory")

  // If using memory checks, create a tombstone
  if (p AND using_mem_chek) {
    tsp = (Tombstone *) calloc (1, sizeof(Tombstone));
    if (tsp == 0)
      DEBUG_ERROR ("Mem_Chek_calloc(): Error, can't allocate tombstone")
    else {
      tsp->size    = nitems * size;
      tsp->address = p;
      tsp->line    = line;
      if (file) {
        tsp->file = (char *) malloc (strlen(file)+1);
        if (tsp->file == 0)
          DEBUG_ERROR ("Mem_Chek_calloc(): Error, can't allocate char array")
        else
          strcpy (tsp->file, file);
      }
      // Add to start of list of tombstones
      EnterCriticalSection (&critsection);  
      if (mem_list == 0)
        mem_list = tsp;
      else {
        tsp->next = mem_list;
        mem_list = tsp;
      }
      blocks_allocated++;
      if (blocks_allocated > max_blocks_allocated)
        max_blocks_allocated = blocks_allocated;
      LeaveCriticalSection (&critsection);  
    }
  }

  return (p);
}

/*____________________________________________________________________
|
| Function: Mem_Chek_realloc
|
| Output: Returns ptr to dynamic memory block.
|___________________________________________________________________*/

void *Mem_Chek_realloc (void *memblock, size_t size, int line, char *file)
{
  char str[200];
  Tombstone **tspp;
  void *p = 0;

  // If using memory checks, find the previously created tombstone
  if (using_mem_chek) {
    EnterCriticalSection (&critsection);  
    // Look through list for this block
    for (tspp=&mem_list; *tspp AND ((*tspp)->address != memblock); tspp=&(*tspp)->next);
    // Found it?
    if (*tspp) {
      // Reallocate the memory
      p = realloc (memblock, size);
      if (p == 0) {
        LeaveCriticalSection (&critsection);  
        DEBUG_ERROR ("Mem_Chek_realloc(): Error, can't allocate memory")
      }
      else {
        // Update tombstone
        (*tspp)->size    = size;
        (*tspp)->address = p;
        LeaveCriticalSection (&critsection);  
      }
    }
    else {
      LeaveCriticalSection (&critsection);  
      sprintf (str, "Mem_Chek_realloc(): ERROR, block not previously allocated (line: %d, file: %s)", line, file);
      DEBUG_ERROR (str)
    }
  }
  else {
    // Just do a regular realloc if not using mem chek
    p = realloc (memblock, size);
    if (p == 0) 
      DEBUG_ERROR ("Mem_Chek_realloc(): Error, can't allocate memory")
  }

  return (p);
}

/*____________________________________________________________________
|
| Function: Mem_Chek_free
|
| Output: Frees a previously allocated memory block.
|___________________________________________________________________*/

void Mem_Chek_free (void *p, int line, char *file)
{
  char str[200];
  Tombstone *tsp, **tspp;

  // If using memory checks, find the tombstone for this block
  if (using_mem_chek) {
    EnterCriticalSection (&critsection);  
    // Look through list for this block
    for (tspp=&mem_list; *tspp AND ((*tspp)->address != p); tspp=&(*tspp)->next);
    // Found it?
    if (*tspp) {
      // Detach it from list
      tsp = *tspp;
      *tspp = (*tspp)->next;
      // Free memory for tombstone and block
      if (tsp->file) 
        free (tsp->file);
      free (tsp);
      free (p);
      blocks_allocated--;
      LeaveCriticalSection (&critsection);  
    }
    else {
      LeaveCriticalSection (&critsection);  
      sprintf (str, "Mem_Chek_free(): ERROR, free() has no effect (line: %d, file: %s)", line, file);
      DEBUG_ERROR (str)
    }
  }
  // Just do a regular free if not using mem chek
  else
    free (p);
}
