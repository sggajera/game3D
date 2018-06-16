/*____________________________________________________________________
|
| File: quantize.cpp
|
| Description: Functions to compress/decompress floats.
|
| Functions:  CompressUnitFloatRL
|             DecompressUnitFloatRL
|             CompressFloatRL
|             DecompressFloatRL
|             CompressQuaternionValue
|             DecompressQuaternionValue
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
#include "quantize.h"

/*____________________________________________________________________
|
| Function: CompressUnitFloatRL
|
| Output: Encode a float value in the range [0,1] into an n-bit integer,
|   according to Jonathan Blow's RL method.  The value returned is a 
|   32-bit unsigned int but only the least-significant n bits are used
|   as specified by the nbits parameter.  So, for example if nbits is
|   16 the returned value can be safely casted to an unsigned short.
|
| Reference: Game Engine Architecture, pg. 548.
|___________________________________________________________________*/

unsigned CompressUnitFloatRL (float unit_float, unsigned nbits)
{
  // Determine the number of intervals based on the number of output bits we want
  unsigned nintervals = (unsigned)1 << nbits;
  // Scale the input value from the range [0,1] into the range
  //  [0,nintervals-1].  Subtract one interval since we want
  //  the largest output value to fit into nbits bits.
  float scaled = unit_float * (float)(nintervals - (unsigned)1);
  // Round to nearest interval center by adding 0.5f and truncating
  //  to the next lowest interval index (by casting to unsigned)
  unsigned rounded = (unsigned)(scaled + 0.5f);
  // Guard against invalid input values
  if (rounded > nintervals - (unsigned)1)
    rounded = nintervals - (unsigned)1;
  return (rounded);
}

/*____________________________________________________________________
|
| Function: DecompressUnitFloatRL
|
| Output: Decode a float compressed with CompressUnitFloatRL()
|
| Reference: Game Engine Architecture, pg. 548.
|___________________________________________________________________*/

float DecompressUnitFloatRL (unsigned quantized, unsigned nbits)
{
  // Determine the number of intervals based on the number of bits used to encode
  unsigned nintervals = (unsigned)1 << nbits;
  // Decode by converting the unsigned to a float and scaling by the interval size
  float interval_size = 1.0f / (float)(nintervals - (unsigned)1);
  float approx_unit_float = (float)quantized * interval_size;
  return (approx_unit_float);
}

/*____________________________________________________________________
|
| Function: CompressFloatRL
|
| Output: Encode a float value in the range [min,max] into an n-bit integer
|
| Reference: Game Engine Architecture, pg. 549.
|___________________________________________________________________*/

unsigned CompressFloatRL (float value, float min, float max, unsigned nbits)
{
  DEBUG_ASSERT (value >= min AND value <= max)
  DEBUG_ASSERT (min < max)
  DEBUG_ASSERT (nbits <= 16)

  float unit_float = (value - min) / (max - min);
  unsigned quantized = CompressUnitFloatRL (unit_float, nbits);
  return (quantized);
}

/*____________________________________________________________________
|
| Function: DecompressFloatRL
|
| Output: Decode a float compressed with CompressFloatRL()
|
| Reference: Game Engine Architecture, pg. 549.
|___________________________________________________________________*/

float DecompressFloatRL (unsigned quantized, float min, float max, unsigned nbits)
{
  DEBUG_ASSERT (min < max)
  DEBUG_ASSERT (nbits <= 16)

  float unit_float = DecompressUnitFloatRL (quantized, nbits);
  float value = min + (unit_float * (max - min));
  return (value);
}

/*____________________________________________________________________
|
| Function: CompressQuaternionValue
|
| Output: Encode a float in the range [-1,1] into a 16-bit unsigned integer
|
| Reference: Game Engine Architecture, pg. 549.
|___________________________________________________________________*/

inline unsigned short CompressQuaternionValue (float qval)
{
  return ((unsigned short)CompressFloatRL (qval, -1.0f, 1.0f, 16));
}

/*____________________________________________________________________
|
| Function: DecompressQuaternionValue
|
| Output: Decode a float compressed with CompressQuaternionValue()
|
| Reference: Game Engine Architecture, pg. 549.
|___________________________________________________________________*/

inline float DecompressQuaternionValue (unsigned short qval)
{
  return (DecompressFloatRL ((unsigned)qval, -1.0f, 1.0f, 16));
}
