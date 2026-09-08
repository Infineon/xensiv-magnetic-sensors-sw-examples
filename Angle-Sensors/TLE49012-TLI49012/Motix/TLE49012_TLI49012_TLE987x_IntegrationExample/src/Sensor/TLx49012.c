#include "tle_device.h"
#include <stdio.h>

#include "Sensor/TLx49012.h"
	
	
void TLx49012_Init(void)
{
	printf("Sensor initialization in progress...\n");
	
	// Populate CRC LUT
	CRC_Init();
	
	// Wait for SPI to become active - ASSUME JUST POWERED ON
	Delay_us(550);
	
	// Unlock sensor
	printf("Unlocking sensor...\n");
	TLx49012_SPI_WriteInFrame(UNLOCK_REG_ADDR, USR_PASS_DATA);

	// Disable Bitmap CRC checks
	printf("Disabling CRC checks for bitmaps...\n");
	TLx49012_SPI_WriteInFrame(STAT_EN_1_REG_ADDR, CRC_BM_DIS_DATA);

	// Test sensor responses 
	uint32_t dataTest;
	dataTest = TLx49012_SPI_ReadInFrame(STAT_EN_1_REG_ADDR, true);
	if(((dataTest & 0x00FFFF00) >> 8) != CRC_BM_DIS_DATA)
	{
		printf("ERROR: Sensor not responding or locked!\n");
		__disable_irq();
		__BKPT(0);
	}

	// Soft configure sensor
	printf("Configuring sensor...\n");
	TLx49012_SPI_WriteInFrame(USR_CONFIG_1_ADDR, USR_CONFIG_1_DATA);

	// Reset sensor from VM memory - keep and apply configuration
	printf("Reseting sensor...\n");
	TLx49012_SPI_WriteInFrame(STAT_EN_REG_ADDR, VAL_SOFT_RESET_VM_DATA);
	
	// Wait for SPI to become active
	Delay_us(900);

	// Configuration check after reset
	dataTest = TLx49012_SPI_ReadInFrame(USR_CONFIG_1_ADDR, true);
	if(((dataTest & 0x00FFFF00) >> 8) != USR_CONFIG_1_DATA)
	{
		printf("ERROR: Sensor not configured correctly!\n");
		__disable_irq();
		__BKPT(0);
	}

	printf("Sensor initialization DONE!\n");         
}


uint16_t TLx49012_GetAngleLSB(void)
{
	uint32_t sensor_response;
	
	sensor_response = TLx49012_SPI_ReadInFrame(ANGLE_PRED_ADDR, true);
	
	return (uint16_t)((sensor_response & 0x00FFFF00) >> 8);
}