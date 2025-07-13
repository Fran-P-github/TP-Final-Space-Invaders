/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     general_defines.h
     25/6/25

     Description:



  ******************************************************************************/

#ifndef _GENERAL_H_
#define _GENERAL_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define PLAYER_INITIAL_LIVES 3

#define MOTHERSHIP_POINTS_SILVER 40
#define MOTHERSHIP_POINTS_GOLD 60
#define MOTHERSHIP_POINTS_NEON 100

#define ALIEN_FAT_POINTS 5
#define ALIEN_WINGED_POINTS 10
#define ALIEN_SMALL_POINTS 15
#define ALIEN_POINTS_SILVER MOTHERSHIP_POINTS_SILVER/2
#define ALIEN_POINTS_GOLD MOTHERSHIP_POINTS_GOLD/2
#define ALIEN_POINTS_NEON MOTHERSHIP_POINTS_NEON/2

#define RPI 0
#define ALLEGRO 2
#ifndef PLATFORM
#error no PLATFORM defined
#endif

#if PLATFORM == ALLEGRO

#define FRAME_RATE 30
#define WORLD_WIDTH 960
#define WORLD_HEIGHT 540

#define MOTHERSHIP_W 60
#define MOTHERSHIP_H 30
#define MOTHERSHIP_MARGIN 35

#define PLAYER_W 80
#define PLAYER_H 40
#define PLAYER_MARGIN 10 // Distance from side and bottom edges

#define ALIENS_ROWS 6
#define ALIENS_COLUMNS 10
#define ALIENS_HORIZONTAL_SEPARATION 28
#define ALIENS_VERTICAL_SEPARATION 18
#define ALIENS_W 40
#define ALIENS_H 32
#define ALIENS_MARGIN 75 // Distance from top edge

#define SHOT_W 8
#define SHOT_H 19

#define SHIELDS_CANT 3
#define SHIELD_BLOCK_LIVES 3
#define SHIELD_BLOCK_W 12
#define SHIELD_BLOCK_H 12
#define SHIELD_W 7
#define SHIELD_H 4
#define SHIELD_FORM {                    \
    {' ', '*', '*', '*', '*', '*', ' '}, \
    {' ', '*', '*', '*', '*', '*', ' '}, \
    {'*', '*', '*', '*', '*', '*', '*'}, \
    {'*', '*', ' ', ' ', ' ', '*', '*'}}
#define SHIELD_TO_PLAYER_MARGIN 40

#elif PLATFORM == RPI

#define FRAME_RATE 1
#define WORLD_WIDTH 16
#define WORLD_HEIGHT 16

#define MOTHERSHIP_W 3
#define MOTHERSHIP_H 1
#define MOTHERSHIP_MARGIN 0

#define PLAYER_W 3
#define PLAYER_H 1
#define PLAYER_MARGIN 0 // Distance from side and bottom edges

#define ALIENS_ROWS 2
#define ALIENS_COLUMNS 3
#define ALIENS_HORIZONTAL_SEPARATION 2
#define ALIENS_VERTICAL_SEPARATION 1
#define ALIENS_W 2
#define ALIENS_H 2
#define ALIENS_MARGIN 2 // Distance from top edge

#define SHOT_W 1
#define SHOT_H 1

#define SHIELDS_CANT 2
#define SHIELD_BLOCK_LIVES 1
#define SHIELD_BLOCK_W 1
#define SHIELD_BLOCK_H 1
#define SHIELD_W 3
#define SHIELD_H 2
#define SHIELD_FORM { \
    {' ', '*', ' '},  \
    {'*', '*', '*'},  \
}
#define SHIELD_TO_PLAYER_MARGIN 1

#endif

// Resource folder macros
#define AUDIO_ROUTE(s) "../assets/Audio/"s
#define BITMAP_ROUTE(s) "../assets/Bitmap/"s
#define FONT_ROUTE(s) "../assets/Font/"s
#define SPRITE_ROUTE(s) "../assets/sprites/"s

// Audio files (Same across all platforms)
#define AUDIO_PLAYER_SHOT    AUDIO_ROUTE("shoot.wav")
#define AUDIO_PLAYER_DEATH   AUDIO_ROUTE("playerkilled.wav")
#define AUDIO_INVADER_DEATH  AUDIO_ROUTE("invaderkilled.wav")
#define AUDIO_INVADER_MOVED  AUDIO_ROUTE("invadermoved.wav")
#define AUDIO_INVADER_HIT    AUDIO_ROUTE("alienHitSound.wav")
#define AUDIO_SHIELD_HIT     AUDIO_ROUTE("shieldHitSound.wav")
#define AUDIO_UFO            AUDIO_ROUTE("ufo.wav")
#define AUDIO_UFO_DEATH      AUDIO_ROUTE("ufoDeath.wav")
#define GAME_BG_MUSIC        AUDIO_ROUTE("gameBackground.wav")
#define AUDIO_MENU_SONG      AUDIO_ROUTE("menuSong.wav")
#define AUDIO_PAUSE          AUDIO_ROUTE("pauseSound.wav")

// Sprite sheets
#define SPRITESHEET_INVADERS "../assets/sprites/alien_and_ufo_sprites.png"
#define SPRITESHEETSHOT "../assets/sprites/shot_animations.png"

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
// TODO: pasar game_state_t a front.h
typedef enum {
  CLOSED = 0,
  MENU,
  GAME,
  PAUSE,
  ENDGAME 
} game_state_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/

#endif // _GENERAL_H_
