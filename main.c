#include "stm32f4xx.h"                  // Device header
#include "inits.h"

///////////////////////////////////////////////////////////////////////Try to fiddle with TIM5 to get a second for each led
int timerCurr = 70;
int timerTotal = 60;
int loopCounter = 5;
int ledCounter = 0;
int ledCurr;
int buttonPresses = 0;
int clickMode = 0;
int coffeeSelection = 0;


int selector (int counter) {
	if (counter > 3) {//roll this into the click code
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

//Possibly turn TIM5 on and off using TIM_Cmd(TIM5, DISABLE);
void click() {
	int buttonValue = 0;
	if (buttonPresses > 0) {
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1 ) {
		buttonPresses = 0;
		buttonValue = 3;
	} else if (buttonPresses == 1) {
		buttonPresses = 0;
		buttonValue = 1;
	} else if (buttonPresses > 1) {
		buttonPresses = 0;
		buttonValue = 2;
	}
	if (clickMode == 0) {
		if (buttonValue == 1) {
			coffeeSelection++;
			coffeeSelection = selector(coffeeSelection);
		} else if (buttonValue == 2) {
			switch (coffeeSelection) {
				case 0: ledCurr = GPIO_Pin_12; timerTotal = 3; clickMode = 1; timerCurr = 0; break;
				case 1: ledCurr = GPIO_Pin_13; timerTotal = 4; clickMode = 1; timerCurr = 0; break;
				case 2: ledCurr = GPIO_Pin_14; timerTotal = 5; clickMode = 1; timerCurr = 0; break;
				case 3: ledCurr = GPIO_Pin_15; timerTotal = 10; clickMode = 1; timerCurr = 0; break;
			}
		}
	} else if (clickMode == 1) {
		 if (buttonValue == 1) {
			 timerCurr = 0;
		 } else if (buttonValue == 2) {
			 timerCurr = 100;
			 clickMode = 0;
		 }
	 }
	}
}

//Detects single and double clicks
//Returns one of three values
// 0 - No click detected
// 1 - A single click
// 2 - A double click
/******************************************************
void click() {
	uint8_t last_pin;
	uint8_t button_pin;
	uint32_t curTicks;
	int value = 0;
	int zeroes = 0;
	int one = 0;
	
	curTicks = msTicks;
	
	//Need some leeway in detecting the first click
	while ((msTicks - curTicks) < 100) {
		button_pin = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
		if (button_pin == 1) {
			one = 1;
		}
	}
	//If there is a single click, start to detect for a second click
	if (one == 1) {
		button_pin = one;
		while ((msTicks - curTicks) < 650) {
			last_pin = button_pin;
			button_pin = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
			//Make sure there is some zeroes in between each click to prevent 'scrubbing'
			if (button_pin == 0) {
				zeroes++;
			}
			//Double click - Make sure there was a second click and a number of zeroes
			if (last_pin == 0 && button_pin == 1 && zeroes > 150) {
				value = 2;
			}
		}
		//Single Click - If there was one click, but not two
		if (value != 2) {
			value = 1;
		}
	}
	
	return value;
}
********************************************/
//Initial startup animation
void TIM5_IRQHandler() {
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
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
			}
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
		} else if (timerCurr == timerTotal) {
			timerCurr = 60;
			//play sound
		}
	}
}

void EXTI0_IRQHandler() {
	if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
		buttonPresses++;
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}



/*
void blink(int time, int pin) {
	int clickValue = 0;
	int counter = 0;
	while (counter < time) {
		int timerValue = TIM_GetCounter(TIM2);
		if (timerValue == 0) {
			GPIO_SetBits(GPIOD, pin);
		} else if (timerValue == 125) {
			GPIO_ResetBits(GPIOD, pin);
		}
		
		clickValue = click();
		if (clickValue == 0) {
			Delay(750);
		} else if (clickValue == 1) {
			counter = -1;
		} else if (clickValue == 2) {
			counter = time;
		}
		counter++;
	}
	//need to play sound
}
*/

int main()
{	
	int coffeeSelect = 0;
	uint32_t ledExpresso = GPIO_Pin_12;
	uint32_t ledLatte = GPIO_Pin_13;
	uint32_t ledMocha = GPIO_Pin_14;
	uint32_t ledDecaf = GPIO_Pin_15;
	/*int timeExpresso = 30;
	int timeLatte = 40;
	int timeMocha = 45;
	int timeDecaf = 35;*/
	int timeExpresso = 3;
	int timeLatte = 4;
	int timeMocha = 5;
	int timeDecaf = 6;
	int clickValue = 0;
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
			if (restart == 1 || timerCurr == 100) {
				selector(coffeeSelection);
				restart = 0;
				timerCurr = 99;
			}
			/*
			if (timerCurr > timerTotal && timerCurr != 60) {
				ledCurr = ledMocha;
				timerTotal = 10;//timeMocha;
				timerCurr = 0;
			}*/
		}
		/*
		if (timerCurr > timerTotal) {
			
			if (clickValue == 1) {
				coffeeSelect++;
				coffeeSelect = selector(coffeeSelect, ledExpresso, ledLatte, ledMocha, ledDecaf);
			} else if (clickValue == 2) {
				
			}
		} else {
			
		}
		clickValue = click();*/
	}
}
//one second timer 83 prescaler, 99999 period, 1 hz
