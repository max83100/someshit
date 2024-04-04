#include "led.h"

typedef enum
{
    Led_ShortImpulse = 200,
    Led_LongImpulse = 750,
    
} LedImpulseLen_t;

typedef struct
{
    LedMode_t mode;
    uint8_t step;
    uint16_t timerMs;
} LedContext_t;

LedContext_t context = {
    .mode = LM_None,
    .step = 0,
    .timerMs = 0,
};

void Led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOC_CLK_ENABLE();

    Led_SetState(false);

    /*Configure GPIO pin : LED_Pin */
    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

inline void Led_SetState(bool state)
{
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Led_MsHandle(void)
{
    if (context.timerMs)
    {
        context.timerMs--;
        return;
    }
    switch(context.mode)
    {
    case LM_None:
    case LM_Error:
        return;
    case LM_Started:
        {
            Led_SetState(!(context.step % 2));
            context.step++;
            context.timerMs = Led_ShortImpulse;
            if (context.step == 6)
            {
                context.mode = LM_None;
                context.step = context.timerMs = 0;
            }
        }
        break;
    case LM_GsmAction:
        {
            Led_SetState(!(context.step % 2));
            context.step++;
            context.timerMs = Led_LongImpulse;
            if (context.step == 2)
            {
                context.mode = LM_None;
                context.step = context.timerMs = 0;
            }
        }
        break;
    case LM_GsmError:
        {
            Led_SetState(!(context.step % 2));
            context.step++;
            context.timerMs = context.step == 1 ? Led_LongImpulse : Led_ShortImpulse;
            if (context.step == 6)
            {
                context.mode = LM_None;
                context.step = context.timerMs = 0;
            }
        }
        break;
    case LM_Warning:
        {
            Led_SetState(!(context.step % 2));
            context.step++;
            context.timerMs = context.step == 1 ? Led_LongImpulse : Led_ShortImpulse;
            if (context.step == 4)
            {
                context.mode = LM_None;
                context.step = context.timerMs = 0;
            }
        }
        break;
    }
}

void Led_SetMode(LedMode_t mode)
{
    if (context.mode == LM_Error)
        return;

    context.mode = mode;
    if (context.mode == LM_Error)
    {
        static bool state = false;
        while(true)
        {
            Led_SetState(state = !state);
            HAL_Delay(Led_ShortImpulse);
        }
    }
    context.step = 0;
    context.timerMs = 0;
}
