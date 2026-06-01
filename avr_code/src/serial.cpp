
#include <avr/io.h>
#include <string.h>

#include <avr/pgmspace.h>

#include "common.h"
#include "serial.h"



#define BUFFER_SIZE 100


/*
    begin()
    print()
    println()
    read()

    if(Serial)
    available()
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
*/


/***********************************************/


void UART_write_str(const char *data) 
{ 

    while(*data)
    { 
        while ( !( UCSR0A & (1<<UDRE0)) );
        UDR0 = *data++; 
    }

} 
  

/******/
void UART_write_str(unsigned char *data) 
{ 

    while(*data)
    { 
        while ( !( UCSR0A & (1<<UDRE0)) );
        UDR0 = *data++; 
    }

} 


/******/
void UART_write_str(char *data) 
{ 
    int i =0;
    while (data[i] != 0x00)
    {
        UART_transmit(data[i]);
        i++;
    }

}
 

/***********************************************/
//this sends data from flash to uart 

void UART_write_str_pgm(const char* s)
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
void println( const char* data)
{
    UART_write_str(data);
    UART_transmit(0x0a);
    UART_transmit(0x0d);
}


void println( unsigned char* data)
{
    UART_write_str(data);
    UART_transmit(0x0a);
    UART_transmit(0x0d);
}

void println( char data)
{
    UART_transmit(data);
    UART_transmit(0x0a);
    UART_transmit(0x0d);
}


void print( const char* data)
{
    UART_write_str(data);
}


void print( unsigned char* data)
{
    UART_write_str(data);
}

void print( char data)
{
    UART_transmit(data);
}


/***********************************************/
void USART_Init( unsigned int ubrr)
{

    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;

    //Enable receiver and transmitter 
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

    //UCSR0A = (1 << U2X0); // enable 2x mode

}

/***********************************************/
void UART_transmit16( uint16_t data )
{


    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data >> 8;
    
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data & 0xff;;

}

/***********************************************/

void UART_transmit( unsigned char data )
{
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data;
}


/***********************************************/
char UART_receive(void)
{
    while (!(UCSR0A & (1 << RXC0))) {}
    return UDR0;

}


/***********************************************/
uint16_t UART_receive_stream(unsigned char * pdata)
{
    uint16_t numbytes = 0;

    while (!(UCSR0A & (1 << RXC0))) {}

    *pdata = UDR0;
    pdata++; numbytes++;         

    return numbytes;
}



/***********************************************/

void print_byte( uint8_t data)
{
   uint8_t i = 0;

   for (i=0; i<=7; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (7 - i))) ){  // MSB
           UART_transmit( BIT_OFF );
       }else{
           UART_transmit( BIT_ON );
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
           UART_transmit( BIT_OFF );
       }else{
           UART_transmit( BIT_ON );
       }
    }
}


/***********************************************/
//specific to the connector tool 
void send_data(char sig, int pin, int state)
{
    UART_transmit(sig);
    UART_transmit(pin);
    UART_transmit(':');
    UART_transmit(state);
    UART_transmit('\n');
}

/***********************************************/
void flushSerial()
{
    //while (Serial.available() > 0) {
    //Serial.read();
    //}
}




/*
int uart_putchar(char, FILE *);
int uart_getchar(FILE *);
void uart_init(unsigned int);

typedef uint8_t rbuf_data_t;
typedef uint8_t rbuf_count_t;

typedef struct {
    rbuf_data_t buffer[80];
    rbuf_data_t *in;
    rbuf_data_t *out;
    rbuf_count_t    count;
} rbuf_t;

void rbuf_init(rbuf_t *);
rbuf_count_t rbuf_getcount(rbuf_t *);
void rbuf_insert(rbuf_t *, const rbuf_data_t);
rbuf_data_t rbuf_remove(rbuf_t *);

volatile unsigned char rbuf_isempty(rbuf_t *);
volatile unsigned char command;      // BOOL Command line active? 
volatile unsigned char quit_early;   // BOOL Abort processing. 

rbuf_t  rbuf;
char line[BUFFER_SIZE];

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
*/




/*

void rbuf_init(rbuf_t* const buffer)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        buffer->in    = buffer->buffer;
        buffer->out   = buffer->buffer;
        buffer->count = 0;
    }
    return;
}


rbuf_count_t rbuf_getcount(rbuf_t* const buffer)
{
    rbuf_count_t count;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        count = buffer->count;
    }
    return count;
}


bool rbuf_isempty(rbuf_t* buffer)
{
    return (rbuf_getcount(buffer) == 0);
}


void rbuf_insert(rbuf_t* const buffer, const rbuf_data_t data)
{
    *buffer->in = data;

    if (++buffer->in == &buffer->buffer[BUFFER_SIZE])
        buffer->in = buffer->buffer;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        buffer->count++;
    }
}


rbuf_data_t rbuf_remove(rbuf_t* const buffer)
 {
    rbuf_data_t data = *buffer->out;

    if (++buffer->out == &buffer->buffer[BUFFER_SIZE])
        buffer->out = buffer->buffer;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        buffer->count--;
    }

    return data;
}


ISR (USART0_RX_vect)
{
    uint8_t c;
    c = UDR0;
    // If command line is active, store the character. 
    if (command)
        rbuf_insert(&rbuf, (rbuf_data_t) c);
    else {  // Otherwise check to see if we need to abort. 
        if (c == 0x03)
            quit_early = TRUE;
    }

    return;
}



//Receive a character from the UART Rx.
int uart_getchar(FILE *stream)
{
    uint8_t c;

    while (rbuf_isempty(&rbuf)); // block until something's there 
    c = rbuf_remove(&rbuf);
    return c;
}


 
// Initialize the UART to baud/bps, tx/rx, 8N1.
 
void uart_init(unsigned int baud)
{
    rbuf_init(&rbuf);

    #if F_CPU < 2000000UL && defined(U2X)
        UCSR0A = _BV(U2X);  // improve baud rate error by using 2x clk 
        UBRR0L = (F_CPU / (8UL * baud)) - 1;
    #else
        UBRR0L = (F_CPU / (16UL * baud)) - 1;
    #endif

    // Clear error flags, MODbus protocol: 
    UCSR0A=0x00;
    // Enable TX, RX, and RX interrupt 
    UCSR0B = (1<<TXEN0) | (1<<RXEN0) | (1<<RXCIE0);

    stdout = stdin = &uart_str;

    return;
}


// Send character c down the UART Tx, wait until tx holding register is empty.

int uart_putchar(char c, FILE *stream)
{
    if (c == '\a') {
        fputs("*ring*\n", stderr);
        return 0;
    }

    if (c == '\n')
        uart_putchar('\r', stream);
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;

    return 0;
}






int main(void)
{
    int i;
    char c;

    sei();
    uart_init(UART_BAUD);

    printf_P(PSTR("\n\nHello World!\nType a line and see it printed back.\n"));
    for (;;) {

        command = TRUE;
        printf_P(PSTR("> "));

        c = fgetc(stdin);
        i = 0;
        while (i < sizeof(line)) {
            if ((c == '\n') || (c == '\r')) {
                line[i] = 0;
                break;
            }

            line[i] = c;
            i++;
            printf("%c", c);
            c = fgetc(stdin);
        }
        command = FALSE;

        printf_P(PSTR("\n- "));

        for (i = 0; i < strlen(line); i++) {
            if (quit_early) {
                printf_P(PSTR(" ABORT"));
                break;
            }
            putchar(line[i]);
            _delay_ms(300);
        }
        printf_P(PSTR("\n"));
        quit_early = FALSE;
    }
    printf_P(PSTR("\nSomething barfed.\n"));
    return 0;
}



*/