/*
 * TWI.c
 *
 * Created: 11/8/2020 11:29:11 AM
 * Author : snspzv
 */ 

#include <avr/io.h>
#include "TWI_funcs.h"
#include "STA013_funcs.h"
#include "board.h"
#include "UART.h"
#include "Control_Outputs.h"
#define TWI_FREQ (100000000)


int main(void)
{
    /* Replace with your application code */
	uint8_t error_flag = no_errors;
	uint8_t data_array[5] = {0};
	UART_init(&UART1, 9600);
	Output_Init(&PC, (1<<PORT_USERLED));
	Output_Clear(&PC, (1<<PORT_USERLED));
	error_flag = TWI_Master_Init(&TWI1, TWI_FREQ);
	
	error_flag = STA013_init(data_array);
	if(error_flag != no_errors)
	{
		Output_Set(&PC, (1<<PORT_USERLED));
	}
    while (1) 
    {
    }
}

