/*____________________________________________________________________
|
| File: duplexbuffer.cpp
|
| Description: A thread safe array (duplex buffer).
|
| Functions:	DuplexBuffer_Init
|							DuplexBuffer_Free
|							DuplexBuffer_Write
|							DuplexBuffer_Read
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
#include <string.h>
#include <conio.h>

#include <defines.h>
#include <clib.h>

#include "duplexbuffer.h"

/*___________________
|
| Type definitions
|__________________*/

struct DuplexBufferData {
  byte     *buff;
  unsigned  size;
  unsigned  count;        // amount of data currently in buffer
  unsigned  read, write;	// indeces into buffer
	CRITICAL_SECTION cs;
};

/*____________________________________________________________________
|
| Function: duplexbuffer_Init
|
| Output: Creates a duplex buffer of size bytes and returns a handle to
|		it or 0 on any error.
|___________________________________________________________________*/

DuplexBuffer DuplexBuffer_Init (unsigned size)
{
	DuplexBufferData *dbdata = 0;

	DEBUG_ASSERT (size > 1)

	// Allocate memory for struct
	dbdata = (DuplexBufferData *) calloc (1, sizeof(DuplexBufferData));
	if (dbdata) {
		// Allocate memory for data buffer
		dbdata->buff = (byte *) malloc (size);
		// On error, release other memory allocated
		if (dbdata == 0) {
			free (dbdata);
			dbdata = 0;
		}
		else {
			dbdata->size  = size;
			dbdata->count = 0;
			dbdata->read  = 0;
			dbdata->write = 0;
			InitializeCriticalSection (&(dbdata->cs));
		}
	}

	return ((DuplexBuffer)dbdata);
}

/*____________________________________________________________________
|
| Function: duplexbuffer_Free
|
| Output: Frees a duplex buffer.
|___________________________________________________________________*/

void DuplexBuffer_Free (DuplexBuffer db)
{
	DuplexBufferData *dbdata = (DuplexBufferData *)db;

	DEBUG_ASSERT (dbdata)
	DEBUG_ASSERT (dbdata->buff)

	if (dbdata) {
		DeleteCriticalSection (&(dbdata->cs));
		free (dbdata->buff);
		free (dbdata);
	}
}

/*____________________________________________________________________
|
| Function: duplexbuffer_Write
|
| Output: Writes data into a duplex buffer.
|___________________________________________________________________*/

void DuplexBuffer_Write (DuplexBuffer db, byte *buff, unsigned buff_size, unsigned *size_written)
{
  unsigned space, n1, n2;
	DuplexBufferData *dbdata = (DuplexBufferData *)db;

  // Lock the buffer
  EnterCriticalSection (&(dbdata->cs));
  // Space available to write new data?
  space = dbdata->size - dbdata->count;
  if (space) {
    // Don't write more data than space availablee
    if (buff_size > space)
      buff_size = space;
    // One write operation?
    if (dbdata->write + buff_size < (dbdata->size-1)) {
      memcpy ((void *)&dbdata->buff[dbdata->write], (void *)buff, buff_size);
      dbdata->write = (dbdata->write + buff_size) % dbdata->size;
    }
    // Break into two write operations because this is a circular buffer
    else {
      n1 = dbdata->size-1 - dbdata->write + 1;
      n2 = buff_size - n1;
      memcpy ((void *)&dbdata->buff[dbdata->write], (void *)buff, n1);
      memcpy ((void *)&dbdata->buff[0], (void *)&buff[n1], n2);
      dbdata->write = n2;
    }
    dbdata->count += buff_size;
		*size_written = buff_size;
  }
	else
		*size_written = 0;
  LeaveCriticalSection (&(dbdata->cs));
}

/*____________________________________________________________________
|
| Function: duplexbuffer_Read
|
| Output: Reads data from a duplex buffer.
|___________________________________________________________________*/

void DuplexBuffer_Read (DuplexBuffer db, byte *buff, unsigned buff_size, unsigned *size_read)
{
  unsigned n1, n2;
	DuplexBufferData *dbdata = (DuplexBufferData *)db;

  // Lock the buffer
  EnterCriticalSection (&(dbdata->cs));
  // Any data to read from duplexbuffer?
  if (dbdata->count) {
    // Compute amount to read
    if (dbdata->count < buff_size)
      buff_size = dbdata->count;
    // One read operation?
    if (dbdata->read + buff_size < (dbdata->size-1)) {
      memcpy ((void *)buff, (void *)&dbdata->buff[dbdata->read], buff_size);
      dbdata->read = (dbdata->read + buff_size) % dbdata->size;
    }
    // Break into two read operations because this is a circular buffer
    else {
      n1 = dbdata->size-1 - dbdata->read + 1;
      n2 = buff_size - n1;
      memcpy ((void *)buff, (void *)&dbdata->buff[dbdata->read], n1);
      memcpy ((void *)&buff[n1], (void *)&dbdata->buff[0], n2);
      dbdata->read = n2;
    }
    dbdata->count -= buff_size;
    *size_read = buff_size;
  }
	else
		*size_read = 0;
  LeaveCriticalSection (&(dbdata->cs));
}
