#ifndef _CONTROLLERSMSCOMMAND_H_
#define _CONTROLLERSMSCOMMAND_H_

#include <stdint.h>

#define CSMS_COUNTOFCOMM 7
#define CSMS_MAXTEXT 12

typedef enum
{
    CSMSE_OK = 0,
    CSMSE_InvalidCommand = 1,
    CSMSE_UnknownNumber = 2,
    CSMSE_WrongParameters = 4,
    CSMSE_CantAnswer = 8,
} ControllerSMS_Errors;

typedef enum
{
  CSMS_ZQRX = 0,
  CSMS_TIME,
  CSMS_ADD_PHONE,
  CSMS_DEL_PHONE,
  CSMS_NUMBERS,
  CSMS_DATA_SET,
  CSMS_STATUS,
  CSMS_NONE,
}ControllerSMS_Commands;

ControllerSMS_Commands ControllerSMSCommand_CheckCommand(char* theMessage);
ControllerSMS_Errors   ControllerSMSCommand_Perform(char* theMessage, char* theNumber);

#endif
