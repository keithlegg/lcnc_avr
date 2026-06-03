
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





 /*



ring_buffer_size_t usart0_recv_queue_size(void) 
{
  ring_buffer_size_t result;
  // Prevent race conditions 
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    result = ring_buffer_num_items(&usart0_recv_ring_buf);
  }
  return result;
}


ring_buffer_size_t usart0_recv_dequeue(char *data) 
{
  ring_buffer_size_t result;
  // Prevent race conditions 
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
      result = ring_buffer_dequeue(&usart0_recv_ring_buf, data);
  }
  return result;
}


ring_buffer_size_t usart0_recv_peek(char *data, ring_buffer_size_t index) {
  ring_buffer_size_t result;
  // Prevent race conditions 
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    result = ring_buffer_peek(&usart0_recv_ring_buf, data, index);
  }
  return result;
}

void usart0_send(char data) {
  // Prevent race conditions 
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    // Add to queue 
    ring_buffer_queue(&usart0_send_ring_buf, data);
    // Enable data register empty interrupt 
    UCSR0B |= (1 << UDRIE0);
  }
}

void usart0_send_arr(const char *data, ring_buffer_size_t size) {
  // Prevent race conditions 
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    ring_buffer_queue_arr(&usart0_send_ring_buf, data, size);
    // Enable data register empty interrupt 
    UCSR0B |= (1 << UDRIE0);
  }
}

usart_desc_t *get_usart0_descriptor(void) {
  static usart_desc_t descriptor =
    {
      .usart_recv_queue_size = usart0_recv_queue_size,
      .usart_recv_dequeue = usart0_recv_dequeue,
      .usart_recv_peek = usart0_recv_peek,
      .usart_send = usart0_send,
      .usart_send_arr = usart0_send_arr
    };
  return &descriptor;
}
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


/******/
void uart_write_str(char *data) 
{ 
    int i =0;
    while (data[i] != 0x00)
    {
        uart_transmit(data[i]);
        i++;
    }

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
void uart_transmit16( uint16_t data )
{


    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data >> 8;
    
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data & 0xff;;

}

/***********************************************/

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
uint16_t uart_receive_stream(unsigned char * pdata)
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


/***********************************************/
//specific to the connector tool 
void send_data(char sig, int pin, int state)
{
    uart_transmit(sig);
    uart_transmit(pin);
    uart_transmit(':');
    uart_transmit(state);
    uart_transmit('\n');
}

/***********************************************/
void flushSerial()
{
    //while (Serial.available() > 0) {
    //Serial.read();
    //}
}








/////////////////////////////////////////////////////////

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


