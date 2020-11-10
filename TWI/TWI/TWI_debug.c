#include <stdio.h>
#include "TWI_debug.h"
#include "TWI_funcs.h"
#include "board.h"
#include "UART_Print.h"
void receive_debug()
{
  uint8_t timeout_value = 10;
  uint8_t index=timeout_value;
  uint8_t error_status;
  uint8_t prnt_buffer [2];
  //should return 0xAC, try 10 times
  do
  {
    error_status = TWI_Master_Receive(&TWI1, 0x43, 0 ,0 ,3,array_name);
    i--;
  }while((error_status!=no_errors)&&(i!=0));
  //print to verify
  sprintf(prnt_buffer, "received value = %2.2bX\n\r", array_name[2]);
  UART_Transmit_String(&UART1,0,prnt_buffer);
  
  return;
}
