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

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

enum choice {INTRO = 10, QUIT};

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void kill_all(ALLEGRO_BITMAP** Intro_frames);
static game_state_t must_setup(bool task, char* msg);
static void Intro_anim(ALLEGRO_FONT* dfont, ALLEGRO_SAMPLE* SLogo, ALLEGRO_SAMPLE* SI1, ALLEGRO_SAMPLE* SI2, ALLEGRO_BITMAP* buffer, ALLEGRO_DISPLAY* display);

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

game_state_t menu_allegro(ALLEGRO_DISPLAY* display, ALLEGRO_TIMER* timer, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_FONT* dfont, ALLEGRO_BITMAP* buffer){
  ALLEGRO_BITMAP* Intro_background_frames[300];
  ALLEGRO_EVENT menu_event;
  ALLEGRO_EVENT dummy;
  ALLEGRO_SAMPLE* Logo_sound;
  ALLEGRO_SAMPLE* Intro_part1;
  ALLEGRO_SAMPLE* Intro_part2;

  char select = INTRO;
  bool redraw = false;
  bool fullscreen = false;
  char Intro_background_path[64];
  int i;

  //if (intro_setup(Intro_background_frames, &Logo_sound, &Intro_part1, &Intro_part2) == CLOSED) return CLOSED;

  for (i = 1; i <= 300 && select != QUIT; i++) {
      sprintf(Intro_background_path, "../assets/Bitmap/Intro_background/frames/frame_%03d.png", i);
      Intro_background_frames[i-1] = al_load_bitmap(Intro_background_path);
      if (!Intro_background_frames[i-1]) {
          fprintf(stderr, "Error loading frame %d: %s\n", i-1, Intro_background_path);
          select = QUIT;
      }
    }
  i = 0;

  
  Logo_sound = al_load_sample(AUDIO_ROUTE(coin.wav));
  if (must_setup(Logo_sound, "Coin sound") == CLOSED) return CLOSED;
  Intro_part1 = al_load_sample(AUDIO_ROUTE(Intro_part1.wav));
  if (must_setup(Logo_sound, "Intro 1st part sound") == CLOSED) return CLOSED;
  Intro_part2 = al_load_sample(AUDIO_ROUTE(Intro_part2.wav));
  if (must_setup(Logo_sound, "Intro 2nd part sound") == CLOSED) return CLOSED;

  Intro_anim(dfont, Logo_sound, Intro_part1, Intro_part2, buffer, display);

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
      break;

    }

    if (redraw){
      al_set_target_backbuffer(display);
      al_draw_scaled_bitmap(buffer,0,0,WORLD_WIDTH,WORLD_HEIGHT,0,0,al_get_display_width(display),al_get_display_height(display),0);
      al_flip_display();
      redraw = false;
    }

}
  kill_all(Intro_background_frames);
  al_stop_timer(timer);
  return CLOSED;


}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

game_state_t must_setup(bool task, char* msg){
  if (!task){
    fprintf(stderr, "%s%s\n", MSJ_ERR_INIT, msg);
    return CLOSED;
  }
  return 1;
}


void kill_all(ALLEGRO_BITMAP** Intro_frames){
  for (int i = 1; i <= 300; i++){
    al_destroy_bitmap(Intro_frames[i-1]);
  }
 }

 void Intro_anim(ALLEGRO_FONT* dfont, ALLEGRO_SAMPLE* SLogo, ALLEGRO_SAMPLE* SI1, ALLEGRO_SAMPLE* SI2, ALLEGRO_BITMAP* buffer, ALLEGRO_DISPLAY* display){
  al_set_target_bitmap(buffer);
  al_draw_textf(dfont,al_map_rgb(255,255,255), WORLD_WIDTH/2, WORLD_HEIGHT/2, ALLEGRO_ALIGN_CENTER,"Lionel Messi Studios presents");
  
 }