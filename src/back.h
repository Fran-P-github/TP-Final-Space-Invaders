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
  NO_EXPLOSION=0,
  ALIEN_EXPLOSION,
  UFO_EXPLOSION
} explosion_type_t;
typedef struct{
  int x;
  int y;
  explosion_type_t type;
}explosion_t;

/*******************************************************************************
 * EXTERN VARIABLES
 ******************************************************************************/

// Not to be added in back.c
// All this variables are indicators the game status
#ifndef _IS_BACK_C_

// True after a frame in which any aliens moved
extern const bool alienWasHit;
// True after a frame in which alien or mothership is killed
extern const bool alienWasKilled;
// True after a frame in which an alien was hit
extern const bool aliensMoved;
// True after a frame in which the player was killed. Remains true for the duration of the player death sound
extern const bool playerDied;
// True after a frame in which a shield block hit
extern const bool shieldWasHit;

#endif

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// Returns milliseconds elapsed since a moment in time. Similar to Arduino millis() function
unsigned long long get_millis();

void shuffle(unsigned *array, unsigned n);
int rand_between(int lo, int hi);
float rand_between_f(float lo, float hi);

// To be called once before any of the following functions
void back_init();

// To be called once every frame. Recieves current level (first level is level 0) and new_level (must be true olnly for the first frame of a new level)
level_state_t back_update(unsigned int current_level, bool new_level);

// To be called and the beginning of a level, with level specific configuration
// current_level: level in progress, first level is level 0
// aliens_rows/columns: number of aliens to spawn. ALIENS_ROWS/COLUMNS assumed if a grater number is given
// aliens_lives_min/max: aliens in top row have aliens_lives_min, aliens in bottom row have aliens_lives_max. Linear Progression.
//                       1 assumed for aliens_lives_min if 0 is given. aliens_lives_min assumed for aliens_lives_max if aliens_lives_max < aliens_lives_min
// shield_block_lives: shots a block in the shield can resist
void level_init(unsigned current_level, unsigned int aliens_rows, unsigned int aliens_cols, unsigned aliens_lives_min, unsigned aliens_lives_max, unsigned shield_block_lives);

// To be called at the beginning of a new game
void player_reset_on_new_game();

// Game control functions

void player_move_right();
void player_move_left();
// Returns true if player was able to shoot
bool player_try_shoot();
// Returns: true if alien in column was able to shoot
bool alien_try_shoot(unsigned column);

// Copies info for explosion to be drawn to explosion_state if there was an explosion during last frame
// Returns false when not copying or if NULL pointer is given
bool get_explosion_state(explosion_t* explosion_state);

// Player status functions

int player_get_x();
int player_get_y();
int player_get_lives();
int player_get_score();

// Player shot status functions

int player_shot_get_x();
int player_shot_get_y();
bool player_shot_is_used();

// Mothership status functions

int mothership_get_x();
int mothership_get_y();
bool mothership_is_active();
int mothership_get_points();

// Aliens status functions

int aliens_get_x(unsigned i, unsigned j);
int aliens_get_y(unsigned i, unsigned j);
int aliens_get_points(unsigned i, unsigned j);
bool aliens_is_alive(unsigned i, unsigned j);
int aliens_get_lives(unsigned i, unsigned j);
alien_type_t alines_get_type(unsigned i, unsigned j);

// Aliens shot status functions

int alien_shot_get_x();
int alien_shot_get_y();
bool alien_shot_is_used();
// Float in [0, 1]. The closer aliens speed is to the max, the closer to 1
float aliens_get_relative_speed();
// Returns column above player, or column above some shield block if no aliens are above player, or -1 if aliens wait to shoot
int get_best_alien_column_to_shoot(); 
unsigned total_aliens_alive();

// Shield status functions

int shield_get_x(unsigned shield, unsigned block_y, unsigned block_x);
int shield_get_y(unsigned shield, unsigned block_y, unsigned block_x);
int shield_get_lives(unsigned shield, unsigned block_y, unsigned block_x);

#endif // _BACK_H_
