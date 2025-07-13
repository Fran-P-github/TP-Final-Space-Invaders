/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     front_allegro.c
     25/6/25

     Description:



  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <allegro5/allegro5.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_video.h>
#include <stdarg.h>
#include <stdio.h>

#include "back.h"
#include "scores.h"
#include "front.h"
#include "general_defines.h"
#include "menu_allegro.h"

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define INIT_SOUND(p, sample, volume, playmode, mixer)    \
  p = al_create_sample_instance(al_load_sample(sample));  \
  init_error(p, #p);                                      \
  al_attach_sample_instance_to_mixer(p, mixer);           \
  initAudioInstance(p, volume, playmode);

#define MSJ_ERR_INIT "Problema al inicializar: "
#define AUDIO_SAMPLES 16
#define MAX_EVENT_WAIT_TIME 0.001
// Floats for the volume of sound effects 
#define VOLUME_PLAYER_SHOT .1
#define VOLUME_PLAYER_DEATH .3
#define VOLUME_ALIENS_MOVED .3
#define VOLUME_ALIENS_DEATH .3
#define VOLUME_ALIENS_HIT .5
#define VOLUME_SHIELD_HIT .5
#define VOLUME_BULLET_HIT .5
#define VOLUME_UFO .1
#define VOLUME_UFO_DEATH .3
#define VOLUME_PAUSE .5
// Sprites
#define MOTHERSHIP_SCALE_X 40
#define MOTHERSHIP_SCALE_Y 35
#define ALIEN_SCALE_X 40
#define ALIEN_SCALE_Y 40
#define SPRITE_ALIENS_NUM 3
#define SPRITE_SHOT_FRAMES 6
#define SPRITE_SHOT_W 3
#define SPRITE_SHOT_H 12
#define SPRITE_SHOT_SPACING 1
#define SPRITE_SHOT_NUM 2

#define STARS_N ((WORLD_WIDTH / 2) - 1)
#define STAR_MIN_SPEED 2.2
#define STAR_MAX_SPEED 8.4
#define STAR_MARGIN 1.5
#define STAR_SEPARATION ((WORLD_WIDTH - 2 * STAR_MARGIN) / (STARS_N - 1))


/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {
    ALIEN_GREEN = 0,
    ALIEN_PINK,
    ALIEN_WHITE,
    ALIEN_GOLD,
    ALIEN_NEON,
    ALIEN_GREY,
    ALIEN_ORANGE,
    ALIEN_LPINK,
    ALIEN_YELLOW,
    ALIEN_SILVER,
    ALIEN_RETRO,
    ALIEN_TOTAL_COLORS
} alien_color_t;

typedef enum {
  UFO_GREEN = 0,
  UFO_ORANGE,
  UFO_PINK,
  UFO_LPINK,
  UFO_WHITE,
  UFO_YELLOW,
  UFO_GOLD,
  UFO_SILVER,
  UFO_NEON,
  UFO_RETRO,
  UFO_GREY,
  UFO_TOTAL_COLORS
} mothership_color_t;

typedef struct{
    float y; // x coordinate depends on star index in array
    float speed;
    ALLEGRO_COLOR color;
} star_t;

typedef struct {
  ALLEGRO_BITMAP *_sheet;
  ALLEGRO_BITMAP *_sheet_shot;
  ALLEGRO_BITMAP *ship;
  ALLEGRO_BITMAP *aliens[SPRITE_ALIENS_NUM][ALIEN_TOTAL_COLORS][2]; // 3 types of aliens, 11 colours, 2 animation states
  ALLEGRO_BITMAP *aliens_explotion[ALIEN_TOTAL_COLORS];             // 11 colours of explosions
  ALLEGRO_BITMAP *shot[SPRITE_SHOT_NUM][SPRITE_SHOT_FRAMES];        // 2 types of shots, 6 animation states  
  ALLEGRO_BITMAP *ufo[UFO_TOTAL_COLORS][2];                         // 11 mothership colors, 2 animation states 
} sprites_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

// Background drawing
static float get_random_star_speed();
static void background_init();
static void background_update();
static void draw_background();
static ALLEGRO_COLOR random_star_color();

// HUD drawing
static void draw_hud(unsigned level, ALLEGRO_FONT* font);

// Frame drawing and sounds
static void process_frame(unsigned long long frame, unsigned level, bool player_shot_made, explosion_t explosion, int explosion_interval, ALLEGRO_FONT* hud_font);

static void draw_mothership(mothership_color_t color);
static void draw_alien(unsigned i, unsigned j, unsigned sprite, alien_color_t color, unsigned char aliensFrame);
static void draw_player();
static void draw_player_shot(unsigned frame, unsigned color);
static void draw_alien_shot(unsigned frame, unsigned color);
static void draw_shield(unsigned shield);
static void draw_explosion(explosion_t explosion, unsigned color);
static ALLEGRO_BITMAP *sprite_grab(ALLEGRO_BITMAP* father, int x, int y, int w, int h);
static void sprites_init();
static void sprites_deinit();

// Wrapper function to initialize a sound effect
static void initAudioInstance(ALLEGRO_SAMPLE_INSTANCE *instance, float volume, ALLEGRO_PLAYMODE playmode);

static void init_error(bool state, const char *name);

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


star_t stars[STARS_N];


static bool fullscreen = true;
static ALLEGRO_TIMER *timer;
static ALLEGRO_DISPLAY *disp;
static ALLEGRO_EVENT_QUEUE *queue;
static ALLEGRO_FONT *default_font;
static ALLEGRO_BITMAP *buffer;
static ALLEGRO_MIXER *mixer;

// Sprites
static sprites_t sprites;

// Pointers to the audio samples
static ALLEGRO_SAMPLE_INSTANCE *playerShotSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *alienMovedSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *ufoSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *ufoDeathSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *alienDeathSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *alienHitSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *playerDeathSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *shieldHitSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *bulletHitSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *pauseSample = NULL;

// keyboard
static unsigned char key[ALLEGRO_KEY_MAX];

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

game_state_t front_init() {
  back_init();

  init_error(al_init(), "Allegro");
  init_error(al_init_primitives_addon(), "Allegro Primitives");
  init_error(al_install_keyboard(), "Keyboard");
  init_error(al_init_font_addon(), "Allegro Fonts");
  init_error(al_init_ttf_addon(), "Allegro TTFs");
  init_error(al_install_mouse(), "Mouse");
  init_error(al_init_video_addon(), "Allegro Videos");
  init_error(al_install_audio(), "Allegro Audio");
  init_error(al_init_acodec_addon(), "Allegro Audio Codec");
  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

  al_reserve_samples(AUDIO_SAMPLES);
  init_error(al_init_image_addon(), "Allegro Image Addon");

  sprites_init();

  // Mixer is created
  mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
  init_error(mixer, "Mixer");
  // The created mixer is attached to the main mixer.
  al_attach_mixer_to_mixer(mixer, al_get_default_mixer());

  // Audio files are loaded
  INIT_SOUND(playerShotSample, AUDIO_PLAYER_SHOT, VOLUME_PLAYER_SHOT, ALLEGRO_PLAYMODE_ONCE, mixer)
  INIT_SOUND(playerDeathSample, AUDIO_PLAYER_DEATH, VOLUME_PLAYER_DEATH, ALLEGRO_PLAYMODE_ONCE, mixer)
  INIT_SOUND(alienDeathSample, AUDIO_INVADER_DEATH, VOLUME_ALIENS_DEATH, ALLEGRO_PLAYMODE_ONCE, mixer)
  INIT_SOUND(alienMovedSample, AUDIO_INVADER_MOVED, VOLUME_ALIENS_MOVED, ALLEGRO_PLAYMODE_ONCE, mixer)
  INIT_SOUND(alienHitSample, AUDIO_INVADER_HIT, VOLUME_ALIENS_HIT, ALLEGRO_PLAYMODE_ONCE, mixer)
  INIT_SOUND(shieldHitSample, AUDIO_SHIELD_HIT, VOLUME_SHIELD_HIT, ALLEGRO_PLAYMODE_ONCE, mixer)
  INIT_SOUND(bulletHitSample, AUDIO_BULLET_HIT, VOLUME_BULLET_HIT, ALLEGRO_PLAYMODE_ONCE, mixer)
  INIT_SOUND(ufoDeathSample, AUDIO_UFO_DEATH, VOLUME_UFO_DEATH, ALLEGRO_PLAYMODE_ONCE, mixer)
  INIT_SOUND(ufoSample, AUDIO_UFO, VOLUME_UFO, ALLEGRO_PLAYMODE_LOOP, mixer)
  INIT_SOUND(pauseSample, AUDIO_PAUSE, VOLUME_PAUSE, ALLEGRO_PLAYMODE_ONCE, mixer)



  al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_FULLSCREEN_WINDOW);

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

game_state_t menu() {
  return menu_allegro(disp, timer, queue, default_font, buffer, mixer, &kill_all_bitmaps, &kill_all_instances, &kill_all_samples, &kill_all_font);
}

// Borders are grey (no hover) and white (hover)
#define CREATE_BUTTON_GAME_PAUSE(color_normal, color_hover) create_button(color_normal, color_normal, GRADIENT_CENTER, color_hover, color_hover, GRADIENT_CENTER, al_map_rgb(200, 200, 200), al_map_rgb(255,255,255), 4, 4, 0.,0., 0.,button_h+0., button_w+0.,button_h+0., button_w+0.,0.)
game_state_t game_pause(unsigned int* level, bool* new_level) {
    al_play_sample_instance(pauseSample); // Play pause sound
    al_stop_timer(timer); // Pause timer while in pause
    al_show_mouse_cursor(disp);

    bool deinit_font_on_exit = true;
    ALLEGRO_FONT *pause_font = al_load_ttf_font(FONT_ROUTE("supercharge-font/Supercharge_halftone.otf"), 28, 0);
    if(!pause_font){
      pause_font = default_font;
      deinit_font_on_exit = false;
    }

    const int space = WORLD_HEIGHT / 6;
    const int button_w = WORLD_WIDTH / 3;
    const int button_h = WORLD_HEIGHT / 12;
    const int start_y = WORLD_HEIGHT / 3; // This is not centered

    const char *labels[] = { "Resume", "Replay", "Main Menu", "Exit" };

    ALLEGRO_COLOR colors[4][2] = {
      // "Resume" - blue
      { al_map_rgb(  80,  80, 200), al_map_rgb(120, 120, 255) },
      // Replay - purple
      { al_map_rgb(  200,  80, 200), al_map_rgb(255, 120, 255) },
      // "Main Menu" - green
      { al_map_rgb(  60, 200,  60), al_map_rgb(100, 255, 100) },
      // "Exit" - red
      { al_map_rgb(200,  60,  60), al_map_rgb(255, 100, 100) }
    };

    button_t buttons[4] = {
      CREATE_BUTTON_GAME_PAUSE(colors[0][0], colors[0][1]),
      CREATE_BUTTON_GAME_PAUSE(colors[1][0], colors[1][1]),
      CREATE_BUTTON_GAME_PAUSE(colors[2][0], colors[2][1]),
      CREATE_BUTTON_GAME_PAUSE(colors[3][0], colors[3][1])
    };
    for(int i = 0; i < 4; ++i){
      buttons[i].position_x = WORLD_WIDTH / 2;
      buttons[i].position_y = start_y + i * (button_h + space / 2);
    }

    ALLEGRO_EVENT ev;
    bool done = false;
    game_state_t result = GAME;

    // Draw background
    al_set_target_bitmap(buffer);    
    al_draw_filled_rectangle(0, 0, WORLD_WIDTH-1, WORLD_HEIGHT-1, al_map_rgba(0, 0, 0, 128));

    while (!done) {
        al_set_target_bitmap(buffer);
        ALLEGRO_MOUSE_STATE ms; al_get_mouse_state(&ms);

        // Show player info
        al_draw_textf(pause_font, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, space, ALLEGRO_ALIGN_CENTER,
                      "Score: %d    Lives: %d    Level: %d",
                      player_get_score(), player_get_lives(), (*level)+1); // First level is level 0

        // Draw buttons
        for(int i = 0; i < 4; ++i) {
            draw_button(&ms, al_get_display_width(disp), al_get_display_height(disp), &buttons[i]);
            draw_smart_text(&ms, al_get_display_width(disp), al_get_display_height(disp), &buttons[i], pause_font, al_map_rgb(230, 230, 230), al_map_rgb(30, 30, 30), ALLEGRO_ALIGN_CENTER, labels[i]);
        }

        // Draw to screen
        al_set_target_backbuffer(disp);
        al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0,
                              al_get_display_width(disp), al_get_display_height(disp), 0);
        al_flip_display();

        // Wait for event
        while (al_get_next_event(queue, &ev)) {
          switch(ev.type){
            case ALLEGRO_EVENT_MOUSE_AXES: 
              continue;
              break;

            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
              for(int i = 0; i < 4; ++i) {
                    if (mouse_hover_button(&buttons[i], &ms, al_get_display_width(disp), al_get_display_height(disp))) {
                        switch(i){
                          case 0:
                            result = GAME;
                            break;
                          case 1:
                            *level = 0;
                            *new_level = true;
                            result = GAME;
                            break;
                          case 2:
                            result = MENU;
                            break;
                          case 3:
                            result = CLOSED;
                            break;
                        }
                        done = true;
                        break;
                    }
                }
                break;

              case ALLEGRO_EVENT_KEY_DOWN:
                switch(ev.keyboard.keycode){
                  case ALLEGRO_KEY_ESCAPE:
                    result = GAME;
                    done = true;
                    break;
                  case ALLEGRO_KEY_F:
                    fullscreen = !fullscreen;
                    al_toggle_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, fullscreen);
                }
                break;
          }
        }
    }

    if(deinit_font_on_exit) al_destroy_font(pause_font);
    al_start_timer(timer); // Resume timer after pause
    al_flush_event_queue(queue); // Flush queue to give it back empty to game_update
    al_clear_to_color(al_map_rgb(0, 0, 0)); // Clear screen on exit
    al_flip_display();
    al_play_sample_instance(pauseSample); // Play pause sound
    return result;
}

game_state_t endgame() {
    game_state_t result;

    char name[NAME_LEN + 1] = "";
    int name_len = 0;
    int score = player_get_score();

    bool deinit_font_on_exit = true;
    ALLEGRO_FONT *font_endgame = al_load_ttf_font(FONT_ROUTE("toreks-font/Toreks_regular.ttf"), 32, 0);
    if(!font_endgame){
      font_endgame = default_font;
      deinit_font_on_exit = false;
    }
    const int space_between_lines = WORLD_HEIGHT/8;

    // Capture of last game screen
    ALLEGRO_BITMAP *background = al_create_bitmap(WORLD_WIDTH, WORLD_HEIGHT);
    al_set_target_bitmap(background);
    al_draw_bitmap(buffer, 0, 0, 0);

    bool done = false;
    unsigned frame = 0;
    ALLEGRO_EVENT ev;
    al_flush_event_queue(queue);
    while (!done) {
        al_wait_for_event(queue, &ev);
        if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) continue; // Ignore mouse movement
        if(ev.type == ALLEGRO_EVENT_TIMER) frame++;

        // Redraw
        al_set_target_bitmap(buffer);
        al_draw_bitmap(background, 0, 0, 0);
        al_draw_filled_rectangle(0, 0, WORLD_WIDTH-1, WORLD_HEIGHT-1, al_map_rgba(0, 0, 0, 160));
        if(frame > FRAME_RATE)
          al_draw_text(font_endgame, al_map_rgb(200, 30, 30), WORLD_WIDTH/2, space_between_lines, ALLEGRO_ALIGN_CENTER, "GAME     OVER");
        if(frame > 2*FRAME_RATE)
          al_draw_textf(font_endgame, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, 2*space_between_lines, ALLEGRO_ALIGN_CENTER,
                      "Score: %d", score);
        if(frame > 3*FRAME_RATE)
          al_draw_text(font_endgame, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, 3*space_between_lines, ALLEGRO_ALIGN_CENTER,
                      "Enter your name:");
        al_draw_text(font_endgame, al_map_rgb(255, 255, 0), WORLD_WIDTH/2, 4*space_between_lines, ALLEGRO_ALIGN_CENTER,
                      name);
        al_set_target_backbuffer(disp);
        al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0, al_get_display_width(disp), al_get_display_height(disp), 0);
        al_flip_display();

        if(frame < 3*FRAME_RATE) continue;

        // Get keyboard input
        if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER && name[0]) {
                done = true;
            } else if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && name_len > 0) {
                name[--name_len] = '\0';
            } else if (name_len < NAME_LEN && ev.keyboard.unichar >= 32 && ev.keyboard.unichar <= 126) {
                name[name_len++] = ev.keyboard.unichar;
                name[name_len] = '\0';
            }
        }
    }

    // Show final message
    al_set_target_bitmap(buffer);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_textf(font_endgame, al_map_rgb(0, 255, 0), WORLD_WIDTH/2, 2*space_between_lines, ALLEGRO_ALIGN_CENTER,
                  "Thanks for playing, %s!", name);
    // Check hi-scores and save score if it is a new best
    highscore_t top_scores[MAX_SCORES]; load_scores(top_scores);
    if( try_insert_score(top_scores, name, player_get_score()) ){
        al_draw_text(font_endgame, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, 3*space_between_lines, ALLEGRO_ALIGN_CENTER,
                  "Congratulations! You just made a new best!");
    }
    save_scores(top_scores);
    al_set_target_backbuffer(disp);
    al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0, al_get_display_width(disp), al_get_display_height(disp), 0); 
    al_flip_display();

    // Show buttons to decide what to do next
    al_show_mouse_cursor(disp);
    const int space = WORLD_WIDTH / 8;
    const int button_w = WORLD_WIDTH / 4.5;
    const int button_h = WORLD_HEIGHT / 12;
    const int total_width = 3 * button_w + 2 * space;
    const int start_x = (WORLD_WIDTH - total_width) / 2 + button_w / 2;

    const char *labels[] = { "Replay", "Main Menu", "Exit" };

    ALLEGRO_COLOR colors[3][2] = {
      // "Replay" - blue
      { al_map_rgb(  80,  80, 200), al_map_rgb(120, 120, 255) },
      // "Main Menu" - green
      { al_map_rgb(  60, 200,  60), al_map_rgb(100, 255, 100) },
      // "Exit" - red
      { al_map_rgb(200,  60,  60), al_map_rgb(255, 100, 100) }
    };

    button_t buttons[3] = { // Buttons will have the same format as the ones in pause menu
      CREATE_BUTTON_GAME_PAUSE(colors[0][0], colors[0][1]),
      CREATE_BUTTON_GAME_PAUSE(colors[1][0], colors[1][1]),
      CREATE_BUTTON_GAME_PAUSE(colors[2][0], colors[2][1])
    };
    for(int i = 0; i < 3; ++i){
      buttons[i].position_x = start_x + i * (button_w + space);
      buttons[i].position_y = WORLD_HEIGHT - WORLD_HEIGHT / 6;
    }
    
    done = false;
    while (!done) {
        al_set_target_bitmap(buffer);
        ALLEGRO_MOUSE_STATE ms; al_get_mouse_state(&ms);

        // Draw buttons
        for(int i = 0; i < 3; ++i) {
            draw_button(&ms, al_get_display_width(disp), al_get_display_height(disp), &buttons[i]);
            draw_smart_text(&ms, al_get_display_width(disp), al_get_display_height(disp), &buttons[i], font_endgame, al_map_rgb(230, 230, 230), al_map_rgb(30, 30, 30), ALLEGRO_ALIGN_CENTER, labels[i]);
        }

        // Draw to screen
        al_set_target_backbuffer(disp);
        al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0,
                              al_get_display_width(disp), al_get_display_height(disp), 0);
        al_flip_display();

        // Wait for event
        while (al_get_next_event(queue, &ev)) {
          switch(ev.type){
            case ALLEGRO_EVENT_MOUSE_AXES:
              continue; // Ignore mouse movement
              break;

            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
              for(int i = 0; i < 3; ++i) {
                  if (mouse_hover_button(&buttons[i], &ms, al_get_display_width(disp), al_get_display_height(disp))) {
                      switch(i){
                        case 0:
                          result = GAME;
                          break;
                        case 1:
                          result = MENU;
                          break;
                        case 2:
                          result = CLOSED;
                          break;
                      }
                      done = true;
                      break;
                  }
              }
              break;

              case ALLEGRO_EVENT_KEY_DOWN:
                switch(ev.keyboard.keycode){
                  case ALLEGRO_KEY_ESCAPE:
                    result = GAME;
                    done = true;
                    break;
                  case ALLEGRO_KEY_F:
                    fullscreen = !fullscreen;
                    al_toggle_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, fullscreen);
                    break;
                }
                break;
          }
        }
    }

    if(deinit_font_on_exit) al_destroy_font(font_endgame);
    al_hide_mouse_cursor(disp);
    al_clear_to_color(al_map_rgb(0,0,0));
    al_flip_display();
    al_play_sample_instance(pauseSample); // Same sound as pause when exiting endgame scene
    return result;
}

// Complete...
void front_deinit() {
  al_destroy_display(disp);
  al_destroy_timer(timer);
  al_destroy_event_queue(queue);
  al_destroy_font(default_font);
  al_destroy_mixer(mixer);
  // The audio-related instances are destroyed. 
  kill_all_instances(
      9, // Amount of instances to destroy.
      playerShotSample,
      playerDeathSample,
      alienMovedSample,
      alienDeathSample,
      alienHitSample,
      shieldHitSample,
      ufoDeathSample,
      ufoSample,
      pauseSample
    );
  al_uninstall_audio();
  sprites_deinit();
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

// Function to destroy all audio samples.
static void kill_all_samples(int len, ...) {
  va_list sample_list;
  va_start(sample_list, len);
  int i;
  for ( i = 0; i < len; i++ ) {
    al_destroy_sample(va_arg(sample_list, ALLEGRO_SAMPLE *));
  }
}

// Function to destroy all audio instances.
static void kill_all_instances(int len, ...) {
  va_list instance_list;
  va_start(instance_list, len);
  int i;
  for ( i = 0; i < len; i++ ) {
    al_destroy_sample_instance(va_arg(instance_list, ALLEGRO_SAMPLE_INSTANCE *));
  }
}

// Function to destroy all bitmaps.
static void kill_all_bitmaps(int len, ...) {
  va_list bitmap_list;
  va_start(bitmap_list, len);
  int i;
  for ( i = 0; i < len; i++ ) {
    al_destroy_bitmap(va_arg(bitmap_list, ALLEGRO_BITMAP *));
  }
}

// Function to destroy all fonts.
static void kill_all_font(int len, ...) {
  va_list font_list;
  va_start(font_list, len);
  int i;
  for ( i = 0; i < len; i++ ) {
    al_destroy_font(va_arg(font_list, ALLEGRO_FONT *));
  }
}

static void initAudioInstance(ALLEGRO_SAMPLE_INSTANCE *instance, float volume, ALLEGRO_PLAYMODE playmode) {
  al_set_sample_instance_playmode(instance, playmode);
  al_set_sample_instance_gain(instance, volume);
}

static void init_error(bool state, const char *name) {
  if ( !state ) {
    fprintf(stderr, "%s%s\n", MSJ_ERR_INIT, name);
    exit(-1);
  }
}

game_state_t game_update(unsigned level, bool new_level) {
  if(new_level){ // Restart on new level
    level_init(level, ALIENS_ROWS - 3 + level / 3, ALIENS_COLUMNS - 2 + level / 2, 1 + level / 4, 4 + level / 3, SHIELD_BLOCK_LIVES - level / 6);
    if ( level == 0 ) player_reset_on_new_game();
    background_init();
  }

  ALLEGRO_FONT *hud_font = al_load_ttf_font(FONT_ROUTE("supercharge-font/Supercharge_halftone.otf"), 26, 0);
  if(!hud_font) hud_font = default_font;
  ALLEGRO_EVENT event;
  bool redraw = false, done = false, moveThisFrame = true, player_shot_made = false;
  level_state_t level_state = LEVEL_NOT_DONE;
  unsigned long long frame = 0;
  explosion_t explosion;
  int explosion_interval = 0;

  al_start_timer(timer);
  al_hide_mouse_cursor(disp);
  memset(key, 0, sizeof(key)); // Clear keys mask for going back to game

  while ( !done && level_state == LEVEL_NOT_DONE ) {
    // Events processing
    if ( al_wait_for_event_timed(queue, &event, MAX_EVENT_WAIT_TIME) ) {
      switch ( event.type ) {
        case ALLEGRO_EVENT_TIMER:
          background_update();
          level_state = back_update(level, new_level);
          new_level = false;
          if(!(frame%2)) explosion_interval--;
          if(explosion_interval <= 0){
            explosion.type = NO_EXPLOSION;
            explosion_interval = 0;
          }
          if(get_explosion_state(&explosion)){
            explosion_interval = 5;
          }

          redraw = true;
          ++frame;
          moveThisFrame = false;
          break;

        case ALLEGRO_EVENT_KEY_DOWN:
          key[event.keyboard.keycode] = 1;
          if ( key[ALLEGRO_KEY_ESCAPE] ){
            done = true;
            al_stop_sample_instance(ufoSample); // Stop mothership sound
            return PAUSE;
          }
          if ( key[ALLEGRO_KEY_F] ) {
            fullscreen = !fullscreen;
            al_toggle_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, fullscreen);
          }
          // X is used to shoot.
          if ( key[ALLEGRO_KEY_X] && player_try_shoot() ) {
            al_play_sample_instance(playerShotSample);
            player_shot_made = true;
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
    // Use arrow keys to move the player
    if ( key[ALLEGRO_KEY_RIGHT] && !moveThisFrame ) {
      player_move_right();
      moveThisFrame = true;
    } else if ( key[ALLEGRO_KEY_LEFT] && !moveThisFrame ) {
      player_move_left();
      moveThisFrame = true;
    }

    if ( redraw ) {
      redraw = false;
      process_frame(frame, level, player_shot_made, explosion, explosion_interval, hud_font);
    }

      
  }

  al_stop_sample_instance(ufoSample); // Stop mothership sound
  if ( level_state == PLAYER_WINS ) {
    return GAME;
  } else {
    return ENDGAME;
  }
}

static void process_frame(unsigned long long frame, unsigned level, bool player_shot_made, explosion_t explosion, int explosion_interval, ALLEGRO_FONT* hud_font){
  al_set_target_bitmap(buffer);
  al_clear_to_color(al_map_rgb(0, 0, 0));
  draw_background();

  static unsigned shotFrame = 0, playerShotColor = 0, alienShotColor = 0;
  static bool frameDecrement = false;
  static int explosionOpacity = 255;
  if(frame == 0){
    shotFrame = 0;
    playerShotColor = 0;
    alienShotColor = 0;
    frameDecrement = false;
    explosionOpacity = 255;
  }

  // Plays sound effect whenever the aliens move
  if ( total_aliens_alive() && aliensMoved ){
    al_play_sample_instance(alienMovedSample);
  }
  // Plays sound effect whenever an alien gets hit (but not killed)
  if(alienWasHit){
    al_play_sample_instance(alienHitSample);
  }
  // Stops player shot sound effect on collition
  if ( !player_shot_is_used() && player_shot_made ) {
    al_stop_sample_instance(playerShotSample);
    player_shot_made = false;
  }

  // HUD
  draw_hud(level, hud_font);

  // Shot animation frames handling (both alien and player)
  if(frame%3 == 0){
    if(shotFrame >= SPRITE_SHOT_FRAMES - 1) frameDecrement = true;
    else if (shotFrame <= 0) frameDecrement = false;
    frameDecrement ? shotFrame-- : shotFrame++;
    alienShotColor += 40;
    playerShotColor += 30;
    explosionOpacity -= 40;
  }
  if(!playerDied) explosionOpacity = 255;
  else if(explosionOpacity <= 0) explosionOpacity = 0;
  // Max color value for player shot
  if(!player_shot_is_used()) playerShotColor = 0;
  else if(playerShotColor >= 255) playerShotColor = 255;
  // Max color value for alien shot
  if(!alien_shot_is_used()) alienShotColor = 0;
  else if(alienShotColor >= 255) alienShotColor = 255;
  unsigned i, j;
  draw_player_shot(shotFrame, playerShotColor);
  unsigned alien_column_to_shoot = get_best_alien_column_to_shoot();
  if ( alien_column_to_shoot >= 0 && !(frame%10)) {
    alien_try_shoot(alien_column_to_shoot);
  }
  draw_alien_shot(shotFrame, alienShotColor);
  for ( int x = 0; x < SHIELDS_CANT; x++ ) {
    draw_shield(x);
  }
  if(shieldWasHit){
    al_play_sample_instance(shieldHitSample);
  }
  draw_player();

  // Player death explosion
  if(playerDied){
    ALLEGRO_BITMAP* sprite = sprites.aliens_explotion[ALIEN_SILVER];
    int srcWidth = al_get_bitmap_width(sprite), srcHeight = al_get_bitmap_height(sprite);
    al_draw_tinted_scaled_bitmap(
      sprite,
      al_map_rgba(255, 255, 255, explosionOpacity),
      0, 0, srcWidth, srcHeight,
      player_get_x(), player_get_y(), PLAYER_W, PLAYER_H,
      0
    );
    al_play_sample_instance(playerDeathSample);
  }

  {
    int alienSprite = SPRITE_ALIENS_NUM;
    static unsigned char aliensFrame = 0;
    if(aliensMoved) aliensFrame = !aliensFrame;

    for ( i = 0; i < ALIENS_ROWS; ++i ) {
      if ( alienSprite > 0 && i % 2 == 0 ) {
        alienSprite--;
      }
      for ( j = 0; j < ALIENS_COLUMNS; ++j ) {
        if ( aliens_is_alive(i, j) ) {
          alien_color_t alienColor;
          switch(aliens_get_lives(i, j)){
            case 1:
              alienColor = ALIEN_LPINK;
              break;
            case 2:
              alienColor = ALIEN_YELLOW;
              break;
            case 3:
              alienColor = ALIEN_ORANGE;
              break;
            case 4:
              alienColor = ALIEN_GREEN;
              break;
            case 5:
              alienColor = ALIEN_GREY;
              break;
            default:
              alienColor = ALIEN_RETRO; // Retro for any number over 5
              break;
          }
          switch(aliens_get_points(i,j)){
            case ALIEN_POINTS_SILVER:
              alienColor = ALIEN_SILVER;
              break;
            case ALIEN_POINTS_GOLD:
              alienColor = ALIEN_GOLD;
              break;
            case ALIEN_POINTS_NEON:
              alienColor = ALIEN_NEON;
              break;
          }
          draw_alien(i, j, alienSprite, alienColor, aliensFrame);
        }
      }
    }
  }

  if(explosion.type == ALIEN_EXPLOSION && explosion_interval > 0){
    alien_color_t color;
    switch(explosion_interval){
      case 5:
      case 4:
        color = ALIEN_ORANGE;
        break;
      case 3:
      case 2:
        color = ALIEN_WHITE;
        break;
      case 1:
        color = ALIEN_GREY;
        break;
      default: // Never to be reached
        color = ALIEN_PINK;
        break;
    }
    draw_explosion(explosion, color);
  }

  {
    mothership_color_t color;
    switch(mothership_get_points()){
      case MOTHERSHIP_POINTS_SILVER:
        color = UFO_SILVER;
        break;
      case MOTHERSHIP_POINTS_GOLD:
        color = UFO_GOLD;
        break;
      case MOTHERSHIP_POINTS_NEON:
        color = UFO_NEON;
        break;
      default: // Never to be reached
        color = UFO_PINK;
        break;
    }
    if ( mothership_is_active() ) {
      al_play_sample_instance(ufoSample);
      draw_mothership(color);
    } else {
      al_stop_sample_instance(ufoSample);
      if(explosion.type == UFO_EXPLOSION && explosion_interval > 0){
        draw_explosion(explosion, color);
      }
    }
  }

  al_set_target_backbuffer(disp);
  al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0, al_get_display_width(disp), al_get_display_height(disp), 0); // flags
  al_flip_display();
}

static void draw_hud(unsigned level, ALLEGRO_FONT* font){
  // Level
  al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 0, 0, "Level: %d", level+1); // First level is level 0

  // Score
  al_draw_textf(font, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, 0, ALLEGRO_ALIGN_CENTER, "%06d", player_get_score());
  
  // Lives
  int srcWidth = al_get_bitmap_width(sprites.ship);
  int srcHeight = al_get_bitmap_height(sprites.ship);
  al_draw_scaled_bitmap(sprites.ship, 0, 0, srcWidth, srcHeight, WORLD_WIDTH-PLAYER_W/1.5-10, 10, PLAYER_W/1.5, PLAYER_H/1.5, 0);
  al_draw_textf(font, al_map_rgb(255, 255, 255), WORLD_WIDTH-PLAYER_W/1.5-55, 10, 0, "%02d", player_get_lives());
}

static float get_random_star_speed(){
  float base_speed = rand_between_f(STAR_MIN_SPEED, STAR_MAX_SPEED);
  float relative = aliens_get_relative_speed(); // in [0, 1]

  // Interpolate between min and full star speed
  return STAR_MIN_SPEED + (base_speed - STAR_MIN_SPEED) * relative;
}

static void background_update(){
  for(int i = 0; i < STARS_N; i++) {
        stars[i].y += stars[i].speed;
        if(stars[i].y >= WORLD_HEIGHT) {
            stars[i].y = 0;
            stars[i].speed = get_random_star_speed();
            stars[i].color = random_star_color();
        }
    }
}

static void background_init() {
    for(int i = 0; i < STARS_N; i++) {
        stars[i].y = rand_between_f(0, WORLD_HEIGHT);
        stars[i].speed = get_random_star_speed();
        stars[i].color = random_star_color();  // assign random color
    }
}

static void draw_background() {
    float star_x = STAR_MARGIN;
    for(int i = 0; i < STARS_N; i++) {
        al_draw_pixel(star_x, stars[i].y, stars[i].color);
        star_x += STAR_SEPARATION;
    }
}

static ALLEGRO_COLOR random_star_color() {
    int r = rand() % 5;  // 5 types of stars

    switch(r) {
        case 0: return al_map_rgb_f(1.0, 1.0, 1.0); // White (type A)
        case 1: return al_map_rgb_f(0.8, 0.8, 1.0); // Light blue (type B)
        case 2: return al_map_rgb_f(1.0, 1.0, 0.6); // Yellow (type G)
        case 3: return al_map_rgb_f(0.9, 0.7, 1.0); // Light violet
        case 4: return al_map_rgb_f(1.0, 0.9, 0.8); // Pale orange (type K)
        default: return al_map_rgb_f(1.0, 1.0, 1.0);
    }
}

static void draw_mothership(mothership_color_t color) {
  //al_draw_rectangle(mothership_get_x(), mothership_get_y(), mothership_get_x() + MOTHERSHIP_W - 1, mothership_get_y() + MOTHERSHIP_H - 1, al_map_rgb(255, 0, 0), 1);
  ALLEGRO_BITMAP* ufoSprite = sprites.ufo[color][0];
  int srcWidth = al_get_bitmap_width(ufoSprite), srcHeight = al_get_bitmap_height(ufoSprite);
  al_draw_scaled_bitmap(
    ufoSprite, 0, 0, srcWidth, srcHeight,
    mothership_get_x()-MOTHERSHIP_SCALE_X/2, mothership_get_y()-MOTHERSHIP_SCALE_Y/2,
    MOTHERSHIP_W+MOTHERSHIP_SCALE_X, MOTHERSHIP_H+MOTHERSHIP_SCALE_Y, 0);
}

static void draw_alien(unsigned i, unsigned j, unsigned sprite, alien_color_t color, unsigned char aliensFrame) {
  int alienX = aliens_get_x(i, j) - ALIEN_SCALE_X/2, alienY = aliens_get_y(i, j) - ALIEN_SCALE_Y/2;
  ALLEGRO_BITMAP *alienSprite = sprites.aliens[alines_get_type(i,j)][color][aliensFrame];
  int srcWidth = al_get_bitmap_width(alienSprite), srcHeight = al_get_bitmap_height(alienSprite);
  int dWidth = ALIENS_W + ALIEN_SCALE_X, dHeight = ALIENS_H + ALIEN_SCALE_Y;
  // Para ver la hitbox
  //al_draw_rectangle(aliens_get_x(i, j), aliens_get_y(i, j), aliens_get_x(i, j) + ALIENS_W, aliens_get_y(i, j) + ALIENS_H, al_map_rgb(255, 0, 0), 1);
  al_draw_scaled_bitmap(alienSprite, 0, 0, srcWidth, srcHeight, alienX, alienY, dWidth, dHeight, 0);
}

static void draw_player() {
  if(playerDied) return;
  int srcWidth = al_get_bitmap_width(sprites.ship);
  int srcHeight = al_get_bitmap_height(sprites.ship);
  al_draw_scaled_bitmap(sprites.ship, 0, 0, srcWidth, srcHeight, player_get_x(), player_get_y(), PLAYER_W, PLAYER_H, 0);
 // al_draw_filled_rectangle(player_get_x(), player_get_y(), player_get_x() + PLAYER_W, player_get_y() + PLAYER_H, al_map_rgb(0, 255, 0));
}

static void draw_alien_shot(unsigned frame, unsigned color) {
  if ( alien_shot_is_used() ){
    //al_draw_rectangle(alien_shot_get_x(), alien_shot_get_y(), alien_shot_get_x() + SHOT_W, alien_shot_get_y() + SHOT_H, al_map_rgb(255, 0, 0), 1);
    al_draw_tinted_scaled_bitmap(
      sprites.shot[0][frame],
      al_map_rgb(255, 255-color, 255-color),
      0, 0,
      SPRITE_SHOT_W,
      SPRITE_SHOT_H,
      alien_shot_get_x()-SPRITE_SHOT_SPACING,
      alien_shot_get_y(),
      SHOT_W, SHOT_H, 0
    );
  }
}

static void draw_player_shot(unsigned frame, unsigned color) {
  if ( player_shot_is_used() ){
    //al_draw_rectangle(player_shot_get_x(), player_shot_get_y(), player_shot_get_x() + SHOT_W, player_shot_get_y() + SHOT_H, al_map_rgb(255, 0, 0), 1);
    al_draw_tinted_scaled_bitmap(
      sprites.shot[1][frame],
      al_map_rgb(255-color, 255, 255-color),
      0, 0,
      SPRITE_SHOT_W,
      SPRITE_SHOT_H,
      player_shot_get_x()-SPRITE_SHOT_SPACING,
      player_shot_get_y(),
      SHOT_W, SHOT_H, 0
    );
  }
}

static void draw_shield(unsigned shield) {
  unsigned i, j;
  for ( i = 0; i < SHIELD_H; ++i ) {
    for ( j = 0; j < SHIELD_W; ++j ) {
      int lives = shield_get_lives(shield, i, j);
      if (lives > 0) {
          // Block coordinates
          float x = shield_get_x(shield, i, j);
          float y = shield_get_y(shield, i, j);

          // Draw white block in background
          al_draw_filled_rectangle(x, y, x + SHIELD_BLOCK_W, y + SHIELD_BLOCK_H, al_map_rgb(255, 255, 255));

          // Draw segments dependeing on lives lost
          int lost = SHIELD_BLOCK_LIVES - lives;
          int max_lines = 60;
          int lines = (lost * max_lines) / SHIELD_BLOCK_LIVES;

          const float SEGMENT_LENGTH = 3.0;
          const float SEGMENT_THICKNESS = 1.0;

          // srand restarted every time for segments not to move
          srand(0);
          for(int p = 0; p < lines; ++p) {
              float px = x + rand_between_f(0, SHIELD_BLOCK_W - SEGMENT_LENGTH);
              float py = y + rand_between_f(0, SHIELD_BLOCK_H);
              al_draw_line(px, py, px + SEGMENT_LENGTH, py, al_map_rgb(0, 0, 0), SEGMENT_THICKNESS);
          }
          srand(time(NULL));
      }
    }
  }
}

static void draw_explosion(explosion_t explosion, unsigned color){
  ALLEGRO_BITMAP* sprite = NULL;
  int width, height, x = explosion.x, y = explosion.y;
  switch(explosion.type){
    case ALIEN_EXPLOSION:
      if(alienWasKilled) al_play_sample_instance(alienDeathSample);
      sprite = sprites.aliens_explotion[color];
      width = ALIENS_W;
      height = ALIENS_H;
      break;
    case UFO_EXPLOSION:
      if(alienWasKilled) al_play_sample_instance(ufoDeathSample);
      sprite = sprites.ufo[color][1];
      width = MOTHERSHIP_W+MOTHERSHIP_SCALE_X;
      height = MOTHERSHIP_H+MOTHERSHIP_SCALE_Y;
      x -= MOTHERSHIP_SCALE_X/2;
      y -= MOTHERSHIP_SCALE_Y/2;
      break;
    case NO_EXPLOSION:
      break;
  }
  int srcWidth = al_get_bitmap_width(sprite);
  int srcHeight = al_get_bitmap_height(sprite);
  al_draw_scaled_bitmap(sprite, 0, 0, srcWidth, srcHeight, x, y, width, height, 0);
}

static ALLEGRO_BITMAP *sprite_grab(ALLEGRO_BITMAP* father, int x, int y, int w, int h) {
  ALLEGRO_BITMAP *sprite = al_create_sub_bitmap(father, x, y, w, h);
  init_error(sprite, "sprite grab");
  return sprite;
}

static void sprites_init() {
  sprites._sheet = al_load_bitmap(SPRITESHEET2);
  init_error(sprites._sheet, "spritesheet");
  sprites._sheet_shot = al_load_bitmap(SPRITESHEETSHOT);
  init_error(sprites._sheet_shot, "spritesheet_shot");
  sprites.ship = al_load_bitmap(BITMAP_ROUTE("intro/intro_ship.png"));
  init_error(sprites.ship, "spritesheet_ship");

  // Alien sprites
  int xOffset = 0;
  for(int i = 0; i < ALIEN_TOTAL_COLORS; i++){
    if(i >= 6) xOffset = 577;
    sprites.aliens[0][i][0] = sprite_grab(sprites._sheet, xOffset+8,  4+(i%6)*74, 72, 70); // 20, 22+(i%6)*74, 48, 32
    sprites.aliens[0][i][1] = sprite_grab(sprites._sheet, xOffset+92,  4+(i%6)*74, 72, 70); // 104, 22...
    sprites.aliens[1][i][0] = sprite_grab(sprites._sheet, xOffset+176, 4+(i%6)*74, 72, 70);
    sprites.aliens[1][i][1] = sprite_grab(sprites._sheet, xOffset+260, 4+(i%6)*74, 72, 70);
    sprites.aliens[2][i][0] = sprite_grab(sprites._sheet, xOffset+340, 4+(i%6)*74, 72, 70);
    sprites.aliens[2][i][1] = sprite_grab(sprites._sheet, xOffset+416, 4+(i%6)*74, 72, 70);
    sprites.aliens_explotion[i] = sprite_grab(sprites._sheet, xOffset+512, 22+(i%6)*74, 48, 32);
  }

  // UFO sprites
  bool frame = 0;
  int i, j, k;
  for(j = 0; j < 2; j++){
    for(k = 0, i = j*5; k+frame < 6 && i < 10; (frame ? (i++,k++) : i), frame = !frame){
      sprites.ufo[i][frame] = sprite_grab(sprites._sheet, 4-j*4+115*k+(k+frame-j)*95+12*frame, (j*75)+450, 95, 65);
    }
  }
  sprites.ufo[10][0] = sprite_grab(sprites._sheet, 1054, 525, 95, 65);
  sprites.ufo[10][1] = sprite_grab(sprites._sheet, 12, 600, 95, 65);

  // Shot animations sprites
  int xSpacing, ySpacing = 0;
  for(int j = 0; j < SPRITE_SHOT_NUM; j++){
    xSpacing = 0;
    for(int i = 0; i < SPRITE_SHOT_FRAMES; i++){
      if(i > 0) xSpacing = SPRITE_SHOT_SPACING;
      sprites.shot[j][i] = sprite_grab(
        sprites._sheet_shot,
        (xSpacing+SPRITE_SHOT_W)*i,
        (ySpacing+SPRITE_SHOT_H)*j,
        SPRITE_SHOT_W,
        SPRITE_SHOT_H
      );
    }
    ySpacing = SPRITE_SHOT_SPACING;
  }
}

static void sprites_deinit() {
  al_destroy_bitmap(sprites._sheet);
  al_destroy_bitmap(sprites._sheet_shot);
  al_destroy_bitmap(sprites.ship);
}