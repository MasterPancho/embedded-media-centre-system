#ifndef GAMEMENU_H
#define GAMEMENU_H

#include "LPC17xx.h"
#include "ModeEnum.h"    // Mode Enumerations

extern void InitGameMenu(void);
extern void ResetGameMenu(void);
extern void HighlightGameMenu(void);
extern void RunGameMenu(uint32_t stickDir, uint32_t *prevStickDir, AppMode *currentMode);

#endif
