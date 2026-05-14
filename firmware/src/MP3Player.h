#ifndef MP3PLAYER_H
#define MP3PLAYER_H

#include "LPC17xx.h"
#include "ModeEnum.h"    // Mode Enumerations

extern void InitMP3Player(void);
//extern void RunMP3Player(uint32_t stickDir, uint32_t *prevStickDir, AppMode *currentMode);
extern void RunMP3Player(uint32_t stickDir, uint32_t *prevStickDir, AppMode *currentMode);

#endif
