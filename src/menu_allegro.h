/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     menu_allegro.h

     Description:
     Handles game main menu. Provides menu_allegro function to be called when going into main menu.
     Also provides functions to work with buttons: creation, killing, and adding text.


  ******************************************************************************/

#ifndef HEADER
#define HEADER

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "general_defines.h"
#include <allegro5/allegro5.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_video.h>
#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/



/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
  ALLEGRO_BITMAP *sprite[2];  // 2 Sprites, 1 default sprite, 1 on_hover sprite
  float size_y, size_x;       // Height and width of the sprite
  int position_y, position_x; // Position of the CENTER of the bitmap
  char valid;                 // flag to identify whether it was correctly initialized or not. Use init_error(button.valid, "buton")
} button_t;

typedef enum gradient_mode { GRADIENT_CENTER = 20,
                             GRADIENT_TOP_TO_BOTTOM,
                             GRADIENT_LEFT_TO_RIGHT } gradient_mode_t;
// gradient_mode: used for draw_rectangle_gradient

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

bool menu_allegro(ALLEGRO_DISPLAY *display, ALLEGRO_TIMER *timer, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT *default_font, ALLEGRO_BITMAP *buffer, ALLEGRO_MIXER *mixer, void (*kill_all_bitmaps)(int, ...), void (*kill_all_instances)(int, ...), void (*kill_all_samples)(int, ...), void (*kill_all_font)(int, ...));

/*
create_button: creates a button to be used freely. Receives:
- color_1, color_2 and gradient for the color of the default button
- color_hover_1, color_hover_2 and gradient_hover for the color of the on_hover button
- color_trace_1, color_trace_2 and line_width for the border lines for default and on_hover states
- vertex_count: self explanatory...1
- Variadic arguments refer to the x and y positions (double not int) of each vertex from the polygon used to create the button shape

Returns a button_t object
*/
button_t create_button(ALLEGRO_COLOR color_1, ALLEGRO_COLOR color_2, gradient_mode_t gradient, ALLEGRO_COLOR color_hover_1, ALLEGRO_COLOR color_hover_2, gradient_mode_t gradient_hover, ALLEGRO_COLOR color_trace_1, ALLEGRO_COLOR color_trace_2, float line_width, unsigned short int vertex_count, ...);

/*
mouse_hover_button: Detects whether the mouse is hovering over the button. Receives:
- button pointer
- mouse pointer
- size_x: screen width (NOT NECESSARILY WORLD_WIDTH)
- size_y: screen height (NOT NECESSARILY WORLD_HEIGHT)
*/
bool mouse_hover_button(button_t *button, ALLEGRO_MOUSE_STATE *mouse, float size_x, float size_y);

/*
kill_all_button: Destroys the bitmaps of each button. Receives:
- count
- Variadic arguments refer to the pointers for each button. (e.g. &button_start)
*/
void kill_all_button(int count, ...);

/*
draw_button: Draws the button on the selected buffer target. Receives:
- callback function to detect the mouse over the button
- mouse pointer
- screen_width
- screen_height
- button pointer
*/
void draw_button( ALLEGRO_MOUSE_STATE *mouse, float screen_width, float screen_height, button_t *button);

/*
draw_smart_text: Inserts static text on top of the button. Receives:
- callback function to detect the mouse over the button
- mouse pointer
- screen_width
- screen_height
- button pointer
- font pointer
- colors for default and on_hover states
- alignemt: ALLEGRO_ALIGN_CENTER, ALLEGRO_ALIGN_LEFT, ALLEGRO_ALIGN_RIGHT
- text
For non_static text, you can set a char buffer and use sprintf to insert variables
*/
void draw_smart_text( ALLEGRO_MOUSE_STATE *mouse, float screen_width, float screen_height, button_t *button, ALLEGRO_FONT *font, ALLEGRO_COLOR color_default, ALLEGRO_COLOR color_hover, char alignment, const char *text);

/*
draw_rectangle_gradient: Draws a rectangle with gradient colors. Receives:
- center_x: x position of the center of the rectangle
- center_y: y position of the center of the rectangle
- radius_x: distance between center and left or right border
- radius_y: distance between center and upper or lower border
- color_1 and color_2: colors bounds for the gradient
- steps: Amount of shapes to draw. Higher for better quality, but slower
*/
void draw_rectangle_gradient(float center_x, float center_y, float radius_x, float radius_y, gradient_mode_t mode, ALLEGRO_COLOR color_1, ALLEGRO_COLOR color_2, int steps);

/*******************************************************************************
 ******************************************************************************/

#endif // HEADER