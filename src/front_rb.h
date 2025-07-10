#ifndef _FRONT_RB_H_
#define _FRONT_RB_H_

#include <stdbool.h>
#include "general_defines.h"

game_state_t front_init();

game_state_t menu();

game_state_t game_update(unsigned int curent_level, bool new_level);

game_state_t game_pause(unsigned int curent_level);

void endgame();

#endif // _FRONT_RB_H_
