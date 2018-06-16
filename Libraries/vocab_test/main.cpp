#include <first_header.h>
#include <windows.h>

#include <iostream>
#include <fstream>
using namespace std;

#include <defines.h>
#include <clib.h>
#include <vocab.h>

#pragma comment (lib, "clib.lib")
#pragma comment (lib, "vocab.lib")

// Aborts program abnormally, optionally prints a string to a message box
void win_Abort_Program (char *str)
{
  exit (1);
}

void main () 
{
  bool found;
  Vocabulary vocab;
  int i, n;
  byte *phonemes, visemes[100];

  cout << "Loading vocabulary ...";
  vocab = vocab_Init ("CMU Dictionary\\cmudict_0_6.txt");
  cout << "done" << endl;

  if (vocab) {
    found = vocab_TranslateWord (vocab, "market", &n, &phonemes);
    if (found) {
      cout << "Word found!" << endl;
      cout << "(phonemes) ";
      for (i=0; i<n; i++)
        cout << (int)(phonemes[i]) << ' ';
      cout << endl;
      // Convert phonemes to visemes
      vocab_TranslatePhonemesToVisemes (phonemes, visemes, n);
      cout << "(visemes) ";
      for (i=0; i<n; i++)
        cout << (int)(visemes[i]) << ' ';
      cout << endl;
      free (phonemes);
    }
    else
      cout << "Word not found" << endl;
  }

  vocab_Free (vocab);
}
