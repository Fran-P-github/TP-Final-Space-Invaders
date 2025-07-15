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

/****************************************FOR SETUPS********************************************/

// Default message when there is an error.
#define MSJ_ERR_INIT "Error with setup: "
// Checks for errors using must_setup.
#define ERRCHECK(obj, txt) \
  if ( must_setup(obj, txt) == 0 ) return 0

/**********************************************************************************************/

/************************************FOR GENERAL DRAWING***************************************/

// Draws a frame.
#define DRAW draw_frame(display, buffer)
// Sets the target buffer.
#define BUFFER al_set_target_bitmap(buffer)
// Clears the target bitmap to black.
#define CLEAR al_clear_to_color(al_map_rgb(0, 0, 0))
// Clears the target bitmap to transparent.
#define CLEAR_A al_clear_to_color(al_map_rgba(0, 0, 0, 0))
// Resets the REDRAW flag and Draws.
#define REDRAW      \
  if ( redraw ) {   \
    DRAW;           \
    redraw = false; \
  }

/**********************************************************************************************/

/**********************************FOR ANIMATIONS**********************************************/

#define DRAW_BACKGROUND draw_background(intro_background_frames)

#define DRAW_FLASH \
  if ( flash_enable ) draw_flash(&flash_enable)

#define DRAW_SHIP_CURSOR                                                                                                                                                                        \
  al_draw_scaled_rotated_bitmap(intro_ship.sprite, intro_ship.cx, intro_ship.cy, mouse.x *WORLD_WIDTH / screen_width, mouse.y * WORLD_HEIGHT / screen_height, 0.5, 0.15 + Accel_y, Accel_x, 0); \
  al_draw_filled_polygon(intro_ship.trail, 10, al_map_rgba(255, 255, 255, 200))

#define DRAW_LOGO \
  al_draw_scaled_rotated_bitmap(intro_logo, al_get_bitmap_width(intro_logo) / 2, al_get_bitmap_height(intro_logo) / 2, WORLD_WIDTH / 2, WORLD_HEIGHT / 4, 1, 1, 0, 0)

#define CURSOR_UPDATE                                                 \
  trail_accel(intro_ship.trail, &mouse, screen_width, screen_height); \
  cursor_accel(&mouse, &Accel_x, &Accel_y)

/**********************************************************************************************/

/**************************************FOR BUTTONS*********************************************/

// Wrapper for drawing the button. Simpler and quicker
#define DRAW_BUTTON(button) draw_button(&mouse, screen_width, screen_height, &(button))
// Wrapper for drawing the button. Simpler and quicker. This one is used on functions that are not menu_allegro.
#define DRAW_BUTTON_FOR_FUNCTIONS(button) draw_button(mouse, screen_width, screen_height, button)
// Wrapper for drawing text over a certain button.
#define BUTTON_TEXT(button, txt) draw_smart_text(&mouse, screen_width, screen_height, &(button), font_supercharge, color_black, color_white, ALLEGRO_ALIGN_RIGHT, (txt))
// Wrapper for drawing text over a certain button. This one is used on functions that are not menu_allegro.
#define BUTTON_TEXT_FOR_FUNCTIONS(font, button, txt) draw_smart_text(mouse, screen_width, screen_height, button, (font), *color_black, *color_white, ALLEGRO_ALIGN_RIGHT, (txt))

/**********************************************************************************************/

/**************************************MENU SECTIONS*******************************************/

// Displays the main menu buttons.
#define MAIN_MENU_BUTTONS(mode) main_menu_buttons(&button_play, &button_settings, &button_scoreboard, &button_exit, &button_credits, &mouse, screen_width, screen_height, font_supercharge, font_cartesian, &color_black, &color_white, (mode), play_background, settings_background, scoreboard_background, credits_background, exit_background)
// Displays the settings.
#define MENU_SETTINGS(reset) menu_settings(&button_left_arrow_enabled, &button_right_arrow_enabled, &button_left_arrow_disabled, &button_right_arrow_disabled, &button_fullscreen, &button_apply, &mouse, screen_width, screen_height, font_supercharge, font_toreks, font_toreks_big, font_cartesian, &color_black, &color_grey, &color_white, &fullscreen, (reset), window_sizes, display_size_counter)
// Displays the scoreboard.
#define MENU_SCOREBOARD(reset) menu_scoreboard(font_supercharge, font_toreks, font_toreks_big, top_scores, (reset))
// Displays the credits.
#define MENU_CREDITS(reset) menu_credits(font_supercharge, font_toreks, font_toreks_big, &color_black, (reset))

/**********************************************************************************************/

/******************************************OTHERS**********************************************/

// Wrapper to detect which button is the mouse hovering over.
#define MOUSE_HOVER(button) mouse_hover_button(&(button), &mouse, screen_width, screen_height)
// Wrapper to detect which button is the mouse hovering over. This one is used on functions that are not menu_allegro.
#define MOUSE_HOVER_FOR_FUNCTION(button) mouse_hover_button(button, mouse, screen_width, screen_height)
// Displays the text to go back.
#define BACK_TEXT al_draw_text(font_cartesian, color_black, WORLD_WIDTH * 0.85, WORLD_WIDTH * 0.02, 0, "Press [ESC] to go back")
// Displays the text to go back. This one is used on functions that are not menu_allegro.
#define BACK_TEXT_FOR_FUNCTIONS(font, color) al_draw_text((font), *(color), WORLD_WIDTH * 0.85, WORLD_WIDTH * 0.02, 0, "Press [ESC] to go back")
// Plays a sound sample with predefined properties.
#define PLAY_SOUND(src) al_play_sample((src), 0.25, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);

/**********************************************************************************************/

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

typedef enum { HIDE = 0,
               SHOW,
               EXTEND,
               SHRINK,
               RESET } window_t;

typedef struct {
  int px;       // X position
  int py;       // Y position
  short int cx; // X center
  short int cy; // Y center
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

/******************************************SETUP***********************************************/

// Makes sure everything setups correctly. Returns 0 on error.
static bool must_setup(bool task, char *msg);
// Assigns all buttons with their corresponding sprites. Returns 0 on error.
static bool setup_buttons(button_t *button_start, button_t *button_play, button_t *button_settings, button_t *button_scoreboard, button_t *button_credits, button_t *button_exit, button_t *button_left_arrow_enabled, button_t *button_left_arrow_disabled, button_t *button_right_arrow_enabled, button_t *button_right_arrow_disabled, button_t *button_fullscreen, button_t *button_apply);
// Loads all bitmaps. Returns 0 on error.
static bool setup_bitmaps(ALLEGRO_BITMAP **intro_logo, ALLEGRO_BITMAP **intro_ship_sprite, ALLEGRO_BITMAP **play_background, ALLEGRO_BITMAP **settings_background, ALLEGRO_BITMAP **scoreboard_background, ALLEGRO_BITMAP **credits_background, ALLEGRO_BITMAP **exit_background);
// Loads all audio samples. Returns 0 on error.
static bool setup_audio_samples(ALLEGRO_SAMPLE **logo_sound, ALLEGRO_SAMPLE **intro_part1, ALLEGRO_SAMPLE **intro_part2, ALLEGRO_SAMPLE **menu_enter, ALLEGRO_SAMPLE **menu_back, ALLEGRO_SAMPLE **menu_locked, ALLEGRO_SAMPLE **menu_modify);
// Loads all fonts. Returns 0 on error.
static bool setup_fonts(ALLEGRO_FONT **font_toreks, ALLEGRO_FONT **font_toreks_big, ALLEGRO_FONT **font_supercharge, ALLEGRO_FONT **font_cartesian);
// Loads all frames of the background. Returns 0 on error.
static bool setup_background(ALLEGRO_BITMAP **intro_background_frames);
// Sets positions values that won't change over time.
static void setup_buttons_static_positions(button_t *button_start, button_t *button_play, button_t *button_settings, button_t *button_scoreboard, button_t *button_credits, button_t *button_exit, button_t *button_left_arrow_enabled, button_t *button_left_arrow_disabled, button_t *button_right_arrow_enabled, button_t *button_right_arrow_disabled, button_t *button_fullscreen, button_t *button_apply);
// Detects the maximum screen size and creates different possible resolutions for later use, including native WORLD_WIDTH and WORLD_HEIGHT. Returns the counter to later iterate through these options.
static short int detect_screen_sizes(short int *window_sizes);

/**********************************************************************************************/

/************************************ANIMATIONS************************************************/

// First animation.
static void intro_anim(ALLEGRO_FONT *dfont, ALLEGRO_SAMPLE *s_logo, ALLEGRO_SAMPLE *sample_intro_1, ALLEGRO_BITMAP *buffer, ALLEGRO_DISPLAY *display, ALLEGRO_SAMPLE_INSTANCE *sample_instance, ALLEGRO_BITMAP *ship);
// Calculates the values to mimic the ship's movement.
static void cursor_accel(ALLEGRO_MOUSE_STATE *mouse, float *Accel_x, float *Accel_y);
// Calculates the values to mimic the ships's trail.
static void trail_accel(float *polygon, ALLEGRO_MOUSE_STATE *mouse, int width, int height);
// Loads each background to the buffer, one by one, cycling through its range.
static void draw_background(ALLEGRO_BITMAP **intro_background_frames);
// Draws the intial flashbang
static void draw_flash(bool *flash_enable);
// Calls the buffer and draws over the display.
static void draw_frame(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *buffer);
// Draws the dark cover. Returns 1 when the transition is completed
static bool draw_dark_cover(bool enable);

/**********************************************************************************************/

/**********************************MENU_BROWSING***********************************************/

// Shows menu sliding window. Returns 0 when its animation has been completed.
static bool menu_slide_window(window_t mode);
// Shows the buttons for the main menu. Returns 0 when its animation has been completed.
static bool main_menu_buttons(button_t *button_play, button_t *button_settings, button_t *button_scoreboard, button_t *button_exit, button_t *button_credits, ALLEGRO_MOUSE_STATE *mouse, short int screen_width, short int screen_height, ALLEGRO_FONT *font_supercharge, ALLEGRO_FONT *font_cartesian, ALLEGRO_COLOR *color_black, ALLEGRO_COLOR *color_white, window_t mode, ALLEGRO_BITMAP *play_background, ALLEGRO_BITMAP *settings_background, ALLEGRO_BITMAP *scoreboard_background, ALLEGRO_BITMAP *credits_background, ALLEGRO_BITMAP *exit_background);
// Displays the settings tab.
static void menu_settings(button_t *button_left_arrow_enabled, button_t *button_right_arrow_enabled, button_t *button_left_arrow_disabled, button_t *button_right_arrow_disabled, button_t *button_fullscreen, button_t *button_apply, ALLEGRO_MOUSE_STATE *mouse, short int screen_width, short int screen_height, ALLEGRO_FONT *font_supercharge, ALLEGRO_FONT *font_toreks, ALLEGRO_FONT *font_toreks_big, ALLEGRO_FONT *font_cartesian, ALLEGRO_COLOR *color_black, ALLEGRO_COLOR *color_grey, ALLEGRO_COLOR *color_white, bool *fullscreen, bool reset, short int *window_sizes, short int display_size_counter);
// Displays the scoreboard
static void menu_scoreboard(ALLEGRO_FONT *font_supercharge, ALLEGRO_FONT *font_toreks, ALLEGRO_FONT *font_toreks_big, highscore_t *top_scores, bool reset);
// Displays the credits
static void menu_credits(ALLEGRO_FONT *font_supercharge, ALLEGRO_FONT *font_toreks, ALLEGRO_FONT *font_toreks_big, ALLEGRO_COLOR *color_black, bool reset);
// Inserts a description at the bottom of the screen.
static void insert_description(ALLEGRO_FONT *font, ALLEGRO_COLOR color, const char *text);
// Dependending on the mouse position, it shows different descriptions.
static void smart_description(button_t *play, button_t *settings, button_t *scoreboard, button_t *credits, button_t *exit, ALLEGRO_MOUSE_STATE *mouse, ALLEGRO_FONT *font, ALLEGRO_COLOR color, short int *screen_width, short int *screen_height);
// Resets the sliders for each background when the mouse moves somewhere else.
static void background_slider_reset_except(short int n, short int *slider);

/**********************************************************************************************/

/*************************************CLEANING*************************************************/

// Destroys all frames to release used space.
static void kill_background(ALLEGRO_BITMAP **intro_background_frames, void (*kill_all_bitmaps)(int counter, ...));

/**********************************************************************************************/

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

/**************************************MENU_ALLEGRO********************************************/

bool menu_allegro(ALLEGRO_DISPLAY *display, ALLEGRO_TIMER *timer, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT *default_font, ALLEGRO_BITMAP *buffer, ALLEGRO_MIXER *mixer, void (*kill_all_bitmaps)(int, ...), void (*kill_all_instances)(int, ...), void (*kill_all_samples)(int, ...), void (*kill_all_font)(int, ...)) {

  /*************DECLARATIONS*****************/

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

  ALLEGRO_SAMPLE *menu_enter;
  ALLEGRO_SAMPLE *menu_back;
  ALLEGRO_SAMPLE *menu_locked;
  ALLEGRO_SAMPLE *menu_modify;

  ALLEGRO_SAMPLE_INSTANCE *sample_instance;
  ALLEGRO_SAMPLE_INSTANCE *sample_instance2;

  ALLEGRO_MOUSE_STATE mouse;

  button_t button_start;

  button_t button_play;
  button_t button_settings;
  button_t button_scoreboard;
  button_t button_credits;
  button_t button_exit;

  button_t button_left_arrow_enabled;
  button_t button_right_arrow_enabled;
  button_t button_left_arrow_disabled;
  button_t button_right_arrow_disabled;
  button_t button_fullscreen;
  button_t button_apply;

  highscore_t top_scores[MAX_SCORES];

  short int display_size_counter;
  short int screen_width, screen_height;
  short int window_sizes[10];

  float Accel_x, Accel_y;

  static bool intro_already_shown = 0;

  bool redraw;
  bool fullscreen;
  bool flash_enable;
  bool menu_enable;
  bool menu_buttons_enable;
  bool scoreboard_enable;
  bool scoreboard_done;
  bool scoreboard_exit;

  /******************************************/

  /**************DEFINITIONS*****************/

  ship_t intro_ship = {0, 0, 0, 0, {0}, NULL};

  choice_t select = INTRO;

  redraw = 0;
  fullscreen = al_get_display_flags(display) & ALLEGRO_FULLSCREEN_WINDOW;
  flash_enable = 1;
  menu_enable = 0;
  menu_buttons_enable = 0;
  scoreboard_enable = 0;
  scoreboard_exit = 1;

  Accel_x = 0;
  Accel_y = 0;

  ALLEGRO_COLOR color_white = al_map_rgb(255, 255, 255);
  ALLEGRO_COLOR color_black = al_map_rgb(0, 0, 0);
  ALLEGRO_COLOR color_grey = al_map_rgb(100, 100, 100);

  screen_width = al_get_display_width(display);
  screen_height = al_get_display_height(display);

  /******************************************/

  /*************INITIALIZERS*****************/

  sample_instance = al_create_sample_instance(NULL);
  ERRCHECK(sample_instance, "Sample Instance");
  sample_instance2 = al_create_sample_instance(NULL);
  ERRCHECK(sample_instance2, "Sample Instance 2");

  ERRCHECK(setup_buttons(&button_start, &button_play, &button_settings, &button_scoreboard, &button_credits, &button_exit, &button_left_arrow_enabled, &button_left_arrow_disabled, &button_right_arrow_enabled, &button_right_arrow_disabled, &button_fullscreen, &button_apply), "Button intializers");

  setup_buttons_static_positions(&button_start, &button_play, &button_settings, &button_scoreboard, &button_credits, &button_exit, &button_left_arrow_enabled, &button_left_arrow_disabled, &button_right_arrow_enabled, &button_right_arrow_disabled, &button_fullscreen, &button_apply);

  ERRCHECK(setup_bitmaps(&intro_logo, &intro_ship.sprite, &play_background, &settings_background, &scoreboard_background, &credits_background, &exit_background), "Bitmaps initializers");

  al_attach_sample_instance_to_mixer(sample_instance, al_get_default_mixer());
  al_attach_sample_instance_to_mixer(sample_instance2, al_get_default_mixer());

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

  setup_background(intro_background_frames);

  ERRCHECK(setup_audio_samples(&logo_sound, &intro_part1, &intro_part2, &menu_enter, &menu_back, &menu_locked, &menu_modify), "Audio intializer");

  ERRCHECK(setup_fonts(&font_toreks, &font_toreks_big, &font_supercharge, &font_cartesian), "Fonts intializer");

  intro_ship.cx = al_get_bitmap_width(intro_ship.sprite) / 2;
  intro_ship.cy = al_get_bitmap_height(intro_ship.sprite) / 2;

  load_scores(top_scores);

  al_set_sample(sample_instance2, intro_part2);

  al_hide_mouse_cursor(display);

  display_size_counter = detect_screen_sizes(window_sizes);

  /******************************************/

  /***************FIRST PHASE****************/

  if ( !intro_already_shown ) intro_anim(default_font, logo_sound, intro_part1, buffer, display, sample_instance, intro_ship.sprite);
  intro_already_shown = 1;

  al_play_sample_instance(sample_instance2);

  /******************************************/

  /***************PRE-LOOP*******************/

  al_start_timer(timer);

  while ( al_get_next_event(queue, &dummy) );

  /******************************************/

  /***************MAIN LOOP******************/

  while ( select != QUIT && select != PLAY ) {
    al_wait_for_event(queue, &menu_event);
    switch ( select ) {

      case INTRO:

        /******************************************/

        /**********INTRO & START MENU**************/

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

          case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:

            al_get_mouse_state(&mouse);
            if ( MOUSE_HOVER(button_start) && mouse.buttons & LEFT_CLICK ) {
              select = MENU_MAIN;
              flash_enable = 0;
              menu_enable = 1;
              al_show_mouse_cursor(display);
              PLAY_SOUND(menu_enter);
              MAIN_MENU_BUTTONS(RESET);
            }
            break;
        }
        break;

        /******************************************/

      case MENU_MAIN:

        /**************MAIN MENU*******************/

        switch ( menu_event.type ) {

          case ALLEGRO_EVENT_TIMER:
            BUFFER;
            CLEAR;

            al_get_mouse_state(&mouse);
            redraw = true;

            DRAW_BACKGROUND;
            DRAW_LOGO;

            if ( menu_enable ) {
              draw_dark_cover(1);
              if ( menu_slide_window(SHOW) ) {
                MAIN_MENU_BUTTONS(SHOW);
                menu_buttons_enable = 1;
              }
            } else {
              if ( menu_buttons_enable ) {
                draw_dark_cover(1);
                menu_slide_window(SHOW);
              } else {
                draw_dark_cover(0);
                if ( menu_slide_window(HIDE) ) {
                  select = INTRO;
                  al_hide_mouse_cursor(display);
                  CURSOR_UPDATE;
                }
              }
              if ( MAIN_MENU_BUTTONS(HIDE) ) {
                menu_buttons_enable = 0;
              }
            }

            break;

          case ALLEGRO_EVENT_KEY_DOWN:
            if ( menu_event.keyboard.keycode == ALLEGRO_KEY_ESCAPE && menu_enable && menu_buttons_enable ) {
              PLAY_SOUND(menu_back);
              menu_enable = 0;
            }
            break;

          case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            al_get_mouse_state(&mouse);
            if ( mouse.buttons & LEFT_CLICK && menu_enable ) {
              if ( MOUSE_HOVER(button_play) ) {
                select = PLAY;
              } else if ( MOUSE_HOVER(button_settings) ) {
                PLAY_SOUND(menu_enter);
                select = MENU_SETTINGS;
                MAIN_MENU_BUTTONS(RESET);
              } else if ( MOUSE_HOVER(button_scoreboard) ) {
                PLAY_SOUND(menu_enter);
                select = MENU_SCORE;
                MENU_SCOREBOARD(1);
                scoreboard_enable = 1;
                MAIN_MENU_BUTTONS(RESET);
              } else if ( MOUSE_HOVER(button_credits) ) {
                PLAY_SOUND(menu_enter);
                select = MENU_CREDITS;
                MENU_CREDITS(1);
                MAIN_MENU_BUTTONS(RESET);
              } else if ( MOUSE_HOVER(button_exit) )
                select = QUIT;
            }
        }

        break;

        /******************************************/

      case MENU_SCORE:

        /***************SCOREBOARD*****************/

        switch ( menu_event.type ) {
          case ALLEGRO_EVENT_KEY_DOWN:
            if ( menu_event.keyboard.keycode == ALLEGRO_KEY_ESCAPE && scoreboard_done ) {
              if ( scoreboard_exit ) {
                PLAY_SOUND(menu_back);
                scoreboard_enable = 0;
                scoreboard_exit = 0;
              }
            }
            break;
          case ALLEGRO_EVENT_TIMER:
            BUFFER;
            CLEAR;

            DRAW_BACKGROUND;
            DRAW_LOGO;
            draw_dark_cover(1);

            if ( scoreboard_enable && (scoreboard_done = menu_slide_window(EXTEND)) ) {
              MENU_SCOREBOARD(0);
            }

            else if ( !scoreboard_enable && (scoreboard_done = menu_slide_window(SHRINK)) ) {
              select = MENU_MAIN;
              scoreboard_exit = 1;
            }

            BACK_TEXT;

            redraw = 1;
            break;
        }
        break;

        /******************************************/

      case MENU_SETTINGS:

        /***************SETTINGS*******************/

        switch ( menu_event.type ) {
          case ALLEGRO_EVENT_KEY_DOWN:
            if ( menu_event.keyboard.keycode == ALLEGRO_KEY_ESCAPE ) {
              PLAY_SOUND(menu_back);
              select = MENU_MAIN;
              MENU_SETTINGS(1);
            }
            break;

          case ALLEGRO_EVENT_TIMER:
            BUFFER;
            CLEAR;

            al_get_mouse_state(&mouse);

            DRAW_BACKGROUND;
            DRAW_LOGO;
            draw_dark_cover(1);
            menu_slide_window(SHOW);

            MENU_SETTINGS(0);
            redraw = true;

            break;

          case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            al_get_mouse_state(&mouse);
            if ( MOUSE_HOVER(button_fullscreen) && mouse.buttons & LEFT_CLICK ) {
              PLAY_SOUND(menu_modify);
              fullscreen = !fullscreen;
              if ( fullscreen )
                display_size_counter = 0;
              else
                display_size_counter = 1;
            } else if ( MOUSE_HOVER(button_right_arrow_enabled) && !fullscreen && mouse.buttons & LEFT_CLICK ) {
              PLAY_SOUND(menu_modify);
              display_size_counter++;
              if ( display_size_counter > 4 ) display_size_counter = 1;
            } else if ( MOUSE_HOVER(button_right_arrow_disabled) && mouse.buttons & LEFT_CLICK ) {
              PLAY_SOUND(menu_locked);
            } else if ( MOUSE_HOVER(button_left_arrow_enabled) && !fullscreen && mouse.buttons & LEFT_CLICK ) {
              PLAY_SOUND(menu_modify);
              display_size_counter--;
              if ( display_size_counter < 1 ) display_size_counter = 4;
            } else if ( MOUSE_HOVER(button_left_arrow_disabled) && mouse.buttons & LEFT_CLICK ) {
              PLAY_SOUND(menu_locked);
            } else if ( MOUSE_HOVER(button_apply) && mouse.buttons & LEFT_CLICK ) {
              if ( fullscreen == ((al_get_display_flags(display) & ALLEGRO_FULLSCREEN_WINDOW) ? 1 : 0) && screen_width == window_sizes[2 * display_size_counter] ) {
                PLAY_SOUND(menu_locked);
              } else {
                PLAY_SOUND(menu_enter);
                al_set_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, fullscreen);
                if ( !fullscreen ) {
                  al_resize_display(display, window_sizes[2 * display_size_counter], window_sizes[2 * display_size_counter + 1]);
                }
                al_acknowledge_resize(display);
                screen_width = al_get_display_width(display);
                screen_height = al_get_display_height(display);
              }
            }
            break;

          default:
            break;
        }
        break;

        /******************************************/

      case MENU_CREDITS:

        /***************CREDITS********************/

        switch ( menu_event.type ) {
          case ALLEGRO_EVENT_TIMER:
            BUFFER;
            CLEAR;
            DRAW_BACKGROUND;
            DRAW_LOGO;
            draw_dark_cover(1);
            menu_slide_window(SHOW);

            MENU_CREDITS(0);

            BACK_TEXT;
            redraw = 1;
            break;
          case ALLEGRO_EVENT_KEY_DOWN:
            if ( menu_event.keyboard.keycode == ALLEGRO_KEY_ESCAPE ) {
              PLAY_SOUND(menu_back);
              select = MENU_MAIN;
            }
            break;
        }

        break;

        /******************************************/

      default:
        break;
    }

    REDRAW;
  }

  /******************************************/

  /********RESOURCE-FREEING & RESETS*********/

  kill_background(intro_background_frames, kill_all_bitmaps);
  (*kill_all_bitmaps)(7, intro_ship.sprite, intro_logo, play_background, settings_background, scoreboard_background, exit_background, credits_background);
  (*kill_all_samples)(7, logo_sound, intro_part1, intro_part2, menu_enter, menu_back, menu_locked, menu_modify);
  (*kill_all_instances)(2, sample_instance, sample_instance2);
  (*kill_all_font)(4, font_cartesian, font_supercharge, font_toreks, font_toreks_big);
  kill_all_button(12, &button_start, &button_play, &button_settings, &button_scoreboard, &button_credits, &button_exit, &button_left_arrow_enabled, &button_left_arrow_disabled, &button_right_arrow_enabled, &button_right_arrow_disabled, &button_fullscreen, &button_apply);

  al_stop_timer(timer);

  flash_enable = 0;
  draw_flash(&flash_enable);

  MAIN_MENU_BUTTONS(RESET);
  MENU_SETTINGS(1);
  menu_slide_window(RESET);
  while ( draw_dark_cover(0) );
  CLEAR;

  /******************************************/

  /*****************RETURN*******************/

  al_hide_mouse_cursor(display);

  if ( select == QUIT )
    return 0;
  else
    return 1;

  /******************************************/
}

/**********************************************************************************************/

/****************************************CREATE_BUTTON*****************************************/

button_t create_button(ALLEGRO_COLOR color_1, ALLEGRO_COLOR color_2, gradient_mode_t gradient, ALLEGRO_COLOR color_hover_1, ALLEGRO_COLOR color_hover_2, gradient_mode_t gradient_hover, ALLEGRO_COLOR color_trace_1, ALLEGRO_COLOR color_trace_2, float line_width, unsigned short int vertex_count, ...) {

  /******************SETUP*******************/

  button_t button = {0};
  float *vertex = calloc(2 * vertex_count, sizeof(float));
  if ( !vertex ) return button;

  va_list vertex_list;
  va_start(vertex_list, vertex_count);
  unsigned short int counter_1;
  float min_x, max_x, min_y, max_y;

  /******************************************/

  /******************VERTEX******************/

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

  /******************************************/

  /*****************BITMAPS*******************/

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

  /******************************************/

  /****************BACKGROUND****************/

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

  /******************************************/

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

  /******************************************/

  /******************BORDER******************/

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

  al_set_target_bitmap(button.sprite[0]);
  al_draw_polygon(vertex, vertex_count, ALLEGRO_LINE_JOIN_ROUND, color_trace_1, line_width, 1.0);
  al_set_target_bitmap(button.sprite[1]);
  al_draw_polygon(vertex, vertex_count, ALLEGRO_LINE_JOIN_ROUND, color_trace_2, line_width, 1.0);

  /******************************************/

  /***************FINISHING******************/

  al_destroy_bitmap(background);
  al_destroy_bitmap(mask);
  al_set_blender(flag_1, flag_2, flag_3);
  free(vertex);

  if ( original_buffer ) al_set_target_bitmap(original_buffer);

  button.valid = 1;
  return button;

  /******************************************/
}

/**********************************************************************************************/

/*************************************MOUSE_HOVER_BUTTON***************************************/

bool mouse_hover_button(button_t *button, ALLEGRO_MOUSE_STATE *mouse, float size_x, float size_y) {
  int position_x = mouse->x * (WORLD_WIDTH / size_x);
  int position_y = mouse->y * (WORLD_HEIGHT / size_y);
  return (position_x >= (button->position_x - button->size_x / 2) && position_x <= (button->position_x + button->size_x / 2) && position_y >= (button->position_y - button->size_y / 2) && position_y <= (button->position_y + button->size_y / 2));
}

/**********************************************************************************************/

/**************************************KILL_ALL_BUTTON*****************************************/

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
  va_end(list);
}

/**********************************************************************************************/

/****************************************DRAW_BUTTON*******************************************/

void draw_button(ALLEGRO_MOUSE_STATE *mouse, float screen_width, float screen_height, button_t *button) {
  if ( mouse_hover_button(button, mouse, screen_width, screen_height) )
    al_draw_bitmap(button->sprite[1], button->position_x - (button->size_x * 0.5), button->position_y - (button->size_y * 0.5), 0);
  else
    al_draw_bitmap(button->sprite[0], button->position_x - (button->size_x * 0.5), button->position_y - (button->size_y * 0.5), 0);
}

/**********************************************************************************************/

/**************************************DRAW_SMART_TEXT*****************************************/

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

/**********************************************************************************************/

/**********************************DRAW_RECTANGLE_GRADIENT*************************************/

void draw_rectangle_gradient(float center_x, float center_y, float radius_x, float radius_y, gradient_mode_t mode, ALLEGRO_COLOR color_1, ALLEGRO_COLOR color_2, int steps) {

  /**************ERROR-CHECK*****************/

  if ( steps <= 0 || (mode != GRADIENT_CENTER && mode != GRADIENT_LEFT_TO_RIGHT && mode != GRADIENT_TOP_TO_BOTTOM) ) return;

  /******************************************/

  /******************SETUP*******************/

  ALLEGRO_COLOR gradient;
  int counter_1;
  float gradient_x_step = radius_x / steps, gradient_y_step = radius_y / steps;
  float r_step = (color_1.r - color_2.r) / steps;
  float g_step = (color_1.g - color_2.g) / steps;
  float b_step = (color_1.b - color_2.b) / steps;
  float a_step = (color_1.a - color_2.a) / steps;

  /******************************************/

  /*************GRADIENT DRAW****************/

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

  /******************************************/
}

/**********************************************************************************************/
/**********************************************************************************************/

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*****************************************MUST_SETUP*******************************************/

static bool must_setup(bool task, char *msg) {
  if ( !task ) {
    fprintf(stderr, "%s%s\n", MSJ_ERR_INIT, msg);
    return 0;
  }
  return 1;
}

/**********************************************************************************************/

/*************************************SETUP_BUTTONS********************************************/

static bool setup_buttons(button_t *button_start, button_t *button_play, button_t *button_settings, button_t *button_scoreboard, button_t *button_credits, button_t *button_exit, button_t *button_left_arrow_enabled, button_t *button_left_arrow_disabled, button_t *button_right_arrow_enabled, button_t *button_right_arrow_disabled, button_t *button_fullscreen, button_t *button_apply) {
  ALLEGRO_COLOR color_white = al_map_rgb(255, 255, 255);
  ALLEGRO_COLOR color_black = al_map_rgb(0, 0, 0);
  ALLEGRO_COLOR color_grey = al_map_rgb(100, 100, 100);

  ALLEGRO_COLOR color_button_left = al_map_rgba(50, 50, 50, 180);
  ALLEGRO_COLOR color_button_right = al_map_rgba(45, 0, 160, 240);
  ALLEGRO_COLOR color_button_hover_left = al_map_rgba(255, 255, 255, 180);
  ALLEGRO_COLOR color_button_hover_right = al_map_rgba(70, 0, 255, 240);

  ALLEGRO_COLOR color_arrow_left = al_map_rgba(135, 115, 0, 180);
  ALLEGRO_COLOR color_arrow_right = al_map_rgba(135, 0, 0, 240);
  ALLEGRO_COLOR color_arrow_hover_left = al_map_rgba(255, 217, 0, 220);
  ALLEGRO_COLOR color_arrow_hover_right = al_map_rgba(255, 0, 0, 255);

  ALLEGRO_COLOR color_arrow_disabled = al_map_rgba(50, 50, 50, 150);
  ALLEGRO_COLOR color_arrow_disabled_hover = al_map_rgba(150, 150, 150, 150);

  float button_base_vertex[8] = {16.0, 0.0, 0.0, 45.0, 350.0, 45.0, 350.0, 0.0};
  float left_arrow_base_vertex[6] = {0.0, 15.0, 26.0, 30.0, 26.0, 0.0};
  float right_arrow_base_vertex[6] = {0.0, 0.0, 0.0, 30.0, 26.0, 15.0};

  float arrow_line_thickness = 3.0;

  *button_start = create_button(al_map_rgb(255, 125, 0), al_map_rgb(200, 0, 220), GRADIENT_CENTER, al_map_rgb(255, 0, 0), al_map_rgb(0, 0, 0), GRADIENT_CENTER, color_black, color_white, 4.0, 4, 50., 0., 0., 100., 200., 100., 250., 0.);
  ERRCHECK(button_start->valid, "Start Button");

  *button_play = create_button(color_button_left, color_button_right, GRADIENT_LEFT_TO_RIGHT, color_button_hover_left, color_button_hover_right, GRADIENT_LEFT_TO_RIGHT, color_black, color_black, 5.0, 4, button_base_vertex[0], button_base_vertex[1], button_base_vertex[2], button_base_vertex[3], button_base_vertex[4], button_base_vertex[5], button_base_vertex[6], button_base_vertex[7]);
  ERRCHECK(button_play->valid, "Play Button");

  *button_settings = create_button(color_button_left, color_button_right, GRADIENT_LEFT_TO_RIGHT, color_button_hover_left, color_button_hover_right, GRADIENT_LEFT_TO_RIGHT, color_black, color_black, 5.0, 4, button_base_vertex[0], button_base_vertex[1], button_base_vertex[2], button_base_vertex[3], button_base_vertex[4] + 20., button_base_vertex[5], button_base_vertex[6] + 20., button_base_vertex[7]);
  ERRCHECK(button_settings->valid, "Settings Button");

  *button_scoreboard = create_button(color_button_left, color_button_right, GRADIENT_LEFT_TO_RIGHT, color_button_hover_left, color_button_hover_right, GRADIENT_LEFT_TO_RIGHT, color_black, color_black, 5.0, 4, button_base_vertex[0], button_base_vertex[1], button_base_vertex[2], button_base_vertex[3], button_base_vertex[4] + 40., button_base_vertex[5], button_base_vertex[6] + 40., button_base_vertex[7]);
  ERRCHECK(button_scoreboard->valid, "Scoreboard Button");

  *button_exit = create_button(color_button_left, color_button_right, GRADIENT_LEFT_TO_RIGHT, color_button_hover_left, color_button_hover_right, GRADIENT_LEFT_TO_RIGHT, color_black, color_black, 5.0, 4, 0.0, 0.0, 0.0, button_base_vertex[3], button_base_vertex[4] / 2 + 20.0, button_base_vertex[5], button_base_vertex[6] / 2 + 20.0, button_base_vertex[7]);
  ERRCHECK(button_exit->valid, "Exit Button");

  *button_credits = create_button(color_button_right, color_button_left, GRADIENT_LEFT_TO_RIGHT, color_button_hover_right, color_button_hover_left, GRADIENT_LEFT_TO_RIGHT, color_black, color_black, 5.0, 4, button_base_vertex[0], button_base_vertex[1], button_base_vertex[2], button_base_vertex[3], button_base_vertex[4] / 2 + 20.0, button_base_vertex[5], button_base_vertex[6] / 2 + 20.0, button_base_vertex[7]);
  ERRCHECK(button_credits->valid, "Credits Button");

  *button_left_arrow_enabled = create_button(color_arrow_left, color_arrow_right, GRADIENT_LEFT_TO_RIGHT, color_arrow_hover_left, color_arrow_hover_right, GRADIENT_LEFT_TO_RIGHT, color_black, color_white, arrow_line_thickness, 3, left_arrow_base_vertex[0], left_arrow_base_vertex[1], left_arrow_base_vertex[2], left_arrow_base_vertex[3], left_arrow_base_vertex[4], left_arrow_base_vertex[5]);
  ERRCHECK(button_left_arrow_enabled->valid, "Left enabled arrow Button");

  *button_left_arrow_disabled = create_button(color_arrow_disabled, color_arrow_disabled, GRADIENT_LEFT_TO_RIGHT, color_arrow_disabled_hover, color_arrow_disabled_hover, GRADIENT_LEFT_TO_RIGHT, color_grey, color_grey, arrow_line_thickness, 3, left_arrow_base_vertex[0], left_arrow_base_vertex[1], left_arrow_base_vertex[2], left_arrow_base_vertex[3], left_arrow_base_vertex[4], left_arrow_base_vertex[5]);
  ERRCHECK(button_left_arrow_disabled->valid, "Left disabled arrow Button");

  *button_right_arrow_enabled = create_button(color_arrow_right, color_arrow_left, GRADIENT_LEFT_TO_RIGHT, color_arrow_hover_right, color_arrow_hover_left, GRADIENT_LEFT_TO_RIGHT, color_black, color_white, arrow_line_thickness, 3, right_arrow_base_vertex[0], right_arrow_base_vertex[1], right_arrow_base_vertex[2], right_arrow_base_vertex[3], right_arrow_base_vertex[4], right_arrow_base_vertex[5]);
  ERRCHECK(button_right_arrow_enabled->valid, "Right enabled arrow Button");

  *button_right_arrow_disabled = create_button(color_arrow_disabled, color_arrow_disabled, GRADIENT_LEFT_TO_RIGHT, color_arrow_disabled_hover, color_arrow_disabled_hover, GRADIENT_LEFT_TO_RIGHT, color_grey, color_grey, arrow_line_thickness, 3, right_arrow_base_vertex[0], right_arrow_base_vertex[1], right_arrow_base_vertex[2], right_arrow_base_vertex[3], right_arrow_base_vertex[4], right_arrow_base_vertex[5]);
  ERRCHECK(button_right_arrow_disabled->valid, "Right disabled arrow Button");

  *button_fullscreen = create_button(al_map_rgba(255, 125, 0, 240), al_map_rgba(200, 0, 220, 180), GRADIENT_CENTER, al_map_rgba(255, 0, 0, 240), al_map_rgba(0, 0, 0, 180), GRADIENT_CENTER, color_black, color_white, 4.0, 6, 0.0, 0.0, 10.0, 25.0, 0.0, 50.0, 210.0, 50.0, 200.0, 25.0, 210.0, 0.0);
  ERRCHECK(button_fullscreen->valid, "Fullscreen Button");

  *button_apply = create_button(al_map_rgba(255, 125, 0, 240), al_map_rgba(200, 0, 220, 180), GRADIENT_CENTER, al_map_rgba(255, 0, 0, 240), al_map_rgba(0, 0, 0, 180), GRADIENT_CENTER, color_black, color_white, 4.0, 5, 0.0, 0.0, 0.0, 30.0, 110.0, 30.0, 100.0, 15.0, 110.0, 0.0);
  ERRCHECK(button_apply->valid, "Apply Button");

  return 1;
}

/**********************************************************************************************/

/*************************************SETUP_BITMAPS********************************************/

static bool setup_bitmaps(ALLEGRO_BITMAP **intro_logo, ALLEGRO_BITMAP **intro_ship_sprite, ALLEGRO_BITMAP **play_background, ALLEGRO_BITMAP **settings_background, ALLEGRO_BITMAP **scoreboard_background, ALLEGRO_BITMAP **credits_background, ALLEGRO_BITMAP **exit_background) {
  *intro_logo = al_load_bitmap(BITMAP_ROUTE("intro/intro_logo.png"));
  ERRCHECK(*intro_logo, "intro logo");
  *intro_ship_sprite = al_load_bitmap(BITMAP_ROUTE("intro/intro_ship.png"));
  ERRCHECK(*intro_ship_sprite, "intro ship");
  *play_background = al_load_bitmap(BITMAP_ROUTE("menu/play_background.png"));
  ERRCHECK(*play_background, "play background");
  *settings_background = al_load_bitmap(BITMAP_ROUTE("menu/settings_background.png"));
  ERRCHECK(*settings_background, "setttings background");
  *scoreboard_background = al_load_bitmap(BITMAP_ROUTE("menu/scoreboard_background.png"));
  ERRCHECK(*scoreboard_background, "scoreboard background");
  *credits_background = al_load_bitmap(BITMAP_ROUTE("menu/placeholder_background.png"));
  ERRCHECK(*credits_background, "credits background");
  *exit_background = al_load_bitmap(BITMAP_ROUTE("menu/exit_background.png"));
  ERRCHECK(*exit_background, "exit background");
  return 1;
}

/**********************************************************************************************/

/**********************************SETUP_AUDIO_SAMPLES*****************************************/

static bool setup_audio_samples(ALLEGRO_SAMPLE **logo_sound, ALLEGRO_SAMPLE **intro_part1, ALLEGRO_SAMPLE **intro_part2, ALLEGRO_SAMPLE **menu_enter, ALLEGRO_SAMPLE **menu_back, ALLEGRO_SAMPLE **menu_locked, ALLEGRO_SAMPLE **menu_modify) {
  *logo_sound = al_load_sample(AUDIO_ROUTE("coin.wav"));
  ERRCHECK(*logo_sound, "Coin Sound");
  *intro_part1 = al_load_sample(AUDIO_ROUTE("intro_part1.wav"));
  ERRCHECK(*intro_part1, "intro 1st part sound");
  *intro_part2 = al_load_sample(AUDIO_ROUTE("intro_part2.wav"));
  ERRCHECK(*intro_part1, "intro 2nd part sound");

  *menu_enter = al_load_sample(AUDIO_ROUTE("menu_enter.wav"));
  ERRCHECK(*menu_enter, "menu enter sound effect");
  *menu_back = al_load_sample(AUDIO_ROUTE("menu_back.wav"));
  ERRCHECK(*menu_back, "menu back sound effect");
  *menu_locked = al_load_sample(AUDIO_ROUTE("menu_locked.wav"));
  ERRCHECK(*menu_locked, "menu locked sound effect");
  *menu_modify = al_load_sample(AUDIO_ROUTE("menu_modify.wav"));
  ERRCHECK(*menu_modify, "menu modify sound effect");
  return 1;
}

/**********************************************************************************************/

/**************************************SETUP_FONTS*********************************************/

static bool setup_fonts(ALLEGRO_FONT **font_toreks, ALLEGRO_FONT **font_toreks_big, ALLEGRO_FONT **font_supercharge, ALLEGRO_FONT **font_cartesian) {
  *font_toreks = al_load_ttf_font(FONT_ROUTE("toreks-font/Toreks_regular.ttf"), 16, 0);
  ERRCHECK(*font_toreks, "Toreks Font");
  *font_toreks_big = al_load_ttf_font(FONT_ROUTE("toreks-font/Toreks_regular.ttf"), 24, 0);
  ERRCHECK(*font_toreks_big, "Big Toreks Font");
  *font_supercharge = al_load_ttf_font(FONT_ROUTE("supercharge-font/Supercharge_halftone.otf"), 24, 0);
  ERRCHECK(*font_supercharge, "Supercharge Font");
  *font_cartesian = al_load_ttf_font(FONT_ROUTE("cartesian-font/Cartesian_regular.ttf"), 12, 0);
  ERRCHECK(*font_cartesian, "Cartesian Font");
  return 1;
}

/**********************************************************************************************/

/*************************************SETUP_BACKGROUND******************************************/

static bool setup_background(ALLEGRO_BITMAP **intro_background_frames) {
  short int background_frames;
  char intro_background_path[64];
  for ( background_frames = 1; background_frames <= 300; background_frames++ ) {
    sprintf(intro_background_path, "../assets/Bitmap/intro_background/frames/frame_%03d.png", background_frames);
    intro_background_frames[background_frames - 1] = al_load_bitmap(intro_background_path);
    ERRCHECK(intro_background_frames[background_frames - 1], intro_background_path);
  }
  return 1;
}

/**********************************************************************************************/

/*****************************SETUP_BUTTONS_STATIC_POSITIONS***********************************/

static void setup_buttons_static_positions(button_t *button_start, button_t *button_play, button_t *button_settings, button_t *button_scoreboard, button_t *button_credits, button_t *button_exit, button_t *button_left_arrow_enabled, button_t *button_left_arrow_disabled, button_t *button_right_arrow_enabled, button_t *button_right_arrow_disabled, button_t *button_fullscreen, button_t *button_apply) {
  button_start->position_x = WORLD_WIDTH / 2;
  button_start->position_y = WORLD_HEIGHT * (3. / 5);

  button_play->position_y = WORLD_HEIGHT * 0.15;
  button_settings->position_y = WORLD_HEIGHT * 0.25;
  button_scoreboard->position_y = WORLD_HEIGHT * 0.35;
  button_exit->position_y = WORLD_HEIGHT * 0.45;
  button_credits->position_y = WORLD_HEIGHT * 0.45;

  button_left_arrow_enabled->position_y = WORLD_HEIGHT * 0.25;
  button_left_arrow_disabled->position_y = button_left_arrow_enabled->position_y;
  button_right_arrow_enabled->position_y = button_left_arrow_enabled->position_y;
  button_right_arrow_disabled->position_y = button_left_arrow_enabled->position_y;

  button_fullscreen->position_y = WORLD_HEIGHT * 0.4;

  button_apply->position_y = WORLD_HEIGHT * 0.6;
}

/**********************************************************************************************/

/**********************************DETECT_SCREEN_SIZES*****************************************/

static short int detect_screen_sizes(short int *window_sizes) {

  /*****************SETUP********************/

  short int display_size_counter;
  bool default_size_registered;
  float window_factor;

  ALLEGRO_DISPLAY_MODE display_mode;
  al_get_display_mode(0, &display_mode);

  /******************************************/

  /************WINDOW SIZE MATH**************/

  for ( display_size_counter = 0, default_size_registered = 0, window_factor = 1.0; display_size_counter < 5; display_size_counter++ ) {
    if ( display_mode.width * window_factor > WORLD_WIDTH || default_size_registered ) {
      window_sizes[2 * display_size_counter] = display_mode.width * window_factor;
      window_sizes[2 * display_size_counter + 1] = display_mode.height * window_factor;

      window_factor -= 1. / 6;

      if ( window_sizes[2 * display_size_counter] % 2 ) window_sizes[2 * display_size_counter]++;
      if ( window_sizes[2 * display_size_counter + 1] % 2 ) window_sizes[2 * display_size_counter + 1]++;

      if ( window_sizes[2 * display_size_counter] == WORLD_WIDTH && window_sizes[2 * display_size_counter + 1] == WORLD_HEIGHT ) {
        window_sizes[2 * display_size_counter] = display_mode.width * window_factor;
        window_sizes[2 * display_size_counter + 1] = display_mode.height * window_factor;

        if ( window_sizes[2 * display_size_counter] % 2 ) window_sizes[2 * display_size_counter]++;
        if ( window_sizes[2 * display_size_counter + 1] % 2 ) window_sizes[2 * display_size_counter + 1]++;
      }

    } else {
      window_sizes[2 * display_size_counter] = WORLD_WIDTH;
      window_sizes[2 * display_size_counter + 1] = WORLD_HEIGHT;
      default_size_registered = 1;
    }
  }

  display_size_counter = 0;
  return display_size_counter;

  /******************************************/
}

/**********************************************************************************************/

/**********************************************************************************************/

/*****************************************INTRO_ANIM*******************************************/

static void intro_anim(ALLEGRO_FONT *default_font, ALLEGRO_SAMPLE *s_logo, ALLEGRO_SAMPLE *sample_intro_1, ALLEGRO_BITMAP *buffer, ALLEGRO_DISPLAY *display, ALLEGRO_SAMPLE_INSTANCE *sample_instance, ALLEGRO_BITMAP *ship) {

  /*****************SETUP********************/

  short int a, b, x, y;
  short int width = al_get_bitmap_width(ship);
  short int height = al_get_bitmap_height(ship);
  float s;

  float trail[20] = {
      WORLD_WIDTH / 2 - 4, WORLD_HEIGHT, WORLD_WIDTH / 2 - 8, WORLD_HEIGHT, WORLD_WIDTH / 2 - 16, WORLD_HEIGHT, WORLD_WIDTH / 2 - 20, WORLD_HEIGHT, WORLD_WIDTH / 2 - 48, WORLD_HEIGHT, WORLD_WIDTH / 2 + 48, WORLD_HEIGHT, WORLD_WIDTH / 2 + 20, WORLD_HEIGHT, WORLD_WIDTH / 2 + 16, WORLD_HEIGHT, WORLD_WIDTH / 2 + 8, WORLD_HEIGHT, WORLD_WIDTH / 2 + 4, WORLD_HEIGHT};

  al_set_sample(sample_instance, sample_intro_1);

  /******************************************/

  /***************FIRST_MSG******************/

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

  /******************************************/

  /*********FIRST_FLIGHTS_ANIMATION**********/

  al_rest(1.0);
  al_play_sample_instance(sample_instance);

  /******LEFT TO RIGHT, BOTTOM TO TOP*******/

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

  /******RIGHT TO LEFT, BOTTOM TO TOP*******/

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

  /********BOTTOM CENTER TO CENTER**********/

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

  /******************************************/
}

/**********************************************************************************************/

/****************************************CURSOR_ACCEL******************************************/

static void cursor_accel(ALLEGRO_MOUSE_STATE *mouse, float *Accel_x, float *Accel_y) {

  static int old_x = WORLD_WIDTH / 2;
  static int old_y = WORLD_HEIGHT / 2;

  short int Delta_x = mouse->x - old_x;
  short int Delta_y = mouse->y - old_y;

  *Accel_x = Delta_x / 100.0;

  *Accel_y = -Delta_y / 200.0;

  old_x = mouse->x;
  old_y = mouse->y;
}

/**********************************************************************************************/

/*****************************************TRAIL_ACCEL******************************************/

static void trail_accel(float *polygon, ALLEGRO_MOUSE_STATE *mouse, int width, int height) {

  static int old_x = WORLD_WIDTH / 2;
  static int old_y = WORLD_HEIGHT / 2;

  short int counter_1, counter_2 = 0;

  int delta_x = old_x - mouse->x;
  int delta_y = old_y - mouse->y;

  float factor_x = (float) WORLD_WIDTH / width;
  float factor_y = (float) WORLD_HEIGHT / height;

  if ( delta_x < 10 && delta_x > -10 ) {
    if ( delta_y < 0 )
      counter_2 = -2;
    else
      counter_2 = 2;
  }

  for ( counter_1 = 1; counter_1 <= 5; counter_1++ ) {
    polygon[11 - 2 * counter_1] = mouse->y * factor_y + delta_y * 2 / counter_1;
    polygon[9 + 2 * counter_1] = mouse->y * factor_y + delta_y * 2 / counter_1;

    polygon[2 * counter_1 - 2] = (mouse->x + delta_x / counter_1) * factor_x - counter_2 * counter_1;
    polygon[20 - 2 * counter_1] = (mouse->x + delta_x / counter_1) * factor_x + counter_2 * counter_1;
  }

  old_x = mouse->x;
  old_y = mouse->y;
}

/**********************************************************************************************/

/*************************************DRAW_BACKGROUND******************************************/

static void draw_background(ALLEGRO_BITMAP **intro_background_frames) {
  static short int background_frames_loop = 0;
  al_draw_bitmap(intro_background_frames[background_frames_loop++], 0, 0, 0);
  if ( background_frames_loop >= 300 ) background_frames_loop = 0;
}

/**********************************************************************************************/

/****************************************DRAW_FLASH********************************************/

static void draw_flash(bool *flash_enable) {
  static short int flash_counter = 255;
  if ( flash_counter && *flash_enable ) {
    al_draw_filled_rectangle(0, 0, WORLD_WIDTH, WORLD_HEIGHT, al_map_rgba(255, 255, 255, flash_counter));
    if ( flash_counter > 10 )
      flash_counter -= 10;
    else {
      flash_counter = 0;
      *flash_enable = 0;
    }
  } else {
    flash_counter = 255;
  }
}

/**********************************************************************************************/

/*****************************************DRAW_FRAME*******************************************/

static void draw_frame(ALLEGRO_DISPLAY *display, ALLEGRO_BITMAP *buffer) {
  al_set_target_backbuffer(display);
  al_draw_scaled_bitmap(buffer, 0, 0, WORLD_WIDTH, WORLD_HEIGHT, 0, 0, al_get_display_width(display), al_get_display_height(display), 0);
  al_flip_display();
}

/**********************************************************************************************/

/*************************************DRAW_DARK_COVER******************************************/

static bool draw_dark_cover(bool enable) {
  static short int cover_counter = 0;
  static bool reset = 0;
  static bool old_mode = 0;
  al_draw_filled_rectangle(0, 0, WORLD_WIDTH, WORLD_HEIGHT, al_map_rgba(0, 0, 0, cover_counter));

  if ( old_mode != enable ) {
    reset = 0;
    old_mode = enable;
  }

  if ( cover_counter < 150 && enable ) {
    if ( !reset ) {
      cover_counter = 0;
      reset = 1;
    }
    cover_counter += 15;
  }

  else if ( cover_counter > 0 && !(enable) ) {
    if ( !reset ) {
      cover_counter = 150;
      reset = 1;
    }
    cover_counter -= 10;
  }

  if ( (cover_counter >= 150 && enable) || (cover_counter <= 0 && !enable) ) {
    return 1;
  }

  else
    return 0;
}

/**********************************************************************************************/

/**********************************************************************************************/

/************************************MENU_SLIDE_WINDOW*****************************************/

static bool menu_slide_window(window_t mode) {

  /*******************SETUP******************/

  static float main_menu_window[] = {WORLD_WIDTH, 0, WORLD_WIDTH, WORLD_HEIGHT, WORLD_WIDTH, WORLD_HEIGHT, WORLD_WIDTH, 0};
  static float menu_slide_counter;

  static window_t old_mode = -1;

  if ( old_mode != mode ) {
    if ( old_mode != SHRINK ) menu_slide_counter = 15;
    old_mode = mode;
  }

  /******************************************/

  /***************SWITCH CASE****************/

  switch ( mode ) {
    case SHOW:
      if ( menu_slide_counter ) {
        main_menu_window[0] = WORLD_WIDTH * (1 - (1. / 2) / (menu_slide_counter));
        main_menu_window[2] = WORLD_WIDTH * (1 - (7. / 10) / (menu_slide_counter--));
      }
      break;
    case HIDE:
      if ( menu_slide_counter ) {
        main_menu_window[0] = WORLD_WIDTH * (1. / 2 + (1. / 2) / (menu_slide_counter));
        main_menu_window[2] = WORLD_WIDTH * (3. / 10 + (7. / 10) / (menu_slide_counter--));
      }
      break;
    case EXTEND:
      if ( menu_slide_counter ) {
        main_menu_window[0] = WORLD_WIDTH * ((1. / 2) * (1 - 1. / (menu_slide_counter)));
        main_menu_window[2] = WORLD_WIDTH * (3. / 10) * (1 - 1. / (menu_slide_counter--));
      }
      break;
    case SHRINK:
      if ( menu_slide_counter ) {
        main_menu_window[0] = WORLD_WIDTH * ((1. / 2) / (menu_slide_counter));
        main_menu_window[2] = WORLD_WIDTH * ((3. / 10) / (menu_slide_counter--));
      }
      break;
    default:
      main_menu_window[0] = 0;
      main_menu_window[2] = 0;
      return 1;
      break;
  }

  /******************************************/

  /****************FINAL DRAW****************/

  al_draw_filled_polygon(main_menu_window, 4, al_map_rgba(255, 255, 255, 180));

  if ( !menu_slide_counter && mode == old_mode ) {
    return 1;
  }
  return 0;

  /******************************************/
}

/**********************************************************************************************/

/************************************MAIN_MENU_BUTTONS*****************************************/

static bool main_menu_buttons(button_t *button_play, button_t *button_settings, button_t *button_scoreboard, button_t *button_exit, button_t *button_credits, ALLEGRO_MOUSE_STATE *mouse, short int screen_width, short int screen_height, ALLEGRO_FONT *font_supercharge, ALLEGRO_FONT *font_cartesian, ALLEGRO_COLOR *color_black, ALLEGRO_COLOR *color_white, window_t mode, ALLEGRO_BITMAP *play_background, ALLEGRO_BITMAP *settings_background, ALLEGRO_BITMAP *scoreboard_background, ALLEGRO_BITMAP *credits_background, ALLEGRO_BITMAP *exit_background) {

  /******************SETUP*******************/

  static float button_slide_1 = 1.0;

  static bool menu_slide_done = 0;
  static bool reset = 0;

  static window_t old_mode = -1;

  static short int background_slide[5] = {-WORLD_WIDTH / 2};

  /******************************************/

  /******************RESET*******************/

  if ( mode == RESET ) {
    reset = 0;
    menu_slide_done = 0;
    old_mode = mode;
    return 1;
  }

  /******************************************/

  /******************UPDATE******************/

  else if ( old_mode != mode ) {
    reset = 0;
    menu_slide_done = 0;
    old_mode = mode;
  }

  /******************************************/

  /*************SLIDE ANIMATION**************/

  if ( !menu_slide_done ) {

    if ( mode == SHOW ) {
      if ( !reset ) {
        button_slide_1 = 1;
        reset = 1;
      }
      if ( button_slide_1 < 100 )
        button_slide_1 += 5;
      else if ( button_slide_1 < 200 )
        button_slide_1 += 20;
      else {
        menu_slide_done = 1;
      }

    } else {
      if ( !reset ) {
        button_slide_1 = 201;
        reset = 1;
      }
      if ( button_slide_1 > 101 )
        button_slide_1 -= 20;
      else if ( button_slide_1 > 1 )
        button_slide_1 -= 5;
      else {
        menu_slide_done = 1;
      }
    }

    button_play->position_x = (WORLD_WIDTH * 0.75) * (1 + 1 / button_slide_1);
    button_settings->position_x = (WORLD_WIDTH * 0.74) * (1 + 1 / button_slide_1);
    button_scoreboard->position_x = (WORLD_WIDTH * 0.73) * (1 + 1 / button_slide_1);
    button_exit->position_x = (WORLD_WIDTH * 0.83) * (1 + 1 / button_slide_1);
    button_credits->position_x = (WORLD_WIDTH * 0.61) * (1 + 1 / button_slide_1);
  }

  /******************************************/

  /*************BUTTON AND TEXT**************/

  DRAW_BUTTON_FOR_FUNCTIONS(button_play);
  DRAW_BUTTON_FOR_FUNCTIONS(button_settings);
  DRAW_BUTTON_FOR_FUNCTIONS(button_scoreboard);
  DRAW_BUTTON_FOR_FUNCTIONS(button_exit);
  DRAW_BUTTON_FOR_FUNCTIONS(button_credits);

  BUTTON_TEXT_FOR_FUNCTIONS(font_supercharge, button_play, "Take off!");
  BUTTON_TEXT_FOR_FUNCTIONS(font_supercharge, button_settings, "Settings");
  BUTTON_TEXT_FOR_FUNCTIONS(font_supercharge, button_scoreboard, "Scoreboard");
  BUTTON_TEXT_FOR_FUNCTIONS(font_supercharge, button_exit, "Quit game");
  BUTTON_TEXT_FOR_FUNCTIONS(font_supercharge, button_credits, "Credits");

  /******************************************/

  /*******DESCRIPTION AND BACKGROUNDS********/

  smart_description(button_play, button_settings, button_scoreboard, button_credits, button_exit, mouse, font_cartesian, *color_black, &screen_width, &screen_height);

  if ( MOUSE_HOVER_FOR_FUNCTION(button_play) ) {
    background_slider_reset_except(0, background_slide);
    al_draw_bitmap(play_background, background_slide[0], 0, 0);
    if ( background_slide[0] < 0 ) background_slide[0] += WORLD_WIDTH / 10;
  } else if ( MOUSE_HOVER_FOR_FUNCTION(button_settings) ) {
    background_slider_reset_except(1, background_slide);
    al_draw_bitmap(settings_background, background_slide[1], 0, 0);
    if ( background_slide[1] < 0 ) background_slide[1] += WORLD_WIDTH / 10;
  } else if ( MOUSE_HOVER_FOR_FUNCTION(button_scoreboard) ) {
    background_slider_reset_except(2, background_slide);
    al_draw_bitmap(scoreboard_background, background_slide[2], 0, 0);
    if ( background_slide[2] < 0 ) background_slide[2] += WORLD_WIDTH / 10;
  } else if ( MOUSE_HOVER_FOR_FUNCTION(button_credits) ) {
    background_slider_reset_except(3, background_slide);
    al_draw_bitmap(credits_background, background_slide[3], 0, 0);
    if ( background_slide[3] < 0 ) background_slide[3] += WORLD_WIDTH / 10;
  } else if ( MOUSE_HOVER_FOR_FUNCTION(button_exit) ) {
    background_slider_reset_except(4, background_slide);
    al_draw_bitmap(exit_background, background_slide[4], 0, 0);
    if ( background_slide[4] < 0 ) background_slide[4] += WORLD_WIDTH / 10;
  } else {
    background_slider_reset_except(5, background_slide);
  }

  BACK_TEXT_FOR_FUNCTIONS(font_cartesian, color_black);

  /******************************************/

  /*************RETURN PROGRESS**************/

  return menu_slide_done;

  /******************************************/
}

/**********************************************************************************************/

/**************************************MENU_SETTINGS*******************************************/

static void menu_settings(button_t *button_left_arrow_enabled, button_t *button_right_arrow_enabled, button_t *button_left_arrow_disabled, button_t *button_right_arrow_disabled, button_t *button_fullscreen, button_t *button_apply, ALLEGRO_MOUSE_STATE *mouse, short int screen_width, short int screen_height, ALLEGRO_FONT *font_supercharge, ALLEGRO_FONT *font_toreks, ALLEGRO_FONT *font_toreks_big, ALLEGRO_FONT *font_cartesian, ALLEGRO_COLOR *color_black, ALLEGRO_COLOR *color_grey, ALLEGRO_COLOR *color_white, bool *fullscreen, bool reset, short int *window_sizes, short int display_size_counter) {

  /******************SETUP*******************/

  static float settings_slide = 1.0;

  /******************************************/

  /******************RESET*******************/

  if ( reset ) {
    settings_slide = 1.0;
    return;
  }

  /******************************************/

  /**************TEXT DISPLAY****************/

  al_draw_text(font_supercharge, *color_black, WORLD_WIDTH * 0.55 * (1 + 1 / settings_slide), WORLD_HEIGHT * 0.1, 0, "settings");
  al_draw_line(WORLD_WIDTH * 0.55 * (1 + 1 / settings_slide), WORLD_HEIGHT * 0.15, WORLD_WIDTH * 0.9 * (1 + 1 / settings_slide), WORLD_HEIGHT * 0.15, *color_black, 3.0);

  al_draw_text(font_toreks_big, *color_black, WORLD_WIDTH * 0.5 * (1 + 1 / settings_slide), button_left_arrow_enabled->position_y - 14, 0, "window size");
  al_draw_text(font_toreks_big, *color_black, WORLD_WIDTH * 0.5 * (1 + 1 / settings_slide), button_fullscreen->position_y - 14, 0, "window mode");

  /******************************************/

  /*************SLIDE ANIMATION**************/

  if ( settings_slide < 100 ) {
    button_left_arrow_enabled->position_x = WORLD_WIDTH * 0.7 * (1 + 1 / settings_slide);
    button_left_arrow_disabled->position_x = button_left_arrow_enabled->position_x;
    button_right_arrow_enabled->position_x = WORLD_WIDTH * 0.95 * (1 + 1 / settings_slide);
    button_right_arrow_disabled->position_x = button_right_arrow_enabled->position_x;

    button_fullscreen->position_x = WORLD_WIDTH * 0.825 * (1 + 1 / settings_slide);

    button_apply->position_x = WORLD_WIDTH * 0.875 * (1 + 1 / settings_slide);

    if ( settings_slide < 50 )
      settings_slide += 5;
    else
      settings_slide += 10;
  }

  /******************************************/

  /************BUTTONS AND TEXT**************/

  al_draw_filled_rectangle(button_left_arrow_enabled->position_x + 25, button_left_arrow_enabled->position_y - 20, button_right_arrow_enabled->position_x - 25, button_right_arrow_enabled->position_y + 20, al_map_rgba(50, 50, 50, 100));

  DRAW_BUTTON_FOR_FUNCTIONS(button_fullscreen);

  if ( !(*fullscreen) ) {
    DRAW_BUTTON_FOR_FUNCTIONS(button_left_arrow_enabled);
    DRAW_BUTTON_FOR_FUNCTIONS(button_right_arrow_enabled);
    al_draw_textf(font_toreks_big, *color_black, button_right_arrow_enabled->position_x - 120, button_left_arrow_enabled->position_y - 12, ALLEGRO_ALIGN_CENTER, "%dx%d", window_sizes[2 * display_size_counter], window_sizes[2 * display_size_counter + 1]);

    draw_smart_text(mouse, screen_width, screen_height, button_fullscreen, font_toreks_big, *color_black, *color_white, ALLEGRO_ALIGN_CENTER, "Windowed");
  } else {
    DRAW_BUTTON_FOR_FUNCTIONS(button_left_arrow_disabled);
    DRAW_BUTTON_FOR_FUNCTIONS(button_right_arrow_disabled);
    al_draw_textf(font_toreks_big, *color_grey, button_right_arrow_enabled->position_x - 120, button_left_arrow_enabled->position_y - 12, ALLEGRO_ALIGN_CENTER, "%dx%d", window_sizes[2 * display_size_counter], window_sizes[2 * display_size_counter + 1]);

    draw_smart_text(mouse, screen_width, screen_height, button_fullscreen, font_toreks_big, *color_black, *color_white, ALLEGRO_ALIGN_CENTER, "Fullscreen");
  }

  DRAW_BUTTON_FOR_FUNCTIONS(button_apply);
  draw_smart_text(mouse, screen_width, screen_height, button_apply, font_toreks, *color_black, *color_white, ALLEGRO_ALIGN_CENTER, "Apply");

  BACK_TEXT_FOR_FUNCTIONS(font_cartesian, color_black);

  /******************************************/
}

/**********************************************************************************************/

/*************************************MENU_SCOREBOARD******************************************/

static void menu_scoreboard(ALLEGRO_FONT *font_supercharge, ALLEGRO_FONT *font_toreks, ALLEGRO_FONT *font_toreks_big, highscore_t *top_scores, bool reset) {

  /******************SETUP*******************/

  static float score_slide = 1.0;

  if ( reset ) {
    score_slide = 1.0;
    return;
  }

  short int counter_1;
  float position_y, position_x, color_1, color_2;

  /******************************************/

  /***************TEXT SLIDE*****************/

  position_x = (1 + 1 / score_slide);
  al_draw_text(font_supercharge, al_map_rgb(0, 0, 0), (WORLD_WIDTH * 0.15) * position_x, WORLD_HEIGHT * 0.1, 0, "Leaderboard");
  al_draw_line(WORLD_WIDTH * 0.1 * position_x, WORLD_HEIGHT * 0.15, WORLD_WIDTH * 0.9 * position_x, WORLD_HEIGHT * 0.15, al_map_rgb(0, 0, 0), 5.0);

  /**************COLOR CHANGE****************/

  for ( counter_1 = 0; counter_1 < 10; counter_1++ ) {
    position_y = WORLD_HEIGHT * (0.2 + counter_1 * 0.05);
    color_1 = 1.0 - 0.1 * counter_1;
    color_2 = 0.1 * counter_1;

    al_draw_textf(font_toreks, al_map_rgb_f(color_1, color_2, 0), (WORLD_WIDTH * 0.15) * position_x, position_y, 0, "%d", counter_1 + 1);
    al_draw_text(font_toreks_big, al_map_rgb_f(color_1, color_2, 0), (WORLD_WIDTH * 0.2) * position_x, position_y - 7, 0, top_scores[counter_1].name);
    al_draw_textf(font_toreks, al_map_rgb_f(color_1, color_2, 0), (WORLD_WIDTH * 0.3) * position_x, position_y, 0, "%d", top_scores[counter_1].score);
  }

  for ( counter_1 = 0; counter_1 < 10; counter_1++ ) {
    position_y = WORLD_HEIGHT * (0.2 + counter_1 * 0.05);
    color_1 = 1.0 - 0.1 * counter_1;
    color_2 = 0.1 * counter_1;

    al_draw_textf(font_toreks, al_map_rgb_f(0, color_1, color_2), (WORLD_WIDTH * 0.55) * position_x, position_y, 0, "%d", counter_1 + 11);
    al_draw_text(font_toreks_big, al_map_rgb_f(0, color_1, color_2), (WORLD_WIDTH * 0.60) * position_x, position_y - 7, 0, top_scores[counter_1 + 10].name);
    al_draw_textf(font_toreks, al_map_rgb_f(0, color_1, color_2), (WORLD_WIDTH * 0.7) * position_x, position_y, 0, "%d", top_scores[counter_1 + 10].score);
  }

  /******************************************/

  if ( score_slide < 50 )
    score_slide += 5;
  else if ( score_slide < 100 )
    score_slide += 10;

  /******************************************/
}

/**********************************************************************************************/

/**************************************MENU_CREDITS********************************************/

static void menu_credits(ALLEGRO_FONT *font_supercharge, ALLEGRO_FONT *font_toreks, ALLEGRO_FONT *font_toreks_big, ALLEGRO_COLOR *color_black, bool reset) {

  /*******************SETUP******************/

  static float credits_slide = 1.0;

  /******************************************/

  /******************RESET*******************/

  if ( reset ) {
    credits_slide = 1.0;
    return;
  }

  /******************************************/

  /*******************TEXT*******************/

  al_draw_text(font_supercharge, *color_black, WORLD_WIDTH * 0.55 * (1 + 1 / credits_slide), WORLD_HEIGHT * 0.1, 0, "CrediTs");
  al_draw_line(WORLD_WIDTH * 0.55 * (1 + 1 / credits_slide), WORLD_HEIGHT * 0.15, WORLD_WIDTH * 0.9 * (1 + 1 / credits_slide), WORLD_HEIGHT * 0.15, *color_black, 3.0);

  al_draw_text(font_toreks, *color_black, WORLD_WIDTH * 0.5 * (1 + 1 / credits_slide), WORLD_HEIGHT * 0.2, 0, "Developers");
  al_draw_line(WORLD_WIDTH * 0.5 * (1 + 1 / credits_slide), WORLD_HEIGHT * 0.23, WORLD_WIDTH * 0.65 * (1 + 1 / credits_slide), WORLD_HEIGHT * 0.23, *color_black, 3.0);

  al_draw_multiline_textf(font_toreks_big, *color_black, WORLD_WIDTH * 0.42 * (1 + 1 / credits_slide), WORLD_HEIGHT * 0.3, 1000, 48, 0, "            Itai Cravchik\n"
                                                                                                                                        "        Rocco Diaz Parisi\n"
                                                                                                                                        "    Francisco Paredes Alonso\n"
                                                                                                                                        "Enzo Nicolas Rosa Fernandez");

  if ( credits_slide < 50 )
    credits_slide += 5;
  else if ( credits_slide < 100 )
    credits_slide += 10;

  /******************************************/
}

/**********************************************************************************************/

/*************************************INSERT_DESCRIPTION***************************************/

static void insert_description(ALLEGRO_FONT *font, ALLEGRO_COLOR color, const char *text) {
  al_draw_multiline_text(font, color, 0.4 * WORLD_WIDTH, 0.7 * WORLD_HEIGHT, 0.55 * WORLD_WIDTH, al_get_font_line_height(font), 0, text);
}

/**********************************************************************************************/

/*************************************SMART_DESCRIPTION****************************************/

static void smart_description(button_t *play, button_t *settings, button_t *scoreboard, button_t *credits, button_t *exit, ALLEGRO_MOUSE_STATE *mouse, ALLEGRO_FONT *font, ALLEGRO_COLOR color, short int *screen_width, short int *screen_height) {
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

/**********************************************************************************************/

/******************************BACKGROUND_SLIDER_RESET_EXCEPT**********************************/

static void background_slider_reset_except(short int n, short int *slider) {
  short int counter_1;
  for ( counter_1 = 0; counter_1 < 5; counter_1++ ) {
    if ( counter_1 != n ) slider[counter_1] = -WORLD_WIDTH / 2;
  }
}

/**********************************************************************************************/

/*************************************KILL_BACKGROUND******************************************/

static void kill_background(ALLEGRO_BITMAP **intro_background_frames, void (*kill_all_bitmaps)(int counter, ...)) {
  short int background_frames;
  for ( background_frames = 0; background_frames < 300; background_frames++ )
    (*kill_all_bitmaps)(1, intro_background_frames[background_frames]);
}

/**********************************************************************************************/

/**********************************************************************************************/
