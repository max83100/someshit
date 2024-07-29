#include "ControllerSMSCommand.h"
#include "ControllerSendData.h"
#include "PDUConvert.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char CSMS_TextComm[CSMS_COUNTOFCOMM][CSMS_MAXTEXT] = {
"ZQRX",
"TIME",
"ADD PHONE",
"DEL PHONE",
"NUMBERS",
"DATA SET",
"STATUS"};

static ControllerSMS_Errors ControllerSMS_zqrx(char* theMessage, char* theNumber);
static ControllerSMS_Errors ControllerSMS_time(char* theMessage, char* theNumber);
static ControllerSMS_Errors ControllerSMS_addphone(char* theMessage, char* theNumber);
static ControllerSMS_Errors ControllerSMS_delphone(char* theMessage, char* theNumber);
static ControllerSMS_Errors ControllerSMS_numbers(char* theMessage, char* theNumber);
static ControllerSMS_Errors ControllerSMS_dataset(char* theMessage, char* theNumber);
static ControllerSMS_Errors ControllerSMS_status(char* theMessage, char* theNumber);

typedef ControllerSMS_Errors (*CSMS_Command)(char* theMessage, char* theNumber);

const CSMS_Command CSMS_AllCommands[CSMS_COUNTOFCOMM] = 
{
    ControllerSMS_zqrx,
    ControllerSMS_time,
    ControllerSMS_addphone,
    ControllerSMS_delphone,
    ControllerSMS_numbers,
    ControllerSMS_dataset,
    ControllerSMS_status,
};

ControllerSMS_Commands ControllerSMSCommand_CheckCommand(char* theMessage)
{  
    for(uint8_t anInd = 0; anInd < CSMS_COUNTOFCOMM; ++anInd)
    {
        if(strstr(theMessage, CSMS_TextComm[anInd]) == theMessage)
            return anInd;  
    }
    return CSMS_NONE;
}

ControllerSMS_Errors ControllerSMSCommand_Perform(char* theMessage, char* theNumber)
{
    ControllerSMS_Commands aCommand = ControllerSMSCommand_CheckCommand(theMessage);
    if(aCommand == CSMS_NONE)
    {
        PDUConvert_InvalidSettings();
        return CSMSE_InvalidCommand;
    }
    
    return CSMS_AllCommands[aCommand](theMessage, theNumber);
}

ControllerSMS_Errors ControllerSMS_time(char* theMessage, char* theNumber)
{
    if(strlen(theMessage) > 5 && strlen(theMessage) < 8)
    {
        int aHour = 0;
        if (sscanf(theMessage + 5, "%d", &aHour) == 1)
        {
            if(aHour <= 48)
            {
                ControllerSendData_GetInfo()->TimeInterval = 3600 * aHour;
                if(!PDUConvert_IntervalIsSet(theNumber, theMessage + 5))
                    return CSMSE_CantAnswer;
                return CSMSE_OK;
            }
        }
    }
    PDUConvert_InvalidSettings(theNumber);
    return CSMSE_WrongParameters;
}    

ControllerSMS_Errors ControllerSMS_addphone(char* theMessage, char* theNumber);

