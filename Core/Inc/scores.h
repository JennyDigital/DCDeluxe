#ifndef _SCORES_H
#define _SCORES_H

#include "notes.h"
#include <stdint.h>

// Structue, First, master volume divider, bpm, then droprate,
// then for each note:  channel, note, beats-delay

const float  scale4[] =
{
  2.2, 160, 3,

  1, nC4, 2,
  2, nD4, 2,
  3, nE4, 2,
  1, nF4, 2,
  2, nG4, 2,
  3, nA4, 2,
  1, nB4, 2,
  0, END_PLAY, 1

};

//float  old_smokey[] =
//{


//};


#endif
