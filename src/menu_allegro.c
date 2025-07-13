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
#include "scores.h"
#include <stdarg.h>

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define MSJ_ERR_INIT "Error with setup: "

#define DRAW draw_frame(display, buffer)
#define BUFFER al_set_target_bitmap(buffer)
#define CLEAR al_clear_to_color(al_map_rgb(0, 0, 0))
#define CLEAR_A al_clear_to_color(al_map_rgba(0, 0, 0, 0))

#define ERRCHECK(obj, txt) \
  if ( must_setup(obj, txt) == CLOSED ) return CLOSED

#define DRAW_BACKGROUND                                          \
  al_draw_bitmap(intro_background_frames[counter_1++], 0, 0, 0); \
  if ( counter_1 == 300 ) counter_1 = 0

#define DRAW_FLASH                                                                                    \
  if ( counter_2 && flash_enable ) {                                                                  \
    al_draw_filled_rectangle(0, 0, WORLD_WIDTH, WORLD_HEIGHT, al_map_rgba(255, 255, 255, counter_2)); \
    if ( counter_2 > 0 ) counter_2 -= 10;                                                             \
    if ( counter_2 < 0 ) counter_2 = 0;                                                               \
  }

#define DRAW_SHIP_CURSOR                                                                                                                                                                        \
  al_draw_scaled_rotated_bitmap(intro_ship.sprite, intro_ship.cx, intro_ship.cy, mouse.x *WORLD_WIDTH / screen_width, mouse.y * WORLD_HEIGHT / screen_height, 0.5, 0.15 + Accel_y, Accel_x, 0); \
  al_draw_filled_polygon(intro_ship.trail, 10, al_map_rgba(255, 255, 255, 200))

#define DRAW_LOGO \
  al_draw_scaled_rotated_bitmap(intro_logo, al_get_bitmap_width(intro_logo) / 2, al_get_bitmap_height(intro_logo) / 2, WORLD_WIDTH / 2, WORLD_HEIGHT / 4, 1, 1, 0, 0)

#define CURSOR_UPDATE                                                                             \
  trail_accel(intro_ship.trail, mouse.x, mouse.y, mouse_x, mouse_y, screen_width, screen_height); \
  cursor_accel(&mouse, &mouse_x, &mouse_y, &Accel_x, &Accel_y)

#define REDRAW      \
  if ( redraw ) {   \
    DRAW;           \
    redraw = false; \
  }

#define DRAW_BUTTON(button) draw_button(&mouse, screen_width, screen_height, &(button))
// Wrapper for drawing the button. Simpler and quicker

#define BUTTON_TEXT(button, txt) draw_smart_text(&mouse, screen_width, screen_height, &(button), font_supercharge, color_black, color_white, ALLEGRO_ALIGN_RIGHT, (txt))
// Wrapper for drawing text over a certain button

#define MOUSE_HOVER(button) mouse_hover_button(&(button), &mouse, screen_width, screen_height)
// Wrapper to detect where the mouse is hovering over

#define BACK_TEXT al_draw_text(font_cartesian, color_black, WORLD_WIDTH * 0.85, WORLD_WIDTH * 0.02, 0, "Press [ESC] to go back")

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

enum clicks { LEFT_CLICK = 1,
              RIGHT_CLICK = 2,
              MIDDLE_CLICK = 4 };

typedef enum choice { INTRO = 10,
                      MENU_MAIN,
                      MENU_SETTINGS,
                      MENU_SCORE,
                      MENU_CREDITS,
                      PLAY,
                      QUIT } choice_t;

typedef struct {
  int px;           // X position
  int py;           // Y position
  unsigned char cx; // X center
  unsigned char cy; // Y center
  float trail[20];
  ALLEGRO_BITMAP *sprite;
} ship_t;

typedef struct {
  float *coords;
  unsigned short int vertex;
} polygon_t;

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
// static polygon_t assign_polygon(unsigned int vertex_count, ...);
// static void draw_polygon(polygon_t *polygon, ALLEGRO_COLOR color, float trace);
// static void kill_all_polygon(unsigned int count, ...);
static void insert_description(ALLEGRO_FONT *font, ALLEGRO_COLOR color, const char *text);
static void smart_description(button_t *play, button_t *settings, button_t *scoreboard, button_t *credits, button_t *exit, ALLEGRO_MOUSE_STATE *mouse, ALLEGRO_FONT *font, ALLEGRO_COLOR color, short int *screen_width, short int *screen_height);
static void background_slider_reset_except(short int n, short int *slider);

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

game_state_t menu_allegro(ALLEGRO_DISPLAY *display, ALLEGRO_TIMER *timer, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT *default_font, ALLEGRO_BITMAP *buffer, ALLEGRO_MIXER *mixer, void (*kill_all_bitmaps)(int, ...), void (*kill_all_instances)(int, ...), void (*kill_all_samples)(int, ...), void (*kill_all_font)(int, ...)) {

  /*************************DECLARATIONS************************/

  ALLEGRO_BITMAP *intro_background_frames[300];
  ALLEGRO_BITMAP *intro_logo;
  ALLEGRO_BITMAP *play_background;
  ALLEGRO_BITMAP *settings_background;
  ALLEGRO_BITMAP *scoreboard_background;
  ALLEGRO_BITMAP *credits_background;
  ALLEGRO_BITMAP *exit_background;

  ALLEGRO_FONT *font_toreks;
  ALLEGRO_FONT *font_toreks_big;
  ALLEGRO_FONT *font_supercharge;
  ALLEGRO_FONT *font_cartesian;

  ALLEGRO_EVENT menu_event;
  ALLEGRO_EVENT dummy;

  ALLEGRO_SAMPLE *logo_sound;
  ALLEGRO_SAMPLE *intro_part1;
  ALLEGRO_SAMPLE *intro_part2;

  ALLEGRO_SAMPLE_INSTANCE *sample_instance;
  ALLEGRO_SAMPLE_INSTANCE *sample_instance2;

  ALLEGRO_MOUSE_STATE mouse;

  button_t button_start;
  button_t button_play;
  button_t button_settings;
  button_t button_scoreboard;
  button_t button_credits;
  button_t button_exit;

  highscore_t top_scores[MAX_SCORES];

  char intro_background_path[64];

  short int counter_1, counter_2, counter_3;
  short int screen_width, screen_height, mouse_x, mouse_y;

  float button_slide_1; //, button_slide_2;
  float score_slide;

  bool redraw;
  bool fullscreen;
  bool flash_enable;
  bool menu_enable;
  bool menu_buttons_enable;
  bool menu_slide_done;

  /*
  counter_1 is used for the background frames loop
  counter_2 is used both for the initial "flashbang" and the background dark cover
  counter_3 is used for the sliding menu window
  */

  float main_menu_window[] = {WORLD_WIDTH, 0, WORLD_WIDTH, WORLD_HEIGHT, WORLD_WIDTH, WORLD_HEIGHT, WORLD_WIDTH, 0};

  /**************************************************************/

  /*************************DEFINITIONS**************************/

  ship_t intro_ship = {0, 0, 0, 0, {0}, NULL};

  choice_t select = INTRO;

  redraw = 0;
  fullscreen = al_get_display_flags(display) & ALLEGRO_FULLSCREEN_WINDOW;
  flash_enable = 1;
  menu_enable = 0;
  menu_buttons_enable = 0;

  score_slide = 1;

  short int background_slide[5] = {-WORLD_WIDTH / 2};

  float Accel_x = 0, Accel_y = 0;

  ALLEGRO_COLOR color_white = al_map_rgb(255, 255, 255);
  ALLEGRO_COLOR color_black = al_map_rgb(0, 0, 0);
  ALLEGRO_COLOR color_button_left = al_map_rgba(50, 50, 50, 180);
  ALLEGRO_COLOR color_button_right = al_map_rgba(45, 0, 160, 240);
  ALLEGRO_COLOR color_button_hover_right = al_map_rgba(70, 0, 255, 240);
  ALLEGRO_COLOR color_button_hover_left = al_map_rgba(255, 255, 255, 180);

  /**************************************************************/

  /************************INITIALIZERS**************************/

  sample_instance = al_create_sample_instance(NULL);
  if ( must_setup(sample_instance, "sample Instance") == CLOSED ) return CLOSED;
  sample_instance2 = al_create_sample_instance(NULL);
  if ( must_setup(sample_instance2, "sample Instance") == CLOSED ) return CLOSED;

  button_start = create_button(al_map_rgb(255, 125, 0), al_map_rgb(200, 0, 220), GRADIENT_CENTER, al_map_rgb(255, 0, 0), al_map_rgb(0, 0, 0), GRADIENT_CENTER, color_black, color_white, 4.0, 4, 50., 0., 0., 100., 200., 100., 250., 0.);
  if ( must_setup(button_start.valid, "Start button") == CLOSED ) return CLOSED;

  button_play = create_button(color_button_left, color_button_right, GRADIENT_LEFT_TO_RIGHT, color_button_hover_left, color_button_hover_right, GRADIENT_LEFT_TO_RIGHT, color_black, color_black, 5.0, 4, 16.0, 0.0, 0.0, 45.0, 350.0, 45.0, 350.0, 0.0);
  if ( must_setup(button_play.valid, "Play button") == CLOSED ) return CLOSED;

  button_settings = create_button(color_button_left, color_button_right, GRADIENT_LEFT_TO_RIGHT, color_button_hover_left, color_button_hover_right, GRADIENT_LEFT_TO_RIGHT, color_black, color_black, 5.0, 4, 16.0, 0.0, 0.0, 45.0, 370.0, 45.0, 370.0, 0.0);
  if ( must_setup(button_settings.valid, "Settings button") == CLOSED ) return CLOSED;

  button_scoreboard = create_button(color_button_left, color_button_right, GRADIENT_LEFT_TO_RIGHT, color_button_hover_left, color_button_hover_right, GRADIENT_LEFT_TO_RIGHT, color_black, color_black, 5.0, 4, 16.0, 0.0, 0.0, 45.0, 390.0, 45.0, 390.0, 0.0);
  if ( must_setup(button_scoreboard.valid, "Settings button") == CLOSED ) return CLOSED;

  button_exit = create_button(color_button_left, color_button_right, GRADIENT_LEFT_TO_RIGHT, color_button_hover_left, color_button_hover_right, GRADIENT_LEFT_TO_RIGHT, color_black, color_black, 5.0, 4, 0.0, 0.0, 0.0, 45.0, 195.0, 45.0, 195.0, 0.0);
  if ( must_setup(button_exit.valid, "Exit button") == CLOSED ) return CLOSED;

  button_credits = create_button(color_button_right, color_button_left, GRADIENT_LEFT_TO_RIGHT, color_button_hover_right, color_button_hover_left, GRADIENT_LEFT_TO_RIGHT, color_black, color_black, 5.0, 4, 16.0, 0.0, 0.0, 45.0, 195.0, 45.0, 195.0, 0.0);
  if ( must_setup(button_credits.valid, "Credits button") == CLOSED ) return CLOSED;

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
  play_background = al_load_bitmap(BITMAP_ROUTE("menu/play_background.png"));
  ERRCHECK(play_background, "play background");
  settings_background = al_load_bitmap(BITMAP_ROUTE("menu/settings_background.png"));
  ERRCHECK(settings_background, "setttings background");
  scoreboard_background = al_load_bitmap(BITMAP_ROUTE("menu/scoreboard_background.png"));
  ERRCHECK(scoreboard_background, "scoreboard background");
  credits_background = al_load_bitmap(BITMAP_ROUTE("menu/placeholder_background.png"));
  ERRCHECK(credits_background, "credits background");
  exit_background = al_load_bitmap(BITMAP_ROUTE("menu/exit_background.png"));
  ERRCHECK(exit_background, "exit background");

  logo_sound = al_load_sample(AUDIO_ROUTE("coin.wav"));
  ERRCHECK(logo_sound, "Coin Sound");
  intro_part1 = al_load_sample(AUDIO_ROUTE("intro_part1.wav"));
  ERRCHECK(intro_part1, "intro 1st part sound");
  intro_part2 = al_load_sample(AUDIO_ROUTE("intro_part2.wav"));
  ERRCHECK(intro_part1, "intro 2nd part sound");

  font_toreks = al_load_ttf_font(FONT_ROUTE("toreks-font/Toreks_regular.ttf"), 16, 0);
  ERRCHECK(font_toreks, "Toreks Font");
  font_toreks_big = al_load_ttf_font(FONT_ROUTE("toreks-font/Toreks_regular.ttf"), 24, 0);
  ERRCHECK(font_toreks_big, "Big Toreks Font");
  font_supercharge = al_load_ttf_font(FONT_ROUTE("supercharge-font/Supercharge_halftone.otf"), 24, 0);
  ERRCHECK(font_supercharge, "Supercharge Font");
  font_cartesian = al_load_ttf_font(FONT_ROUTE("cartesian-font/Cartesian_regular.ttf"), 12, 0);
  ERRCHECK(font_cartesian, "Cartesian Font");

  intro_ship.cx = al_get_bitmap_width(intro_ship.sprite) / 2;
  intro_ship.cy = al_get_bitmap_height(intro_ship.sprite) / 2;

  load_scores(top_scores);

  al_set_sample(sample_instance2, intro_part2);

  al_hide_mouse_cursor(display);

  /**************************************************************/

  /***********************FIRST PHASE****************************/

  // intro_anim(default_font, logo_sound, intro_part1, buffer, display, sample_instance, intro_ship.sprite);

  al_play_sample_instance(sample_instance2);

  /**************************************************************/

  /*************************PRE-LOOP*****************************/

  al_start_timer(timer);

  counter_1 = 0;
  counter_2 = 255;

  button_start.position_x = WORLD_WIDTH / 2;
  button_start.position_y = WORLD_HEIGHT * (3. / 5);

  // button_play.position_x = WORLD_WIDTH * 0.75;
  button_play.position_y = WORLD_HEIGHT * 0.15;

  // button_settings.position_x = WORLD_WIDTH * 0.74;
  button_settings.position_y = WORLD_HEIGHT * 0.25;

  // button_scoreboard.position_x = WORLD_WIDTH * 0.73;
  button_scoreboard.position_y = WORLD_HEIGHT * 0.35;

  // button_exit.position_x = WORLD_WIDTH * 0.83;
  button_exit.position_y = WORLD_HEIGHT * 0.45;

  // button_credits.position_x = WORLD_WIDTH * 0.61;
  button_credits.position_y = WORLD_HEIGHT * 0.45;

  screen_width = al_get_display_width(display);
  screen_height = al_get_display_height(display);

  mouse_x = mouse.x;
  mouse_y = mouse.y;

  while ( al_get_next_event(queue, &dummy) );

  /**************************************************************/

  /*************************MAIN-LOOP****************************/

  while ( select != QUIT && select != PLAY ) {
    al_wait_for_event(queue, &menu_event);
    switch ( select ) {

      case INTRO:

        switch ( menu_event.type ) {

          case ALLEGRO_EVENT_TIMER:

            BUFFER;
            CLEAR;

            redraw = true;
            al_get_mouse_state(&mouse);
            CURSOR_UPDATE;

            DRAW_BACKGROUND;
            DRAW_LOGO;

            if ( !mouse_hover_button(&button_start, &mouse, screen_width, screen_height) ) {
              al_draw_scaled_rotated_bitmap(button_start.sprite[0], button_start.size_x / 2, button_start.size_y / 2, button_start.position_x, button_start.position_y, 1, 1, 0, 0);
              al_draw_multiline_textf(font_supercharge, al_map_rgb(0, 0, 0), button_start.position_x, button_start.position_y - button_start.size_y * 0.25, button_start.size_x, 24, ALLEGRO_ALIGN_CENTER, "Dive inTo \n Space");
            } else {
              al_draw_scaled_rotated_bitmap(button_start.sprite[1], button_start.size_x / 2, button_start.size_y / 2, button_start.position_x, button_start.position_y, 1, 1, 0, 0);
              al_draw_multiline_textf(font_supercharge, al_map_rgb(255, 255, 255), button_start.position_x, button_start.position_y - button_start.size_y * 0.25, button_start.size_x, 24, ALLEGRO_ALIGN_CENTER, "Dive inTo \n Space");
            }

            DRAW_SHIP_CURSOR;

            DRAW_FLASH;
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

          case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:

            al_get_mouse_state(&mouse);
            if ( MOUSE_HOVER(button_start) && mouse.buttons & LEFT_CLICK ) {
              button_slide_1 = 1;
              select = MENU_MAIN;
              counter_2 = 0;
              flash_enable = 0;
              menu_enable = 1;
              menu_slide_done = 0;
              counter_3 = 15;
              al_show_mouse_cursor(display);
            }
            break;

          case ALLEGRO_EVENT_DISPLAY_CLOSE:

            select = QUIT;
            break;
        }

        break;

      case MENU_MAIN:

        switch ( menu_event.type ) {

          case ALLEGRO_EVENT_TIMER:
            BUFFER;
            CLEAR;

            al_get_mouse_state(&mouse);
            redraw = true;

            DRAW_BACKGROUND;
            DRAW_LOGO;

            al_draw_filled_rectangle(0, 0, WORLD_WIDTH, WORLD_HEIGHT, al_map_rgba(0, 0, 0, counter_2));
            if ( counter_2 < 150 && menu_enable )
              counter_2 += 15;
            else if ( counter_2 > 0 && !menu_enable && menu_slide_done )
              counter_2 -= 15;

            if ( counter_3 && menu_enable ) {
              main_menu_window[0] = WORLD_WIDTH * (1 - (1. / 2) / (counter_3));
              main_menu_window[2] = WORLD_WIDTH * (1 - (7. / 10) / (counter_3--));

            }

            else if ( counter_3 && !menu_enable && menu_slide_done ) {
              main_menu_window[0] = WORLD_WIDTH * (1. / 2 + (1. / 2) / (counter_3));
              main_menu_window[2] = WORLD_WIDTH * (3. / 10 + (7. / 10) / (counter_3--));
            }
            al_draw_filled_polygon(main_menu_window, 4, al_map_rgba(255, 255, 255, 180));

            if ( !counter_3 && menu_enable ) menu_buttons_enable = 1;

            if ( menu_buttons_enable || !menu_enable ) {

              if ( !menu_slide_done ) {
                button_play.position_x = (WORLD_WIDTH * 0.75) * (1 + 1 / button_slide_1);
                button_settings.position_x = (WORLD_WIDTH * 0.74) * (1 + 1 / button_slide_1);
                button_scoreboard.position_x = (WORLD_WIDTH * 0.73) * (1 + 1 / button_slide_1);
                button_exit.position_x = (WORLD_WIDTH * 0.83) * (1 + 1 / button_slide_1);
                button_credits.position_x = (WORLD_WIDTH * 0.61) * (1 + 1 / button_slide_1);

                if ( menu_buttons_enable ) {
                  if ( button_slide_1 < 100 )
                    button_slide_1 += 5;
                  else if ( button_slide_1 < 200 )
                    button_slide_1 += 20;
                  else
                    menu_slide_done = 1;
                } else {
                  if ( button_slide_1 > 101 )
                    button_slide_1 -= 20;
                  else if ( button_slide_1 > 1 )
                    button_slide_1 -= 5;
                  else
                    menu_slide_done = 1;
                }
              }

              DRAW_BUTTON(button_play);
              DRAW_BUTTON(button_settings);
              DRAW_BUTTON(button_scoreboard);
              DRAW_BUTTON(button_exit);
              DRAW_BUTTON(button_credits);

              BUTTON_TEXT(button_play, "Take off!");
              BUTTON_TEXT(button_settings, "Settings");
              BUTTON_TEXT(button_scoreboard, "Scoreboard");
              BUTTON_TEXT(button_exit, "Quit game");
              BUTTON_TEXT(button_credits, "Credits");

              smart_description(&button_play, &button_settings, &button_scoreboard, &button_credits, &button_exit, &mouse, font_cartesian, color_black, &screen_width, &screen_height);

              if ( MOUSE_HOVER(button_play) ) {
                background_slider_reset_except(0, background_slide);
                al_draw_bitmap(play_background, background_slide[0], 0, 0);
                if ( background_slide[0] < 0 ) background_slide[0] += WORLD_WIDTH / 10;
              } else if ( MOUSE_HOVER(button_settings) ) {
                background_slider_reset_except(1, background_slide);
                al_draw_bitmap(settings_background, background_slide[1], 0, 0);
                if ( background_slide[1] < 0 ) background_slide[1] += WORLD_WIDTH / 10;
              } else if ( MOUSE_HOVER(button_scoreboard) ) {
                background_slider_reset_except(2, background_slide);
                al_draw_bitmap(scoreboard_background, background_slide[2], 0, 0);
                if ( background_slide[2] < 0 ) background_slide[2] += WORLD_WIDTH / 10;
              } else if ( MOUSE_HOVER(button_credits) ) {
                background_slider_reset_except(3, background_slide);
                al_draw_bitmap(credits_background, background_slide[3], 0, 0);
                if ( background_slide[3] < 0 ) background_slide[3] += WORLD_WIDTH / 10;
              } else if ( MOUSE_HOVER(button_exit) ) {
                background_slider_reset_except(4, background_slide);
                al_draw_bitmap(exit_background, background_slide[4], 0, 0);
                if ( background_slide[4] < 0 ) background_slide[4] += WORLD_WIDTH / 10;
              } else {
                background_slider_reset_except(5, background_slide);
              }

              BACK_TEXT;
            }

            if ( counter_2 <= 0 && !menu_enable && !counter_3 ) {

              select = INTRO;
              counter_2 = 0;
              // menu_enable = 0;
              al_hide_mouse_cursor(display);
              CURSOR_UPDATE;
            }

            break;

          case ALLEGRO_EVENT_KEY_DOWN:
            if ( menu_event.keyboard.keycode == ALLEGRO_KEY_ESCAPE && menu_enable ) {
              menu_enable = 0;
              menu_buttons_enable = 0;
              menu_slide_done = 0;
              counter_3 = 15;
            }
            break;

          case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            al_get_mouse_state(&mouse);
            if ( mouse.buttons && LEFT_CLICK ) {
              if ( MOUSE_HOVER(button_play) )
                select = PLAY;
              else if ( MOUSE_HOVER(button_settings) )
                ; // select = MENU_SETTINGS;
              else if ( MOUSE_HOVER(button_scoreboard) ) {
                select = MENU_SCORE;
                score_slide = 1;
              } else if ( MOUSE_HOVER(button_credits) )
                ;
              else if ( MOUSE_HOVER(button_exit) )
                select = QUIT;
            }
        }

        break;

      case MENU_SCORE:

        switch ( menu_event.type ) {
          case ALLEGRO_EVENT_KEY_DOWN:
            if ( menu_event.keyboard.keycode == ALLEGRO_KEY_ESCAPE ) select = MENU_MAIN;
            break;
          case ALLEGRO_EVENT_TIMER:
            BUFFER;
            CLEAR;

            DRAW_BACKGROUND;
            DRAW_LOGO;
            al_draw_filled_rectangle(0, 0, WORLD_WIDTH, WORLD_HEIGHT, al_map_rgba(0, 0, 0, counter_2));
            al_draw_filled_polygon(main_menu_window, 4, al_map_rgba(255, 255, 255, 180));

            al_draw_text(font_supercharge, color_black, (WORLD_WIDTH * 0.55) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.1, 0, "LeaderboaRd");

            al_draw_text(font_toreks, al_map_rgb(255, 0, 0), (WORLD_WIDTH * 0.55) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.2, 0, "1");
            al_draw_text(font_toreks_big, al_map_rgb(255, 0, 0), (WORLD_WIDTH * 0.6) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.2 - 7, 0, top_scores[0].name);
            al_draw_textf(font_toreks, al_map_rgb(255, 0, 0), (WORLD_WIDTH * 0.75) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.2, 0, "%d", top_scores[0].score);

            al_draw_text(font_toreks, al_map_rgb(255, 160, 0), (WORLD_WIDTH * 0.55) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.25, 0, "2");
            al_draw_text(font_toreks_big, al_map_rgb(255, 160, 0), (WORLD_WIDTH * 0.6) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.25 - 7, 0, top_scores[1].name);
            al_draw_textf(font_toreks, al_map_rgb(255, 160, 0), (WORLD_WIDTH * 0.75) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.25, 0, "%d", top_scores[1].score);

            al_draw_text(font_toreks, al_map_rgb(255, 255, 0), (WORLD_WIDTH * 0.55) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.3, 0, "3");
            al_draw_text(font_toreks_big, al_map_rgb(255, 255, 0), (WORLD_WIDTH * 0.6) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.3 - 7, 0, top_scores[2].name);
            al_draw_textf(font_toreks, al_map_rgb(255, 255, 0), (WORLD_WIDTH * 0.75) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.3, 0, "%d", top_scores[2].score);

            al_draw_text(font_toreks, al_map_rgb(190, 255, 0), (WORLD_WIDTH * 0.55) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.35, 0, "4");
            al_draw_text(font_toreks_big, al_map_rgb(190, 255, 0), (WORLD_WIDTH * 0.6) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.35 - 7, 0, top_scores[3].name);
            al_draw_textf(font_toreks, al_map_rgb(190, 255, 0), (WORLD_WIDTH * 0.75) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.35, 0, "%d", top_scores[3].score);

            al_draw_text(font_toreks, al_map_rgb(120, 255, 0), (WORLD_WIDTH * 0.55) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.4, 0, "5");
            al_draw_text(font_toreks_big, al_map_rgb(120, 255, 0), (WORLD_WIDTH * 0.6) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.4 - 7, 0, top_scores[4].name);
            al_draw_textf(font_toreks, al_map_rgb(120, 255, 0), (WORLD_WIDTH * 0.75) * (1 + 1 / score_slide), WORLD_HEIGHT * 0.4, 0, "%d", top_scores[4].score);

            if ( score_slide < 50 )
              score_slide += 5;
            else if ( score_slide < 100 )
              score_slide += 10;

            BACK_TEXT;

            redraw = 1;
            break;
        }
        break;

      default:
        break;
    }

    REDRAW;
  }

  /**************************************************************/

  /***********************RESOURCE-FREEING***********************/

  for ( counter_1 = 0; counter_1 < 300; counter_1++ ) (*kill_all_bitmaps)(1, intro_background_frames[counter_1]);
  (*kill_all_bitmaps)(7, intro_ship.sprite, intro_logo, play_background, settings_background, scoreboard_background, exit_background, credits_background);
  (*kill_all_samples)(3, logo_sound, intro_part1, intro_part2);
  (*kill_all_instances)(2, sample_instance, sample_instance2);
  (*kill_all_font)(4, font_cartesian, font_supercharge, font_toreks, font_toreks_big);
  kill_all_button(6, &button_start, &button_play, &button_settings, &button_scoreboard, &button_credits, &button_exit);

  al_stop_timer(timer);

  /**************************************************************/

  /****************************RETURN****************************/

  al_hide_mouse_cursor(display);

  if ( select == QUIT ) return CLOSED;
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

  float trail[20] = {
      WORLD_WIDTH / 2 - 4, WORLD_HEIGHT, WORLD_WIDTH / 2 - 8, WORLD_HEIGHT, WORLD_WIDTH / 2 - 16, WORLD_HEIGHT, WORLD_WIDTH / 2 - 20, WORLD_HEIGHT, WORLD_WIDTH / 2 - 48, WORLD_HEIGHT, WORLD_WIDTH / 2 + 48, WORLD_HEIGHT, WORLD_WIDTH / 2 + 20, WORLD_HEIGHT, WORLD_WIDTH / 2 + 16, WORLD_HEIGHT, WORLD_WIDTH / 2 + 8, WORLD_HEIGHT, WORLD_WIDTH / 2 + 4, WORLD_HEIGHT};

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

/******************DRAW_RECTANGLE_GRADIENT*********************/

void draw_rectangle_gradient(float center_x, float center_y, float radius_x, float radius_y, gradient_mode_t mode, ALLEGRO_COLOR color_1, ALLEGRO_COLOR color_2, int steps) {
  if ( steps <= 0 || (mode != GRADIENT_CENTER && mode != GRADIENT_LEFT_TO_RIGHT && mode != GRADIENT_TOP_TO_BOTTOM) ) return;
  ALLEGRO_COLOR gradient;
  int counter_1;
  float gradient_x_step = radius_x / steps, gradient_y_step = radius_y / steps;
  float r_step = (color_1.r - color_2.r) / steps;
  float g_step = (color_1.g - color_2.g) / steps;
  float b_step = (color_1.b - color_2.b) / steps;
  float a_step = (color_1.a - color_2.a) / steps;

  switch ( mode ) {
    case GRADIENT_CENTER:
      for ( counter_1 = steps; counter_1 > 0; counter_1-- ) {
        gradient.r = color_2.r + r_step * counter_1;
        gradient.g = color_2.g + g_step * counter_1;
        gradient.b = color_2.b + b_step * counter_1;
        gradient.a = color_2.a + a_step * counter_1;
        al_draw_filled_rectangle(center_x - gradient_x_step * counter_1, center_y - gradient_y_step * counter_1, center_x + gradient_x_step * counter_1, center_y + gradient_y_step * counter_1, gradient);
      }
      break;

    case GRADIENT_TOP_TO_BOTTOM:
      for ( counter_1 = steps; counter_1 > 0; counter_1-- ) {
        gradient.r = color_1.r - r_step * counter_1;
        gradient.g = color_1.g - g_step * counter_1;
        gradient.b = color_1.b - b_step * counter_1;
        gradient.a = color_1.a - a_step * counter_1;
        al_draw_filled_rectangle(center_x - radius_x, center_y - radius_y, center_x + radius_x, center_y - radius_y + 2 * gradient_y_step * counter_1, gradient);
      }
      break;

    case GRADIENT_LEFT_TO_RIGHT:
      for ( counter_1 = steps; counter_1 > 0; counter_1-- ) {
        gradient.r = color_2.r + r_step * counter_1;
        gradient.g = color_2.g + g_step * counter_1;
        gradient.b = color_2.b + b_step * counter_1;
        gradient.a = color_2.a + a_step * counter_1;
        al_draw_filled_rectangle(center_x + radius_x - 2 * gradient_x_step * counter_1, center_y - radius_y, center_x + radius_x, center_y + radius_y, gradient);
      }
      break;
  }
}

/*polygon_t assign_polygon(unsigned int vertex_count, ...) {
  polygon_t polygon = {0};

  if ( !vertex_count ) return polygon;

  va_list va_vertex;
  va_start(va_vertex, vertex_count);

  polygon.coords = calloc(2 * vertex_count, sizeof(float));
  if ( !polygon.coords ) return polygon;

  unsigned int counter_1;
  for ( counter_1 = 0; counter_1 < 2 * vertex_count; counter_1++ ) {
    polygon.coords[counter_1] = (float) va_arg(va_vertex, double);
  }

  polygon.vertex = vertex_count;
  va_end(va_vertex);

  return polygon;
}*/

/*void kill_all_polygon(unsigned int count, ...) {
  va_list polygon_list;
  va_start(polygon_list, count);

  int counter_1;

  for ( counter_1 = 0; counter_1 < count; counter_1++ ) {
    free(va_arg(polygon_list, polygon_t *)->coords);
  }
}*/

/*void draw_polygon(polygon_t *polygon, ALLEGRO_COLOR color, float trace) {
  if ( !polygon ) return;

  if ( trace )
    al_draw_polygon(polygon->coords, polygon->vertex, ALLEGRO_LINE_JOIN_ROUND, color, trace, 1);
  else
    al_draw_filled_polygon(polygon->coords, polygon->vertex, color);
}*/

button_t create_button(ALLEGRO_COLOR color_1, ALLEGRO_COLOR color_2, gradient_mode_t gradient, ALLEGRO_COLOR color_hover_1, ALLEGRO_COLOR color_hover_2, gradient_mode_t gradient_hover, ALLEGRO_COLOR color_trace_1, ALLEGRO_COLOR color_trace_2, float line_width, unsigned short int vertex_count, ...) {
  button_t button = {0};
  float *vertex = calloc(2 * vertex_count, sizeof(float));
  if ( !vertex ) return button;

  va_list vertex_list;
  va_start(vertex_list, vertex_count);
  unsigned short int counter_1;
  float min_x, max_x, min_y, max_y;

  for ( counter_1 = 0; counter_1 < vertex_count; counter_1++ ) {
    vertex[2 * counter_1] = va_arg(vertex_list, double) + line_width * 0.5 + (line_width <= 0 ? 0.1 : 0);
    vertex[2 * counter_1 + 1] = va_arg(vertex_list, double) + line_width * 0.5 + (line_width <= 0 ? 0.1 : 0);

    if ( !counter_1 ) {
      min_x = vertex[2 * counter_1];
      max_x = min_x;
      min_y = vertex[2 * counter_1 + 1];
      max_y = min_y;
    }

    if ( min_x > vertex[2 * counter_1] ) min_x = vertex[2 * counter_1];
    if ( max_x < vertex[2 * counter_1] ) max_x = vertex[2 * counter_1];

    if ( min_y > vertex[2 * counter_1 + 1] ) min_y = vertex[2 * counter_1 + 1];
    if ( max_y < vertex[2 * counter_1 + 1] ) max_y = vertex[2 * counter_1 + 1];
  }

  button.size_x = (max_x - min_x) + (line_width);
  button.size_y = (max_y - min_y) + (line_width);

  va_end(vertex_list);

  ALLEGRO_BITMAP *background;
  ALLEGRO_BITMAP *mask;
  ALLEGRO_BITMAP *original_buffer = al_get_target_bitmap();

  int flag_1, flag_2, flag_3;
  al_get_blender(&flag_1, &flag_2, &flag_3);

  button.sprite[0] = al_create_bitmap(button.size_x, button.size_y);
  button.sprite[1] = al_create_bitmap(button.size_x, button.size_y);
  background = al_create_bitmap(button.size_x, button.size_y);
  mask = al_create_bitmap(button.size_x, button.size_y);

  if ( !button.sprite[0] ) return button;
  if ( !button.sprite[1] ) {
    al_destroy_bitmap(button.sprite[0]);
    return button;
  }
  if ( !background ) {
    al_destroy_bitmap(button.sprite[0]);
    al_destroy_bitmap(button.sprite[1]);
    return button;
  }
  if ( !mask ) {
    al_destroy_bitmap(button.sprite[0]);
    al_destroy_bitmap(button.sprite[1]);
    al_destroy_bitmap(background);
    return button;
  }

  /**************************************/

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
  al_set_target_bitmap(background);
  CLEAR_A;
  al_draw_filled_polygon(vertex, vertex_count, al_map_rgb(255, 255, 255));

  al_set_target_bitmap(mask);
  CLEAR;
  al_set_blender(ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ZERO, ALLEGRO_INVERSE_ALPHA);
  al_draw_bitmap(background, 0, 0, 0);

  al_set_target_bitmap(background);
  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
  CLEAR_A;
  draw_rectangle_gradient(button.size_x * 0.5, button.size_y * 0.5, button.size_x * 0.5, button.size_y * 0.5, gradient, color_1, color_2, button.size_x > button.size_y ? button.size_x * 0.5 : button.size_y * 0.5);
  al_set_blender(ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ZERO, ALLEGRO_INVERSE_ALPHA);
  al_draw_bitmap(mask, 0, 0, 0);

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
  al_set_target_bitmap(button.sprite[0]);
  CLEAR_A;
  al_draw_bitmap(background, 0, 0, 0);

  /**************************************/

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
  al_set_target_bitmap(background);
  CLEAR_A;
  al_draw_filled_polygon(vertex, vertex_count, al_map_rgb(255, 255, 255));

  al_set_target_bitmap(mask);
  CLEAR;
  al_set_blender(ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ZERO, ALLEGRO_INVERSE_ALPHA);
  al_draw_bitmap(background, 0, 0, 0);

  al_set_target_bitmap(background);
  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
  CLEAR_A;
  draw_rectangle_gradient(button.size_x * 0.5, button.size_y * 0.5, button.size_x * 0.5, button.size_y * 0.5, gradient_hover, color_hover_1, color_hover_2, button.size_x > button.size_y ? button.size_x * 0.5 : button.size_y * 0.5);
  al_set_blender(ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ZERO, ALLEGRO_INVERSE_ALPHA);
  al_draw_bitmap(mask, 0, 0, 0);

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
  al_set_target_bitmap(button.sprite[1]);
  CLEAR_A;
  al_draw_bitmap(background, 0, 0, 0);

  /**************************************/

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

  al_set_target_bitmap(button.sprite[0]);
  al_draw_polygon(vertex, vertex_count, ALLEGRO_LINE_JOIN_ROUND, color_trace_1, line_width, 1.0);
  al_set_target_bitmap(button.sprite[1]);
  al_draw_polygon(vertex, vertex_count, ALLEGRO_LINE_JOIN_ROUND, color_trace_2, line_width, 1.0);

  al_destroy_bitmap(background);
  al_destroy_bitmap(mask);
  al_set_blender(flag_1, flag_2, flag_3);
  free(vertex);

  if ( original_buffer ) al_set_target_bitmap(original_buffer);

  button.valid = 1;
  return button;
}

void kill_all_button(int count, ...) {
  va_list list;
  va_start(list, count);

  int counter_1;
  button_t *button;

  for ( counter_1 = 0; counter_1 < count; counter_1++ ) {
    button = va_arg(list, button_t *);
    al_destroy_bitmap(button->sprite[0]);
    al_destroy_bitmap(button->sprite[1]);
    button->valid = 0;
  }
}

bool mouse_hover_button(button_t *button, ALLEGRO_MOUSE_STATE *mouse, float size_x, float size_y) {
  int position_x = mouse->x * (WORLD_WIDTH / size_x);
  int position_y = mouse->y * (WORLD_HEIGHT / size_y);
  return (position_x >= (button->position_x - button->size_x / 2) && position_x <= (button->position_x + button->size_x / 2) && position_y >= (button->position_y - button->size_y / 2) && position_y <= (button->position_y + button->size_y / 2));
}

void draw_button(ALLEGRO_MOUSE_STATE *mouse, float screen_width, float screen_height, button_t *button) {
  if ( mouse_hover_button(button, mouse, screen_width, screen_height) )
    al_draw_bitmap(button->sprite[1], button->position_x - (button->size_x * 0.5), button->position_y - (button->size_y * 0.5), 0);
  else
    al_draw_bitmap(button->sprite[0], button->position_x - (button->size_x * 0.5), button->position_y - (button->size_y * 0.5), 0);
}

void draw_smart_text(ALLEGRO_MOUSE_STATE *mouse, float screen_width, float screen_height, button_t *button, ALLEGRO_FONT *font, ALLEGRO_COLOR color_default, ALLEGRO_COLOR color_hover, char alignment, const char *text) {
  ALLEGRO_COLOR color_text;
  float position_y = button->position_y - (al_get_font_line_height(font) >> 1);
  float position_x;
  if ( mouse_hover_button(button, mouse, screen_width, screen_height) )
    color_text = color_hover;
  else
    color_text = color_default;

  switch ( alignment ) {
    case ALLEGRO_ALIGN_LEFT:
      position_x = button->position_x - button->size_x * 0.5 + al_get_font_line_height(font);
      break;
    case ALLEGRO_ALIGN_RIGHT:
      position_x = button->position_x + button->size_x * 0.5 - al_get_font_line_height(font);
      break;
    default:
      position_x = button->position_x;
      break;
  }
  al_draw_text(font, color_text, position_x, position_y, alignment, text);
}

void insert_description(ALLEGRO_FONT *font, ALLEGRO_COLOR color, const char *text) {
  al_draw_multiline_text(font, color, 0.4 * WORLD_WIDTH, 0.7 * WORLD_HEIGHT, 0.55 * WORLD_WIDTH, al_get_font_line_height(font), 0, text);
}

void smart_description(button_t *play, button_t *settings, button_t *scoreboard, button_t *credits, button_t *exit, ALLEGRO_MOUSE_STATE *mouse, ALLEGRO_FONT *font, ALLEGRO_COLOR color, short int *screen_width, short int *screen_height) {
  if ( mouse_hover_button(play, mouse, *screen_width, *screen_height) )
    insert_description(font, color, " Launch into space!\n"
                                    " Fend off the aliens to protect your crew. Beware of their ferocious attacks and relentless approach.\n"
                                    " Fear not, captain! Your trustworhty MKII blaster will be the key to success, but make sure your aim is as good as your dodging skills.\n"
                                    " Keys:\n"
                                    " Left arrow: Move to the left.\n"
                                    " Right arrow: Move to the right.\n"
                                    " X: Fire bullets.");
  else if ( mouse_hover_button(settings, mouse, *screen_width, *screen_height) )
    insert_description(font, color, " Want to play with some tweaks?\n"
                                    " Change the windows size, as well as the fullscreen setting\n"
                                    " Pretty much nothing else...");
  else if ( mouse_hover_button(scoreboard, mouse, *screen_width, *screen_height) )
    insert_description(font, color, " Check out the scoreboard!\n"
                                    " Want to become the king of the leaderboard?\n"
                                    " Take a look at the top 10 players and their scores.\n"
                                    " Who knows? maybe you will get to be in the podium!");
  else if ( mouse_hover_button(credits, mouse, *screen_width, *screen_height) )
    insert_description(font, color, " Credits!\n"
                                    " That's it.\n"
                                    " Credits...\n");
  else if ( mouse_hover_button(exit, mouse, *screen_width, *screen_height) )
    insert_description(font, color, " Exit the game\n"
                                    " Take a break and come back full of energy!\n"
                                    " Or maybe you have skill issue.");
}

void background_slider_reset_except(short int n, short int *slider) {
  short int counter_1;
  for ( counter_1 = 0; counter_1 < 5; counter_1++ ) {
    if ( counter_1 != n ) slider[counter_1] = -WORLD_WIDTH / 2;
  }
}