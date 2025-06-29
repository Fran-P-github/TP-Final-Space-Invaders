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
#include <stdint.h>

#include <stdarg.h>


/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define MSJ_ERR_INIT "Error with setup: "

#define DRAW Draw_frame(Display,Buffer)
#define BUFFER al_set_target_bitmap(Buffer)
#define CLEAR al_clear_to_color(al_map_rgb(0,0,0))

#define ERRCHECK(obj,txt) if (must_setup(obj, txt) == CLOSED) return CLOSED

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

enum choice {INTRO = 10, QUIT};

typedef struct {
  int16_t px;   //X position
  int16_t py;   //Y position
  int8_t cx;    //X center
  int8_t cy;    //Y center
  double r;     //Tilt
  ALLEGRO_BITMAP* sprite;
} Ship_t;

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void kill_all(ALLEGRO_BITMAP** Intro_frames,...);
static game_state_t must_setup(bool task, char* msg);
static void Intro_anim(ALLEGRO_FONT* dfont, ALLEGRO_SAMPLE* SLogo, ALLEGRO_SAMPLE* SI1, ALLEGRO_BITMAP* buffer, ALLEGRO_DISPLAY* display, ALLEGRO_SAMPLE_INSTANCE* SInstance, ALLEGRO_BITMAP * Ship);
static void Draw_frame(ALLEGRO_DISPLAY* Display, ALLEGRO_BITMAP* Buffer);

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

game_state_t menu_allegro(ALLEGRO_DISPLAY* Display, ALLEGRO_TIMER* Timer, ALLEGRO_EVENT_QUEUE* Queue, ALLEGRO_FONT* Dfont, ALLEGRO_BITMAP* Buffer, ALLEGRO_MIXER* Mixer){
  ALLEGRO_BITMAP* Intro_background_frames[300];
  //ALLEGRO_BITMAP* Intro_ship;
  ALLEGRO_BITMAP* Intro_logo;
  ALLEGRO_EVENT Menu_event;
  ALLEGRO_EVENT Dummy;
  ALLEGRO_SAMPLE* Logo_sound;
  ALLEGRO_SAMPLE* Intro_part1;
  ALLEGRO_SAMPLE* Intro_part2;
  ALLEGRO_SAMPLE_INSTANCE* Sample_instance;
  ALLEGRO_SAMPLE_INSTANCE* Sample_instance2;

  Ship_t Intro_ship;

  Sample_instance = al_create_sample_instance(NULL);
  if (must_setup(Sample_instance, "Sample Instance") == CLOSED) return CLOSED;
  Sample_instance2 = al_create_sample_instance(NULL);
  if (must_setup(Sample_instance2, "Sample Instance") == CLOSED) return CLOSED;

  al_attach_sample_instance_to_mixer(Sample_instance,al_get_default_mixer());
  al_attach_sample_instance_to_mixer(Sample_instance2,al_get_default_mixer());

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

  char select = INTRO;
  bool redraw = false;
  bool fullscreen = false;
  char intro_background_path[64];
  short int i,j;

  for (i = 1; i <= 300 && select != QUIT; i++) {
      sprintf(intro_background_path, "../assets/Bitmap/Intro_background/frames/frame_%03d.png", i);
      Intro_background_frames[i-1] = al_load_bitmap(intro_background_path);
      if (!Intro_background_frames[i-1]) {
          fprintf(stderr, "Error loading frame %d: %s\n", i-1, intro_background_path);
          select = QUIT;
      }
    }
  
  Intro_logo = al_load_bitmap(BITMAP_ROUTE(Intro/Intro_logo.png));
  ERRCHECK(Intro_logo, "Intro logo");
  Intro_ship.sprite = al_load_bitmap(BITMAP_ROUTE(Intro/Intro_ship.png));
  ERRCHECK(Intro_ship.sprite, "Intro Ship");
  Logo_sound = al_load_sample(AUDIO_ROUTE(coin.wav));
  ERRCHECK(Logo_sound, "Coin Sound");
  Intro_part1 = al_load_sample(AUDIO_ROUTE(Intro_part1.wav));
  ERRCHECK(Intro_part1, "Intro 1st part sound");
  Intro_part2 = al_load_sample(AUDIO_ROUTE(Intro_part2.wav));
  ERRCHECK(Intro_part1, "Intro 2nd part sound");

  Intro_ship.cx = al_get_bitmap_width(Intro_ship.sprite)/2;
  Intro_ship.cy = al_get_bitmap_height(Intro_ship.sprite)/2;
  Intro_ship.r = 0.0;

  al_set_sample(Sample_instance2, Intro_part2);

  Intro_anim(Dfont, Logo_sound, Intro_part1, Buffer, Display, Sample_instance, Intro_ship.sprite);

  al_play_sample_instance(Sample_instance2);

  

  i = 0;
  j=255;

  al_start_timer(Timer);
  while (al_get_next_event(Queue,&Dummy));

  while (select != QUIT){
    al_wait_for_event (Queue,&Menu_event);

    switch (Menu_event.type){
      case ALLEGRO_EVENT_TIMER:
      redraw = true;
      BUFFER;
      CLEAR;
      al_draw_bitmap(Intro_background_frames[i++],0,0,0);
      al_draw_line(WORLD_WIDTH/2, WORLD_HEIGHT, WORLD_WIDTH/2, 3*WORLD_HEIGHT/4, al_map_rgb(255,255,255),20.0);
      al_draw_scaled_rotated_bitmap(Intro_ship.sprite,Intro_ship.cx,Intro_ship.cy,WORLD_WIDTH/2,2*WORLD_HEIGHT/4,0.75,0.45,Intro_ship.r,0);
      al_draw_scaled_rotated_bitmap(Intro_logo,al_get_bitmap_width(Intro_logo)/2, al_get_bitmap_height(Intro_logo)/2, WORLD_WIDTH/2, WORLD_HEIGHT/4,1,1,0,0);
      al_draw_textf(Dfont,al_map_rgb(255,255,255),WORLD_WIDTH/2,WORLD_HEIGHT/2,ALLEGRO_ALIGN_CENTER,"*Press any key to continue*");
      al_draw_textf(Dfont,al_map_rgb(255,255,255),WORLD_WIDTH/2,WORLD_HEIGHT/2+12,ALLEGRO_ALIGN_CENTER,"(No hace nada aun xd)");

      if (j > 0) j -= 10;
      if (j < 0) j = 0;
      if (i == 300) i = 0;
      break;
      case ALLEGRO_EVENT_KEY_DOWN:
      switch (Menu_event.keyboard.keycode){
        case ALLEGRO_KEY_F:
        fullscreen = !fullscreen;
        al_toggle_display_flag(Display,ALLEGRO_FULLSCREEN_WINDOW,fullscreen);
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
      al_draw_filled_rectangle(0,0,WORLD_WIDTH,WORLD_HEIGHT,al_map_rgba(255,255,255,j));
      DRAW;
      redraw = false;
    }

}
  kill_all(Intro_background_frames, Logo_sound, Intro_part1, Intro_part2, Sample_instance, Sample_instance2, Intro_ship.sprite, Intro_logo);
  al_stop_timer(Timer);
  return GAME;


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


void kill_all(ALLEGRO_BITMAP** Intro_frames,...){
  va_list Param_list;
  va_start(Param_list, Intro_frames);
  

  for (int i = 1; i <= 300; i++){
    al_destroy_bitmap(Intro_frames[i-1]);
  }

  al_destroy_sample(va_arg(Param_list,ALLEGRO_SAMPLE*));
  al_destroy_sample(va_arg(Param_list,ALLEGRO_SAMPLE*));
  al_destroy_sample(va_arg(Param_list,ALLEGRO_SAMPLE*));
  al_destroy_sample_instance(va_arg(Param_list,ALLEGRO_SAMPLE_INSTANCE*));
  al_destroy_sample_instance(va_arg(Param_list,ALLEGRO_SAMPLE_INSTANCE*));
  al_destroy_bitmap(va_arg(Param_list,ALLEGRO_BITMAP*));
  al_destroy_bitmap(va_arg(Param_list,ALLEGRO_BITMAP*));

  

  va_end (Param_list);
 }

 void Intro_anim(ALLEGRO_FONT* Dfont, ALLEGRO_SAMPLE* SLogo, ALLEGRO_SAMPLE* SI1, ALLEGRO_BITMAP* Buffer, ALLEGRO_DISPLAY* Display, ALLEGRO_SAMPLE_INSTANCE* SInstance, ALLEGRO_BITMAP * Ship){
  short int a, b, x, y;

  al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
  al_set_sample(SInstance, SI1);

  BUFFER;
  al_draw_textf(Dfont,al_map_rgb(255,255,255), WORLD_WIDTH/2, WORLD_HEIGHT/2, ALLEGRO_ALIGN_CENTER,"Lionel Messi Studios presents");
  DRAW;
  al_play_sample(SLogo, 1.5, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
  al_rest(2.0);

  for(a = 255; a>0;a-=5){
    BUFFER;
    al_draw_textf(Dfont,al_map_rgb(a, a, a), WORLD_WIDTH/2, WORLD_HEIGHT/2, ALLEGRO_ALIGN_CENTER,"Lionel Messi Studios presents");
    DRAW;
    al_rest(0.01);
  }
  CLEAR;
  al_flip_display();

  al_rest(1.0);
  al_play_sample_instance (SInstance);
  //al_start_timer(Timer);

  
  for(x = -32, y = WORLD_HEIGHT+32, b = 255; x <= WORLD_WIDTH + 64; x+=100, y-=56){
    BUFFER;
    CLEAR;
    al_draw_line(-32,WORLD_HEIGHT+32,x,y,al_map_rgba(255,255,255,b),20.0);
    al_draw_scaled_rotated_bitmap(Ship,al_get_bitmap_width(Ship)/2,al_get_bitmap_height(Ship)/2,x,y,0.75,0.75,6*ALLEGRO_PI/18,0);
    al_rest(1/30.0);
    DRAW;
  }
  while(b > 0){
    BUFFER;
    CLEAR;
    al_draw_line(-32,WORLD_HEIGHT+32,x,y,al_map_rgba(255,255,255,b),20.0);
    b-=8;
    al_rest(1/30.0);
    if (b < 0) b = 0;
    DRAW;
  }
  for(x = WORLD_WIDTH + 32, y = WORLD_HEIGHT + 32, b = 255; x >= -64; x-=100, y-=56){
    BUFFER;
    CLEAR;
    al_draw_line(WORLD_WIDTH + 32, WORLD_HEIGHT + 32,x,y,al_map_rgba(255,255,255,b),20.0);
    al_draw_scaled_rotated_bitmap(Ship,al_get_bitmap_width(Ship)/2,al_get_bitmap_height(Ship)/2,x,y,0.75,0.75,-6*ALLEGRO_PI/18,0);
    al_rest(1/30.0);
    DRAW;
  }
  while(b > 0){
    BUFFER;
    CLEAR;
    al_draw_line(WORLD_WIDTH + 32, WORLD_HEIGHT + 32,x,y,al_map_rgba(255,255,255,b),20.0);
    b-=8;
    al_rest(1/30.0);
    if (b < 0) b = 0;
    DRAW;
  }
  for (x = WORLD_WIDTH/2, y = WORLD_HEIGHT + 64, a = 100; al_get_sample_instance_playing(SInstance); y -= a){
    BUFFER;
    CLEAR;
    al_draw_line(WORLD_WIDTH/2, WORLD_HEIGHT + 64 , x, y, al_map_rgb(255,255,255),20.0);
    al_draw_scaled_rotated_bitmap(Ship,al_get_bitmap_width(Ship)/2,al_get_bitmap_height(Ship)/2,x,y,0.75,0.45,0,0);
    if (a > 20) a -= 20;
    else if (a > 10) a -= 5;
    else if (a > -3) a -= 1;
    al_rest(1/30.0);
    DRAW;
  }

 }

void Draw_frame(ALLEGRO_DISPLAY* Display, ALLEGRO_BITMAP* Buffer){
    al_set_target_backbuffer(Display);
    al_draw_scaled_bitmap(Buffer,0,0,WORLD_WIDTH,WORLD_HEIGHT,0,0,al_get_display_width(Display),al_get_display_height(Display),0);
    al_flip_display();
}