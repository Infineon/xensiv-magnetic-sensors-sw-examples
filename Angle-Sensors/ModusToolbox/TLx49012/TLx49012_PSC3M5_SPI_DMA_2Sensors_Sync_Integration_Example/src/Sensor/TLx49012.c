#include "src/Sensor/TLx49012.h"
#include "cy_gpio.h"
#include "cy_utils.h"
#include "cycfg_pins.h"
#include "stdio.h"
#include "cy_syslib.h"
	
	
void TLx49012_Init(void)
{
	printf("Sensor initializations in progress...\r\n");
	
	// Populate CRC LUT
	CRC_Init();
	
	// Wait for SPI to become active - ASSUME JUST POWERED ON
	Cy_SysLib_DelayUs(550);
	
	// Unlock sensors
	printf("Unlocking sensors...\r\n");
	TLx49012_SPI_WriteInFrame(UNLOCK_REG_ADDR, USR_PASS_DATA, SPI_SLAVE0);
	Cy_SysLib_DelayUs(1);
	TLx49012_SPI_WriteInFrame(UNLOCK_REG_ADDR, USR_PASS_DATA, SPI_SLAVE1);

	// Disable Bitmap CRC checks
	printf("Disabling CRC checks for bitmaps...\r\n");
	TLx49012_SPI_WriteInFrame(STAT_EN_1_REG_ADDR, CRC_BM_DIS_DATA, SPI_SLAVE0);
	Cy_SysLib_DelayUs(1);
	TLx49012_SPI_WriteInFrame(STAT_EN_1_REG_ADDR, CRC_BM_DIS_DATA, SPI_SLAVE1);

	// Test sensors responses 
	uint32_t dataTest;
	dataTest = TLx49012_SPI_ReadInFrame(STAT_EN_1_REG_ADDR, true,SPI_SLAVE0);
	if(((dataTest & 0x00FFFF00) >> 8) != CRC_BM_DIS_DATA)
	{
		printf("ERROR: Sensor0 not responding or locked\r\n");
		CY_ASSERT(0); // block
	}
	
	dataTest = TLx49012_SPI_ReadInFrame(STAT_EN_1_REG_ADDR, true,SPI_SLAVE1);
	if(((dataTest & 0x00FFFF00) >> 8) != CRC_BM_DIS_DATA)
	{
		printf("ERROR: Sensor1 not responding or locked\r\n");
		CY_ASSERT(0); // block
	}

	// Soft configure sensors
	printf("Configuring sensors...\r\n");
	TLx49012_SPI_WriteInFrame(USR_CONFIG_1_ADDR, USR_CONFIG_1_DATA, SPI_SLAVE0);
	Cy_SysLib_DelayUs(1);
	TLx49012_SPI_WriteInFrame(USR_CONFIG_1_ADDR, USR_CONFIG_1_DATA, SPI_SLAVE1);

	// Reset sensor from VM memory - keep and apply configuration
	printf("Reseting sensors...\r\n");
	TLx49012_SPI_WriteInFrame(STAT_EN_REG_ADDR, VAL_SOFT_RESET_VM_DATA,SPI_SLAVE0);
	Cy_SysLib_DelayUs(1); 
	TLx49012_SPI_WriteInFrame(STAT_EN_REG_ADDR, VAL_SOFT_RESET_VM_DATA,SPI_SLAVE1); 
	
	// Wait for SPI to become active
	Cy_SysLib_DelayUs(900);

	// Configuration check after reset
	dataTest = TLx49012_SPI_ReadInFrame(USR_CONFIG_1_ADDR, true,SPI_SLAVE0);
	if(((dataTest & 0x00FFFF00) >> 8) != USR_CONFIG_1_DATA)
	{
		printf("ERROR: Sensor0 not configured correctly\r\n");
		CY_ASSERT(0); // block
	}
	
	dataTest = TLx49012_SPI_ReadInFrame(USR_CONFIG_1_ADDR, true,SPI_SLAVE1);
	if(((dataTest & 0x00FFFF00) >> 8) != USR_CONFIG_1_DATA)
	{
		printf("ERROR: Sensor1 not configured correctly\r\n");
		CY_ASSERT(0); // block
	}

	// Clear SYNC registers
	TLx49012_SPI_ReadInFrame(ANGLE_SYNC_ADDR, true,SPI_SLAVE0);
	Cy_SysLib_DelayUs(1);
	TLx49012_SPI_ReadInFrame(ANGLE_SYNC_ADDR, true,SPI_SLAVE1);	 

	printf("Sensor initializations DONE!\r\n");         
}

uint16_t TLx49012_GetAngleLSB(uint8_t slaveSelect)
{
	uint32_t sensor_response;
	
	sensor_response = TLx49012_SPI_ReadInFrame(ANGLE_PRED_ADDR, true, slaveSelect);
	
	return (uint16_t)((sensor_response & 0x00FFFF00) >> 8);
}

AngleSyncRegister TLx49012_GetAngleSyncRegister(uint8_t slaveSelect)
{
	uint32_t sensor_response;
	AngleSyncRegister sync_register;

	sensor_response = TLx49012_SPI_ReadInFrame(ANGLE_SYNC_ADDR, true, slaveSelect);
	sync_register.unsignedValue = (uint16_t) ((sensor_response & 0x00FFFF00) >> 8);
	
	return sync_register;
}

void TLx49012_TriggerSyncPin(void)
{
	Cy_GPIO_Write(SYNC_PIN_PORT, SYNC_PIN_NUM, 0);
	Cy_SysLib_DelayUs(5);
	Cy_GPIO_Write(SYNC_PIN_PORT, SYNC_PIN_NUM, 1);
}


