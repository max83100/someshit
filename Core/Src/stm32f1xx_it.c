#include "main.h"
#include "stm32f1xx_it.h"
#include "sim800c.h"
#include "error_handler.h"

//extern I2C_HandleTypeDef hi2c1;
//extern RTC_HandleTypeDef hrtc;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim4;

void NMI_Handler(void)
{
    Error_Handler();
}

void HardFault_Handler(void)
{
    Error_Handler();
}

void MemManage_Handler(void)
{
    Error_Handler();
}

void BusFault_Handler(void)
{
    Error_Handler();
}

void UsageFault_Handler(void)
{
    Error_Handler();
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void RTC_IRQHandler(void)
{
//    HAL_RTCEx_RTCIRQHandler(&hrtc);
}

void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
}

void I2C1_EV_IRQHandler(void)
{
//    HAL_I2C_EV_IRQHandler(&hi2c1);
}

void I2C1_ER_IRQHandler(void)
{
//    HAL_I2C_ER_IRQHandler(&hi2c1);
}

void SPI1_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&hspi1);
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}
