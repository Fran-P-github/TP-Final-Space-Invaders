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

typedef struct{
    int x, y;
    bool is_alive;
    int points; // Point given to player when killed
} alien_t;

typedef struct{
    double x, y;
    int lives;
    int score;
} player_t;

struct shield{
    int x, y;
    int lives; // Shots that a block of the shield can resist
};
typedef struct shield shield_t[SHIELD_H][SHIELD_W];

typedef struct{
    int x, y;
    int dy;
    int is_used;
} shot_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// Functions to get typedefs instances
player_t* get_player();
alien_t (*get_aliens(void)) [ALIENS_ROWS][ALIENS_COLUMNS];
double* get_aliens_move_interval();
shot_t* get_player_shot();
shot_t* get_alien_shot();
shield_t (*get_shields(void)) [SHIELDS_CANT];

void aliens_init();
bool aliens_update(); // Returns: true if aliens reach player height, false otherwise

void player_init();
void player_move_right();
void player_move_left();

void shots_update();
bool player_try_shoot();
bool alien_try_shoot(unsigned column);

void shields_init();

#endif // _BACK_H_
