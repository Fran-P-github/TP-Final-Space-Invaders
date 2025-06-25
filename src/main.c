

#include"general_defines.h"
#include"back.h"
#include"front_allegro.h"

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

int main(){
    player_init();
    aliens_init();
    front_init();

    front_update();

    return 0;
}
