/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     front.h

     Description:
     Funtions to call from main.c for running the game. Functions are the same for RPI and PC
     Functions will return game_state_t enum values for main.c to know what to call next
     Game ends when any function returns CLOSED

  ******************************************************************************/
 
#ifndef _FRONT_H_
#define _FRONT_H_

#include <stdbool.h>
#include "general_defines.h"

// To be called once before any other function. Returns initial state of the game (MENU)
game_state_t front_init();

// To be called after a function returning MENU
game_state_t menu();

// To be called after a function returning GAME
// Receives: curent_level (starting from level 0) and new_level (true when starting a new level)
game_state_t game_update(unsigned int curent_level, bool new_level);

// To be called after a function returning PAUSE
// Receives: curent_level (starting from level 0)
game_state_t game_pause(unsigned int* curent_level, bool* new_level);

// To be called after a function returning ENGAME
game_state_t endgame();

// To be called before ending the program
void front_deinit();

#endif // _FRONT_H_
