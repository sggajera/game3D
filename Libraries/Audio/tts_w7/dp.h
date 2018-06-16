/*____________________________________________________________________
|
| File: dp.h (tts_w7.h)
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

/*____________________
|
| Include files
|___________________*/

#include <windows.h>
#include <winbase.h>

#include <stdio.h>
#include <string.h>
#include <sapi.h>
#pragma warning(disable: 4996)  // to eliminate a deprecation error in sphelper.h
#include <sphelper.h>

#include <defines.h>
#include <clib.h>
#include <list.h>

#include "tts_w7.h"

/*___________________
|
| Type definitions
|__________________*/

enum ttsType {
  tts_TYPE_REGULAR,
  tts_TYPE_STREAMING_DSOUND,
  tts_TYPE_STATIC_DSOUND
};

struct TextToSpeechData {
  ttsType            type;
  ISpVoice          *ispvoice;
  // If linked to a DirectSound buffer
  HANDLE             thread;
  HANDLE             event[2];
  Sound              dsound_buffer;
  CComPtr<ISpStream> ispStream;
  CComPtr<IStream>   iStream; 
  unsigned           stream_size;   // # bytes in stream (at any one time)
  List               queue;
  CRITICAL_SECTION   critsection_queue;
  HANDLE             speak_done;    // used by tts_eax.cpp 
};

/*___________________
|
| Macros
|__________________*/

#define TTS ((TextToSpeechData *)tts)

/*___________________
|
| Global variables
|__________________*/

#ifdef _DEBUG
#ifdef _TTS_W7_CPP_
bool tts_caps_printed = false;
#else
extern bool tts_caps_printed;
#endif
#endif
