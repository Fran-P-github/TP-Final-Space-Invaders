#include<time.h>
#include<stdbool.h>

#include"back.h"

#define ALIENS_DX ( (ALIENS_W + ALIENS_HORIZONTAL_SEPARATION) / 2 )
#define ALIENS_DY ( (ALIENS_H + ALIENS_VERTICAL_SEPARATION) / 2 )

#define PLAYER_DX ( PLAYER_W / 2 )

#define SHOT_DY 10

static player_t player;

static shield_t shields[SHIELDS_CANT];

static alien_t aliens[ALIENS_ROWS][ALIENS_COLUMNS];
static double aliens_move_interval = 0.01; // Seconds

// Player and Aliens can have only one active shot at a time
static shot_t player_shot;
static shot_t alien_shot;

shield_t (*get_shields(void)) [SHIELDS_CANT]{ return &shields; }
player_t* get_player(){ return &player; }
alien_t (*get_aliens(void)) [ALIENS_ROWS][ALIENS_COLUMNS]{ return &aliens; }
double* get_aliens_move_interval(){ return &aliens_move_interval; }
shot_t* get_player_shot(){ return &player_shot; }
shot_t* get_alien_shot(){ return &alien_shot; }

static void shield_init(unsigned k, int x, int y);
static void player_shot_update();
static void alien_shot_update();
static bool collide(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2);

static bool collide(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2){
    if(ax1 > bx2) return false;
    if(ax2 < bx1) return false;
    if(ay1 > by2) return false;
    if(ay2 < by1) return false;

    return true;
}

void shields_init(){
    unsigned i;
    for(i=0; i<SHIELDS_CANT; ++i){
        int x = (i+1) * WORLD_WIDTH/(SHIELDS_CANT+1) - SHIELD_W*SHIELD_BLOCK_W/2;
        int y = WORLD_HEIGHT - PLAYER_MARGIN - PLAYER_H - SHIELD_TO_PLAYER_MARGIN - SHIELD_H*SHIELD_BLOCK_H;
        shield_init(i, x, y);
    }
}

static void shield_init(unsigned k, int x, int y){
    char form[SHIELD_H][SHIELD_W] = SHIELD_FORM;
    unsigned i, j;
    for(i=0; i<SHIELD_H; ++i){
        for(j=0; j<SHIELD_W; ++j){
            shields[k][i][j].x = x + j * SHIELD_BLOCK_W;
            shields[k][i][j].y = y + i * SHIELD_BLOCK_H;
            shields[k][i][j].lives = (form[i][j] == '*') ? SHIELD_BLOCK_LIVES : 0;
        }
    }
}

#define INITIAL_PLAYER_X_COORDINATE ( (WORLD_WIDTH - PLAYER_W) / 2 )
void player_init(){
    player.x = INITIAL_PLAYER_X_COORDINATE;
    player.y = WORLD_HEIGHT - PLAYER_MARGIN - PLAYER_H;
    player.lives = PLAYER_INITIAL_LIVES;
    player_shot.is_used = false;
    player.score = 0;
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
            aliens[i][j].points = 2;

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

// Returns: index of the lowest alien alive in the column, or -1 if no aliens are alive
#define lowest_alien_alive_index(c) ( aliens_alive_in_column(c) - 1 )

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
    player_shot_update();
    alien_shot_update();
}

static void player_shot_update(){
    if(player_shot.is_used){
        player_shot.y -= SHOT_DY;
        unsigned i, j;
        for(i=0; i<ALIENS_ROWS && player_shot.is_used; ++i){
            for(j=0; j<ALIENS_COLUMNS; ++j){
                if(aliens[i][j].is_alive && collide(player_shot.x, player_shot.y, player_shot.x+SHOT_W, player_shot.y+SHOT_H, aliens[i][j].x, aliens[i][j].y, aliens[i][j].x+ALIENS_W, aliens[i][j].y+ALIENS_H)){
                    player_shot.is_used = false;
                    aliens[i][j].is_alive = false;
                    player.score += aliens[i][j].points;
                    break;
                }
            }
        }
    }
}

static void alien_shot_update(){
    if(alien_shot.is_used){
        alien_shot.y += SHOT_DY;
        if(collide(alien_shot.x, alien_shot.y, alien_shot.x+SHOT_W, alien_shot.y+SHOT_H, player.x, player.y, player.x+PLAYER_W, player.y+PLAYER_H)){
            alien_shot.is_used = false;
            player.lives--;
        }
    }
}
