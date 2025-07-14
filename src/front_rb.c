#include<time.h>
#include<unistd.h>
#include<string.h>

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

// Program modules
#include "front.h"
#include "general_defines.h"
#include "back.h"
#include "font_3x5.h"
#include "scores.h"

// LED matrix and joystick
#include "../libs/joydisp/disdrv.h"
#include "../libs/joydisp/joydrv.h"

// Audio
#include <SDL2/SDL.h>
#include "../libs/SDL2/audio.h"

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define FPS 6

// Player joystick defnitions
#define JOY_THRESHOLD_SLOW  JOY_MAX_POS / 6
#define JOY_THRESHOLD_FAST  JOY_MAX_POS / 1.1
#define SLOW_MOVEMENT_WAIT_TIME 0.2
#define FAST_MOVEMENT_WAIT_TIME 0.1

// Intro letters definitions
#define INTRO_LETTER_HEIGHT 5
#define INTRO_LETTER_MARGIN 2
#define INTRO_LETTERS_WIDTH 57
#define INTRO_LETTERS_WAIT_TIME 0.1 // Seconds

// Pause menu definitions
typedef enum{
    RESUME=0,
    HOME,
    EXIT
} pause_option_t;
#define PAUSE_MENU_OPTIONS EXIT+1
#define ARROW_X 2 // Arrows initial
#define ARROW_Y 3 // position
#define ARROW_SPACING 5   // Options horizontal spacing
#define BUTTON_PAUSE_TIME 0.55 // Seconds to hold the button to go into pause

// Sizes to use font3x5 variables
#define CHAR_WIDTH 3
#define CHAR_HEIGHT 5
#define CHAR_SPACING 1 // Space between letters
#define LINE_SPACING 1

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

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

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

#define MSJ_ERR_INIT "Problem initialazing: "
static void init_error(bool state, const char *name);

 // Draws a filled rectangle. Used by all other draw functions
static void draw_rectangle(int x1, int y1, int x2, int y2);

static void draw_mothership();
// Draws alien in position i,j in matrix, if alive
static void draw_alien(unsigned i, unsigned j);
static void draw_player();
static void draw_player_shot();
static void draw_alien_shot();
static void draw_shield(unsigned shield);

static void wait_button_press();
static void wait_button_release();

// Returns true when going into PAUSE. false otherwise
static bool update_joystick();
// Returns true when pause condition is trigered: holding button for at least BUTTON_PAUSE_TIME seconds
static bool check_pause(joyinfo_t joystick);

static movement_x_t movement_read_x(int joystick_x_coordinate);
static movement_y_t movement_read_y(int joystick_y_coordinate);

// Draws a 3x3 icon to x,y position
static void draw3x3(const char icon[3][4], unsigned x, unsigned y);

// Draws text to x,y position. Tries to use more than 1 line if it doesn't fit
static void draw_text_wrapped(const char* str, int x, int y);

// Blinks string in x,y position
static void blink_string(char buf[], unsigned x, unsigned y);

// Returns true when reaching logo end
static bool logo_menu_display();

// Returns true after showing last place in leaderboard
static bool leaderboard_menu_display();

// Show a message at the end of each level
static void level_end_animation(level_state_t level_state);

// Reads player name (3 letters). Returns to name string. Promt appears in x,y
static void get_player_name(char name[4], unsigned x, unsigned y);

// Plays game sounds
static void sounds_update();

// Function to play a sound. Waits duration_ms before playing again
#define play_sound_with_duration(sound, duration_ms) \
{ \
    static unsigned long long start = 0; \
    double elapsed = (double)(get_millis() - start); \
    if(elapsed > (duration_ms)){ \
        start = get_millis(); \
        playSoundFromMemory((sound), SDL_MIX_MAXVOLUME); \
    } \
}

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// Audio sounds
#define INIT_SOUND(p, route) p = createAudio(route, 0, SDL_MIX_MAXVOLUME); init_error(p, #p);
static Audio *mothershipMusic = NULL;
static Audio *playerShotSound = NULL;
static Audio *playerDeathSound = NULL;
static Audio *alienDeathSound = NULL;
static Audio* mothershipDeathSound = NULL;
static Audio *alienHitSound = NULL;
static Audio *alienMovedSound = NULL;
static Audio *shieldHitSound = NULL;
static Audio *pauseSound = NULL;

/*******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

game_state_t front_init(){
    back_init();

    joy_init();

    disp_init();
    disp_clear();
    disp_update();

    init_error(initAudio(), "Audio library");
    INIT_SOUND(playerShotSound, AUDIO_PLAYER_SHOT);
    INIT_SOUND(playerDeathSound, AUDIO_PLAYER_DEATH);
    INIT_SOUND(alienDeathSound, AUDIO_INVADER_DEATH);
    INIT_SOUND(mothershipDeathSound, AUDIO_UFO_DEATH);
    INIT_SOUND(alienHitSound, AUDIO_INVADER_HIT);
    INIT_SOUND(alienMovedSound, AUDIO_INVADER_MOVED);
    INIT_SOUND(mothershipMusic, AUDIO_UFO);
    INIT_SOUND(shieldHitSound, AUDIO_SHIELD_HIT);
    INIT_SOUND(pauseSound, AUDIO_PAUSE);

    return MENU; // Go into menu scene first
}

game_state_t game_pause(unsigned int* level, bool* new_level){ // level and new_level unused here. Added for compatibility for pause in front_allegro
    playSoundFromMemory(pauseSound, SDL_MIX_MAXVOLUME);

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

    // Draw menu
    disp_clear();
    draw3x3(resume, 1, ARROW_Y+2);
    draw3x3(menu, 6, ARROW_Y+2);
    draw3x3(exit, 11, ARROW_Y+2);
    draw3x3(lives, 4, 12);
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", player_get_lives());
    draw_text_wrapped(buf, 8, 11);
    disp_update();

    // Wait for button release before allowing to select any options
    wait_button_release();

    pause_option_t selected = RESUME; // Current option pointed by arrow

    game_state_t ret;
    while (true) {
        // Read joystick and move arrow
        joyinfo_t js = joy_read();
        movement_x_t pos;
        static movement_x_t prev_pos = NO_MOVE_X;

        if (js.x < -JOY_THRESHOLD_SLOW) pos = MOVE_LEFT_FAST;
        else if (js.x > JOY_THRESHOLD_SLOW) pos = MOVE_RIGHT_FAST;
        else pos = NO_MOVE_X;
        if(pos != prev_pos){
            if(pos == MOVE_RIGHT_FAST && selected != EXIT) selected++;
            if(pos == MOVE_LEFT_FAST && selected != RESUME) selected--;
        }
        prev_pos = pos;

        // Draw arrow
        for (int i = 0; i < PAUSE_MENU_OPTIONS; ++i) {
            dcoord_t coord = { .x = ARROW_X+i*ARROW_SPACING, .y = ARROW_Y };
            disp_write(coord, i == selected ? D_ON : D_OFF);
        }
        disp_update();

        // Selection on button press
        if (js.sw == J_PRESS) {
            wait_button_release();
            switch(selected){
                case RESUME:
                    ret = GAME;
                    break;
                case HOME:
                    ret = MENU;
                    break;
                case EXIT:
                    ret = CLOSED;
                    break;
            }
            break;
        }
    }
    
    disp_clear();
    disp_update();
    playSoundFromMemory(pauseSound, SDL_MIX_MAXVOLUME);
    return ret;
}

game_state_t menu(){
    static bool show_logo = true; // Show leaderboard when false
    static int logo_iterations = 0;

    if(show_logo){
        if(logo_menu_display() && ++logo_iterations > 3){ // Show logo 3 times before showing hi-scores
            show_logo = false;
            disp_clear();
        }
    }else{ // Show leaderboard
        if(leaderboard_menu_display()){
            show_logo = true;
            logo_iterations = 0;
            disp_clear();
        }
    }

    jswitch_t button = joy_read().sw;
    if(button == J_PRESS){
        wait_button_release();
        return GAME;
    }else{
        return MENU;
    }
}

game_state_t game_update(unsigned level, bool new_level){
    // Initial level config
    if(new_level){ // Restart on new level
        level_init(level, ALIENS_ROWS-1+level/2, ALIENS_COLUMNS-1+level/3, 1, 1, SHIELD_BLOCK_LIVES-level/6);
        if ( level == 0 ) player_reset_on_new_game();
    }

    bool redraw = false;
    level_state_t level_state = LEVEL_NOT_DONE;
    unsigned long long frame = 0;

    while(level_state == LEVEL_NOT_DONE){
        if(update_joystick()){
            return PAUSE;
        }

        // New frame detection and game logic
        static unsigned long long frame_start = 0;
        double frame_elapsed = (double)(get_millis() - frame_start) / 1000;
        const double frame_time = 1 / FPS; // Seconds
        if(frame_elapsed >= frame_time){
            ++frame;
            frame_start = get_millis();
            level_state = back_update(level, new_level);
            new_level = false;
            sounds_update();
            unsigned alien_column_to_shoot = get_best_alien_column_to_shoot();
            if(alien_column_to_shoot >= 0 && !(frame%30)){
                alien_try_shoot(alien_column_to_shoot);
            }
            redraw = true;
        }

        // Draw new frame
        if(redraw){
            redraw = false;
            disp_clear();
            unsigned i, j;
            draw_player_shot();
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
        return ENDGAME;
    }
}

game_state_t endgame(){
    disp_clear();

    // Show player score
    char score[10];
    snprintf(score, sizeof(score), "%d", player_get_score());
    disp_clear();
    blink_string(score, 0, 3);

    // Ask for player name
    draw_text_wrapped("NAME", 0, 3);
    disp_update();
    char name[NAME_LEN+1]; // 3 letters
    get_player_name(name, 2, 10);
    disp_clear();
    blink_string(name, 2, 5);

    // Check hi-scores and save score if it is a new best
    highscore_t top_scores[MAX_SCORES]; load_scores(top_scores);
    if( try_insert_score(top_scores, name, player_get_score()) ){
        disp_clear();
        blink_string("NEW BEST", 0, 3);
    }
    save_scores(top_scores);

    disp_clear();
    disp_update();
    return MENU;
}

void front_deinit(){
    // Free audios
    freeAudio(mothershipMusic);
    freeAudio(playerShotSound);
    freeAudio(playerDeathSound);
    freeAudio(alienDeathSound);
    freeAudio(mothershipDeathSound);
    freeAudio(alienHitSound);
    freeAudio(alienMovedSound);
    freeAudio(shieldHitSound);
    freeAudio(pauseSound);

    // Clear display
    disp_clear();
    disp_update();
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void sounds_update(){
    if(mothership_is_active()){
        play_sound_with_duration(mothershipMusic, 2000);
    }

    if(playerDied){
        play_sound_with_duration(playerDeathSound, 800);
    }

    if(aliensMoved){
        play_sound_with_duration(alienMovedSound, 90);
    }

    if(alienWasHit){
        playSoundFromMemory(alienHitSound, SDL_MIX_MAXVOLUME);
    }

    if(shieldWasHit){
        playSoundFromMemory(shieldHitSound, SDL_MIX_MAXVOLUME);
    }

    if(alienWasKilled){
        explosion_t explosion;
        get_explosion_state(&explosion);
        switch(explosion.type){
            case ALIEN_EXPLOSION:
                playSoundFromMemory(alienDeathSound, SDL_MIX_MAXVOLUME);
                break;
            case UFO_EXPLOSION:
                // We dont' play the ufo explosion sound here. It doesn't "fit" in the raspberry game
                break;
            case NO_EXPLOSION:
                break;
        }
    }
}

static bool leaderboard_menu_display(){
    static bool show_score = true; // Show TOP number when false
    highscore_t top_scores[MAX_SCORES]; load_scores(top_scores);
    static unsigned current_score = 0; // Score currently shown
    static unsigned cont_this_score = 0; // Counts up to displays_per_score, to show each score multiple times before going to the next
    const unsigned displays_per_score = 2;

    static unsigned long long start = 0;
    double elapsed = (double)(get_millis() - start) / 1000;
    if(elapsed >= 1){
        show_score = !show_score;
        start = get_millis();
        if(cont_this_score == 2*displays_per_score){
            cont_this_score  = 0;
            if(++current_score == MAX_SCORES){
                current_score = 0;
                show_score = true;
                return true;
            }
            show_score = false;
        }
        cont_this_score++;
    }

    // Print score to display
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
    const char space[INTRO_LETTER_HEIGHT][INTRO_LETTERS_WIDTH] = {
        "        ***  ****   ***   ***  *****                    ",
        "       *     *   * *   * *     *                        ",
        "        ***  ****  ***** *     ***                      ",
        "           * *     *   * *     *                        ",
        "        ***  *     *   *  ***  *****                    "
    };
    const char invaders[INTRO_LETTER_HEIGHT][INTRO_LETTERS_WIDTH] = {
        "***** *   * *   *  ***  ****  ***** ****   ***         ",
        "  *   **  * *   * *   * *   * *     *   * *            ",
        "  *   * * *  * *  ***** *   * ***   ****   ***         ",
        "  *   *  **  * *  *   * *   * *     * *       *        ",
        "***** *   *   *   *   * ****  ***** *  *   ***         "
    };
    
    // Current first column
    static unsigned column = INTRO_LETTERS_WIDTH - 8; // -8 to start with letters more to the right

    static unsigned long long start = 0;
    double elapsed = (double)(get_millis() - start) / 1000;
    if(elapsed >= INTRO_LETTERS_WAIT_TIME){
        start = get_millis();

        unsigned i, j;
        // SPACE
        for(i=0; i<INTRO_LETTER_HEIGHT; ++i){
            for(j=0; j<WORLD_WIDTH; ++j){
                dcoord_t coord = { .x=j, .y=i+INTRO_LETTER_MARGIN };
                disp_write(coord, space[i][(j+column)%INTRO_LETTERS_WIDTH]=='*' ? D_ON : D_OFF);
            }
        }

        // INVADERS
        for(i=0; i<INTRO_LETTER_HEIGHT; ++i){
            for(j=0; j<WORLD_WIDTH; ++j){
                dcoord_t coord = { .x=j, .y=i+INTRO_LETTER_HEIGHT+2*INTRO_LETTER_MARGIN };
                disp_write(coord, invaders[i][(j+column)%INTRO_LETTERS_WIDTH]=='*' ? D_ON : D_OFF);
            }
        }

        disp_update();
        ++column;
        return !(column % INTRO_LETTERS_WIDTH);
    }

    return false;
}

static void level_end_animation(level_state_t level_state){
    disp_clear();
    char buf[11];
    int x_offset; // Horizontal offset for message
    if(level_state == PLAYER_WINS){
        snprintf(buf, sizeof(buf), "YOUWIN");
        x_offset = 3;
    }else{
        snprintf(buf, sizeof(buf), "GAMEOVER");
        x_offset = 1;
    }
    draw_text_wrapped(buf, x_offset, 2);
    disp_update();
    sleep(1); // Hold message for one second at least
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

static void blink_string(char buf[], unsigned x, unsigned y){
    for(unsigned i=0; i<3; ++i){ // String blinks 3 times
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

    unsigned index = 0; // Current character in name
    unsigned current_letter = 0; // Current letter in letters array
    movement_y_t prev_movement = NO_MOVE_Y;

    name[0] = name[1] = name[2] = name[3] = '\0'; // Clean

    // Read each letter
    while(index < 3){
        joyinfo_t js = joy_read();
        movement_y_t movement = movement_read_y(js.y);

        // Detect movement
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

        // Save letter on button release
        static bool was_pressed = false;
        if(js.sw == J_PRESS){
            was_pressed = true;
        }else if(was_pressed && js.sw == J_NOPRESS){
            name[index++] = letters[current_letter]; // Save letter to name, got to next letter
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
        // Letter. Lower case letters will be printed as upper case
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

        // Go to next line if no more characters fit in this line
        if (x + CHAR_WIDTH > WORLD_WIDTH) {
            x = orig_x;
            y += CHAR_HEIGHT + LINE_SPACING;
        }

        // Done if no more characters fit in display
        if (y + CHAR_HEIGHT > WORLD_HEIGHT) {
            break;
        }
    }
}

static void wait_button_press(){
    while(joy_read().sw == J_NOPRESS);

    // Debouncing
    usleep(100000); // 0.1 sec
}

static void wait_button_release(){
    while(joy_read().sw == J_PRESS);

    // Debouncing
    usleep(100000); // 0.1 sec
}

static bool update_joystick(){
        joyinfo_t joystick = joy_read();

        if(check_pause(joystick)){
            return true;
        }
        if(playerDied) return false; // Nothing to check while player is dead

        {
            static jswitch_t prev_sw = J_NOPRESS; // Only shoot when pressing, not when is pressed
            if(joystick.sw == J_PRESS && prev_sw == J_NOPRESS && player_try_shoot()){ // Play shot sound if player was allowed to shoot
                playSoundFromMemory(playerShotSound, SDL_MIX_MAXVOLUME);
            }
            prev_sw = joystick.sw;
        }

        movement_x_t movement = movement_read_x(joystick.x);
        static movement_x_t prev_movement = NO_MOVE_X;
        static unsigned long long player_time_start = 0;
        double player_elapsed = (double)(get_millis() - player_time_start) / 1000;
        // Wait time depends on type of movement: fast or slow
        double player_wait_time =   movement == MOVE_LEFT_FAST || movement == MOVE_RIGHT_FAST ? FAST_MOVEMENT_WAIT_TIME :
                                    movement == MOVE_LEFT_SLOW || movement == MOVE_RIGHT_SLOW ? SLOW_MOVEMENT_WAIT_TIME :
                                    99999; // Never to be elapsed, player won't move

        if(movement != prev_movement || player_elapsed > player_wait_time){
            player_time_start = get_millis();

            switch(movement){
                case MOVE_RIGHT_SLOW:
                case MOVE_RIGHT_FAST:
                        player_move_right();
                    break;
                case MOVE_LEFT_SLOW:
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
    static unsigned long long pause_time_start = 0;
    static bool was_pressed = false;

    if(joystick.sw == J_PRESS) {
        if(!was_pressed) {
            pause_time_start = get_millis();  // Button press just started
        }
        was_pressed = true;

        double pause_elapsed = (double)(get_millis() - pause_time_start) / 1000;
        if(pause_elapsed >= BUTTON_PAUSE_TIME) {
            was_pressed = false;
            return true;  // Game paused
        }
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
            if(i>=0 && j>=0 && i<WORLD_WIDTH && j<WORLD_HEIGHT){ // Turning on LED out of bounds is an error
                disp_write((dcoord_t){.x=i, .y=j}, D_ON); // LED {i,j} turned on
            }
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
    if(!playerDied){ // Do not draw while dead
        draw_rectangle(player_get_x(), player_get_y(), player_get_x()+PLAYER_W-1, player_get_y()+PLAYER_H-1);
    }
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
            if(shield_get_lives(shield,i,j)){
                draw_rectangle(shield_get_x(shield,i,j), shield_get_y(shield,i,j), shield_get_x(shield,i,j)+SHIELD_BLOCK_W-1, shield_get_y(shield,i,j)+SHIELD_BLOCK_H-1);
            }
        }
    }
}

static void init_error(bool state, const char *name) {
  if ( !state ) {
    fprintf(stderr, "%s%s\n", MSJ_ERR_INIT, name);
    exit(-1);
  }
}

