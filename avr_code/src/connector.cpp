
/*
  LinuxCNC_AVR connector 

  Created By Alexander Richter 
  Ruined by Keith Legg May 2026 


  This Software is used as IO Expansion for LinuxCNC. 

  It is NOT intended for timing and security relevant IO's. Don't use it for Emergency Stops or Endstop switches!

  You can create as many digital & analog Inputs, Outputs and PWM Outputs as your Arduino can handle.
  You can also generate "virtual Pins" by using latching Potentiometers, which are connected to one analog Pin, but are read in Hal as individual Pins.

  Currently the Software Supports:
  - analog Inputs
  - latching Potentiometers
  - 1 binary encoded selector Switch
  - digital Inputs
  - digital Outputs
  - Matrix Keypad
  - Multiplexed LEDs
  - Quadrature encoders
  - Joysticks

  The Send and receive Protocol is <Signal><PinNumber>:<Pin State>
  To begin Transmitting Ready is send out and expects to receive E: to establish connection. Afterwards Data is exchanged.
  Data is only send everythime it changes once.

  Inputs & Toggle Inputs  = 'I' -write only  -Pin State: 0,1
  Outputs                 = 'O' -read only   -Pin State: 0,1

Communication Status      = 'E' -read/Write  -Pin State: 0:0

  The Keyboard is encoded in the Number of the Key in the Matrix. The according Letter is defined in the receiving end in the Python Skript.
  Here you only define the Size of the Matrix.

  Command 'E0:0' is used for connectivity checks and is send every 5 seconds as keep alive signal. If the Signal is not received again, the Status LED will Flash.
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

//#pragma GCC optimize ("O0")


#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>

//#include <stdlib.h>
#include <ctype.h>


#include "common.h"
#include <util/delay.h>


#include "common.h"

#include "millis.h"

#include "serial.h"




#define STATE_CMD 0
#define STATE_IO 1
#define STATE_VALUE 2

#define DEBUG

#define INPUTS 
#define OUTPUTS 
//#define SINPUTS 
//#define STATUSLED

const int timeout = 10000;   // timeout after 10 sec not receiving Stuff
const int debounceDelay = 50;

unsigned long oldmillis  = 0;
unsigned long newcom     = 0;
unsigned long lastcom    = 0;
int connectionState      = 0;

unsigned char state = STATE_CMD;
unsigned char inputbuffer[5];
unsigned char bufferIndex = 0;
unsigned char cmd = 0;

uint16_t io = 0;
uint16_t value = 0;

/***********************************************/

 
// Function Prototypes
void readCommands();
void readsInputs();
void readInputs();
void reconnect();
void comalive();

 
 
void writeOutputs(uint8_t Pin, uint8_t Stat);
void StatLedErr(uint8_t offtime, uint8_t ontime);
void send_data(char sig, uint8_t pin, uint8_t state);
 



/***********************************************/
 
#ifdef INPUTS
    #define Inputs 2             
    int InPinmap[] = {8,9};
#endif
 
#ifdef SINPUTS
    #define sInputs 1            
    int sInPinmap[] = {10};
#endif

#ifdef OUTPUTS
    #define Outputs 2              
    int OutPinmap[] = {11,12};
#endif


//The Software will detect if there is an communication issue. When you power on your machine, the Buttons etc won't work, till LinuxCNC is running. THe StatusLED will inform you about the State of Communication.
// Slow Flash = Not Connected
// Steady on = connected
// short Flash = connection lost.

// if connection is lost, something happened. (Linuxcnc was closed for example or USB Connection failed.) It will recover when Linuxcnc is restartet. (you could also run "unloadusr arduino", "loadusr arduino" in Hal)
// Define an Pin you want to connect the LED to. it will be set as Output indipendand of the OUTPUTS function, so don't use Pins twice.
// If you use Digital LED's such as WS2812 or PL9823 (only works if you set up the DLED settings below) you can also define a position of the LED. In this case StatLedPin will set the number of the Digital LED Chain.


#ifdef STATUSLED
    const int StatLedPin = 13;                //Pin for Status LED
    const int StatLedErrDel[] = {1000,10};   //Blink Timing for Status LED Error (no connection)
    const int DLEDSTATUSLED = 0;              //set to 1 to use Digital LED instead. set StatLedPin to the according LED number in the chain.
#endif
 
//Variables for Saving States
#ifdef INPUTS
    int InState[Inputs];
    int oldInState[Inputs];
    unsigned long lastInputDebounce[Inputs];
#endif

#ifdef SINPUTS
    int sInState[sInputs];
    int soldInState[sInputs];
    int togglesinputs[sInputs];
    unsigned long lastsInputDebounce[sInputs];
#endif

#ifdef OUTPUTS
    int OutState[Outputs];
    int oldOutState[Outputs];
#endif


/************************************************/
/************************************************/

void setup() 
{

    //setting Inputs with internal Pullup Resistors
    #ifdef INPUTS
        uint8_t i=0;
        for (int i=0; i<Inputs;i++)
        {
            //pinMode(InPinmap[i], INPUT_PULLUP);
            oldInState[i] = -1;
        }
    #endif

      
    //setting Inputs with internal Pullup Resistors
    #ifdef SINPUTS
        for(uint8_t i= 0; i<sInputs;i++)
        {
          //pinMode(sInPinmap[i], INPUT_PULLUP);
          soldInState[i] = -1;
          togglesinputs[i] = 0;
        }
    #endif

    #ifdef OUTPUTS
        for(uint8_t o= 0; o<Outputs;o++)
        {
          //pinMode(OutPinmap[o], OUTPUT);
          oldOutState[o] = 0;
        }
    #endif


    #ifdef STATUSLED
      //pinMode(StatLedPin, OUTPUT);
    #endif



}//end setup

/************************************************/
/************************************************/
 
void loop() 
{
    readCommands(); //receive and execute Commands
    comalive();     //if nothing is received for 10 sec. blink warning LED

    //KEEP THESE// #ifdef INPUTS
    //KEEP THESE//   readInputs(); //read Inputs & send data
    //KEEP THESE// #endif
    //KEEP THESE// #ifdef SINPUTS
    //KEEP THESE//   readsInputs(); //read Inputs & send data
    //KEEP THESE// #endif

}


/************************************************/

 
void comalive()
{
    //no connection yet. send E0:0 periodicly and wait for response
    if(lastcom == 0)
    {
        while (lastcom == 0)
        {
          readCommands();
          flushSerial();

          UART_write_str("E0:0");

          _delay_ms(200);

          #ifdef STATUSLED
              StatLedErr(1000,1000);
          #endif
        }
        connectionState = 1;
        flushSerial();

        #ifdef DEBUG
            UART_write_str_pgm("first connect");
        #endif
    }
}



/************************************************/
void reconnect()
{
    #ifdef DEBUG
        UART_write_str("reconnected");
        UART_write_str("resending Data");
    #endif

    #ifdef INPUTS
        for (int x = 0; x < Inputs; x++)
        {
            InState[x]= -1;
        }
    #endif

    #ifdef SINPUTS
        for (int x = 0; x < sInputs; x++)
        {
            soldInState[x]= -1;
            togglesinputs[x] = 0;
        }
    #endif

    #ifdef INPUTS
        readInputs(); //read Inputs & send data
    #endif

    #ifdef SINPUTS
        readsInputs(); //read Inputs & send data
    #endif


    connectionState = 1;


}
 



/************************************************/

#ifdef STATUSLED
    void StatLedErr(uint8_t offtime, uint8_t ontime)
    {
       
      unsigned long newMillis = millis();

      //if (newMillis - oldmillis >= offtime)
      if (newMillis - oldmillis >= offtime+ontime)
      {
          oldmillis = newMillis;
      }
    }
#endif

#ifdef OUTPUTS
    void writeOutputs(uint8_t Pin, uint8_t Stat){
      //digitalWrite(Pin, Stat);
    }
#endif
 
#ifdef INPUTS
    void readInputs()
    {
           
        for(int i= 0;i<Inputs; i++)
        {
            
            //int State = digitalRead(InPinmap[i]);
            int State = 1; //keith hacked this 

            if(InState[i]!= State && millis()- lastInputDebounce[i] > debounceDelay)
            {
                InState[i] = State;
                //send_data('I',InPinmap[i],InState[i]);
                UART_transmit('I');
                UART_transmit(InPinmap[i]);
                UART_transmit(InState[i]);                
                
                lastInputDebounce[i] = millis();
            }
        } 

    }
#endif

 
#ifdef SINPUTS
    void readsInputs()
    { 
        for(int i= 0;i<sInputs; i++)
        {
            //sInState[i] = digitalRead(sInPinmap[i]);

            if (sInState[i] != soldInState[i] && millis()- lastsInputDebounce[i] > debounceDelay){
              // Button state has changed and debounce delay has passed
              
              // Stuff after || is only there to send States at Startup
              if (sInState[i] == LOW || soldInState[i]== -1) 
              { 
                // Button has been pressed
                togglesinputs[i] = !togglesinputs[i];  // Toggle the LED state

                if (togglesinputs[i]) {
                  send_data('I',sInPinmap[i],togglesinputs[i]);  // Turn the LED on
                }
                else {
                  send_data('I',sInPinmap[i],togglesinputs[i]);   // Turn the LED off
                }
              }
              soldInState[i] = sInState[i];
              lastsInputDebounce[i] = millis();
          }
        }
         
    }
#endif
 



 
void commandReceived(char cmd, uint16_t io, uint16_t value)
{
     
    #ifdef OUTPUTS
    if(cmd == 'O'){
      writeOutputs(io,value);
      lastcom=millis();

    }
    #endif

    if(cmd == 'E'){
      lastcom=millis();
      if(connectionState == 2){
       reconnect();
      }
    }


    #ifdef DEBUG
        UART_write_str("I Received= ");
        UART_transmit(cmd);
        UART_transmit(io);
        UART_write_str(":");
        UART_transmit(value);
    #endif
     
}




void readCommands()
{
    unsigned char current;
     
    //while(Serial.available() > 0)
    //{
        //UART_receive_stream(current);
        
        switch(state)
        {
            case STATE_CMD:
                   cmd = current;
                   state = STATE_IO;
                   bufferIndex = 0;
            break;
            
            case STATE_IO:
                if(isdigit(current))
                {
                    inputbuffer[bufferIndex++] = current;
                }else if(current == ':')
                {
                    inputbuffer[bufferIndex] = 0;
                    //io = atoi(inputbuffer);
                    state = STATE_VALUE;
                    bufferIndex = 0;

                }
                else
                {
                    #ifdef DEBUG
                        //println("german debug: ");
                        //println(current);
                    #endif
                }
            break;
            
            case STATE_VALUE:
                
                if(isdigit(current))
                {
                    inputbuffer[bufferIndex++] = current;
                }
                else if(current == '\n')
                {
                    inputbuffer[bufferIndex] = 0;
                    
                    //value = atoi(inputbuffer);
               
                    commandReceived(cmd, io, value);

                    

                    state = STATE_CMD;
                }
                else
                {
                  #ifdef DEBUG
                      //UART_write_str("Ungültiges zeichen: ");
                      //UART_transmit(current);
                  #endif

                }
            break;
        }
         

    //} 
}
 

/******************************************************/

unsigned char current[100] = "";

int main (void)
{
    sei(); 
    millis_init();

    setup();

    DDRB = 0xff;     
 
    USART_Init(MYUBRR);
    
    millis_resume();
    


    while (1)
    {
        //loop();
        
        println("heloo fuckers");

        //UART_receive_stream(current); 
        //if(sizeof(current)>5)
        //{
        //    UART_write_str(sizeof(current));
        //    
        //} 

        _delay_ms(800); 

    }
    
} 

