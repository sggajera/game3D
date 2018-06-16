/*____________________________________________________________________
|
| File: tts_test.cpp
|
| Description: Test program for tts_vista library.
|
| Functions: win_Get_Window_Handle
|            win_Abort_Program
|            main
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

#include <iostream>
using namespace std;

#include <defines.h>
#include <clib.h>
#include <tts_w7.h>

// Libraries to link in
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "dxguid.lib")

#pragma comment (lib, "dsound.lib")

#pragma comment (lib, "clib.lib")
#pragma comment (lib, "list.lib")
#pragma comment (lib, "snd8.lib")
#pragma comment (lib, "tts_w7.lib")

/*____________________________________________________________________
|
| Function: win_Get_Window_Handle
|
| Outputs: Returns the window handle of a console application.
|___________________________________________________________________*/

HWND win_Get_Window_Handle ()
{
  char title[256];
  // Get the name of this console app
  if (GetConsoleTitle (title, 256) != 0) 
    // Use that to get the window handle
    return (FindWindow (0, title));
  else {
    cout << "Error getting Window Handle\n";
    return (0);
  }
}

/*____________________________________________________________________
|
| Function: win_Abort_Program
|
| Outputs: Aborts program abnormally, optionally prints a string to a 
|   message box
|___________________________________________________________________*/

void win_Abort_Program (char *str)
{
  exit (1);
}

/*____________________________________________________________________
|
| Function: main
|
| Outputs: 
|___________________________________________________________________*/

void main ()
{
  TextToSpeech tts;
  
  cout << "Hello, how are you today?\n";

  tts = tts_Init ();
  tts_SetVoice (tts, tts_VOICE_ATTAUDREY);
  tts_Speak (tts, "Hello, how are you today?", tts_SPEAKFLAGS_SYNC);
	// Write to a file also
	tts_SpeakToWavFile (tts, "Hello, how are you today?", "how_are_you.wav");
  tts_Free (tts);
}
