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

typedef struct{
    int x, y;
    int dy;
    int is_used;
} shot_t;

player_t* get_player();
alien_t (*get_aliens(void)) [ALIENS_ROWS][ALIENS_COLUMNS];
double* get_aliens_move_interval();
shot_t* get_player_shot();
shot_t* get_alien_shot();

void aliens_init();
void aliens_update_position();

void player_init();
void player_move_right();
void player_move_left();

void shots_update();
bool player_try_shoot();
bool alien_try_shoot(unsigned column);

#endif // _BACK_H_
