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

#define WORLD_WIDTH 960
#define WORLD_HEIGHT 540

#define PLAYER_INITIAL_LIVES 3
#define PLAYER_W 10
#define PLAYER_H 20
#define PLAYER_MARGIN 10 // Que tan separado debe estar de margenes laterales e inferior

#define ALIENS_ROWS 6
#define ALIENS_COLUMNS 4
#define ALIENS_HORIZONTAL_SEPARATION 6
#define ALIENS_VERTICAL_SEPARATION 12
#define ALIENS_W 32
#define ALIENS_H 32

#define SHOT_W 4
#define SHOT_H 14

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

enum flags {CLOSED = 0, MENU, GAME, PAUSE};

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/

#endif // _GENERAL_H_
