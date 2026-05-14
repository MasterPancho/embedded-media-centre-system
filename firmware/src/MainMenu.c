#include "MainMenu.h"
#include <stdio.h>
#include "LPC17xx.h"                      
#include "GLCD.h"
#include "Board_ADC.h"
#include "Board_Joystick.h"

#define __FI        1                      /* Font index 16x24               */

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

//Define Main Menu Section
#define PHOTO 3
#define MP3   4
#define GAME  5

static volatile uint16_t AD_dbg;

uint16_t ADC_last;                      // Last converted value
/* Import external variables from IRQ.c file                                  */
extern uint8_t  clock_ms;

//Current selection
int selection = PHOTO;

//Clean up the menu
void ResetMenu(void) {
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(Blue);
    GLCD_DisplayString(PHOTO, 0, __FI, "1. Photo Gallery    ");
    GLCD_DisplayString(MP3, 0, __FI, "2. MP3 Player         ");
    GLCD_DisplayString(GAME, 0, __FI, "3. Games             ");
}

void HighlightMenu(){
	//Restart the menu to clean old blue selection
	ResetMenu();

	GLCD_SetBackColor(Blue);
	GLCD_SetTextColor(White);

	//Set the limitations to make it a selection loop.
	if (selection < PHOTO)
		selection = GAME;

	if (selection > GAME)
		selection = PHOTO;

	//Highlight the given option
	switch(selection){
		case PHOTO:
			GLCD_DisplayString(PHOTO, 0, __FI, "1. Photo Gallery            ");
			break;

		case MP3:
			GLCD_DisplayString(MP3, 0, __FI, "2. MP3 Player           ");
			break;

		case GAME:
			GLCD_DisplayString(GAME, 0, __FI, "3. Games            ");
			break;
	}

	//Put design back to default for future display
	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Blue);
}

//Set up the menu
void InitMainMenu(void){
	//Setting up the LCD Interface
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(Yellow);
  GLCD_DisplayString(0, 0, __FI, "     COE718 Project    ");
	GLCD_DisplayString(1, 0, __FI, "                       ");
	GLCD_SetTextColor(White);
  GLCD_DisplayString(2, 0, __FI, "       Main Menu       ");

	//Display the Menu
	ResetMenu();
	HighlightMenu();
}

// Runs in the main loop when currentMode == MODE_MENU
void RunMainMenu (uint32_t stickDir, uint32_t *prevStickDir, AppMode *currentMode) {

	// Toogle Joystick: Only perform action when the direction changes
	if (stickDir != *prevStickDir) {
		switch(stickDir){

			case JOYSTICK_UP:
				//Decrement the selection by one
				selection--;

				//Highlight the current option with blue
				HighlightMenu();
				break;

			case JOYSTICK_DOWN:
				//Increment the selection by one
				selection++;

				//Highlight the current option with blue
				HighlightMenu();
				break;

			case JOYSTICK_CENTER:
				if(selection == PHOTO) {
					*currentMode = MODE_PHOTO;
				}
				
				else if(selection == MP3){
					*currentMode = MODE_MP3;
				}
				
				else if(selection == GAME) {
					*currentMode = MODE_GAME;
				}
				break;
		}
		*prevStickDir = stickDir; // update previous state
	}

	// Reset when joystick released (no direction)
	if (stickDir == 0)
		*prevStickDir = 0;
}
