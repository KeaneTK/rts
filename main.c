#include "stm32f4xx.h"                  // Device header
#include "inits.h"
#include "stm32f4xx_tim.h"              // Keil::Device:StdPeriph Drivers:TIM
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"   

int timerCurr = 70;
int timerTotal = 60;
int loopCounter = 5;
int ledCounter = 0;
int ledCurr;
int buttonPresses = 0;
int clickMode = 0;
int coffeeSelection = 0;


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

void initCountdown(int time, uint32_t led) {
	ledCurr = led;
	timerTotal = time;
	timerCurr = 0;
	clickMode = 1;
	TIM_Cmd(TIM2, ENABLE);
}

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
					case 1: initCountdown(40, GPIO_Pin_13); break;
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
			 }
		}
		TIM_Cmd(TIM5, DISABLE);
	}
}

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

//Initial startup animation
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

//This handles the blinking and audio
void TIM2_IRQHandler() {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		if (timerCurr < timerTotal) {
			GPIO_ToggleBits(GPIOD, ledCurr);
			timerCurr++;
		} else {
			TIM_Cmd(TIM2, DISABLE);
			//play sound
		}
	}
}

void EXTI0_IRQHandler() {
	if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
		buttonPresses++;
		EXTI_ClearITPendingBit(EXTI_Line0);
		TIM_Cmd(TIM5, ENABLE);
	}
}

int main()
{	
	int restart = 1;
	InitButtons();
	InitLeds();
	InitTimers();
	EnableTimerInterrupt();
	InitEXTI();
	EnableEXTIInterrupt();
	loopCounter = 0;
	
	while(1){
		//this is to make sure the main program runs after the startup animation is done
		if (loopCounter == 4) {
			selector(coffeeSelection);
			loopCounter = 5;
		}
	}
}
