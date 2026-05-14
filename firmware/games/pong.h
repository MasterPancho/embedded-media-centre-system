#ifndef PONG_H
#define PONG_H

#include "LPC17xx.h"
#include "ModeEnum.h"    // Mode Enumerations

//Menu Logic
extern void InitPongMenu(void);
extern void ResetPongMenu(void);
extern void HighlightPongMenu(void);
extern void RunPongMenu(uint32_t **prevStickDir, GameMode *currentMode);		//Double pointer since it's coming from game menu as a pointer already.

//Game Logic
extern void InitPongGame(void);
extern void DrawPongGame(void);
extern void RunPongGame(void);
#endif

