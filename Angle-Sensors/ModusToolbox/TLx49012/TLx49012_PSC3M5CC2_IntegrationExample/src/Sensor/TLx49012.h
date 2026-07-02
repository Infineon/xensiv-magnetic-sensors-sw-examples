#ifndef SRC_SENSOR_TLX49012_H_
#define SRC_SENSOR_TLX49012_H_


#include "Interface/SPI_Frontend.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/

// UNLOCK
#define UNLOCK_REG_ADDR	  			0x78
#define USR_PASS_DATA		    	0x4711

// CRC BITMAP DISABLE
#define STAT_EN_1_REG_ADDR	  		0x04
#define CRC_BM_DIS_DATA		  		0x7F18

// USER CONFIG 1 - holds sync pulse configuration
#define USR_CONFIG_1_ADDR			0x3F
// SPI + SYNC ON IFE + FALLING EDGE SYNC (SPI CONFIG VALID ONLY FOR S0001 & E0001) - CHECK VARIANT 
#define USR_CONFIG_1_DATA			0x400

// PREDICTED ANGLE REGISTER
#define ANGLE_PRED_ADDR 			0x0C

// ANGLE REGISTER
#define ANGLE_ADDR 					0x0F

// ANGLE SYNC REGISTER
#define ANGLE_SYNC_ADDR				0x0A
#define SYNC_READ_STATUS_MASK		0x1
#define SYNC_TRIG_STATUS_MASK		0x2
#define SYNC_ANGLE_MASK				0xFFFC

// STATUS / RESET REGISTER
#define STAT_EN_REG_ADDR     		0x02
#define VAL_SOFT_RESET_VM_DATA 	  	0x8E82
#define VAL_SOFT_RESET_NVM_DATA   	0x8E81

/*******************************************************************************
 * Function Name: TLx49012_Init
 ***************************************************************************//**
 * \brief 	Initializes the sensor through a series of SPI commands.
		  	Soft-resets the sensor.
			Disables self-calibration.
 ******************************************************************************/
void TLx49012_Init(void);


/*******************************************************************************
 * Function Name: TLx49012_GetAngleLSB
 ***************************************************************************//**
 * \brief  	Reads the internal register at address 15 via SPI.
 * \return 	Angle value in LSB, 16-bit.
 ******************************************************************************/
uint16_t TLx49012_GetAngleLSB(void);


#endif /* SRC_SENSOR_TLX49012_H_ */
