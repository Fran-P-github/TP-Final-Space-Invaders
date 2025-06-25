#include<stdio.h>
#include<allegro5/allegro5.h>
#include<allegro5/allegro_primitives.h>

#include"front_allegro.h"
#include"general_defines.h"
#include"back.h"

#define MSJ_ERR_INIT "Problema al inicializar: "

static ALLEGRO_TIMER* timer;
static ALLEGRO_DISPLAY* disp;
static ALLEGRO_EVENT_QUEUE* queue;

static void draw_alien(unsigned i, unsigned j);
static void init_error(bool state, const char* name);

static void init_error(bool state, const char* name){
    if(!state){
        fprintf(stderr, "%s%s\n", MSJ_ERR_INIT, name);
        exit(-1);
    }
}

void front_init(){
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
void front_update(){
    ALLEGRO_EVENT event;
    bool redraw = false;
    bool done = false;
    unsigned long long frame = 0;

    while(!done){
        while(!al_get_next_event(queue, &event));

        switch(event.type){
            case ALLEGRO_EVENT_TIMER:
                aliens_update_position();
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

            for(i=0; i<ALIENS_ROWS; ++i){
                for(j=0; j<ALIENS_COLUMNS; ++j){
                    if(aliens[i][j].is_alive){
                        draw_alien(i, j);
                    }
                }
            }

            al_flip_display();
        }
    }
}

static void draw_alien(unsigned i, unsigned j){
    al_draw_filled_rectangle(aliens[i][j].x, aliens[i][j].y, aliens[i][j].x+ALIENS_W, aliens[i][j].y+ALIENS_H, al_map_rgb(255, 0, 0));
}
