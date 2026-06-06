
#include "common.h"

#include <avr/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

//#include <avr/interrupt.h>
#include <util/atomic.h>


 

#include <string.h>


//#include "usart0.h"
#include "serial.h"



#define BUFFER_SIZE 100



// Ring buffer for received data.
ring_buffer_t usart0_recv_ring_buf;

//Ring buffer for data to send.
ring_buffer_t usart0_send_ring_buf;


char rx_buf_arr[128];
char tx_buf_arr[128];



/*
    begin()
    print()
    println()
    read()
    available()


    if(Serial)
    availableForWrite()
    end()
    find()
    findUntil()
    flush()
    parseFloat()
    parseInt()
    peek()
    readBytes()
    readBytesUntil()
    readString()
    readStringUntil()
    setTimeout()
    write()
    serialEvent()

    ///////////////////
    //directly manipulate the bits in your C code:

    Check if ready to send a byte     : while (!(UCSR1A & (1 << UDRE1)));
    Send data byte                    : UDR1 = my_data_byte;
    Wait for transmission to finish   : while (!(UCSR1A & (1 << TXC1)));
    Clear Transmit Complete Flag      : UCSR1A |= (1 << TXC1);

*/



/***********************************************/
void init_debug_led(void)
{
    DDRB |= (1<<7);
}

/***********************************************/
void init_uart( unsigned int ubrr)
{
      cli();

      // Initialize ring buffers 
      ring_buffer_init(&usart0_recv_ring_buf, rx_buf_arr, sizeof(rx_buf_arr));
      ring_buffer_init(&usart0_send_ring_buf, tx_buf_arr, sizeof(tx_buf_arr));

      // enabling interrupts, etc.
      UCSR0A = 0;
      
      // Enable USART0 TX and RX 
      UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
      
      // Async USART, 8bit, no parity and 1 stop bit 
      UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
    
      UBRR0H = (unsigned char)(ubrr>>8);
      UBRR0L = (unsigned char)ubrr;

 
      sei();

      // enable 2x mode
      //UCSR0A = (1 << U2X0); 

}

/***********************************************/
void flush_serial()
{   
    char tmp;
    while(ring_buffer_dequeue(&usart0_recv_ring_buf, &tmp) > 0) 
    { }

}


/***********************************************/
//no need to call - this was put into uart_init()
void init_rx_interrupts(void)
{

   // Enable TX, RX, and RX interrupt 
   //UCSR0B = (1<<TXEN0) | (1<<RXEN0) | (1<<RXCIE0);
   
   //USART0_UDRE_vect

   UCSR0B = (1<<RXEN0) | (1<<RXCIE0);
   //UCSR0B |= (1<<RXCIE0);  

}


/***********************************************/
void debug_led(void)
{
    PORTB |= (1<<7);
    _delay_ms(100);

    PORTB &= !(1<<7);
    _delay_ms(100);

}


/***********************************************/
 
void uart_write_str(char *data) 
{ 
    int i =0;
    while (data[i] != 0x00)
    {
        uart_transmit(data[i]);
        i++;
    }

}

void uart_write_str(const char *data) 
{ 

    while(*data)
    { 
        while ( !( UCSR0A & (1<<UDRE0)) );
        UDR0 = *data++; 
    }

} 
  

/******/
void uart_write_str(unsigned char *data) 
{ 

    while(*data)
    { 
        while ( !( UCSR0A & (1<<UDRE0)) );
        UDR0 = *data++; 
    }

} 



 




/***********************************************/
void println( const char* data)
{
    uart_write_str(data);
    uart_transmit(0x0a);
    uart_transmit(0x0d);
}


void println( unsigned char* data)
{
    uart_write_str(data);
    uart_transmit(0x0a);
    uart_transmit(0x0d);
}

void println( char data)
{
    uart_transmit(data);
    uart_transmit(0x0a);
    uart_transmit(0x0d);
}


void print( const char* data)
{
    uart_write_str(data);
}

void print( unsigned char* data)
{
    uart_write_str(data);
}

void print( char data)
{
    uart_transmit(data);
}



/***********************************************/
//send two 8 bit bytes from a single 16 
void uart_transmit16( uint16_t data )
{
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data >> 8;
    
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data & 0xff;;

}

/***********************************************/

// "encode" ascii from its numeric value 
void transmit_ascii_digit( uint8_t a )
{ 
    //0-9 ascii charaters are : decimal 48 - 57  
    //if (a >= '0' && a <= '9')

    uint8_t digit = a - '0'; //'0' is 0x30 , decimal 48
    //print(a);

    uart_transmit(a);

}


/***********************************************/
// send 3 (8 bit unsigned integer) represeting an ascii char encoding value
void send_ascii_decimal( uint16_t num )
{ 
    uint16_t n = num;

    uint8_t tenthous,thous,hundreds,tens,ones = 0;
    uint8_t cnt,sum,rem = 0;

    //uint8_t hi = num >> 8;   
    //uint8_t lo = num & 0xFF;

    //chop it up, powers of 10 (up to numeric 128) 
    while(n!=0)
    {
        rem = n%10;
        if(cnt==0){ones = rem;}
        if(cnt==1){tens = rem;}
        if(cnt==2){hundreds = rem;}            
        if(cnt==3){thous = rem;} 
        if(cnt==4){tenthous = rem;} 

        n=n/10;
        cnt++;
    } 
    
    //get the original value (it got decomposed above)
    n = num;
     
    //add 48 to encode into ascii numbers 
    if(n>=10000)
    {
        transmit_ascii_digit(48+tenthous);
    } 
    if(n>=1000)
    {
        transmit_ascii_digit(48+thous);
    } 
    if(n>=100)
    {
        transmit_ascii_digit(48+hundreds);
    } 
    if(n>=10)
    {
        transmit_ascii_digit(48+tens);
    }
    transmit_ascii_digit(48+ones);
    
}

/***********************************************/
// send 3 (8 bit unsigned integer) represeting an ascii char encoding value
void send_ascii_decimal( char* a )
{ 
    uint8_t hundreds,tens,ones = 0;
    uint8_t cnt,sum,rem = 0;
    
    //cast to int 
    uint8_t n = ((uint8_t)*a);
    
    //chop it up, powers of 10 (up to numeric 128) 
    while(n!=0)
    {
        rem = n%10;
        if(cnt==0){ones = rem;}
        if(cnt==1){tens = rem;}
        if(cnt==2){hundreds = rem;}            
        n=n/10;
        cnt++;
    } 
    
    //get the original value (it got decomposed above)
    n = ((uint8_t)*a);

    //add 48 to encode into ascii numbers 
    if(n>=100)
    {
        transmit_ascii_digit(48+hundreds);
    } 
    if(n>=10)
    {
        transmit_ascii_digit(48+tens);
    }
    transmit_ascii_digit(48+ones);
    
}


/***********************************************/
// send 3 (8 bit unsigned integer) represeting an ascii char encoding value
void send_ascii_decimal( const char* a )
{ 
    send_ascii_decimal( (char *)a );

}


/***********************************************/
/**
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 
 //https://stackoverflow.com/questions/28662696/convert-hexadecimal-to-decimal-in-avr-studio

uint32_t hex2int(char *hex) {
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        char byte = *hex++; 
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        // shift 4 to make space for new digit, and add the 4 bits of the new digit 
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}
*/




/***********************************************/
/*
void uart_transmit( char data )
{
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data;
}
*/

void uart_transmit( unsigned char data )
{
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data;
}


/***********************************************/
char uart_receive(void)
{
    while (!(UCSR0A & (1 << RXC0))) {}
    return UDR0;

}

/***********************************************/
//this sends data from flash to uart 

void uart_write_str_pgm(const char* s)
{
    uint8_t c;

    for (uint8_t i=0; i < strlen_P(s); i++)
    {
        c = pgm_read_byte(&(s[i]));
        while (( UCSR0A & (1<<UDRE0))  == 0){};
        UDR0 = c;          
    }

}  


/***********************************************/

void print_byte( uint8_t data)
{
   uint8_t i = 0;

   for (i=0; i<=7; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (7 - i))) ){  // MSB
           uart_transmit( BIT_OFF );
       }else{
           uart_transmit( BIT_ON );
       }
    }
}

/***********************************************/
void print_byte_16( uint16_t data)
{
   uint8_t i = 0;

   for (i=0; i<=15; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (15 - i))) ){  // MSB
           uart_transmit( BIT_OFF );
       }else{
           uart_transmit( BIT_ON );
       }
    }
}











