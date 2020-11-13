/*
 * TWI.c
 *
 * Created: 11/8/2020 11:29:11 AM
 * Author : snspzv
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "TWI_funcs.h"
#include "UART.h"
#include "TWI_debug.h"
#include "Control_Outputs.h"
#include "Write_STA.h"
#define TWI_FREQ (25000)


int main(void)
{
	extern const  uint8_t CONFIG[3998];
	extern const  uint8_t CONFIG2[50];
	uint8_t error_status;
	//initialize port c as output for UART
	Output_Init(&PC, 0x80);
	UART_init(&UART1,9600);
	//not sure what output to set it to
    /* Replace with your application code */
	
	error_status = TWI_Master_Init(&TWI1, TWI_FREQ);
	if(error_status == no_errors)
	{
		transmit_receive_debug();
	}
	
	write_STA();
	
	while (1); 
    //{
    //}
}

