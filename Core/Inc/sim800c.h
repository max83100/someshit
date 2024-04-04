#ifndef SIM800C_H
#define SIM800C_H

#include <stdbool.h>

#include "stm32f1xx_hal.h"

#define RING_Pin GPIO_PIN_8
#define RESET_Pin GPIO_PIN_11
#define POWER_Pin GPIO_PIN_15

#define RESET_GPIO_Port GPIOA
#define RING_GPIO_Port GPIOA
#define POWER_GPIO_Port GPIOA

#define PHONE_NUMBER_LENGTH 15
#define SMS_TEXT_BUFFER_LENGTH 256
#define IMEI_LENGTH 16

typedef enum
{
    Sim800C_S_Off       = 0x00000000U,
    Sim800C_S_On        = 0x00000001U,
    Sim800C_S_Busy      = 0x00000002U,
    Sim800C_S_Ready     = 0x00000004U,
    Sim800C_S_Timeout   = 0x20000000U,
    Sim800C_S_Error     = 0x40000000U,
} Sim800C_State_t;

typedef enum
{
    Sim800C_O_None = 0,
    Sim800C_O_Starting,
    Sim800C_O_PowerOff,
    Sim800C_O_RequestImei,
    Sim800C_O_Ping,
    Sim800C_O_Sms,
    Sim800C_O_IncomeSms,
} Sim800C_Operation_t;

typedef void (Sim800C_OperationHandler_t)(Sim800C_Operation_t operation, uint32_t state);

extern UART_HandleTypeDef huart1;

void Sim800C_Init(Sim800C_OperationHandler_t *operationHandler);
void Sim800C_MsHandler(void);
void Sim800C_MainHandler(void);
bool Sim800C_IsBusy(void);
bool Sim800C_IsOn(void);
bool Sim800C_IsReady(void);
void Sim800C_SendSms(const char* phone, const char* message);
void Sim800C_Reset(void);
void Sim800C_Power(bool state, uint32_t timerMs);
bool Sim800C_RequestImei(void);
bool Sim800C_GetImei(char* imei);
bool Sim800C_ClearMessages(void);
bool Sim800C_GetSms(char* phone, char* message);

#endif //SIM800C_H
