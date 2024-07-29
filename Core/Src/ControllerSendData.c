#include "ControllerSendData.h"
#include "sim800.h"
#include "ControllerIndication.h"
#include "ControllerSMSCommand.h"
#include "PDUConvert.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Info_t* myInfo;
static SIM800_SMSMessage_t myMessage;
uint8_t isGetMessage = 0;

inline void ControllerSendData_Config(Info_t* theInfo)
{
    myInfo = theInfo;
}

void ControllerSendData_StartMemoryInit()
{
    memset(myInfo->Numbers, 0, CONTROLLERSENDDATA_NUMBERS_BYTESIZE);
    myInfo->NumbersAmount = 0;
    memset(myInfo->Data, 0, CONTROLLERSENDDATA_DATA_BYTESIZE);
    memset(&myMessage, 0, sizeof(SIM800_SMSMessage_t));
}

inline Info_t* ControllerSendData_GetInfo()
{
    return myInfo;
}

void ControllerSendData_Init()
{    
    if(myInfo->NumbersAmount == 0)
    {
        char mess[100];
        while(myInfo->NumbersAmount == 0)    
        { 
            ControllerIndication_DebugInfo("Waiting for number");
            if (!isGetMessage)
                continue;
            else
                if(strcmp(myMessage.text, "CONF") == 0)
                {
                    ControllerSendData_AddNumber(myMessage.sender);
                }
                else
                {
                    PDUConvert_InvalidSettings(myMessage.sender);
                    ControllerIndication_DebugInfo("Nope");
                }
            isGetMessage = 0;
        }
        
        PDUConvert_NumberIsSet(myInfo->Numbers[0]);
        
        uint8_t isSetInterval = 0;
        while(!isSetInterval)
        {
            ControllerIndication_DebugInfo("Waiting for Interval");
            if(isGetMessage)
            {
                if(ControllerSMSCommand_CheckCommand(myMessage.text) == CSMS_TIME && strcmp(myMessage.sender, myInfo->Numbers[0]) == 0)
                {
                    if(ControllerSMSCommand_Perform(myMessage.text))
                       PDUConvert_InvalidSettings(myInfo->Numbers[0]);
                    else
                        isSetInterval = 1;
                }
                else
                    PDUConvert_InvalidSettings(myInfo->Numbers[0]);
            }
            isGetMessage = 0;
        }
        
    }
    else
    {
        //SendMessageAboutTurningOn
    }
}

bool ControllerSendData_AddNumber(Number_t theNumber)
{
    if(myInfo->NumbersAmount == NUMBER_AMOUNT)
        return false;
    
    for(uint8_t anInd = 0; anInd < NUMBER_AMOUNT; ++anInd)
    {
        if(myInfo->Numbers[anInd][0] == 0)
        {
            memcpy(myInfo->Numbers[anInd], theNumber, sizeof(Number_t));
            myInfo->NumbersAmount++;
            return true;
        }
    }
    return false;
}


void ControllerSendData_DeInit()
{
    //???
}

void SIM800_NewSMSNotificationCallBack(SIM800_Handle_t *handle, uint32_t sms_index)
{
  SIM800_RequestSMSMessage(handle, sms_index);
}


void SIM800_RcvdSMSCallBack(SIM800_Handle_t *handle, SIM800_SMSMessage_t *message)
{
  memcpy(&myMessage, message, sizeof(SIM800_SMSMessage_t));
  isGetMessage = 1;
}

