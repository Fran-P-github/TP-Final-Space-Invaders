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
#include "front_allegro.h"
#include "general_defines.h"
#include "menu_allegro.h"

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define MSJ_ERR_INIT "Problema al inicializar: "
#define AUDIO_SAMPLES 16
#define MAX_EVENT_WAIT_TIME 0.001
// Floats para el volumen de los efectos de sonido
#define VOLUME_PLAYER_SHOT .1
#define VOLUME_PLAYER_DEATH .3
#define VOLUME_ALIENS_MOVED .3
#define VOLUME_ALIENS_DEATH .3
#define VOLUME_UFO .1
// Sprites
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
  ALLEGRO_BITMAP *aliens[SPRITE_ALIENS_NUM][ALIEN_TOTAL_COLORS][2]; // 3 tipos de aliens, 11 colores, 2 estados de animacion
  ALLEGRO_BITMAP *aliens_explotion[ALIEN_TOTAL_COLORS];             // 11 colores de explosiones
  ALLEGRO_BITMAP *shot[SPRITE_SHOT_NUM][SPRITE_SHOT_FRAMES];        // 2 tipos de disparos, 6 estados de animacion
  ALLEGRO_BITMAP *ufo[ALIEN_TOTAL_COLORS][2];                       // 11 colores de naves nodrizas, 2 estados de animacion
} sprites_t;

typedef struct{
  int x;
  int y;
  int explosion_interval;
}explosion_t;

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

extern const bool aliensMoved;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

// Background drawing
static float get_random_star_speed();
static void background_init();
static void background_update();
static void draw_background();
static ALLEGRO_COLOR random_star_color();

static void alien_death(unsigned i, unsigned j);
static void draw_mothership();
static void draw_alien(unsigned i, unsigned j, unsigned sprite, alien_color_t color, unsigned char aliensFrame);
static void draw_player();
static void draw_player_shot(unsigned frame, unsigned color);
static void draw_alien_shot(unsigned frame, unsigned color);
static void draw_shield(unsigned shield);
static void draw_explosion(unsigned frame, unsigned color);
static ALLEGRO_BITMAP *sprite_grab(ALLEGRO_BITMAP* father, int x, int y, int w, int h);
static void sprites_init();
static void sprites_deinit();

// Funcion wrapper para inicializar un efecto de sonido
static void initAudioInstance(ALLEGRO_SAMPLE_INSTANCE *instance, float volume, ALLEGRO_PLAYMODE playmode);

static void init_error(bool state, const char *name);

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


star_t stars[STARS_N];

static ALLEGRO_TIMER *timer;
static ALLEGRO_DISPLAY *disp;
static ALLEGRO_EVENT_QUEUE *queue;
static ALLEGRO_FONT *default_font;
static ALLEGRO_BITMAP *buffer;
static ALLEGRO_MIXER *mixer;

// Sprites
static sprites_t sprites;

explosion_t explosion; // explosion struct

// Punteros a los samples para el audio
static ALLEGRO_SAMPLE *playerShotSound = NULL;
static ALLEGRO_SAMPLE *playerDeathSound = NULL;
static ALLEGRO_SAMPLE *alienDeathSound = NULL;
static ALLEGRO_SAMPLE *alienMovedSound = NULL;
static ALLEGRO_SAMPLE *ufoSound = NULL;
static ALLEGRO_SAMPLE_INSTANCE *playerShotSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *alienMovedSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *ufoSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *alienDeathSample = NULL;
static ALLEGRO_SAMPLE_INSTANCE *playerDeathSample = NULL;

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
  mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
  init_error(mixer, "Mixer");
  // Se adjunta el mixer creado al mixer principal.
  al_attach_mixer_to_mixer(mixer, al_get_default_mixer());

  // Se crean instancias de samples para el disparo del jugador y para el movimiento de los aliens
  playerShotSample = al_create_sample_instance(playerShotSound);
  playerDeathSample = al_create_sample_instance(playerDeathSound);
  alienMovedSample = al_create_sample_instance(alienMovedSound);
  alienDeathSample = al_create_sample_instance(alienDeathSound);
  ufoSample = al_create_sample_instance(ufoSound);

  al_attach_sample_instance_to_mixer(playerShotSample, mixer);
  al_attach_sample_instance_to_mixer(playerDeathSample, mixer);
  al_attach_sample_instance_to_mixer(alienMovedSample, mixer);
  al_attach_sample_instance_to_mixer(alienDeathSample, mixer);
  al_attach_sample_instance_to_mixer(ufoSample, mixer);

  // Se setean los valores predeterminados para cada audio.
  initAudioInstance(playerShotSample, VOLUME_PLAYER_SHOT, ALLEGRO_PLAYMODE_ONCE);
  initAudioInstance(playerDeathSample, VOLUME_PLAYER_DEATH, ALLEGRO_PLAYMODE_ONCE);
  initAudioInstance(alienMovedSample, VOLUME_ALIENS_MOVED, ALLEGRO_PLAYMODE_ONCE);
  initAudioInstance(alienDeathSample, VOLUME_ALIENS_DEATH, ALLEGRO_PLAYMODE_ONCE);
  initAudioInstance(ufoSample, VOLUME_UFO, ALLEGRO_PLAYMODE_LOOP);

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

game_state_t game_pause(unsigned int level) {
    al_stop_timer(timer); // Pause timer while in pause
    al_stop_sample_instance(ufoSample); // Stop mothership sound

    ALLEGRO_FONT *font = al_load_ttf_font(FONT_ROUTE("supercharge-font/Supercharge_halftone.otf"), 28, 0);
    init_error(font, "Pause menu font");

    const int space = WORLD_HEIGHT / 10;
    const int button_w = WORLD_WIDTH / 3;
    const int button_h = WORLD_HEIGHT / 12;
    const int x = WORLD_WIDTH / 2 - button_w / 2;
    const int start_y = 3 * space;

    const char *labels[] = { "Resume", "Main Menu", "Exit" };
    ALLEGRO_COLOR colors[] = {
        al_map_rgb(100, 100, 255),
        al_map_rgb(100, 255, 100),
        al_map_rgb(255, 100, 100)
    };

    ALLEGRO_EVENT ev;
    bool done = false;
    game_state_t result = GAME;

    while (!done) {
        // Draw background
        al_set_target_bitmap(buffer);
        al_clear_to_color(al_map_rgb(0, 0, 0));

        // Show player info
        al_draw_textf(font, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, space, ALLEGRO_ALIGN_CENTER,
                      "Score: %d    Lives: %d    Level: %d",
                      player_get_score(), player_get_lives(), level+1); // First level is level 0

        // Draw buttons
        for(int i = 0; i < 3; ++i) {
            int y = start_y + i * (button_h + space / 2);
            al_draw_filled_rectangle(x, y, x + button_w, y + button_h, colors[i]);
            al_draw_rectangle(x, y, x + button_w, y + button_h, al_map_rgb(255, 255, 255), 2);
            al_draw_text(font, al_map_rgb(0, 0, 0), WORLD_WIDTH/2, y + button_h / 4, ALLEGRO_ALIGN_CENTER, labels[i]);
        }

        // Draw to screen
        al_set_target_backbuffer(disp);
        al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0,
                              al_get_display_width(disp), al_get_display_height(disp), 0);
        al_flip_display();

        // Wait for event
        while (al_get_next_event(queue, &ev)) {
          if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) continue; // Ignore mouse movement

          if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
              int mx = ev.mouse.x * WORLD_WIDTH / al_get_display_width(disp);
              int my = ev.mouse.y * WORLD_HEIGHT / al_get_display_height(disp);

              for(int i = 0; i < 3; ++i) {
                  int y = start_y + i * (button_h + space / 2);
                  if (mx >= x && mx <= x + button_w && my >= y && my <= y + button_h) {
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
          } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
              result = GAME;
              done = true;
          }
        }
    }

    al_destroy_font(font);
    al_start_timer(timer); // Resume timer after pause
    al_flush_event_queue(queue); // Flush queue to give it back empty to game_update
    memset(key, 0, sizeof(key)); // Clear keys mask for going back to game
    al_clear_to_color(al_map_rgb(0, 0, 0)); // Clear screen on exit
    al_flip_display();
    return result;
}

void endgame() {
    char name[NAME_LEN + 1] = "";
    int name_len = 0;
    int score = player_get_score();

    ALLEGRO_FONT *font_endgame = al_load_ttf_font(FONT_ROUTE("toreks-font/Toreks_regular.ttf"), 32, 0);
    init_error(font_endgame, "Endgame font");
    const int space_between_lines = WORLD_HEIGHT/8;

    al_set_target_bitmap(buffer);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_textf(font_endgame, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, space_between_lines, ALLEGRO_ALIGN_CENTER,
                  "Score: %d", score);
    al_draw_text(font_endgame, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, 2*space_between_lines, ALLEGRO_ALIGN_CENTER,
                  "Enter your name:");
    al_set_target_backbuffer(disp);
    al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0, al_get_display_width(disp), al_get_display_height(disp), 0); 
    al_flip_display();

    bool done = false;
    while (!done) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);
        al_set_target_bitmap(buffer);

        if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                done = true;
            } else if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && name_len > 0) {
                name[--name_len] = '\0';
            } else if (name_len < NAME_LEN && ev.keyboard.unichar >= 32 && ev.keyboard.unichar <= 126) {
                name[name_len++] = ev.keyboard.unichar;
                name[name_len] = '\0';
            }

            // Redraw
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_textf(font_endgame, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, space_between_lines, ALLEGRO_ALIGN_CENTER,
                          "Score: %d", score);
            al_draw_text(font_endgame, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, 2*space_between_lines, ALLEGRO_ALIGN_CENTER,
                          "Enter your name:");
            al_draw_text(font_endgame, al_map_rgb(255, 255, 0), WORLD_WIDTH/2, 3*space_between_lines, ALLEGRO_ALIGN_CENTER,
                          name);
            
            al_set_target_backbuffer(disp);
            al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0, al_get_display_width(disp), al_get_display_height(disp), 0);
            al_flip_display();
        }
    }

    // Show final message
    al_set_target_bitmap(buffer);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_textf(font_endgame, al_map_rgb(0, 255, 0), WORLD_WIDTH/2, space_between_lines, ALLEGRO_ALIGN_CENTER,
                  "Thanks for playing, %s!", name);
    // Check hi-scores and save score if it is a new best
    highscore_t top_scores[MAX_SCORES]; load_scores(top_scores);
    if( try_insert_score(top_scores, name, player_get_score()) ){
        al_draw_text(font_endgame, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, 2*space_between_lines, ALLEGRO_ALIGN_CENTER,
                  "Congratulations! You just made a new best");
    }
    save_scores(top_scores);
    al_set_target_backbuffer(disp);
    al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0, al_get_display_width(disp), al_get_display_height(disp), 0); 
    al_flip_display();

    al_rest(3.0); // Wait in final mesage screen

    al_destroy_font(font_endgame);
    kill_all();
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

// Complete...
static void kill_all() {
  al_destroy_display(disp);
  al_destroy_timer(timer);
  al_destroy_event_queue(queue);
  al_destroy_font(default_font);
  al_destroy_mixer(mixer);
  // Se matan los procesos relacionados al audio.
  kill_all_instances(
      5, // Cantidad de instancias a destruir.
      playerShotSample,
      playerDeathSample,
      alienMovedSample,
      alienDeathSample,
      ufoSample
    );
  kill_all_samples(
      5, // Cantidad de samples a destruir.
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
static void kill_all_samples(int len, ...) {
  va_list sample_list;
  va_start(sample_list, len);
  int i;
  for ( i = 0; i < len; i++ ) {
    al_destroy_sample(va_arg(sample_list, ALLEGRO_SAMPLE *));
  }
}

// Funcion para matar todas las instancias de audio samples creadas.
static void kill_all_instances(int len, ...) {
  va_list instance_list;
  va_start(instance_list, len);
  int i;
  for ( i = 0; i < len; i++ ) {
    al_destroy_sample_instance(va_arg(instance_list, ALLEGRO_SAMPLE_INSTANCE *));
  }
}

// Funcion para matar todas los bitmaps creados.
static void kill_all_bitmaps(int len, ...) {
  va_list bitmap_list;
  va_start(bitmap_list, len);
  int i;
  for ( i = 0; i < len; i++ ) {
    al_destroy_bitmap(va_arg(bitmap_list, ALLEGRO_BITMAP *));
  }
}

// Funcion para matar todas las fuentes creadas.
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
    level_init(level, ALIENS_ROWS-3 + level / 3, ALIENS_COLUMNS + level / 2, 1 + level / 4, 4 + level / 3, SHIELD_BLOCK_LIVES - level / 6);
    player_reset_on_new_level();
    if ( level == 0 ) player_reset_on_new_game();
    background_init();
  }

  ALLEGRO_EVENT event;
  bool redraw = false, done = false, fullscreen = true, moveThisFrame = true, shotMade = false;
  level_state_t level_state = LEVEL_NOT_DONE;
  unsigned long long frame = 0;
  unsigned shotFrame = 0, playerShotColor = 0, alienShotColor = 0;
  bool frameDecrement = false;

  al_start_timer(timer);

  while ( !done && level_state == LEVEL_NOT_DONE ) {
    // Procesamiento de eventos
    if ( al_wait_for_event_timed(queue, &event, MAX_EVENT_WAIT_TIME) ) {
      switch ( event.type ) {
        case ALLEGRO_EVENT_TIMER:
          background_update();
          level_state = back_update(level, alien_death);
          redraw = true;
          ++frame;
          moveThisFrame = false;
          break;

        case ALLEGRO_EVENT_KEY_DOWN:
          key[event.keyboard.keycode] = 1;
          if ( key[ALLEGRO_KEY_ESCAPE] ){
            done = true;
            return PAUSE;
          }
          if ( key[ALLEGRO_KEY_F] ) {
            fullscreen = !fullscreen;
            al_toggle_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, fullscreen);
          }
          // Se utiliza X para disparar.
          if ( key[ALLEGRO_KEY_X] && player_try_shoot() ) {
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
    if ( key[ALLEGRO_KEY_RIGHT] && !moveThisFrame ) {
      player_move_right();
      moveThisFrame = true;
    } else if ( key[ALLEGRO_KEY_LEFT] && !moveThisFrame ) {
      player_move_left();
      moveThisFrame = true;
    }

    // Reproduce el sonido cuando los aliens se mueven (si siguen vivos) se aprovecha el "laziness" de c.
    if ( total_aliens_alive() && aliensMoved )
      al_play_sample_instance(alienMovedSample);

    // Para el sonido del disparo cuando se hace muy seguido
    if ( !player_shot_is_used() && shotMade ) {
      al_stop_sample_instance(playerShotSample);
      shotMade = false;
    }

    if ( redraw ) {
      // Manejo de frames para las animaciones de disparo (alien y jugador)
      if(frame%3 == 0){
        if(shotFrame >= SPRITE_SHOT_FRAMES - 1) frameDecrement = true;
        else if (shotFrame <= 0) frameDecrement = false;
        frameDecrement ? shotFrame-- : shotFrame++;
        alienShotColor += 40;
        playerShotColor += 30;
      }
      // Maximos valores de color para el disparo del jugador
      if(!player_shot_is_used()) playerShotColor = 0;
      else if(playerShotColor >= 255) playerShotColor = 255;
      // Maximos valores de color para el disparo del alien
      if(!alien_shot_is_used()) alienShotColor = 0;
      else if(alienShotColor >= 255) alienShotColor = 255;
      redraw = false;

      al_set_target_bitmap(buffer);
      al_clear_to_color(al_map_rgb(0, 0, 0));
      draw_background();
      al_draw_textf(default_font, al_map_rgb(255, 255, 255), 0, 0, 0, "Level: %d", level+1); // First level is level 0
      al_draw_textf(default_font, al_map_rgb(255, 255, 255), WORLD_WIDTH/2, 0, ALLEGRO_ALIGN_CENTER, "%06d", player_get_score());
      al_draw_textf(default_font, al_map_rgb(255, 255, 255), 0, WORLD_HEIGHT-16, 0, "Lives: %d", player_get_lives());
      unsigned i, j;
      draw_player_shot(shotFrame, playerShotColor);
      unsigned alien_column_to_shoot = get_best_alien_column_to_shoot();
      if ( alien_column_to_shoot >= 0 ) {
        alien_try_shoot(alien_column_to_shoot);
      }
      draw_alien_shot(shotFrame, alienShotColor);
      for ( int x = 0; x < SHIELDS_CANT; x++ ) {
        draw_shield(x);
      }
      draw_player();

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
                default:
                  alienColor = ALIEN_RETRO; // Never to be reached
                  break;
              }
              draw_alien(i, j, alienSprite, alienColor, aliensFrame);
            }
          }
        }
      }

      if(explosion.explosion_interval > 0) draw_explosion(frame, ALIEN_NEON);

      if ( mothership_is_active() ) {
        al_play_sample_instance(ufoSample);
        draw_mothership(ALIEN_GOLD);
      } else {
        al_stop_sample_instance(ufoSample);
      }

      al_set_target_backbuffer(disp);
      al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0, al_get_display_width(disp), al_get_display_height(disp), 0); // flags
      al_flip_display();
    }
  }

  if ( level_state == PLAYER_WINS ) {
    return GAME;
  } else {
    return CLOSED;
  }
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
        stars[i].color = random_star_color();  // Asignar color aleatorio
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
    int r = rand() % 5;  // 5 tipos de estrellas

    switch(r) {
        case 0: return al_map_rgb_f(1.0, 1.0, 1.0); // Blanco (tipo A)
        case 1: return al_map_rgb_f(0.8, 0.8, 1.0); // Azul claro (tipo B)
        case 2: return al_map_rgb_f(1.0, 1.0, 0.6); // Amarillento (tipo G)
        case 3: return al_map_rgb_f(0.9, 0.7, 1.0); // Violeta suave
        case 4: return al_map_rgb_f(1.0, 0.9, 0.8); // Naranja pálido (tipo K)
        default: return al_map_rgb_f(1.0, 1.0, 1.0);
    }
}

static void alien_death(unsigned i, unsigned j){
  // Play animation
  explosion.x = aliens_get_x(i, j);
  explosion.y = aliens_get_y(i, j);
  explosion.explosion_interval = 5;
  // Play sound effect
  al_play_sample_instance(alienDeathSample);
}

static void draw_mothership(mothership_color_t color) {
  //al_draw_filled_rectangle(mothership_get_x(), mothership_get_y(), mothership_get_x() + MOTHERSHIP_W - 1, mothership_get_y() + MOTHERSHIP_H - 1, al_map_rgb(128, 0, 255));
  ALLEGRO_BITMAP* ufoSprite = sprites.ufo[color][0];
  int srcWidth = al_get_bitmap_width(ufoSprite), srcHeight = al_get_bitmap_height(ufoSprite);
  al_draw_scaled_bitmap(
    ufoSprite, 0, 0, srcWidth, srcHeight,
    mothership_get_x(), mothership_get_y(),
    MOTHERSHIP_W, MOTHERSHIP_H, 0);
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
      if ( shield_get_lives(shield, i, j) )
        al_draw_filled_rectangle(shield_get_x(shield, i, j), shield_get_y(shield, i, j), shield_get_x(shield, i, j) + SHIELD_BLOCK_W, shield_get_y(shield, i, j) + SHIELD_BLOCK_H, al_map_rgb(255, 255, 255));
    }
  }
}

static void draw_explosion(unsigned frame, unsigned color){
  int srcWidth = al_get_bitmap_width(sprites.aliens_explotion[color]);
  int srcHeight = al_get_bitmap_height(sprites.aliens_explotion[color]);
  al_draw_scaled_bitmap(sprites.aliens_explotion[color], 0, 0, srcWidth, srcHeight, explosion.x, explosion.y, ALIENS_W, ALIENS_H, 0);
  if(frame % 2) explosion.explosion_interval--;
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