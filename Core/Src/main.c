#include <string.h>

#include "main.h"
#include "led.h"
#include "sim800c.h"
#include "macros.h"
#include "error_handler.h"
#include "flash.h"

typedef struct
{
    uint8_t head[4];
    char phone[PHONE_NUMBER_LENGTH];
    uint16_t checkSum;
} Config_t;

Config_t config = {
    .head = {'C','O','N','F'},
    .phone = {0},
    .checkSum = 0,
};

static const uint32_t version = 230610;

static struct
{
    uint8_t resetOrPowerOff;
} context = {
    .resetOrPowerOff = 0,
};

//I2C_HandleTypeDef hi2c1;
//RTC_HandleTypeDef hrtc;
TIM_HandleTypeDef htim4;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
//static void MX_I2C1_Init(void);
//static void MX_RTC_Init(void);
static void MX_TIM4_Init(void);
static uint16_t getChecksum(void* buffer, uint32_t len);
static Sim800C_OperationHandler_t operationHandler;

int main(void)
{
    UNUSED(version);
    
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    Led_Init();
    MX_TIM4_Init();

    Flash_Init();
    Sim800C_Init(operationHandler);
    //MX_I2C1_Init();
    //MX_RTC_Init();
    Led_SetMode(LM_Started);
    
    if (!Flash_Read(&config, 0, sizeof(config)))
        Led_SetMode(LM_Error);
    if (memcmp(config.head, "CONF", 4) || config.checkSum != getChecksum(&config, sizeof(config)-2))
    {
        Led_SetMode(LM_Warning);
        memcpy(config.head, "CONF", 4);
        memset(config.phone, 0, sizeof(config.phone));
        config.checkSum = getChecksum(&config, sizeof(config)-2);
        Flash_Write(&config, 0, sizeof(config));
    }
    
    HAL_Delay(1100);
    Sim800C_Power(true, 0);
    
    while (1)
    {
        Sim800C_MainHandler();
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
        |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}
/*
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}
*/
/*
static void MX_RTC_Init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef DateToUpdate = {0};

    hrtc.Instance = RTC;
    hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
    hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        Error_Handler();
    }
    
    sTime.Hours = 0x0;
    sTime.Minutes = 0x0;
    sTime.Seconds = 0x0;
    
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
    {
        Error_Handler();
    }
    DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
    DateToUpdate.Month = RTC_MONTH_JANUARY;
    DateToUpdate.Date = 0x1;
    DateToUpdate.Year = 0x0;
    
    if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
    {
        Error_Handler();
    }
}
*/

static void MX_TIM4_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 3;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 17999;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    
    __HAL_RCC_TIM4_CLK_ENABLE();
    if (HAL_TIM_Base_Start_IT(&htim4) != HAL_OK)
	{
		Error_Handler();
	}
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim4)
    {
        //Milliseconds
        Sim800C_MsHandler();
        Led_MsHandle();
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(WIRE_GPIO_Port, WIRE_Pin, GPIO_PIN_RESET);
    
    /*Configure GPIO pin : WIRE_Pin */
    GPIO_InitStruct.Pin = CS_Pin | WIRE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(WIRE_GPIO_Port, &GPIO_InitStruct);
}

static uint16_t getChecksum(void* buffer, uint32_t len)
{
    uint16_t result = 0;
    for(uint32_t z=0;z<len/2;z++)
        result ^= ((uint16_t*)buffer)[z] + z;
    if (len%2)
        result ^= ((uint8_t*)buffer)[len-1]+len;
    return result;
}

static void processIncomeSms(void)
{
    char phone[PHONE_NUMBER_LENGTH];
    char message[SMS_TEXT_BUFFER_LENGTH];
    memset(phone, 0, sizeof(phone));
    memset(message, 0, sizeof(message));
    if (!Sim800C_GetSms(phone, message))
        return;
    
    if (!memcmp(phone, config.phone, PHONE_NUMBER_LENGTH))
    {
        //TODO Обработка команд с зарегистрированного номера
        
        
    }else if (!memcmp(message, "REG", 3))
    {
        char imei[IMEI_LENGTH];
        memset(imei, 0, sizeof(imei));
        Sim800C_GetImei(imei);
        
        uint8_t imeiLen = strlen(imei);
        if (memcmp(&message[4], &imei[imeiLen-6], 6))
            return;
        
        memcpy(config.phone, phone, PHONE_NUMBER_LENGTH);
        config.checkSum = getChecksum(&config, sizeof(config)-2);
        Flash_Write(&config, 0, sizeof(config));
        
        //Sim800C_SendSms(config.phone, "BeeHive Registered");
    }
}

static void operationHandler(Sim800C_Operation_t operation, uint32_t state)
{
    if (state != (Sim800C_S_On | Sim800C_S_Ready))
    {
        if (context.resetOrPowerOff)
            Sim800C_Power(false, 60000);
        else
            Sim800C_Reset();
        context.resetOrPowerOff = (context.resetOrPowerOff+1)%2;
        return;
    }
    context.resetOrPowerOff = 0;
    
    switch(operation)
    {
    case Sim800C_O_Starting:
        Sim800C_RequestImei();
        break;
    case Sim800C_O_RequestImei:
        Sim800C_ClearMessages();
        break;
    case Sim800C_O_IncomeSms:
        processIncomeSms();
        break;
    default:
        break;
    }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
