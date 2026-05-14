#ifndef PHOTOGALLERY_H
#define PHOTOGALLERY_H

#include "LPC17xx.h"
#include "ModeEnum.h"    // Mode Enumerations

extern void InitPhotoGallery(void);
extern void RunPhotoGallery(uint32_t stickDir, uint32_t *prevStickDir, AppMode *currentMode);

#endif
