/*
 * TWI.c
 *
 * Created: 11/8/2020 11:29:11 AM
 * Author : snspzv
 */ 

#include <avr/io.h>
#include "TWI_funcs.h"
#include "UART.h"
#include "TWI_debug.h"
#define TWI_FREQ (16000000)


int main(void)
{
	uint8_t error_status;
	//initialize port c as output for UART
	Output_Init(&PC, 0x80);
	UART_Init(&UART0,9600);
	//not sure what output to set it to
    /* Replace with your application code */
	error_status = TWI_Master_Init(&TWI0, TWI_FREQ);
	if(error_status = no_errors)
		transmit_receive_debug();    
	while (1) 
    {
    }
}

