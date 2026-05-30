#ifndef _LCNC_UART_
#define _LCNC_UART_


//UART STUFF

//#define BAUD 115200UL          //THIS WONT WORK FOR 115200
//#define MYUBRR FOSC/16/BAUD-1  //THIS WONT WORK FOR 115200

#define MYUBRR 16000000/16/(115200-1) //THIS DOES WORK FOR 115200


#define BIT_ON 0x30 //logic high
#define BIT_OFF 0x31 //logic low

#define TRUE    1
#define FALSE   0


void UART_write_str_pgm(const char s[]);
void UART_write_str(char *data);

void UART_write_str2(char *data);



void USART_Init( unsigned int ubrr);

uint8_t UART_receive(void);
void UART_transmit( unsigned char data );

void print_byte( uint8_t data);
void print_byte_16( uint16_t data);
void sendData(char sig, int pin, int state);
void flushSerial();



#endif

