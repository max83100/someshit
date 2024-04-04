#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(D9, D10); //RX, TX

void setup()
{
   gsmSerial.begin(38400); // Setting the baud rate of GSM Module
   Serial.begin(38400);    // Setting the baud rate of Serial Monitor (Arduino)
   delay(1000);
   Serial.println("Preparing to send SMS");
   SendMessage();
}

void loop()
{

}
void SendMessage()
{
   Serial.println("Setting the GSM in text mode");
   gsmSerial.println("AT+CMGF=1\r");
   delay(2000);
   Serial.println("Sending SMS to the desired phone number!");
   gsmSerial.println("AT+CMGS=+79108750305\r"); //gsmSerial.print("\r");
   // Replace x with mobile number
   delay(2000);

   gsmSerial.println("Hello from SIM900");    // SMS Text
   delay(200);
   gsmSerial.println((char)26);               // ASCII code of CTRL+Z
   delay(2000);
}