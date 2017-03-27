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
#include "semphr.h"

//******************************************************************************


//***********************
//constants for how often to check if the selected led is turned on or not
//	so it can block when it is not in use
void vTurnOnSelectedLed(void* pvIgnored);
#define MS_SELECTED_LED_POLL_FREQUENCY 25
Led_TypeDef ledGlobalSelectedLed;
#define PRIORITY_LED_SELECTOR 10

#define LED_FOR_OPTION_NULL 				LED_ORANGE
#define LED_FOR_OPTION_MOCHA 				LED_GREEN
#define LED_FOR_OPTION_LATTE 				LED_BLUE
#define LED_FOR_OPTION_ESPRESSO 		LED_RED

#define LED_FOR_NULL_TO_MOCHA 			LED_GREEN
#define LED_FOR_MOCHA_TO_LATTE 			LED_BLUE
#define LED_FOR_LATTE_TO_ESPRESSO		LED_RED
#define LED_FOR_ESPRESSO_TO_NULL		LED_ORANGE


//***********************Variables for brewing
//	In milliseconds
//	Default values:
//		Mocha: 45 seconds
//		Latte: 40 seconds
//		Espresso: 30 seconds
#define TIME_MOCHA (5 * 1000)
#define TIME_LATTE (10 * 1000)
#define TIME_ESPRESSO (20 * 1000)


//How often the blinking task should CHECK if we should start blinking
//	Note: this is only to check the flag variable enable<Coffee>
//	Blinking tiself is a different frequency
//		This is the duration that the task will go to sleep on unsuccessful check
#define MS_IS_BREWING_CHECK_FREQUENCY 25
#define MS_BLINKING_FREQUENCY 500
void vTaskBlinkLed( void* pvLedToBlink);
//Give the blinking priority task less than the LED selector's priority
#define PRIORITY_LED_BLINKER (PRIORITY_LED_SELECTOR - 1)

void vMochaTimerCallback( TimerHandle_t xTimer );
void vLatteTimerCallback( TimerHandle_t xTimer );
void vEspressoTimerCallback( TimerHandle_t xTimer );

BaseType_t 		enableMocha;
BaseType_t 		enableLatte;
BaseType_t 		enableEspresso;

TimerHandle_t xTimerMocha;
TimerHandle_t xTimerLatte;
TimerHandle_t xTimerEspresso;

#define BREWING_START 1
#define BREWING_STOP 0

void vStartMocha(void);
void vStartLatte(void);
void vStartEspresso(void);


//***********************END variables for brewing

//***********************button related prototypes
void vClickSingle(void);
void vClickDouble(void);
void vClickLong(void);
//***********************END button related prototypes



void vTimerWaitCallback(TimerHandle_t xTimer);
void vTimerButtonCallback(TimerHandle_t xTimer);



#define mainDONT_BLOCK (0UL)
TimerHandle_t buttonTimer;
int buttonPressed;
int waitButton = 0;
TimerHandle_t waitTimer;
TimerHandle_t buttonTimer;

struct TIMER_DATA {
    int totalTime;
    int currTime;
    Led_TypeDef led_coffee;
    int flagUsed;
} typedef timer_data;

timer_data timerData1;
timer_data timerData2;
timer_data timerData3;
timer_data buttonData;

#define STACK_SIZE_MIN 128 /* usStackDepth	- the stack size DEFINED IN WORDS.*/

void prvButtonTestTask(void* pvParameters);
void prvTimer1Task(void* pvParameters);
void prvTimer2Task(void* pvParameters);
void prvTimer3Task(void* pvParameters);

SemaphoreHandle_t xButtonSemaphore = NULL;
SemaphoreHandle_t xTaskSemaphore = NULL;

int main(void) {
		Led_TypeDef mochaLed;
		Led_TypeDef latteLed;
		Led_TypeDef espressoLed;
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    STM_EVAL_LEDInit(LED_BLUE);
    STM_EVAL_LEDInit(LED_GREEN);
    STM_EVAL_LEDInit(LED_ORANGE);
    STM_EVAL_LEDInit(LED_RED);
	
		//create the task that enables the LED to be selected
	//LED priorities:
	//	Orange: 4
	//	Green: 	1
	//	Blue: 	2
	//	Red: 		3
		ledGlobalSelectedLed = LED_FOR_OPTION_NULL;
    xTaskCreate(vTurnOnSelectedLed, "Led Illuminator", STACK_SIZE_MIN, (void*)NULL, PRIORITY_LED_SELECTOR, NULL);
	
		//create the tasks that blink the LEDs when they are on
		mochaLed = LED_FOR_OPTION_MOCHA;
		latteLed = LED_FOR_OPTION_LATTE;
		espressoLed = LED_FOR_OPTION_ESPRESSO;
		xTaskCreate(vTaskBlinkLed, "Mocha Blinker", STACK_SIZE_MIN, (void*)&mochaLed, PRIORITY_LED_BLINKER, NULL);
		xTaskCreate(vTaskBlinkLed, "Latte Blinker", STACK_SIZE_MIN, (void*)&latteLed, PRIORITY_LED_BLINKER, NULL);
		xTaskCreate(vTaskBlinkLed, "Espresso Blinker", STACK_SIZE_MIN, (void*)&espressoLed, PRIORITY_LED_BLINKER, NULL);
		
	
	//create the timers that will be responsible for turning OFF blinking (brewing)
		//	The timers will be one-shot timers
		xTimerMocha = xTimerCreate("Mocha Timer", pdMS_TO_TICKS(TIME_MOCHA), pdFALSE, (void*)0, vMochaTimerCallback);
		xTimerLatte = xTimerCreate("Latte Timer", pdMS_TO_TICKS(TIME_LATTE), pdFALSE, (void*)0, vLatteTimerCallback);
		xTimerEspresso = xTimerCreate("Espresso Timer", pdMS_TO_TICKS(TIME_ESPRESSO), pdFALSE, (void*)0, vEspressoTimerCallback);
	
		//enable buttons
    STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
		buttonData.totalTime = 750;
		buttonData.flagUsed = 0;
		buttonData.currTime = 0;

    xTaskCreate(prvButtonTestTask, "BtnTest", STACK_SIZE_MIN, (void*)NULL, 5, NULL);

    xButtonSemaphore = xSemaphoreCreateBinary();
    xTaskSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(xTaskSemaphore);

    buttonTimer = xTimerCreate("timerButton", pdMS_TO_TICKS(1), pdTRUE, (void*)0, vTimerButtonCallback);
    waitTimer = xTimerCreate("waitButton", pdMS_TO_TICKS(100), pdTRUE, (void*)0, vTimerWaitCallback);

    vTaskStartScheduler();
    for (;;) {
    }
}

/**
Logic:
	check if the led is available to blink first
*/
void vTaskBlinkLed( void* pvLedToBlink) {
	TickType_t xWaitTimeBetweenNotBrewing;
	TickType_t xBlinkTime;
	//cast the parameter to the anticipated type (Led_TypeDef pointer) and dereference it 
	Led_TypeDef thisLed = *((Led_TypeDef*) pvLedToBlink);
	
	//store the correct pointer in this enable flag
	//	this pointer points to the correct variable that corresponds
	//	to the flag which basically says "this coffee timer is brewing"
	BaseType_t *thisLedEnableFlag;
	char* thisTaskName = pcTaskGetName(NULL);
	switch (thisLed) {
		case LED_FOR_OPTION_MOCHA:
			thisLedEnableFlag = &enableMocha;
			break;
		case LED_FOR_OPTION_LATTE:
			thisLedEnableFlag = &enableLatte;
			break;
		case LED_FOR_OPTION_ESPRESSO:
			thisLedEnableFlag = &enableEspresso;
			break;
		case LED_FOR_OPTION_NULL:
			//shouldnt happen
			thisLedEnableFlag = NULL;
			break;
	}
	
	xWaitTimeBetweenNotBrewing = pdMS_TO_TICKS(MS_IS_BREWING_CHECK_FREQUENCY);
	xBlinkTime = pdMS_TO_TICKS(MS_BLINKING_FREQUENCY);
	
	for (;;){
		if (*thisLedEnableFlag == BREWING_START) {
			//time to blink this LED, toggle it:
			STM_EVAL_LEDToggle(thisLed);
			//only toggle every X ms
			vTaskDelay(xBlinkTime);
		} else {
			//only check every X seconds so this task does not starve others
			vTaskDelay(xWaitTimeBetweenNotBrewing);
		}
	}
}


/**
	Every X ms, makes sure the specified LED is on if global flag is enabled
	Some LED is always on, so enable that light
*/
void vTurnOnSelectedLed(void* pvIgnored) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
		const TickType_t xPeriod = pdMS_TO_TICKS(MS_SELECTED_LED_POLL_FREQUENCY);
    for (;;) {
			STM_EVAL_LEDOn(ledGlobalSelectedLed);
			vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}




/**
	General idea for the timers is to disable the flag for blinking.

	Also sets the LED to off in so it does not pause on led-on
		If this is a selected LED, the rapid-polling higher-priority task vSelectedLedOn method 
			will make sure the LED turns on
*/
void vMochaTimerCallback( TimerHandle_t xTimer ){
	enableMocha = BREWING_STOP;
	STM_EVAL_LEDOff(LED_FOR_OPTION_MOCHA);
}
void vLatteTimerCallback( TimerHandle_t xTimer ){
	enableLatte = BREWING_STOP;
	STM_EVAL_LEDOff(LED_FOR_OPTION_LATTE);
}
void vEspressoTimerCallback( TimerHandle_t xTimer ){
	enableEspresso = BREWING_STOP;
	STM_EVAL_LEDOff(LED_FOR_OPTION_ESPRESSO);
}

void vStartMocha(void) {
	BaseType_t success; 
	
	//tell the polling task to blink the LED
	enableMocha = BREWING_START;
	//enable the timer for that duration
	success = xTimerReset(xTimerMocha, 0);
	if (success == pdPASS){
	} else {
			//failed to enable timer
	}
}

void vStartLatte(void){
	BaseType_t success; 
	
	enableLatte = BREWING_START;
	//enable the timer for that duration
	success = xTimerReset(xTimerLatte, 0);
	if (success == pdPASS){
	} else {
			//failed to enable timer
	}
}

void vStartEspresso(void) {
	BaseType_t success; 
	
	enableEspresso = BREWING_START;
	//enable the timer for that duration
	success = xTimerReset(xTimerEspresso, 0);
	if (success == pdPASS){
	} else {
			//failed to enable timer
	}
}


//Cycle between the LEDs as defined in the header
//	forces this LED to remain on using the created task
void vClickSingle(void) {
		//turn off this LED first
		STM_EVAL_LEDOff(ledGlobalSelectedLed);
    switch (ledGlobalSelectedLed) {
			case LED_FOR_OPTION_NULL:
				ledGlobalSelectedLed = LED_FOR_NULL_TO_MOCHA;
				break;
			case LED_FOR_OPTION_MOCHA:
				ledGlobalSelectedLed = LED_FOR_MOCHA_TO_LATTE;
				break;
			case LED_FOR_OPTION_LATTE:
				ledGlobalSelectedLed = LED_FOR_LATTE_TO_ESPRESSO;
				break;
			case LED_FOR_OPTION_ESPRESSO:
				ledGlobalSelectedLed = LED_FOR_ESPRESSO_TO_NULL;
				break;
		}			
}

//Starts or resets the selected timer, then resets the selector to NULL
void vClickDouble(void) {	
    switch (ledGlobalSelectedLed) {
			case LED_FOR_OPTION_NULL:
				//the currently selected LED is NULl, do nothing as this corresponds to no timer
				break;
			case LED_FOR_OPTION_MOCHA:
				//reset/start the timer that corresponds to the mocha
				vStartMocha();
				break;
			case LED_FOR_OPTION_LATTE:
				vStartLatte();
				break;
			case LED_FOR_OPTION_ESPRESSO:
				vStartEspresso();
				break;
		}			
		//turn off this LED
		STM_EVAL_LEDOff(ledGlobalSelectedLed);
	ledGlobalSelectedLed = LED_FOR_OPTION_NULL;
}

//Stop the timer
void vClickLong(void) {
    switch (ledGlobalSelectedLed) {
			case LED_FOR_OPTION_NULL:
				//the currently selected LED is NULL, do nothing as this corresponds to no timer
				break;
			case LED_FOR_OPTION_MOCHA:
				//reset/start the timer that corresponds to the mocha
				enableMocha = BREWING_STOP;
				break;
			case LED_FOR_OPTION_LATTE:
				enableLatte = BREWING_STOP;
				break;
			case LED_FOR_OPTION_ESPRESSO:
				enableEspresso = BREWING_STOP;
				break;
		}			
	//turn off this LED
	STM_EVAL_LEDOff(ledGlobalSelectedLed);
	ledGlobalSelectedLed = LED_FOR_OPTION_NULL;
}

void vTimerWaitCallback(TimerHandle_t xTimer) {
    waitButton = 0;
    xTimerStop(xTimer, 0);
}

void vTimerButtonCallback(TimerHandle_t xTimer) {
    uint32_t ulCount;

    ulCount = (uint32_t)pvTimerGetTimerID(xTimer);

    ulCount++;
    if (ulCount < buttonData.totalTime) {
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1) {
            buttonPressed++;
        }
        vTimerSetTimerID(xTimer, (void*)ulCount);
    }
    else {
        vTimerSetTimerID(xTimer, (void*)0);
        if (buttonPressed > 500) {
            vClickLong();
        }
        else if (buttonData.flagUsed == 1) {
            vClickSingle();
        }
        else if (buttonData.flagUsed == 2) {
            vClickDouble();
        }
        buttonData.flagUsed = 0;
        buttonPressed = 0;
        xTimerStop(xTimer, 0);
        waitButton = 1;
        xTimerStart(waitTimer, 0);
    }
};

static void prvButtonTestTask(void* pvParameters) {
    uint32_t ulCount;
    int lastPress = 0;

    xSemaphoreTake(xButtonSemaphore, mainDONT_BLOCK);

    for (;;) {
        xSemaphoreTake(xButtonSemaphore, portMAX_DELAY);

        if (waitButton == 0) {
            ulCount = (uint32_t)pvTimerGetTimerID(buttonTimer);

            if (buttonData.flagUsed == 0) {
                buttonPressed++;
                lastPress = 0;
                xTimerReset(buttonTimer, 0);
                buttonData.flagUsed++;
            }
            if (ulCount - lastPress >= 150) {
                lastPress = ulCount;
                buttonData.flagUsed++;
            }
        }
    }
}

void EXTI0_IRQHandler(void) {
    long lHigherPriorityTaskWoken = pdFALSE;

    EXTI_ClearITPendingBit(EXTI_Line0);
    xSemaphoreGiveFromISR(xButtonSemaphore, &lHigherPriorityTaskWoken);
    portEND_SWITCHING_ISR(lHigherPriorityTaskWoken);
}

