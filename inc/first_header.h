/*____________________________________________________________________
|
| File: first_header.h
|
| Description: Contains things EVERY source file needs and always should
|		be the first header file included.
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define _WIN32_WINNT 0x0501 //_WIN32_WINNT_WINXP 

// Enable older CRT functions (such as strcpy) without warnings from vc8 (vc 2005 .net)
#if _MSC_VER >= 1400
 #define _CRT_SECURE_NO_DEPRECATE				// shut up the vc8 compiler
 #define _CRT_NONSTDC_NO_DEPRECATE
#endif
