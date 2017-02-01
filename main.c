#include "stm32f4xx.h"                  // Device header

volatile uint32_t msTicks;                      // counts 1ms timeTicks       

void Delay (uint32_t dlyTicks) {                                              
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}

void SysTick_Handler(void) {
  msTicks++;
}

int main()
{
	GPIO_InitTypeDef GPIO_Initstructure;//struct define elsewhere
	uint8_t button_pin;
	int time1 = 2000;
	int time2 = 3000;
	int time3 = 4000;
	int time4 = 5000;
	
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
		
	while(1){
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
		}
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
