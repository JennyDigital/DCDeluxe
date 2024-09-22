/*  DCDeluxe. A door chime based on the STM32.
    Copyright (C) 2024 Jennifer Gunn (JennyDigital).

	jennifer.a.gunn@outlook.com

    This software/hardware is free and open; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
*/

#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#define TRUE    -1
#define FALSE   0


/** User configuration section
  *
  */
//#define TEST_CYCLING_SET
//#define DEBUG_MODE

/** The three pre-defined chime-sets.
  *
  * It should be noted that I have yet to tailor set three to differing values,
  * but you know what you like more than me.
  */

//#define CHIMESET_ONE
//#define CHIMESET_TWO
#define CHIMESET_THREE

#ifdef CHIMESET_ONE
  #define CUTOFF_POINT            24U
  #define DECAY_COUNTS            5U      // Sets the number of systick counts per decay step of the note(s)
  #define SECOND_NOTE_THRESHOLD   680U    // Threshold of bing note to trigger the bong note.
  #define THIRD_NOTE_THRESHOLD    680U    // Threshold of bong note to trigger the dong note.
  #define PH1_STEP                48      // Offset for phase accumulator 1 (Bing note).
  #define PH2_STEP                32      // Offset for phase accumulator 2 (Bong note).
  #define PH3_STEP                24      // Offset for phase accumulator 3 (Dong note).
  #define DROP_RATE               2U      // Rate at which to drop off notes.
  #define NOTE1_VOL               1024U   // Initial volume of note 1
  #define NOTE2_VOL               1024U   // Initial volume of note 2
  #define NOTE3_VOL               1024U   // Initial volume of note 3
#endif

#ifdef CHIMESET_TWO
  #define CUTOFF_POINT            24U
  #define DECAY_COUNTS            3U      // Sets the number of systick counts per decay step of the note(s)
  #define SECOND_NOTE_THRESHOLD   710U    // Threshold of bing note to trigger the bong note.
  #define THIRD_NOTE_THRESHOLD    710U    // Threshold of bong note to trigger the dong note.
  #define PH1_STEP                60U     // Offset for phase accumulator 1 (Bing note).
  #define PH2_STEP                50U     // Offset for phase accumulator 2 (Bong note).
  #define PH3_STEP                40U     // Offset for phase accumulator 3 (Dong note).
  #define DROP_RATE               2U      // Rate at which to drop off notes.
  #define NOTE1_VOL               1024U   // Initial volume of note 1
  #define NOTE2_VOL               1024U   // Initial volume of note 2
  #define NOTE3_VOL               1024U   // Initial volume of note 3
#endif

#ifdef CHIMESET_THREE
  #define CUTOFF_POINT            40U
  #define DECAY_COUNTS            5U      // Sets the number of systick counts per decay step of the note(s)
  #define SECOND_NOTE_THRESHOLD   800U    // Threshold of bing note to trigger the bong note.
  #define THIRD_NOTE_THRESHOLD    800U    // Threshold of bong note to trigger the dong note.
  #define PH1_STEP                32U     // Offset for phase accumulator 1 (Bing note).
  #define PH2_STEP                48U     // Offset for phase accumulator 2 (Bong note).
  #define PH3_STEP                64U     // Offset for phase accumulator 3 (Dong note).
  #define DROP_RATE               2U      // Rate at which to drop off notes.
  #define NOTE1_VOL               1024U   // Initial volume of note 1
  #define NOTE2_VOL               1024U   // Initial volume of note 2
  #define NOTE3_VOL               1024U   // Initial volume of note 3
#endif


#include <stdint.h>

#ifdef DEBUG_MODE
  #include <cross_studio_io.h>
#endif

#endif
