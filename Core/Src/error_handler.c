#include "error_handler.h"
#include "stm32f1xx_hal.h"
#include "led.h"

void Error_Handler(void)
{
    Led_SetMode(LM_Error);
    __disable_irq();
    while (1)
    {
    }
}
