#include<time.h>
#include<stdbool.h>

#include"back.h"

alien_t aliens[ALIENS_ROWS][ALIENS_COLUMNS];
double aliens_move_interval = 0.01; // Segundos

#define ALIENS_DX ( (ALIENS_W + ALIENS_HORIZONTAL_SEPARATION) / 2 )
#define ALIENS_DY ( (ALIENS_H + ALIENS_VERTICAL_SEPARATION) / 2 )

#define FIRST_ALIEN_X_COORDINATE ( (WORLD_WIDTH - ALL_ALIENS_WIDTH) / 2 )
void aliens_init(){
    unsigned i, j;
    int x = FIRST_ALIEN_X_COORDINATE;
    int y = 20;
    for(i=0; i<ALIENS_ROWS; ++i){
        for(j=0; j<ALIENS_COLUMNS; ++j){
            aliens[i][j].x = x;
            aliens[i][j].y = y;
            aliens[i][j].is_alive = true;

            x += ALIENS_W + ALIENS_HORIZONTAL_SEPARATION;
        }
        y += ALIENS_H + ALIENS_VERTICAL_SEPARATION;
        x = FIRST_ALIEN_X_COORDINATE;
    }
}

static void aliens_move(int x, int y){
    unsigned i, j;
    for(i=0; i<ALIENS_ROWS; ++i){
        for(j=0; j<ALIENS_COLUMNS; ++j){
            aliens[i][j].x += x;
            aliens[i][j].y += y;
        }
    }
}

static void aliens_move_right(){
    aliens_move(ALIENS_DX, 0);
}
static void aliens_move_left(){
    aliens_move(-ALIENS_DX, 0);
}
static void aliens_move_down(){
    aliens_move(0, ALIENS_DY);
}

static bool alien_alive_in_column(unsigned c){
    if(c >= ALIENS_COLUMNS) return false;
    unsigned i;
    for(i=0; i<ALIENS_ROWS; ++i){
        if(aliens[i][c].is_alive) return true;
    }
    return false;
}

typedef enum movement{
    MOVEMENT_RIGHT=0,
    MOVEMENT_LEFT
} movement_t;
void aliens_update_position(){
    static clock_t start = 0;
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    static movement_t movement = MOVEMENT_RIGHT;

    if(elapsed >= aliens_move_interval){
        start = clock();

        unsigned i;
        switch(movement){
            case MOVEMENT_RIGHT:
                aliens_move_right();
                for(i=ALIENS_COLUMNS-1; i<ALIENS_COLUMNS; --i){ // Comparación así por i unsigned
                    if(alien_alive_in_column(i)
                    && (aliens[0][i].x+ALIENS_W > WORLD_WIDTH-1)){
                        aliens_move_left(); // Nos habíamos pasado
                        aliens_move_down();
                        movement = MOVEMENT_LEFT;
                        break;
                    }
                }
                break;
            case MOVEMENT_LEFT:
                aliens_move_left();
                for(i=0; i<ALIENS_COLUMNS; ++i){
                    if(alien_alive_in_column(i)
                    && (aliens[0][i].x < 0)){
                        aliens_move_right(); // Nos habíamos pasado
                        aliens_move_down();
                        movement = MOVEMENT_RIGHT;
                        break;
                    }
                }
                break;
        }
    }
}
