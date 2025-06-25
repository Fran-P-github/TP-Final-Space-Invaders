/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     front_allegro.c
     25/6/25

     Description:


     
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#include "front_allegro.h"
#include "general_defines.h"
#include "back.h"
#include "menu_allegro.h"

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define MSJ_ERR_INIT "Problema al inicializar: "

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/
 
/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void draw_alien(unsigned i, unsigned j);
static void draw_player();
static void draw_player_shot();
static void draw_alien_shot();
static void init_error(bool state, const char* name);

static void front_init();
static void front_update();
static void kill_all();

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static ALLEGRO_TIMER* timer;
static ALLEGRO_DISPLAY* disp;
static ALLEGRO_EVENT_QUEUE* queue;

// Back variables
static player_t* player;
static alien_t (*aliens) [ALIENS_ROWS][ALIENS_COLUMNS];
static double* aliens_move_interval;
static shot_t* player_shot;
static shot_t* alien_shot;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void front_run(){
    front_init();
    front_loop();
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

//Complete...
static void kill_all(){

}

static void init_error(bool state, const char* name){
    if(!state){
        fprintf(stderr, "%s%s\n", MSJ_ERR_INIT, name);
        exit(-1);
    }
}



void front_loop(){
    char state = MENU;
    while (state){
        switch (state){
            case MENU:
            load_menu();
            break;
            
        }
    }
    killall();
}

static void front_init(){
    player = get_player();
    aliens = get_aliens();
    aliens_move_interval = get_aliens_move_interval();
    player_shot = get_player_shot();
    alien_shot = get_alien_shot();

    init_error(al_init(), "Allegro");

    init_error(al_init_primitives_addon(), "Allegro Primitives");

    timer = al_create_timer(1.0 / 30.0); // 30 FPS
    init_error(timer, "Timer");
    al_start_timer(timer);

    disp = al_create_display(WORLD_WIDTH, WORLD_HEIGHT);
    init_error(disp, "Display");

    queue = al_create_event_queue();
    init_error(queue, "Queue");
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));
}



// TODO acomodar esto para que no se quede adentro de la funcion. mejor que se llame muchas veces desde el main
static void front_update(){
    ALLEGRO_EVENT event;
    bool redraw = false;
    bool done = false;
    unsigned long long frame = 0;

    while(!done){
        while(!al_get_next_event(queue, &event));

        switch(event.type){
            case ALLEGRO_EVENT_TIMER:
                aliens_update_position();
                shots_update();
                redraw = true;
                ++frame;
                break;

            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                done = true;
                break;
        }

        if(redraw){
            redraw = false;
            al_clear_to_color(al_map_rgb(0, 0, 0));
            unsigned i, j;

            player_try_shoot();
            draw_player_shot();
            alien_try_shoot(3);
            draw_alien_shot();

            draw_player();
            for(i=0; i<ALIENS_ROWS; ++i){
                for(j=0; j<ALIENS_COLUMNS; ++j){
                    if((*aliens)[i][j].is_alive){
                        draw_alien(i, j);
                    }
                }
            }

            al_flip_display();
        }
    }
}

static void draw_alien(unsigned i, unsigned j){
    al_draw_filled_rectangle((*aliens)[i][j].x, (*aliens)[i][j].y, (*aliens)[i][j].x+ALIENS_W, (*aliens)[i][j].y+ALIENS_H, al_map_rgb(255, 0, 0));
}

static void draw_player(){
    al_draw_filled_rectangle(player->x, player->y, player->x+PLAYER_W, player->y+PLAYER_H, al_map_rgb(0, 255, 0));
}

static void draw_alien_shot(){
    if(alien_shot->is_used)
        al_draw_filled_rectangle(alien_shot->x, alien_shot->y, alien_shot->x+SHOT_W, alien_shot->y+SHOT_H, al_map_rgb(255, 255, 255));
}

static void draw_player_shot(){
    if(player_shot->is_used)
        al_draw_filled_rectangle(player_shot->x, player_shot->y, player_shot->x+SHOT_W, player_shot->y+SHOT_H, al_map_rgb(255, 255, 255));
}
