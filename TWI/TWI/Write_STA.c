/*
 * Write_STA.c
 *
 * Created: 11/12/2020 5:24:20 PM
 *  Author: snspzv
 */ 
#include "TWI_funcs.h"
#include <avr/pgmspace.h>
#include <stdio.h>
void write_STA()
{
	extern const  uint8_t CONFIG[3998];
	extern const  uint8_t CONFIG2[50];
	
	uint32_t reg_addr;
	uint8_t * send_array;
	uint8_t index = 0;
	uint8_t status = no_errors;
	uint8_t timeout = 50;
	uint8_t prnt_bffr[30];
	do 
	{
		reg_addr = pgm_read_byte(&CONFIG[index]);
		index++;
		send_array[0] = pgm_read_byte(&CONFIG[index]);
		index++;
		timeout = 50;
		do 
		{
			status = TWI_Master_Transmit(&TWI1, 0x43, reg_addr, 1, 1, send_array);
			timeout--;
		} while ((status != no_errors) && (timeout != 0));
	} while ((reg_addr != 0xFF) && (timeout != 0));

	if(timeout!=0)
	{
		sprintf(prnt_bffr, "Config sent.../n/r");
		UART_Transmit_String(&UART1,0,prnt_bffr);
	}
	
	
	
}
