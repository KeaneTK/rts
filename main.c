#include "stm32f4xx.h"                  // Device header

TIM_TimeBaseInitTypeDef timer_InitStructure;

volatile uint32_t msTicks;                      // counts 1ms timeTicks      
int timerCurr = 60;
int timerTotal = 0;
int ledCurr;

void InitTimers() {
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
      //(timer base clock / (prescaler + 1) ) / (period + 1) = the hertz or ticks per second
  timer_InitStructure.TIM_Prescaler = 40000;
  timer_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;//UP = 0++, Down = x--
  timer_InitStructure.TIM_Period = 500;//number of ticks, before it resets itself
  timer_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_InitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &timer_InitStructure);
  TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	//4.19 hertz or 4 ticks per second
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
      //(timer base clock / (prescaler + 1) ) / (period + 1) = the hertz or ticks per second
  timer_InitStructure.TIM_Prescaler = 40000;
  timer_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;//UP = 0++, Down = x--
  timer_InitStructure.TIM_Period = 500;//number of ticks, before it resets itself
  timer_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_InitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM5, &timer_InitStructure);
  TIM_Cmd(TIM5, ENABLE);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
}

void InitLeds() {
	GPIO_InitTypeDef GPIO_Initstructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); // activate RCC_AHB1 clock LED 

	//LED Configuration
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_OUT;//output
	GPIO_Initstructure.GPIO_OType = GPIO_OType_PP;//operating type, PUSHPULL or opendrain (PP = both transistors)
	GPIO_Initstructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//PuPd = pull up pull down, triggers events
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;//enabling the four pins from the block diagram
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;//define the speed
	GPIO_Init(GPIOD, &GPIO_Initstructure);//push the change
}

void InitButtons() {
	GPIO_InitTypeDef GPIO_Initstructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // Button
	//Button Configuration
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_IN;//input
	GPIO_Initstructure.GPIO_OType = GPIO_OType_PP;  
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_0;//blue button
	GPIO_Initstructure.GPIO_PuPd = GPIO_PuPd_DOWN;//down event
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_Initstructure);//push the change
}

void EnableTimerInterrupt() {
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
	nvicStructure.NVIC_IRQChannel = TIM5_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
}
/*
//Copied from the included Blinky schedule
void Delay (uint32_t dlyTicks) {                                              
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}

//Copied from the included Blinky schedule
void SysTick_Handler(void) {
  msTicks++;
}*/

//Detects single and double clicks
//Returns one of three values
// 0 - No click detected
// 1 - A single click
// 2 - A double click
int click() {
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

//Initial startup animation
void startSpinner() {
	int timerValue = TIM_GetCounter(TIM5);
	
	int counter = 0;
	for (counter = 0; counter < 3; counter++) {
		switch(timerValue) {
			case 125:
				GPIO_ResetBits(GPIOD, GPIO_Pin_14);
				GPIO_SetBits(GPIOD, GPIO_Pin_12);
				break;
			case 250:
				GPIO_ResetBits(GPIOD, GPIO_Pin_15);
				GPIO_SetBits(GPIOD, GPIO_Pin_13);
				break;
			case 375:
				GPIO_ResetBits(GPIOD, GPIO_Pin_12);
				GPIO_SetBits(GPIOD, GPIO_Pin_14);
				break;
			case 500:
				GPIO_ResetBits(GPIOD, GPIO_Pin_13);
				GPIO_SetBits(GPIOD, GPIO_Pin_15);
				break;
		}
	}
	GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
}

//This handles the blinking and audio
void TIM2_IRQHandler() {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		if (timerCurr < timerTotal) {
			GPIO_ToggleBits(GPIOD, ledCurr);
		} else if (timerCurr == timerTotal) {
			timerCurr = 60;
			//play sound
		}
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
int selector (int counter, uint32_t pinExpresso, uint32_t pinLatte, uint32_t pinMocha, uint32_t pinDecaf) {
	if (counter > 3) {
		counter = 0;
	}
	GPIO_ResetBits(GPIOD, pinExpresso | pinLatte | pinMocha | pinDecaf);
	switch (counter) {
		case 0:
			GPIO_SetBits(GPIOD, pinExpresso);
			break;
		case 1:
			GPIO_SetBits(GPIOD, pinLatte);
			break;
		case 2:
			GPIO_SetBits(GPIOD, pinMocha);
			break;
		case 3:
			GPIO_SetBits(GPIOD, pinDecaf);
			break;
	}
	return counter;
}

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
	InitButtons();
	InitLeds();
	InitTimers();
	EnableTimerInterrupt();
	
	/*SystemCoreClockUpdate();                      //	Get Core Clock Frequency   
  if (SysTick_Config(SystemCoreClock / 1000)) { // SysTick 1 msec interrupts  
    while (1);                                  // Capture error              
  }*/
			
	startSpinner();
	while(1){
		selector(coffeeSelect, ledExpresso, ledLatte, ledMocha, ledDecaf);

		clickValue = click();
		
		if (clickValue == 1) {
			coffeeSelect++;
			coffeeSelect = selector(coffeeSelect, ledExpresso, ledLatte, ledMocha, ledDecaf);
		} else if (clickValue == 2) {
			switch (coffeeSelect) {
				case 0: ledCurr = ledExpresso; timerTotal = timeExpresso; timerCurr = 0; /*blink(timeExpresso, ledExpresso); */break;
				case 1: ledCurr = ledLatte; timerTotal = timeLatte; timerCurr = 0;/*blink(timeLatte, ledLatte); */break;
				case 2: ledCurr = ledMocha; timerTotal = timeMocha; timerCurr = 0;/*blink(timeMocha, ledMocha); */break;
				case 3: ledCurr = ledDecaf; timerTotal = timeDecaf; timerCurr = 0;/*blink(timeDecaf, ledDecaf); */break;
			}
		}
	}
}
