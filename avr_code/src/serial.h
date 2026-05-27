#ifndef _LCNC_UART_
#define _LCNC_UART_


//UART STUFF
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1


#define BIT_ON 0x30 //logic high
#define BIT_OFF 0x31 //logic low



void USART_Init( unsigned int ubrr);
static uint8_t USART_receive(void);
void USART_Transmit( unsigned char data );
void print_byte( uint8_t data);
void print_byte_16( uint16_t data);
void sendData(char sig, int pin, int state);
void flushSerial();



#endif

