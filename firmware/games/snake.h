#ifndef SNAKE_H
#define SNAKE_H

#include "LPC17xx.h"
#include "ModeEnum.h"    // Mode Enumerations


//Menu Logic
extern void InitSnakeMenu(void);
extern void ResetSnakeMenu(void);
extern void HighlightSnakeMenu(void);
extern void RunSnakeMenu(uint32_t **prevStickDir, GameMode *currentMode);		//Double pointer since it's coming from game menu as a pointer already.

//Game Logic
extern void InitSnakeGame(void);
extern void DrawGame(void);
extern void RunSnakeGame(void);
#endif
