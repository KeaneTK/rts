#ifndef INITS
#define INITS

#include "stm32f4xx.h" 
#include "stm32f4xx_tim.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"   

void InitTimers(void);
void InitLeds(void);
void InitButtons(void);
void EnableTimerInterrupt(void);
void InitEXTI(void);
void EnableEXTIInterrupt(void);



#endif
