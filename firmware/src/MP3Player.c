#include "MP3Player.h"
#include <stdio.h>
#include "LPC17xx.h"                      
#include "GLCD.h"
#include "Board_Joystick.h"
#include "type.h"
#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbaudio.h"

#define __FI        1                      /* Font index 16x24               */

//Control to go back
volatile AppMode *curMode;

//use joystick
uint32_t audioStickDir;

volatile uint8_t joystickCenterPressed = 0; // Flag set when CENTER pressed

extern  void SystemClockUpdate(void);
extern uint32_t SystemFrequency;  
uint8_t  Mute;                                 /* Mute State */
uint32_t Volume;                               /* Volume Level */

//Memory Buffers to control info and audio data
#if USB_DMA
	uint32_t *InfoBuf = (uint32_t *)(DMA_BUF_ADR);
	short *DataBuf = (short *)(DMA_BUF_ADR + 4*P_C);
#else
	uint32_t InfoBuf[P_C];
	short DataBuf[B_S];                         /* Data Buffer */
#endif

//Pointers to the position inside DataBuf
uint16_t  DataOut;                              /* Data Out Index */
uint16_t  DataIn;                               /* Data In Index */

uint8_t   DataRun;                              /* Data Stream Run State */
uint16_t  PotVal;                               /* Potenciometer Value */
uint32_t  VUM;                                  /* Volume Unit Meter */
uint32_t  Tick;                                 /* Time Tick */

//Delays in ms
static void Delay(unsigned int ms) {
    volatile unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 9080; j++) {
        }
    }
}

/*
 * Get Potenciometer Value
 */

void get_potval (void) {
  uint32_t val;

  LPC_ADC->CR |= 0x01000000;              /* Start A/D Conversion */
  do {
    val = LPC_ADC->GDR;                   /* Read A/D Data Register */
  } while ((val & 0x80000000) == 0);      /* Wait for end of A/D Conversion */
  LPC_ADC->CR &= ~0x01000000;             /* Stop A/D Conversion */
  PotVal = ((val >> 8) & 0xF8) +          /* Extract Potenciometer Value */
           ((val >> 7) & 0x08);
}

/*
 * Timer Counter 0 Interrupt Service Routine
 *   executed each 31.25us (32kHz frequency).
	Send audio samples to the DAC and handle 
	volume/VU meter with the potentiometer.
 */

void TIMER0_IRQHandler(void) 
{
  long  val;
  uint32_t cnt;
	
	//Run when audio is playing
  if (DataRun) {                            /* Data Stream is running */
    val = DataBuf[DataOut];                 /* Get Audio Sample */
    cnt = (DataIn - DataOut) & (B_S - 1);   /* Buffer Data Count */
    if (cnt == (B_S - P_C*P_S)) {           /* Too much Data in Buffer */
      DataOut++;                            /* Skip one Sample */
    }
    if (cnt > (P_C*P_S)) {                  /* Still enough Data in Buffer */
      DataOut++;                            /* Update Data Out Index */
    }
    DataOut &= B_S - 1;                     /* Adjust Buffer Out Index */
    if (val < 0) VUM -= val;                /* Accumulate Neg Value */
    else         VUM += val;                /* Accumulate Pos Value */
    val  *= Volume;                         /* Apply Volume Level */
    val >>= 16;                             /* Adjust Value */
    val  += 0x8000;                         /* Add Bias */
    val  &= 0xFFFF;                         /* Mask Value */
  } else {
    val = 0x8000;                           /* DAC Middle Point */
  }

	//Silence the output if muted
  if (Mute) {
    val = 0x8000;                           /* DAC Middle Point */
  }

  LPC_DAC->CR = val & 0xFFC0;             /* Set Speaker Output */
	
	//Every 1024 ticks, update the VU meter and read volume
  if ((Tick++ & 0x03FF) == 0) {             /* On every 1024th Tick */
    get_potval();                           /* Read Potentiometer Value */
    if (VolCur == 0x8000) {                 /* Check for Minimum Level */
      Volume = 0;                           /* No Sound */
    } else {
      Volume = VolCur * PotVal;             /* Chained Volume Level */
    }
    val = VUM >> 20;                        /* Scale Accumulated Value */
    VUM = 0;                                /* Clear VUM */
    if (val > 7) val = 7;                   /* Limit Value */
  }

	audioStickDir = Joystick_GetState();
		
	//Go back to the main menu
	if (audioStickDir == JOYSTICK_CENTER){
			*curMode = MODE_MENU;
		
			//Disable this interrupt
			NVIC_DisableIRQ(TIMER0_IRQn);
			LPC_TIM0->TCR = 0;								//Stop counter and set its value to 0.
		
			//Disable USB interrupt
			NVIC_DisableIRQ(USB_IRQn);
			USB_Connect(FALSE);
			LPC_USB->DevIntEn = 0;							//Disable interrupt enabler for the USB peripheral.
	}
		
	//Clear interrut flag to allow next interrupt
  LPC_TIM0->IR = 1;
}

void RunMP3Player(uint32_t stickDir, uint32_t *prevStickDir, AppMode *currentMode){
	volatile uint32_t pclkdiv, pclk;
	
	curMode = currentMode;
	
	GLCD_Clear(White);
	GLCD_DisplayString(2, 0, __FI, "Starting MP3 Player...");
	
	SystemClockUpdate();

	// ADC + DAC setup
	LPC_PINCON->PINSEL1 &=~((0x03<<18)|(0x03<<20));  
	LPC_PINCON->PINSEL1 |= ((0x01<<18)|(0x02<<20));

	LPC_SC->PCONP |= (1 << 12);
	LPC_ADC->CR = 0x00200E04;
	LPC_DAC->CR = 0x00008000;

	pclkdiv = (LPC_SC->PCLKSEL0 >> 2) & 0x03;
	switch (pclkdiv) {
			case 0x01: pclk = SystemFrequency; break;
			case 0x02: pclk = SystemFrequency/2; break;
			case 0x03: pclk = SystemFrequency/8; break;
			default:   pclk = SystemFrequency/4; break;
	}

	LPC_TIM0->MR0 = pclk/DATA_FREQ - 1;
	LPC_TIM0->MCR = 3;
	LPC_TIM0->TCR = 1;

	GLCD_DisplayString(5, 0, __FI, "USB Audio Connected");
	GLCD_DisplayString(7, 0, __FI, "Press CENTER to Exit");

	NVIC_EnableIRQ(TIMER0_IRQn);	
	USB_Init();
	USB_Connect(TRUE);
}

