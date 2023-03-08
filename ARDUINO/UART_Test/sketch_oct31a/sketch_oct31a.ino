#include<SoftwareSerial.h>
SoftwareSerial SUART(16,17); //SRX = DPin-2; STX = DPin-3

void setup()
{
  Serial.begin(9600);
  SUART.begin(19200);
}

void loop()
{
  byte n = SUART.available(); //check if a character has arrived via SUART Port
  if (n != 0) //a charctaer has arrived; it has been auto saved in FIFO; say 1 as 0x31
  {
    char x = SUART.read(); //read arrived character from FIFO (say 1) and put into x as 0x31 
    Serial.print(x);  //send 0x31 to Serial Monitor to show 1 via UART Port
  }
}
