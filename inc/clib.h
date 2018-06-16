/*____________________________________________________________________
|
| File: clib.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _CLIB_H_
#define _CLIB_H_

#include <stdio.h>
#include <assert.h>

/*___________________
|
| Macros
|__________________*/

// Assert macro (caller needs to include stdio.h for this to work)
#ifdef _DEBUG
#define DEBUG_ASSERT(_assert_stuff_)                            \
  if (!(_assert_stuff_)) {                                      \
    char str[256];                                              \
    sprintf (str, "<ASSERT> line %d, %s", __LINE__, __FILE__);  \
    debug_WriteFile (str);                                      \
    assert (_assert_stuff_);                                    \
  }
#else
#define DEBUG_ASSERT(_assert_stuff_)  { /* nop */ }
#endif

// Writes error message to debug file
#ifdef _DEBUG
#define DEBUG_ERROR(_error_str_)                                          \
  {                                                                       \
    int i, j;                                                             \
    char tstr[256];                                                       \
    sprintf (tstr, "<ERROR> line %d, %s: ", __LINE__, __FILE__);          \
    for (i=(int)strlen(tstr), j=0; i<256-1 AND _error_str_[j]; i++, j++)  \
      tstr[i] = _error_str_[j];                                           \
    tstr[i] = 0;                                                          \
    debug_WriteFile (tstr);                                               \
  }
#else
#define DEBUG_ERROR(_error_str_)  { /* nop */ }
#endif

// Writes message to debug file
#ifdef _DEBUG
#define DEBUG_WRITE(_str_)    \
  {                           \
    debug_WriteFile (_str_);  \
  }
#else
#define DEBUG_WRITE(_str_)  { /* nop */ }
#endif

// Writes terminal error message to debug file and terminates program
#define TERMINAL_ERROR(_error_str_)                                       \
  {                                                                       \
    int i, j;                                                             \
    char tstr[256];                                                       \
    sprintf (tstr, "<TERMINAL_ERROR> line %d, %s: ", __LINE__, __FILE__); \
    for (i=(int)strlen(tstr), j=0; i<256-1 AND _error_str_[j]; i++, j++)  \
      tstr[i] = _error_str_[j];                                           \
    tstr[i] = 0;                                                          \
    debug_WriteFile (tstr);                                               \
    debug_AbortProgram (_error_str_);                                     \
  }                                                                       

//#define PERCENT(_percent_chance_)	(random_GetInt(1,100) <= _percent_chance_)

/*___________________
|
| Function Prototypes
|__________________*/

// CLIB.CPP
char *strins (char *str1, int pos, char *str2);
int   Valid_DOS_Filename (char *str);
int   File_Exists (char *filename);
void  File_Delete (char *filename);
void  Extract_Filename (char *pathname, char *filename);
void  Extract_Filename_Minus_Extension (char *pathname, char *filename);
void  Extract_Directoryname (char *pathname, char *dirname);

// DEBUG.CPP
void  debug_WriteFile (char *str);
void  debug_WriteConsole (char *str);
void  debug_AbortProgram (char *str);

// RANDOM.CPP
typedef enum {
  randomTYPE_FAST_1,
//	randomTYPE_MT,			// variant of Mersenne Twister
} RandomType;

typedef void *Random;

       Random     random_Init        (RandomType type);
       void       random_Init        ();
       void       random_Free        (Random r);
       void       random_SetSeed     (Random r, unsigned seed);
       unsigned   random_GetSeed     (Random r);
       RandomType random_GetType     (Random r);
			 unsigned   random_GetUnsigned (Random r);
			 unsigned   random_GetUnsigned ();
       int        random_GetInt      (Random r, int low, int high);
inline int        random_GetInt      (int low, int high); 
       float      random_GetFloat    (Random r);
       float      random_GetFloat    ();
       double     random_GetDouble   (Random r);
       double     random_GetDouble   ();
       float      random_GetPercent  (Random r);
inline float      random_GetPercent  ();

// MATH.CPP
float safe_acosf (float x);

// MEM_CHEK.CPP
void  Mem_Chek_Start ();
void  Mem_Chek_Stop ();
int   Mem_Chek_Blocks_Allocated (void);
int   Mem_Chek_Max_Blocks_Allocated (void);
void *Mem_Chek_malloc (size_t size, int line, char *file);
void *Mem_Chek_calloc (size_t nitems, size_t size, int line, char *file);
void *Mem_Chek_realloc (void *memblock, size_t size, int line, char *file);
void  Mem_Chek_free (void *p, int line, char *file);

#endif
