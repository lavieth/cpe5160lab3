#include "TWI_debug.h"

void transmit_receive_debug()
{
  uint8_t timeout_value = 10;
  uint8_t index=timeout_value;
  uint8_t error_status;
  uint8_t prnt_buffer [2];
  uint8_t array_name[4];

  //TODO FOR TESTING, TRY JUST RECEIVE FIRST, THEN UNCOMMENT TRANSMIT BLOCK
  //transmit sets internal addr to 1, receive reads the value and it should be
  //0xAC 
  /*do
  {
    error_status=TWI_Master_Transmit(&TWI1, 0x43, 0x01,1, 0,array_name);
    index--;
  }while((error_status!=no_errors)&&(index!=0));*/
  
  //print to verify
  //If receive only has one value to read, it is repeatedly printed out. It's the 
  //correct value but something is wrong for it to loop like that
  do
  {
     //this is for first try, with receive by itself
     error_status = TWI_Master_Receive(&TWI1, 0x43, 0 ,0 ,3,array_name);
     //for try with transmit running first
     //error_status = TWI_Master_Receive(&TWI1, 0x43, 0 ,0 ,1,array_name);
     index--;
  }while((error_status!=no_errors)&&(index!=0));

  sprintf(prnt_buffer, "received value = %2.2X\n\r", array_name[2]);
  //sprintf(prnt_buffer, "received value = %2.2X\n\r", array_name[0]);
  UART_Transmit_String(&UART1,0,prnt_buffer); 
  return;
}


