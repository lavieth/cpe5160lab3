/*
 * STA013_funcs.c
 *
 * Created: 11/8/2020 9:27:09 PM
 *  Author: snspzv
 */ 

#include "board.h"
#include "UART_Print.h"
#include "TWI_funcs.h"

uint8_t STA013_init(uint8_t data_array[])
{
	uint8_t error_value = no_errors;
	uint8_t i = 20;
	//uint8_t data_array[5];
	uint8_t * print_buffer;
	
	do 
	{
		error_value = TWI_Master_Receive(&TWI1, 0x43, 0, 0, 3, data_array);
		i--;
	} while ((error_value != no_errors) && (i != 0));
	
	sprintf(print_buffer, "Received Value = %2.2bX\n\r", data_array[2]);
	UART_Transmit_String(&UART1, 0, print_buffer);
	return error_value;
}
