#ifndef _BACK_H_
#define _BACK_H_

#include<stdbool.h>

#include"general_defines.h"

#define ALL_ALIENS_WIDTH ( ALIENS_COLUMNS*ALIENS_W + (ALIENS_COLUMNS-1)*ALIENS_HORIZONTAL_SEPARATION )

typedef struct alien{
    int x, y;
    bool is_alive;
} alien_t;

extern alien_t aliens[ALIENS_ROWS][ALIENS_COLUMNS];
extern double aliens_move_interval;

void aliens_init();
void aliens_update_position();

#endif // _BACK_H_
