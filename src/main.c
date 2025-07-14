/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     main.c

     Description:
     Manages the main loop and controls scene transitions by calling the appropriate functions from the front module.
     Each scene corresponds to a game_state_t value, as defined in front.h.


  ******************************************************************************/

#include "front.h"

int main() {
  game_state_t state = front_init();

  state = MENU; // Testing. Despues cambiar a MENU

  unsigned int level = 0;
  bool new_level = true;

  while ( state != CLOSED ) {
    switch ( state ) {
      case MENU:
        level = 0;
        new_level = true;
        state = menu();
        break;
      case GAME:
        state = game_update(level, new_level);
        if ( state == GAME ) {
          level++; // Player won
          new_level = true;
        } else {
          new_level = false;
        }
        break;
      case PAUSE:
        state = game_pause(&level, &new_level);
        break;
      case ENDGAME:
        state = endgame();
        level = 0;
        new_level = true;
        break;
      case CLOSED:
        break;
    }
  }

  front_deinit();

  return 0;
}
