#include "sim800c.h"
#include "stm32f1xx_hal_rtc.h"

#define DATA_AMOUNT   24
#define NUMBER_AMOUNT  5

#define CONTROLLERSENDDATA_NUMBERS_BYTESIZE      (sizeof(Number_t) * NUMBER_AMOUNT)
#define CONTROLLERSENDDATA_NUMBERAMOUNT_BYTESIZE (sizeof(uint8_t))
#define CONTROLLERSENDDATA_DATA_BYTESIZE         (sizeof(Data_t) * DATA_AMOUNT)
#define CONTROLLERSENDDATA_CRC_BYTESIZE          (sizeof(uint32_t))
    
#define CONTROLLERSENDDATA_NUMBERS_RECORDPOS      0  
#define CONTROLLERSENDDATA_NUMBERAMOUNT_RECORDPOS (CONTROLLERSENDDATA_NUMBERS_RECORDPOS + CONTROLLERSENDDATA_NUMBERS_BYTESIZE)
#define CONTROLLERSENDDATA_DATA_RECORDPOS         (CONTROLLERSENDDATA_NUMBERAMOUNT_RECORDPOS + CONTROLLERSENDDATA_NUMBERAMOUNT_BYTESIZE)
#define CONTROLLERSENDDATA_CRC_RECORDPOS          (CONTROLLERSENDDATA_DATA_RECORDPOS + CONTROLLERSENDDATA_DATA_BYTESIZE)
#define CONTROLLERSENDDATA_RECORDBYTESIZE         (CONTROLLERSENDDATA_CRC_RECORDPOS + CONTROLLERSENDDATA_CRC_BYTESIZE)

typedef uint8_t Number_t[PHONE_NUMBER_LENGTH];

typedef struct
{
    float           TopTemperature;
    float           BottomTemperature;
    float           Weight;
    RTC_DateTypeDef Date;
    RTC_TimeTypeDef Time;
    uint8_t         IsSent;
} Data_t;

typedef struct
{
    Number_t Numbers[NUMBER_AMOUNT];
    uint8_t  NumbersAmount;
    Data_t   Data[DATA_AMOUNT];
    uint32_t CRC32;
}__attribute__((__packed__)) Info_t;

void ControllerSendData_StartMemoryInit();
void ControllerSendData_Config();
void ControllerSendData_Init();
void ControllerSendData_DeInit();
void ControllerSendData_Clock();


