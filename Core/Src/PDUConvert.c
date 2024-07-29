#include "PDUConvert.h"
#include "sim800c.h"
#include "sim800.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define NUMBER_LENGHT 12
#define MESSAGE_END_LINE 0x1a

#define PDUDOT "002E"
#define PDUMINUS "002D"
#define PDUPLUS "002B"

#define PDUSTARTMESSAGE "0001000B91"

#define PDUNUMSTART "003"

static char mySwappedNumber[13];
static char myConvertedResult[49];

static void send_Message(char* theMessage, uint8_t theLen)
{
    char cmd[50];
    sprintf(cmd,"AT+CMGF=0\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t *)cmd,strlen(cmd),1000);
    HAL_Delay(1000);
    
    uint8_t anIndex = SIM800_SetWaitingMess(&sim800h);
    uint8_t aTimes = 0;
    while(!SIM800_PDUSuccess())
    {
        sprintf(cmd,"AT+CMGS=%d\r\n", theLen);
        HAL_UART_Transmit(&huart1,(uint8_t *)cmd,strlen(cmd),1000);
        HAL_Delay(100);
        HAL_UART_Transmit(&huart1,(uint8_t *)theMessage,strlen(theMessage),1000);
        HAL_Delay(6000);
        ++aTimes;
        if(aTimes >= 2 && !SIM800_PDUSuccess())
        {
            SIM800_RemoveWaitingMess(&sim800h, anIndex);
            sprintf(cmd,"AT+CMGF=1\r\n");
            HAL_UART_Transmit(&huart1,(uint8_t *)cmd,strlen(cmd),1000);
            HAL_Delay(1000);
            HAL_UART_Transmit(&huart1,(uint8_t *)"AT+CPOF\r\n",strlen("AT+CPOF\r\n"),1000);
            HAL_Delay(60000);
            aTimes = 0;
            
            sprintf(cmd,"AT+CMGF=0\r\n");
            HAL_UART_Transmit(&huart1,(uint8_t *)cmd,strlen(cmd),1000);
            HAL_Delay(1000);
            anIndex = SIM800_SetWaitingMess(&sim800h);
        }
    }
    SIM800_RemoveWaitingMess(&sim800h, anIndex);
    sprintf(cmd,"AT+CMGF=1\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t *)cmd,strlen(cmd),1000);
    HAL_Delay(1000);
}

static bool swap_Number(char* theNumber)
{
    bool aResult = false;
    
    memset(mySwappedNumber, '\0', 13);
    
    if(theNumber[0] == '+')
    {
        memcpy(mySwappedNumber, theNumber, NUMBER_LENGHT);
        for(uint8_t anInd = 1; anInd < NUMBER_LENGHT; ++anInd)
            mySwappedNumber[anInd-1] = mySwappedNumber[anInd];
        mySwappedNumber[NUMBER_LENGHT-1] = 'F';
        
        char aChar;
        
        for (uint8_t anPair = 0; anPair < 6; ++anPair)
        {
            aChar                         = mySwappedNumber[anPair*2];
            mySwappedNumber[anPair*2]     = mySwappedNumber[anPair*2 + 1];
            mySwappedNumber[anPair*2 + 1] = aChar;
        }
        aResult = true;
    }
    
    return aResult;
}

static bool convert_Value(char* theValue)
{
    bool aResult = true;
    
    memset(myConvertedResult, '\0', 49);
    
    uint8_t aLen = strlen(theValue);
    for (uint8_t anInd = 0; anInd < aLen; anInd++)
    {
        if(theValue[anInd] >= '0' && theValue[anInd] <= '9')
        {
            strcat(myConvertedResult, PDUNUMSTART);
            strncat(myConvertedResult, theValue + anInd, 1);
        }
        else if(theValue[anInd] == '.')
            strcat(myConvertedResult, PDUDOT);
        else if(theValue[anInd] == '-')
            strcat(myConvertedResult, PDUMINUS);
        else if(theValue[anInd] == '+')
            strcat(myConvertedResult, PDUPLUS);
        else
        {
            aResult = false;
            break;
        }
    }
    
    return aResult;
}

bool PDUConvert_NumberIsSet(char* theNumber)
{   
    if(!swap_Number(theNumber) || !convert_Value(theNumber))
        return false;
    
    char anEndMessage[2];
    anEndMessage[0] = MESSAGE_END_LINE;
    anEndMessage[1] = '\0';
    
    char aMessage[206] = PDUSTARTMESSAGE;
    
    strcat(aMessage, mySwappedNumber);
    strcat(aMessage, "000858041D043E043C043504400020");
    strcat(aMessage, myConvertedResult);
    strcat(aMessage, "00200434043E043104300432043B0435043D00200434043B044F0020043E043F043E0432043504490435043D04380439000A000A");
    strcat(aMessage, anEndMessage);
    
    send_Message(aMessage, 101);
    
    return true;
}

bool PDUConvert_InvalidSettings(char* theNumber)
{
    if(!swap_Number(theNumber))
        return false;
    
    char anEndMessage[2];
    anEndMessage[0] = MESSAGE_END_LINE;
    anEndMessage[1] = '\0';
    
    char aMessage[186] = PDUSTARTMESSAGE;
    strcat(aMessage, mySwappedNumber);
    strcat(aMessage, "00084E0412044B0020043204320435043B04380020043D0435043204350440043D044B0435002004340430043D043D044B04350020043F044004380020043D0430044104420440043E0439043A0435000A");
    strcat(aMessage, anEndMessage);
    send_Message(aMessage, 91);
    return true;
}

bool PDUConvert_IntervalIsSet(char* theNumber, char* theHour)
{
    if(!swap_Number(theNumber) || !convert_Value(theHour))
        return false;
    
    uint8_t aMesSize = 75 + (strlen(theHour) * 2);
    
    char anEndMessage[2];
    anEndMessage[0] = MESSAGE_END_LINE;
    anEndMessage[1] = '\0';
    char aMessage[165] = PDUSTARTMESSAGE;
    strcat(aMessage, mySwappedNumber);
    strcat(aMessage, "00084");
    strcat(aMessage, aMesSize == 77 ? "0" : "2");
    strcat(aMessage,"041F043504400438043E04340020043E043F043E0432043504490435043D0438043900200432044B043104400430043D0020");
    strcat(aMessage, myConvertedResult);
    strcat(aMessage, "00200447043004410430000A");
    strcat(aMessage, anEndMessage);
    send_Message(aMessage, aMesSize);
    return true;
}

bool PDUConvert_Temperature(char* theNumber, char* theTempDS, char* theTempDH)
{
     if(!swap_Number(theNumber))
        return false;
     
     uint8_t aTempSizeSum = (strlen(theTempDH) + strlen(theTempDS)) * 2;
     uint8_t aMesSize = 103 + aTempSizeSum;
     uint8_t aHexSize = 90  + aTempSizeSum;
    
     char anEndMessage[2];
     anEndMessage[0] = MESSAGE_END_LINE;
     anEndMessage[1] = '\0';
    
     char aHexMess[3] = "00";
    
     char aMessage[245] = PDUSTARTMESSAGE;
     strcat(aMessage, mySwappedNumber);
     strcat(aMessage, "0008");
     sprintf(aHexMess, "%X", aHexSize);
     strcat(aMessage, aHexMess);
     strcat(aMessage, "04220435043C043F043504400430044204430440043000200044005300310038006200320030002020130020");
     
     if(!convert_Value(theTempDS))
        return false;
     
     strcat(aMessage, myConvertedResult);
     strcat(aMessage,"0043000A04220435043C043F0435044004300442044304400430002000440048005400310031002020130020");
     
     if(!convert_Value(theTempDH))
        return false;
     
     strcat(aMessage, myConvertedResult);
     strcat(aMessage,"0043");
     
     strcat(aMessage, anEndMessage);
     send_Message(aMessage, aMesSize);
     
     return true;
}

bool PDUConvert_OtherData(char* theNumber, char* theHumid, char* theWeight, char* theBattVolt)
{
    if(!swap_Number(theNumber))
        return false;
    
    uint8_t aTempSizeSum = (strlen(theHumid) + strlen(theWeight) + strlen(theBattVolt)) * 2;
    uint8_t aMesSize = 107 + aTempSizeSum;
    uint8_t aHexSize = 94  + aTempSizeSum;
    
    char anEndMessage[2];
    anEndMessage[0] = MESSAGE_END_LINE;
    anEndMessage[1] = '\0';
    
    char aHexMess[3] = "00";
    
    char aMessage[280] = PDUSTARTMESSAGE;
    strcat(aMessage, mySwappedNumber);
    strcat(aMessage, "0008");
    sprintf(aHexMess, "%X", aHexSize);
    strcat(aMessage, aHexMess);
    strcat(aMessage,"0412043B04300436043D043E04410442044C002020130020");
    
    if(!convert_Value(theHumid))
        return false;
    strcat(aMessage, myConvertedResult);
    
    strcat(aMessage,"0025000A041204350441002020130020");
    if(!convert_Value(theWeight))
        return false;
    strcat(aMessage, myConvertedResult);
    
    strcat(aMessage,"0020041A0433000A041D0430043F0440044F04360435043D0438043500200431043004420430044004350438002020130020");
    if(!convert_Value(theBattVolt))
        return false;
    strcat(aMessage, myConvertedResult);
    
    strcat(aMessage, "00200412");
    strcat(aMessage, anEndMessage);
    send_Message(aMessage, aMesSize);
    
    return true;
}

bool PDUConvert_BattIsLow(char* theNumber)
{

    if(!swap_Number(theNumber))
        return false;
    
    char anEndMessage[2];
    anEndMessage[0] = MESSAGE_END_LINE;
    anEndMessage[1] = '\0';
    
    char aMessage[220] = PDUSTARTMESSAGE;  
    strcat(aMessage, mySwappedNumber);
    strcat(aMessage, "00085E041704300440044F0434002004310430044204300440043504380020043D04380437043A04380439002E0020041F043E044004300020043704300440044F043404380442044C00200443044104420440043E0439044104420432043E002E");
    strcat(aMessage, anEndMessage);
    
    send_Message(aMessage, 107);
    
    return true;
}

bool PDUConvert_InvalidCommand(char* theNumber)
{
    
}

bool PDUConvert_CouldntAnswer(char* theNumber);
