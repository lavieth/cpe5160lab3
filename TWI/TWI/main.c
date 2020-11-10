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
    /* Replace with your application code */
	TWI_Master_Init(&TWI0, TWI_FREQ);
    
	while (1) 
    {
    }
}

