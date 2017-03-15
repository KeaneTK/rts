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
#define TIME_ESPRESSO 5 
#define TIME_LATTE 40
#define TIME_MOCHA 45
#define TIME_CAPPACCINO 10

int totalTime1 = 0;
int led_coffee1;
int totalTime2 = 0;
int led_coffee2;
int totalTime3 = 0;
int led_coffee3;
void vTimer1Callback( TimerHandle_t xTimer );
void vTimer2Callback( TimerHandle_t xTimer );
void vTimer3Callback( TimerHandle_t xTimer );
void vTimerButtonCallback( TimerHandle_t xTimer);
void secondTask(void* ptr);
void buttonPressTask(void * ptr);
TimerHandle_t coffeeTimer1; 
TimerHandle_t coffeeTimer2; 
TimerHandle_t coffeeTimer3; 
TimerHandle_t buttonTimer;
int test = 0;
int flagTimer1Used = 0;
int flagTimer2Used = 0;
int flagTimer3Used = 0;

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

	STM_EVAL_PBInit( BUTTON_USER, BUTTON_MODE_EXTI );
	
	xTaskCreate( secondTask, (const char*)"Mocha", STACK_SIZE_MIN, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( buttonPressTask, (const char*)"Button", STACK_SIZE_MIN, NULL, 5, NULL);
	
	//params(char* not used, pdMS_TO_TICKS(#numberOfMilliseconds), true if you want the timer to repeat, can be used to store int values, function called after timer stops)
	coffeeTimer1 = xTimerCreate("timerCoffee1", pdMS_TO_TICKS(1000), pdTRUE, (void *) 0, vTimer1Callback);
	coffeeTimer2 = xTimerCreate("timerCoffee2", pdMS_TO_TICKS(1000), pdTRUE, (void *) 0, vTimer2Callback);
	coffeeTimer3 = xTimerCreate("timerCoffee3", pdMS_TO_TICKS(1000), pdTRUE, (void *) 0, vTimer3Callback);
	
	buttonTimer = xTimerCreate("timerButton", pdMS_TO_TICKS(1), pdTRUE, (void *) 0, vTimerButtonCallback);
	totalTime1 = TIME_CAPPACCINO;
	led_coffee1 = LED_GREEN;
	//xTimerStart(coffeeTimer1, 0); 
	
	vTaskStartScheduler();
	for(;;) {
	} 
}

//maybe have four different tasks/timers for the four different coffees

void secondTask(void * ptr) {
	for(;;) {
		if (test >= TIME_CAPPACCINO) {
			test = 0;
			led_coffee1 = LED_ORANGE;
			totalTime1 = TIME_ESPRESSO;
			xTimerReset(coffeeTimer1, 0);
		}
	}
}

void buttonPressTask(void * ptr) {
	for(;;) {
		
	}
}

void vTimer1Callback( TimerHandle_t xTimer ) {
	uint32_t ulCount;

	ulCount = ( uint32_t ) pvTimerGetTimerID( xTimer );

	ulCount++;
	test++;
	if( ulCount >= totalTime1 ) {
		STM_EVAL_LEDOn(led_coffee1);
		xTimerStop( xTimer, 0 );
		vTimerSetTimerID( xTimer, ( void * ) 0 );
	}
	else { 
		STM_EVAL_LEDToggle(led_coffee1);
		vTimerSetTimerID( xTimer, ( void * ) ulCount );
	}
}
/*xTaskCreate( prvButtonTestTask, "BtnTest", configMINIMAL_STACK_SIZE, ( void * ) NULL, tskIDLE_PRIORITY, NULL );*/

// The variable that is incremented by the task synchronised with the button
//interrupt. 
/*volatile unsigned long ulButtonPressCounts = 0UL;*/

/*
static void prvButtonTestTask( void *pvParameters )
{
	configASSERT( xTestSemaphore );

	// This is the task used as an example of how to synchronise a task with
	//an interrupt.  Each time the button interrupt gives the semaphore, this task
	//will unblock, increment its execution counter, then return to block
	//again. 

	// Take the semaphore before started to ensure it is in the correct
	//state. 
	xSemaphoreTake( xTestSemaphore, mainDONT_BLOCK );

	for( ;; )
	{
		xSemaphoreTake( xTestSemaphore, portMAX_DELAY );
		ulButtonPressCounts++;
	}
}*/

/*
void EXTI9_5_IRQHandler(void)
{
long lHigherPriorityTaskWoken = pdFALSE;

	// Only line 6 is enabled, so there is no need to test which line generated
	//the interrupt. 
	EXTI_ClearITPendingBit( EXTI_Line6 );

	// This interrupt does nothing more than demonstrate how to synchronise a
	//task with an interrupt.  First the handler releases a semaphore.
	//lHigherPriorityTaskWoken has been initialised to zero. 
	xSemaphoreGiveFromISR( xTestSemaphore, &lHigherPriorityTaskWoken );

	// If there was a task that was blocked on the semaphore, and giving the
	//semaphore caused the task to unblock, and the unblocked task has a priority
	//higher than the currently executing task (the task that this interrupt
	//interrupted), then lHigherPriorityTaskWoken will have been set to pdTRUE.
	//Passing pdTRUE into the following macro call will cause this interrupt to
	//return directly to the unblocked, higher priority, task. 
	portEND_SWITCHING_ISR( lHigherPriorityTaskWoken );
}*/