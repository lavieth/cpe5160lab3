/*
 * TWI.c
 *
 * Created: 11/8/2020 11:29:11 AM
 * Author : snspzv
 */ 

#include <avr/io.h>
#include "TWI_funcs.h"
#define TWI_FREQ (16000000)


int main(void)
{
	uint8_t error_status;
    /* Replace with your application code */
	error_status = TWI_Master_Init(&TWI0, TWI_FREQ);
	if(error_status = no_errors)
	{
		//call debugging here
    
	while (1) 
    {
    }
}

