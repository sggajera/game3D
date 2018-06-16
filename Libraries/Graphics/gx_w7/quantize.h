/*____________________________________________________________________
|
| File: quantize.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*___________________
|
| Functions prototypes
|__________________*/

// Encode a float in the range [0,1] into an n-bit integer
unsigned CompressUnitFloatRL (float unit_float, unsigned nbits);
// Decode a float compressed with CompressUnitFloatRL()
float    DecompressUnitFloatRL (unsigned quantized, unsigned nbits);
// Encode a float in the range [min,max] into an n-bit integer
unsigned CompressFloatRL (float value, float min, float max, unsigned nbits);
// Decode a float compressed with CompressFloatRL()
float    DecompressFloatRL (unsigned quantized, float min, float max, unsigned nbits);
// Encode a float in the range [-1,1] into a 16-bit unsigned integer
inline unsigned short CompressQuaternionValue (float qval);
// Decode a float compressed with CompressQuaternionValue()
inline float DecompressQuaternionValue (unsigned short qval);
