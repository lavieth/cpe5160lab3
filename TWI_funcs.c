
#include "TWI_funcs.h"

uint8_t no_errors=0;

//set twi clock frequency, 
uint8_t TWI_Master_Init(uint8_t volatile *TWI_addr, uint32_t I2C_freq)
{
  uint8_t prescale_value = (((F_CPU/F_DIV)/(uint32_t)(I2C_freq))-16UL)/(2UL*255);
  // setting the status register
  if(prescale_value<1)
     *(TWI_addr + TWSR) = 0b00000000;
  else if(prescale_value<4)
     *(TWI_addr + TWSR) = 0b00000001;
  else if(prescale_value<16)
     *(TWI_addr + TWSR) = 0b00000010;
  else if(prescale_value<64)
     *(TWI_addr + TWSR) = 0b00000011;
  //if prescale is higher return error value to then reenter?
  //same for twbr
  uint16_t TWBR_value = (((F_CPU/F_DIV)/(uint32_t)(SCL_freq))-16UL)/(2UL*prescale_value);
  //since TWBR register is a 8-bit register thus max val is 255
  if(TWBR_value<256)
    *(TWI_addr + TWBR) = ((uint8_t)TWBR_value);
}
uint8_t TWI_Master_Receive(uint8_t volatile *TWI_addr, uint8_t device_addr, uint32_t int_addr, uint8_t int_addr_sz, uint16_t num_bytes, uint8_t * array_name)
{
  uint8_t return_value=no_errors;
  uint8_t status;
  uint8_t send_value=(device_addr<<1)|0x01; //set lsb to 1
  //set a 1 to TWSTA
  
  *(TWI_addr + TWCR) = ((1<<TWSTA)|(1<<TWEN)|(1<<TWINT));
  
  //wait until all above bits are actaully 1
  do
  {
    status=*(TWI_addr+TWCR);
  }while(status&(1<<TWINT)==0);
  // clear bottom three bits
  uint8_t temp8=(*(TWI_addr+TWSR)&0xF8);
  //start or repeated start has been sent
  if((temp8==0x08)||(temp8==0x10))
  {
    //send the device addr we want to send to and a 1 for 'read'
    *(TWI_addr+TWDR)=send_value;
    *(TWI_addr+TWCR)((1<<TWINT)|(1<<TWEN));
  }
  //check for other values in temp8 for error conditions below
  //if(error)...
  // return_val = error_XYZ
  if(return_val==no_errors)
  {
    if(num_bytes==1)
      *(TWI_addr_TWCR)=((1<<TWINT)|(0<<TWEA)|(1<<TWEN));
    else
      *(TWI_addr_TWCR)=((1<<TWINT)|(1<<TWEA)|(1<<TWEN));
    uint8_t index=0;
    //while no errors and we have something to read
    while((num_bytes!=0)&&(return_value==no_errors))
    {
      do
      {
        status=*(TWI_adrr+TWCR);
      }while((status&0x80)==0)
    }
    //START BACK HERE AT PG.21 of lect 15b
      
  }
}

