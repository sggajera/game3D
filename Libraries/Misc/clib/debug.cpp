/*____________________________________________________________________
|
| File: debug.cpp
|
| Description: Contains functions for run-time debugging support. All
|             functions are thread-safe.
|
| Functions:	debug_WriteFile
|             debug_WriteConsole
|             debug_AbortProgram
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
#include <win_support.h>

#include "clib.h"

/*___________________
|
| Constants
|__________________*/

#define FILENAME "DEBUG.TXT"

/*___________________
|
| Global variables
|__________________*/

FILE *debug_file = NULL;
static CRITICAL_SECTION debug_critsection;

/*____________________________________________________________________
|
| Function: debug_WriteFile
|
| Outputs: Writes a line of text to the debug file.  This routine adds a 
|   carriage return to the input string.
|___________________________________________________________________*/

void debug_WriteFile (char *str)
{
  static int first_time = TRUE;

  // Initialize this function?
  if (first_time) {
    // Init critical section
    InitializeCriticalSection (&debug_critsection);
    first_time = FALSE;
  }
  
  EnterCriticalSection (&debug_critsection);  

  if (str) {
    // Open a file - creating it if needed
    if (debug_file == NULL)
      debug_file = fopen (FILENAME, "wt");
    // If already opened before, open it for append
    else
      debug_file = fopen (FILENAME, "at");

    // Write string and close file
    if (debug_file) {
      fprintf (debug_file, "%s\n", str);
      fclose (debug_file);
    }
  }

  LeaveCriticalSection (&debug_critsection);
}

/*____________________________________________________________________
|
| Function: debug_WriteConsole
|
| Outputs: Writes a line of text to the Visual C++ output window.
|___________________________________________________________________*/

void debug_WriteConsole (char *str)
{
  if (str) {
    OutputDebugString (str);
    OutputDebugString ("\n");
  }
}

/*____________________________________________________________________
|
| Function: debug_AbortProgram
|
| Outputs: Writes string to debug file and exits program.
|___________________________________________________________________*/

void debug_AbortProgram (char *str)
{
  win_Abort_Program (str);
}
