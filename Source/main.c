//******************************************************************************
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "discoveryf4utils.h"
//******************************************************************************

//******************************************************************************
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "croutine.h"
#include "timers.h"
//******************************************************************************
#define TIME_ESPRESSO 30000
#define TIME_LATTE 40000
#define TIME_MOCHA 45000
#define TIME_CAPPACCINO 60000

int totalTime = 0;
int led_coffee;
void vTimerCallback( TimerHandle_t xTimer );
TimerHandle_t coffeeTimer; 
//void Delay(uint32_t val);

#define STACK_SIZE_MIN	128	/* usStackDepth	- the stack size DEFINED IN WORDS.*/



//******************************************************************************
int main(void)
{
	/*!< At this stage the microcontroller clock setting is already configured,
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f4xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f4xx.c file
	 */
	
	/*!< Most systems default to the wanted configuration, with the noticeable 
		exception of the STM32 driver library. If you are using an STM32 with 
		the STM32 driver library then ensure all the priority bits are assigned 
		to be preempt priority bits by calling 
		NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); before the RTOS is started.
	*/
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	STM_EVAL_LEDInit(LED_BLUE);
	STM_EVAL_LEDInit(LED_GREEN);
	STM_EVAL_LEDInit(LED_ORANGE);
	STM_EVAL_LEDInit(LED_RED);
	
//	xTaskCreate( vMocha, (const char*)"Mocha", 
//		STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );
	
	coffeeTimer = xTimerCreate("timerCoffee", pdMS_TO_TICKS(1), pdTRUE, (void *) 0, vTimerCallback);
	vTaskStartScheduler();
	
	totalTime = TIME_CAPPACCINO;
	xTimerStart(coffeeTimer, 0); 
	for(;;) { } 
}

//maybe have four different tasks/timers for the four different coffees

void vTimerCallback( TimerHandle_t xTimer ) {
	uint32_t ulCount;

	ulCount = ( uint32_t ) pvTimerGetTimerID( xTimer );

	ulCount++;
	if( ulCount >= totalTime ) {
		STM_EVAL_LEDOn(led_coffee);
		xTimerStop( xTimer, 0 );
		//vTimerSetTimerID( xTimer, ( void * ) 0 );
	}
	else { 
		STM_EVAL_LEDToggle(led_coffee);
		//vTimerSetTimerID( xTimer, ( void * ) ulCount );
	}
}

//******************************************************************************
