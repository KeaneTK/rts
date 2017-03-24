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
#define TIME_ESPRESSO 15
#define TIME_LATTE 40
#define TIME_MOCHA 45
#define TIME_CAPPACCINO 20

#define mainDONT_BLOCK (0UL)

volatile unsigned long ulButtonPressCounts = 0UL;
void vTimer1Callback(TimerHandle_t xTimer);
void vTimer2Callback(TimerHandle_t xTimer);
void vTimer3Callback(TimerHandle_t xTimer);
void vTimerWaitCallback(TimerHandle_t xTimer);
void vTimerButtonCallback(TimerHandle_t xTimer);
void setLEDs(void);
TimerHandle_t buttonTimer;
int selectedCoffee = 0;
int buttonPressed;
int waitButton = 0;
TimerHandle_t waitTimer;

struct TIMER_DATA {
    int totalTime;
    Led_TypeDef led_coffee;
    TimerHandle_t timerHandle;
    int flagUsed;
} typedef timer_data;

timer_data timerData1;
timer_data timerData2;
timer_data timerData3;
timer_data buttonData;

#define STACK_SIZE_MIN 128 /* usStackDepth	- the stack size DEFINED IN WORDS.*/

void prvButtonTestTask(void* pvParameters);//static

static SemaphoreHandle_t xButtonSemaphore = NULL;

void initTimerStruct()
{
    timerData1.totalTime = 0;
    timerData1.flagUsed = 0;

    timerData2.totalTime = 0;
    timerData2.flagUsed = 0;

    timerData3.totalTime = 0;
    timerData3.flagUsed = 0;

    buttonData.totalTime = 750;
    buttonData.flagUsed = 0;
}

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    STM_EVAL_LEDInit(LED_BLUE);
    STM_EVAL_LEDInit(LED_GREEN);
    STM_EVAL_LEDInit(LED_ORANGE);
    STM_EVAL_LEDInit(LED_RED);

    STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);

    initTimerStruct();
		setLEDs();

    xTaskCreate(prvButtonTestTask, "BtnTest", STACK_SIZE_MIN, (void*)NULL, tskIDLE_PRIORITY, NULL);

    timerData1.timerHandle = xTimerCreate("timerCoffee1", pdMS_TO_TICKS(1000), pdTRUE, (void*)0, vTimer1Callback);
    timerData2.timerHandle = xTimerCreate("timerCoffee2", pdMS_TO_TICKS(1000), pdTRUE, (void*)0, vTimer2Callback);
    timerData3.timerHandle = xTimerCreate("timerCoffee3", pdMS_TO_TICKS(1000), pdTRUE, (void*)0, vTimer3Callback);
		
    xButtonSemaphore = xSemaphoreCreateBinary();

    buttonData.timerHandle = xTimerCreate("timerButton", pdMS_TO_TICKS(1), pdTRUE, (void*)0, vTimerButtonCallback);
		waitTimer = xTimerCreate("waitButton", pdMS_TO_TICKS(75), pdTRUE, (void*)0, vTimerWaitCallback);

    vTaskStartScheduler();
    for (;;) {
    }
}

void vTimer1Callback(TimerHandle_t xTimer) {
    uint32_t ulCount;
    timerData1.flagUsed = 1;

    ulCount = (uint32_t)pvTimerGetTimerID(xTimer);

    ulCount++;
    if (ulCount >= timerData1.totalTime) {
        STM_EVAL_LEDOff(timerData1.led_coffee);
        timerData1.flagUsed = 0;
        xTimerStop(xTimer, 0);
        vTimerSetTimerID(xTimer, (void*)0);
    }
    else {
        STM_EVAL_LEDToggle(timerData1.led_coffee);
        vTimerSetTimerID(xTimer, (void*)ulCount);
    }
}

void vTimer2Callback(TimerHandle_t xTimer) {
    uint32_t ulCount;
    timerData2.flagUsed = 1;

    ulCount = (uint32_t)pvTimerGetTimerID(xTimer);

    ulCount++;
    if (ulCount >= timerData2.totalTime) {
        STM_EVAL_LEDOff(timerData2.led_coffee);
        timerData2.flagUsed = 0;
        xTimerStop(xTimer, 0);
        vTimerSetTimerID(xTimer, (void*)0);
    }
    else {
        STM_EVAL_LEDToggle(timerData2.led_coffee);
        vTimerSetTimerID(xTimer, (void*)ulCount);
    }
}

void vTimer3Callback(TimerHandle_t xTimer) {
    uint32_t ulCount;
    timerData3.flagUsed = 1;

    ulCount = (uint32_t)pvTimerGetTimerID(xTimer);

    ulCount++;
    if (ulCount >= timerData3.totalTime) {
        STM_EVAL_LEDOff(timerData3.led_coffee);
        timerData3.flagUsed = 0;
        xTimerStop(xTimer, 0);
        vTimerSetTimerID(xTimer, (void*)0);
    }
    else {
        STM_EVAL_LEDToggle(timerData3.led_coffee);
        vTimerSetTimerID(xTimer, (void*)ulCount);
    }
}

void setLEDs() {
	STM_EVAL_LEDOff(LED_BLUE);
	STM_EVAL_LEDOff(LED_GREEN);
	STM_EVAL_LEDOff(LED_ORANGE);
	STM_EVAL_LEDOff(LED_RED);
	
	switch(selectedCoffee) {
		case 0: STM_EVAL_LEDOn(LED_GREEN); break;
		case 1: STM_EVAL_LEDOn(LED_ORANGE); break;
		case 2: STM_EVAL_LEDOn(LED_RED); break;
		case 3: STM_EVAL_LEDOn(LED_BLUE); break;
	}
}

timer_data insertData(int coffee, timer_data dataTimer) {
	switch(coffee) {
		case 0: 
			dataTimer.led_coffee = LED_GREEN;
			dataTimer.totalTime = TIME_ESPRESSO;
			break;
		case 1: 
			dataTimer.led_coffee = LED_ORANGE;
			dataTimer.totalTime = TIME_LATTE;
			break;
		case 2: 
			dataTimer.led_coffee = LED_RED;
			dataTimer.totalTime = TIME_MOCHA;
			break;
		case 3: 
			dataTimer.led_coffee = LED_BLUE;
			dataTimer.totalTime = TIME_CAPPACCINO;
			break;
	}
	return dataTimer;
}

void singleClick() {
	selectedCoffee++;
	if (selectedCoffee == 4) {
		selectedCoffee = 0;
	}
	
	setLEDs();
}

void doubleClick() {
	xTimerStop(timerData1.timerHandle, 0);
	xTimerStop(timerData2.timerHandle, 0);
	xTimerStop(timerData3.timerHandle, 0);
	timerData1.flagUsed = 0;
	timerData2.flagUsed = 0;
	timerData3.flagUsed = 0;
		
	setLEDs();
}

void longClick() {
	if (timerData1.flagUsed == 0) {
		timerData1 = insertData(selectedCoffee, timerData1);
		xTimerReset(timerData1.timerHandle, 0);
	} else if (timerData2.flagUsed == 0) {
		timerData2 = insertData(selectedCoffee, timerData2);
		xTimerReset(timerData2.timerHandle, 0);
	} else if (timerData3.flagUsed == 0) {
		timerData3 = insertData(selectedCoffee, timerData3);
		xTimerReset(timerData3.timerHandle, 0);
	}
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
            longClick();
        }
        else if (buttonData.flagUsed == 1) {
            singleClick();
        }
        else if (buttonData.flagUsed == 2) {
            doubleClick();
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
        ulCount = (uint32_t)pvTimerGetTimerID(buttonData.timerHandle);

        if (buttonData.flagUsed == 0) {
            buttonPressed++;
            lastPress = 0;
            xTimerReset(buttonData.timerHandle, 0);
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
