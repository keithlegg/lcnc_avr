/*
    LinuxCNC connector 

    Created: Alexander Richter     2022 
    Modified Keith Legg        May 2026 
     
    
    Keith made this just to understand the serial protocol. It does not work.



    The Send and receive Protocol is <Signal><PinNumber>:<Pin State>
    To begin Transmitting Ready is send out and expects to receive E: to establish connection. 
    Afterwards Data is exchanged.
    Data is only sent everytime it changes once.

    Inputs & Toggle Inputs    = 'I' -write only  -Pin State: 0,1
    Outputs                   = 'O' -read only   -Pin State: 0,1
    Communication Status      = 'E' -read/Write  -Pin State: 0:0

    Command 'E0:0' is used for connectivity checks and is send every 5 seconds as keep alive signal. 
    If the Signal is not received again, the Status LED will Flash.
    The Board will still work as usual and try to send it's data, so this feature is only to inform the User.


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#define STATE_CMD 0
#define STATE_IO 1
#define STATE_VALUE 2

#define DEBUG


const int timeout       = 10000;   // timeout after 10 sec not receiving Stuff
const int debounceDelay = 50;

unsigned long oldmillis = 0;
unsigned long newcom    = 0;
unsigned long lastcom   = 0;
int connectionState     = 0;

unsigned char state = STATE_CMD;

char inputbuffer[5];

unsigned char bufferIndex = 0;
char cmd         = 0;
uint16_t io      = 0;
uint16_t value   = 0;



const int Inputs = 2;               //number of inputs using internal Pullup resistor. (short to ground to trigger)
int InPinmap[] = {8,9};

int InState[Inputs];
int oldInState[Inputs];
unsigned long lastInputDebounce[Inputs];

//***************************//

// Function Prototypes
void readCommands();
void commandReceived(char cmd, uint16_t io, uint16_t value);

void readsInputs();
void readInputs();



void flushSerial();
void sendData(char sig, int pin, int state);
void reconnect();
void comalive();




//***************************//

void setup() 
{
    Serial.begin(115200);
    while (!Serial){}
    comalive();
}


//***************************//
void loop() {

    readCommands();  //receive and execute Commands
    comalive();      //if nothing is received for 10 sec. blink warning LED


}


//***************************//
void comalive()
{
    //no connection yet. send E0:0 periodicly and wait for response
    if(lastcom == 0)
    { 
        while (lastcom == 0)
        {
            readCommands();
            flushSerial();
            Serial.println("E0:0");
            delay(200);
        }

        connectionState = 1;

        flushSerial();

        #ifdef DEBUG
            Serial.println("first connect");
        #endif
    }

    if(millis() - lastcom > timeout)
    {

        if(connectionState == 1)
        {
          #ifdef DEBUG
              Serial.println("disconnected");
          #endif
          connectionState = 2;
        }

    }
    else
    {
        connectionState=1;
    }
}

//***************************//
void reconnect()
{
    #ifdef DEBUG
        Serial.println("reconnected");
        Serial.println("resending Data");
    #endif

    readInputs();

    connectionState = 1;
}

//***************************//
void sendData(char sig, int pin, int state)
{
    Serial.print(sig);
    Serial.print(pin);
    Serial.print(":");
    Serial.println(state);
}

//***************************//
void flushSerial()
{
    while (Serial.available() > 0) 
    {
        Serial.read();
    }
}



void readInputs()
{
    for(int i= 0;i<Inputs; i++)
    {
      int State = digitalRead(InPinmap[i]);
      if(InState[i]!= State && millis()- lastInputDebounce[i] > debounceDelay)
      {
          InState[i] = State;
          sendData('I',InPinmap[i],InState[i]);
          lastInputDebounce[i] = millis();
      }
    }
}



//***************************//
void commandReceived(char cmd, uint16_t io, uint16_t value)
{
    #ifdef OUTPUTS
        if(cmd == 'O')
        {
            //writeOutputs(io,value);
            lastcom=millis();
        }
    #endif

    if(cmd == 'E')
    {
        lastcom=millis();
        if(connectionState == 2)
        {
            reconnect();
        }
    }

    #ifdef DEBUG
        Serial.print("I Received: ");
        Serial.print(cmd);
        Serial.print(io);
        Serial.print(":");
        Serial.println(value);
    #endif
}

//***************************//
void readCommands()
{
    unsigned char current;

    while(Serial.available() > 0)
    {
        current = Serial.read();

        switch(state)
        {
            case STATE_CMD:
               cmd = current;
               state = STATE_IO;
               bufferIndex = 0;
            break;
            
            case STATE_IO:
                if(isDigit(current))
                {
                    inputbuffer[bufferIndex++] = current;
                }else if(current == ':')
                {
                    inputbuffer[bufferIndex] = 0;
                    io = atoi(inputbuffer);
                    state = STATE_VALUE;
                    bufferIndex = 0;

                }
                else
                {
                    #ifdef DEBUG
                        Serial.print("STATE IO: ");
                        Serial.println(current);
                    #endif
                }
            break;
            
            case STATE_VALUE:
                if(isDigit(current))
                {
                    inputbuffer[bufferIndex++] = current;
                }
                else if(current == '\n')
                {
                    inputbuffer[bufferIndex] = 0;
                    value = atoi(inputbuffer);
                    commandReceived(cmd, io, value);
                    state = STATE_CMD;
                }
                else
                {
                  #ifdef DEBUG
                      Serial.print("STATE VALUE: ");
                      Serial.println(current);
                  #endif
                }
            break;
        }

    }
}
