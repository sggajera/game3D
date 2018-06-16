/*____________________________________________________________________
|
| File: defines.h
|
| Description: Contains global defines, data types to be included by
|	all source files.
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

// Enable older CRT functions (such as strcpy) without warnings
//#if _MSC_VER >= 1400
// #define _CRT_SECURE_NO_DEPRECATE				// shut up the vs2005 compiler
// #define _CRT_NONSTDC_NO_DEPRECATE
//#endif

// Data types
typedef unsigned char    byte;
typedef unsigned short   word;
typedef unsigned long    dword;
typedef unsigned __int64 qword, *pqword;
typedef int int_plusone;  // designed to hold an integer value (with 1 added to it), so that a valid value can never be zero

// For C code, create some special defines
#ifndef __cplusplus
typedef int						   bool;
#define inline           __inline
// For C++ code, creates some special defines
#else
#define NULL  0
#endif

// If compiling a debug build, no inlines!
#ifdef _DEBUG
#define inline /* */
#endif

// If compiling a debug build, redirect all heap functions
#ifdef _DEBUG
#define malloc(_size_)          Mem_Chek_malloc(_size_,__LINE__,__FILE__)
#define calloc(_nitems_,_size_) Mem_Chek_calloc(_nitems_,_size_,__LINE__,__FILE__)
#define realloc(_ptr_,_size_)   Mem_Chek_realloc(_ptr_,_size_,__LINE__,__FILE__)
#define free(_ptr_)             Mem_Chek_free(_ptr_,__LINE__,__FILE__)
#endif

// Defines
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define AND &&
#define OR  ||
#define NOT !
#define MOD %

#define PI                  ((float) 3.141592654)
#define RADIANS_TO_DEGREES  ((float) ((float)180/PI))
#define DEGREES_TO_RADIANS  ((float) (PI/(float)180))
#define METERS_TO_FEET      ((float) 3.280839895)
#define FEET_TO_METERS      ((float) 0.3048)
#define FEET_PER_MILE       5280

/*____________________________________________________________________
|
| Bit masks
|___________________________________________________________________*/

#define BIT_0	  0x1
#define BIT_1   0x2
#define BIT_2   0x4
#define BIT_3   0x8
#define BIT_4   0x10
#define BIT_5   0x20
#define BIT_6   0x40
#define BIT_7   0x80
#define BIT_8   0x100
#define BIT_9   0x200
#define BIT_10  0x400
#define BIT_11  0x800
#define BIT_12  0x1000
#define BIT_13  0x2000
#define BIT_14  0x4000
#define BIT_15  0x8000
#define BIT_16  0x10000
#define BIT_17  0x20000
#define BIT_18  0x40000
#define BIT_19  0x80000
#define BIT_20  0x100000
#define BIT_21  0x200000
#define BIT_22  0x400000
#define BIT_23  0x800000
#define BIT_24  0x1000000
#define BIT_25  0x2000000
#define BIT_26  0x4000000
#define BIT_27  0x8000000
#define BIT_28  0x10000000
#define BIT_29  0x20000000
#define BIT_30  0x40000000
#define BIT_31  0x80000000

/*____________________________________________________________________
|
| Macros
|___________________________________________________________________*/

#define sinf(_expr_) ((float)sin((double)(_expr_)))
#define cosf(_expr_) ((float)cos((double)(_expr_)))
#define tanf(_expr_) ((float)tan((double)(_expr_)))
#define asinf(_expr_) ((float)asin((double)(_expr_)))
#define acosf(_expr_) ((float)acos((double)(_expr_)))
#define atanf(_expr_) ((float)atan((double)(_expr_)))
#define sqrtf(_expr_) ((float)sqrt((double)(_expr_)))
//#define fabsf(_expr_) ((float)fabs((double)(_expr_)))
#define absf(_expr_) ((float)fabs((double)(_expr_)))
#define logf(_expr_)  ((float)log((double)(_expr_)))
#define log10f(_expr_) ((float)log10((double)(_expr_)))
#define fabsf(_expr_) ((_expr_) >=0 ? (_expr_) : -(_expr_))

/*____________________________________________________________________
|
| GBA-specific
|___________________________________________________________________*/

#ifndef GBA
#define ALIGN(_n_)  /* nop */
#define PACKED			/* nop */
#endif
