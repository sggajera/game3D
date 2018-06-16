/*____________________________________________________________________
|
| File: tts_w7.h
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#ifndef _TTS_W7_H_
#define _TTS_W7_H_

// The library uses snd8.lib
#include <snd8.h>

typedef void *TextToSpeech;

// voices
enum ttsVoice {
  tts_VOICE_MSMARY,     // Microsoft Mary (female)
  tts_VOICE_MSMIKE,     // Microsoft Mike (male)
  tts_VOICE_MSSAM,      // Microsoft Sam (male, older)
  tts_VOICE_ATTMIKE,    // AT&T Natural Voices Mike (16K)
  tts_VOICE_ATTCRYSTAL, // AT&T Natural Voices Crystal (16K)
  tts_VOICE_ATTAUDREY   // AT&T Natural Voices Audrey (16K)
};                  

// speak flags
enum ttsSpeakFlags {
  tts_SPEAKFLAGS_NONE   = 0,
//  tts_SPEAKFLAGS_ISFILE = 0x1, // string is a filename
  tts_SPEAKFLAGS_SYNC   = 0x2, // speak synchronously, default is Speak() is asynchronous
  tts_SPEAKFLAGS_PURGE  = 0x4  // purge any pending Speak() requests
};

// Initialize a TTS object
TextToSpeech tts_Init ();

// Print info about installed TTS voices
void         tts_PrintInfo ();
// Frees the TTS object
void         tts_Free      (TextToSpeech tts);
// Speaks
void         tts_Speak     (TextToSpeech tts, char *str, ttsSpeakFlags flags = tts_SPEAKFLAGS_NONE);
// Stops any speaking immediately
void         tts_Stop      (TextToSpeech tts);
// Sets the voice to use
void         tts_SetVoice  (TextToSpeech tts, ttsVoice voice);
// Sets the base volume (0-100)
void         tts_SetVolume (TextToSpeech tts, int volume);
// Adjusts the speaking reate (-10 to 10)
void         tts_SetRate   (TextToSpeech tts, int rate);
// Speaks to a WAV file
void         tts_SpeakToWavFile (TextToSpeech tts, char *str, char *filename);

#endif
