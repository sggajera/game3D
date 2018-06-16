/*____________________________________________________________________
|
| File: math.cpp
|
| Description: Contains general purpose C math functions.  All functions
|   are thread-safe.
|
| Functions:	safe_acosf
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <stdio.h>
#include <math.h>

#include <defines.h>

#include "clib.h"

/*____________________________________________________________________
|
| Function: safe_acosf
|
| Inputs: Called from various routines.
| Outputs: Same as acos(x) but if x is out of range, it gets clamped to
|   the nearest valid value.  The value returned is in range 0..pi, the
|   same as the standardd C acos() function.
|___________________________________________________________________*/

float safe_acosf (float x)
{
  if (x <= -1.0f)
    return (PI);
  else if (x >= 1.0f)
    return (0);
  else
    return (acosf(x));
}
