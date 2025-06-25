#include<time.h>
#include<stdbool.h>

#include"back.h"

#define ALIENS_DX ( (ALIENS_W + ALIENS_HORIZONTAL_SEPARATION) / 2 )
#define ALIENS_DY ( (ALIENS_H + ALIENS_VERTICAL_SEPARATION) / 2 )

#define PLAYER_DX ( PLAYER_W / 2 )

#define SHOT_DY 10

player_t player;

alien_t aliens[ALIENS_ROWS][ALIENS_COLUMNS];
double aliens_move_interval = 0.01; // Seconds

// Player and Aliens can have only one active shot at a time
shot_t player_shot;
shot_t alien_shot;

#define INITIAL_PLAYER_X_COORDINATE ( (WORLD_WIDTH - PLAYER_W) / 2 )
void player_init(){
    player.x = INITIAL_PLAYER_X_COORDINATE;
    player.y = WORLD_HEIGHT - PLAYER_MARGIN - PLAYER_H;
    player.lives = PLAYER_INITIAL_LIVES;
    player_shot.is_used = false;
}

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
    alien_shot.is_used = false;
}

static void player_move(int x, int y){
    player.x += x;
    player.y += y;
}

void player_move_right(){
    if(player.x + PLAYER_DX + PLAYER_W <= WORLD_WIDTH - PLAYER_MARGIN)
        player_move(PLAYER_DX, 0);
}
void player_move_left(){
    if(player.x - PLAYER_DX >= PLAYER_MARGIN)
    player_move(-PLAYER_DX, 0);
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

// Returns: how many aliens are alive in column c
static unsigned aliens_alive_in_column(unsigned c){
    if(c >= ALIENS_COLUMNS) return 0;
    unsigned i;
    for(i=ALIENS_ROWS-1; i<ALIENS_ROWS; --i){
        if(aliens[i][c].is_alive) return i+1;
    }
    return 0;
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
                    if(aliens_alive_in_column(i)
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
                    if(aliens_alive_in_column(i)
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

bool player_try_shoot(){
    if(player_shot.is_used) return false;

    player_shot.is_used = true;
    player_shot.x = player.x + PLAYER_W/2 - SHOT_W/2;
    player_shot.y = player.y - SHOT_H;
    return true;
}

// c = columna de aliens que quiere disparar
bool alien_try_shoot(unsigned c){
    unsigned alive_aliens = aliens_alive_in_column(c);
    if(alien_shot.is_used || !alive_aliens) return false;

    alien_shot.is_used = true;
    alien_shot.x = aliens[0][c].x + ALIENS_W/2 - SHOT_W/2;
    alien_shot.y = aliens[alive_aliens-1][c].y + ALIENS_H;
    return true;
}

void shots_update(){
    if(player_shot.is_used){
        player_shot.y -= SHOT_DY;
    }

    if(alien_shot.is_used){
        alien_shot.y += SHOT_DY;
    }
}
