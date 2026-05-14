#include "ModeEnum.h"    // Mode Enumerations
#include "MainMenu.h"    // menu functions
#include "PhotoGallery.h"    // menu functions
#include "MP3Player.h"
#include "GameMenu.h"
#include "Board_Joystick.h"
#include "GLCD.h"
#include "Board_ADC.h"

//Currently we are in Main Menu
AppMode currentMode = MODE_MENU;

//Set a previousMode in order for initialization execution to happen only once.
AppMode previousMode = MODE_GAME;

int main(void) {
    uint32_t stickDir;
    uint32_t prevStickDir = 0;
	
	//Initialize Hardware
	ADC_Initialize();
	GLCD_Init();                               /* Initialize graphical LCD (if enabled */
	Joystick_Initialize();
	
    while(1) {
			
        stickDir = Joystick_GetState();

		if(currentMode != previousMode){
			GLCD_Clear(White); 
			switch(currentMode){
				
				//Initialize Main Menu GUI 
				case MODE_MENU:
					InitMainMenu();     												// initialize menu (titles, colors, items)
					break;
				
				//Initialize Photo Gallery GUI 	
				case MODE_PHOTO:
					InitPhotoGallery();     												// initialize menu (titles, colors, items)
					break;
			
				//Run MP3 logic, since this one doesn't need to be called constantly.
				case MODE_MP3:
					RunMP3Player(stickDir, &prevStickDir, &currentMode);     												
					break;
				
				//Initialize Game Menu GUI
				case MODE_GAME:
					InitGameMenu();      												// initialize menu (titles, colors, items)
					break;
			}
			previousMode = currentMode;
		}
				
		//&prevSitckDir and &currentMode since we want to modify the actual variable in the function, not its copy
        switch(currentMode) {

					//Update Main Menu process
					case MODE_MENU:
						RunMainMenu(stickDir, &prevStickDir, &currentMode);
						break;
					
					//Update Photo Gallery process
					case MODE_PHOTO:
						RunPhotoGallery(stickDir, &prevStickDir, &currentMode);
						break;
					
					//Not required; Only in initialization
					case MODE_MP3:            
						break;
					
					//Update Game Menu process
					case MODE_GAME:
						RunGameMenu(stickDir, &prevStickDir, &currentMode);
						break;						
        }
    }
}
