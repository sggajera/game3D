/*____________________________________________________________________
|
| File: duplexbuffer.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _DUPLEXBUFFER_H_
#define _DUPLEXBUFFER_H_

/*___________________
|
| Type definitions
|__________________*/

typedef void *DuplexBuffer;

/*___________________
|
| Functions
|__________________*/

DuplexBuffer DuplexBuffer_Init  (unsigned size);
void				 DuplexBuffer_Free  (DuplexBuffer db);
void				 DuplexBuffer_Write (DuplexBuffer db, byte *buff, unsigned buff_size, unsigned *size_written);
void				 DuplexBuffer_Read  (DuplexBuffer db, byte *buff, unsigned buff_size, unsigned *size_read);

#endif
