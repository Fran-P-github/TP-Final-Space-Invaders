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

#define LETTER_HEIGHT 5
#define LETTER_MARGIN 2
#define LETTERS_WIDTH 57
#define LETTERS_WAIT_TIME 0.1 // Seconds

#define MENU_OPTIONS 3
#define ARROW_X 2
#define ARROW_Y 6
#define ARROW_SPACING 5   // Espaciado horizontal entre opciones
#define BUTTON_PAUSE_TIME 1.2 // Seconds to hold the button to go into pause

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

static void wait_button_release();
static bool update_joystick();
static bool check_pause(joyinfo_t joystick);
static movement_t movement_read(int joystick_x_coordinate);

// Back variables
static player_t* player;
static alien_t (*aliens) [ALIENS_ROWS][ALIENS_COLUMNS];
static double* aliens_move_interval;
static shot_t* player_shot;
static shot_t* alien_shot;
static shield_t (*shields) [SHIELDS_CANT];

game_state_t front_init(){
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

    return MENU;
}

game_state_t menu(){
    const char space[LETTER_HEIGHT][LETTERS_WIDTH] = {
        "        ***  ****   ***   ***  *****                    ",
        "       *     *   * *   * *     *                        ",
        "        ***  ****  ***** *     ***                      ",
        "           * *     *   * *     *                        ",
        "        ***  *     *   *  ***  *****                    "
    };
    const char invaders[LETTER_HEIGHT][LETTERS_WIDTH] = {
        "***** *   * *   *  ***  ****  ***** ****   ***         ",
        "  *   **  * *   * *   * *   * *     *   * *            ",
        "  *   * * *  * *  ***** *   * ***   ****   ***         ",
        "  *   *  **  * *  *   * *   * *     * *       *        ",
        "***** *   *   *   *   * ****  ***** *  *   ***         "
    };
    static unsigned column = 0;

    static clock_t start = 0;
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    if(elapsed >= LETTERS_WAIT_TIME){
        start = clock();

        unsigned i, j;
        // SPACE
        for(i=0; i<LETTER_HEIGHT; ++i){
            for(j=0; j<WORLD_WIDTH; ++j){
                dcoord_t coord = { .x=j, .y=i+LETTER_MARGIN };
                disp_write(coord, space[i][(j+column)%LETTERS_WIDTH]=='*' ? D_ON : D_OFF);
            }
        }

        // INVADERS
        for(i=0; i<LETTER_HEIGHT; ++i){
            for(j=0; j<WORLD_WIDTH; ++j){
                dcoord_t coord = { .x=j, .y=i+LETTER_HEIGHT+2*LETTER_MARGIN };
                disp_write(coord, invaders[i][(j+column)%LETTERS_WIDTH]=='*' ? D_ON : D_OFF);
            }
        }

        disp_update();
        ++column;
    }

    jswitch_t button = joy_read().sw;
    if(button == J_PRESS){
        column = 0;
        return GAME;
    }else{
        return MENU;
    }
}

game_state_t game_update(){
    bool redraw = false;
    bool done = false;
    unsigned long long frame = 0;

    while(!done){
        if(update_joystick()){
            return PAUSE;
        }

        static clock_t frame_start = 0;
        double frame_elapsed = (double)(clock() - frame_start) / CLOCKS_PER_SEC;
        const double fps = 6;
        const double frame_time = 1 / fps; // Seconds
        if(frame_elapsed >= frame_time){
            ++frame;
            frame_start = clock();
            done = aliens_update(); // aliens_update() returns true when aliens reach player height
            shots_update();
            redraw = true;
        }

        if(redraw){
            redraw = false;
            disp_clear();
            unsigned i, j;
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

    return CLOSED;
}

static void draw3x3(const char icon[3][4], unsigned x, unsigned y){
    for(unsigned i=0; i<3; ++i){
        for(unsigned j=0; j<3; ++j){
            dcoord_t coord = { .x = x+j, .y = y+i };
            disp_write(coord, icon[i][j]=='*' ? D_ON : D_OFF);
        }
    }
}

game_state_t game_pause(){
    const char resume[3][4] = {
        " * ",
        " **",
        " * "
    };
    const char menu[3][4] = {
        " * ",
        "***",
        "* *"
    };
    const char exit[3][4] = {
        "* *",
        " * ",
        "* *"
    };

    disp_clear();
    draw3x3(resume, 1, 8);
    draw3x3(menu, 6, 8);
    draw3x3(exit, 11, 8);
    disp_update();

    // Wait for button release
    wait_button_release();

    int selected = 0;

    while (true) {
        joyinfo_t js = joy_read();
        movement_t pos;
        static movement_t prev_pos = NO_MOVE;

        if (js.x < -50) pos = MOVE_LEFT_FAST;
        else if (js.x > 50) pos = MOVE_RIGHT_FAST;
        else pos = NO_MOVE;
        if(pos != prev_pos){
            if(pos == MOVE_RIGHT_FAST) selected++;
            if(pos == MOVE_LEFT_FAST) selected--;

            if(selected<0) selected = 0;
            if(selected>=MENU_OPTIONS) selected = MENU_OPTIONS-1;
        }
        prev_pos = pos;

        // Draw options
        for (int i = 0; i < MENU_OPTIONS; ++i) {
            dcoord_t coord = { .x = ARROW_X+i*ARROW_SPACING, .y = ARROW_Y };
            disp_write(coord, i == selected ? D_ON : D_OFF);
        }
        disp_update();

        // Selección con botón
        if (js.sw == J_PRESS) {
            wait_button_release();
            switch(selected){
                case 0:
                    return GAME;
                    break;
                case 1:
                    return MENU;
                    break;
                case 2:
                    return CLOSED;
                    break;
            }
        }
    }
    return GAME;
}

void endgame(){

}

static void wait_button_release(){
    while(joy_read().sw == J_PRESS);

    // Debouncing
    clock_t clk = clock();
    while((double)(clock() - clk)/CLOCKS_PER_SEC <= 0.01);
}

// Returns: true when going into PAUSE. false otherwise
static bool update_joystick(){
        joyinfo_t joystick = joy_read();

        if(check_pause(joystick)){
            return true;
        }

        if(joystick.sw == J_PRESS){
            player_try_shoot();
        }

        movement_t movement = movement_read(joystick.x);
        static movement_t prev_movement = NO_MOVE;
        static clock_t player_time_start = 0;
        double player_elapsed = (double)(clock() - player_time_start) / CLOCKS_PER_SEC;
        double player_wait_time =   movement == MOVE_LEFT_FAST || movement == MOVE_RIGHT_FAST ? FAST_MOVEMENT_WAIT_TIME :
                                    movement == MOVE_LEFT_SLOW || movement == MOVE_RIGHT_SLOW ? SLOW_MOVEMENT_WAIT_TIME :
                                    99999;

        if(movement != prev_movement || player_elapsed > player_wait_time){
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

        return false;
}

static bool check_pause(joyinfo_t joystick){
    static clock_t pause_time_start = 0;
    static bool was_pressed = false;

    if(joystick.sw == J_PRESS) {
        if(!was_pressed) {
            pause_time_start = clock();  // primer frame de la pulsación
        }
        double pause_elapsed = (double)(clock() - pause_time_start) / CLOCKS_PER_SEC;
        if(pause_elapsed >= BUTTON_PAUSE_TIME) {
            was_pressed = false;
            return true;  // Se pausó el juego
        }
        was_pressed = true;
    } else {
        was_pressed = false;
    }
    return false;
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

