/*
	Sound code was taken from the Sound.zip file provided.
	
*/


#include "inits.h"
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f4xx_tim.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"             // Keil::Device:StdPeriph Drivers:EXTI
#include "codec.h"

//constants for brewing times
//	Program set in milliseconds, so multiply by 1000
#define TIME_ESPRESSO 30 * 1000
#define TIME_LATTE 40 * 1000
#define TIME_MOCHA 45 * 1000
#define TIME_CAPPACCINO 60 * 1000

//easier way to remember the colored LEDs
#define LED_GREEN GPIO_Pin_12
#define LED_ORANGE GPIO_Pin_13
#define LED_RED GPIO_Pin_14
#define LED_BLUE GPIO_Pin_15



int timer2_Elapsed = 0;
int timer5_Elapsed = 0;
int loopCounter = 0;
int ledCounter = 0;

//variables for tracking the current mode the program is in
int ledCurr;
int currBrewTime;
int brewingTimeElapsed = 0;
int programRunning = 0;
int coffeeSelection = 0;

//variables for determing single/double/long presses
int buttonPresses = 0;
int buttonHeld = 0;
int buttonHoldTime = 0;
int buttonOperationCompleted = 0;

//--------------sound variables---------------

#define SECONDS_TO_PLAY_SOUND 2	//how long to play the sound that indicates end of brewing

#define NOTEFREQUENCY 0.015		//frequency of saw wave: f0 = 0.5 * NOTEFREQUENCY * 48000 (=sample rate)
#define NOTEAMPLITUDE 500.0		//amplitude of the saw wave

double sawWave = 0.0;
float filteredSaw = 0.0;

typedef struct {
	float tabs[8];
	float params[8];
	uint8_t currIndex;
} fir_8;

volatile uint32_t sampleCounter = 0;
volatile int16_t sample = 0;

fir_8 filt;
void playSound(int);
void initFilter(fir_8* theFilter);

//--------------end sound variables-----------

void clickLong(void);
void clickSingle(void);
void clickDouble(void);

void blinkLed(void);
void initCountdown(void);
void finishedBrewing(void);


//Force change the LED color (reset/set) and store the current mode
//	is ONLY called by single clicks IF the program is not running
int selector (int counter) {
	if (counter > 3) {
		counter = 0;
	}
	GPIO_ResetBits(GPIOD, LED_GREEN | LED_ORANGE | LED_RED | LED_BLUE);
	switch (counter) {
		case 0:
			GPIO_SetBits(GPIOD, LED_GREEN);
			ledCurr = LED_GREEN;
			currBrewTime = TIME_ESPRESSO;
			break;
		case 1:
			GPIO_SetBits(GPIOD, LED_ORANGE);
			ledCurr = LED_ORANGE;
			currBrewTime = TIME_LATTE;
			break;
		case 2:
			GPIO_SetBits(GPIOD, LED_RED);
			ledCurr = LED_RED;
			currBrewTime = TIME_MOCHA;
			break;
		case 3:
			GPIO_SetBits(GPIOD, LED_BLUE);
			ledCurr = LED_BLUE;
			currBrewTime = TIME_CAPPACCINO;
			break;
	}
	return counter;
}

void blinkLed() {
	GPIO_ToggleBits(GPIOD, ledCurr);
}

//Enables the coffeeTimer with the proper values
void initCountdown() {
	brewingTimeElapsed = 0;
	programRunning = 1;
}

//resets variables when program is done
void finishedBrewing() {
	programRunning = 0;
	playSound(SECONDS_TO_PLAY_SOUND);
}

//start the program
void clickLong() {
	if (programRunning == 0) {		
		switch (coffeeSelection) {
			case 0: 
				initCountdown(); 
				break;
			case 1: 
				initCountdown(); 
				break;
			case 2: 
				initCountdown(); 
				break;
			case 3: 
				initCountdown(); 
				break;
		}	
	}
}

//Reset the program if its running
//	return to menu
void clickDouble() {
	if (programRunning == 1) {
		programRunning = 0;
		GPIO_SetBits(GPIOD, ledCurr);
	}

}

//If program is running:
//	Do nothing
//else:
//	Cycle LED
void clickSingle() {
	//Program is in selector mode
	if (programRunning == 0) {
		coffeeSelection++;
		coffeeSelection = selector(coffeeSelection);
	} else if (programRunning == 1) {
		//reset the brewing time
		brewingTimeElapsed = 0;
	}
}


//set to 1000 updates per second (1 kHz)
//	runs all the time 
void TIM5_IRQHandler() {
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
		timer5_Elapsed++;
		if (timer5_Elapsed >= (600 * 1000)) 
			timer5_Elapsed = 0;
		
		if (programRunning == 1) {
			//update the brewing time
			brewingTimeElapsed++;
			//only execute if a half second has passed (blink = on + off)
			if (timer5_Elapsed % 500 == 0) {
				//reset the counter to avoid integer overflow
				
				//check the status of the program
				if (brewingTimeElapsed >= currBrewTime) {
					//finished brewing, play a sound
					finishedBrewing();
				} else {
					//program is running and this is still brewing,
					//	therefore blink LED
					blinkLed();
				}
			}
		}
		
		
		
	}
}

//Timer handler for coffeeTimer blinking, audio, and determining durations of button pressse
//set to 2000 updates per second (2 kHz) (closest whole number to capture 1.5 seconds)
void TIM2_IRQHandler() {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		//count how long the timer has been enabled for
		timer2_Elapsed++;
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1) {
			//Count how long the button has been held down
			buttonHoldTime++;
		}

		//Cases to consider if a button has been pressed:
		//	Case 1: Button held for 1.5 seconds
		//		Do: long press
		//	Case 2: Button pressed once in >0.5 seconds
		//		Do: single click
		//	Case 3: Button pressed twice within 0.5 seconds
		//		Do: double click
		if (buttonPresses > 0 && buttonOperationCompleted == 0) {			
			if (buttonHoldTime >= 1500 && GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1) {	//Case 1
				buttonHoldTime = 0;
				buttonPresses = 0;
				buttonOperationCompleted = 1;
				clickLong();
			} else if (buttonPresses >= 2 && timer2_Elapsed <= 500) {												//Case 3
				buttonHoldTime = 0;
				buttonPresses = 0;					
				buttonOperationCompleted = 1;
				clickDouble();
			} else if (buttonPresses == 1 && timer2_Elapsed > 500 && GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0) {			//Case 2
				buttonHoldTime = 0;
				buttonPresses = 0;
				buttonOperationCompleted = 1;
				clickSingle();
			}
		}	
		
		if (buttonOperationCompleted == 1) {
				timer2_Elapsed = 0;
				buttonOperationCompleted = 0;
				TIM_Cmd(TIM2, DISABLE);
		}

	} 
}

//Button handler
void EXTI0_IRQHandler() {
	if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
		//tell the timer that the button is being held down
		//enable the timer if this is the first click
		if (timer2_Elapsed == 0) buttonPresses = 0;
		if (buttonPresses == 0) {
				TIM_Cmd(TIM2, ENABLE);
				buttonPresses = 1;
		} else { 
			//this could potentially be a double click, force a buffer between button presses
			if (timer2_Elapsed > 150 && buttonPresses == 1) {
				buttonPresses++;
			}
		}
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}


void initFilter(fir_8* theFilter)
{
	uint8_t i;

	theFilter->currIndex = 0;

	for (i=0; i<8; i++)
		theFilter->tabs[i] = 0.0;

	theFilter->params[0] = 0.01;
	theFilter->params[1] = 0.05;
	theFilter->params[2] = 0.12;
	theFilter->params[3] = 0.32;
	theFilter->params[4] = 0.32;
	theFilter->params[5] = 0.12;
	theFilter->params[6] = 0.05;
	theFilter->params[7] = 0.01;
}

// a very crude FIR lowpass filter
float updateFilter(fir_8* filt, float val)
{
	uint16_t valIndex;
	uint16_t paramIndex;
	float outval = 0.0;

	valIndex = filt->currIndex;
	filt->tabs[valIndex] = val;

	for (paramIndex=0; paramIndex<8; paramIndex++)
	{
		outval += (filt->params[paramIndex]) * (filt->tabs[(valIndex+paramIndex)&0x07]);
	}

	valIndex++;
	valIndex &= 0x07;

	filt->currIndex = valIndex;

	return outval;
}

void playSound(int duration) {
	sampleCounter = 0;
	   while(sampleCounter < 96000 * duration)
    {

    	if (SPI_I2S_GetFlagStatus(CODEC_I2S, SPI_I2S_FLAG_TXE))
    	{
    		SPI_I2S_SendData(CODEC_I2S, sample);

    		//only update on every second sample to insure that L & R ch. have the same sample value
    		if (sampleCounter & 0x00000001)
    		{
    			sawWave += NOTEFREQUENCY;
    			if (sawWave > 1.0)
    				sawWave -= 2.0;

    			filteredSaw = updateFilter(&filt, sawWave);
    			sample = (int16_t)(NOTEAMPLITUDE*filteredSaw);
    		}
    		sampleCounter++;
    	}

    }
}

void InitSound() {
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//enables GPIO clock for PortD
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOD, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	codec_init();
	codec_ctrl_init();

	I2S_Cmd(CODEC_I2S, ENABLE);

	initFilter(&filt);
}

int main()
{	
	InitButtons();
	InitLeds();
	InitTimers();
	EnableTimerInterrupt();
	InitEXTI();
	EnableEXTIInterrupt();
	
	//init sound variables
	InitSound();
	
	
	//Set the LED once
	selector(coffeeSelection);
	
	while(1){
			
	}
}
