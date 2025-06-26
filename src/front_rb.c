#include<time.h>

// Modulos
#include "front_rb.h"
#include "general_defines.h"
#include "back.h"
// Matriz de leds y joystick
#include "../libs/joydisp/disdrv.h"
#include "../libs/joydisp/joydrv.h"
// Audio
#include <SDL2/SDL.h>
#include "../libs/SDL2/audio.h"

#define JOY_THRESHOLD_SLOW  20
#define JOY_THRESHOLD_FAST  100

#define SLOW_MOVEMENT_WAIT_TIME 4
#define FAST_MOVEMENT_WAIT_TIME 0.3

typedef enum{
    MOVE_RIGHT_SLOW=0,
    MOVE_RIGHT_FAST,
    MOVE_LEFT_SLOW,
    MOVE_LEFT_FAST,
    NO_MOVE
} movement_t;

static void draw_rectangle(int x1, int y1, int x2, int y2);
static void draw_alien(unsigned i, unsigned j);
static void draw_player();
static void draw_player_shot();
static void draw_alien_shot();
static void draw_shield(unsigned shield);

static void update_joystick();

static movement_t movement_read(int joystick_x_coordinate);

// Back variables
static player_t* player;
static alien_t (*aliens) [ALIENS_ROWS][ALIENS_COLUMNS];
static double* aliens_move_interval;
static shot_t* player_shot;
static shot_t* alien_shot;
static shield_t (*shields) [SHIELDS_CANT];

void front_init(){
    player = get_player();
    aliens = get_aliens();
    aliens_move_interval = get_aliens_move_interval();
    player_shot = get_player_shot();
    alien_shot = get_alien_shot();
    shields = get_shields();

    joy_init();

    disp_init();
    disp_clear();
    disp_update();
}

void front_run(){
    bool redraw = false;
    bool done = false;
    unsigned long long frame = 0;

    while(!done){
        update_joystick();

        static clock_t frame_start = 0;
        double frame_elapsed = (double)(clock() - frame_start) / CLOCKS_PER_SEC;
        const double fps = 6;
        const double frame_time = 1 / fps; // Seconds
        if(frame_elapsed >= frame_time){
            ++frame;
            frame_start = clock();
            aliens_update_position();
            shots_update();
            redraw = true;
        }

        if(redraw){
            redraw = false;
            disp_clear();
            unsigned i, j;
            player_try_shoot();
            draw_player_shot();
            alien_try_shoot(1);
            draw_alien_shot();
            for (i=0; i<SHIELDS_CANT; ++i){
                draw_shield(i);
            }
            draw_player();
            for(i=0; i<ALIENS_ROWS; ++i){
                for(j=0; j<ALIENS_COLUMNS; ++j){
                    if((*aliens)[i][j].is_alive){
                        draw_alien(i, j);
                    }
                }
            }

            disp_update();
        }
    }
}

#define same_direction() ( ( (movement == MOVE_LEFT_SLOW  || movement == MOVE_LEFT_FAST ) && \
                      (prev_movement == MOVE_LEFT_SLOW || prev_movement == MOVE_LEFT_FAST) ) \
                   || ( (movement == MOVE_RIGHT_SLOW || movement == MOVE_RIGHT_FAST) && \
                      (prev_movement == MOVE_RIGHT_SLOW || prev_movement == MOVE_RIGHT_FAST) ) )
// TODO: fix bug where player sometimes moves two places at a time (usually one every four moves)
static void update_joystick(){
        joyinfo_t joystick = joy_read();
        movement_t movement = movement_read(joystick.x);
        static movement_t prev_movement = NO_MOVE;
        static clock_t player_time_start = 0;
        double player_elapsed = (double)(clock() - player_time_start) / CLOCKS_PER_SEC;
        double player_wait_time =   movement == MOVE_LEFT_FAST || movement == MOVE_RIGHT_FAST ? FAST_MOVEMENT_WAIT_TIME :
                                    movement == MOVE_LEFT_SLOW || movement == MOVE_RIGHT_SLOW ? SLOW_MOVEMENT_WAIT_TIME :
                                    99999;
        if(movement != NO_MOVE && (!same_direction() || player_elapsed > player_wait_time)){
            player_time_start = clock();

            switch(movement){
                case MOVE_RIGHT_SLOW:
                        player_move_right();
                    break;
                case MOVE_RIGHT_FAST:
                        player_move_right();
                    break;
                case MOVE_LEFT_SLOW:
                        player_move_left();
                    break;
                case MOVE_LEFT_FAST:
                        player_move_left();
                    break;
                case NO_MOVE:
                    break;
            }
        }
        prev_movement = movement;
}

static movement_t movement_read(int x){
    movement_t movement;

    if(x > JOY_THRESHOLD_FAST){
        movement = MOVE_RIGHT_FAST;
    }
    else if(x > JOY_THRESHOLD_SLOW){
        movement = MOVE_RIGHT_SLOW;
    }
    else if(x < -JOY_THRESHOLD_FAST){
        movement = MOVE_LEFT_FAST;
    }
    else if(x < -JOY_THRESHOLD_SLOW){
        movement = MOVE_LEFT_SLOW;
    }
    else{
        movement = NO_MOVE;
    }

    return movement;
}

static void draw_rectangle(int x1, int y1, int x2, int y2){
    if(x1>x2 || y1>y2) return;
    int i, j;
    for(i = x1; i <= x2; i++){
        for(j = y1; j <= y2; j++){
            if(i>=0 && j>=0 && i<WORLD_WIDTH && j<WORLD_HEIGHT)
                disp_write((dcoord_t){.x=i, .y=j}, D_ON); // Se prende el led en la posicion {i, j}
        }
    } 
}

static void draw_alien(unsigned i, unsigned j){
    draw_rectangle((*aliens)[i][j].x, (*aliens)[i][j].y, (*aliens)[i][j].x+ALIENS_W-1, (*aliens)[i][j].y+ALIENS_H-1);
}

static void draw_player(){
    draw_rectangle(player->x, player->y, player->x+PLAYER_W-1, player->y+PLAYER_H-1);
}

static void draw_alien_shot(){
    if(alien_shot->is_used)
        draw_rectangle(alien_shot->x, alien_shot->y, alien_shot->x+SHOT_W-1, alien_shot->y+SHOT_H-1);
}

static void draw_player_shot(){
    if(player_shot->is_used)
        draw_rectangle(player_shot->x, player_shot->y, player_shot->x+SHOT_W-1, player_shot->y+SHOT_H-1);
}

static void draw_shield(unsigned shield){
    unsigned i, j;
    for(i=0; i<SHIELD_H; ++i){
        for(j=0; j<SHIELD_W; ++j){
            if((*shields)[shield][i][j].lives)
            draw_rectangle((*shields)[shield][i][j].x, (*shields)[shield][i][j].y, (*shields)[shield][i][j].x+SHIELD_BLOCK_W-1, (*shields)[shield][i][j].y+SHIELD_BLOCK_H-1);
        }
    }
}

