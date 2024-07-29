#ifndef _CONTROLLERINDICATION_H_
#define _CONTROLLERINDICATION_H_

#include <stdint.h>

#include "ControllerSendData.h"

void ControllerIndication_ShowData(Data_t* theData);
void ControllerIndication_ShowError(uint8_t theErrorID);
void ControllerIndication_DebugInfo(char* theInfo);

#endif
