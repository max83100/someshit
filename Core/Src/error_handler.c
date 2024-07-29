#include "error_handler.h"
#include "stm32f1xx_hal.h"
#include "ControllerIndication.h"
#include "led.h"

void Error_Handler(void)
{
    ControllerIndication_DebugInfo("Error!!!!");
    Led_SetMode(LM_Error);
    __disable_irq();
    while (1)
    {
       ControllerIndication_DebugInfo("Error!!!!");
    }
}
