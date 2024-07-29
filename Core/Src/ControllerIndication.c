#include <stdio.h>
#include "ControllerIndication.h"
#include "ssd1306.h"

static char myText[100];
static char myDebugTest[100];

void ControllerIndication_ShowData(Data_t* theData)
{
    SSD1306_Clear();
    SSD1306_GotoXY (0,0); // goto 10, 10
    sprintf(myText, "Date: %.2d.%.2d.%.4d", theData->Date.Date, theData->Date.Month, theData->Date.Year);
    SSD1306_Puts (myText, &Font_7x10, SSD1306_COLOR_WHITE); // Date
    SSD1306_GotoXY (0,10); // goto 10, 10
    sprintf(myText, "Time: %.2d:%.2d:%.2d", theData->Time.Hours, theData->Time.Minutes, theData->Time.Seconds);
    SSD1306_Puts (myText, &Font_7x10, SSD1306_COLOR_WHITE); // Time
    SSD1306_GotoXY (0,20); // goto 10, 10
    sprintf(myText, "Top: %.1f C", theData->TopTemperature);
    SSD1306_Puts (myText, &Font_7x10, SSD1306_COLOR_WHITE); // Top Temp
    SSD1306_GotoXY (0,30); // goto 10, 10
    sprintf(myText, "Bottom: %.1f C", theData->BottomTemperature);
    SSD1306_Puts (myText, &Font_7x10, SSD1306_COLOR_WHITE); // print Hello
    SSD1306_GotoXY (0,40); // goto 10, 10
    sprintf(myText, "Weight: %.1f kg", theData->Weight);
    SSD1306_Puts (myText, &Font_7x10, SSD1306_COLOR_WHITE); // print Hello
    //SSD1306_GotoXY (0,50); // goto 10, 10
    //sprintf(aText, "Battery: %.1f V", 3.48);
    //SSD1306_Puts (aText, &Font_7x10, SSD1306_COLOR_WHITE); // print Hello
    SSD1306_UpdateScreen(); // update screen
}

void ControllerIndication_DebugInfo(char* theInfo)
{
    if(strcmp(myDebugTest, theInfo) == 0) return;
    SSD1306_Clear();
    SSD1306_GotoXY(0,30);
    SSD1306_Puts(theInfo, &Font_7x10, SSD1306_COLOR_WHITE);
    strcpy(myDebugTest, theInfo);
    SSD1306_UpdateScreen();
}
