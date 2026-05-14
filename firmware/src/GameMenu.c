#include "GameMenu.h"
#include "snake.h"
#include "pong.h"
#include <stdio.h>
#include "LPC17xx.h"                      
#include "GLCD.h"
#include "Board_ADC.h"
#include "Board_Joystick.h"

#define __FI        1                      /* Font index 16x24               */

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

//Define Main Menu Section
#define SNAKE  3
#define PONG   4
//#define GAME3  5
#define BACK   8

static volatile uint16_t AD_dbg;

//Currently we are in the Game Menu
GameMode currentGame = GAME_MENU;

//Current selection
int gameSelect = SNAKE;

//Clean up the menu
void ResetGameMenu(void) {
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(Blue);
    GLCD_DisplayString(SNAKE, 0, __FI, "1. Snake             ");
    GLCD_DisplayString(PONG, 0, __FI, "2. Pong               ");
//    GLCD_DisplayString(GAME3, 0, __FI, "3. Game3             ");
		GLCD_DisplayString(BACK, 0, __FI, "   Go to Main Menu    ");
}

void HighlightGameMenu(){
	//Restart the menu to clean old blue selection
	ResetGameMenu();

	GLCD_SetBackColor(Blue);
	GLCD_SetTextColor(White);

	//Set the limitations to make it a selection loop.
	if (gameSelect < SNAKE)
		gameSelect = BACK;

	if (gameSelect > BACK)
		gameSelect = SNAKE;

	//Highlight the given option
	switch(gameSelect){
		case SNAKE:
			GLCD_DisplayString(SNAKE, 0, __FI, "1. SNAKE            ");
			break;

		case PONG:
			GLCD_DisplayString(PONG, 0, __FI, "2. PONG           ");
			break;

//		case GAME3:
//			GLCD_DisplayString(GAME3, 0, __FI, "3. GAME3            ");
//			break;

		case BACK:
			GLCD_DisplayString(BACK, 0, __FI, "   Go to Main Menu   ");
			break;
	}

	//Put design back to default for future display
	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Blue);
}

//Set up the menu
void InitGameMenu(void){
	
	//Setting up the LCD Interface
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(Yellow);
  GLCD_DisplayString(0, 0, __FI, "     COE718 Project    ");
	GLCD_DisplayString(1, 0, __FI, "                       ");
	GLCD_SetTextColor(White);
  GLCD_DisplayString(2, 0, __FI, "       Game Menu       ");
	
	//Display the Menu
	ResetGameMenu();
	HighlightGameMenu();
}

// Runs in the main loop when currentMode == MODE_GAME
void RunGameMenu (uint32_t stickDir, uint32_t *prevStickDir, AppMode *currentMode) {
	
	// Toogle Joystick: Only perform action when the direction changes
	if (stickDir != *prevStickDir) {
		switch(stickDir){

			case JOYSTICK_UP:
				
			  //If we are on the back button, move it to GAME3
				if(gameSelect == BACK){
					gameSelect = PONG;
				}
				
				else{
					//Decrement the selection by one
					gameSelect--;
				}
				
				//Highlight the current option with blue
				HighlightGameMenu();
				break;

			case JOYSTICK_DOWN:
			  //If we are on the last game option, move it to BACK
				if(gameSelect == PONG){
					gameSelect = BACK;
				}
			
				else{
					//Increment the selection by one
					gameSelect++;
				}			

				//Highlight the current option with blue
				HighlightGameMenu();
				break;

			case JOYSTICK_CENTER:
				
				switch(gameSelect){

					case SNAKE:
						if (currentGame != GAME_SNAKE){
							currentGame = GAME_SNAKE;
							InitSnakeMenu();
						} 
						
						//Loop through the snake game until the user chooses to go back
						RunSnakeMenu(&prevStickDir, &currentGame);   
						
						// Snake exited — refresh Game Menu immediately
						if (currentGame == GAME_MENU) {
								GLCD_Clear(White);
								InitGameMenu();
								HighlightGameMenu();
								*prevStickDir = 0;  // reset joystick state
						}
						break;
					
					case PONG:
						if (currentGame != GAME_PONG){
							currentGame = GAME_PONG;
							InitPongMenu();
						} 
						
						//Loop through the snake game until the user chooses to go back
						RunPongMenu(&prevStickDir, &currentGame);   
						
						// Snake exited — refresh Game Menu immediately
						if (currentGame == GAME_MENU) {
								GLCD_Clear(White);
								InitGameMenu();
								HighlightGameMenu();
								*prevStickDir = 0;  // reset joystick state
						}
						break;

					case BACK:
						*currentMode = MODE_MENU;
						break;
					
				}
			}
			
		*prevStickDir = stickDir; // update previous state
	}

	// Reset when joystick released (no direction)
	if (stickDir == 0)
		*prevStickDir = 0;
}
