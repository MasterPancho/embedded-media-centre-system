#ifndef MAINMENU_H
#define MAINMENU_H

#include "LPC17xx.h"
#include "ModeEnum.h"    // Mode Enumerations

extern void InitMainMenu(void);
extern void ResetMenu(void);
extern void HighlightMenu(void);
extern void RunMainMenu(uint32_t stickDir, uint32_t *prevStickDir, AppMode *currentMode);

#endif
