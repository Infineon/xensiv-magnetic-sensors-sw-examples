#include "src/Sensor/TLx49012.h"
#include "SPI_Backend.h"
#include "cy_utils.h"
#include "stdio.h"
#include "cy_syslib.h"
	
	
void TLx49012_Init(void)
{
	printf("Sensor initialization in progress...\r\n");
	
	// Populate CRC LUT
	CRC_Init();
	
	// Wait for SPI to become active - ASSUME JUST POWERED ON
	Cy_SysLib_DelayUs(550);
	
	// Unlock sensor
	printf("Unlocking sensor...\r\n");
	TLx49012_SPI_WriteInFrame(UNLOCK_REG_ADDR, USR_PASS_DATA);

	// Disable Bitmap CRC checks
	printf("Disabling CRC checks for bitmaps...\r\n");
	TLx49012_SPI_WriteInFrame(STAT_EN_1_REG_ADDR, CRC_BM_DIS_DATA);

	// Test sensor responses 
	uint32_t dataTest;
	dataTest = TLx49012_SPI_ReadInFrame(STAT_EN_1_REG_ADDR, true);
	if(((dataTest & 0x00FFFF00) >> 8) != CRC_BM_DIS_DATA)
	{
		printf("ERROR: Sensor not responding or locked\r\n");
		CY_ASSERT(0); // block
	}

	// Soft configure sensor
	printf("Configuring sensor...\r\n");
	TLx49012_SPI_WriteInFrame(USR_CONFIG_1_ADDR, USR_CONFIG_1_DATA);

	// Reset sensor from VM memory - keep and apply configuration
	printf("Reseting sensor...\r\n");
	TLx49012_SPI_WriteInFrame(STAT_EN_REG_ADDR, VAL_SOFT_RESET_VM_DATA);
	
	// Wait for SPI to become active
	Cy_SysLib_DelayUs(900);

	// Configuration check after reset
	dataTest = TLx49012_SPI_ReadInFrame(USR_CONFIG_1_ADDR, true);
	if(((dataTest & 0x00FFFF00) >> 8) != USR_CONFIG_1_DATA)
	{
		printf("ERROR: Sensor not configured correctly\r\n");
		CY_ASSERT(0); // block
	}

	printf("Sensor initializations DONE!\r\n");         
}

uint16_t TLx49012_GetAngleLSB(void)
{
	uint32_t sensor_response;
	
	sensor_response = TLx49012_SPI_ReadInFrame(ANGLE_PRED_ADDR, true);
	
	return (uint16_t)((sensor_response & 0x00FFFF00) >> 8);
}

