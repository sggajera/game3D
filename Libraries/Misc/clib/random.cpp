/*____________________________________________________________________
|
| File: random.cpp
|
| Description: Functions to generate random numbers.  All functions are 
|   thread-safe.
|
| Functions:	random_Init
|							random_Init
|             random_Free
|             random_SetSeed
|             random_GetSeed
|             random_GetType
|							random_GetUnsigned
|							random_GetUnsigned	(default version)
|		          random_GetInt
|             random_GetInt     (default version)
|             random_GetFloat
|             random_GetFloat   (default version)
|             random_GetDouble   
|             random_GetDouble  (default version)
|             random_GetPercent
|             random_GetPercent (default version)
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
#include <time.h>

#include <defines.h>
#include "clib.h"

#include "mt.h"

/*___________________
|
| Type definitions
|__________________*/

typedef struct {
  RandomType  type;   // FAST or ...
  unsigned    seed;
} RandomData;  

/*___________________
|
| Defines
|__________________*/

#define MAX_RANDNUM 0xFFFFFFFF
//#define MAX_RANDNUM 0x7FFF  // 32767

// Generates a new random seed from 0 to MAX_RANDNUM
#define GENERATE_SEED_FAST_1(_r_)  ((_r_) = (_r_) * 214013L + 2531011L)
//#define GENERATE_SEED_FAST_1(_r_)  ((((_r_) = (_r_) * 1103515245 + 12345) >> 16) & 0x7FFF)

#define RANDOM ((RandomData *)r)

/*___________________
|
| Global variables
|__________________*/

// Used by default routines
static RandomData default_random = { randomTYPE_FAST_1, 1L };

/*____________________________________________________________________
|
| Function: random_Init
|
| Output: Initializes random # generator.
|___________________________________________________________________*/

Random random_Init (RandomType type)
{
  RandomData *random = NULL;

  random = (RandomData *) calloc (1, sizeof(RandomData));
  if (random) {
    random->type = type;
    // Set seed using system clock
    random->seed = (unsigned)time(0) + (unsigned)timeGetTime(); 
  }

  return ((Random)random);
}

/*____________________________________________________________________
|
| Function: random_Init
|
| Output: Initializes the default random # generator seed.
|___________________________________________________________________*/

void random_Init ()
{
  // Set seed using system clock
  default_random.seed = (unsigned)time(0) + (unsigned)timeGetTime();
	default_random.seed &= MAX_RANDNUM;
}

/*____________________________________________________________________
|
| Function: random_Free
|
| Output: Frees memory used by a random # generator.
|___________________________________________________________________*/

void random_Free (Random r)
{
  if (RANDOM) 
    free (RANDOM);
}

/*____________________________________________________________________
|
| Function: random_SetSeed
|
| Output: Initializes random # generator.  If seed is 0, sets seed 
|   using current system time.
|___________________________________________________________________*/

void random_SetSeed (Random r, unsigned seed)
{
	if (seed == 0) {
    RANDOM->seed = seed = (unsigned)time(0) + (unsigned)timeGetTime();
		RANDOM->seed &= MAX_RANDNUM;
	}
  else
    RANDOM->seed = seed;
}

/*____________________________________________________________________
|
| Function: random_GetSeed
|
| Output: Returns the current seed.
|___________________________________________________________________*/

unsigned random_GetSeed (Random r)
{
  return (RANDOM->seed);
}

/*____________________________________________________________________
|
| Function: random_GetType
|
| Output: Returns the random number type.
|___________________________________________________________________*/

RandomType random_GetType (Random r)
{
  return (RANDOM->type);
}

/*____________________________________________________________________
|
| Function: random_GetUnsigned
|
| Output: Returns a random unsigned integer.
|___________________________________________________________________*/

unsigned random_GetUnsigned (Random r)
{
  GENERATE_SEED_FAST_1(RANDOM->seed);
	return (RANDOM->seed);
}

/*____________________________________________________________________
|
| Function: random_GetUnsigned
|
| Output: Returns a random unsigned integer.
|___________________________________________________________________*/

unsigned random_GetUnsigned ()
{
  GENERATE_SEED_FAST_1(default_random.seed);
	return (default_random.seed);
}

/*____________________________________________________________________
|
| Function: random_GetInt
|
| Output: Returns a random integer within the range.
|___________________________________________________________________*/

int random_GetInt (Random r, int low, int high)
{
  int rnum;
  int pos_offset = 0;

  // If low number is negative, adjust to positive numbers
  if (low < 0) {
    pos_offset = -low;
    low += pos_offset;
    high += pos_offset;
  }

  // Compute random number
  if (high <= low)
    rnum = low - pos_offset;
  else {
//    rnum = RANDNUM(RANDOM->seed) % (high-low+1) + low - pos_offset;
    GENERATE_SEED_FAST_1(RANDOM->seed);
    rnum = RANDOM->seed / (MAX_RANDNUM / (unsigned)(high-low+1)) + low - pos_offset;
    if (rnum > high)
      rnum = high;
  }

  DEBUG_ASSERT ((rnum >= (low-pos_offset)) AND (rnum <= (high-pos_offset)))

  return (rnum);
}

/*____________________________________________________________________
|
| Function: random_GetInt
|
| Output: Returns a random integer within the range using the default
|   seed.
|___________________________________________________________________*/

inline int random_GetInt (int low, int high)
{
  return (random_GetInt ((Random)&default_random, low, high));
}

/*____________________________________________________________________
|
| Function: random_GetFloat
|
| Output: Returns a random float from 0 to 1.
|___________________________________________________________________*/

float random_GetFloat (Random r)
{
//  return ((float)RANDNUM(RANDOM->seed) / (float)MAX_RANDNUM);
  GENERATE_SEED_FAST_1(RANDOM->seed);
  return ((float)RANDOM->seed / (float)MAX_RANDNUM);
}

/*____________________________________________________________________
|
| Function: random_GetFloat
|
| Output: Returns a random float from 0 to 1 using the default seed.
|___________________________________________________________________*/

float random_GetFloat ()
{
//  return ((float)RANDNUM(default_random.seed) / (float)MAX_RANDNUM);
  GENERATE_SEED_FAST_1(default_random.seed);
  return ((float)default_random.seed / (float)MAX_RANDNUM);
}

/*____________________________________________________________________
|
| Function: random_GetDouble
|
| Output: Returns a random double from 0 to 1.
|___________________________________________________________________*/

double random_GetDouble (Random r)
{
//  return ((double)RANDNUM(RANDOM->seed) / (double)MAX_RANDNUM);
  GENERATE_SEED_FAST_1(RANDOM->seed);
  return ((double)RANDOM->seed / (double)MAX_RANDNUM);
}

/*____________________________________________________________________
|
| Function: random_GetDouble
|
| Output: Returns a random double from 0 to 1 using the default seed.
|___________________________________________________________________*/

double random_GetDouble ()
{
//  return ((double)RANDNUM(default_random.seed) / (double)MAX_RANDNUM);
  GENERATE_SEED_FAST_1(default_random.seed);
  return ((double)default_random.seed / (double)MAX_RANDNUM);
}

/*____________________________________________________________________
|
| Function: random_GetPercent
|
| Output: Returns a random float within the 0-100.
|___________________________________________________________________*/

float random_GetPercent (Random r)
{
  return (random_GetFloat (r) * 100);
}

/*____________________________________________________________________
|
| Function: random_GetPercent
|
| Output: Returns a random integer within the range using the default
|   seed.
|___________________________________________________________________*/

inline float random_GetPercent ()
{
  return (random_GetFloat () * 100);
}
