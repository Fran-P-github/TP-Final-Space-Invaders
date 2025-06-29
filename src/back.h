/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     front_allegro.h
     25/6/25

     Description:



  ******************************************************************************/

#ifndef _BACK_H_
#define _BACK_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include<stdbool.h>

#include"general_defines.h"

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

typedef enum{ LEVEL_NOT_DONE=0, PLAYER_WINS, ALIENS_WIN } level_state_t;

void back_init();

// First level is level 0
level_state_t back_update(unsigned int current_level);

// aliens_rows/columns: number of aliens to spawn. ALIENS_ROWS/COLUMNS assumed if a grater number is given
// For aliens_lives, 1 is assumed if 0 is given
void level_init(unsigned int aliens_rows, unsigned int aliens_cols, unsigned aliens_lives, unsigned shield_block_lives);

int player_get_x();
int player_get_y();
int player_get_score();
int player_shot_get_x();
int player_shot_get_y();
bool player_shot_is_used();

int mothership_get_x();
int mothership_get_y();
int aliens_get_x(unsigned i, unsigned j);
int aliens_get_y(unsigned i, unsigned j);
bool aliens_is_alive(unsigned i, unsigned j);
int alien_shot_get_x();
int alien_shot_get_y();
bool alien_shot_is_used();
double aliens_get_move_interval();
void aliens_set_move_interval(double interval);
unsigned total_aliens_alive();

int shield_get_x(unsigned shield, unsigned block_y, unsigned block_x);
int shield_get_y(unsigned shield, unsigned block_y, unsigned block_x);
int shield_get_lives(unsigned shield, unsigned block_y, unsigned block_x);

bool mothership_is_active();

void player_move_right();
void player_move_left();

bool player_try_shoot();
bool alien_try_shoot(unsigned column);

#endif // _BACK_H_
