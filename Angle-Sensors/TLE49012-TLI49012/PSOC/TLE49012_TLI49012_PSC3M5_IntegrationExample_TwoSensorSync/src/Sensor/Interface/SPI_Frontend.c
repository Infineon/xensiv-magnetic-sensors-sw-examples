#include "SPI_Frontend.h"
#include "CRC8_SAE_J1850.h"
#include <stdint.h>


uint32_t TLx49012_SPI_WriteInFrame(uint8_t addr, uint16_t data, uint8_t slaveSelect)
{
	static uint8_t data_to_send[4] 	= {0,0,0,0};
	static bool r_w					= 1;

	data_to_send[0] = (addr << 1) | (r_w & 1); 				// 7bit address 1 bit R/W
	data_to_send[1] = (data >> 8) & 0xFF;					// 1 data byte MSB
	data_to_send[2] = data & 0xFF;							// 1 data byte LSB
	data_to_send[3] = CalcCRC(data_to_send, 3);	// 1 byte CRC

	// Assemble 32-bit command
	//data_to_send = (data_temp[0] << 24) | (data_temp[1] << 16) | (data_temp[2] << 8) | data_temp[3];
	
	return PSC3M5_SPI_SendReceive(data_to_send, slaveSelect);
}

uint32_t TLx49012_SPI_ReadInFrame(uint8_t addr, bool clearStatus, uint8_t slaveSelect)
{
	static uint8_t data_to_send[4] 	= {0,0,0,0};
	static bool r_w					= 0;
	
	data_to_send[0] = (addr << 1) | (r_w & 1); 						// 7bit address 1 bit R/W
	data_to_send[1] = 0x00;											// Unused
	
	if(clearStatus)
	{
		data_to_send[2] = 0xFF;										// All 1s clear device status bit
	}
	else
	{
		data_to_send[2] = 0x00;										// All 0s no clearing
	}
	
	data_to_send[3] = CalcCRC(data_to_send, 3);			// 1 byte CRC

	// Assemble 32-bit command
	//data_to_send = (data_temp[0] << 24) | (data_temp[1] << 16) | (data_temp[2] << 8) | data_temp[3];
	
	return PSC3M5_SPI_SendReceive(data_to_send, slaveSelect);
}




