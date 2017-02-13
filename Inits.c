#include "inits.h"
#include "stm32f4xx.h" 
#include "stm32f4xx_exti.h"             // Keil::Device:StdPeriph Drivers:EXTI

void InitEXTI() {
	EXTI_InitTypeDef EXTI_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStructure);
}

void EnableEXTIInterrupt() {
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = EXTI0_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelSubPriority = 0;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
}

void InitTimers() {                 // Device header
	TIM_TimeBaseInitTypeDef timer_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
      //(timer base clock / (prescaler + 1) ) / (period + 1) = the hertz or ticks per second
  timer_InitStructure.TIM_Prescaler = 14000-1;
  timer_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;//UP = 0++, Down = x--
  timer_InitStructure.TIM_Period = 2000-1;//number of ticks, before it resets itself
  timer_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_InitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &timer_InitStructure);
  TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  timer_InitStructure.TIM_Prescaler = 21000-1;
  timer_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
  timer_InitStructure.TIM_Period = 500-1;
  timer_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  timer_InitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM5, &timer_InitStructure);
  TIM_Cmd(TIM5, ENABLE);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	
}

void InitLeds() {
	GPIO_InitTypeDef GPIO_Initstructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 

	//LED Configuration
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Initstructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Initstructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_Initstructure);
}

void InitButtons() {
	GPIO_InitTypeDef GPIO_Initstructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 
	//Button Configuration
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Initstructure.GPIO_OType = GPIO_OType_PP;  
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Initstructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_Initstructure);
}

void EnableTimerInterrupt() {
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 1;
	nvicStructure.NVIC_IRQChannelSubPriority = 2;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
	
	nvicStructure.NVIC_IRQChannel = TIM5_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 1;
	nvicStructure.NVIC_IRQChannelSubPriority = 1;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
}
