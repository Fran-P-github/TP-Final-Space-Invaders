// TODO: ver como arreglar la forma de ver el tiempo que pasa para algo en la rpi. Creo que eso hace que no ande tan bien el control del jugador

#include<time.h>
#include<unistd.h>
#include<string.h>

// Modulos
#include "front_rb.h"
#include "general_defines.h"
#include "back.h"
#include "font_3x5.h"
#include "scores.h"
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
#define ARROW_Y 3
#define ARROW_SPACING 5   // Espaciado horizontal entre opciones
#define BUTTON_PAUSE_TIME 1.2 // Seconds to hold the button to go into pause

#define CHAR_WIDTH 3
#define CHAR_HEIGHT 5
#define CHAR_SPACING 1
#define LINE_SPACING 1

typedef enum{
    NO_MOVE_X=0,
    MOVE_RIGHT_SLOW,
    MOVE_RIGHT_FAST,
    MOVE_LEFT_SLOW,
    MOVE_LEFT_FAST
} movement_x_t;

typedef enum{
    NO_MOVE_Y=0,
    MOVE_UP,
    MOVE_DOWN
} movement_y_t;

static void draw_rectangle(int x1, int y1, int x2, int y2);
static void draw_mothership();
static void draw_alien(unsigned i, unsigned j);
static void draw_player();
static void draw_player_shot();
static void draw_alien_shot();
static void draw_shield(unsigned shield);

static void wait_button_press();
static void wait_button_release();
static bool update_joystick();
static bool check_pause(joyinfo_t joystick);
static movement_x_t movement_read_x(int joystick_x_coordinate);
static movement_y_t movement_read_y(int joystick_y_coordinate);

static void draw3x3(const char icon[3][4], unsigned x, unsigned y);
static void draw_text_wrapped(const char* str, int x, int y);
static void blink_string(char buf[], unsigned size, unsigned x, unsigned y);

// Returns true when reaching logo end
static bool logo_menu_display();

static bool leaderboard_menu_display();

static void level_end_animation(level_state_t level_state);
static void get_player_name(char name[4], unsigned x, unsigned y);

game_state_t front_init(){
    back_init();

    joy_init();

    disp_init();
    disp_clear();
    disp_update();

    return MENU;
}

game_state_t menu(){
    static bool show_logo = true; // Show leaderboard when false
    static clock_t start = 0;
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;

    if(show_logo){
        if(logo_menu_display() && elapsed >= 8){
            show_logo = false;
            disp_clear();
        }
    }else{ // Show leaderboard
        if(leaderboard_menu_display()){
            show_logo = true;
            start = clock();
            disp_clear();
        }
    }

    jswitch_t button = joy_read().sw;
    if(button == J_PRESS){
        return GAME;
    }else{
        return MENU;
    }
}

game_state_t game_update(unsigned level){
    level_init(ALIENS_ROWS-1+level/2, ALIENS_COLUMNS-1+level/3, 1+level/3, SHIELD_BLOCK_LIVES-level/6);
    player_reset_on_new_level();
    if(level==0) player_reset_on_new_game();

    bool redraw = false;
    level_state_t level_state = LEVEL_NOT_DONE;
    unsigned long long frame = 0;

    while(level_state == LEVEL_NOT_DONE){
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
            level_state = back_update(level);
            redraw = true;
        }

        if(redraw){
            redraw = false;
            disp_clear();
            unsigned i, j;
            draw_player_shot();
            unsigned alien_column_to_shoot = get_best_alien_column_to_shoot();
            if(alien_column_to_shoot >= 0){
                alien_try_shoot(alien_column_to_shoot);
            }
            draw_alien_shot();
            for (i=0; i<SHIELDS_CANT; ++i){
                draw_shield(i);
            }
            draw_player();
            for(i=0; i<ALIENS_ROWS; ++i){
                for(j=0; j<ALIENS_COLUMNS; ++j){
                    if(aliens_is_alive(i,j)){
                        draw_alien(i, j);
                    }
                }
            }
            if(mothership_is_active()){
                draw_mothership();
            }

            disp_update();
        }
    }

    level_end_animation(level_state);

    if(level_state == PLAYER_WINS){
        return GAME;
    }else{
        return CLOSED;
    }
}

static bool leaderboard_menu_display(){
    static bool show_score = false; // Show TOP number when false
    highscore_t top_scores[MAX_SCORES]; load_scores(top_scores);
    static unsigned current_score = 0;
    static unsigned cont_this_score = 0;
    const unsigned displays_per_score = 2;

    static clock_t start = 0;
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    if(elapsed >= 0.2){ // No son 0.2 segundos, es porque algo anda mal con esta forma de ver el tiempo en la rpi
        cont_this_score++;
        show_score = !show_score;
        start = clock();
        if(cont_this_score == 2*displays_per_score){
            if(++current_score == MAX_SCORES){
                current_score = 0;
                return true;
            }
            cont_this_score  = 0;
            show_score = false;
        }
    }

    char buf[5];
    if(show_score) snprintf(buf, sizeof(buf), "%4d", top_scores[current_score].score);
    else snprintf(buf, sizeof(buf), "TOP%d", current_score+1);
    draw_text_wrapped(buf, 0, 3);
    snprintf(buf, sizeof(buf), "%s", top_scores[current_score].name);
    draw_text_wrapped(buf, 3, 10);
    disp_update();

    return false;
}

static bool logo_menu_display(){
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
    static unsigned column = LETTERS_WIDTH - 4;

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

    return !(column % LETTERS_WIDTH);
}

static void level_end_animation(level_state_t level_state){
    disp_clear();
    char buf[11];
    int x_offset;
    if(level_state == PLAYER_WINS){
        snprintf(buf, sizeof(buf), "YOUWIN");
        x_offset = 3;
    }else{
        snprintf(buf, sizeof(buf), "GAMEOVER");
        x_offset = 1;
    }
    draw_text_wrapped(buf, x_offset, 2);
    disp_update();
    sleep(1);
    wait_button_press();
    wait_button_release();
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
        "***"
    };
    const char exit[3][4] = {
        "* *",
        " * ",
        "* *"
    };
    const char lives[3][4] = {
        "* *",
        "***",
        " * "
    };

    disp_clear();
    draw3x3(resume, 1, ARROW_Y+2);
    draw3x3(menu, 6, ARROW_Y+2);
    draw3x3(exit, 11, ARROW_Y+2);
    draw3x3(lives, 4, 12);
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", player_get_lives());
    draw_text_wrapped(buf, 8, 11);
    disp_update();

    // Wait for button release
    wait_button_release();

    int selected = 0;

    while (true) {
        joyinfo_t js = joy_read();
        movement_x_t pos;
        static movement_x_t prev_pos = NO_MOVE_X;

        if (js.x < -50) pos = MOVE_LEFT_FAST;
        else if (js.x > 50) pos = MOVE_RIGHT_FAST;
        else pos = NO_MOVE_X;
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
            disp_clear();
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
    disp_clear();

    char score[10];
    snprintf(score, sizeof(score), "%d", player_get_score());
    disp_clear();
    blink_string(score, strlen(score), 0, 3);

    draw_text_wrapped("NAME", 0, 3);
    disp_update();
    char name[NAME_LEN+1]; // 3 letters
    get_player_name(name, 2, 10);
    disp_clear();
    blink_string(name, sizeof(name)-1, 2, 5);

    highscore_t top_scores[MAX_SCORES]; load_scores(top_scores);
    if( try_insert_score(top_scores, name, player_get_score()) ){
        disp_clear();
        blink_string("NEW BEST", sizeof("NEW BEST")-1, 0, 3);
    }
    save_scores(top_scores);
}

static void blink_string(char buf[], unsigned size, unsigned x, unsigned y){
    for(unsigned i=0; i<3; ++i){ // Score blinks 3 times
        usleep(300000); // 0.3 sec
        draw_text_wrapped(buf, x, y);
        disp_update();
        usleep(400000); // 0.4 sec
        disp_clear();
        disp_update();
    }
}

static void get_player_name(char name[4], unsigned x, unsigned y){
    static const char letters[LETTERS_CANT+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    unsigned index = 0;
    unsigned current_letter = 0;
    movement_y_t prev_movement = NO_MOVE_Y;

    name[0] = name[1] = name[2] = name[3] = '\0'; // Limpiar

    while(index < 3){
        joyinfo_t js = joy_read();
        movement_y_t movement = movement_read_y(js.y);

        // Detectar flanco (cambio de movimiento)
        if(movement != prev_movement){
            if(movement == MOVE_UP){
                current_letter = (current_letter + 1) % LETTERS_CANT;
            } else if(movement == MOVE_DOWN){
                current_letter = (current_letter + LETTERS_CANT - 1) % LETTERS_CANT;
            }
            prev_movement = movement;
        }

        name[index] = letters[current_letter];
        draw_text_wrapped(name, x, y);
        disp_update();

        // Confirmar letra al soltar el botón después de presionarlo
        static bool was_pressed = false;
        if(js.sw == J_PRESS){
            was_pressed = true;
        }else if(was_pressed && js.sw == J_NOPRESS){
            name[index++] = letters[current_letter];
            current_letter = 0;
            was_pressed = false;
        }

        usleep(10000); // 10 ms debounce
    }
}

static void draw_text_wrapped(const char *str, int x, int y) {
    int orig_x = x;

    for (const char *p = str; *p != '\0'; ++p) {
        char c = *p;

        // Space
        if (c == ' ') {
            for (int i = 0; i < CHAR_HEIGHT; ++i) {
                for (int j = 0; j < CHAR_WIDTH; ++j) {
                    dcoord_t coord = { .x = x+j, .y = y+i };
                    disp_write(coord, D_OFF);
                }
            }
            x += CHAR_WIDTH + CHAR_SPACING;
        }
        // Digit
        else if (c >= '0' && c <= '9') {
            const char (*glyph)[4] = NUM_FONT[c - '0'];
            for (int i = 0; i < CHAR_HEIGHT; ++i) {
                for (int j = 0; j < CHAR_WIDTH; ++j) {
                    dcoord_t coord = { .x = x+j, .y = y+i };
                    disp_write(coord, glyph[i][j]=='*' ?  D_ON : D_OFF);
                }
            }
            x += CHAR_WIDTH + CHAR_SPACING;
        }
        // Letter (upper case)
        else if ((c=toupper(c)) >= 'A' && c <= 'Z') {
            const char (*glyph)[4] = LETTER_FONT[c - 'A'];
            for (int i = 0; i < CHAR_HEIGHT; ++i) {
                for (int j = 0; j < CHAR_WIDTH; ++j) {
                    dcoord_t coord = { .x = x+j, .y = y+i };
                    disp_write(coord, glyph[i][j]=='*' ?  D_ON : D_OFF);
                }
            }
            x += CHAR_WIDTH + CHAR_SPACING;
        }

        // Salto de línea si no entra el siguiente carácter
        if (x + CHAR_WIDTH > WORLD_WIDTH) {
            x = orig_x;
            y += CHAR_HEIGHT + LINE_SPACING;
        }

        // Cortar si nos pasamos del alto del display
        if (y + CHAR_HEIGHT > WORLD_HEIGHT) {
            break;
        }
    }
}

static void wait_button_press(){
    while(joy_read().sw == J_NOPRESS);
}

static void wait_button_release(){
    while(joy_read().sw == J_PRESS);

    // Debouncing
    usleep(100000); // 0.1 sec
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

        movement_x_t movement = movement_read_x(joystick.x);
        static movement_x_t prev_movement = NO_MOVE_X;
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
                case NO_MOVE_X:
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

static movement_x_t movement_read_x(int x){
    movement_x_t movement;

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
        movement = NO_MOVE_X;
    }

    return movement;
}

static movement_y_t movement_read_y(int y){
    movement_y_t movement;

    if(y > JOY_THRESHOLD_FAST){
        movement = MOVE_UP;
    }
    else if(y < -JOY_THRESHOLD_FAST){
        movement = MOVE_DOWN;
    }
    else{
        movement = NO_MOVE_Y;
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

static void draw_mothership(){
    draw_rectangle(mothership_get_x(), mothership_get_y(), mothership_get_x()+MOTHERSHIP_W-1, mothership_get_y()+MOTHERSHIP_H-1);
}

static void draw_alien(unsigned i, unsigned j){
    draw_rectangle(aliens_get_x(i,j), aliens_get_y(i,j), aliens_get_x(i,j)+ALIENS_W-1, aliens_get_y(i,j)+ALIENS_H-1);
}

static void draw_player(){
    draw_rectangle(player_get_x(), player_get_y(), player_get_x()+PLAYER_W-1, player_get_y()+PLAYER_H-1);
}

static void draw_alien_shot(){
    if(alien_shot_is_used())
        draw_rectangle(alien_shot_get_x(), alien_shot_get_y(), alien_shot_get_x()+SHOT_W-1, alien_shot_get_y()+SHOT_H-1);
}

static void draw_player_shot(){
    if(player_shot_is_used())
        draw_rectangle(player_shot_get_x(), player_shot_get_y(), player_shot_get_x()+SHOT_W-1, player_shot_get_y()+SHOT_H-1);
}

static void draw_shield(unsigned shield){
    unsigned i, j;
    for(i=0; i<SHIELD_H; ++i){
        for(j=0; j<SHIELD_W; ++j){
            if(shield_get_lives(shield,i,j))
            draw_rectangle(shield_get_x(shield,i,j), shield_get_y(shield,i,j), shield_get_x(shield,i,j)+SHIELD_BLOCK_W-1, shield_get_y(shield,i,j)+SHIELD_BLOCK_H-1);
        }
    }
}

