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
	int counter;
	counter = 0;
	while (counter < time) {//insert click, if the button_pin is single, reset counter	//	if the button_pin is double, break and rerun selector
		GPIO_SetBits(GPIOD, pin);
		Delay(250);
		GPIO_ResetBits(GPIOD, pin);
		Delay(750);
		counter++;
	}
	//need to play sound
}

int click() {
	uint8_t button_pin;	
	uint32_t curTicks;
	int firstOne = 0;
	int secondOne = 0;
	int firstZero = 0;
	int total = 0;
	int value = 0;
	
  curTicks = msTicks;
  while ((msTicks - curTicks) < 750) {
		button_pin = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
		if (firstZero > 0 && button_pin == 1) {
				secondOne++;
		}	else if (button_pin == 1) {
				firstOne++;
		} else if (firstOne > 0 && button_pin == 0) {
				firstZero++;
		}
		total++;
	}
	total = total / 2;
	if (firstOne < total && firstOne > 0) {
			value = 1;
	} else if (firstOne > 0 && firstZero > 0 && secondOne > 1000) {
			value = 2;
	} else if (firstOne > total) {
			value = 3;
	}
	return value;

	//another version would be not to start the timer until the first click
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
	uint8_t button_pin;
	int coffeeSelect = 0;
	uint32_t ledExpresso = GPIO_Pin_12;
	uint32_t ledLatte = GPIO_Pin_13;
	uint32_t ledMocha = GPIO_Pin_14;
	uint32_t ledDecaf = GPIO_Pin_15;
	int timeExpresso = 30000;
	int timeLatte = 40000;
	int timeMocha = 45000;
	int timeDecaf = 35000;
	
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
		selector(coffeeSelect, ledExpresso, ledLatte, ledMocha, ledDecaf);//need to detect double click or long press to initiate timer.
		timeExpresso = click();
		timeExpresso = timeMocha * timeExpresso;
		/*
		button_pin = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
		
		if (button_pin) {
			Delay(200); // required to not overload the ++ statement below.  The user pressing the button will trigger multiple rounds.
			coffeeSelect++;
			coffeeSelect = selector(coffeeSelect, ledExpresso, ledLatte, ledMocha, ledDecaf);
		}
		
		*/
		/*
		first = msTicks;
		button_pin = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);//get data from the blue button
		if (button_pin){
			GPIO_SetBits(GPIOD, GPIO_Pin_12);
			Delay(time1);
			GPIO_ResetBits(GPIOD, GPIO_Pin_12);
			GPIO_SetBits(GPIOD, GPIO_Pin_13);
			Delay(time2);
			GPIO_ResetBits(GPIOD, GPIO_Pin_13);
			GPIO_SetBits(GPIOD, GPIO_Pin_14);
			Delay(time3);
			GPIO_ResetBits(GPIOD, GPIO_Pin_14);
			GPIO_SetBits(GPIOD, GPIO_Pin_15);
			Delay(time4);
			GPIO_ResetBits(GPIOD, GPIO_Pin_15);
			last = msTicks;
			difference = last - first;
			difference = difference * 1.1;
		}*/
	}
}//possibly use gyro for selecting the different cup sizes, if everything else works in time


/*
#include <stdio.h>
#include "STM32F4xx.h"
#include "LED.h"
#include "Keyboard.h"



//----------------------------------------------------------------------------
 // SysTick_Handler
 //----------------------------------------------------------------------------


//----------------------------------------------------------------------------
  //delays number of tick Systicks (happens every 1 ms)
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
  //Main function
 //----------------------------------------------------------------------------
int main (void) {
  int32_t led_num = LED_Num();
  int32_t num = -1; 
  int32_t dir =  1;
 uint32_t btns = 0;



  LED_Initialize();
  Keyboard_Initialize();

  while(1) {                                    // Loop forever               
    btns = Keyboard_GetKeys();                  // Read button states         

    if (btns != (1UL << 0)) {
      // Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...  
      num += dir;
      if (num == led_num) { dir = -1; num =  led_num-1; } 
      else if   (num < 0) { dir =  1; num =  0;         }

      LED_On (num);
      Delay( 50);                               // Delay 50ms                 
      LED_Off(num);
      Delay(200);                               // Delay 200ms                
    }
    else {
      LED_Out (0x0F);
      Delay(10);                                // Delay 10ms                 
    }
  }
}
*/
