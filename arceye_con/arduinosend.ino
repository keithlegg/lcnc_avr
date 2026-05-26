//arduinosend.ino

/*

NOTES:

    Taken from here: 
    https://forum.linuxcnc.org/18-computer/23166-arduino-i-o-with-linuxcnc

    Arduino Mega 2560 IO board for Linuxcnc

    the values at 32 pins on the arduino mega.
    The arduino pins used are 14-49, which I think are all digital and I set them to input

    I have just tested it printing to stderr, you will need to connect the arduino pins to something to test them and connect the component pins bit-0 to 31 to something in hal to do anything.
    Comment out the 3 test lines when using for real.

    You just get mostly 0 but some 1's in the test print, but the pins are in an unknown floating state unconnected.

*/

#include <SoftwareSerial.h>
#include "Wire.h"

void setup() 
{
    Serial.begin(9600);
    for(int x = 14; x < 50; x++)pinMode(x, INPUT);
}

void loop() 
{
int x, y, z;  
uint8_t  buff[5];

    buff[4] = '\0';
    z = 14;
    for(x = 0; x < 4; x++) // 4 packets 8 bits
        for(y = 0; y < 8 ; y++) bitWrite(buff[x], y, digitalRead(z++));
    Serial.write(buff, 4);
    delay(1000);    
}