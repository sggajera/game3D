/*____________________________________________________________________
|
| File: snd8.cpp
|
| Description: Contains functions for sound support using DirectSound 8.
|
| Functions:	snd_Init
|             snd_Free	
|							snd_Optimize
|             snd_LoadSound
|							snd_LoadSound
|              Load_Streaming_Sound
|               Fill_Buffer_With_Silence
|              Load_Static_Sound
|             snd_FreeSound
|              Free_Effects
|             snd_PlaySound
|              Streaming_Sound_Thread
|             snd_StopSound
|             snd_PauseSound
|             snd_UnpauseSound
|             snd_SetSoundVolume
|             snd_SetSoundPan
|							snd_SetSoundFrequency
|							snd_ResetSoundFrequency
|							snd_GetSoundFrequency
|             snd_IsPlaying
|
|							snd_EnableEffects
|							snd_SetEnvironment
|             snd_SetEffectProperties
|             snd_GetEffectProperties
|
|             snd_SetSoundMode
|             snd_SetSoundPosition
|             snd_SetSoundMinDistance
|             snd_SetSoundMaxDistance
|             snd_SetSoundConeOrientation
|             snd_SetSoundConeAngles
|             snd_SetSoundConeOutsideVolume
|             snd_SetSoundVelocity
|
|             snd_SetListenerDistanceFactor
|							snd_SetListenerDistanceFactorToFeet
|             snd_SetListenerRolloff
|             snd_SetListenerVelocity
|             snd_SetListenerPosition
|             snd_SetListenerOrientation
|             snd_SetListenerDopplerFactor
|
|             snd_Commit3DDeferredSettings
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
#include <process.h>   
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#include <malloc.h>
#include <process.h>
#include <mmsystem.h>                    

#include <dsound.h>

#include <defines.h>
#include <events.h>
#include <clib.h>
#include <win_support.h>

#include "wave.h"

#include "snd8.h"

/*___________________
|
| Constants
|__________________*/

#define NUM_CHUNKS              4 // # chunks to break up streaming play buffer

#define NUM_STREAMING_EVENTS	  3
#define SOUND_EVENT_PAUSE		    0
#define SOUND_EVENT_UNPAUSE	    1
#define SOUND_EVENT_QUIT		    2
#define SOUND_EVENT_STOPPED     (NUM_STREAMING_EVENTS+NUM_CHUNKS)

#define STREAMING_SOUND_SECONDS	2 // # seconds worth of streaming sound data to keep in RAM

/*___________________
|
| Type definitions
|__________________*/

typedef unsigned (WINAPI *PBEGINTHREADEX_THREADFUNC)(LPVOID lpThreadParameter);
typedef unsigned *PBEGINTHREADEX_THREADID;

// This is a system struct, reprinted here for info purposes
//typedef struct tWAVEFORMATEX
//{
//    WORD    wFormatTag;        /* format type */
//    WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
//    DWORD   nSamplesPerSec;    /* sample rate */
//    DWORD   nAvgBytesPerSec;   /* for buffer estimation */
//    WORD    nBlockAlign;       /* block size of data */
//    WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
//    WORD    cbSize;            /* The count in bytes of the size of
//                                    extra information (after cbSize) */
//} WAVEFORMATEX;

typedef struct {
	LPDIRECTSOUNDBUFFER8   buffer;
  LPDIRECTSOUND3DBUFFER8 buffer3d;  
  DSBUFFERDESC	         dsbdesc;						// sound buffer description
	char									*filename;					// ASCIIZ string
	unsigned							 controls_enabled;	// mask
  byte									 global_focus;      // boolean
	// Applies to streaming buffers only	
	byte									 repeat;			      // boolean				
	WAVEFORMATEX				  *pwfx;
	HMMIO									 hmmio;
	MMCKINFO							 mmckinfo, mmckinfoParent;
	HANDLE								 streamEvent [NUM_STREAMING_EVENTS+NUM_CHUNKS+1];
  int										 num_events;
	DSBPOSITIONNOTIFY			 streamPos [NUM_CHUNKS+1];
  LPDIRECTSOUNDNOTIFY8	 dsnotify;
  HANDLE dsstream_thread;
	// Effect interfaces
  LPDIRECTSOUNDFXCHORUS8			chorus;
  LPDIRECTSOUNDFXCOMPRESSOR8	compressor;		
  LPDIRECTSOUNDFXDISTORTION8	distortion;
  LPDIRECTSOUNDFXECHO8				echo;
  LPDIRECTSOUNDFXFLANGER8			flange;
  LPDIRECTSOUNDFXGARGLE8			gargle;
  LPDIRECTSOUNDFXPARAMEQ8			param_eq;
  LPDIRECTSOUNDFXWAVESREVERB8	waves_reverb;
	LPDIRECTSOUNDFXI3DL2REVERB8 env_reverb;
} SoundData;

/*___________________
|
| Function prototypes
|__________________*/

static int   Load_Streaming_Sound (SoundData *snd);
static void  Fill_Buffer_With_Silence (LPDIRECTSOUNDBUFFER8 soundbuffer);
static bool Load_Static_Sound (
  SoundData *snd,
  unsigned  *num_samples,           // optional
  unsigned  *bits_per_sample,       // optional
  unsigned  *duration_milliseconds, // optional
  byte     **data );                // optional
static void  Free_Effects (SoundData *snd);
static int   Create_Streaming_Sound (SoundData *snd);
static int   Create_Static_Sound (SoundData *snd);
DWORD WINAPI Streaming_Sound_Thread (LPVOID pParam);

/*___________________
|
| Global variables
|__________________*/

static LPDIRECTSOUND8           dsound8           = 0;  // pointer to DirectSound8 interface
static LPDIRECTSOUNDBUFFER      dsprimarybuffer   = 0;  // pointer to DirectSound primary buffer interface
static LPDIRECTSOUND3DLISTENER8 dsound3dlistener8 = 0;  // pointer to DirectSound3DListener8 interface
static WAVEFORMATEX             dsformat;               // format of primary buffer

static DWORD dwApply [2] = { DS3D_IMMEDIATE, DS3D_DEFERRED };

/*____________________________________________________________________
|
| Function: snd_Init
|
| Outputs: Initializes sound library interface .  Returns true if 
|       initialized, else false on any error. 
|
|       If rate = 0, the default sound card format will be used.
|
| Description: rate      = 0 (sound card default) 8 (8000 Hz), 11 (11025 Hz), 22 (22050 Hz), 44 (44100 Hz)
|              bits      = 8, 16
|              channels  = 1, 2
|              enable_3d = boolean
|___________________________________________________________________*/

int snd_Init (int rate, int bits, int channels, int enable_3d, int mute_background_apps)
{
  DWORD        dwLevel;
	DSBUFFERDESC dsbdesc;                                
  WAVEFORMATEX wfx;
	int initialized = FALSE;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

	// Initialize the COM system
	CoInitialize (NULL);

  dsound8          = 0;
  dsprimarybuffer  = 0;
  
/*____________________________________________________________________
|
| Init DirectSound
|___________________________________________________________________*/

  // Error checking on input parameters
	if (((rate == 0) OR (rate == 8) OR (rate == 11) OR (rate == 22) OR (rate == 44)) AND
			((bits == 8)  OR (bits == 16)) AND
      ((channels == 1) OR (channels == 2))) {

		// Get a ptr to DirectSound8 interface
		if (DirectSoundCreate8 (NULL, &dsound8, NULL) != DS_OK) 
			debug_WriteFile ("snd_Init(): Error getting DirectSound8 interface");
		else {
      // Set cooperative level
			if (mute_background_apps)
				dwLevel = DSSCL_EXCLUSIVE;
			else
				dwLevel = DSSCL_PRIORITY;
			if (dsound8->SetCooperativeLevel (win_Get_Window_Handle(), dwLevel) != DS_OK) 
				debug_WriteFile ("snd_Init(): Error setting cooperative level");
			else {
        memset (&dsbdesc, 0, sizeof(DSBUFFERDESC));
				dsbdesc.dwSize  = sizeof(DSBUFFERDESC);
				dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
        if (enable_3d)
          dsbdesc.dwFlags |= DSBCAPS_CTRL3D;
				dsbdesc.dwBufferBytes = 0;		  // must be 0 for primary buffer
				dsbdesc.lpwfxFormat   = NULL;		// must be NULL for primary buffer
        // Get an interface to the primary buffer
        if (dsound8->CreateSoundBuffer (&dsbdesc, &dsprimarybuffer, NULL) != DS_OK) 
					debug_WriteFile ("snd_Init(): Error creating the primary buffer");
				else {
          // Change format of primary buffer?
          if (rate != 0) {
            memset (&wfx, 0, sizeof(WAVEFORMATEX));
					  wfx.wFormatTag = WAVE_FORMAT_PCM;
					  wfx.nChannels  = channels;
					  switch (rate) {
              case 8:  rate = 8000;
                       break;
						  case 11: rate = 11025;
										   break;
						  case 22: rate = 22050;
										   break;
						  case 44: rate = 44100;
										   break;
					  }
					  wfx.nSamplesPerSec  = rate;
					  wfx.wBitsPerSample  = bits;
					  wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
					  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
					  // Attempt to set primary buffer to desired format
					  dsprimarybuffer->SetFormat (&wfx);
          }
          // Get format of primary buffer
          if (dsprimarybuffer->GetFormat (&wfx, sizeof(WAVEFORMATEX), NULL) != DS_OK) 
						debug_WriteFile ("snd_Init(): Error getting format of primary buffer");
					else {
            // Copy this to a global variable
            dsformat = wfx;
            // Keep mixer running continuously for better performance
	  				dsprimarybuffer->Play (0, 0, DSBPLAY_LOOPING);

            // Get 3d listener interface?
            if (enable_3d) {
              if (dsprimarybuffer->QueryInterface (IID_IDirectSound3DListener8, (void **)&dsound3dlistener8) != S_OK) 
								debug_WriteFile ("snd_Init(): Error getting directsound3dlistener interface");
							else
                initialized =  TRUE;  
            }                                     
            else
              initialized = TRUE;
          }
				}
      }
    }
  }
  
/*____________________________________________________________________
|
| On any error, release all resources
|___________________________________________________________________*/

  if (NOT initialized)
    snd_Free ();

	return (initialized);
}

/*___________________________________________________________________
|
|	Function: snd_Free
| 
|	Output: Frees sound library interface.  Individual sounds should have
|					previously been freed by caller.
|___________________________________________________________________*/

void snd_Free (void)
{
  // Free DirectSound resources
  if (dsound8) {
    if (dsound3dlistener8) {
      dsound3dlistener8->Release ();
      dsound3dlistener8 = 0;
    }
		if (dsprimarybuffer) {
			dsprimarybuffer->Stop ();
			dsprimarybuffer = 0;
		}
		dsound8->Release ();
		dsound8 = 0;
	}

  CoUninitialize ();
}

/*___________________________________________________________________
|
|	Function: snd_Optimize
| 
|	Output: Moves the unused portions of on-board sound memory, if any,
|		to a contiguous block so the largest portion of free memory will
|		be available.  Use this after loading and free a lot of sounds.
|___________________________________________________________________*/

void snd_Optimize (void)
{
	if (dsound8)
		dsound8->Compact ();
}

/*___________________________________________________________________
|
|	Function: snd_LoadSound
| 
|	Input: Initializes and loads a sound from a file.
| Output:	Returns a pointer to the sound.
|___________________________________________________________________*/

Sound snd_LoadSound (
  char		*filename,      // wave file
	unsigned controls,			// mask
  int			 global_focus )	// boolean (if true, sound will keep playing while app in background)
{
	int sound_loaded;
	SoundData *snd = NULL;

	// Allocate memory for Sound struct
	snd = (SoundData *) calloc (1, sizeof(SoundData));
	if (snd) {
		// Allocate memory for filename
		snd->filename = (char *) calloc (strlen(filename)+1, sizeof(char));
		// On error, release any memory allocated
		if (snd->filename == NULL) {
			free (snd);
			snd = NULL;
		}
		else {
			// Copy filename into struct
			strcpy (snd->filename, filename);
			// Set other variables in struct
			snd->controls_enabled = controls;
			// Some controls can't be combined, make choices here
			if (snd->controls_enabled & snd_CONTROL_3D)
				snd->controls_enabled &= ~snd_CONTROL_PAN;
			if (snd->controls_enabled & snd_CONTROL_EFFECTS)
				snd->controls_enabled &= ~snd_CONTROL_FREQUENCY;
      snd->global_focus = global_focus;
			// Load sound from a file
			if (snd->controls_enabled & snd_CONTROL_STREAMING)
				sound_loaded = Load_Streaming_Sound (snd);
			else
				sound_loaded = Load_Static_Sound (snd,0,0,0,0);
			if (NOT sound_loaded) {
				// On error, release any memory allocated
				free (snd->filename);
				free (snd);
				snd = NULL;
			}
		}
	}

	return ((byte *)snd);
}

/*___________________________________________________________________
|
|	Function: snd_LoadSound
| 
|	Input: Initializes and loads a sound from a file.
| Output:	Returns a pointer to the sound or 0 on any error.  This
|   overloaded version of this function returns other data about the
|   loaded sound.  This data only applies to static sounds, not
|   streaming sounds.
|___________________________________________________________________*/

Sound snd_LoadSound (
  char     *filename,       
  unsigned  controls,       
  unsigned *num_samples,            
  unsigned *bits_per_sample,          
  unsigned *duration_milliseconds,
  byte    **data )   
{
	int sound_loaded;
	SoundData *snd = 0;

/*____________________________________________________________________
|
| Verify input params
|___________________________________________________________________*/

  DEBUG_ASSERT (dsound8)
  DEBUG_ASSERT (filename)

/*____________________________________________________________________
|
| Main procedure
|___________________________________________________________________*/

  // Init callers variables to default values
  if (num_samples)
    *num_samples = 0;
  if (bits_per_sample)
    *bits_per_sample = 0;
  if (duration_milliseconds)
    *duration_milliseconds = 0;
  if (data)
    *data = 0;
 
  // Allocate memory for Sound struct
	snd = (SoundData *) calloc (1, sizeof(SoundData));
	if (snd) {
    // Allocate memory for filename
		snd->filename = (char *) calloc (strlen(filename)+1, sizeof(char));
		// On error, release any memory allocated
		if (snd->filename == NULL) {
      DEBUG_ERROR ("snd_LoadSound(): Error allocating memory for filename")
			free (snd);
			snd = 0;
		}
		else {
			// Copy filename into struct
			strcpy (snd->filename, filename);
			// Set other variables in struct
			snd->controls_enabled = controls;
      // Some controls can't be combined, make choices here
			if (snd->controls_enabled & snd_CONTROL_3D)
				snd->controls_enabled &= ~snd_CONTROL_PAN;
			// Load sound from a file
			if (snd->controls_enabled & snd_CONTROL_STREAMING)
				sound_loaded = Load_Streaming_Sound (snd);
			else
				sound_loaded = Load_Static_Sound (snd, num_samples, bits_per_sample, duration_milliseconds, data);
    }
	}

  if (snd == 0) {
    char str[200];
    sprintf (str, "snd_LoadSound(): Error, sound not loaded (%s)", filename);
    DEBUG_ERROR (str)
  }

	return ((Sound)snd);
}

/*___________________________________________________________________
|
|	Function: Load_Streaming_Sound
| 
|	Input: Called from snd_LoadSound()
| Output:	Loads sound data from a file into sound buffer, creating
|					buffer if needed.  Returns true if successful, else false on
|					any error.
|___________________________________________________________________*/

static int Load_Streaming_Sound (SoundData *snd)
{
	int i, events_created, ok;
  LPDIRECTSOUNDBUFFER dsbuffer;
	int loaded = FALSE;

	if (dsound8) {
		// Close any open file and free the buffer
		if (snd->buffer) {
			Wave2CloseReadFile (&snd->hmmio, &snd->pwfx);
			snd->buffer->Release ();
			snd->buffer = NULL;
		}
		// Open the file, get the wave format and descend to the data chunk
		if (Wave2OpenFile (snd->filename, &snd->hmmio, &snd->pwfx, &snd->mmckinfoParent) == 0) {
			// Advance to the start of the data chunk
			if (Wave2StartDataRead (&snd->hmmio, &snd->mmckinfo, &snd->mmckinfoParent) == 0) {
				// Create a new sound buffer to hold STREAMING_SOUND_SECONDS worth of data
				memset (&(snd->dsbdesc), 0, sizeof(DSBUFFERDESC));
				snd->dsbdesc.dwSize = sizeof(DSBUFFERDESC);
				snd->dsbdesc.dwFlags |= DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;
				if (snd->controls_enabled & snd_CONTROL_VOLUME)
					snd->dsbdesc.dwFlags |= DSBCAPS_CTRLVOLUME;
        if ((snd->controls_enabled & snd_CONTROL_PAN) AND (NOT (snd->controls_enabled & snd_CONTROL_3D)))
          snd->dsbdesc.dwFlags |= DSBCAPS_CTRLPAN;
        if ((snd->controls_enabled & snd_CONTROL_FREQUENCY) AND (NOT (snd->controls_enabled & snd_CONTROL_EFFECTS)))
					snd->dsbdesc.dwFlags |= DSBCAPS_CTRLFREQUENCY;
				if (snd->controls_enabled & snd_CONTROL_EFFECTS)
					snd->dsbdesc.dwFlags |= DSBCAPS_CTRLFX;
        if (snd->controls_enabled & snd_CONTROL_3D) {
          snd->dsbdesc.dwFlags |= DSBCAPS_CTRL3D;
          snd->dsbdesc.dwFlags |= DSBCAPS_MUTE3DATMAXDISTANCE;
        }
        if (snd->global_focus)
          snd->dsbdesc.dwFlags |= DSBCAPS_GLOBALFOCUS;
				snd->dsbdesc.dwBufferBytes = snd->pwfx->nAvgBytesPerSec * STREAMING_SOUND_SECONDS;
        // Make sure # bytes in buffer is divisible by NUM_CHUNKS
        snd->dsbdesc.dwBufferBytes += (snd->dsbdesc.dwBufferBytes % NUM_CHUNKS);
				snd->dsbdesc.lpwfxFormat = snd->pwfx;
				// Get a DirectSoundBuffer interface
				if (dsound8->CreateSoundBuffer (&(snd->dsbdesc), &dsbuffer, NULL) != DS_OK) 
					debug_WriteFile ("Load_Streaming_Sound(): Error creating DirectSoundBuffer");
			  else {
					// Get a DirectSoundBuffer8 interface from the DirectSoundBuffer interface
					if (dsbuffer->QueryInterface (IID_IDirectSoundBuffer8, (void **)&(snd->buffer)) != DS_OK)
						debug_WriteFile ("Load_Streaming_Sound(): Error creating DirectSoundBuffer8");
					dsbuffer->Release ();
				}
				// Any error creating buffer?
				if (snd->buffer == 0) {
					Wave2CloseReadFile (&snd->hmmio, &snd->pwfx);
					snd->buffer = NULL;
				}
				else {
          // Create 3D sound interface if needed
          ok = FALSE;
          if (snd->controls_enabled & snd_CONTROL_3D) {
            if (snd->buffer->QueryInterface (IID_IDirectSound3DBuffer8, (void **)&(snd->buffer3d)) != S_OK) 
							debug_WriteFile ("Load_Streaming_Sound(): Error creating DirectSound3DBuffer8");
						else
              ok = TRUE;  
          }                                     
          else
  					ok = TRUE;
          if (ok) {
            // Fill sound buffer with silence
					  Fill_Buffer_With_Silence (snd->buffer);
					  // Create an array of event objects - one for every notification postion
					  //  in the streaming buffer, plus one extra.
            events_created = TRUE;
					  for (i=0; i<(NUM_STREAMING_EVENTS+NUM_CHUNKS+1); i++) {
						  snd->streamEvent[i] = CreateEvent (NULL, FALSE, FALSE, NULL);
						  if (snd->streamEvent[i] == NULL) {
							  events_created = FALSE;
							  break;
						  }
					  }
					  // Were events created successfully
					  if (events_created) {
              snd->num_events = NUM_STREAMING_EVENTS + NUM_CHUNKS + 1;
              // Setup array of structs, one for each buffer event.  This struct has
						  //  2 members to be initialized: dwOffset - offset into buffer in which
						  //  signal occurs & hEventNotify - handle of event to set.
              for (i=0; i<NUM_CHUNKS; i++) {
                snd->streamPos[i].dwOffset     = (snd->dsbdesc.dwBufferBytes / NUM_CHUNKS) * (i+1) - 1;
                snd->streamPos[i].hEventNotify = snd->streamEvent[NUM_STREAMING_EVENTS+i];
              }
						  snd->streamPos[i].dwOffset		 = DSBPN_OFFSETSTOP;
 						  snd->streamPos[i].hEventNotify = snd->streamEvent[NUM_STREAMING_EVENTS+i];

						  // Get a ptr to DirectSoundNotify interface
						  if (snd->buffer->QueryInterface (IID_IDirectSoundNotify, (void **)&(snd->dsnotify)) == S_OK) 
							  // Call interfaces only method, passing info
                if (snd->dsnotify->SetNotificationPositions (NUM_CHUNKS+1, snd->streamPos) == DS_OK) 
  								  loaded = TRUE;
					  }
          }
				}
			}
		}
	}

	// On any error, free any resources
	if (NOT loaded) {
		for (i=0; i<(NUM_STREAMING_EVENTS+NUM_CHUNKS+1); i++) 
			if (snd->streamEvent[i]) {
				CloseHandle (snd->streamEvent[i]);
				snd->streamEvent[i] = 0;
			}
		if (snd->dsnotify) {
			snd->dsnotify->Release ();
			snd->dsnotify = 0;
		}
	}

	return (loaded);
}

/*___________________________________________________________________
|
|	Function: Fill_Buffer_With_Silence
| 
|	Input: Called from Load_Streaming_Sound(), Create_Streaming_Sound()
| Output:	Fills sound buffer with silence.
|___________________________________________________________________*/

static void Fill_Buffer_With_Silence (LPDIRECTSOUNDBUFFER8 soundbuffer)
{
  WAVEFORMATEX wfx;
  DWORD        cb1;
  PBYTE        pb1;
  
  if (soundbuffer->GetFormat (&wfx, sizeof(WAVEFORMATEX), NULL) == DS_OK) 
		if (soundbuffer->Lock (0, 0, (LPVOID *)&pb1, &cb1, NULL, NULL, DSBLOCK_ENTIREBUFFER) == DS_OK) {
			memset ((void *)pb1, (byte)((wfx.wBitsPerSample==8)?128:0), (size_t)cb1);
			soundbuffer->Unlock (pb1, cb1, NULL, 0);
		}
}

/*___________________________________________________________________
|
|	Function: Load_Static_Sound
| 
|	Input: Called from snd_LoadSound(), snd_PlaySound()
| Output:	Loads sound data from a file into sound buffer, creating
|					buffer if needed.  Returns true if successful, else false on
|					any error.
|___________________________________________________________________*/

static bool Load_Static_Sound (
  SoundData *snd,
  unsigned  *num_samples,           // optional
  unsigned  *bits_per_sample,       // optional
  unsigned  *duration_milliseconds, // optional
  byte     **data )                 // optional
{
  bool ok;
	unsigned min_buffsize;
	WAVEFORMATEX *pwfx;						// wave format info
	HMMIO					hmmio;					// file handle
	MMCKINFO			mmckinfo;				// chunk info
	MMCKINFO			mmckinfoParent;	// parent chunk info
	LPVOID			  lpvAudio1;
	DWORD					dwBytes1;
	UINT					cbBytesRead;
  LPDIRECTSOUNDBUFFER dsbuffer;
	bool loaded = false;

	// Get info about file
  if (WaveOpenFile (snd->filename, &hmmio, &pwfx, &mmckinfoParent) == 0) {
    // If sound is 3D, make sure loading a mono file
    if ((snd->controls_enabled & snd_CONTROL_3D) AND (pwfx->nChannels != 1)) {
      char str[200];
      sprintf (str, "Load_Static_Sound(): Error loading a non-mono 3D sound (%s)", snd->filename);
      DEBUG_ERROR (str)
    }
    else {
		  // Advance to the start of the data chunk
		  if (WaveStartDataRead (&hmmio, &mmckinfo, &mmckinfoParent) == 0) {

        // Set callers variables, if any
        if (num_samples)
          *num_samples = mmckinfo.cksize / (pwfx->wBitsPerSample / 8);
        if (bits_per_sample)
          *bits_per_sample = pwfx->wBitsPerSample;
        if (duration_milliseconds)
          *duration_milliseconds = (*num_samples * 1000) / pwfx->nSamplesPerSec; 

// Print some info to debug file
//char str[200];
//DEBUG_WRITE (itoa (pwfx->nSamplesPerSec, str, 10))
//DEBUG_WRITE (itoa (pwfx->wBitsPerSample, str, 10))
//DEBUG_WRITE (itoa (pwfx->nChannels, str, 10))
//DEBUG_WRITE (itoa (mmckinfo.cksize, str, 10))

			  // Create a new sound buffer?
			  if (snd->buffer == 0) {
				  memset (&(snd->dsbdesc), 0, sizeof(DSBUFFERDESC));
				  snd->dsbdesc.dwSize = sizeof(DSBUFFERDESC);  
//          // If eax available and this is not a 3d sound, put it in software
//          if (eax_Available () AND (NOT (snd->controls_enabled & snd_CONTROL_3D)))
//            snd->dsbdesc.dwFlags |= DSBCAPS_LOCSOFTWARE;
//          // Otherwise use dynamic voice management
//          else
            snd->dsbdesc.dwFlags |= DSBCAPS_LOCDEFER;
          if (snd->controls_enabled & snd_CONTROL_VOLUME)
					  snd->dsbdesc.dwFlags |= DSBCAPS_CTRLVOLUME;
				  if ((snd->controls_enabled & snd_CONTROL_PAN) AND (NOT (snd->controls_enabled & snd_CONTROL_3D)))
					  snd->dsbdesc.dwFlags |= DSBCAPS_CTRLPAN;
				  if (snd->controls_enabled & snd_CONTROL_FREQUENCY)
					  snd->dsbdesc.dwFlags |= DSBCAPS_CTRLFREQUENCY;
				  if (snd->controls_enabled & snd_CONTROL_EFFECTS)
					  snd->dsbdesc.dwFlags |= DSBCAPS_CTRLFX;
				  if (snd->controls_enabled & snd_CONTROL_3D) {
					  snd->dsbdesc.dwFlags |= DSBCAPS_CTRL3D;
					  snd->dsbdesc.dwFlags |= DSBCAPS_MUTE3DATMAXDISTANCE;
            snd->dsbdesc.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
          }
				  snd->dsbdesc.dwBufferBytes = mmckinfo.cksize;
				  // Make sure buffer is large enough for effects, if using effects
				  if (snd->controls_enabled & snd_CONTROL_EFFECTS) {
					  min_buffsize = DSBSIZE_FX_MIN * (pwfx->nSamplesPerSec * (pwfx->wBitsPerSample / 8) * pwfx->nChannels) / 1000;
					  if (snd->dsbdesc.dwBufferBytes < min_buffsize)
						  snd->dsbdesc.dwBufferBytes = min_buffsize;
				  }
				  snd->dsbdesc.lpwfxFormat = pwfx;
				  // Get a DirectSoundBuffer interface
				  if (dsound8->CreateSoundBuffer (&(snd->dsbdesc), &dsbuffer, NULL) != DS_OK) 
					  DEBUG_ERROR ("Load_Static_Sound(): Error creating DirectSoundBuffer interface")
  			  else {
					  // Get a DirectSoundBuffer8 interface from the DirectSoundBuffer interface
					  if (dsbuffer->QueryInterface (IID_IDirectSoundBuffer8, (void **)&(snd->buffer)) != S_OK)
						  DEBUG_ERROR ("Load_Static_Sound(): Error creating DirectSoundBuffer8 interface")
					  dsbuffer->Release ();
				  }
				  // Any error creating buffer?
				  if (snd->buffer == 0) 
					  WaveCloseReadFile (&hmmio, &pwfx);
			  }
			  // Does sound buffer exist?
			  if (snd->buffer) {
          // Create 3D sound interface if needed
          ok = false;
          if (snd->controls_enabled & snd_CONTROL_3D) {
            // Create 3D buffer interface
					  if (snd->buffer->QueryInterface (IID_IDirectSound3DBuffer8, (void **)&(snd->buffer3d)) != S_OK) 
						  DEBUG_ERROR ("Load_Static_Sound(): Error creating DirectSound3DBuffer8 interface")
            else 
              ok = true;  
          }                                     
          else
  				  ok = true;
          if (ok) {
            // Lock entire buffer
					  if (snd->buffer->Lock (0,0,&lpvAudio1,&dwBytes1,NULL,NULL,DSBLOCK_ENTIREBUFFER) != DS_OK) 
						  DEBUG_ERROR ("Load_Static_Sound(): Error locking buffer")
					  else {
						  // Read data from file, putting into buffer
						  if (WaveReadFile (hmmio,								// file handle
															  dwBytes1,							// # bytes to read
															  (BYTE *)lpvAudio1,		// destination
															  &mmckinfo,						// file chunk info
                                &cbBytesRead) == 0) {	// actual # bytes read
                // Make a copy of the data for the caller? (caller must free this buffer)
                if (data) {
                  *data = (byte *) malloc (mmckinfo.cksize);
                  if (*data) 
                    memcpy (*data, lpvAudio1, mmckinfo.cksize);
                }
							  loaded = true;
              }
						  else
							  DEBUG_ERROR ("Load_Static_Sound(): Error reading data from wave")
						  // Unlock the buffer
						  snd->buffer->Unlock (lpvAudio1, dwBytes1, NULL, 0);
					  }
          }
			  }
      }
    }
	  // Close input file
	  WaveCloseReadFile (&hmmio, &pwfx);
  }

	return (loaded);
}

/*___________________________________________________________________
|
|	Function: snd_FreeSound
| 
|	Output: Frees memory associated with a sound.
|___________________________________________________________________*/

void snd_FreeSound (Sound s)
{
  int i;
	SoundData *snd = (SoundData *)s;
	
	if (snd) {
		// If sound is playing, stop it
  	snd_StopSound (s);
		// Release the sound resources
		if (snd->buffer) {
      // Is this a streaming buffer?
      if (snd->controls_enabled & snd_CONTROL_STREAMING) {
				// Close streaming file				
        WaveCloseReadFile (&snd->hmmio, &snd->pwfx);
				// Release event handles
				for (i=0; i<snd->num_events; i++) 
					if (snd->streamEvent[i]) {
						CloseHandle (snd->streamEvent[i]);
						snd->streamEvent[i] = 0;
					}
				// Release DirectSoundNotify interface
				if (snd->dsnotify) {
					snd->dsnotify->Release ();
					snd->dsnotify = 0;
				}
			}
      // Free effects for this sound
      Free_Effects (snd);
			// Free all sound buffers for this sound
      if (snd->buffer3d)
        snd->buffer3d->Release ();
			snd->buffer->Release ();
		}
		// Free memory for filename
		if (snd->filename)
			free (snd->filename);
		// Free memory for Sound struct
		free (snd);
	}
}

/*___________________________________________________________________
|
|	Function: Free_Effects
| 
| Input: Called from snd_FreeSound(), snd_EnableEffects()
|	Output: Frees effects interfaces of a sound, if any.
|___________________________________________________________________*/

static void Free_Effects (SoundData *snd)
{
  if (snd->chorus) {
    snd->chorus->Release ();
    snd->chorus = 0;
  }
  if (snd->compressor) {
    snd->compressor->Release ();
    snd->compressor = 0;
  }
  if (snd->distortion) {
    snd->distortion->Release ();
    snd->distortion = 0;
  }
  if (snd->echo) {
    snd->echo->Release ();
    snd->echo = 0;
  }
  if (snd->flange) {
    snd->flange->Release ();
    snd->flange = 0;
  }
  if (snd->gargle) {
    snd->gargle->Release ();
    snd->gargle = 0;
  }
  if (snd->param_eq) {
    snd->param_eq->Release ();
    snd->param_eq = 0;
  }
  if (snd->waves_reverb) {
    snd->waves_reverb->Release ();
    snd->waves_reverb = 0;
  }
  if (snd->env_reverb) {
    snd->env_reverb->Release ();
    snd->env_reverb = 0;
  }
}

/*___________________________________________________________________
|
|	Function: snd_PlaySound
| 
|	Output: Plays a sound.
|
| Description: If sound is a streaming sound and gets its data from a
|   callback function, caller will have to explicity call snd_StopSound()
|   in order to stop the sound from 'playing'.  repeat has no effect to
|   this type of sound since it will continue to play forever.
|___________________________________________________________________*/

void snd_PlaySound (Sound s, int repeat)
{
	int sound_loaded;
	HRESULT hres;
	SoundData *snd = (SoundData *)s;
	DWORD dwThreadID, dwFlags = 0;

  if (dsound8 AND snd) {
		// Set play ptr to start of sound
		snd->buffer->SetCurrentPosition (0);
		// Play the sound if it's a streaming sound	
    if (snd->controls_enabled & snd_CONTROL_STREAMING) {
			// Is sound currently not playing?
			if (NOT snd->dsstream_thread) {
				snd->repeat = repeat;
        snd->dsstream_thread = (HANDLE)_beginthreadex (NULL, 
                                                       1024, 
                                                       (PBEGINTHREADEX_THREADFUNC)Streaming_Sound_Thread,
                                                       (void *)snd,
                                                       0,
                                                       (PBEGINTHREADEX_THREADID)&dwThreadID);
      }
		}
		// Play a static sound
		else {
			if (repeat) 
				dwFlags = DSBPLAY_LOOPING;
			hres = snd->buffer->Play (0, 0, dwFlags);
			// Was sound not played due to a lost buffer?
			if (hres == DSERR_BUFFERLOST)
				// Restore a lost buffer
				if (snd->buffer->Restore () == DS_OK) { 
					// Reload the sound data into the buffer
					sound_loaded = Load_Static_Sound (snd,0,0,0,0);
					if (sound_loaded)
					  // Try to play the sound now
						snd->buffer->Play (0, 0, dwFlags);
				}
		}
	}
}

/*___________________________________________________________________
|
|	Function: Streaming_Sound_Thread
| 
|	Input: Called from DirectSound_Play_Sound()
|	Output: The thread that plays a streaming sound.
|___________________________________________________________________*/

#define NOT_PAUSED		0				
#define PAUSED				1

DWORD WINAPI Streaming_Sound_Thread (LPVOID pParam)
{
	int done, load_new_data, pause, end_of_file, buffpos;
	DWORD dwStartOfs, dwNumBytes, dwBytesLocked, event;
	UINT	cbBytesRead;
	VOID *lpvData;
  WAVEFORMATEX wfx;
	SoundData *snd = (SoundData *)pParam;

	// Init variables
	snd->buffer->GetFormat (&wfx, sizeof(WAVEFORMATEX), NULL); 
	end_of_file = FALSE;
	pause				= NOT_PAUSED;
	done				= FALSE;

	// Start sound playing
  snd->buffer->Play (0, 0, DSBPLAY_LOOPING);

	while ((event = WaitForMultipleObjects (snd->num_events, snd->streamEvent, FALSE, INFINITE)) != WAIT_FAILED) {							
		load_new_data = FALSE;
		// Deal with the event that got signalled.
		switch (event-WAIT_OBJECT_0) {
      case 0:	// Pause the sound
              if (pause == NOT_PAUSED)      
                snd->buffer->Stop ();
              pause = PAUSED;
              ResetEvent (snd->streamEvent[0]);
							break;
			case 1: // Unpause the sound
							if (pause == PAUSED) 
								snd->buffer->Play (0, 0, DSBPLAY_LOOPING);
							pause = NOT_PAUSED;
							ResetEvent (snd->streamEvent[1]);
							break;
			case 2:	// Quit playing sound and exit
							snd->buffer->Stop ();
							done = TRUE;
							ResetEvent (snd->streamEvent[2]);
							break;
      default: // End of a buffer position reached - load new data (if not at end of file)   
              buffpos = event - WAIT_OBJECT_0 - NUM_STREAMING_EVENTS;

              // Sound has stopped playing?
              if ((buffpos == SOUND_EVENT_STOPPED) AND (NOT pause))
								done = TRUE;
              // End of file reached - stopped playing?
              if (end_of_file) {
								snd->buffer->Stop ();
								done = TRUE;
							}
              // Load new data?
              if ((NOT done) AND (pause != PAUSED)) {
                dwStartOfs = (snd->dsbdesc.dwBufferBytes / NUM_CHUNKS) * buffpos;
                dwNumBytes = snd->dsbdesc.dwBufferBytes / NUM_CHUNKS;
                load_new_data = TRUE;
              }
							break;
		}

/*___________________________________________________________________
|
| Load new data into streaming buffer, if needed
|___________________________________________________________________*/

		if (load_new_data) {
			// Lock half of buffer
			if (snd->buffer->Lock (dwStartOfs,			// offset of lock start
														 dwNumBytes,      // # bytes to lock
														 &lpvData,				// address of lock start
														 &dwBytesLocked,  // # bytes locked
														 NULL,						// address of wraparound lock
														 NULL,						// # of wraparound bytes
														 0) == 0) {			  // flags
        // Read data from a file
        cbBytesRead = 0;
        Wave2ReadFile (snd->hmmio,			// file handle
											 dwBytesLocked,		// # bytes to read
											 (BYTE *)lpvData,	// destination
											 &snd->mmckinfo,	// file chunk info
											 &cbBytesRead);		// actual # bytes read
				// End of file reached?                                                  
				if (cbBytesRead < (unsigned)dwBytesLocked) { 
          // If sound doesn't repeat, fill remainder of this block with silence
          if (NOT snd->repeat) {
						memset ((BYTE *)lpvData+cbBytesRead, (byte)((wfx.wBitsPerSample==8)?128:0), (size_t)(dwBytesLocked-cbBytesRead));
						end_of_file = TRUE;
					}
          // If sound repeats, start filling again from start of sound file
          else {						
						if (Wave2StartDataRead (&snd->hmmio, &snd->mmckinfo, &snd->mmckinfoParent) == 0) 
							Wave2ReadFile (snd->hmmio,									// file handle
														 dwBytesLocked-cbBytesRead,		// # bytes to read
														 (BYTE *)lpvData+cbBytesRead,	// destination
														 &snd->mmckinfo,							// file chunk info
														 &cbBytesRead);								// actual # bytes read
					}
        }
				// Unlock the buffer
				snd->buffer->Unlock (lpvData, dwBytesLocked, NULL, 0);
			}
		}

		// Break out of while loop when done
		if (done) 
			break;
	}

	return (0);
}

/*___________________________________________________________________
|
|	Function: snd_StopSound
| 
|	Input: Stops playing a sound.
|___________________________________________________________________*/

void snd_StopSound (Sound s)
{
	SoundData *snd = (SoundData *)s;

	if (dsound8 AND snd)
		// Stop playing a streaming sound?
    if (snd->controls_enabled & snd_CONTROL_STREAMING) {
			// Is this sound playing?
			if (snd->dsstream_thread) {
				SetEvent (snd->streamEvent[SOUND_EVENT_QUIT]);
        // Wait for thread to terminate
        WaitForSingleObject (snd->dsstream_thread, INFINITE);
        CloseHandle (snd->dsstream_thread);
				snd->dsstream_thread = 0;
			}
		}
		// Stop playing a static sound
		else
			snd->buffer->Stop ();
}

/*___________________________________________________________________
|
|	Function: snd_PauseSound
| 
|	Output: Pauses a currently playing streaming sound.  Only works with
|					streaming sounds.
|___________________________________________________________________*/

void snd_PauseSound (Sound s)
{
	SoundData *snd = (SoundData *)s;

	if (dsound8 AND snd)
		// Make sure this is a streaming buffer
    if (snd->controls_enabled & snd_CONTROL_STREAMING) 
			// Is this sound playing?
			if (snd->dsstream_thread) 	
				SetEvent (snd->streamEvent[SOUND_EVENT_PAUSE]);
}

/*___________________________________________________________________
|
|	Function: snd_UnpauseSound
| 
|	Output: Unpauses a currently paused streaming sound.  Only works with
|					streaming sounds.
|___________________________________________________________________*/

void snd_UnpauseSound (Sound s)
{
	SoundData *snd = (SoundData *)s;

	if (dsound8 AND snd)
		// Make sure this is a streaming buffer
    if (snd->controls_enabled & snd_CONTROL_STREAMING) 
			// Is this sound playing?
			if (snd->dsstream_thread)
				SetEvent (snd->streamEvent[SOUND_EVENT_UNPAUSE]);
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundVolume
| 
|	Output: Sets volume for a sound from 100 (loudest) to 0 (faintest).
|					Volume control must be enabled for the sound.
|___________________________________________________________________*/

void snd_SetSoundVolume (Sound s, int volume)
{
	LONG lVolume;
	SoundData *snd = (SoundData *)s;

	if (dsound8 AND snd)
		// Make sure volume control is enabled for this sound
    if (snd->controls_enabled & snd_CONTROL_VOLUME) 
			// Make sure volume is 0-100
			if ((volume >= 0) AND (volume <= 100)) {
				switch (volume) {
					case 100: // max volume
									  lVolume = DSBVOLUME_MAX;
									  break;
					case 0:	  // min volume (muted!)
									  lVolume = DSBVOLUME_MIN;
									  break;
					default:  // something in between
									  lVolume = (LONG)((float)(100-volume) / 100 * DSBVOLUME_MIN);
									  break;
        }           
				snd->buffer->SetVolume (lVolume);
			}
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundPan
| 
|	Output: Sets pan for a sound from -10 (left) to 10 (right) to 0 (center).
|					Pan control must be enabled for the sound.
|___________________________________________________________________*/

void snd_SetSoundPan (Sound s, int pan)
{
	LONG lPan;
	SoundData *snd = (SoundData *)s;

	if (dsound8 AND snd)
		// Make sure pan control is enabled for this sound
    if (snd->controls_enabled & snd_CONTROL_PAN) 
			// Make sure pan is -10 to 10
			if ((pan >= -10) AND (pan<= 10)) {
				switch (pan) {
					case -10: // left
									  lPan = DSBPAN_LEFT;
									  break;
					case 0:	  // center
									  lPan = DSBPAN_CENTER;
									  break;
          case 10:  // right
                    lPan = DSBPAN_RIGHT;
                    break;
					default:  // something in between
                    if (pan < 0)
                      lPan = (LONG)((float)(10-(-pan)) / 10 * DSBPAN_LEFT);
                    else // pan > 0
                      lPan = (LONG)((float)(10-pan) / 10 * DSBPAN_RIGHT);
 									 break;
				}
				snd->buffer->SetPan (lPan);
			}
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundFrequency
| 
|	Output: Sets frequency in hertz for a sound. Typical values for 
|		frequency are 100 to 100,000
|___________________________________________________________________*/

void snd_SetSoundFrequency (Sound s, unsigned hertz)
{
	SoundData *snd = (SoundData *)s;

	if (snd) 
		if ((hertz >= DSBFREQUENCY_MIN) AND (hertz <= DSBFREQUENCY_MAX)) 
			snd->buffer->SetFrequency ((DWORD)hertz);
}

/*___________________________________________________________________
|
|	Function: snd_ResetSoundFrequency
| 
|	Output: Sets frequency of a sound to its original frequency.
|___________________________________________________________________*/

void snd_ResetSoundFrequency (Sound s)
{
	SoundData *snd = (SoundData *)s;

	if (snd) 
		snd->buffer->SetFrequency (DSBFREQUENCY_ORIGINAL);
}

/*___________________________________________________________________
|
|	Function: snd_GetSoundFrequency
| 
|	Output: Returns the current frequency of a sound in hertz.
|___________________________________________________________________*/

unsigned snd_GetSoundFrequency (Sound s)
{
	DWORD dwFrequency = 0;
	SoundData *snd = (SoundData *)s;

	if (snd)
		snd->buffer->GetFrequency (&dwFrequency);

	return ((unsigned)dwFrequency);
}

/*___________________________________________________________________
|
|	Function: snd_IsPlaying
| 
|	Output: Returns true is a sound is playing, else false.
|___________________________________________________________________*/

int snd_IsPlaying (Sound s)
{
  DWORD status;
  SoundData *snd = (SoundData *)s;

  snd->buffer->GetStatus (&status);

  return (status & DSBSTATUS_PLAYING);
}

/*___________________________________________________________________
|
|	Function: snd_EnableEffects
| 
| Output:	Enables effect on a sound or 0 to disable all effects.  If 
|		sound is playing, stops it.  Returns a mask containing the enabled 
|   effects.
|
|		Only works with 8 or 16-bit PCM sounds with no more than two channels
|		and with a buffer large enough to hold DSBSIZE_FX_MIN milliseonds of
|		data.
|___________________________________________________________________*/

#define MAX_EFFECTS	9

unsigned snd_EnableEffects (Sound s, unsigned effects)
{
	int i, n;
	HRESULT hres;
	DWORD requested[MAX_EFFECTS], available[MAX_EFFECTS];
	DSEFFECTDESC dsfxdesc[MAX_EFFECTS];
	SoundData *snd = (SoundData *)s;
	unsigned effects_enabled = 0;

/*___________________________________________________________________
|
| Stop playing sound?
|___________________________________________________________________*/

  if (snd) {

    if (snd_IsPlaying (s)) 
      snd_StopSound (s);

/*___________________________________________________________________
|
| Disable all effects?
|___________________________________________________________________*/
  
		// Disable all effects?
    if (effects == 0) { 
      snd->buffer->SetFX (0, NULL, NULL);
      // Release any effects interfaces
//      Free_Effects (snd);
    }

/*___________________________________________________________________
|
| Attempt to enable some effects?
|___________________________________________________________________*/

		// Create array of requested effects
		else {
			for (i=0; i<MAX_EFFECTS; i++) {
				memset (&(dsfxdesc[i]), 0, sizeof(DSEFFECTDESC));
				dsfxdesc[i].dwSize = sizeof(DSEFFECTDESC);
			}
			n = 0;
			if (effects & snd_EFFECT_CHORUS) {
				dsfxdesc[n].guidDSFXClass = GUID_DSFX_STANDARD_CHORUS;
			  requested[n] = snd_EFFECT_CHORUS;
				n++;
			}
			if (effects & snd_EFFECT_COMPRESSION) {
				dsfxdesc[n].guidDSFXClass = GUID_DSFX_STANDARD_COMPRESSOR;
				requested[n] = snd_EFFECT_COMPRESSION;
				n++;
			}
			if (effects & snd_EFFECT_DISTORTION) {
				dsfxdesc[n].guidDSFXClass = GUID_DSFX_STANDARD_DISTORTION;
				requested[n] = snd_EFFECT_DISTORTION;
				n++;
			}
			if (effects & snd_EFFECT_ECHO) {
				dsfxdesc[n].guidDSFXClass = GUID_DSFX_STANDARD_ECHO;
				requested[n] = snd_EFFECT_ECHO;
				n++;
			}
  		if (effects & snd_EFFECT_FLANGE) {
				dsfxdesc[n].guidDSFXClass = GUID_DSFX_STANDARD_FLANGER;
				requested[n] = snd_EFFECT_FLANGE;
				n++;
			}
			if (effects & snd_EFFECT_GARGLE) {
				dsfxdesc[n].guidDSFXClass = GUID_DSFX_STANDARD_GARGLE;
				requested[n] = snd_EFFECT_GARGLE;
				n++;
			}
			if (effects & snd_EFFECT_PARAMETRIC_EQUALIZER) {
				dsfxdesc[n].guidDSFXClass = GUID_DSFX_STANDARD_PARAMEQ;
				requested[n] = snd_EFFECT_PARAMETRIC_EQUALIZER;
			}
			if (effects & snd_EFFECT_WAVES_REVERB) {
				dsfxdesc[n].guidDSFXClass = GUID_DSFX_WAVES_REVERB;
				requested[n] = snd_EFFECT_WAVES_REVERB;
				n++;
			}
			if (effects & snd_EFFECT_ENVIRONMENTAL_REVERB) {
				dsfxdesc[n].guidDSFXClass = GUID_DSFX_STANDARD_I3DL2REVERB;
				requested[n] = snd_EFFECT_ENVIRONMENTAL_REVERB;
				n++;
			}
				
			// Enable the selected effects
			hres = snd->buffer->SetFX (n, dsfxdesc, available);

			// Was the call successful?
//			if ((hres == DS_OK) OR (hres == DS_INCOMPLETE)) {
			if (hres == DS_OK) {
				// See what effects were enabled
				for (i=0; i<n; i++) 
					// Was this effect enabled?
					if ((available[i] == DSFXR_LOCHARDWARE) OR (available[i] == DSFXR_LOCSOFTWARE)) 
						effects_enabled |= requested[i];
			}
      else {
        n = 0;
				debug_WriteFile ("snd_EnableEffects(): call to SetFX() failed");
      }

/*___________________________________________________________________
|
| Get interfaces for any new effects enabled
|___________________________________________________________________*/

		  for (i=0; i<n; i++) {
			  switch (requested[i]) {
				  case snd_EFFECT_CHORUS:
					  if (snd->chorus == 0) 
						  if (snd->buffer->GetObjectInPath (GUID_DSFX_STANDARD_CHORUS, i, IID_IDirectSoundFXChorus8, (void **)&(snd->chorus)) != DS_OK) {
							  snd->chorus = 0;
							  effects_enabled &= ~snd_EFFECT_CHORUS;
						  }
					  break;
				  case snd_EFFECT_COMPRESSION:			
					  if (snd->compressor == 0) 
						  if (snd->buffer->GetObjectInPath (GUID_DSFX_STANDARD_COMPRESSOR, i, IID_IDirectSoundFXCompressor8, (void **)&(snd->compressor)) != DS_OK) {
							  snd->compressor = 0;
							  effects_enabled &= ~snd_EFFECT_COMPRESSION;
						  }
					  break;
				  case snd_EFFECT_DISTORTION:
					  if (snd->distortion == 0) 
						  if (snd->buffer->GetObjectInPath (GUID_DSFX_STANDARD_DISTORTION, i, IID_IDirectSoundFXDistortion8, (void **)&(snd->distortion)) != DS_OK) {
							  snd->distortion = 0;
							  effects_enabled &= ~snd_EFFECT_DISTORTION;
						  }
					  break;
				  case snd_EFFECT_ECHO:									
					  if (snd->echo == 0) 
						  if (snd->buffer->GetObjectInPath (GUID_DSFX_STANDARD_ECHO, i, IID_IDirectSoundFXEcho8, (void **)&(snd->echo)) != DS_OK) {
							  snd->echo = 0;
							  effects_enabled &= ~snd_EFFECT_ECHO;
						  }
					  break;
				  case snd_EFFECT_FLANGE:								
					  if (snd->flange == 0) 
						  if (snd->buffer->GetObjectInPath (GUID_DSFX_STANDARD_FLANGER, i, IID_IDirectSoundFXFlanger8, (void **)&(snd->flange)) != DS_OK) {
							  snd->flange = 0;
							  effects_enabled &= ~snd_EFFECT_FLANGE;
						  }
					  break;
				  case snd_EFFECT_GARGLE:
					  if (snd->gargle == 0) 
						  if (snd->buffer->GetObjectInPath (GUID_DSFX_STANDARD_GARGLE, i, IID_IDirectSoundFXGargle8, (void **)&(snd->gargle)) != DS_OK) {
							  snd->gargle = 0;
							  effects_enabled &= ~snd_EFFECT_GARGLE;
						  }
					  break;
				  case snd_EFFECT_PARAMETRIC_EQUALIZER:
					  if (snd->param_eq == 0) 
						  if (snd->buffer->GetObjectInPath (GUID_DSFX_STANDARD_PARAMEQ, i, IID_IDirectSoundFXParamEq8, (void **)&(snd->param_eq)) != DS_OK) {
							  snd->param_eq = 0;
							  effects_enabled &= ~snd_EFFECT_PARAMETRIC_EQUALIZER;
						  }
					  break;
				  case snd_EFFECT_WAVES_REVERB:					
					  if (snd->waves_reverb == 0) 
						  if (snd->buffer->GetObjectInPath (GUID_DSFX_WAVES_REVERB, i, IID_IDirectSoundFXWavesReverb8, (void **)&(snd->waves_reverb)) != DS_OK) {
							  snd->waves_reverb = 0;
							  effects_enabled &= ~snd_EFFECT_WAVES_REVERB;
						  }
					  break;
				  case snd_EFFECT_ENVIRONMENTAL_REVERB:	
					  if (snd->env_reverb == 0) 
						  if (snd->buffer->GetObjectInPath (GUID_DSFX_STANDARD_I3DL2REVERB, i, IID_IDirectSoundFXI3DL2Reverb8, (void **)&(snd->env_reverb)) != DS_OK) {
							  snd->env_reverb = 0;
							  effects_enabled &= ~snd_EFFECT_ENVIRONMENTAL_REVERB;
						  }
					  break;
			  }	// switch
      } // for
    } // else
  } // if

	return (effects_enabled);
}

/*___________________________________________________________________
|
|	Function: snd_SetEnvironment
| 
|	Output: Sets the environmental reverb properties to a preset.
|___________________________________________________________________*/

void snd_SetEnvironment (Sound s, int environment_preset)
{
  DWORD preset;
  SoundData *snd = (SoundData *)s;

  if (snd)
    if (snd->env_reverb) {
      switch (environment_preset) {
        case snd_ENVIRONMENT_DEFAULT:	
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
          break;
        case snd_ENVIRONMENT_GENERIC:					
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_GENERIC;
          break;
        case snd_ENVIRONMENT_PADDEDCELL:
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_PADDEDCELL;
          break;
        case snd_ENVIRONMENT_ROOM:						
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_ROOM;
          break;
        case snd_ENVIRONMENT_BATHROOM:				
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_BATHROOM;
          break;
        case snd_ENVIRONMENT_LIVINGROOM:			
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_LIVINGROOM;
          break;
        case snd_ENVIRONMENT_STONEROOM:				
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_STONEROOM;
          break;
        case snd_ENVIRONMENT_AUDITORIUM:			
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM;
          break;
        case snd_ENVIRONMENT_CONCERTHALL:			
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_CONCERTHALL;
          break;
        case snd_ENVIRONMENT_CAVE:						
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_CAVE;
          break;
        case snd_ENVIRONMENT_ARENA:						
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_ARENA;
          break;
        case snd_ENVIRONMENT_HANGAR:					
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_HANGAR;
          break;
        case snd_ENVIRONMENT_CARPETEDHALLWAY:	
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_CARPETEDHALLWAY;
          break;
        case snd_ENVIRONMENT_HALLWAY:					
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_HALLWAY;
          break;
        case snd_ENVIRONMENT_STONECORRIDOR:		
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
          break;
        case snd_ENVIRONMENT_ALLEY:						
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_ALLEY;
          break;
        case snd_ENVIRONMENT_FOREST:					
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_FOREST;
          break;
        case snd_ENVIRONMENT_CITY:						
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_CITY;
          break;
        case snd_ENVIRONMENT_MOUNTAINS:				
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_MOUNTAINS;
          break;
        case snd_ENVIRONMENT_QUARRY:				  
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_QUARRY;
          break;
        case snd_ENVIRONMENT_PLAIN:						
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_PLAIN;
          break;
        case snd_ENVIRONMENT_PARKINGLOT:			
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_PARKINGLOT;
          break;
        case snd_ENVIRONMENT_SEWERPIPE:				
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_SEWERPIPE;
          break;
        case snd_ENVIRONMENT_UNDERWATER:			
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_UNDERWATER;
          break;
        case snd_ENVIRONMENT_SMALLROOM:				
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_SMALLROOM;
          break;
        case snd_ENVIRONMENT_MEDIUMROOM:			
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_MEDIUMROOM;
          break;
        case snd_ENVIRONMENT_LARGEROOM:				
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_LARGEROOM;
          break;
        case snd_ENVIRONMENT_MEDIUMHALL:			
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_MEDIUMHALL;
          break;
        case snd_ENVIRONMENT_LARGEHALL:				
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_LARGEHALL;
          break;
        case snd_ENVIRONMENT_PLATE:						
          preset = DSFX_I3DL2_ENVIRONMENT_PRESET_PLATE;
          break;
      }
      if (snd->env_reverb->SetPreset (preset) != DS_OK)
        debug_WriteFile ("snd_SetEnvironment(): Error returned from SetPreset()");
    }
}

/*___________________________________________________________________
|
|	Function: snd_SetEffectProperties
| 
|	Output: Sets properties for an effect.
|___________________________________________________________________*/

void snd_SetEffectProperties (Sound s, unsigned effect, sndEffectProperties *properties)
{
  SoundData *snd = (SoundData *)s;

  if (snd)
    switch (effect) {
      case snd_EFFECT_CHORUS:
        if (snd->chorus) 
          snd->chorus->SetAllParameters ((DSFXChorus *) properties);
        break;
      case snd_EFFECT_COMPRESSION:			
        if (snd->compressor) 
          snd->compressor->SetAllParameters ((DSFXCompressor *) properties);
        break;
      case snd_EFFECT_DISTORTION:
        if (snd->distortion) 
          snd->distortion->SetAllParameters ((DSFXDistortion *) properties);
        break;
      case snd_EFFECT_ECHO:									
        if (snd->echo) 
          snd->echo->SetAllParameters ((DSFXEcho *) properties);
        break;
      case snd_EFFECT_FLANGE:								
        if (snd->flange) 
          snd->flange->SetAllParameters ((DSFXFlanger *) properties);
        break;
      case snd_EFFECT_GARGLE:
        if (snd->gargle) 
          snd->gargle->SetAllParameters ((DSFXGargle *) properties);
        break;
      case snd_EFFECT_PARAMETRIC_EQUALIZER:
        if (snd->param_eq) 
          snd->param_eq->SetAllParameters ((DSFXParamEq *) properties);
        break;
      case snd_EFFECT_WAVES_REVERB:					
        if (snd->waves_reverb) 
          snd->waves_reverb->SetAllParameters ((DSFXWavesReverb *) properties);
        break;
      case snd_EFFECT_ENVIRONMENTAL_REVERB:	
        if (snd->env_reverb) 
          snd->env_reverb->SetAllParameters ((DSFXI3DL2Reverb *) properties);
        break;
    }
}

/*___________________________________________________________________
|
|	Function: snd_GetEffectProperties
| 
|	Output: Gets properties for an effect.
|___________________________________________________________________*/

void snd_GetEffectProperties (Sound s, unsigned effect, sndEffectProperties *properties)
{
  SoundData *snd = (SoundData *)s;

  if (snd)
    switch (effect) {
      case snd_EFFECT_CHORUS:
        if (snd->chorus) 
          snd->chorus->GetAllParameters ((DSFXChorus *) properties);
        break;
      case snd_EFFECT_COMPRESSION:			
        if (snd->compressor) 
          snd->compressor->GetAllParameters ((DSFXCompressor *) properties);
        break;
      case snd_EFFECT_DISTORTION:
        if (snd->distortion) 
          snd->distortion->GetAllParameters ((DSFXDistortion *) properties);
        break;
      case snd_EFFECT_ECHO:									
        if (snd->echo) 
          snd->echo->GetAllParameters ((DSFXEcho *) properties);
        break;
      case snd_EFFECT_FLANGE:								
        if (snd->flange) 
          snd->flange->GetAllParameters ((DSFXFlanger *) properties);
        break;
      case snd_EFFECT_GARGLE:
        if (snd->gargle) 
          snd->gargle->GetAllParameters ((DSFXGargle *) properties);
        break;
      case snd_EFFECT_PARAMETRIC_EQUALIZER:
        if (snd->param_eq) 
          snd->param_eq->GetAllParameters ((DSFXParamEq *) properties);
        break;
      case snd_EFFECT_WAVES_REVERB:					
        if (snd->waves_reverb) 
          snd->waves_reverb->GetAllParameters ((DSFXWavesReverb *) properties);
        break;
      case snd_EFFECT_ENVIRONMENTAL_REVERB:	
        if (snd->env_reverb) 
          snd->env_reverb->GetAllParameters ((DSFXI3DL2Reverb *) properties);
        break;
    }
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundMode
| 
|	Output: Sets the mode for a 3d sound to:
|   snd_3D_MODE_DISABLE_3D      - disables 3D processing
|   snd_3D_MODE_HEAD_RELATIVE   - realtive to head
|   snd_3D_MODE_ORIGIN_RELATIVE - relative to origin
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetSoundMode (Sound s, int mode, int apply)
{
  DWORD dwMode;
	SoundData *snd = (SoundData *)s;

  switch (mode) {
    case snd_3D_MODE_DISABLE_3D:      dwMode = DS3DMODE_DISABLE;
                                      break;
    case snd_3D_MODE_HEAD_RELATIVE:   dwMode = DS3DMODE_HEADRELATIVE;
                                      break;
    case snd_3D_MODE_ORIGIN_RELATIVE: dwMode = DS3DMODE_NORMAL;
                                      break;
  }
  
  return (snd->buffer3d->SetMode (dwMode, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundPosition
| 
|	Output: Sets the position for a sound in 3-space relative to listener
|   or origin depending on mode.  3D coordinate system has x-axis increasing
|   to the right, y-axis increasing to the top and z-axis increasing
|   outward.
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetSoundPosition (Sound s, float x, float y, float z, int apply)
{
	SoundData *snd = (SoundData *)s;

  return (snd->buffer3d->SetPosition ((D3DVALUE)x, (D3DVALUE)y, (D3DVALUE)z, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundMinDistance
| 
|	Output: Sets the minimum distance the sound begins to decrease in 
|   volume.  Default is 1 units. (Default units is meters)
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetSoundMinDistance (Sound s, float distance, int apply)
{
	SoundData *snd = (SoundData *)s;

  return (snd->buffer3d->SetMinDistance ((D3DVALUE)distance, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundMaxDistance
| 
|	Output: Sets the maximum distance the sound ceases to decrease in 
|   volume.  Default is 1,000,000 kilometers.
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetSoundMaxDistance (Sound s, float distance, int apply)
{
	SoundData *snd = (SoundData *)s;

  return (snd->buffer3d->SetMaxDistance ((D3DVALUE)distance, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundConeOrientation
| 
|	Output: Sets the orientation for a sound cone using a vector relative 
|   to listener or origin depending on mode.  Has no effect unless the
|   cone angle and cone volume factor have also been set.
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetSoundConeOrientation (Sound s, float x, float y, float z, int apply)
{
	SoundData *snd = (SoundData *)s;

  return (snd->buffer3d->SetConeOrientation ((D3DVALUE)x, (D3DVALUE)y, (D3DVALUE)z, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundConeAngles
| 
|	Output: Sets the inside and outside angles of the sound projection
|   cone.  Sound will be full volume inside the angle and will be quiet
|   at edge of outside angle with transition in between outside of inside
|   angle and outside of outside angle.  Default for both is 360.
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetSoundConeAngles (Sound s, unsigned inside_angle, unsigned outside_angle, int apply)
{
	SoundData *snd = (SoundData *)s;

  return (snd->buffer3d->SetConeAngles ((DWORD)inside_angle, (DWORD)outside_angle, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundConeOutsideVolume
| 
|	Output: Sets the outside volume of sound cone.  Examples:
|   -1000 = sound attenuated by 10 decibles (sound quieter)
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetSoundConeOutsideVolume (Sound s, int volume, int apply)
{
	SoundData *snd = (SoundData *)s;

  return (snd->buffer3d->SetConeOutsideVolume ((LONG)volume, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetSoundVelocity
| 
|	Output: Sets the velocity vector for a sound.
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetSoundVelocity (Sound s, float x, float y, float z, int apply)
{
	SoundData *snd = (SoundData *)s;

  return (snd->buffer3d->SetVelocity ((D3DVALUE)x, (D3DVALUE)y, (D3DVALUE)z, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetListenerDistanceFactor
| 
|	Output: Sets the current distance factor which is the number of meters
|   per unit used by DirectSound (default is 1).
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetListenerDistanceFactor (float factor, int apply)
{
  return (dsound3dlistener8->SetDistanceFactor ((D3DVALUE)factor, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetListenerDistanceFactorToFeet
| 
|	Output: Sets the current distance factor used by DirectSound to feet.
|		(default is meters)
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetListenerDistanceFactorToFeet (int apply)
{
  return (dsound3dlistener8->SetDistanceFactor ((D3DVALUE)0.3048, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetListenerRolloff
| 
|	Output: Sets rolloff for all 3D sounds from:
|   -10 = min rolloff - sound diminishes less in relation to distance
|     0 = normal rolloff (as in reality)
|    10 = max rolloff - sound diminishes more in relation to distance
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetListenerRolloff (int factor, int apply)
{
  double rolloff;

  if (factor < 0) 
    rolloff = DS3D_DEFAULTROLLOFFFACTOR + (DS3D_DEFAULTROLLOFFFACTOR - DS3D_MINROLLOFFFACTOR) * ((float)factor / 10.0);
  else
    rolloff = DS3D_DEFAULTROLLOFFFACTOR + (DS3D_MAXROLLOFFFACTOR - DS3D_DEFAULTROLLOFFFACTOR) * ((float)factor / 10.0);

  // Error checking - make sure rolloff is within acceptable range
  if (rolloff < DS3D_MINROLLOFFFACTOR)
    rolloff = DS3D_MINROLLOFFFACTOR;
  else if (rolloff > DS3D_MAXROLLOFFFACTOR)
    rolloff = DS3D_MAXROLLOFFFACTOR;

  return (dsound3dlistener8->SetRolloffFactor ((D3DVALUE)rolloff, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetListenerVelocity
| 
|	Output: Sets the velocity vector for the listener.
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetListenerVelocity (float x, float y, float z, int apply)
{
  return (dsound3dlistener8->SetVelocity ((D3DVALUE)x, (D3DVALUE)y, (D3DVALUE)z, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetListenerPosition
| 
|	Output: Sets the position for the listener in 3-space distance units. 
|   3D coordinate system has x-axis increasing to the right, y-axis 
|   increasing to the top and z-axis increasing outward.
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetListenerPosition (float x, float y, float z, int apply)
{
  return (dsound3dlistener8->SetPosition ((D3DVALUE)x, (D3DVALUE)y, (D3DVALUE)z, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetListenerOrientation
| 
|	Output: Sets the orientation for the listener in terms of a front and
|   top vector.
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetListenerOrientation (
  float front_x, 
  float front_y, 
  float front_z, 
  float up_x,
  float up_y,
  float up_z,
  int   apply )
{
  return (dsound3dlistener8->SetOrientation ((D3DVALUE)front_x, (D3DVALUE)front_y, (D3DVALUE)front_z, 
                                             (D3DVALUE)up_x,    (D3DVALUE)up_y,    (D3DVALUE)up_z,
                                             dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_SetListenerDopplerFactor
| 
|	Output: Sets doppler factor for all 3D sounds from:
|   -10 = min - less than normal doppler effects
|     0 = normal (as in reality)
|    10 = max - more than normal doppler effects
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_SetListenerDopplerFactor (int factor, int apply)
{
  double doppler;

  if (factor < 0) 
    doppler = DS3D_DEFAULTDOPPLERFACTOR + (DS3D_DEFAULTDOPPLERFACTOR - DS3D_MINDOPPLERFACTOR) * ((float)factor / 10.0);
  else
    doppler = DS3D_DEFAULTDOPPLERFACTOR + (DS3D_MAXDOPPLERFACTOR - DS3D_DEFAULTDOPPLERFACTOR) * ((float)factor / 10.0);

  // Error checking - make sure doppler is within acceptable range
  if (doppler < DS3D_MINDOPPLERFACTOR)
    doppler = DS3D_MINDOPPLERFACTOR;
  else if (doppler > DS3D_MAXDOPPLERFACTOR)
    doppler = DS3D_MAXDOPPLERFACTOR;

  return (dsound3dlistener8->SetDopplerFactor ((D3DVALUE)doppler, dwApply[apply]) == DS_OK);
}

/*___________________________________________________________________
|
|	Function: snd_Commit3DDeferredSettings
| 
|	Output: Commits any deferred settings made since last call to this
|   function.
|
|   Returns true on success else false on any error.
|___________________________________________________________________*/

int snd_Commit3DDeferredSettings (void)
{
  return (dsound3dlistener8->CommitDeferredSettings () == DS_OK);
}
