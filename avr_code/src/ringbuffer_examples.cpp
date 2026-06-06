///////////////////////////////////////////////////////

/*
// RINGBUFFER WITH UART EXAMPLE 
// RECEIVE 3 bytes into a queue, print them and reset cache 

unsigned char current[100] = "";
uint16_t num = 0;
char tmp;
int main(void)
{
    uint8_t i, cnt;

    init_uart(MYUBRR); //inits ringbuffer and interrupts too 

    init_debug_led();

    while(1)
    {
        //uart_transmit(ring_buffer_num_items(&usart0_recv_ring_buf));

        if(ring_buffer_num_items(&usart0_recv_ring_buf)==3)
        {
            //debug_led();
            //for(uint8_t x=0;x<ring_buffer_num_items(&usart0_recv_ring_buf);x++)
            //{  
            //    ring_buffer_peek(&usart0_recv_ring_buf, &tmp, x);
            //    print( tmp);
            //}


            while(ring_buffer_dequeue(&usart0_recv_ring_buf, &tmp) > 0) 
            {
                print( tmp);
            }
        }
        
        //arbitrary pause to let things settle 
        _delay_ms(10);
    } 
}
*/ 

///////////////////////////////////////////////////////

/*
//RINGBUFFER EXAMPLES 
int main(void) 
{

  init_uart(MYUBRR);

  int i, cnt;
  char tmp;
  char tmp_arr[50];
  
  // Create and initialize ring buffer 
  // The size of the memory buffer must be a power-of-two, 
  // the ring buffer can contain at most `buf_size-1` bytes.

  ring_buffer_t ring_buffer;
  char buf_arr[128];
  ring_buffer_init(&ring_buffer, buf_arr, sizeof(buf_arr));
  
  // Add elements to buffer; one at a time 
  for(i = 0; i < 100; i++) {
      ring_buffer_queue(&ring_buffer, i);
  }

  // Verify size 
  assert(ring_buffer_num_items(&ring_buffer) == 100);

  // Peek third element 
  cnt = ring_buffer_peek(&ring_buffer, &tmp, 3);
  // Assert byte returned 
  assert(cnt == 1);
  // Assert contents 
  assert(tmp == 3);

  // Dequeue all elements 
  for(cnt = 0; ring_buffer_dequeue(&ring_buffer, &tmp) > 0; cnt++) 
  {
      // Do something with buf... 
      assert(tmp == cnt);
      println( tmp);
  }

  // Add array 
  ring_buffer_queue_arr(&ring_buffer, "Hello, Ring Buffer!", 20);

  // Is buffer empty? 
  assert(!ring_buffer_is_empty(&ring_buffer));

  // Dequeue all elements 
  while(ring_buffer_dequeue(&ring_buffer, &tmp) > 0) 
  {
      // Print contents 
      //printf("Read: %c\n", tmp);
  }
  
  // Add new array 
  ring_buffer_queue_arr(&ring_buffer, "Hello again, Ring Buffer!", 26);
  
  // Dequeue array in two parts 
  println("Read:\n");
  cnt = ring_buffer_dequeue_arr(&ring_buffer, tmp_arr, 13);
  println( cnt);
  assert(cnt == 13);

  // Add \0 termination before printing 
  tmp_arr[13] = '\0';
  println( tmp_arr);
  
  // Dequeue remaining 
  cnt = ring_buffer_dequeue_arr(&ring_buffer, tmp_arr, 13);
  assert(cnt == 13);
  
  println(tmp_arr);

  // Overfill buffer 
  for(i = 0; i < 1000; i++) {
      ring_buffer_queue(&ring_buffer, (i % 127));
  }
  
  // Is buffer full? 
  if(ring_buffer_is_full(&ring_buffer)) 
  {
      cnt = ring_buffer_num_items(&ring_buffer);
      print("Buffer is full and contains ");
      print_byte(cnt);
      println(" bytes\n");
    
  }
  
  // Dequeue all elements 
  while(ring_buffer_dequeue(&ring_buffer, &tmp) > 0) 
  {
      // Print contents 
      print("Read: ");
      println(tmp);
  }
  
  return 0;
}
*/


