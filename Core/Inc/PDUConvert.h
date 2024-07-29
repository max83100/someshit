#ifndef _PDUCONVERT_H
#define _PDUCONVERT_H
#include <stdbool.h>

bool PDUConvert_NumberIsSet(char* theNumber);
bool PDUConvert_IntervalIsSet(char* theNumber, char* theHour);
bool PDUConvert_InvalidSettings(char* theNumber);
bool PDUConvert_Temperature(char* theNumber, char* theTempDS, char* theTempDH);
bool PDUConvert_OtherData(char* theNumber, char* theHumid, char* theWeight, char* theBattVolt);
bool PDUConvert_BattIsLow(char* theNumber);
bool PDUConvert_NumberDeleted(char* theNumber);
bool PDUConvert_InvalidCommand(char* theNumber);
bool PDUConvert_CouldntAnswer(char* theNumber);
#endif
