

#include "ringbuffer.h"



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


//---------------
//my attempt to use ringbuffer library here 

ring_buffer_size_t usart0_recv_queue_size(void);
ring_buffer_size_t usart0_recv_dequeue(char *data);
 

void uart_transmit_asint( char* data );

void transmit_ascii_digit( uint8_t a );

void transmit_digit_ascii( uint16_t a );
void transmit_digit_ascii( const char* a );






//---------------

void init_debug_led(void);
void init_rx_interrupts(void);
void init_uart( unsigned int ubrr);


void debug_led( void );



void uart_transmit16( uint16_t data );

//void uart_transmit( unsigned char* data );
void uart_transmit( unsigned char data );
//void uart_transmit( char data );
void uart_transmit_c( char data );

//void UART_write_str(char data); 
void uart_write_str(char *data);
void uart_write_str(unsigned char data); 
void uart_write_str(const char *data);

void uart_write_str_pgm(const char s[]);

void print( const char* data);
void print( unsigned char* data);
void print( char data);

void println( char data);
void println( unsigned char* data);
void println( const char* data);

void print_byte( uint8_t data);
void print_byte_16( uint16_t data);

char uart_receive(void);

//uint16_t uart_receive_stream(unsigned char * pdata);
void flush_serial();











#endif

