/*____________________________________________________________________
|
| File: mt.cpp
|
| Description: Modified version of Mersenne Twister random number
|		generator.
|
| Functions:  MT_Init
|							MT_GenerateUnsigned
|
|	Description:
|   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
|   All rights reserved.                          
|
|		Modified by Mores Prachyabrued and Timothy E. Roden, Dec/2006.
|
|   Redistribution and use in source and binary forms, with or without
|   modification, are permitted provided that the following conditions
|   are met:
|
|     1. Redistributions of source code must retain the above copyright
|        notice, this list of conditions and the following disclaimer.
|
|     2. Redistributions in binary form must reproduce the above copyright
|        notice, this list of conditions and the following disclaimer in the
|        documentation and/or other materials provided with the distribution.
|
|     3. The names of its contributors may not be used to endorse or promote 
|        products derived from this software without specific prior written 
|        permission.
|
|   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
|   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
|   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
|   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
|   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
|   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
|   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
|   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
|   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
|   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
|   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include <windows.h>
#include <winbase.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>

#include <defines.h>

#include "mt.h"

/*___________________
|
| Defines
|__________________*/

// Period parameters
#define N		624
#define M		397
#define MATRIX_A		((unsigned)0x9908b0df)  // constant vector a
#define UPPER_MASK	((unsigned)0x80000000)	// most significant w-r bits
#define LOWER_MASK	((unsigned)0x7fffffff)  // least significant r bits

/*___________________
|
| Global variables
|__________________*/

static unsigned long mt[N]; // the array for the state vector 
static int mti=N+1;					// mti==N+1 means mt[N] is not initialized

/*____________________________________________________________________
|
| Function: MT_Init 
|
| Output: Initializes mt[N] with a seed.
|___________________________________________________________________*/

void MT_Init (unsigned seed)
{
  mt[0]= seed & (unsigned)0xFFFFFFFF;
  for (mti=1; mti<N; mti++) {
    mt[mti] = ((unsigned)1812433253 * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array mt[].                        */
    /* 2002/01/09 modified by Makoto Matsumoto             */
  }
}

/*____________________________________________________________________
|
| Function: MT_GenerateUnsigned 
|
| Output: Generates a random number on [0,0xffffffff]-interval.
|___________________________________________________________________*/

unsigned MT_GenerateUnsigned ()
{
  unsigned y;
  static unsigned mag01 [2] = {0, MATRIX_A};
  /* mag01[x] = x * MATRIX_A  for x=0,1 */

	// generate one word at a time
	if (mti >= N) {
		if (mti == N+1)			// if MT_Init() has not been called, a default seed is used
			MT_Init (5489U);
		mti = 0;
	}

	if (mti < N-M) {
		y = (mt[mti] & UPPER_MASK)|(mt[mti+1] & LOWER_MASK);
		mt[mti] = mt[mti+M] ^ (y >> 1) ^ mag01[y & 1];
	} 
	else if (mti < N-1) {
		y = (mt[mti] & UPPER_MASK)|(mt[mti+1] & LOWER_MASK);
		mt[mti] = mt[mti+(M-N)] ^ (y >> 1) ^ mag01[y & 1];
	} 
	else { // mti == N-1
		y = (mt[N-1] & UPPER_MASK)|(mt[0] & LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 1];
	}
	y = mt[mti++];

	// Tempering
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680U;
	y ^= (y << 15) & 0xefc60000U;
	y ^= (y >> 18);

	return (y);
}
