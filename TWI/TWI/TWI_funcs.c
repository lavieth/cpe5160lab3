
#include "TWI_funcs.h"
#include "board.h"
#include "Control_Outputs.h"
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
		//Data byte will be received and NACK returned
		*(TWI_addr + TWCR)=((1<<TWINT)|(0<<TWEA)|(1<<TWEN));
	}
	
	else
	{
		//DAta byte will be received and ACK returned
		*(TWI_addr + TWCR)=((1<<TWINT)|(1<<TWEA)|(1<<TWEN));
	}
}

//set twi clock frequency,
uint8_t TWI_Master_Init(uint8_t volatile *TWI_addr, uint32_t I2C_freq)
{	
	//reset pin here for clean slate before init
	Output_Init(&PB,0x02);
	Output_Clear(&PB,0x02);
	Output_Set(&PB,0x02);
	
	
	uint8_t return_value = no_errors;
	uint8_t prescale_value = (((F_CPU/F_DIV)/(uint32_t)(I2C_freq))-16UL)/(2UL*255);
	
	// setting the status register
	if(prescale_value<1)
	{
		*(TWI_addr + TWSR) = PRESCALE_1;
		prescale_value=1;
	}
	
	else if(prescale_value<4)
	{
		*(TWI_addr + TWSR) = PRESCALE_4;
		prescale_value = 4;
	}
	
	else if(prescale_value<16)
	{
		*(TWI_addr + TWSR) = PRESCALE_16;
		prescale_value = 16;
	}
	
	else if(prescale_value<64)
	{
		*(TWI_addr + TWSR) = PRESCALE_64;
		prescale_value = 64;
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


uint8_t TWI_Master_Transmit(uint8_t volatile *TWI_addr, uint8_t device_addr, uint32_t int_addr, uint8_t int_addr_sz, uint16_t num_bytes, uint8_t * array_name)
{
	uint8_t return_value=no_errors;
	uint8_t send_value;
	uint8_t status;
	uint8_t temp8;
	
	//Address + 0lsb, SLA+W
	send_value=(device_addr<<1);
	
	send_start(TWI_addr);
	
	//read status to see what we need to do next
	//clear lower three bits
	temp8=(*(TWI_addr+TWSR)&0xF8);
	
	//start sent
	if(temp8 == 0x08 || temp8 == 0x10)
	{
		*(TWI_addr+TWDR)=send_value;
		*(TWI_addr+TWCR)=((1<<TWINT)|(1<<TWEN));
		
		do
		{
			status=*(TWI_addr+TWCR);
		}while((status & (1 << TWINT)) == 0);
		
		//Status register with bottom three bits cleared
		temp8 = (*(TWI_addr + TWSR) & 0xF8);
	}
	
	else
	{
		return_value = start_error;
	}
	//if no errors send data bytes until all sent
	if(return_value == no_errors)
	{
		uint8_t index=0;
		while(((num_bytes != 0) && (return_value == no_errors)) || (int_addr_sz!= 0))
		{
			send_value=array_name[index];
			index++;
			num_bytes--;
			
			//wait until TWINT is set to send again
			do
			{
				status =* (TWI_addr + TWCR);
			}while((status & (1<<TWINT)) == 0);
			//clear lower three bits
			temp8=(*(TWI_addr + TWSR) & 0xF8);
			//read status and see what to do next
			
			//SLA+W sent and ACK received OR data sent and ACK received
			if(temp8 == 0x18 || temp8 == 0x28)
			{
				if(int_addr_sz > 0)
				{
					//Shift 8 bytes if size 2 and no bytes if size 1
					//Casting as uint8_t will select 8 rightmost bits
					send_value = (uint8_t)(int_addr >> ((int_addr_sz - 1) * 8));
					
					//Increment num_bytes and decrement index because nothing was read from array this loop
					num_bytes++;
					index--;
					int_addr_sz--;
					
				}
				
				*(TWI_addr+TWDR)=send_value;
				*(TWI_addr+TWCR)=((1<<TWINT)|(1<<TWEN));
				
				if((num_bytes == 0) && (int_addr_sz == 0))
				{
					send_stop(TWI_addr);
				}
				
			}
			
			//SLA+W sent, NACK received
			else if (temp8 == 0x20)
			{
				send_stop(TWI_addr);
				return_value = SLA_W_NACK;
			}
			
			//Data sent and NACK received
			else if(temp8 == 0x30)
			{
				send_stop(TWI_addr);
				return_value = data_sent_NACK;
			}
			
			//Arbitration lost in SLA+W or data bytes
			else if (temp8 == 0x38)
			{
				//release bus and enter slave mode
				*(TWI_addr + TWCR) = (1<<TWINT);
				return_value = transmit_arbitration;
			}
			
		}
	}
	return return_value;
}

uint8_t TWI_Master_Receive(uint8_t volatile *TWI_addr, uint8_t device_addr, uint32_t int_addr, uint8_t int_addr_sz, uint16_t num_bytes, uint8_t * array_name)
{
	uint8_t return_value = no_errors;
	uint8_t status;
	uint8_t send_value=(device_addr<<1)|0x01; //set lsb to 1
	uint8_t index = 0;
	uint8_t * t_array;

	uint8_t i = 10;
	/*do
	{
		return_value=TWI_Master_Transmit(TWI_addr, device_addr, int_addr, int_addr_sz, 0,t_array);
		i--;
	}while((return_value!=no_errors)&&(i!=0));
	 */
	 
	
	send_start(TWI_addr);
	
	// clear bottom three bits
	uint8_t temp8=(*(TWI_addr + TWSR) & 0xF8);
	
	//start or repeated start has been sent
	if((temp8 == 0x08)||(temp8 == 0x10))
	{
		//send the device addr we want to read from and a 1 for 'read'
		*(TWI_addr+TWDR) = send_value;
		*(TWI_addr+TWCR) = ((1<<TWINT)|(1<<TWEN));
		//wait till we get response
		do
		{
			status=*(TWI_addr+TWCR);
		}while((status & (1 << TWINT)) == 0);
		//write to temp8
		temp8=(*(TWI_addr + TWSR) & 0xF8);
	}
	
	else
	{
		return_value = start_error; 
	}
	
	if(return_value == no_errors)
	{
		while((num_bytes > 0) && (return_value == no_errors))
		{	
			
			//SLA+R sent and ACK received
			if(temp8 == 0x40)
			{
				handle_one_byte(num_bytes, TWI_addr);
			}
			
			//Data byte has been received and ACK has been returned
			else if(temp8 == 0x50)
			{
				array_name[index] = *(TWI_addr +TWDR);
				
				handle_one_byte(num_bytes, TWI_addr);
				index++;
				num_bytes--;
			}
			
			//Data byte has been received and NACK has been returned
			else if (temp8 == 0x58)
			{
				array_name[index] = *(TWI_addr +TWDR);
				send_stop(TWI_addr);
				index++;
				num_bytes--;
			}
			
			
			
			if(num_bytes > 0)
			{
				do
				{
					status=*(TWI_addr+TWCR);
				}while((status & (1 << TWINT)) == 0);
				
				//write to temp8
				temp8 = (*(TWI_addr + TWSR) & 0xF8);
			}
		} 
	}
	
	return return_value;
}




