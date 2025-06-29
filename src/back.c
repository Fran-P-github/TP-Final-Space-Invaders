/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     front_allegro.c
     25/6/25

     Description:



  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include<time.h>
#include<stdbool.h>
#include<stdlib.h>

#include"back.h"

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define ALIENS_MOVE_MIN_INTERVAL 0.05
#define ALIENS_MOVE_MAX_INTERVAL 0.5

#if PLATFORM == ALLEGRO

#define MOTHERSHIP_X_VELOCITY 250
#define MOTHERSHIP_DX ( MOTHERSHIP_X_VELOCITY / FRAME_RATE )

#define ALIENS_X_VELOCITY 500//( (ALIENS_W + ALIENS_HORIZONTAL_SEPARATION) / 2 )
#define ALIENS_Y_VELOCITY 500//( (ALIENS_H + ALIENS_VERTICAL_SEPARATION) / 2 )
#define ALIENS_DX (ALIENS_X_VELOCITY / FRAME_RATE)
#define ALIENS_DY (ALIENS_Y_VELOCITY / FRAME_RATE)

#define PLAYER_VELOCITY 200//( PLAYER_W / 2 )
#define PLAYER_DX (PLAYER_VELOCITY / FRAME_RATE)

//#define SHOT_DY ( SHOT_W / 2 )
#define SHOT_VELOCITY_ALIEN 200
#define SHOT_VELOCITY_PLAYER 200
#define SHOT_DY_ALIEN (SHOT_VELOCITY_ALIEN / FRAME_RATE)
#define SHOT_DY_PLAYER (SHOT_VELOCITY_PLAYER / FRAME_RATE)

#elif PLATFORM == RPI

#define MOTHERSHIP_DX 1

#define ALIENS_DX 1
#define ALIENS_DY 1

#define PLAYER_DX 1

#define SHOT_DY_ALIEN 1
#define SHOT_DY_PLAYER 1

#endif

#define ALL_ALIENS_WIDTH ( ALIENS_COLUMNS*ALIENS_W + (ALIENS_COLUMNS-1)*ALIENS_HORIZONTAL_SEPARATION )

// VARIABLE GLOBAL Y PUBLICA

bool aliensMoved; // Variable para reproducir el sonido cuando se mueven los aliens

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct{
    int x, y;
    bool is_alive;
    int points; // Point given to player when killed
} alien_t;

typedef struct{
    int x, y;
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
    bool is_used;
} shot_t;

typedef enum movement{
    MOVEMENT_RIGHT=0,
    MOVEMENT_LEFT
} movement_t;

typedef struct{
    int x, y;
    int dx;
    bool is_active;
    int points; // Point given to player when shot
} mothership_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static bool aliens_update();
static void mothership_update();
static void shots_update();

// Inits shield in given coordinates
static void shield_init(unsigned shield, int x, int y);

static int rand_between(int lo, int hi);

// Detects collition between a and b
static bool collide(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2);

static bool should_spawn_mothership(double elapsed_time);

// Call for shots to update
static void player_shot_update();
static void alien_shot_update();

// Move player
static void player_move(int x, int y);

// Move alien and wrappers
static void aliens_move(int x, int y);
static void aliens_move_right();
static void aliens_move_left();
static void aliens_move_down();

static void aliens_update_position();
static void update_aliens_speed();
void aliens_shield_collition();

static unsigned total_aliens_alive();

// Returns: how many aliens are alive in column c
static unsigned aliens_alive_in_column(unsigned c);

// Returns: how many aliens are alive in row r
static unsigned aliens_alive_in_row(unsigned r);

// Returns: index of the lowest alien alive in the column, or -1 if no aliens are alive
#define lowest_alien_alive_index(c) ( aliens_alive_in_column(c) - 1 )

// Returns: index of the lowest alien alive in the column, or -1 if no aliens are alive
#define lowest_alien_alive_index(c) ( aliens_alive_in_column(c) - 1 )

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static player_t player;

static shield_t shields[SHIELDS_CANT];

static alien_t aliens[ALIENS_ROWS][ALIENS_COLUMNS];
static double aliens_move_interval; // Seconds. Time in between aliens movements

// Player and Aliens can have only one active shot at a time
static shot_t player_shot = {.is_used = true};
static shot_t alien_shot;

static mothership_t mothership;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

int mothership_get_x(){ return mothership.x; }
int mothership_get_y(){ return mothership.y; }
bool mothership_is_active(){ return mothership.is_active; }

int player_get_x(){ return player.x; }
int player_get_y(){ return player.y; }
int player_get_score(){ return player.score; }
int player_shot_get_x(){ return player_shot.x; }
int player_shot_get_y(){ return player_shot.y; }
bool player_shot_is_used(){ return player_shot.is_used; }

int aliens_get_x(unsigned i, unsigned j){ return aliens[i][j].x; }
int aliens_get_y(unsigned i, unsigned j){ return aliens[i][j].y; }
bool aliens_is_alive(unsigned i, unsigned j){ return aliens[i][j].is_alive; }
double aliens_get_move_interval(){ return aliens_move_interval; }
void aliens_set_move_interval(double interval){ aliens_move_interval = interval; }
int alien_shot_get_x(){ return alien_shot.x; }
int alien_shot_get_y(){ return alien_shot.y; }
bool alien_shot_is_used(){ return alien_shot.is_used; }

int shield_get_x(unsigned s, unsigned y, unsigned x){ return shields[s][y][x].x; }
int shield_get_y(unsigned s, unsigned y, unsigned x){ return shields[s][y][x].y; }
int shield_get_lives(unsigned s, unsigned y, unsigned x){ return shields[s][y][x].lives; }

shield_t (*get_shields(void)) [SHIELDS_CANT]{ return &shields; }
player_t* get_player(){ return &player; }
alien_t (*get_aliens(void)) [ALIENS_ROWS][ALIENS_COLUMNS]{ return &aliens; }
double* get_aliens_move_interval(){ return &aliens_move_interval; }

#define INITIAL_SHIELD_Y_COORDINATE     WORLD_HEIGHT - PLAYER_MARGIN - PLAYER_H - SHIELD_TO_PLAYER_MARGIN - SHIELD_H*SHIELD_BLOCK_H
void shields_init(){
    unsigned i;
    for(i=0; i<SHIELDS_CANT; ++i){
        int x = (i+1) * WORLD_WIDTH/(SHIELDS_CANT+1) - SHIELD_W*SHIELD_BLOCK_W/2;
        int y = INITIAL_SHIELD_Y_COORDINATE;
        shield_init(i, x, y);
    }
}

#define INITIAL_PLAYER_X_COORDINATE     ( (WORLD_WIDTH - PLAYER_W) / 2 )
void player_init(){
    player.x = INITIAL_PLAYER_X_COORDINATE;
    player.y = WORLD_HEIGHT - PLAYER_MARGIN - PLAYER_H;
    player.lives = PLAYER_INITIAL_LIVES;
    player_shot.is_used = false;
    player.score = 0;
}

#define FIRST_ALIEN_X_COORDINATE  ( (WORLD_WIDTH - ALL_ALIENS_WIDTH) / 2 )
void aliens_init(){
    unsigned i, j;
    int x = FIRST_ALIEN_X_COORDINATE;
    int y = ALIENS_MARGIN;
    for(i=0; i<ALIENS_ROWS; ++i){
        for(j=0; j<ALIENS_COLUMNS; ++j){
            aliens[i][j].x = x;
            aliens[i][j].y = y;
            aliens[i][j].is_alive = true;
            aliens[i][j].points = ALIENS_POINTS;

            x += ALIENS_W + ALIENS_HORIZONTAL_SEPARATION;
        }
        y += ALIENS_H + ALIENS_VERTICAL_SEPARATION;
        x = FIRST_ALIEN_X_COORDINATE;
    }
    alien_shot.is_used = false;
}

void player_move_right(){
    if(player.x + PLAYER_DX + PLAYER_W <= WORLD_WIDTH - PLAYER_MARGIN)
        player_move(PLAYER_DX, 0);
}
void player_move_left(){
    if(player.x - PLAYER_DX >= PLAYER_MARGIN)
    player_move(-PLAYER_DX, 0);
}

level_state_t back_update(unsigned current_level){
    shots_update();
    mothership_update();
    if(aliens_update(current_level)) return ALIENS_WIN;
    if(total_aliens_alive() == 0) return PLAYER_WINS;

    return LEVEL_NOT_DONE;
}

// Returns: true if shot was available when called, false otherwise
bool player_try_shoot(){
    if(player_shot.is_used) return false;

    player_shot.is_used = true;
    player_shot.x = player.x + PLAYER_W/2 - SHOT_W/2;
    player_shot.y = player.y - SHOT_H;
    return true;
}

// c = aliens column trying to shoot
// Returns: true if shot was available when called, false otherwise
bool alien_try_shoot(unsigned c){
    unsigned alive_aliens = aliens_alive_in_column(c);
    if(alien_shot.is_used || !alive_aliens) return false;

    alien_shot.is_used = true;
    alien_shot.x = aliens[0][c].x + ALIENS_W/2 - SHOT_W/2;
    alien_shot.y = aliens[alive_aliens-1][c].y + ALIENS_H;
    return true;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

// Returns: true if aliens win (reach the bottom of the screen)
static bool aliens_update(unsigned current_level){
    aliens_update_position();
    update_aliens_speed(current_level);
    aliens_shield_collition();

    unsigned i;
    for(i=ALIENS_ROWS-1; i<ALIENS_ROWS; --i){
        if(aliens_alive_in_row(i) && aliens[i][0].y+ALIENS_H-1 >= player.y) return true;
    }

    return false;
}

static void shots_update(){
    player_shot_update();
    alien_shot_update();
}

static void mothership_update(){
    static clock_t start = 0;
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    if(!mothership.is_active && !should_spawn_mothership(elapsed)) return; // Mothership inactive and not activated yet
    start = clock();

    static bool spawn_right;

    if(!mothership.is_active){
        spawn_right = rand()%2;

        mothership.is_active = true;
        mothership.points = 5 * rand_between(2, 16); // 10 to 80, 5 steps
        mothership.y = MOTHERSHIP_MARGIN;
        if(spawn_right){
            mothership.x = WORLD_WIDTH;
        }else{
            mothership.x = -MOTHERSHIP_W;
        }
    }

    if(spawn_right){
        if(mothership.x+MOTHERSHIP_W-1 < 0){
            mothership.is_active = false;
        }
        mothership.x -= MOTHERSHIP_DX;
    }
    else{
        if(mothership.x > WORLD_WIDTH-1){
            mothership.is_active = false;
        }
        mothership.x += MOTHERSHIP_DX;
    }
}

static int rand_between(int lo, int hi){
    return lo + rand() % (hi - lo + 1);
}

static bool collide(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2){
    if(ax1 > bx2) return false;
    if(ax2 < bx1) return false;
    if(ay1 > by2) return false;
    if(ay2 < by1) return false;

    return true;
}

static bool should_spawn_mothership(double elapsed_time){
    const double max_prob = 0.5; // 50% max
    const double rate = 0.00005;    // 0.005% increase per second

    double probability = elapsed_time * rate;

    // Limita al máximo
    if (probability > max_prob)
        probability = max_prob;

    // Genera número entre 0 y 1
    double r = (double)rand() / RAND_MAX;

    return r < probability;
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

static void player_move(int x, int y){
    player.x += x;
    player.y += y;
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

static unsigned total_aliens_alive(){
    unsigned count = 0;
    for(unsigned i = 0; i < ALIENS_ROWS; ++i){
        for(unsigned j = 0; j < ALIENS_COLUMNS; ++j){
            if(aliens[i][j].is_alive)
                ++count;
        }
    }
    return count;
}

static unsigned aliens_alive_in_column(unsigned c){
    if(c >= ALIENS_COLUMNS) return 0;
    unsigned i;
    for(i=ALIENS_ROWS-1; i<ALIENS_ROWS; --i){
        if(aliens[i][c].is_alive) return i+1;
    }
    return 0;
}

static unsigned aliens_alive_in_row(unsigned r){
    if(r >= ALIENS_ROWS) return 0;
    unsigned rta = 0;
    unsigned i;
    for(i=0; i<ALIENS_COLUMNS; ++i){
        if(aliens[r][i].is_alive) ++rta;
    }
    return rta;
}

// First level is level 0
static void update_aliens_speed(unsigned level){

    unsigned total = ALIENS_ROWS * ALIENS_COLUMNS;
    unsigned alive = total_aliens_alive();

    double alive_ratio = (double)alive / total;

    // Cuanto menos aliens, más rápido. También aumenta con el nivel.
    aliens_move_interval = ALIENS_MOVE_MAX_INTERVAL * alive_ratio;

    // Aplicar escalado con el nivel (aumenta la velocidad base)
    aliens_move_interval /= (1 + 0.2 * level); // 20% más rápido por nivel

    // Limitar al mínimo
    if(aliens_move_interval < ALIENS_MOVE_MIN_INTERVAL)
        aliens_move_interval = ALIENS_MOVE_MIN_INTERVAL;
}


static void aliens_update_position(){
    static clock_t start = 0;
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    static movement_t movement = MOVEMENT_RIGHT;
    aliensMoved = elapsed >= aliens_move_interval; 
    if(elapsed >= aliens_move_interval){
        start = clock();

        unsigned i;
        switch(movement){
            case MOVEMENT_RIGHT:
                aliens_move_right();
                for(i=ALIENS_COLUMNS-1; i<ALIENS_COLUMNS; --i){ // Comparación así por i unsigned
                    if(aliens_alive_in_column(i)
                    && (aliens[0][i].x+ALIENS_W-1 > WORLD_WIDTH-1)){
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

void aliens_shield_collition(){
    // Shield collition
    unsigned shield, i, j;
    unsigned alien_row, alien_column;
    for(shield=0; shield<SHIELDS_CANT; ++shield){
        for(i=0; i<SHIELD_H; ++i){
            for(j=0; j<SHIELD_W; ++j){
                if(shields[shield][i][j].lives == 0)
                    continue;

                for(alien_row=0; alien_row<ALIENS_ROWS; ++alien_row){
                    for(alien_column=0; alien_column<ALIENS_COLUMNS; ++alien_column){
                        if(!aliens[alien_row][alien_column].is_alive)
                            continue;

                        if(collide(
                            aliens[alien_row][alien_column].x, aliens[alien_row][alien_column].y,
                            aliens[alien_row][alien_column].x + ALIENS_W - 1, aliens[alien_row][alien_column].y + ALIENS_H - 1,
                            shields[shield][i][j].x, shields[shield][i][j].y,
                            shields[shield][i][j].x + SHIELD_BLOCK_W - 1, shields[shield][i][j].y + SHIELD_BLOCK_H - 1)){

                            shields[shield][i][j].lives = 0;
                            goto next_block;  // Done with this block, check for next block
                        }
                    }
                }
            next_block:;
            }
        }
    }
}

static void player_shot_update(){
    if(!player_shot.is_used) return;

    player_shot.y -= SHOT_DY_PLAYER;
    unsigned i, j;

    // Mothership collition
    if(collide(player_shot.x, player_shot.y, player_shot.x+SHOT_W-1, player_shot.y+SHOT_H-1, mothership.x, mothership.y, mothership.x+MOTHERSHIP_W-1, mothership.y+MOTHERSHIP_H-1)){
        player_shot.is_used = false;
        player.score += mothership.points;
        mothership.is_active = false;
    }

    // Alien_shot collition
    if(collide(player_shot.x, player_shot.y, player_shot.x+SHOT_W-1, player_shot.y+SHOT_H-1, alien_shot.x, alien_shot.y, alien_shot.x+SHOT_W-1, alien_shot.y+SHOT_H-1)){
        player_shot.is_used = false;
        alien_shot.is_used = false;
    }

    // Alien collition
    for(i=0; i<ALIENS_ROWS && player_shot.is_used; ++i){
        for(j=0; j<ALIENS_COLUMNS; ++j){
            if(aliens[i][j].is_alive && collide(player_shot.x, player_shot.y, player_shot.x+SHOT_W-1, player_shot.y+SHOT_H-1, aliens[i][j].x, aliens[i][j].y, aliens[i][j].x+ALIENS_W-1, aliens[i][j].y+ALIENS_H-1)){
                player_shot.is_used = false;
                aliens[i][j].is_alive = false;
                player.score += aliens[i][j].points;
                break;
            }
        }
    }

    // Shield collition
    unsigned k; // shield
    if(player_shot.y+SHOT_H >= INITIAL_SHIELD_Y_COORDINATE){
        for(k=0; k<SHIELDS_CANT && player_shot.is_used; ++k){
            for(j=0; j<SHIELD_W && player_shot.is_used; ++j){
                for(i=SHIELD_H-1; i<SHIELD_H; --i){
                    if(shields[k][i][j].lives && collide(player_shot.x, player_shot.y, player_shot.x+SHOT_W-1, player_shot.y+SHOT_H-1, shields[k][i][j].x, shields[k][i][j].y, shields[k][i][j].x+SHIELD_BLOCK_W-1, shields[k][i][j].y+SHIELD_BLOCK_H-1)){
                    player_shot.is_used = false;
                    shields[k][i][j].lives--;
                    break;
                    }
                }
            }
        }
    }

    // Window limit
    if(player_shot.y < 0) // Out of bounds
        player_shot.is_used = false;
}

static void alien_shot_update(){
    if(!alien_shot.is_used) return;

    alien_shot.y += SHOT_DY_ALIEN;

    // Player_shot collition: it is in player_shot_update

    // Player collition
    if(collide(alien_shot.x, alien_shot.y, alien_shot.x+SHOT_W-1, alien_shot.y+SHOT_H-1, player.x, player.y, player.x+PLAYER_W-1, player.y+PLAYER_H-1)){
        alien_shot.is_used = false;
        player.lives--;
    }

    // Shield collition
    unsigned i, j;
    unsigned k; // shield
    if(alien_shot.y <= INITIAL_SHIELD_Y_COORDINATE + SHIELD_H*SHIELD_BLOCK_H){
        for(k=0; k<SHIELDS_CANT && alien_shot.is_used; ++k){
            for(j=0; j<SHIELD_W && alien_shot.is_used; ++j){
                for(i=0; i<SHIELD_H; ++i){
                    if(shields[k][i][j].lives && collide(alien_shot.x, alien_shot.y, alien_shot.x+SHOT_W-1, alien_shot.y+SHOT_H-1, shields[k][i][j].x, shields[k][i][j].y, shields[k][i][j].x+SHIELD_BLOCK_W-1, shields[k][i][j].y+SHIELD_BLOCK_H-1)){
                    alien_shot.is_used = false;
                    shields[k][i][j].lives--;
                    break;
                    }
                }
            }
        }
    }
}
