
/*
  LinuxCNC_ArduinoConnector
  By Alexander Richter, info@theartoftinkering.com 2022

  This Software is used as IO Expansion for LinuxCNC. Here i am using a Mega 2560.

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
  PWM Outputs             = 'P' -read only   -Pin State: 0-255
  Digital LED Outputs     = 'D' -read only   -Pin State: 0,1
  Analog Inputs           = 'A' -write only  -Pin State: 0-1024
  Latching Potentiometers = 'L' -write only  -Pin State: 0-max Position
  binary encoded Selector = 'K' -write only  -Pin State: 0-32
  rotary encoder          = 'R' -write only  -Pin State: up/ down / -2147483648 to 2147483647
  joystick                = 'R' -write only  -Pin State: up/ down / -2147483648 to 2147483647
  multiplexed LEDs        = 'M' -read only   -Pin State: 0,1

Keyboard Input:
  Matrix Keypad           = 'M' -write only  -Pin State: 0,1

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



#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed

//UART STUFF
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1


#define BIT_ON 0x30 //logic high
#define BIT_OFF 0x31 //logic low



#include <util/delay.h>



/***********************************************/

void USART_Init( unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    /*Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

}


/***********************************************/

static uint8_t USART_receive(void)
{
    while (!(UCSR0A & (1 << RXC0))) {/*Busy wait.*/}
    return UDR0;
}

/***********************************************/

void USART_Transmit( unsigned char data )
{
  while ( !( UCSR0A & (1<<UDRE0)) );
  UDR0 = data;
}


/***********************************************/

void print_byte( uint8_t data){
   uint8_t i = 0;

   for (i=0; i<=7; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (7 - i))) ){  // MSB
           USART_Transmit( BIT_OFF );
       }else{
           USART_Transmit( BIT_ON );
       }
    }
    //USART_Transmit( CHAR_TERM ); //CHAR_TERM = new line  
    //USART_Transmit( 0xd ); //0xd = carriage return
}

/***********************************************/


void print_byte_16( uint16_t data){
   uint8_t i = 0;

   for (i=0; i<=15; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (15 - i))) ){  // MSB
           USART_Transmit( BIT_OFF );
       }else{
           USART_Transmit( BIT_ON );
       }
    }
    //USART_Transmit( CHAR_TERM ); //CHAR_TERM = new line  
    //USART_Transmit( 0xd ); //0xd = carriage return
}



/******************************************************/
/******************************************************/
/******************************************************/
/******************************************************/







//###################################################IO's###################################################


#define INPUTS                  //Use Arduino IO's as Inputs. Define how many Inputs you want in total and then which Pins you want to be Inputs.
#ifdef INPUTS
  #define Inputs 1              //number of inputs using internal Pullup resistor. (short to ground to trigger)
  int InPinmap[] = {8,9};
#endif

/*
                                //Use Arduino IO's as Toggle Inputs, which means Inputs (Buttons for example) keep HIGH State after Release and Send LOW only after beeing Pressed again.
#define SINPUTS                 //Define how many Toggle Inputs you want in total and then which Pins you want to be Toggle Inputs.
#ifdef SINPUTS
  int sInputs = 1;              //number of inputs using internal Pullup resistor. (short to ground to trigger)
  int sInPinmap[] = {10};
#endif

#define OUTPUTS                     //Use Arduino IO's as Outputs. Define how many Outputs you want in total and then which Pins you want to be Outputs.
#ifdef OUTPUTS
  int Outputs = 2;              //number of outputs
  int OutPinmap[] = {11,12};
#endif

//#define PWMOUTPUTS                     //Use Arduino PWM Capable IO's as PWM Outputs. Define how many  PWM Outputs you want in total and then which Pins you want to be  PWM Outputs.
#ifdef PWMOUTPUTS
  int PwmOutputs = 2;              //number of outputs
  int PwmOutPinmap[] = {12,11};
#endif

//#define AINPUTS                       //Use Arduino ADC's as Analog Inputs. Define how many Analog Inputs you want in total and then which Pins you want to be Analog Inputs.
                                        //Note that Analog Pin numbering is different to the Print on the PCB.
#ifdef AINPUTS
  int AInputs = 1;
  int AInPinmap[] = {0};                //Potentiometer for SpindleSpeed override
  int smooth = 200;                     //number of samples to denoise ADC, try lower numbers on your setup 200 worked good for me.
#endif

*/



/*This is a special mode of AInputs. My machine had originally Selector Knobs with many Pins on the backside to select different Speed Settings.
I turned them into a "Potentiometer" by connecting all Pins with 10K Resistors in series. Then i applied GND to the first and 5V to the last Pin.
Now the Selector is part of an Voltage Divider and outputs different Voltage for each Position. This function generates Pins for each Position in Linuxcnc Hal.

It can happen, that when you switch position, that the selector is floating for a brief second. This might be detected as Position 0.
This shouldn't be an issue in most usecases, but think about that in your application.



Connect it to an Analog In Pin of your Arduino and define how many of these you want.
Then in the Array, {which Pin, How many Positions}
Note that Analog Pin numbering is different to the Print on the PCB.

*/

/*
//#define LPOTIS
#ifdef LPOTIS
  const int LPotis = 2;
  const int LPotiPins[LPotis][2] = {
                    {1,9},             //Latching Knob Spindle Overdrive on A1, has 9 Positions
                    {2,4}              //Latching Knob Feed Resolution on A2, has 4 Positions
                    };
  int margin = 20;                      //giving it some margin so Numbers dont jitter, make this number smaller if your knob has more than 50 Positions
#endif



//#define BINSEL                   //Support of an Rotating Knob that was build in my Machine. It encodes 32 Positions with 5 Pins in Binary. This will generate 32 Pins in LinuxCNC Hal.
#ifdef BINSEL
  const int BinSelKnobPins[] = {2,6,4,3,5};  //1,2,4,8,16
#endif


//#define QUADENC
//Support for Quadrature Encoders. Define Pins for A and B Signals for your encoders. Visit https://www.pjrc.com/teensy/td_libs_Encoder.html for further explanation.
// Download Zip from here: https://github.com/PaulStoffregen/Encoder and import as Library to your Arduino IDE.
#ifdef QUADENC
  #include <Encoder.h>
  #define QUADENCS 2  //how many Rotary Encoders do you want?

    // Encoders have 2 signals, which must be connected to 2 pins. There are three options.

    //Best Performance: Both signals connect to interrupt pins.
    //Good Performance: First signal connects to an interrupt pin, second to a non-interrupt pin.
    //Low Performance: Both signals connect to non-interrupt pins, details below.

    //Board             Interrupt Pins              LED Pin(do not use)
    //Teensy 4.0 - 4.1  All Digital Pins              13
    //Teensy 3.0 - 3.6  All Digital Pins              13
    //Teensy LC         2 - 12, 14, 15, 20 - 23     13
    //Teensy 2.0          5, 6, 7, 8                    11
    //Teensy 1.0          0, 1, 2, 3, 4, 6, 7, 16
    //Teensy++ 2.0      0, 1, 2, 3, 18, 19, 36, 37  6
    //Teensy++ 1.0      0, 1, 2, 3, 18, 19, 36, 37
    //Arduino Due         All Digital Pins            13
    //Arduino Uno         2, 3                        13
    //Arduino Leonardo  0, 1, 2, 3                  13
    //Arduino Mega      2, 3, 18, 19, 20, 21          13
    //Sanguino          2, 10, 11                     0

Encoder Encoder0(2,3);      //A,B Pin
Encoder Encoder1(31,33);    //A,B Pin
//Encoder Encoder2(A,B);
//Encoder Encoder3(A,B);
//Encoder Encoder4(A,B);
  const int QuadEncSig[] = {2,2};   //define wich kind of Signal you want to generate.
                                  //1= send up or down signal (typical use for selecting modes in hal)
                                  //2= send position signal (typical use for MPG wheel)
  const int QuadEncMp[] = {4,4};   //some Rotary encoders send multiple Electronical Impulses per mechanical pulse. How many Electrical impulses are send for each mechanical Latch?

#endif

//#define JOYSTICK                   //Support of an Rotating Knob that was build in my Machine. It encodes 32 Positions with 5 Pins in Binary. This will generate 32 Pins in LinuxCNC Hal.
#ifdef JOYSTICK
const int JoySticks = 1;             // Number of potentiometers connected
const int JoyStickPins[JoySticks*2] = {0, 1}; // Analog input pins for the potentiometers
const int middleValue = 512;        // Middle value of the potentiometer
const int deadband = 20;            // Deadband range around the middleValue
const float scalingFactor = 0.01;   // Scaling factor to control the impact of distanceFromMiddle
#endif
*/





//The Software will detect if there is an communication issue. When you power on your machine, the Buttons etc won't work, till LinuxCNC is running. THe StatusLED will inform you about the State of Communication.
// Slow Flash = Not Connected
// Steady on = connected
// short Flash = connection lost.

// if connection is lost, something happened. (Linuxcnc was closed for example or USB Connection failed.) It will recover when Linuxcnc is restartet. (you could also run "unloadusr arduino", "loadusr arduino" in Hal)
// Define an Pin you want to connect the LED to. it will be set as Output indipendand of the OUTPUTS function, so don't use Pins twice.
// If you use Digital LED's such as WS2812 or PL9823 (only works if you set up the DLED settings below) you can also define a position of the LED. In this case StatLedPin will set the number of the Digital LED Chain.

#define STATUSLED
#ifdef STATUSLED
  const int StatLedPin = 13;                //Pin for Status LED
  const int StatLedErrDel[] = {1000,10};   //Blink Timing for Status LED Error (no connection)
  const int DLEDSTATUSLED = 0;              //set to 1 to use Digital LED instead. set StatLedPin to the according LED number in the chain.
#endif










//#define DEBUG
//#######################################   END OF CONFIG     ###########################

//###Misc Settings###
const int timeout = 10000;   // timeout after 10 sec not receiving Stuff
const int debounceDelay = 50;

 
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

#ifdef PWMOUTPUTS
  int OutPWMState[PwmOutputs];
  int oldOutPWMState[PwmOutputs];
#endif

#ifdef AINPUTS
  int oldAinput[AInputs];
  unsigned long sumAinput[AInputs];
#endif

#ifdef LPOTIS
  int Lpoti[LPotis];
  int oldLpoti[LPotis];
#endif

#ifdef BINSEL
  int oldAbsEncState;
#endif

#ifdef KEYPAD
  byte KeyState = 0;
#endif

#ifdef MULTIPLEXLEDS
  byte KeyLedStates[numVccPins*numGndPins];
#endif

#if QUADENCS == 1
  const int QuadEncs = 1;
#endif

#if QUADENCS == 2
  const int QuadEncs = 2;
#endif

#if QUADENCS == 3
  const int QuadEncs = 3;
#endif

#if QUADENCS == 4
  const int QuadEncs = 4;
#endif

#if QUADENCS == 5
  const int QuadEncs = 5;
#endif

#ifdef QUADENC
  long EncCount[QuadEncs];
  long OldEncCount[QuadEncs];
#endif


#ifdef JOYSTICK
    long counter[JoySticks*2] = {0};      // Initialize an array for the counters
    long prevCounter[JoySticks*2] = {0};  // Initialize an array for the previous counters
    float incrementFactor[JoySticks*2] = {0.0}; // Initialize an array for the incrementFactors
    unsigned long lastUpdateTime[JoySticks*2] = {0}; // Store the time of the last update for each potentiometer
#endif

//### global Variables setup###
//Please don't touch them
unsigned long oldmillis = 0;
unsigned long newcom = 0;
unsigned long lastcom = 0;
int connectionState = 0;

#define STATE_CMD 0
#define STATE_IO 1
#define STATE_VALUE 2


unsigned char state = STATE_CMD;
unsigned char inputbuffer[5];
unsigned char bufferIndex = 0;
unsigned char cmd = 0;

uint16_t io = 0;
uint16_t value = 0;

/*
// Function Prototypes
void readCommands();
void commandReceived(char cmd, uint16_t io, uint16_t value);
void multiplexLeds();
void readKeypad();
uint8_t readAbsKnob();
void readsInputs();
void readInputs();
void readAInputs();
void readLPoti();
void controlDLED(uint8_t Pin, uint8_t Stat);
void initDLED();
void writePwmOutputs(uint8_t Pin, uint8_t Stat);
void writeOutputs(uint8_t Pin, uint8_t Stat);
void StatLedErr(uint8_t offtime, uint8_t ontime);
void flushSerial();
void sendData(char sig, uint8_t pin, uint8_t state);
void reconnect();
void comalive();
void readEncoders();
void readJoySticks();
*/

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

    /* 
    //setting Inputs with internal Pullup Resistors
    #ifdef SINPUTS
      for(uint8_t i= 0; i<sInputs;i++){
        //pinMode(sInPinmap[i], INPUT_PULLUP);
        soldInState[i] = -1;
        togglesinputs[i] = 0;
      }

    #endif
    #ifdef AINPUTS

      for(uint8_t i= 0; i<AInputs;i++){
        //pinMode(AInPinmap[i], INPUT);
        oldAinput[i] = -1;
        sumAinput[i] = 0;
        }
    #endif
    #ifdef OUTPUTS
      for(uint8_t o= 0; o<Outputs;o++){
        //pinMode(OutPinmap[o], OUTPUT);
        oldOutState[o] = 0;
        }
    #endif

    #ifdef PWMOUTPUTS
      for(int o= 0; o<PwmOutputs;o++){
        //pinMode(PwmOutPinmap[o], OUTPUT);
        oldOutPWMState[o] = 0;
        }
    #endif

    #ifdef STATUSLED
      //pinMode(StatLedPin, OUTPUT);
    #endif
    */


    /*
    #ifdef BINSEL
      pinMode(BinSelKnobPins[0], INPUT_PULLUP);
      pinMode(BinSelKnobPins[1], INPUT_PULLUP);
      pinMode(BinSelKnobPins[2], INPUT_PULLUP);
      pinMode(BinSelKnobPins[3], INPUT_PULLUP);
      pinMode(BinSelKnobPins[4], INPUT_PULLUP);
    #endif
    */


}//end setup


/*
void loop() {
    readCommands(); //receive and execute Commands
    comalive(); //if nothing is received for 10 sec. blink warning LED


    #ifdef INPUTS
      readInputs(); //read Inputs & send data
    #endif
    #ifdef SINPUTS
      readsInputs(); //read Inputs & send data
    #endif
    #ifdef AINPUTS
      readAInputs();  //read Analog Inputs & send data
    #endif
    #ifdef LPOTIS
      readLPoti(); //read LPotis & send data
    #endif
    #ifdef BINSEL
      readAbsKnob(); //read ABS Encoder & send data
    #endif

    #ifdef KEYPAD
      readKeypad(); //read Keyboard & send data
    #endif

    #ifdef QUADENC
      readEncoders(); //read Encoders & send data
    #endif

    #ifdef JOYSTICK
      readJoySticks(); //read Encoders & send data
    #endif
    #ifdef MULTIPLEXLEDS
      multiplexLeds();// cycle through the 2D LED Matrix}
    #endif
}
*/

/*  
#ifdef JOYSTICK

void readJoySticks() {
  for (int i = 0; i < JoySticks*2; i++) {
    unsigned long currentTime = millis(); // Get the current time

    // Check if it's time to update the counter for this potentiometer
    if (currentTime - lastUpdateTime[i] >= 100) { // Adjust 100 milliseconds based on your needs
      lastUpdateTime[i] = currentTime; // Update the last update time for this potentiometer

      int potValue = analogRead(JoyStickPins[i]); // Read the potentiometer value

      // Calculate the distance of the potentiometer value from the middle
      int distanceFromMiddle = potValue - middleValue;

      // Apply deadband to ignore small variations around middleValue
      if (abs(distanceFromMiddle) <= deadband) {
        incrementFactor[i] = 0.0; // Set incrementFactor to 0 within the deadband range
      } else {
        // Apply non-linear scaling to distanceFromMiddle to get the incrementFactor
        incrementFactor[i] = pow((distanceFromMiddle * scalingFactor), 3);
      }

      // Update the counter if the incrementFactor has reached a full number
      if (incrementFactor[i] >= 1.0 || incrementFactor[i] <= -1.0) {
        counter[i] += static_cast<long>(incrementFactor[i]); // Increment or decrement the counter by the integer part of incrementFactor
        incrementFactor[i] -= static_cast<long>(incrementFactor[i]); // Subtract the integer part from incrementFactor
      }

      // Check if the counter value has changed
      if (counter[i] != prevCounter[i]) {
        sendData('R',JoyStickPins[i],counter[i]);
        // Update the previous counter value with the current counter value
        prevCounter[i] = counter[i];
      }
    }
  }
}
#endif
*/

/*  
#ifdef QUADENC
void readEncoders(){
    if(QuadEncs>=1){
      #if QUADENCS >= 1
        EncCount[0] = Encoder0.read()/QuadEncMp[0];
      #endif
    }
    if(QuadEncs>=2){
      #if QUADENCS >= 2
        EncCount[1] = Encoder1.read()/QuadEncMp[1];
      #endif
    }
    if(QuadEncs>=3){
      #if QUADENCS >= 3
        EncCount[2] = Encoder2.read()/QuadEncMp[2];
      #endif
    }
    if(QuadEncs>=4){
      #if QUADENCS >= 4
        EncCount[3] = Encoder3.read()/QuadEncMp[3];
      #endif
    }
    if(QuadEncs>=5){
      #if QUADENCS >= 5
        EncCount[4] = Encoder4.read()/QuadEncMp[4];
      #endif
    }

    for(int i=0; i<QuadEncs;i++){
      if(QuadEncSig[i]==2){
        if(OldEncCount[i] != EncCount[i]){
          sendData('R',i,EncCount[i]);//send Counter
          OldEncCount[i] = EncCount[i];
        }
      }
      if(QuadEncSig[i]==1){
        if(OldEncCount[i] < EncCount[i]){
        sendData('R',i,1); //send Increase by 1 Signal
        OldEncCount[i] = EncCount[i];
        }
        if(OldEncCount[i] > EncCount[i]){
        sendData('R',i,0); //send Increase by 1 Signal
        OldEncCount[i] = EncCount[i];
        }
      }
    }
}

#endif
*/

/*  
void comalive(){
  if(lastcom == 0){ //no connection yet. send E0:0 periodicly and wait for response
    while (lastcom == 0){
      readCommands();
      flushSerial();
      Serial.println("E0:0");
      delay(200);
      #ifdef STATUSLED
        StatLedErr(1000,1000);
      #endif
    }
    connectionState = 1;
    flushSerial();
    #ifdef DEBUG
      Serial.println("first connect");
    #endif
  }
  if(millis() - lastcom > timeout){
  #ifdef STATUSLED
     StatLedErr(500,200);
  #endif
      if(connectionState == 1){
        #ifdef DEBUG
          Serial.println("disconnected");
        #endif
        connectionState = 2;
      }

   }
   else{
      connectionState=1;
      #ifdef STATUSLED
        if(DLEDSTATUSLED == 1){
          #ifdef DLED
            controlDLED(StatLedPin, 1);
          #endif
        }
        else{
          digitalWrite(StatLedPin, HIGH);
        }
      #endif
    }
}


void reconnect(){
  #ifdef DEBUG
    Serial.println("reconnected");
    Serial.println("resending Data");
  #endif

  #ifdef INPUTS
    for (int x = 0; x < Inputs; x++){
      InState[x]= -1;
    }
  #endif
  #ifdef SINPUTS
    for (int x = 0; x < sInputs; x++){
      soldInState[x]= -1;
      togglesinputs[x] = 0;
    }
  #endif
  #ifdef AINPUTS
    for (int x = 0; x < AInputs; x++){
      oldAinput[x] = -1;
      sumAinput[x] = 0;
    }
  #endif
  #ifdef LPOTIS
    for (int x = 0; x < LPotis; x++){
      oldLpoti[x] = -1;
    }
  #endif
  #ifdef BINSEL
    oldAbsEncState = -1;
  #endif


  #ifdef INPUTS
    readInputs(); //read Inputs & send data
  #endif
  #ifdef SINPUTS
    readsInputs(); //read Inputs & send data
  #endif
  #ifdef AINPUTS
    readAInputs();  //read Analog Inputs & send data
  #endif
  #ifdef LPOTIS
    readLPoti(); //read LPotis & send data
  #endif
  #ifdef BINSEL
    readAbsKnob(); //read ABS Encoder & send data
  #endif
  #ifdef MULTIPLEXLEDS
    multiplexLeds(); //Flash LEDS.
  #endif

  connectionState = 1;


}
*/

/*
void sendData(char sig, int pin, int state){
        Serial.print(sig);
        Serial.print(pin);
        Serial.print(":");
        Serial.println(state);
}

void flushSerial(){
  while (Serial.available() > 0) {
  Serial.read();
  }
}
*/



/*
#ifdef OUTPUTS
void writeOutputs(int Pin, int Stat){
  digitalWrite(Pin, Stat);
}
#endif

#ifdef PWMOUTPUTS
void writePwmOutputs(int Pin, int Stat){
  analogWrite(Pin, Stat);
}

#endif
*/


/*
#ifdef LPOTIS
void readLPoti(){
    for(int i= 0;i<LPotis; i++){
      int var = analogRead(LPotiPins[i][0])+margin;
      int pos = 1024/(LPotiPins[i][1]-1);
      var = var/pos;
      if(oldLpoti[i]!= var){
        oldLpoti[i] = var;
        sendData('L', LPotiPins[i][0],oldLpoti[i]);
      }
    }
}
#endif
*/

/*
#ifdef AINPUTS
void readAInputs(){
  static unsigned int samplecount = 0;

   for(int i= 0;i<AInputs; i++){

    if (samplecount < smooth) {
      sumAinput[i] = sumAinput[i] + analogRead(AInPinmap[i]);
    }
    else {
      sumAinput[i] = sumAinput[i] / smooth;
      if(oldAinput[i]!= sumAinput[i]){
        oldAinput[i] = sumAinput[i];
        sendData('A',AInPinmap[i],oldAinput[i]);
      }
      sumAinput[i] = 0;
    }
   }

  if(samplecount < smooth){ samplecount = samplecount + 1;}
  else {samplecount = 0;}

}
#endif
*/

/*  
#ifdef INPUTS
void readInputs(){
    for(int i= 0;i<Inputs; i++){
      int State = digitalRead(InPinmap[i]);
      if(InState[i]!= State && millis()- lastInputDebounce[i] > debounceDelay){
        InState[i] = State;
        sendData('I',InPinmap[i],InState[i]);

      lastInputDebounce[i] = millis();
      }
    }
}
#endif
#ifdef SINPUTS
void readsInputs(){
  for(int i= 0;i<sInputs; i++){
    sInState[i] = digitalRead(sInPinmap[i]);
    if (sInState[i] != soldInState[i] && millis()- lastsInputDebounce[i] > debounceDelay){
      // Button state has changed and debounce delay has passed

      if (sInState[i] == LOW || soldInState[i]== -1) { // Stuff after || is only there to send States at Startup
        // Button has been pressed
        togglesinputs[i] = !togglesinputs[i];  // Toggle the LED state

        if (togglesinputs[i]) {
          sendData('I',sInPinmap[i],togglesinputs[i]);  // Turn the LED on
        }
        else {
          sendData('I',sInPinmap[i],togglesinputs[i]);   // Turn the LED off
        }
      }
      soldInState[i] = sInState[i];
      lastsInputDebounce[i] = millis();
    }
  }
}
#endif
*/

/*  
#ifdef BINSEL
int readAbsKnob(){
  int var = 0;
  if(digitalRead(BinSelKnobPins[0])==1){
    var += 1;
  }
  if(digitalRead(BinSelKnobPins[1])==1){
    var += 2;
  }
  if(digitalRead(BinSelKnobPins[2])==1){
    var += 4;
  }
  if(digitalRead(BinSelKnobPins[3])==1){
    var += 8;
  }
  if(digitalRead(BinSelKnobPins[4])==1){
    var += 16;
  }
  if(var != oldAbsEncState){
    Serial.print("K0:");
    Serial.println(var);
    }
  oldAbsEncState = var;
  return (var);
}
#endif
*/

/*  
#ifdef KEYPAD
void readKeypad(){
  //detect if Button is Pressed
  for (int col = 0; col < numCols; col++) {
    pinMode(colPins[col], OUTPUT);
    digitalWrite(colPins[col], LOW);
    // Read the state of the row pins
    for (int row = 0; row < numRows; row++) {
      pinMode(rowPins[row], INPUT_PULLUP);
      if (digitalRead(rowPins[row]) == LOW && lastKey != keys[row][col]) {
        // A button has been pressed
        sendData('M',keys[row][col],1);
        lastKey = keys[row][col];
        break;

      }
      if (digitalRead(rowPins[row]) == HIGH && lastKey == keys[row][col]) {
        // The Last Button has been unpressed
        sendData('M',keys[row][col],0);
        lastKey = -1; //reset Key pressed
        break;
      }
    }

    // Set the column pin back to input mode
    pinMode(colPins[col], INPUT);
  }

}
#endif
*/


/*
void commandReceived(char cmd, uint16_t io, uint16_t value){
  #ifdef OUTPUTS
  if(cmd == 'O'){
    writeOutputs(io,value);
    lastcom=millis();

  }
  #endif
  #ifdef PWMOUTPUTS
  if(cmd == 'P'){
    writePwmOutputs(io,value);
    lastcom=millis();

  }
  #endif
  #ifdef DLED
  if(cmd == 'D'){
    controlDLED(io,value);
    lastcom=millis();
    #ifdef debug
      Serial.print("DLED:");
      Serial.print(io);
      Serial.print(" State:");
      Serial.println(DLEDstate[io]);
    #endif

  }
  #endif
  #ifdef MULTIPLEXLEDS
    if(cmd == 'M'){
      ledStates[io] = value; // Set the LED state
      lastcom=millis();
      #ifdef DEBUG
        Serial.print("multiplexed Led No:");
        Serial.print(io);
        Serial.print("Set to:");
        Serial.println(ledStates[io]);
      #endif

  }
  #endif


  if(cmd == 'E'){
    lastcom=millis();
    if(connectionState == 2){
     reconnect();
    }
  }


  #ifdef DEBUG
    Serial.print("I Received= ");
    Serial.print(cmd);
    Serial.print(io);
    Serial.print(":");
    Serial.println(value);
  #endif
}


void readCommands(){
    byte current;
    while(Serial.available() > 0){
        current = Serial.read();
        switch(state){
            case STATE_CMD:
                   cmd = current;
                   state = STATE_IO;
                   bufferIndex = 0;
                break;
            case STATE_IO:
                if(isDigit(current)){
                    inputbuffer[bufferIndex++] = current;
                }else if(current == ':'){
                    inputbuffer[bufferIndex] = 0;
                    io = atoi(inputbuffer);
                    state = STATE_VALUE;
                    bufferIndex = 0;

                }
                else{
                    #ifdef DEBUG
                    Serial.print("Ungültiges zeichen: ");
                    Serial.println(current);
                    #endif
                }
                break;
            case STATE_VALUE:
                if(isDigit(current)){
                    inputbuffer[bufferIndex++] = current;
                }
                else if(current == '\n'){
                    inputbuffer[bufferIndex] = 0;
                    value = atoi(inputbuffer);
                    commandReceived(cmd, io, value);
                    state = STATE_CMD;
                }
                else{
                  #ifdef DEBUG
                  Serial.print("Ungültiges zeichen: ");
                  Serial.println(current);
                  #endif

                }
                break;
        }

    }
}
*/

/******************************************************/
/******************************************************/
/******************************************************/
/******************************************************/


int main (void)
{
    setup();

    DDRB = 0xff;     

    USART_Init(MYUBRR);
   
    while (1)
    {
     

    }
    
} 

/******************************************************/



