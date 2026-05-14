#include "PhotoGallery.h"
#include <stdio.h>
#include "LPC17xx.h"                      
#include "GLCD.h"
#include "Board_Joystick.h"
#include "worldcup.c"
#include "messi.c"
#include "barcelona.c"

//Array storing all images
const unsigned char *gallery[] = {BARCELONA_PIXEL_DATA, WORLDCUP_PIXEL_DATA, MESSI_PIXEL_DATA};

int curPic = 0;
int totalImages = sizeof(gallery) / sizeof(gallery[0]);		//sizeof gives it in bytes, which is why we divide
	
void LoadPhoto(void){
	GLCD_Clear(White);
	
	//Loop to the last image in the array
	if(curPic < 0){
		curPic = totalImages-1;
	}
	
	//Loop to the first image in the array
	if(curPic >= totalImages){
		curPic = 0;
	}
	
	//Load image
	GLCD_Bitmap (  0,   0, 320,  240, (unsigned char*)gallery[curPic]);
}

void InitPhotoGallery(void){
	LoadPhoto();
}

void RunPhotoGallery(uint32_t stickDir, uint32_t *prevStickDir, AppMode *currentMode){
	
	// Toogle Joystick: Only perform action when the direction changes
	if (stickDir != *prevStickDir) {
		switch(stickDir){
			
			//Go to the previous image
			case JOYSTICK_LEFT:
				curPic--;
				LoadPhoto();
				break;

			//Go to the next image
			case JOYSTICK_RIGHT:
				curPic++;
				LoadPhoto();
				break;
			
			//Go back to the main menu
			case JOYSTICK_CENTER:
				*currentMode = MODE_MENU;
				break;
		}
		*prevStickDir = stickDir; // update previous state
	}

	// Reset when joystick released (no direction)
	if (stickDir == 0)
		*prevStickDir = 0;
}
