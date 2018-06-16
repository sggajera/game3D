/*____________________________________________________________________
|
| File: tts_w7.cpp
|
| Description: Contains functions for Text-to-Speech support.  Written
|   for SAPI 5.1 SDK.  
|
|   Assumes the same thread will make use of this library - so this 
|   code is not necessarily thread safe.
|
| Functions:	tts_Init
|             tts_Free
|             tts_PrintInfo
|             tts_Speak
|             tts_Stop
|             tts_SetVoice  
|             tts_SetVolume
|             tts_SetRate
|             tts_SpeakToWavFile
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define _TTS_W7_CPP_

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>

#include "dp.h"

/*____________________________________________________________________
|
| Function: tts_Init
|
| Outputs: Initializes a regular TTS object.  Returns object handle 
|   or 0 on any error.
|___________________________________________________________________*/

TextToSpeech tts_Init ()
{
  TextToSpeechData *ttsdata;
  bool initialized = false;

/*___________________________________________________________________
|
| Create the tts object
|___________________________________________________________________*/

  // Create the tts object
  ttsdata = (TextToSpeechData *) calloc (1, sizeof(TextToSpeechData));
  if (ttsdata) {
    ttsdata->type = tts_TYPE_REGULAR;
    
/*___________________________________________________________________
|
| Initialize tts
|___________________________________________________________________*/

    // Initialize the COM system
    CoInitialize (0);
    
    // Get a ptr to TTS voice interface
    if (CoCreateInstance (CLSID_SpVoice, 0, CLSCTX_ALL, IID_ISpVoice, (void **)&(ttsdata->ispvoice)) != S_OK) 
      DEBUG_ERROR ("tts_Init(): Error getting ISpVoice interface")
    else {
      // Set to 'blend' speaking with other voices (don't serialize all TTS voices)
      ttsdata->ispvoice->SetPriority (SPVPRI_OVER);
      initialized = true;
    }
  }

/*___________________________________________________________________
|
| On any error, release any resources created
|___________________________________________________________________*/

  if (NOT initialized) {
    tts_Free (ttsdata);
    ttsdata = 0;
    DEBUG_ERROR ("tts_Init(): Error creating tts object")
  }
    
/*___________________________________________________________________
|
| Print TTS capabilities to debug file
|___________________________________________________________________*/

//#ifdef _DEBUG
//  if (NOT tts_caps_printed) {
//    tts_PrintInfo ();
//    tts_caps_printed = true;
//  }
//#endif

  return ((TextToSpeech)ttsdata);                              
}

/*___________________________________________________________________
|
|	Function: tts_Free
| 
|	Output: Frees TTS object.
|___________________________________________________________________*/

void tts_Free (TextToSpeech tts)
{
  if (tts) {
    // Stop voice playing if needed
    tts_Stop (tts);
    // Release the TTS voice interface, if any
    if (TTS->ispvoice) 
      TTS->ispvoice->Release ();
    // Free the TTS object
    free (TTS);
    
    CoUninitialize ();
  }
}

/*___________________________________________________________________
|
|	Function: tts_PrintInfo
| 
|	Output: Enumerates voices to debug file.
|___________________________________________________________________*/

void tts_PrintInfo ()
{
  int i, size, n;
  ULONG ulCount;
  WCHAR *wstr;
  char *id, *p;
  IEnumSpObjectTokens *cpEnum = 0;
  ISpObjectToken      *cpVoiceToken = 0;

  const int INDENT = 2;

  debug_WriteFile ("__________ TTS Info __________");
  debug_WriteFile ("Installed voices:");

  // Enumerate the available voices
  n = 0;
  if (SpEnumTokens (SPCAT_VOICES, 0, 0, &cpEnum) == S_OK) {
    // Get the number of voices installed
    if (cpEnum->GetCount (&ulCount) == S_OK) {
      // Cycle through list of voices
      for (bool set=false; (NOT set) AND ulCount; ulCount--) {
        if (cpEnum->Next (1, &cpVoiceToken, 0) == S_OK) {
          // Get the ID for this voice
          if (cpVoiceToken->GetId (&wstr) == S_OK) {
            // Count number of characters in the ID
            for (size=0; wstr[size]; size++);
            // Convert the ID to a (normal) string
            id = (char *) calloc (size+1+INDENT, sizeof(char));
            if (id) {
              for (i=0; i<size; i++)
                id[i] = (char)wstr[i];     
              // If a '\' in the string strip off everything before the last '\'
              p = strrchr (id, '\\');
              if (p) {
                p++;  // skip '\'
                if (*p)
                  strcpy (id, p);
              }
              for (i=0; i<INDENT; i++)
                strins (id, 0, " ");
              debug_WriteFile (id);
              free (id);
            }
            CoTaskMemFree (wstr);
          }
          cpVoiceToken->Release ();
          n++;
        }
      }
    }
  }

  if (n == 0)
    debug_WriteFile ("  No voices found");
  debug_WriteFile ("");
}

/*___________________________________________________________________
|
|	Function: tts_Speak
| 
|	Output: Speaks a string (or file) using the current voice.
|___________________________________________________________________*/

void tts_Speak (TextToSpeech tts, char *str, ttsSpeakFlags flags)
{
  unsigned i;
  WCHAR *wstr;
  DWORD spkflags = SPF_DEFAULT;

/*___________________________________________________________________
|
|	Validate input params
|___________________________________________________________________*/

  DEBUG_ASSERT (tts)
  DEBUG_ASSERT (TTS->ispvoice)
  DEBUG_ASSERT (str)

/*___________________________________________________________________
|
|	Main routine
|___________________________________________________________________*/

  if (tts AND str) {

/*___________________________________________________________________
|
|	Convert the string into a wide character string
|___________________________________________________________________*/

    wstr = (WCHAR *) calloc (strlen(str)+1, sizeof(WCHAR));
    if (wstr) {
      for (i=0; i<strlen(str); i++)
        wstr[i] = str[i];

/*___________________________________________________________________
|
|	Speak directly to audio hardware (sound card)
|___________________________________________________________________*/

      // Set any speak flags
      if (NOT (flags & tts_SPEAKFLAGS_SYNC))
        spkflags |= SPF_ASYNC;
      if (flags & tts_SPEAKFLAGS_PURGE)
        spkflags |= SPF_PURGEBEFORESPEAK;
      // Speak the string (or file)
      TTS->ispvoice->Speak (wstr, spkflags, 0);

/*___________________________________________________________________
|
|	Free the wide character string
|___________________________________________________________________*/

      free (wstr);
    }
  }
}

/*___________________________________________________________________
|
|	Function: tts_Stop
| 
|	Output: Stops any speaking immediately.
|___________________________________________________________________*/

void tts_Stop (TextToSpeech tts)
{
  DWORD spkflags = SPF_DEFAULT;

/*___________________________________________________________________
|
|	Validate input params
|___________________________________________________________________*/

  DEBUG_ASSERT (tts)
  DEBUG_ASSERT (TTS->ispvoice)

/*___________________________________________________________________
|
|	Main routine
|___________________________________________________________________*/

  if (tts) {
    spkflags |= SPF_PURGEBEFORESPEAK;
    TTS->ispvoice->Speak (0, spkflags, 0);
  }
}

/*___________________________________________________________________
|
|	Function: tts_SetVoice
| 
|	Output: Sets the voice.
|___________________________________________________________________*/

void tts_SetVoice (TextToSpeech tts, ttsVoice voice)
{
  int i, size;
  ULONG ulCount;
  WCHAR *wstr;
  char voice_to_use[30], *id;
  IEnumSpObjectTokens *cpEnum = 0;
  ISpObjectToken      *cpVoiceToken = 0;

/*___________________________________________________________________
|
|	Validate input params
|___________________________________________________________________*/

  DEBUG_ASSERT (tts)
  DEBUG_ASSERT (TTS->ispvoice)

/*___________________________________________________________________
|
|	Main routine
|___________________________________________________________________*/

  if (tts) {
    // Create a string containing voice to look for
    switch (voice) {
      case tts_VOICE_MSMARY: strcpy (voice_to_use, "Mary");
                            break;
      case tts_VOICE_MSMIKE: strcpy (voice_to_use, "Mike");
                            break;
      case tts_VOICE_MSSAM:  strcpy (voice_to_use, "Sam");
                            break;
      case tts_VOICE_ATTMIKE: strcpy (voice_to_use, "ATT-DT-14-Mike");
                            break;
      case tts_VOICE_ATTCRYSTAL: strcpy (voice_to_use, "ATT-DT-14-Crystal");
                            break;
      case tts_VOICE_ATTAUDREY: strcpy (voice_to_use, "ATT-DT-14-Audrey");
                            break;
    }

    // Enumerate the available voices
    if (SpEnumTokens (SPCAT_VOICES, 0, 0, &cpEnum) == S_OK) {
      // Get the number of voices installed
      if (cpEnum->GetCount (&ulCount) == S_OK) {
        // If there is at least one voice available
        if (ulCount >= 1) {
          // Search through list of voices for the one looking for
          for (bool set=false; (NOT set) AND ulCount; ulCount--) {
            if (cpEnum->Next (1, &cpVoiceToken, 0) == S_OK) {
              // Get the ID for this voice
              if (cpVoiceToken->GetId (&wstr) == S_OK) {
                // Count number of characters in the ID
                for (size=0; wstr[size]; size++);
                // Is the ID at least as long the voice looking for?
                if (size >= (int)strlen(voice_to_use)) {
                  // Convert the ID to a (normal) string
                  id = (char *) calloc (size+1, sizeof(char));
                  if (id) {
                    for (i=0; i<size; i++)
                      id[i] = (char)wstr[i];     
                    // Are the ending characters of the ID the same as the voice looking for
                    bool same;
                    for (i=0, same=true; i<(int)strlen(voice_to_use) AND same; i++)
                      if (id[size-1-i] != voice_to_use[strlen(voice_to_use)-1-i])
                        same = false;
                    free (id);
                    // Set new voice?
                    if (same) 
                      if (TTS->ispvoice->SetVoice (cpVoiceToken) == S_OK)
                        set = true;
                  }
                }
                CoTaskMemFree (wstr);
              }
              cpVoiceToken->Release ();
            }
          }
        }
      }
    }
  }
}

/*___________________________________________________________________
|
|	Function: tts_SetVolume
| 
|	Output: Sets the volume for a voice from 0 (lowest) to 100 (highest).
|___________________________________________________________________*/

void tts_SetVolume (TextToSpeech tts, int volume)
{

/*___________________________________________________________________
|
|	Validate input params
|___________________________________________________________________*/

  DEBUG_ASSERT (tts)
  DEBUG_ASSERT (TTS->ispvoice)
  DEBUG_ASSERT (volume >= 0 AND volume <= 100)

/*___________________________________________________________________
|
|	Main routine
|___________________________________________________________________*/

  if (tts) 
    // Is volume in valid range?
    if ((volume >= 0) AND (volume <= 100))
      TTS->ispvoice->SetVolume (volume);
}

/*___________________________________________________________________
|
|	Function: tts_SetRate
| 
|	Output: Adjusts the speaking rate from -10 to 10.
|___________________________________________________________________*/

void tts_SetRate (TextToSpeech tts, int rate)
{

/*___________________________________________________________________
|
|	Validate input params
|___________________________________________________________________*/

  DEBUG_ASSERT (tts)
  DEBUG_ASSERT (TTS->ispvoice)
  DEBUG_ASSERT (rate >= -10 AND rate <= 10)

/*___________________________________________________________________
|
|	Main routine
|___________________________________________________________________*/

  if (tts) 
    // Is rate in valid range?
    if ((rate >= -10) AND (rate <= 10))
      TTS->ispvoice->SetRate (rate);
}

/*___________________________________________________________________
|
|	Function: tts_SetVtts_SpeakToWavFileolume
| 
|	Output: Speaks to a WAV file.
|___________________________________________________________________*/

void tts_SpeakToWavFile (TextToSpeech tts, char *str, char *filename)
{
  unsigned i;
	CComPtr <ISpStream>	cpStream;
	CSpStreamFormat			cAudioFmt;
  WCHAR              *wstr = 0, *wfilename = 0;

/*___________________________________________________________________
|
|	Validate input params
|___________________________________________________________________*/

  DEBUG_ASSERT (tts)
  DEBUG_ASSERT (TTS->ispvoice)

/*___________________________________________________________________
|
|	Main routine
|___________________________________________________________________*/

  if (tts) {
    // Convert strings into a wide character strings
    wstr = (WCHAR *) calloc (strlen(str)+1, sizeof(WCHAR));
    if (wstr) 
      for (i=0; i<strlen(str); i++)
        wstr[i] = str[i];
    wfilename = (WCHAR *) calloc (strlen(filename)+1, sizeof(WCHAR));
    if (wfilename) 
      for (i=0; i<strlen(filename); i++)
        wfilename[i] = filename[i];

    // Strings created ok?
    if (wstr AND wfilename) 
      // Set the output audio format
      if (SUCCEEDED (cAudioFmt.AssignFormat (SPSF_16kHz16BitMono)))
	      // Call SPBindToFile, a SAPI helper method,  to bind the audio stream to the file
        if (SUCCEEDED (SPBindToFile (wfilename,  SPFM_CREATE_ALWAYS, &cpStream, &cAudioFmt.FormatId(),cAudioFmt.WaveFormatExPtr()))) 
  	      // Set the output to cpStream so that the output audio data will be stored in cpStream
          if (SUCCEEDED (TTS->ispvoice->SetOutput (cpStream, true))) {
       	    // Speak the text synchronously
            TTS->ispvoice->Speak (wstr, SPF_DEFAULT, 0);
        	  // Close the stream
		        cpStream->Close ();
            // Reset the tts voice output stream
            TTS->ispvoice->SetOutput (0, false);
          }
  }

/*___________________________________________________________________
|
|	Free resources
|___________________________________________________________________*/

  cpStream.Release ();

  if (wstr)
    free (wstr);
  if (wfilename)
    free (wfilename);
}
