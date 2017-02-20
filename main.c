#include "inits.h"
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f4xx_tim.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"             // Keil::Device:StdPeriph Drivers:EXTI
#include "codec.h"



//constants for brewing times
#define TIME_ESPRESSO 30
#define TIME_LATTE 40
#define TIME_MOCHA 45

//
int timerCurr = 0;
//
int timerTotal = 0;
//
int loopCounter = 0;
//
int ledCounter = 0;
//
int ledCurr;
//
int buttonPresses = 0;
//
int clickMode = 0;
//
int coffeeSelection = 0;

//--------------sound variables---------------

int x;

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
void playSound(void);
void initFilter(fir_8* theFilter);

//--------------end sound variables-----------


//Changes the coffee selection LED
int selector (int counter) {
	if (counter > 3) {
		counter = 0;
	}
	GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
	switch (counter) {
		case 0:
			GPIO_SetBits(GPIOD, GPIO_Pin_12);
			break;
		case 1:
			GPIO_SetBits(GPIOD, GPIO_Pin_13);
			break;
		case 2:
			GPIO_SetBits(GPIOD, GPIO_Pin_14);
			break;
		case 3:
			GPIO_SetBits(GPIOD, GPIO_Pin_15);
			break;
	}
	return counter;
}

//Enables the coffeeTimer with the proper values
void initCountdown(int time, uint32_t led) {
	ledCurr = led;
	timerTotal = time;
	timerCurr = 0;
	clickMode = 1;
	TIM_Cmd(TIM2, ENABLE);
}

//Detects single or double click, then, depending on the mode, activates the proper function
void click() {
	int buttonValue = 0;
	if (buttonPresses > 0) {
		if (buttonPresses == 1) {
			buttonPresses = 0;
			buttonValue = 1;
		} else if (buttonPresses > 1) {
			buttonPresses = 0;
			buttonValue = 2;
		}
		//Program is in selector mode
		if (clickMode == 0) {
			if (buttonValue == 1) {
				coffeeSelection++;
				coffeeSelection = selector(coffeeSelection);
			} else if (buttonValue == 2) {
				switch (coffeeSelection) {
					case 0: initCountdown(30, GPIO_Pin_12); break;
					case 1: initCountdown(6, GPIO_Pin_13); break;
					case 2: initCountdown(45, GPIO_Pin_14); break;
					case 3: initCountdown(35, GPIO_Pin_15); break;
				}
			}
		}
		//Program is in timer mode
		else if (clickMode == 1) {
			 if (buttonValue == 1) {
				 timerCurr = 0;
			 } else if (buttonValue == 2) {
				 TIM_Cmd(TIM2, DISABLE); 
				 clickMode = 0;
				 GPIO_SetBits(GPIOD, ledCurr);
			 }
		}
		TIM_Cmd(TIM5, DISABLE);
	}
}

//Startup animation spinner
void startup() {
	if (loopCounter < 3) {
		if (ledCounter == 0) {
			GPIO_ResetBits(GPIOD, GPIO_Pin_14);
			GPIO_SetBits(GPIOD, GPIO_Pin_12);
			ledCounter++;
		} else if (ledCounter == 1) {
			GPIO_ResetBits(GPIOD, GPIO_Pin_15);
			GPIO_SetBits(GPIOD, GPIO_Pin_13);
			ledCounter++;
		} else if (ledCounter == 2) {
			GPIO_ResetBits(GPIOD, GPIO_Pin_12);
			GPIO_SetBits(GPIOD, GPIO_Pin_14);
			ledCounter++;
		} else if (ledCounter == 3) {
			GPIO_ResetBits(GPIOD, GPIO_Pin_13);
			GPIO_SetBits(GPIOD, GPIO_Pin_15);
			ledCounter = 0;
			loopCounter++;
		}
	} else if (loopCounter == 3) {
		if (ledCounter == 0) {
			GPIO_ResetBits(GPIOD, GPIO_Pin_14);
			ledCounter++;
		} else if (ledCounter == 1) {
			GPIO_ResetBits(GPIOD, GPIO_Pin_15);
			ledCounter = 0;
			loopCounter++;
			TIM_Cmd(TIM5, DISABLE);
		}
	}
}

//Initial startup animation and handles click timing
void TIM5_IRQHandler() {
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
		if (loopCounter < 4) {
			startup();
		} else {
			click();
		}
	}
}

//Timer handler for coffeeTimer blinking and audio
void TIM2_IRQHandler() {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		if (timerCurr < timerTotal) {
			GPIO_ToggleBits(GPIOD, ledCurr);
			timerCurr++;
		} else {
			TIM_Cmd(TIM2, DISABLE);
			GPIO_SetBits(GPIOD, ledCurr);
			clickMode = 0;
			//play sound
		}
	}
}

//Button handler
void EXTI0_IRQHandler() {
	if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
		if (buttonPresses == 0) {
				TIM_Cmd(TIM5, ENABLE);
		}
		buttonPresses++;
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


void playSound() {
	sampleCounter = 0;
	   while(sampleCounter < 48000 + 1)
    {

    	if (SPI_I2S_GetFlagStatus(CODEC_I2S, SPI_I2S_FLAG_TXE))
    	{
    		SPI_I2S_SendData(CODEC_I2S, sample);

    		//only update on every second sample to insure that L & R ch. have the same sample value
    		if (sampleCounter & 0x00000001)
    		{
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
	for (x = 0; x < 5; x++) 
		playSound();

	while(1){
		//this is to make sure the main program runs after the startup animation is done, then start the selection loop
		if (loopCounter == 4) {
			selector(coffeeSelection);
			loopCounter = 5;
		}
	}
}
