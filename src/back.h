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

typedef enum{ LEVEL_NOT_DONE=0, PLAYER_WINS, ALIENS_WIN } level_state_t;

typedef enum{
  ALIEN_FAT=0,
  ALIEN_WINGED,
  ALIEN_SMALL
} alien_type_t;

typedef enum {
  ALIEN_EXPLOSION = 0,
  UFO_EXPLOSION
} explosion_type_t;

/*******************************************************************************
 * EXTERN VARIABLES
 ******************************************************************************/

// Not to be added in back.c
#ifndef _IS_BACK_C_
extern const bool aliensMoved;
extern const bool playerDied;
extern const bool shieldWasHit;
#endif

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

unsigned long long get_millis();
int rand_between(int lo, int hi);
float rand_between_f(float lo, float hi);

void back_init();

// First level is level 0
level_state_t back_update(unsigned int current_level, void (*alienDeath)(int x, int y, explosion_type_t explosionType), void (*alienHit)(void));

// aliens_rows/columns: number of aliens to spawn. ALIENS_ROWS/COLUMNS assumed if a grater number is given
// For aliens_lives, 1 is assumed if 0 is given
void level_init(unsigned current_level, unsigned int aliens_rows, unsigned int aliens_cols, unsigned aliens_lives_min, unsigned aliens_lives_max, unsigned shield_block_lives);

void player_reset_on_new_level();
void player_reset_on_new_game();
int player_get_x();
int player_get_y();
int player_get_lives();
int player_get_score();
int player_shot_get_x();
int player_shot_get_y();
bool player_shot_is_used();

int mothership_get_x();
int mothership_get_y();

int aliens_get_x(unsigned i, unsigned j);
int aliens_get_y(unsigned i, unsigned j);
int aliens_get_points(unsigned i, unsigned j);
bool aliens_is_alive(unsigned i, unsigned j);
int aliens_get_lives(unsigned i, unsigned j);
alien_type_t alines_get_type(unsigned i, unsigned j);

int alien_shot_get_x();
int alien_shot_get_y();
bool alien_shot_is_used();
float aliens_get_relative_speed();
int get_best_alien_column_to_shoot(); // Returns column above player, or column above some shield block if no aliens are above player, or -1 if aliens wait to shoot
unsigned total_aliens_alive();

int shield_get_x(unsigned shield, unsigned block_y, unsigned block_x);
int shield_get_y(unsigned shield, unsigned block_y, unsigned block_x);
int shield_get_lives(unsigned shield, unsigned block_y, unsigned block_x);

bool mothership_is_active();
int mothership_get_points();

void player_move_right();
void player_move_left();

bool player_try_shoot();
bool alien_try_shoot(unsigned column);

#endif // _BACK_H_
