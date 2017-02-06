#include "stm32f4xx.h"                  // Device header

volatile uint32_t msTicks;                      // counts 1ms timeTicks      
int half = 80000;

void Delay (uint32_t dlyTicks) {                                              
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}

void SysTick_Handler(void) {
  msTicks++;
}

int click() {
	uint8_t last_pin;
	uint8_t button_pin;
	uint32_t curTicks;
	int value = 0;
	int zeroes = 0;
	curTicks = msTicks;
	
	button_pin = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
	if (button_pin == 1) {
		while ((msTicks - curTicks) < 750) {
			last_pin = button_pin;
			button_pin = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
			if (button_pin == 0) {
				zeroes++;
			}
			if (last_pin == 0 && button_pin == 1 && zeroes > 150) {
				value = 2;
			}
		}
		if (value != 2) {
			value = 1;
		}
	}
	
	return value;
}

void startSpinner() {
	int counter = 0;
	while (counter < 3) {
		GPIO_ResetBits(GPIOD, GPIO_Pin_14);
		GPIO_SetBits(GPIOD, GPIO_Pin_12);
		Delay(250);
		GPIO_ResetBits(GPIOD, GPIO_Pin_15);
		GPIO_SetBits(GPIOD, GPIO_Pin_13);
		Delay(250);
		GPIO_ResetBits(GPIOD, GPIO_Pin_12);
		GPIO_SetBits(GPIOD, GPIO_Pin_14);
		Delay(250);
		GPIO_ResetBits(GPIOD, GPIO_Pin_13);
		GPIO_SetBits(GPIOD, GPIO_Pin_15);
		Delay(250);
		counter++;
	}
	GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
}

void blink(int time, int pin) {
	int clickValue = 0;
	int counter = 0;
	while (counter < time) {
		GPIO_SetBits(GPIOD, pin);
		Delay(250);
		GPIO_ResetBits(GPIOD, pin);
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
	GPIO_InitTypeDef GPIO_Initstructure;//struct define elsewhere
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
	
	SystemCoreClockUpdate();                      //	Get Core Clock Frequency   
  if (SysTick_Config(SystemCoreClock / 1000)) { // SysTick 1 msec interrupts  
    while (1);                                  // Capture error              
  }
	
	//D refers to port D, the LEDs are listed as belonging to PD12-15 (D referring to GPIO D), same for A and buttons
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); // activate RCC_AHB1 clock LED 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); // Button
	
	//LED Configuration
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_OUT;//output
	GPIO_Initstructure.GPIO_OType = GPIO_OType_PP;//operating type, PUSHPULL or opendrain (PP = both transistors)
	GPIO_Initstructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//PuPd = pull up pull down, triggers events
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;//enabling the four pins from the block diagram
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;//define the speed
	GPIO_Init(GPIOD, &GPIO_Initstructure);//push the change
	
	//Button Configuration
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_IN;//input
	GPIO_Initstructure.GPIO_OType = GPIO_OType_PP;  
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_0;//blue button
	GPIO_Initstructure.GPIO_PuPd = GPIO_PuPd_DOWN;//down event
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_Initstructure);//push the change
		
	startSpinner();
	while(1){
		selector(coffeeSelect, ledExpresso, ledLatte, ledMocha, ledDecaf);

		clickValue = click();
		
		if (clickValue == 1) {
			coffeeSelect++;
			coffeeSelect = selector(coffeeSelect, ledExpresso, ledLatte, ledMocha, ledDecaf);
		} else if (clickValue == 2) {
			switch (coffeeSelect) {
				case 0: blink(timeExpresso, ledExpresso); break;
				case 1: blink(timeLatte, ledLatte); break;
				case 2: blink(timeMocha, ledMocha); break;
				case 3: blink(timeDecaf, ledDecaf); break;
			}
		}
	}
}
