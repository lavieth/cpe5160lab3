#ifndef TWI_DEBUG_H
#define TWI_DEBUG_H

#include <stdio.h>
#include "TWI_funcs.h"
#include "board.h"
#include "UART_Print.h"

//takes in TWI_Master_Receive() and checks that it is working
void transmit_receive_debug();

#endif
