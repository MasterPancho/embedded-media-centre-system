/*------------------------------------------------------------------------------
 * Example header Blinky module
 * Copyright (c) 2019-2020 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    Blinky.h
 * Purpose: Blinky header 
 *----------------------------------------------------------------------------*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODEENUM_H
#define __MODEENUM_H

//Use enum to differentiate between the execution options
typedef enum {
    MODE_MENU,
    MODE_PHOTO,
    MODE_MP3,
    MODE_GAME
} AppMode;

typedef enum {
		GAME_MENU,
    GAME_SNAKE,
    GAME_PONG,
    //GAME_GAME3,
} GameMode;

#endif /* __BLINKY_H */
