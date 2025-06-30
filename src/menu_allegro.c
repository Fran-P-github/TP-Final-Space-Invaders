/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     menu_allegro.c
     25/6/25

     Description:



  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "menu_allegro.h"
#include <stdarg.h>

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define MSJ_ERR_INIT "Error with setup: "

#define DRAW draw_frame(display, buffer)
#define BUFFER al_set_target_bitmap(buffer)
#define CLEAR al_clear_to_color(al_map_rgb(0, 0, 0))

#define ERRCHECK(obj, txt) \
  if ( must_setup(obj, txt) == CLOSED ) return CLOSED

#define ANIMATION_LOOP_1                                                                                                                                                                                                                                    \
  trail_accel(intro_ship.trail, mouse.x, mouse.y, mouse_x, mouse_y, screen_width, screen_height);                                                                                                                                                           \
  cursor_accel(&mouse, &mouse_x, &mouse_y, &Accel_x, &Accel_y);                                                                                                                                                                                             \
                                                                                                                                                                                                                                                            \
  al_draw_bitmap(intro_background_frames[counter_1++], 0, 0, 0);                                                                                                                                                                                            \
  al_draw_filled_polygon(intro_ship.trail, 10, al_map_rgba(255, 255, 255, 200));                                                                                                                                                                            \
  al_draw_scaled_rotated_bitmap(intro_ship.sprite, al_get_bitmap_width(intro_ship.sprite) / 2, al_get_bitmap_height(intro_ship.sprite) / 2, mouse.x * WORLD_WIDTH / screen_width, mouse.y * WORLD_HEIGHT / screen_height, 0.5, 0.15 + Accel_y, Accel_x, 0); \
  al_draw_scaled_rotated_bitmap(intro_logo, al_get_bitmap_width(intro_logo) / 2, al_get_bitmap_height(intro_logo) / 2, WORLD_WIDTH / 2, WORLD_HEIGHT / 4, 1, 1, 0, 0);                                                                                      \
                                                                                                                                                                                                                                                            \
  if ( counter_2 > 0 ) counter_2 -= 10;                                                                                                                                                                                                                     \
  if ( counter_2 < 0 ) counter_2 = 0;                                                                                                                                                                                                                       \
  if ( counter_1 == 300 ) counter_1 = 0

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

enum choice { INTRO = 10,
              QUIT };

typedef struct {
  int px;           // X position
  int py;           // Y position
  unsigned char cx; // X center
  unsigned char cy; // Y center
  float trail[20];
  ALLEGRO_BITMAP *sprite;
} ship_t;

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static game_state_t must_setup(bool task, char *msg);
static void intro_anim(ALLEGRO_FONT *dfont, ALLEGRO_SAMPLE *s_logo, ALLEGRO_SAMPLE *sample_intro_1, ALLEGRO_BITMAP *buffer, ALLEGRO_DISPLAY *display, ALLEGRO_SAMPLE_INSTANCE *sample_instance, ALLEGRO_BITMAP *ship);
static void draw_frame(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *buffer);
static void cursor_accel(ALLEGRO_MOUSE_STATE *mouse, short int *mouse_old_x, short int *mouse_old_y, float *accel_x, float *Accel_y);
static void trail_accel(float *polygon, int new_x, int new_y, int old_x, int old_y, int width, int height);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

game_state_t menu_allegro(ALLEGRO_DISPLAY *display, ALLEGRO_TIMER *timer, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT *default_font, ALLEGRO_BITMAP *buffer, ALLEGRO_MIXER *mixer, void (*kill_all_bitmaps)(int, ...), void (*kill_all_instances)(int, ...), void (*kill_all_samples)(int, ...)) {

  /*************************DECLARATIONS************************/

  ALLEGRO_BITMAP *intro_background_frames[300];
  ALLEGRO_BITMAP *intro_logo;
  ALLEGRO_EVENT menu_event;
  ALLEGRO_EVENT dummy;
  ALLEGRO_SAMPLE *logo_sound;
  ALLEGRO_SAMPLE *intro_part1;
  ALLEGRO_SAMPLE *intro_part2;
  ALLEGRO_SAMPLE_INSTANCE *sample_instance;
  ALLEGRO_SAMPLE_INSTANCE *sample_instance2;
  ALLEGRO_MOUSE_STATE mouse;
  char intro_background_path[64];
  short int counter_1, counter_2, screen_width, screen_height, mouse_x, mouse_y;

  /**************************************************************/

  /*************************DEFINITIONS**************************/

  ship_t intro_ship = {0, 0, 0, 0, {0}, NULL};
  char select = INTRO;
  bool redraw = false;
  bool fullscreen = al_get_display_flags(display) & ALLEGRO_FULLSCREEN_WINDOW;
  float Accel_x = 0, Accel_y = 0;

  /**************************************************************/

  /************************INITIALIZERS**************************/

  sample_instance = al_create_sample_instance(NULL);
  if ( must_setup(sample_instance, "sample Instance") == CLOSED ) return CLOSED;
  sample_instance2 = al_create_sample_instance(NULL);
  if ( must_setup(sample_instance2, "sample Instance") == CLOSED ) return CLOSED;

  al_attach_sample_instance_to_mixer(sample_instance, al_get_default_mixer());
  al_attach_sample_instance_to_mixer(sample_instance2, al_get_default_mixer());

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

  for ( counter_1 = 1; counter_1 <= 300 && select != QUIT; counter_1++ ) {
    sprintf(intro_background_path, "../assets/Bitmap/intro_background/frames/frame_%03d.png", counter_1);
    intro_background_frames[counter_1 - 1] = al_load_bitmap(intro_background_path);
    if ( must_setup(intro_background_frames[counter_1 - 1], "Intro background frame") == CLOSED ) return CLOSED;
  }

  intro_logo = al_load_bitmap(BITMAP_ROUTE("intro/intro_logo.png"));
  ERRCHECK(intro_logo, "intro logo");
  intro_ship.sprite = al_load_bitmap(BITMAP_ROUTE("intro/intro_ship.png"));
  ERRCHECK(intro_ship.sprite, "intro ship");
  logo_sound = al_load_sample(AUDIO_ROUTE("coin.wav"));
  ERRCHECK(logo_sound, "Coin Sound");
  intro_part1 = al_load_sample(AUDIO_ROUTE("intro_part1.wav"));
  ERRCHECK(intro_part1, "intro 1st part sound");
  intro_part2 = al_load_sample(AUDIO_ROUTE("intro_part2.wav"));
  ERRCHECK(intro_part1, "intro 2nd part sound");

  intro_ship.cx = al_get_bitmap_width(intro_ship.sprite) / 2;
  intro_ship.cy = al_get_bitmap_height(intro_ship.sprite) / 2;

  al_set_sample(sample_instance2, intro_part2);

  al_hide_mouse_cursor(display);

  /**************************************************************/

  /***********************FIRST PHASE****************************/

  intro_anim(default_font, logo_sound, intro_part1, buffer, display, sample_instance, intro_ship.sprite);

  al_play_sample_instance(sample_instance2);

  /**************************************************************/

  /*************************PRE-LOOP*****************************/

  al_start_timer(timer);

  counter_1 = 0;
  counter_2 = 255;

  screen_width = al_get_display_width(display);
  screen_height = al_get_display_height(display);

  mouse_x = mouse.x;
  mouse_y = mouse.y;

  while ( al_get_next_event(queue, &dummy) );

  /**************************************************************/

  /*************************MAIN-LOOP****************************/

  while ( select != QUIT ) {
    al_wait_for_event(queue, &menu_event);

    switch ( menu_event.type ) {
    case ALLEGRO_EVENT_TIMER:
      BUFFER;
      CLEAR;
      redraw = true;
      al_get_mouse_state(&mouse);
      ANIMATION_LOOP_1;
      break;
    case ALLEGRO_EVENT_KEY_DOWN:
      switch ( menu_event.keyboard.keycode ) {
      case ALLEGRO_KEY_F:
        fullscreen = !fullscreen;
        al_toggle_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, fullscreen);
        break;
      case ALLEGRO_KEY_ESCAPE:
        select = QUIT;
        break;
      }
      break;
    case ALLEGRO_EVENT_DISPLAY_RESIZE:
      al_acknowledge_resize(display);
      screen_width = al_get_display_width(display);
      screen_height = al_get_display_height(display);
      break;
    case ALLEGRO_EVENT_DISPLAY_CLOSE:
      select = QUIT;
      break;
    }

    if ( redraw ) {
      al_draw_filled_rectangle(0, 0, WORLD_WIDTH, WORLD_HEIGHT, al_map_rgba(255, 255, 255, counter_2));
      DRAW;
      redraw = false;
    }
  }

  /**************************************************************/

  /***********************RESOURCE-FREEING***********************/

  for ( counter_1 = 0; counter_1 < 300; counter_1++ ) (*kill_all_bitmaps)(1, intro_background_frames[counter_1]);
  (*kill_all_bitmaps)(2, intro_ship.sprite, intro_logo);
  (*kill_all_samples)(3, logo_sound, intro_part1, intro_part2);
  (*kill_all_instances)(2, sample_instance, sample_instance2);

  al_stop_timer(timer);

  /**************************************************************/

  return GAME;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/************************MUST_SETUP*****************************/

game_state_t must_setup(bool task, char *msg) {
  if ( !task ) {
    fprintf(stderr, "%s%s\n", MSJ_ERR_INIT, msg);
    return CLOSED;
  }
  return 1;
}

/****************************************************************/

/**********************INTRO_ANIM********************************/

void intro_anim(ALLEGRO_FONT *default_font, ALLEGRO_SAMPLE *s_logo, ALLEGRO_SAMPLE *sample_intro_1, ALLEGRO_BITMAP *buffer, ALLEGRO_DISPLAY *display, ALLEGRO_SAMPLE_INSTANCE *sample_instance, ALLEGRO_BITMAP *ship) {
  short int a, b, x, y;
  short int width = al_get_bitmap_width(ship);
  short int height = al_get_bitmap_height(ship);
  float s;

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

  float trail[20] = {
      WORLD_WIDTH / 2 - 4, WORLD_HEIGHT, WORLD_WIDTH / 2 - 8, WORLD_HEIGHT, WORLD_WIDTH / 2 - 16, WORLD_HEIGHT, WORLD_WIDTH / 2 - 20, WORLD_HEIGHT, WORLD_WIDTH / 2 - 48, WORLD_HEIGHT,

      WORLD_WIDTH / 2 + 48,
      WORLD_HEIGHT,
      WORLD_WIDTH / 2 + 20,
      WORLD_HEIGHT,
      WORLD_WIDTH / 2 + 16,
      WORLD_HEIGHT,
      WORLD_WIDTH / 2 + 8,
      WORLD_HEIGHT,
      WORLD_WIDTH / 2 + 4,
      WORLD_HEIGHT};

  al_set_sample(sample_instance, sample_intro_1);

  BUFFER;
  CLEAR;
  al_draw_textf(default_font, al_map_rgb(255, 255, 255), WORLD_WIDTH / 2, WORLD_HEIGHT / 2, ALLEGRO_ALIGN_CENTER, "Lionel Messi Studios presents");
  DRAW;
  al_play_sample(s_logo, 1.5, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
  al_rest(2.0);

  for ( a = 255; a > 0; a -= 5 ) {
    BUFFER;
    al_draw_textf(default_font, al_map_rgb(a, a, a), WORLD_WIDTH / 2, WORLD_HEIGHT / 2, ALLEGRO_ALIGN_CENTER, "Lionel Messi Studios presents");
    DRAW;
    al_rest(0.01);
  }
  CLEAR;
  al_flip_display();

  al_rest(1.0);
  al_play_sample_instance(sample_instance);

  for ( x = -32, y = WORLD_HEIGHT + 32, b = 255; x <= WORLD_WIDTH + 64; x += 100, y -= 56 ) {
    BUFFER;
    CLEAR;
    al_draw_line(-32, WORLD_HEIGHT + 32, x, y, al_map_rgba(255, 255, 255, b), 20.0);
    al_draw_scaled_rotated_bitmap(ship, width / 2, height / 2, x, y, 0.75, 0.75, 6 * ALLEGRO_PI / 18, 0);
    al_rest(1 / 30.0);
    DRAW;
  }
  while ( b > 0 ) {
    BUFFER;
    CLEAR;
    al_draw_line(-32, WORLD_HEIGHT + 32, x, y, al_map_rgba(255, 255, 255, b), 20.0);
    b -= 8;
    al_rest(1 / 30.0);
    if ( b < 0 ) b = 0;
    DRAW;
  }
  for ( x = WORLD_WIDTH + 32, y = WORLD_HEIGHT + 32, b = 255; x >= -64; x -= 100, y -= 56 ) {
    BUFFER;
    CLEAR;
    al_draw_line(WORLD_WIDTH + 32, WORLD_HEIGHT + 32, x, y, al_map_rgba(255, 255, 255, b), 20.0);
    al_draw_scaled_rotated_bitmap(ship, width / 2, height / 2, x, y, 0.75, 0.75, -6 * ALLEGRO_PI / 18, 0);
    al_rest(1 / 30.0);
    DRAW;
  }
  while ( b > 0 ) {
    BUFFER;
    CLEAR;
    al_draw_line(WORLD_WIDTH + 32, WORLD_HEIGHT + 32, x, y, al_map_rgba(255, 255, 255, b), 20.0);
    b -= 8;
    al_rest(1 / 30.0);
    if ( b < 0 ) b = 0;
    DRAW;
  }

  for ( x = WORLD_WIDTH / 2, y = WORLD_HEIGHT + 64, a = 100, s = 1.0; al_get_sample_instance_playing(sample_instance); y -= a ) {
    BUFFER;
    CLEAR;
    trail[1] = y;
    trail[19] = y;

    for ( b = 1; b <= 3; b++ ) {
      trail[1 + 2 * b] = WORLD_HEIGHT + (y - WORLD_HEIGHT) / b;
      trail[19 - 2 * b] = WORLD_HEIGHT + (y - WORLD_HEIGHT) / b;
    }

    al_draw_filled_polygon(trail, 10, al_map_rgba(255, 255, 255, 85));
    al_draw_scaled_rotated_bitmap(ship, width / 2, height / 2, x, y, 2.0 - (1.55 - s), 0.55 - (0.40 - s / 4), 0, 0);

    if ( s > 0.5 )
      s -= 0.5;
    else if ( s > 0.1 )
      s -= 0.1;
    else if ( s > 0 )
      s -= 0.01;

    if ( a > 40 )
      a -= 35;
    else if ( a > 20 )
      a -= 10;
    else
      a -= 1;

    al_rest(1 / 30.0);
    DRAW;
  }
}

/**************************************************************/

/**************************DRAW_FRAME**************************/

void draw_frame(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *buffer) {
  al_set_target_backbuffer(display);
  al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0, al_get_display_width(display), al_get_display_height(display), 0);
  al_flip_display();
}

/**************************************************************/

/***********************CURSOR_ACCEL***************************/

void cursor_accel(ALLEGRO_MOUSE_STATE *mouse, short int *mouse_old_x, short int *mouse_old_y, float *Accel_x, float *Accel_y) {
  short int Delta_x = mouse->x - *mouse_old_x;
  short int Delta_y = mouse->y - *mouse_old_y;

  *Accel_x = Delta_x / 100.0;

  *Accel_y = -Delta_y / 150.0;

  *mouse_old_x = mouse->x;
  *mouse_old_y = mouse->y;
}

/**************************************************************/

/************************TRAIL_ACCEL***************************/

void trail_accel(float *polygon, int new_x, int new_y, int old_x, int old_y, int width, int height) {
  short int counter_1, counter_2 = 0;
  int delta_x = old_x - new_x;
  int delta_y = old_y - new_y;
  float factor_x = (float) WORLD_WIDTH / width;
  float factor_y = (float) WORLD_HEIGHT / height;

  if ( delta_x < 10 && delta_x > -10 ) {
    if ( delta_y < 0 )
      counter_2 = -2;
    else
      counter_2 = 2;
  }

  for ( counter_1 = 1; counter_1 <= 5; counter_1++ ) {
    polygon[11 - 2 * counter_1] = new_y * factor_y + delta_y * 2 / counter_1;
    polygon[9 + 2 * counter_1] = new_y * factor_y + delta_y * 2 / counter_1;

    polygon[2 * counter_1 - 2] = (new_x + delta_x / counter_1) * factor_x - counter_2 * counter_1;
    polygon[20 - 2 * counter_1] = (new_x + delta_x / counter_1) * factor_x + counter_2 * counter_1;
  }
}

/**************************************************************/