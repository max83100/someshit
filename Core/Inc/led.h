#ifndef LED_H
#define LED_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"

#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC

typedef enum
{
    LM_None = 0,
    LM_Started,
    LM_Error,
    LM_GsmAction,
    LM_GsmError,
    LM_Warning,
} LedMode_t;

void Led_Init(void);
void Led_SetState(bool state);
void Led_MsHandle(void);
void Led_SetMode(LedMode_t mode);
#endif
