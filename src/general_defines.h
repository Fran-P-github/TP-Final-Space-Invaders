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
#define ALIENS_POINTS 5
#define SHIELDS_CANT 3
#define SHIELD_BLOCK_LIVES 2


#define RPI 0
#define ALLEGRO 2
//#define PLATFORM ALLEGRO
#ifndef PLATFORM
#error no PLATFORM defined
#endif

#if PLATFORM == ALLEGRO

#define FRAME_RATE 30
#define WORLD_WIDTH 960
#define WORLD_HEIGHT 540

#define PLAYER_W 10
#define PLAYER_H 20
#define PLAYER_MARGIN 10 // Que tan separado debe estar de margenes laterales e inferior

#define ALIENS_ROWS 6
#define ALIENS_COLUMNS 4
#define ALIENS_HORIZONTAL_SEPARATION 6
#define ALIENS_VERTICAL_SEPARATION 12
#define ALIENS_W 32
#define ALIENS_H 32
#define ALIENS_MARGIN 20 // Que tan separados del borde superior

#define SHOT_W 4
#define SHOT_H 14

#define SHIELDS_CANT 3
#define SHIELD_BLOCK_LIVES 2
#define SHIELD_BLOCK_W 12
#define SHIELD_BLOCK_H 12
#define SHIELD_W 7
#define SHIELD_H 4
#define SHIELD_FORM { \
{' ', '*', '*', '*', '*', '*', ' '}, \
{' ', '*', '*', '*', '*', '*', ' '}, \
{'*', '*', '*', '*', '*', '*', '*'}, \
{'*', '*', ' ', ' ', ' ', '*', '*'}  }
#define SHIELD_TO_PLAYER_MARGIN 14

#elif PLATFORM == RPI

#define FRAME_RATE 1
#define WORLD_WIDTH 16
#define WORLD_HEIGHT 16

#define PLAYER_W 3
#define PLAYER_H 1
#define PLAYER_MARGIN 0 // Que tan separado debe estar de margenes laterales e inferior

#define ALIENS_ROWS 2
#define ALIENS_COLUMNS 3
#define ALIENS_HORIZONTAL_SEPARATION 2
#define ALIENS_VERTICAL_SEPARATION 1
#define ALIENS_W 2
#define ALIENS_H 2
#define ALIENS_MARGIN 2 // Que tan separados del borde superior

#define SHOT_W 1
#define SHOT_H 1

#define SHIELD_BLOCK_W 1
#define SHIELD_BLOCK_H 1
#define SHIELD_W 3
#define SHIELD_H 2
#define SHIELD_FORM { \
{' ', '*', ' '}, \
{'*', '*', '*'}, }
#define SHIELD_TO_PLAYER_MARGIN 1

#endif

// Audio files (Son los mismos independientemente de la plataforma)
#define AUDIO_PLAYER_SHOT "../assets/Audio/shoot.wav" 
#define AUDIO_PLAYER_DEATH "../assets/Audio/explosion.wav" 
#define AUDIO_INVADER_DEATH "../assets/Audio/invaderkilled.wav" 
#define AUDIO_INVADER_MOVED "../assets/Audio/invadermoved.wav" 
#define AUDIO_UFO "../assets/Audio/ufo.wav"
#define AUDIO_MENU_SONG "../assets/Audio/menuSong.wav"

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {CLOSED = 0, MENU, GAME, PAUSE} game_state_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/

#endif // _GENERAL_H_
