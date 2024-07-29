#ifndef _CONTROLLERSENDDATA_H_
#define _CONTROLLERSENDDATA_H_

#include "sim800c.h"
#include "stm32f1xx_hal_rtc.h"

#define DATA_AMOUNT   24
#define NUMBER_AMOUNT  5

#define CONTROLLERSENDDATA_NUMBERS_BYTESIZE      (sizeof(Number_t) * NUMBER_AMOUNT)
#define CONTROLLERSENDDATA_NUMBERAMOUNT_BYTESIZE (sizeof(uint8_t))
#define CONTROLLERSENDDATA_DATA_BYTESIZE         (sizeof(Data_t) * DATA_AMOUNT)

#define CONTROLLERSENDDATA_RECORDBYTESIZE         (CONTROLLERSENDDATA_DATA_RECORDPOS + CONTROLLERSENDDATA_DATA_BYTESIZE)

typedef char Number_t[PHONE_NUMBER_LENGTH];

typedef struct
{
    RTC_DateTypeDef Date;
    RTC_TimeTypeDef Time;
    float           TopTemperature;
    float           BottomTemperature;
    float           Weight;
    uint8_t         IsSent;
}Data_t;

typedef struct
{
    Number_t Numbers[NUMBER_AMOUNT];
    uint8_t  NumbersAmount;
    uint16_t TimeInterval;
    Data_t   Data[DATA_AMOUNT];
}Info_t;

void ControllerSendData_StartMemoryInit(void);
void ControllerSendData_Config(Info_t* theInfo);
void ControllerSendData_Init(void);
void ControllerSendData_DeInit(void);
void ControllerSendData_Clock(void);
bool ControllerSendData_AddNumber(Number_t theNumber);
bool ControllerSendData_RemoveNumber(Number_t theNumber);
bool ControllerSendData_AddData(Data_t* theData);
bool ControllerSendData_SendData(void);

Info_t* ControllerSendData_GetInfo(void);

#endif
