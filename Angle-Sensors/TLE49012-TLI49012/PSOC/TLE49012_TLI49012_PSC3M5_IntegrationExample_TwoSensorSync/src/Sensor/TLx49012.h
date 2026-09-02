#ifndef SRC_SENSOR_TLX49012_H_
#define SRC_SENSOR_TLX49012_H_


#include "Interface/SPI_Frontend.h"

/*********************************************************************************************************************/
/*------------------------------------------CONFIGURABLE PARAMETERS--------------------------------------------------*/
/*********************************************************************************************************************/

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

// Sensor Slaves
#define SPI_SLAVE0						0
#define SPI_SLAVE1						1

/*********************************************************************************************************************/
/*-------------------------------------------------Data Structures---------------------------------------------------*/
/*********************************************************************************************************************/

// Sync register strucutre
typedef struct
{
	uint8_t readStatus:1;
	uint8_t triggerStatus:1;
	uint16_t AngleSync:14;

}AngleSyncRegisterStructure;

typedef union
{
    AngleSyncRegisterStructure bitfieldAccess;           /* brief Bit field access */
    uint16 unsignedValue;               				 /* brief Unsigned access */
} AngleSyncRegister;


/*******************************************************************************
 * Function Name: TLx49012_Init
 ***************************************************************************//**
 * \brief 	Initializes the sensor through a series of SPI commands.
		  	Soft-resets the sensor.
 ******************************************************************************/
void TLx49012_Init(void);

/*******************************************************************************
 * Function Name: TLx49012_GetAngleLSB
 ***************************************************************************//**
 * \brief  	Reads the internal register at configured address via SPI.
 * \return 	Angle value in LSB, 16-bit.
 ******************************************************************************/
uint16_t TLx49012_GetAngleLSB(uint8_t slaveSelect);

/*******************************************************************************
 * Function Name: TLx49012_GetAngleSyncRegister
 ***************************************************************************//**
 * \brief  	Reads the internal register at configured address via SPI.
 * \return 	Register value of type AngleSyncRegister
 ******************************************************************************/
AngleSyncRegister TLx49012_GetAngleSyncRegister(uint8_t slaveSelect);

/*******************************************************************************
 * Function Name: TLx49012_TriggerSyncPin
 ***************************************************************************//**
 * \brief  	Trigger the Sync pin for configured edge.
 ******************************************************************************/
void TLx49012_TriggerSyncPin(void);


#endif /* SRC_SENSOR_TLX49012_H_ */
