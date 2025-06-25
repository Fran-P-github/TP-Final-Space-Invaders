#include <stdio.h>

// Modulos
#include "front_rb.h"
#include "general_defines.h"
#include "back.h"
// Matriz de leds y joystick
#include "../libs/joydisp/disdrv.h"
#include "../libs/joydisp/joydrv.h"
// Audio
#include <SDL2/SDL.h>
#include "../libs/SDL2/audio.h"

static void drawRectangle(dcoord_t p, int width, int height);
static dcoord_t wrapCords(int x, int y);

void front_init(){
    joy_init(); // Se inicializan el joystick y el display.
    disp_init();
    disp_clear(); // Se apaga el display.
    disp_update();
}

void front_update(){
    drawRectangle({3, 3}, 3, 3);
    disp_update();
}

static dcoord_t wrapCords(int x, int y){
    return {x, y};
}

static void drawRectangle(dcoord_t p, int width, int height){ // Recibe un punto del cual dibuja para abajo a la derecha
    if(p.x > DISP_MAX_X || p.y > DISP_MAX_Y){
        printf("No se dibuja nada, el rectangulo sobresale del display.");
        return;
    }
    int i, j;
    for(i = 0; i < width; i++){
        for(j = 0; j < height; j++){
            disp_write({i, j}, D_ON); // Se prende el led en la posicion {i, j}
        }
    } 
}

