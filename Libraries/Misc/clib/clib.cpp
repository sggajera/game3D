/*____________________________________________________________________
|
| File: clib.cpp
|
| Description: Contains general purpose C functions.  All functions are
|   thread-safe.
|
| Functions:	strins
|             Valid_DOS_Filename
|             File_Exists
|             File_Delete
|             Extract_Filename
|             Extract_Filename_Minus_Extension
|             Extract_Directoryname
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
#include <ctype.h>
#include <string.h>

#include <defines.h>

#include "clib.h"

/*____________________________________________________________________
|
| Function: strins
|
| Inputs: Called from various routines.
| Outputs: Inserts into str1, str2 at position pos in str1.
|___________________________________________________________________*/

char *strins (char *str1, int pos, char *str2)
{
  int i, len, len2, src, des, move;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (str1)
  DEBUG_ASSERT (str2)
  DEBUG_ASSERT (pos >= 0)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Get lengths of each string
  for (len=0; str1[len]; len++);
  for (len2=0; str2[len2]; len2++);

  // Make room in str1
  move = len - pos;
  if (move >= 0)
    for (i=0, src=len, des=len+len2; i<=move; i++)
      str1[des--] = str1[src--];

  // Insert str2
  for (i=0; str2[i]; i++)
    str1[pos++] = str2[i];
  
  return (str1);
}

/*____________________________________________________________________
|
| Function: Valid_DOS_Filename
|
| Inputs: Called from various routines.
| Outputs: Returns true if str is a valid DOS filename.
|___________________________________________________________________*/

int Valid_DOS_Filename (char *str)
{
  int i, j, len, ext_pos;
  int valid = FALSE;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (str)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Get length of this string
  for (i=0, len=0; str[i]; i++, len++);

  // str must contain <= 12 characters
  if (len <= 12) {
    // First character must be a letter
    if (isalpha (str[0])) {
      // Rest must be alphanumeric or '.' or '-' or '_'
      for (i=1; i<len; i++)
        if ((NOT isalpha (str[i])) AND
    	      (NOT isdigit (str[i])) AND
    	      (str[i] != '.') AND
    	      (str[i] != '-') AND
    	      (str[i] != '_'))
          break;
      if (i == len) {
        // Must contain 1 and only 1 '.' character
        for (i=0, j=0; str[i]; i++)
          if (str[i] == '.') {
            j++;
            ext_pos = i;
          }
        if (j == 1) {
          // No more than 3 characters after '.' is allowed
          ext_pos++;
          if ((len - ext_pos) <= 3)
            valid = TRUE;
        }
      }
    }
  }

  return (valid);
}

/*____________________________________________________________________
|
| Function: File_Exists
|
| Inputs: Called from various routines.
| Outputs: Returns true if file exists, else false.
|___________________________________________________________________*/

int File_Exists (char *filename)
{
  FILE *fp;
  int exists = FALSE;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (filename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (filename)
    if (filename[0]) {
      fp = fopen (filename, "rb");
      if (fp) {
        fclose (fp);
        exists = TRUE;
      }
    }

  return (exists);
}

/*____________________________________________________________________
|
| Function: File_Delete
|
| Inputs: Called from various routines.
| Outputs: Delete the file if it exists.
|___________________________________________________________________*/

/***** Need a better routine that doesn't use system() which brings up a DOS box! (ARGH!) *****/

void File_Delete (char *filename)
{
  char *command;
  char *del = "del ";

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (filename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  if (File_Exists (filename)) {
    command = (char *) calloc (strlen(del) + strlen(filename) + 1, sizeof(char));
    if (command) {
      strcpy (command, del);
      strcat (command, filename);
      system (command);
      free (command);
    }
  }
}

/*____________________________________________________________________
|
| Function: Extract_Filename
|
| Inputs: Called from various routines.
| Outputs: Extracts filename from a pathname.  Puts filename into callers
|   buffer.  filename should point to a buffer as least as large as the
|   buffer pointed to by pathname.
|___________________________________________________________________*/

void Extract_Filename (char *pathname, char *filename)
{
 char *p;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (pathname)
  DEBUG_ASSERT (filename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Get last occurrence of '/' in pathname
  p = strrchr (pathname, '\\');
  if (p == 0)
    p = pathname;
  else
    p++;
  strcpy (filename, p);
}

/*____________________________________________________________________
|
| Function: Extract_Filename_Minus_Extension
|
| Inputs: Called from various routines.
| Outputs: Extracts filename from a pathname.  Puts filename into callers
|   buffer.  filename should point to a buffer as least as large as the
|   buffer pointed to by pathname. Removes the file extension from the
|   filename found.
|___________________________________________________________________*/

void Extract_Filename_Minus_Extension (char *pathname, char *filename)
{
  char *p;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (pathname)
  DEBUG_ASSERT (filename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Get filename portion of pathname
  Extract_Filename (pathname, filename);
  // Get first occurrence of '.' in filename
  p = strchr (filename, '.');
  if (p)
    *p = 0;
}

/*____________________________________________________________________
|
| Function: Extract_Directoryname
|
| Inputs: Called from various routines.
| Outputs: Extracts directory name from a pathname.  Puts directory name
|   into callers buffer. dirname should point to a buffer as least as 
|   large as the buffer pointed to by pathname.
|___________________________________________________________________*/

void Extract_Directoryname (char *pathname, char *dirname)
{
  int i;
  char *p;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (pathname)
  DEBUG_ASSERT (dirname)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Assume dirname may be an empty string
  dirname[0] = 0;

  // Get last occurrence of '/' in pathname
  p = strrchr (pathname, '\\');
  if (p) {
    for (i=0; &pathname[i] <= p; i++) {
      dirname[i] = pathname[i];
      dirname[i+1] = 0;
    }
  }
}
