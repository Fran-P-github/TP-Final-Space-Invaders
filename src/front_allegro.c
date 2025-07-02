/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     front_allegro.c
     25/6/25

     Description:


     
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_video.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#include "front_allegro.h"
#include "general_defines.h"
#include "back.h"
#include "menu_allegro.h"

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define MSJ_ERR_INIT "Problema al inicializar: "
#define AUDIO_SAMPLES 16
#define MAX_EVENT_WAIT_TIME 0.001
// Floats para el volumen de los efectos de sonido
#define VOLUME_PLAYER_SHOT .1
#define VOLUME_ALIENS_MOVED .3
#define VOLUME_UFO .1
// Sprites
// SPRITESHEET 2 ES DE 1160x600
#define SPRITE_ALIENS_NUM 3
#define SPRITE_ALIENS_COLORS 6 // HAY 6 COLORES EN LA PRIMER "COLUMNA" DE SPRITES
#define SPRITE_ALIENS_FRAMES 2
#define SPRITE_ALIENS_X 4
#define SPRITE_ALIENS_Y 4
#define SPRITE_ALIENS_1_2_W 80
#define SPRITE_ALIENS_3_W 72
#define SPRITE_ALIENS_H 70
#define SPRITE_SPACING 4


/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
    ALLEGRO_BITMAP* _sheet;
    ALLEGRO_BITMAP* ship;
    ALLEGRO_BITMAP* aliens[SPRITE_ALIENS_NUM][SPRITE_ALIENS_COLORS][2]; //3 tipos de aliens, 11 colores, 2 estados de animacion
    ALLEGRO_BITMAP* aliens_explotion[SPRITE_ALIENS_COLORS]; //11 colores de explosiones

}sprites_t;

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

extern const bool aliensMoved;
extern bool aliensFrame;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void draw_mothership();
static void draw_alien(unsigned i, unsigned j, unsigned sprite, unsigned color);
static void draw_player();
static void draw_player_shot();
static void draw_alien_shot();
static void draw_shield(unsigned shield);
static ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h);
static void sprites_init();
static void sprites_deinit();

// Funcion wrapper para inicializar un efecto de sonido
static void initAudioInstance(ALLEGRO_SAMPLE_INSTANCE* instance, float volume, ALLEGRO_PLAYMODE playmode);

static void init_error(bool state, const char* name);

static void kill_all();
static void kill_all_instances(int len, ...);
static void kill_all_samples(int len, ...);
static void kill_all_bitmaps(int len, ...);
static void kill_all_font(int len, ...);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static ALLEGRO_TIMER* timer;
static ALLEGRO_DISPLAY* disp;
static ALLEGRO_EVENT_QUEUE* queue;
static ALLEGRO_FONT* default_font;
static ALLEGRO_BITMAP* buffer;
static ALLEGRO_MIXER* mixer;

// Sprites
static sprites_t sprites;

// Punteros a los samples para el audio
static ALLEGRO_SAMPLE* playerShotSound = NULL;
static ALLEGRO_SAMPLE* playerDeathSound = NULL;
static ALLEGRO_SAMPLE* alienDeathSound = NULL;
static ALLEGRO_SAMPLE* alienMovedSound = NULL;
static ALLEGRO_SAMPLE* ufoSound = NULL;
static ALLEGRO_SAMPLE_INSTANCE* playerShotSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE* alienMovedSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE* ufoSample = NULL;

//keyboard
static unsigned char key[ALLEGRO_KEY_MAX];

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

game_state_t front_init(){
    back_init();


    init_error(al_init(), "Allegro");
    init_error(al_init_primitives_addon(), "Allegro Primitives");
    init_error(al_install_keyboard(), "Keyboard");
    init_error(al_init_font_addon(), "Allegro Fonts");
    init_error(al_init_ttf_addon(), "Allegro TTFs");
    init_error(al_install_mouse(), "Mouse");
    init_error(al_init_video_addon(),"Allegro Videos");
    init_error(al_install_audio(), "Allegro Audio");
    init_error(al_init_acodec_addon(), "Allegro Audio Codec");

    al_reserve_samples(AUDIO_SAMPLES);
    init_error(al_init_image_addon(), "Allegro Image Addon");

    sprites_init();

    // Se cargan los archivos de audio
    playerShotSound = al_load_sample(AUDIO_PLAYER_SHOT);
    playerDeathSound = al_load_sample(AUDIO_PLAYER_DEATH);
    alienDeathSound = al_load_sample(AUDIO_INVADER_DEATH);
    alienMovedSound = al_load_sample(AUDIO_INVADER_MOVED);
    ufoSound = al_load_sample(AUDIO_UFO);
    init_error(playerShotSound, "Audio disparo del jugador.");
    init_error(playerDeathSound, "Audio muerte del jugador.");
    init_error(alienDeathSound, "Audio muerte del alien.");
    init_error(playerDeathSound, "Audio muerte del jugador.");
    init_error(ufoSound, "Audio del OVNI.");
    
    // Se crea el mixer
    mixer = al_create_mixer(44100,ALLEGRO_AUDIO_DEPTH_FLOAT32,ALLEGRO_CHANNEL_CONF_2);
    init_error(mixer, "Mixer");
    // Se adjunta el mixer creado al mixer principal.
    al_attach_mixer_to_mixer(mixer, al_get_default_mixer());

    // Se crean instancias de samples para el disparo del jugador y para el movimiento de los aliens
    playerShotSample = al_create_sample_instance(playerShotSound);
    alienMovedSample = al_create_sample_instance(alienMovedSound);
    ufoSample = al_create_sample_instance(ufoSound);

    al_attach_sample_instance_to_mixer(playerShotSample, mixer);
    al_attach_sample_instance_to_mixer(alienMovedSample, mixer);
    al_attach_sample_instance_to_mixer(ufoSample, mixer);

    // Se setean los valores predeterminados para cada audio.
    initAudioInstance(playerShotSample, VOLUME_PLAYER_SHOT, ALLEGRO_PLAYMODE_ONCE);
    initAudioInstance(alienMovedSample, VOLUME_ALIENS_MOVED, ALLEGRO_PLAYMODE_ONCE);
    initAudioInstance(ufoSample, VOLUME_UFO, ALLEGRO_PLAYMODE_LOOP);

    al_set_new_display_flags (ALLEGRO_OPENGL | ALLEGRO_FULLSCREEN_WINDOW);

    default_font = al_create_builtin_font();
    init_error(default_font, "Font");

    timer = al_create_timer(1.0 / FRAME_RATE); // 30 FPS
    init_error(timer, "Timer");

    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

    disp = al_create_display(WORLD_WIDTH, WORLD_HEIGHT);
    init_error(disp, "Display");

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();

    buffer = al_create_bitmap(WORLD_WIDTH, WORLD_HEIGHT);
    init_error(buffer, "Buffer");

    queue = al_create_event_queue();
    init_error(queue, "Queue");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    return MENU;
}

game_state_t menu(){
    return menu_allegro(disp, timer, queue, default_font, buffer, mixer, &kill_all_bitmaps, &kill_all_instances, &kill_all_samples, &kill_all_font);
}

game_state_t game_pause(){
    return GAME;
}

void endgame(){
    kill_all();
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

//Complete...
static void kill_all(){
    al_destroy_display(disp);
    al_destroy_timer (timer);
    al_destroy_event_queue(queue);
    al_destroy_font(default_font);
    al_destroy_mixer(mixer);
    // Se matan los procesos relacionados al audio.
    kill_all_instances(
        3,                  // Cantidad de instancias a destruir.
        playerShotSample,
        alienMovedSample,
        ufoSample
    );
    kill_all_samples(
        5,                  // Cantidad de samples a destruir.
        playerShotSound,
        playerDeathSound,
        alienDeathSound,
        alienMovedSound,
        ufoSound
    );
    al_uninstall_audio();
    sprites_deinit();
}

// Funcion para matar todos los audio samples cargados.
static void kill_all_samples(int len, ...){
    va_list sample_list;
    va_start(sample_list, len);
    int i;
    for(i = 0; i < len; i++){
        al_destroy_sample(va_arg(sample_list, ALLEGRO_SAMPLE*));
    }
}

// Funcion para matar todas las instancias de audio samples creadas.
static void kill_all_instances(int len, ...){
    va_list instance_list;
    va_start(instance_list, len);
    int i;
    for(i = 0; i < len; i++){
        al_destroy_sample_instance(va_arg(instance_list, ALLEGRO_SAMPLE_INSTANCE*));
    }
}

// Funcion para matar todas los bitmaps creados.
static void kill_all_bitmaps(int len, ...){
    va_list bitmap_list;
    va_start(bitmap_list, len);
    int i;
    for(i = 0; i < len; i++){
        al_destroy_bitmap(va_arg(bitmap_list, ALLEGRO_BITMAP*));
    }
}

// Funcion para matar todas las fuentes creadas.
static void kill_all_font(int len, ...){
    va_list font_list;
    va_start(font_list, len);
    int i;
    for(i = 0; i < len; i++){
        al_destroy_font(va_arg(font_list, ALLEGRO_FONT*));
    }
}

static void initAudioInstance(ALLEGRO_SAMPLE_INSTANCE* instance, float volume, ALLEGRO_PLAYMODE playmode){
    al_set_sample_instance_playmode(instance, playmode);
    al_set_sample_instance_gain(instance, volume);
}

static void init_error(bool state, const char* name){
    if(!state){
        fprintf(stderr, "%s%s\n", MSJ_ERR_INIT, name);
        exit(-1);
    }
}

game_state_t game_update(unsigned level){
    //level_init(ALIENS_ROWS-3+level/3, ALIENS_COLUMNS-3+level/2, 1+level/3, SHIELD_BLOCK_LIVES-level/6);
    level_init(ALIENS_ROWS+level/3, ALIENS_COLUMNS+level/2, 1+level/3, SHIELD_BLOCK_LIVES-level/6);
    player_reset_on_new_level();
    if(level==0) player_reset_on_new_game();

    ALLEGRO_EVENT event;
    bool redraw = false, done = false, fullscreen = true, moveThisFrame = true, shotMade = false;
    level_state_t level_state = LEVEL_NOT_DONE;
    unsigned long long frame = 0;

    al_start_timer(timer);

    while(!done && level_state==LEVEL_NOT_DONE){
        // Procesamiento de eventos
        if(al_wait_for_event_timed(queue, &event, MAX_EVENT_WAIT_TIME)){
            switch(event.type){
                case ALLEGRO_EVENT_TIMER:
                    level_state = back_update(level);
                    redraw = true;
                    ++frame;
                    moveThisFrame = false;
                    break;

                case ALLEGRO_EVENT_KEY_DOWN:
                    key[event.keyboard.keycode] = 1;
                    if (key[ALLEGRO_KEY_ESCAPE])
                        done = true;
                    if (key[ALLEGRO_KEY_F]){
                        fullscreen = !fullscreen;
                        al_toggle_display_flag(disp,ALLEGRO_FULLSCREEN_WINDOW,fullscreen);
                    }
                    // Se utiliza X para disparar.
                    if(key[ALLEGRO_KEY_X] && player_try_shoot()){
                        al_play_sample_instance(playerShotSample);
                        shotMade = true;
                    }
                    break;
                case ALLEGRO_EVENT_KEY_UP:
                    key[event.keyboard.keycode] = 0;
                    break;

                case ALLEGRO_EVENT_DISPLAY_RESIZE:
                    al_acknowledge_resize(disp);
                    break;

                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    done = true;
                    break;
            }
        }
        // Se utilizan las flechas para mover al jugador
        if(key[ALLEGRO_KEY_RIGHT] && !moveThisFrame){ 
            player_move_right();
            moveThisFrame = true;
        }
        else if(key[ALLEGRO_KEY_LEFT] && !moveThisFrame){ 
            player_move_left();
            moveThisFrame = true;
        }

        // Reproduce el sonido cuando los aliens se mueven (si siguen vivos) se aprovecha el "laziness" de c.
        if(total_aliens_alive() && aliensMoved)
        al_play_sample_instance(alienMovedSample);

        // Para el sonido del disparo cuando se hace muy seguido
        if(!player_shot_is_used() && shotMade){
            al_stop_sample_instance(playerShotSample);
            shotMade = false;
        }
        
        if(redraw){
            redraw = false;
            al_set_target_bitmap(buffer);
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_textf(default_font, al_map_rgb(255, 255, 255), 0, 0, 0, "width: %d height: %d", al_get_display_width (disp), al_get_display_height(disp));
            unsigned i, j;
            draw_player_shot();
            unsigned alien_column_to_shoot = get_best_alien_column_to_shoot();
            if(alien_column_to_shoot >= 0){
                alien_try_shoot(alien_column_to_shoot);
            }
            draw_alien_shot();
            for (int x = 0; x < SHIELDS_CANT; x++){
                draw_shield(x);
            }
            draw_player();
            int alienSprite = SPRITE_ALIENS_NUM;
            for(i=0; i<ALIENS_ROWS; ++i){
                if(alienSprite>0 && i%2==0){
                    alienSprite--;
                }
                for(j=0; j<ALIENS_COLUMNS; ++j){
                    if(aliens_is_alive(i,j)){
                        draw_alien(i, j, alienSprite, 0);
                    }
                }
            }
            if(mothership_is_active()){
                al_play_sample_instance(ufoSample);
                draw_mothership();
            }
            else{
                al_stop_sample_instance(ufoSample);
            }
            al_set_target_backbuffer(disp);
            al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0, al_get_display_width (disp), al_get_display_height(disp), 0);                           // flags
            al_flip_display();
        }
    }

    if(level_state == PLAYER_WINS){
        return GAME;
    }else{
        return CLOSED;
    }
}

static void draw_mothership(){
    al_draw_filled_rectangle(mothership_get_x(), mothership_get_y(), mothership_get_x()+MOTHERSHIP_W-1, mothership_get_y()+MOTHERSHIP_H-1, al_map_rgb(128, 0, 255));
}

static void draw_alien(unsigned i, unsigned j, unsigned sprite, unsigned color){
    int alienX = aliens_get_x(i, j), alienY = aliens_get_y(i, j);
    ALLEGRO_BITMAP* alienSprite = sprites.aliens[sprite][color][aliensFrame];
    int srcWidth = al_get_bitmap_width(alienSprite);
    int srcHeight = al_get_bitmap_height(alienSprite);
    al_draw_rectangle(alienX, alienY, alienX+ALIENS_W, alienY+ALIENS_H, al_map_rgb(255, 0, 0), 1);
    al_draw_scaled_bitmap(alienSprite, 0, 0, srcWidth, srcHeight, alienX, alienY, ALIENS_W, ALIENS_H, 0);
}

static void draw_player(){
    al_draw_filled_rectangle(player_get_x(), player_get_y(), player_get_x()+PLAYER_W, player_get_y()+PLAYER_H, al_map_rgb(0, 255, 0));
}

static void draw_alien_shot(){
    if(alien_shot_is_used())
        al_draw_filled_rectangle(alien_shot_get_x(), alien_shot_get_y(), alien_shot_get_x()+SHOT_W, alien_shot_get_y()+SHOT_H, al_map_rgb(255, 255, 255));
}

static void draw_player_shot(){
    if(player_shot_is_used())
        al_draw_filled_rectangle(player_shot_get_x(), player_shot_get_y(), player_shot_get_x()+SHOT_W, player_shot_get_y()+SHOT_H, al_map_rgb(255, 255, 255));
}

static void draw_shield(unsigned shield){
    unsigned i, j;
    for(i=0; i<SHIELD_H; ++i){
        for(j=0; j<SHIELD_W; ++j){
            if(shield_get_lives(shield, i, j))
            al_draw_filled_rectangle(shield_get_x(shield,i,j), shield_get_y(shield,i,j), shield_get_x(shield,i,j)+SHIELD_BLOCK_W, shield_get_y(shield,i,j)+SHIELD_BLOCK_H, al_map_rgb(255, 255, 255));
        }
    }
}

static ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h){
    ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(sprites._sheet, x, y, w, h);
    init_error(sprite, "sprite grab");
    return sprite;
}

static void sprites_init(){
    sprites._sheet = al_load_bitmap(SPRITESHEET2);
    init_error(sprites._sheet, "spritesheet");
    for(int j = 0; j < SPRITE_ALIENS_COLORS; j++){
        int width = SPRITE_ALIENS_1_2_W;
        int spacingX = 0, spacingY = 0, spriteNum = 0, lastWidth = 0;
        // A partir de la segunda fila de sprites en adelante
        if(j > 0) spacingY = SPRITE_SPACING;
        for(int i = 0; i < SPRITE_ALIENS_NUM; i++){
            // Cuando se llega al 3er tipo de alien
            for(int frame = 0; frame < SPRITE_ALIENS_FRAMES; frame++, spriteNum++){
                if(i >= 2) width = SPRITE_ALIENS_3_W;
                if(frame > 0) spacingX = SPRITE_SPACING;
                sprites.aliens[i][j][frame] = sprite_grab(
                    SPRITE_ALIENS_X+spriteNum*(lastWidth+spacingX),
                    SPRITE_ALIENS_Y+j*SPRITE_ALIENS_H+spacingY,
                    width,
                    SPRITE_ALIENS_H
                );
                printf("al[%d][%d][%d]: (%d, %d, %d, %d), spriteNum: %d\n",
                    i, j, frame,
                    SPRITE_ALIENS_X+spriteNum*(lastWidth+spacingX),
                    SPRITE_ALIENS_Y+j*SPRITE_ALIENS_H+spacingY,
                    width,
                    SPRITE_ALIENS_H,
                    spriteNum
                );
                lastWidth = width;
            }
        }
    }

    // MANUAL TESTING
    // sprites.aliens[0][1][0] = sprite_grab(4, 4, 80, 70);
    // sprites.aliens[0][1][1] = sprite_grab(88, 4, 80, 70);
    // sprites.aliens[1][1][0] = sprite_grab(172, 4, 80, 70);
    // sprites.aliens[1][1][1] = sprite_grab(256, 4, 80, 70);
    // sprites.aliens[2][1][0] = sprite_grab(340, 4, 72, 70);
    // sprites.aliens[2][1][1] = sprite_grab(416, 4, 72, 70);
}

static void sprites_deinit(){
    al_destroy_bitmap(sprites._sheet);
}