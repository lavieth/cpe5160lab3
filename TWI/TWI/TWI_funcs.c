
#include "TWI_funcs.h"
#include "board.h"
#define PRESCALE_1 (0);
#define PRESCALE_4 (1);
#define PRESCALE_16 (2);
#define PRESCALE_64 (3);

void send_start(uint8_t volatile *TWI_addr)
{
	uint8_t status;
	
	//Send start bit
	*(TWI_addr+TWCR )=( (1<<TWINT) | (1<<TWSTA) | (1<<TWEN));
	
	//wait for TWINT to be set
	do
	{
		status = *(TWI_addr + TWCR);
	} while ((status & (1<<TWINT)) == 0);
		
}

//helper to handle sending stop bit
void send_stop(uint8_t volatile * TWI_addr)
{	
	uint8_t status;
	
	//Send stop bit
	*(TWI_addr+TWCR )=( (1<<TWINT ) | (1<<TWSTO) | (1<<TWEN));
	
	//wait for stop bit to be cleared, indicating top condition is executed on bus
	do
	{
		status = *(TWI_addr + TWCR);
	} while ((status & (1<<TWSTO)) == 1);
	
}

//helper to act if one byte is left to receive
void handle_one_byte(uint8_t bytes_left, uint8_t volatile * TWI_addr)
{
	if(bytes_left == 1)
	{
		//Send NACK
		*(TWI_addr + TWCR)=((1<<TWINT)|(0<<TWEA)|(1<<TWEN));
		send_stop(TWI_addr);
	}
	
	else
	{
		//Send ACK
		*(TWI_addr + TWCR)=((1<<TWINT)|(1<<TWEA)|(1<<TWEN));
	}
}

//set twi clock frequency,
uint8_t TWI_Master_Init(uint8_t volatile *TWI_addr, uint32_t I2C_freq)
{	
	uint8_t return_value = no_errors;
	uint8_t prescale_value = (((F_CPU/F_DIV)/(uint32_t)(I2C_freq))-16UL)/(2UL*255);
	
	// setting the status register
	if(prescale_value<1)
	{
		*(TWI_addr + TWSR) = PRESCALE_1;
	}
	
	else if(prescale_value<4)
	{
		*(TWI_addr + TWSR) = PRESCALE_4;
	}
	
	else if(prescale_value<16)
	{
		*(TWI_addr + TWSR) = PRESCALE_16;
	}
	
	else if(prescale_value<64)
	{
		*(TWI_addr + TWSR) = PRESCALE_64;
	}

	//if prescale is higher return error value to then reenter?
	else
		return_value = prescaler_size_error;
	//same for twbr
	uint16_t TWBR_value = (((F_CPU/F_DIV)/(uint32_t)(I2C_freq))-16UL)/(2UL*prescale_value);
	
	//since TWBR register is a 8-bit register thus max val is 255
	if(TWBR_value<256)
	{
		*(TWI_addr + TWBR) = ((uint8_t)TWBR_value);
	}
	return return_value;
}

uint8_t TWI_Master_Receive(uint8_t volatile *TWI_addr, uint8_t device_addr, uint32_t int_addr, uint8_t int_addr_sz, uint16_t num_bytes, uint8_t * array_name)
{
	uint8_t return_value = no_errors;
	uint8_t status;
	uint8_t send_value=(device_addr<<1)|0x01; //set lsb to 1
	uint8_t index=0;
	
	/*//Start bit, enable, and write 1 to TWINT to clear after command is written
	*(TWI_addr + TWCR) = ((1<<TWSTA)|(1<<TWEN)|(1<<TWINT));
	
	//wait until TWINT bit is set, indicating TWI is ready for next command
	do
	{
		status=*(TWI_addr+TWCR);
	}while(status & (1 << TWINT) == 0);*/
	
	send_start();
	
	// clear bottom three bits
	uint8_t temp8=(*(TWI_addr + TWSR) & 0xF8);
	
	//start or repeated start has been sent
	if((temp8 == 0x08)||(temp8 == 0x10))
	{
		//send the device addr we want to read from and a 1 for 'read'
		*(TWI_addr+TWDR) = send_value;
		*(TWI_addr+TWCR) = ((1<<TWINT)|(1<<TWEN));
	}
	
	else
	{
		return_value = start_error; 
	}
	
	//Checking different statuses of SLA+R transmission
	if(return_value == no_errors)
	{
		if(temp8==0x40) // SLA+R sent, ACK received
		{
			
			handle_one_byte(num_bytes, TWI_addr);
			
			//1 byte read
			num_bytes--;
		
			//while no errors and we have something to read
			while((num_bytes != 0) && (return_value == no_errors))
			{
				//wait for TWINT bit to be set, indicating new data
				do
				{
					status = *(TWI_addr+TWCR);
				}while((status & (1<<TWINT)) == 0);
				
				//clear lower 3 bits to read 
				temp8 = ( *(TWI_addr + TWSR) & 0xF8);
				
				//Data byte received, ACK sent
				if(temp8 = 0x50)
				{
					num_bytes--;
					array_name[index] = *(TWI_addr + TWDR);//write data received into array
					index++;
					
					handle_one_byte(num_bytes, TWI_addr);
				}
				
				//Data byte received, NACK sent
				else if(temp8 == 0x58) 
				{
					num_bytes--;
					array_name[index] = *(TWI_addr + TWDR);//write data received into array
					send_stop(TWI_addr);
					return_value = bad_receive;
				}
				
				else
				{
					return_value = unknown_error;
				}
			}
		}
		
		 //Arbitration lost in SLA+R or NACK bit
		else if(temp8 == 0x38)
		{
			//release serial bus and enter slave mode
			*(TWI_addr + TWCR) = ((1<<TWINT) | (0<<TWSTA) | (0<<TWSTO));
			return_value = SLA_R_error;
		}
		
		//SLA+R transmitted and NACK received
		else if(temp8 = 0x48)
		{
			//Stop condition transmitted and TWSTO flag reset
			*(TWI_addr + TWCR) = ((1<<TWINT) | (0<<TWSTA) | (1<<TWSTO));
			return_value = SLA_R_NACK;
		}
		
		else
		{
			return_value = unknown_error;
		}
	}
	
	return return_value;
}

uint8_t TWI_Master_Transmit(uint8_t volatile *TWI_addr, uint8_t device_addr, uint32_t int_addr, uint8_t int_addr_sz, uint16_t num_bytes, uint8_t* array_name)
{
	uint8_t return_value=no_errors;
	uint8_t send_value;
	uint8_t status;
	
	send_value=device_addr<<1;
	
	send_start();
	
	//read status to see what we need to do next
	//clear lower three bits	
	uint8_t temp8=(*(TWI_addr+TWSR)&0xF8);
	
	//start sent
	if(temp8==0x08)
	{
		*(TWI_addr+TWDR)=send_value;
		*(TWI_addr+TWCR)=((1<<TWINT)|(1<<TWEN));
		
	}
	else
	{
		return_value = start_error;	
	}
	//if no errors send data bytes until all sent 
	if(return_value == no_errors)
	{	
		uint8_t index=0;
		while((num_bytes!=0)&&(return_value==no_errors))
		{
			send_value=array_name[index];
			index++;
			num_bytes--;
			//wait until TWINT is set to send again
			do
			{
				status=*(TWI_addr+TWCR);
			}while(status&0x80)==0)
			//clear lower three bits	
			temp8=(*(TWI_addr+TWSR)&0xF8);
			//read status and see what to do next
			//SLA+W sent, ack rec'd
			if(temp8==0x18)
			{
				*(TWI_addr+TWDR)=send_value;
				*(TWI_addr+TWCR)=((1<<TWINT)|(1<<TWEN));
			}
			//data sent, ACK rec'd
			else if(temp8==0x28)
			{
				*(TWI_addr+TWDR)=send_value;
				*(TWI_addr+TWDR)=((1<<TWINT)|(1<<TWEN));
			}
			//sla+w sent, nack rec'd
			else if(temp8==0x20)
			{
				/**(TWI_addr+TWCR)=((1<<TWINT)|(1<<TWSTO)|(1<<TWEN));
				//wait for stop condition to be cleared
				do
				{
					status=*(TWI_addr+TWCR);
				}while((status&(1<<TWSTO))!=0);*/
				send_stop();
				//send nack error
				return_value=SLA_W_NACK; 
				

			}
			//do we need the arbitration condition here as well?
		
		}
		//after all bytes have been sent, send the stop condition
		//state programming?
		//sla+w w/ ack rec'd
		if(temp8==0x18)
		{
			*(TWI_addr+TWDR)=send_val;
			/**(TWI_addr+TWDR)=((1<<TWINT)|(1<<TWSTO)|(1<<TWEN));
			//wait for the stop =0 
			do
			{
				status=*(TWI_addr+TWCR);
			}while((status&(1<<TWSTO))!=0);*/
			send_stop();
		}
		//sla+w w/ nack rec'd
		else if(temp8==0x20)
		{
			send_stop();
		}
		//byte send, ack rec'd
		else if(temp8==0x28)
		{
			*(TWI_addr+TWDR)=send_value;
			send_stop();
			
		}
		//arbitration lost in sla+w
		else if(temp8==0x38)
		{
			*(TWI_addr+TWDR)=((0<<TWSTA)|(0<<TWSTO)|(1<<TWINT));
			return_value=STA_W_error;
		}

	}	
	return return_value;
}
