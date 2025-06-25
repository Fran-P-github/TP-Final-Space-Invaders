#ifndef _BACK_H_
#define _BACK_H_

#include<stdbool.h>

#include"general_defines.h"

#define ALL_ALIENS_WIDTH ( ALIENS_COLUMNS*ALIENS_W + (ALIENS_COLUMNS-1)*ALIENS_HORIZONTAL_SEPARATION )

typedef struct{
    int x, y;
    bool is_alive;
} alien_t;

typedef struct{
    int x, y;
    int lives;
} player_t;

extern alien_t aliens[ALIENS_ROWS][ALIENS_COLUMNS];
extern double aliens_move_interval;

extern player_t player;

void aliens_init();
void aliens_update_position();

void player_init();
void player_move_right();
void player_move_left();

#endif // _BACK_H_
