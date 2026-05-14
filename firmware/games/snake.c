#include "snake.h"
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

//Define Main Menu Section
#define PLAY   5
#define BACK   6

//Current selection
int snakeSelect = PLAY;

//use joystick
uint32_t snakeStickDir;
uint32_t prevDir = 0;

//Snake's body location
typedef struct {
	int x;
	int y;
} body;

//Food location
int foodX;
int foodY;

//Snake Location + Size
body snake[MAX_LENGTH];
int snakeSize;

//Player states
bool isAlive;
int score;
char scoreStr[20];


//Delays in ms
static void Delay(unsigned int ms) {
    volatile unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 9080; j++) {
        }
    }
}

//Clean up the menu
void ResetSnakeMenu(void) {
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(Blue);
		GLCD_DisplayString(2, 0, __FI, "        SNAKE               ");
		GLCD_DisplayString(3, 0, __FI, "       The Game             ");
		GLCD_DisplayString(PLAY, 0, __FI, "        PLAY             ");
		GLCD_DisplayString(BACK, 0, __FI, "        BACK             ");
}

void HighlightSnakeMenu(void){
	//Restart the menu to clean old blue selection
	ResetSnakeMenu();

	GLCD_SetBackColor(Blue);
	GLCD_SetTextColor(White);

	//Set the limitations to make it a selection loop.
	if (snakeSelect < PLAY)
		snakeSelect = BACK;

	if (snakeSelect > BACK)
		snakeSelect = PLAY;

	//Highlight the given option
	switch(snakeSelect){
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
void InitSnakeMenu(void){
	//Setting up the LCD Interface
	GLCD_Clear(White);                         /* Clear graphical LCD display   */
	
	//Display the Menu
	HighlightSnakeMenu();
}

// Runs in the main loop when currentMode == MODE_MENU
void RunSnakeMenu(uint32_t **prevStickDir, GameMode *currentGame) {
	
	while(*currentGame == GAME_SNAKE){
		snakeStickDir = Joystick_GetState();
		
		// Toogle Joystick: Only perform action when the direction changes
		if (snakeStickDir != **prevStickDir) {
		  snakeStickDir = Joystick_GetState();
			switch(snakeStickDir){

				case JOYSTICK_UP:
					//Decrement the selection by one
					snakeSelect--;
					
					//Highlight the current option with blue
					HighlightSnakeMenu();
					break;

				case JOYSTICK_DOWN:
					//Increment the selection by one
					snakeSelect++;
					
					//Highlight the current option with blue
					HighlightSnakeMenu();
					break;

				case JOYSTICK_CENTER:
					if(snakeSelect == PLAY) {
						RunSnakeGame();
					}
					
					else if(snakeSelect == BACK){
						*currentGame = GAME_MENU;
					}				
					break;
			}
			**prevStickDir = snakeStickDir; // update previous state
		}

		// Reset when joystick released (no direction)
		if (snakeStickDir == 0)
			**prevStickDir = 0;
	}
}

////////////////
// Game Logic //
////////////////

//Draw the game boundaries.
void DrawGame(){
    int i;
    
    // Borders
    GLCD_SetBackColor(Black);
    for(i = 0; i < BOARD_WIDTH; i++){
        GLCD_DisplayString(0, i, __FI, " ");
        GLCD_DisplayString(9, i, __FI, " ");
    }
    for(i = 0; i < BOARD_HEIGHT; i++){
        GLCD_DisplayString(i, 0, __FI, " ");
        GLCD_DisplayString(i, 19, __FI, " ");
    }

    // Snake head
    GLCD_SetBackColor(Green);
    GLCD_DisplayString(snake[0].y, snake[0].x, __FI, " ");  // Redraw head
    
    // Snake body
    for(i = 1; i < snakeSize; i++){
        GLCD_SetBackColor(Green);
        GLCD_DisplayString(snake[i].y, snake[i].x, __FI, " ");  // Redraw body segments
    }

    // Food
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(Red);
    GLCD_DisplayString(foodY, foodX, __FI, "*");

    // Score
    GLCD_SetBackColor(Black);
    GLCD_SetTextColor(White);
    sprintf(scoreStr, "Score: %d", score);
    GLCD_DisplayString(0, 11, __FI, (unsigned char*)scoreStr);
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(Blue);
}

void InitSnakeGame(){
	//Setting up the LCD Interface
	GLCD_Clear(White);                         /* Clear graphical LCD display   */
	
	//Initial food location
	foodX = 17;
	foodY = 4;
	
	//Initialize snake at the center of the LCD
  snakeSize = 3;
	snake[0].x = BOARD_WIDTH / 2;
	snake[0].y = BOARD_HEIGHT / 2;
	snake[1].x = snake[0].x - 1;
	snake[1].y = snake[0].y;
	snake[2].x = snake[1].x - 1;
	snake[2].y = snake[1].y;
	
	//Snake initial direction
	prevDir = JOYSTICK_UP;
	
	//User states
	isAlive = true;
	score = 0;
} 

void ControlFood(){
	//Set a random number depending on he axis, and dont let the fruit appear on the border line
	foodX = (rand() % 17)+1;		//random from 1-18
	foodY = (rand() % 7)+1;			//random from 1-8
}

void ControlSnake(){
    int i;

    // Clear the tail by redrawing it as empty (white)
    GLCD_SetBackColor(White);
    GLCD_DisplayString(snake[snakeSize-1].y, snake[snakeSize-1].x, __FI, " ");  // Erase the last segment (tail)

    // Move the snake body by shifting its segments forward
    for(i = snakeSize - 1; i > 0; i--){
        snake[i] = snake[i-1];  // Shift body segments forward
    }

    // Move the head based on joystick direction
    if(snakeStickDir != 0){
        prevDir = snakeStickDir;
    }

    switch(prevDir){
        case JOYSTICK_UP: 
            snake[0].y -= 1; 
            break;
        case JOYSTICK_DOWN: 
            snake[0].y += 1; 
            break;
        case JOYSTICK_LEFT: 
            snake[0].x -= 1; 
            break;
        case JOYSTICK_RIGHT: 
            snake[0].x += 1; 
            break;
        case JOYSTICK_CENTER:
            snake[0].x += 1;
            snake[0].y += 1;
            break;
        case 0: 
            snake[0].y -= 1; 
            break;
    }

    // Check for collisions with walls or itself
    if(snake[0].x <= 0 || snake[0].x >= BOARD_WIDTH-1 || snake[0].y <= 0 || snake[0].y >= BOARD_HEIGHT-1){
        isAlive = false;
        return;
    }

    for(i = 1; i < snakeSize; i++){
        if(snake[0].x == snake[i].x && snake[0].y == snake[i].y){
            isAlive = false;
            return;
        }
    }

    // Check if the snake eats the food
    if(snake[0].x == foodX && snake[0].y == foodY){
        score++;
        snakeSize++;

        // Extend the snake (new tail at the position of the previous tail)
        snake[snakeSize-1].x = snake[snakeSize-2].x;
        snake[snakeSize-1].y = snake[snakeSize-2].y;

        ControlFood();
    }
}

void RunSnakeGame(){
	
	//Game Initialization
	InitSnakeGame();
	DrawGame();
	
	//Game Execution
	while(isAlive == true){
		snakeStickDir = Joystick_GetState();
		
		ControlSnake();	
		DrawGame();	
		
		Delay(150);  // controls game speed
	}
	
	//Game Over
	GLCD_SetTextColor(Red);
	DrawGame();
	GLCD_SetTextColor(Red);
	GLCD_DisplayString(4, 5, __FI, " GAME OVER ");
	GLCD_DisplayString(5, 6, __FI, (unsigned char*)scoreStr);
	
	//Delay for 2 seconds
	Delay(2000);
	InitSnakeMenu();
	
	GLCD_SetTextColor(Blue);
}

