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
#include "general_defines.h"


/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

enum choice {INTRO = 0, QUIT};

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void kill_all(ALLEGRO_BITMAP** Intro_frames);

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

void menu_allegro(ALLEGRO_DISPLAY* display, ALLEGRO_TIMER* timer, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_FONT* dfont, ALLEGRO_BITMAP* buffer){
  ALLEGRO_BITMAP* Intro_background_frames[300];
  ALLEGRO_EVENT menu_event;
  ALLEGRO_EVENT dummy;
  char select = INTRO;
  bool redraw = false;
  bool fullscreen = false;
  char Intro_background_path[64];
  int i;
  for (i = 1; i <= 300 && select != QUIT; i++) {
      sprintf(Intro_background_path, "../assets/Intro_background/frames/frame_%03d.png", i);
      Intro_background_frames[i-1] = al_load_bitmap(Intro_background_path);
      if (!Intro_background_frames[i-1]) {
          fprintf(stderr, "Error loading frame %d: %s\n", i-1, Intro_background_path);
          select = QUIT;
      }
  }
  i = 0;

  al_start_timer(timer);
  while (al_get_next_event(queue,&dummy));

  while (select != QUIT){
    al_wait_for_event (queue,&menu_event);

    switch (menu_event.type){
      case ALLEGRO_EVENT_TIMER:
      redraw = true;
      al_set_target_bitmap(buffer);
      al_draw_bitmap(Intro_background_frames[i++],0,0,0);
      if (i == 300) i = 0;
      break;
      case ALLEGRO_EVENT_KEY_DOWN:
      switch (menu_event.keyboard.keycode){
        case ALLEGRO_KEY_F:
        fullscreen = !fullscreen;
        al_toggle_display_flag(display,ALLEGRO_FULLSCREEN_WINDOW,fullscreen);
        break;
        case ALLEGRO_KEY_ESCAPE:
        select = QUIT;
        break;
      }
      break;
      case ALLEGRO_EVENT_DISPLAY_CLOSE:
      select = QUIT;
      kill_all(Intro_background_frames);
      break;

    }

    

    if (redraw){
      al_set_target_backbuffer(display);
      al_draw_scaled_bitmap(buffer,0,0,WORLD_WIDTH,WORLD_HEIGHT,0,0,al_get_display_width(display),al_get_display_height(display),0);
      al_flip_display();
      redraw = false;
    }

}

al_stop_timer(timer);

}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/


 void kill_all(ALLEGRO_BITMAP** Intro_frames){
  for (int i = 1; i <= 300; i++){
    al_destroy_bitmap(Intro_frames[i-1]);
  }
 }