#include <string.h>
#include <stdlib.h>

#include "sim800c.h"
#include "error_handler.h"
#include "buffer.h"
#include "led.h"

extern UART_HandleTypeDef huart1;

#define recvBufferLen 1024
#define sendBufferLen 1024

#define packetDetectionTimeoutMs 200
#define standartTimeoutMs 5000
#define pingPeriodMs 120000
#define smsTimeoutMs 60000

uint8_t recvBufferData[recvBufferLen] = {0};
uint8_t sendBufferData[sendBufferLen] = {0};

typedef enum
{
    IS_None = 0,
    IS_Reset,
    IS_GetBaud,
    IS_SetBaud,
    IS_Rdy,
    IS_Cfun,
    IS_CpinReady,
    IS_CallReady,
    IS_SmsReady,
    IS_Done,
    IS_TextMode,
    IS_Phone,
    IS_SmsText,
    IS_ReadSms,
    IS_ClearMessages,
} InternalState_t;

static struct
{
    uint32_t timerMs;
    uint32_t pingTimerMs;
    Buffer_t recvBuffer;
    Buffer_t sendBuffer;
    uint32_t internalState;
    uint32_t state;
    uint32_t packetDetectionTimerMs;
    Sim800C_Operation_t operation;
    char phone[PHONE_NUMBER_LENGTH];
    char message[SMS_TEXT_BUFFER_LENGTH];
    Sim800C_OperationHandler_t *operationHandler;
    char imei[IMEI_LENGTH];
    uint16_t smsNumber;
} context = {
    .timerMs = 0,
    .internalState = IS_None,
    .state = Sim800C_S_Off,
    .operation = Sim800C_O_None,
    .pingTimerMs = 0,
    .packetDetectionTimerMs = 0,
    .operationHandler = NULL,
    .imei = {0},
    .smsNumber = 0,
};

static void setError(Sim800C_State_t error);
static void sendAt(void);
static void sendTextMode(void);
static void sendPhone(void);
static void startRecv(void);
static void sendByte(void);
static void sendSmsText(void);
static void ping(void);
static void setReset(bool state);
static void getBaud(void);
static void setBaud(void);
static void setPower(bool state);
static void sendDone(void);
static void sendReadSms(void);

void Sim800C_Init(Sim800C_OperationHandler_t operationHandler)
{
    context.operationHandler = operationHandler;
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = RING_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(RING_GPIO_Port, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RESET_GPIO_Port, &GPIO_InitStruct);
    setReset(false);
    
    GPIO_InitStruct.Pin = POWER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(POWER_GPIO_Port, &GPIO_InitStruct);
    setPower(false);
    
    Buffer_Init(&context.recvBuffer, recvBufferData, recvBufferLen);
    Buffer_Init(&context.sendBuffer, sendBufferData, sendBufferLen);
    
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
        Error_Handler();
    startRecv();
}

static void startRecv(void)
{
    HAL_UART_Receive_IT(&huart1, (uint8_t*)&context.recvBuffer.byte, 1);
}

static void sendByte(void)
{
    HAL_UART_Transmit_IT(&huart1, (uint8_t*)&context.sendBuffer.data[context.sendBuffer.pos++], 1);
}

void Sim800C_MsHandler(void)
{
    if (context.timerMs)
        context.timerMs--;
    if (context.packetDetectionTimerMs < packetDetectionTimeoutMs)
        context.packetDetectionTimerMs++;
    
    if (!Sim800C_IsBusy() && Sim800C_IsOn())
        context.pingTimerMs++;
    if (context.pingTimerMs == pingPeriodMs)
        ping();
}

static void ping(void)
{
    context.pingTimerMs = 0;
    if ((context.operation == Sim800C_O_None)
        && Sim800C_IsOn() && Sim800C_IsReady())
        context.operation = Sim800C_O_Ping;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *handle)
{
    if (handle != &huart1)
        return;
    Buffer_Append(&context.recvBuffer);
    context.packetDetectionTimerMs = 0;
    startRecv();
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *handle)
{
    if (handle != &huart1)
        return;
    if (context.sendBuffer.pos == context.sendBuffer.count)
        Buffer_Clear(&context.sendBuffer);
    else
        sendByte();
}

void Sim800C_MainHandler(void)
{
    bool skipOk = false;
    while(context.packetDetectionTimerMs == packetDetectionTimeoutMs && context.recvBuffer.count > 1)
    {
        int16_t pos = Buffer_Find(&context.recvBuffer, (uint8_t*)"\r\n", 2);
        if (pos != -1)
        {
            Led_SetMode(LM_GsmAction);
            uint16_t len = pos+2;
            bool processed = false;
            if (!context.recvBuffer.data[0] || 
                (len >= 2 && (!memcmp(context.recvBuffer.data, "AT", 2) || !memcmp(context.recvBuffer.data, "\r\n", 2))))
                __NOP();
            else if (len > 6 && !memcmp(context.recvBuffer.data, "+IPR", 4) && context.operation == Sim800C_O_Starting)
            {
                uint32_t baud = atoi((char *)&context.recvBuffer.data[6]);
                processed = true;
                if (baud != 38400)
                    skipOk = true;
            }
            else if (len >= 2 && !memcmp(context.recvBuffer.data, "OK", 2))
            {
                if (skipOk)
                {
                    processed = true;
                    skipOk = false;
                }else if (context.operation == Sim800C_O_Starting && context.internalState == IS_SetBaud)
                    Sim800C_Reset();
                else if (context.operation == Sim800C_O_Ping)
                {
                    context.operation = Sim800C_O_None;
                    processed = true;
                }else if (context.operation == Sim800C_O_Sms && context.internalState == IS_TextMode)
                {
                    context.internalState = IS_Phone;
                    processed = true;
                }else if (context.operation == Sim800C_O_Sms && context.internalState == IS_ClearMessages)
                {
                    sendDone();
                    processed = true;
                }
            }
            else if (len > 3 && !memcmp(context.recvBuffer.data, "RDY", 3) && context.operation == Sim800C_O_Starting)
            {
                context.internalState = IS_Rdy;
                processed = true;            
            }
            else if (len > 7 && !memcmp(context.recvBuffer.data, "+CFUN: ", 7) && context.operation == Sim800C_O_Starting)
            {
                if (context.recvBuffer.data[7] != '1')
                    setError(Sim800C_S_Error);
                else
                {
                    context.internalState = IS_Cfun;
                    processed = true;
                }
            }
            else if (len > 7 && !memcmp(context.recvBuffer.data, "+CPIN: ", 7) && context.operation == Sim800C_O_Starting)
            {
                if (memcmp(&context.recvBuffer.data[7], "READY", 5))
                    setError(Sim800C_S_Error);
                else
                {
                    context.internalState = IS_CpinReady;
                    processed = true;
                }
            }
            else if (len > 10 && !memcmp(context.recvBuffer.data, "CALL READY", 10) && context.operation == Sim800C_O_Starting)
            {
                context.internalState = IS_CallReady;
                processed = true;            
            }
            else if (len > 9 && !memcmp(context.recvBuffer.data, "SMS READY", 9) && context.operation == Sim800C_O_Starting)
            {
                context.internalState = IS_SmsReady;
                processed = true;            
            }
            else if (len == 17 && context.operation == Sim800C_O_RequestImei)
            {
                bool error = false;
                for(uint8_t z=0;z<15;z++)
                {
                    uint8_t byte = context.recvBuffer.data[z] ;
                    if (byte >= '0' && byte <= '9')
                        context.imei[z] = byte;
                    else
                    {
                        error = true;
                        break;
                    }
                }
                context.imei[error ? 0 : 16] = 0;
                if (!error)
                    context.internalState = IS_Done;
                processed = true;
            }
            else if (len > 5 && !memcmp(context.recvBuffer.data, "ERROR", 5))
                setError(Sim800C_S_Error);
            else if (len > 7 && !memcmp(context.recvBuffer.data, "+CMT: \"", 7))
            {
                if (len > 21 && (!memcmp(context.recvBuffer.data + 7, "+", 1) || !memcmp(context.recvBuffer.data + 7, "8", 1)))
                {
                    memcpy(context.phone, context.recvBuffer.data + 7, !memcmp(context.recvBuffer.data + 7, "+", 1) ? 12 : 11);
                    HAL_UART_Transmit(&huart1,"GotNum",sizeof("GotNum"),10);
                    context.smsNumber = atoi((const char*)&context.recvBuffer.data[12]);
                    context.operation = Sim800C_O_IncomeSms;
                    context.internalState = IS_ReadSms;
                }
                processed = true;
            }else if (len > 12 && !memcmp(context.recvBuffer.data, "+CMGR: \"REC ", 12) && context.operation == Sim800C_O_IncomeSms &&
                      context.internalState == IS_ReadSms && 0)
            {
                do
                {
                    int16_t index[2] = {-1, -1};
                    index[0] = Buffer_Find(&context.recvBuffer, (uint8_t*)",\"", 2);
                    Buffer_Shift(&context.recvBuffer, index[0]+2);
                    index[1] = Buffer_Find(&context.recvBuffer, (uint8_t*)"\",", 2);
                    if (index[0] == -1 || index[1] == -1 || index[1] > 14)
                        break;
                    memcpy(context.phone, &context.recvBuffer.data[0], index[1]);
                    context.phone[index[1]] = 0;
                    index[0] = Buffer_Find(&context.recvBuffer, (uint8_t*)"\r\n", 2);
                    index[1] = Buffer_Find(&context.recvBuffer, (uint8_t*)"\r\n\r\n", 4);
                    uint16_t len = index[1]-index[0]-2;
                    if (len >= sizeof(context.message))
                        len = sizeof(context.message)-1;
                    memcpy(context.message, &context.recvBuffer.data[index[0]+2], len);
                    context.message[len] = 0;
                    context.internalState = IS_Done;
                    sendDone();
                    processed = true;
                    skipOk = true;
                }while(false);
            }
            else
            {
                volatile static uint32_t notRecognized = 0;
                notRecognized++;
            }
            if (processed)
            {
                context.state |= Sim800C_S_On;
                context.state &= ~(Sim800C_S_Error | Sim800C_S_Busy);
                context.pingTimerMs = 0;  
            }
            Buffer_Shift(&context.recvBuffer, len);
        }else if (context.recvBuffer.count >= 2 && !memcmp(context.recvBuffer.data, "> ", 2) && context.operation == Sim800C_O_Sms && 
                  context.internalState == IS_Phone)
        {
            context.internalState = IS_SmsText;
            context.state |= Sim800C_S_On;
            context.state &= ~(Sim800C_S_Error | Sim800C_S_Busy);
            context.pingTimerMs = 0;            
            Buffer_Shift(&context.recvBuffer, 2);
        }
        else
            Buffer_Shift(&context.recvBuffer, 1);
    }
    
    switch(context.operation)
    {
    case Sim800C_O_None:
        break;
    case Sim800C_O_Starting:
        {
            if (!context.timerMs)
            {
                if (context.internalState == IS_Reset)
                {
                    context.internalState = IS_GetBaud;
                    setReset(false);
                    context.state = Sim800C_S_On | Sim800C_S_Busy;
                    context.timerMs = 60000;
                }else
                    setError(Sim800C_S_Timeout);
            }
            if (context.internalState == IS_SmsReady)
                sendDone();
            else if (context.internalState == IS_GetBaud)
            {
                getBaud();
                context.internalState = IS_None;
                context.state = Sim800C_S_On | Sim800C_S_Busy;
            }else if (context.internalState == IS_SetBaud)
            {
                setBaud();
                context.internalState = IS_None;
                context.state = Sim800C_S_On | Sim800C_S_Busy;
            }
        }
        break;
    case Sim800C_O_RequestImei:
        {
            if (!context.timerMs)
                setError(Sim800C_S_Timeout);
            else if (context.internalState == IS_Done)
                sendDone();
        }
        break;
    case Sim800C_O_PowerOff:
        {
            if (!context.timerMs)
                Sim800C_Power(true, 0);
        }
        break;
    case Sim800C_O_Ping:
        {
            if (!Sim800C_IsBusy())
                sendAt();
            else if (!context.timerMs)
                setError(Sim800C_S_Timeout);
        }
        break;
    case Sim800C_O_Sms:
        {
            switch(context.internalState)
            {
            case IS_TextMode:
                {
                    if (!Sim800C_IsBusy())
                        sendTextMode();
                    else if (!context.timerMs)
                        setError(Sim800C_S_Timeout);
                }
                break;
            case IS_Phone:
                {
                    if (!Sim800C_IsBusy())
                        sendPhone();
                    else if (!context.timerMs)
                        setError(Sim800C_S_Timeout);
                }
                break;
            case IS_SmsText:
                {
                    if (!Sim800C_IsBusy())
                        sendSmsText();
                    else if (!context.timerMs)
                        setError(Sim800C_S_Timeout);
                }
                break;
            }
        }
        break;
    case Sim800C_O_IncomeSms:
        {
            switch(context.internalState)
            {
            case IS_ReadSms:
                {
                    if (!Sim800C_IsBusy())
                        sendReadSms();
                    else if (!context.timerMs)
                        setError(Sim800C_S_Timeout);
                }
                break;
            }
        }
        break;
    }
}

static void setError(Sim800C_State_t error)
{
    Buffer_Clear(&context.recvBuffer);
    Buffer_Clear(&context.sendBuffer);
    Sim800C_Operation_t operation = context.operation;
    context.state &= ~(Sim800C_S_Timeout | Sim800C_S_Error);
    context.state |= error;
    context.state &= ~Sim800C_S_Busy;
    context.operation = Sim800C_O_None;
    context.internalState = IS_None;
    Led_SetMode(LM_GsmError);
    if (context.operationHandler)
        context.operationHandler(operation, context.state);
}

static void sendAt(void)
{
    uint8_t command[] = "AT\r\n";
    Buffer_AppendArray(&context.sendBuffer, command, strlen((char*)command));
    context.state |= Sim800C_S_Busy;
    context.timerMs = standartTimeoutMs;
    sendByte();
}

static void getBaud(void)
{
    //uint8_t command[] = "AT+IPR=0\r\nAT&W\r\n";
    uint8_t command[] = "AT+IPR?\r\n";
    Buffer_AppendArray(&context.sendBuffer, command, strlen((char*)command));
    context.state |= Sim800C_S_Busy;
    sendByte(); 
}

static void setBaud(void)
{
    uint8_t command[] = "AT+IPR=38400\r\nAT&W\r\n";
    Buffer_AppendArray(&context.sendBuffer, command, strlen((char*)command));
    context.state |= Sim800C_S_Busy;
    sendByte(); 
}

static void sendTextMode(void)
{
    uint8_t command[] = "AT+CMGF=1\r\n";
    Buffer_AppendArray(&context.sendBuffer, command, strlen((char*)command));
    context.state |= Sim800C_S_Busy;
    context.timerMs = standartTimeoutMs;
    sendByte();
}

static void sendPhone(void)
{
    uint8_t command[] = "AT+CMGS=\"";
    Buffer_AppendArray(&context.sendBuffer, command, strlen((char*)command));
    Buffer_AppendArray(&context.sendBuffer, (uint8_t*)context.phone, strlen(context.phone));
    Buffer_AppendArray(&context.sendBuffer, (uint8_t*)"\"\r\n", 3);
    context.state |= Sim800C_S_Busy;
    context.timerMs = smsTimeoutMs;
    sendByte();
}

static void sendSmsText(void)
{
    uint16_t len = strlen(context.message);
    context.message[len] = 0x1A;
    len++;
    Buffer_AppendArray(&context.sendBuffer, (uint8_t*)context.message, len);
    context.state |= Sim800C_S_Busy;
    context.timerMs = smsTimeoutMs;
    sendByte();
}

inline bool Sim800C_IsBusy(void)
{
    return (context.state & Sim800C_S_Busy) != 0;
}

inline bool Sim800C_IsOn(void)
{
    return (context.state & Sim800C_S_On) != 0;
}

inline bool Sim800C_IsReady(void)
{
    return (context.state & Sim800C_S_Ready) != 0;
}

void Sim800C_SendSms(const char* phone, const char* message)
{
    strcpy(context.phone, phone);
    strcpy(context.message, message);

    context.internalState = IS_TextMode;
    context.operation = Sim800C_O_Sms;
    context.timerMs = standartTimeoutMs;
}

static inline void setReset(bool state)
{
    HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Sim800C_Reset(void)
{
    setReset(true);
    context.state = Sim800C_S_On | Sim800C_S_Busy;
    context.operation = Sim800C_O_Starting;
    context.internalState = IS_Reset;
    context.timerMs = 250;
}

static inline void setPower(bool state)
{
    HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

//state: true - on, false - off; timerMs: 0 - infinite, otherwise - duration of power off state in milliseconds
void Sim800C_Power(bool state, uint32_t timerMs)
{
    if (state)
    {
        setPower(true);
        Sim800C_Reset();
    }else
    {
        setPower(false);
        context.operation = Sim800C_O_None;
        context.state = Sim800C_S_Off;
        setReset(false);
        context.internalState = IS_None;
        if (timerMs)
        {
            context.operation = Sim800C_O_PowerOff;
            context.timerMs = timerMs;
        }
    }
}

bool Sim800C_RequestImei(void)
{
    if (Sim800C_IsBusy())
        return false;
    
    uint8_t command[] = "AT+GSN\r\n";
    Buffer_AppendArray(&context.sendBuffer, command, strlen((char*)command));
    context.state |= Sim800C_S_Busy;
    context.operation = Sim800C_O_RequestImei;
    context.internalState = IS_None;
    context.timerMs = standartTimeoutMs;
    sendByte();
    return true;
}

bool Sim800C_ClearMessages(void)
{
    if (Sim800C_IsBusy())
        return false;
    
    uint8_t command[] = "AT+CMGD=1,4\r\n";
    Buffer_AppendArray(&context.sendBuffer, command, strlen((char*)command));
    context.state |= Sim800C_S_Busy;
    context.operation = Sim800C_O_Sms;
    context.internalState = IS_ClearMessages;
    context.timerMs = smsTimeoutMs;
    sendByte();
    return true;
}

static void sendDone(void)
{
    context.state = Sim800C_S_On | Sim800C_S_Ready;
    context.internalState = IS_None;
    Sim800C_Operation_t operation = context.operation;
    context.operation = Sim800C_O_None;
    context.timerMs = 0;
    if (context.operationHandler)
        context.operationHandler(operation, context.state);
}

static void sendReadSms(void)
{
    uint8_t command[] = "AT+CMGR=";
    Buffer_AppendArray(&context.sendBuffer, command, strlen((char*)command));
    Buffer_AppendInteger(&context.sendBuffer, context.smsNumber, 4, false);
    Buffer_AppendArray(&context.sendBuffer, (uint8_t*)"\r\n", 2);
    context.state |= Sim800C_S_Busy;
    context.timerMs = standartTimeoutMs;
    sendByte();
}

bool Sim800C_GetSms(char* phone, char* message)
{
    if (!context.smsNumber)
        return false;
    context.smsNumber = 0;
    memcpy(phone, context.phone, PHONE_NUMBER_LENGTH);
    memcpy(message, context.message, SMS_TEXT_BUFFER_LENGTH);
    return true;
}

bool Sim800C_GetImei(char* imei)
{
    if (!context.imei[0])
        return false;
    memcpy(imei, context.imei, IMEI_LENGTH);
    return true;
}
