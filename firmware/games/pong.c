#include "pong.h"
#include <stdio.h>
#include "LPC17xx.h"                      
#include "GLCD.h"
#include "Board_ADC.h"
#include "Board_Joystick.h"
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>   // For time() to seed rand()

#define __FI        1                      /* Font index 16x24               */
#define BOARD_WIDTH   20   							// GLCD columns 
#define BOARD_HEIGHT  10   							// GLCD rows 
#define MAX_LENGTH 100
#define MAX_POINTS 3										//Max score to win

//Racket Defaults
#define RACKET_LENGTH 3
#define PLAYER_X 1
#define CPU_X 18

//Define Main Menu Section
#define PLAY   5
#define BACK   6

//Current selection
int pongSelect = PLAY;

//use joystick
uint32_t pongStickDir;
uint32_t pongPrevDir = 0;

//Racket Location
typedef struct {
	int x;
	int y;
} racLocation;

//Player racket
racLocation playerRacket[RACKET_LENGTH];
racLocation cpuRacket[RACKET_LENGTH];

//Ball location + speed
typedef struct {
	double x;
	double y;
	double dx;						//Speed in x-axis
	double dy;						//Speed in y-axis
} ballLocation;

ballLocation ball;

//Player states
bool isPongAlive;
int playerScore;
int cpuScore;

char playerScoreStr[20];
char cpuScoreStr[20];
char result[20];
//Delays in ms
static void Delay(unsigned int ms) {
    volatile unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 9080; j++) {
        }
    }
}

//Clean up the menu
void ResetPongMenu(void) {
    GLCD_SetBackColor(Black);
    GLCD_SetTextColor(White);
		GLCD_DisplayString(2, 0, __FI, "        PONG               ");
		GLCD_DisplayString(3, 0, __FI, "       The Game             ");
		GLCD_DisplayString(PLAY, 0, __FI, "        PLAY             ");
		GLCD_DisplayString(BACK, 0, __FI, "        BACK             ");
}

void HighlightPongMenu(void){
	//Restart the menu to clean old blue selection
	ResetPongMenu();

	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Black);

	//Set the limitations to make it a selection loop.
	if (pongSelect < PLAY)
		pongSelect = BACK;

	if (pongSelect > BACK)
		pongSelect = PLAY;

	//Highlight the given option
	switch(pongSelect){
		case PLAY:
			GLCD_DisplayString(PLAY, 0, __FI, "        PLAY            ");
			break;

		case BACK:
			GLCD_DisplayString(BACK, 0, __FI, "        BACK            ");
			break;
	}

	//Put design back to default for future display
	GLCD_SetBackColor(White);
	GLCD_SetTextColor(Blue);
}

//Set up the menu
void InitPongMenu(void){
	GLCD_Clear(Black);                         /* Clear graphical LCD display   */
  
	//Display the Menu
	HighlightPongMenu();
}

// Runs in the main loop when currentMode == MODE_MENU
void RunPongMenu(uint32_t **prevStickDir, GameMode *currentGame) {
	
	while(*currentGame == GAME_PONG){
		pongStickDir = Joystick_GetState();
		
		// Toogle Joystick: Only perform action when the direction changes
		if (pongStickDir != **prevStickDir) {
		  pongStickDir = Joystick_GetState();
			switch(pongStickDir){

				case JOYSTICK_UP:
					//Decrement the selection by one
					pongSelect--;
					
					//Highlight the current option with blue
					HighlightPongMenu();
					break;

				case JOYSTICK_DOWN:
					//Increment the selection by one
					pongSelect++;
					
					//Highlight the current option with blue
					HighlightPongMenu();
					break;

				case JOYSTICK_CENTER:
					if(pongSelect == PLAY) {
						RunPongGame();
					}
					
					else if(pongSelect == BACK){
						*currentGame = GAME_MENU;
					}				
					break;
			}
			**prevStickDir = pongStickDir; // update previous state
		}

		// Reset when joystick released (no direction)
		if (pongStickDir == 0)
			**prevStickDir = 0;
	}
}

////////////////
// Game Logic //
////////////////

//Draw the game boundaries.
void DrawPongGame(){
	int i;
	//Draw Center Line
	for(i = 1; i < BOARD_HEIGHT-1; i++){
		GLCD_SetBackColor(Black);
		GLCD_SetTextColor(White);
		GLCD_DisplayString(i, 9, __FI, "|");
		GLCD_SetBackColor(White);
	}
	
	//Player Score
	GLCD_SetTextColor(Black);
	sprintf(playerScoreStr, "Pts:%d", playerScore);
	GLCD_DisplayString(0, 0, __FI, (unsigned char*)playerScoreStr);
	
	//CPU Score
	sprintf(cpuScoreStr, "Pts:%d", cpuScore);
	GLCD_DisplayString(0, 15, __FI, (unsigned char*)cpuScoreStr);
	
	GLCD_SetTextColor(Blue);
}

void InitPongGame(){
	int i;
	//Setting up the LCD Interface
	GLCD_Clear(Black);                         /* Clear graphical LCD display   */
		
	//Draw Borders
	for(i =0; i < BOARD_WIDTH; i++){
		GLCD_DisplayString(0, i, __FI, " ");
		GLCD_DisplayString(9, i, __FI, " ");
	}
	
	//Initialize player racket at the center of the left side of the LCD
  playerRacket[0].y = BOARD_HEIGHT / 2;
	playerRacket[1].y = playerRacket[0].y + 1;
	playerRacket[2].y = playerRacket[1].y + 1;
	
	//Initialize CPU racket at the center of the right side of the LCD
  cpuRacket[0].y = BOARD_HEIGHT / 2;
	cpuRacket[1].y = playerRacket[0].y + 1;
	cpuRacket[2].y = playerRacket[1].y + 1;
	
	//Ball position
	ball.x = 10.0;
	ball.y = 4.0;
	
	//Ball Speed (Random direction)
	ball.dx = (rand() % 2 == 0) ? 1.0 : -1.0;
	ball.dy = (rand() % 2 == 0) ? 1.0 : -1.0;
	
	//User states
	isPongAlive = true;
	playerScore = 0;
	cpuScore = 0;
} 

void MoveBall(){
	//Delete current position of ball
	GLCD_SetBackColor(Black);
	GLCD_DisplayString(ball.y,ball.x, __FI, " ");
	
	ball.x += ball.dx;
	ball.y += ball.dy;
	
	//Collision with walls
	if(ball.y < 2 || ball.y > BOARD_HEIGHT-3){
		ball.dy = -ball.dy;
	}
	
	//Collision with rackets
	if((ball.x == PLAYER_X + 1 && ball.y >= playerRacket[0].y && ball.y <= playerRacket[2].y) ||
		(ball.x == CPU_X - 1 && ball.y >= cpuRacket[0].y && ball.y <= cpuRacket[2].y)) {
        ball.dx = -ball.dx;
    }
	
	// Check if the ball goes out of bounds
	if (ball.x <= 0) {
			// CPU scores a point
			cpuScore++;
			ball.x = BOARD_WIDTH / 2; // Reset ball position
			ball.y = BOARD_HEIGHT / 2;
			ball.dx = (rand() % 2 == 0) ? 1 : -1;  // Randomize direction
			ball.dy = (rand() % 2 == 0) ? 1 : -1;
		
		//GAME OVER
		if(cpuScore == MAX_POINTS){
			isPongAlive = false;
			sprintf(result,  "YOU LOSE!");
		}
	} 
	else if (ball.x >= BOARD_WIDTH - 1) {
			// Player scores a point
			playerScore++;
			ball.x = BOARD_WIDTH / 2; // Reset ball position
			ball.y = BOARD_HEIGHT / 2;
			ball.dx = (rand() % 2 == 0) ? 1 : -1;		// Randomize direction
			ball.dy = (rand() % 2 == 0) ? 1 : -1;
		
		
		//Add GAME OVER
		if(playerScore == MAX_POINTS){
			isPongAlive = false;
			sprintf(result,  "YOU WIN!");
		}
	}
	
	//Redraw the ball in the new location
	GLCD_SetBackColor(White);
	GLCD_DisplayString(ball.y,ball.x, __FI, " ");
	Delay(50);
}

void MovePlayerRacket() {
	int i;
 
	// Delete old player racket position
	GLCD_SetBackColor(Black);
	for(i = 0; i < RACKET_LENGTH; i++) {
			GLCD_DisplayString(playerRacket[i].y, PLAYER_X, __FI, " ");
	}
		
	//Move up
	if (pongStickDir == JOYSTICK_UP && playerRacket[0].y > 1){
		for(i=0; i<RACKET_LENGTH; i++){
			playerRacket[i].y--;
		}
	}
	//Move down
	if (pongStickDir == JOYSTICK_DOWN && playerRacket[RACKET_LENGTH-1].y < 8){
		for(i=0; i<RACKET_LENGTH; i++){
			playerRacket[i].y++;
		}
	}
	
	//Re-draw Player
	GLCD_SetBackColor(White);
	for(i = 0; i < RACKET_LENGTH; i++) {
			GLCD_DisplayString(playerRacket[i].y, PLAYER_X, __FI, " ");
	}
}

void MoveCPURacket() {
	int i;
	
	// Delete old player racket position
	GLCD_SetBackColor(Black);
	for(i = 0; i < RACKET_LENGTH; i++) {
			GLCD_DisplayString(cpuRacket[i].y, CPU_X, __FI, " ");
	}
	
	// CPU racket follows the ball's y-position
	if (ball.y < cpuRacket[1].y && cpuRacket[0].y > 1){
		if(rand() % 2 == 0){
			for(i = 0; i < RACKET_LENGTH; i++){
				cpuRacket[i].y--;
			}
		}
	}
	 else if (ball.y > cpuRacket[1].y && cpuRacket[RACKET_LENGTH-1].y < 8){
		if(rand() % 2 == 0){
			for(i=0; i<RACKET_LENGTH; i++){
				cpuRacket[i].y++;
			}
		}
	}
	
	//Re-draw CPU
	GLCD_SetBackColor(White);
	for(i = 0; i < RACKET_LENGTH; i++) {
			GLCD_DisplayString(cpuRacket[i].y, CPU_X, __FI, " ");
	}
}

void RunPongGame() {
    //Game Initialization
    InitPongGame();
    DrawPongGame();
    
    while (isPongAlive) {
			pongStickDir = Joystick_GetState();

			// Move ball and handle collision
			MoveBall();
			
			// Move player and CPU rackets
			MoveCPURacket();
			MovePlayerRacket();
							
			// Draw game state (ball, rackets, score)
			DrawPongGame();
			
			// Delay for game speed control
			Delay(200); // Adjust this for game speed
		}
    
    // Game Over
		GLCD_SetBackColor(Black);
    GLCD_SetTextColor(White);
    GLCD_DisplayString(4, 5, __FI, " GAME OVER ");
    GLCD_DisplayString(5, 6, __FI, (unsigned char*)result);
		
		//Delay for 2 seconds
		Delay(2000);
		InitPongMenu();
}
