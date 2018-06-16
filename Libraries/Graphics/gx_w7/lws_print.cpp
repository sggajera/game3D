/*____________________________________________________________________
|
| File: lws_print.cpp
|
| Description: A function to print out in ASCII form the contents of an
|   LWS file.  LWS files are already in ASCII but this function cleans
|   up things a bit and only outputs important information.  Also, measures
|   are in feet and angles are in degrees.
|
| Functions: gx3d_PrintLWSFile
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include "dp.h"
#include "lws.h"

/*____________________________________________________________________
|
| Function: gx3d_PrintLWSFile
|
| Output: Translates a LWS file and creates an simplified ASCII version.
|___________________________________________________________________*/

void gx3d_PrintLWSFile (char *filename, char *outputfilename)
{
  lws_ObjectLayer *olayer = 0;

  DEBUG_ASSERT (filename)
  DEBUG_ASSERT (outputfilename)

  // Read the file
  olayer = lws_ReadFile (filename, 0, true);
  // Print it
  if (olayer) {
    lws_WriteTextFile (outputfilename, olayer);
    // Free the data
    lws_FreeObjectLayer (olayer);
  }
}
