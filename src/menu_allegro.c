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
  ALLEGRO_VIDEO* video_player1;
  ALLEGRO_MIXER* mixer_player1;
  ALLEGRO_BITMAP* video_frame1;
  ALLEGRO_EVENT menu_event;
  char select = INTRO;
  bool redraw = false;

  mixer_player1= al_get_default_mixer();

  video_player1 = al_open_video("../assets/Intro_backgroundhigh.ogv");

  if (video_player1 == NULL){
    printf ("Error");
    select = QUIT;
  }
  else{
    al_register_event_source(queue, al_get_video_event_source(video_player1));
    al_start_video(video_player1,mixer_player1);
  }

  while (select != QUIT){
    al_wait_for_event (queue,&menu_event);

    switch (menu_event.type){
      case ALLEGRO_EVENT_TIMER:
      if (!al_is_video_playing(video_player1)){
        al_seek_video(video_player1,0.0);
        al_set_video_playing(video_player1,true);
      }
      break;
      case ALLEGRO_EVENT_VIDEO_FRAME_SHOW:
      redraw = true;
      al_set_target_bitmap(buffer);
      video_frame1 = al_get_video_frame(video_player1);
      al_draw_bitmap(video_frame1,0,0,0);
      break;
      case ALLEGRO_EVENT_DISPLAY_CLOSE:
      al_close_video(video_player1);
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

}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
