#ifndef SRC_SENSOR_INTERFACE_CRC8_SAE_J1850_H_
#define SRC_SENSOR_INTERFACE_CRC8_SAE_J1850_H_


#include "stdint.h"
#include "stdbool.h"


/*******************************************************************************
 * Function Name: CRC_Init
 ***************************************************************************//**
 * \brief  Populates the CRC LUT for following SPI commands.
 ******************************************************************************/
void CRC_Init(void);


/*******************************************************************************
 * Function Name: CalcCRC
 ***************************************************************************//**
 * \brief  Calculates the CRC based on the command to be sent.
 * \param  buf  Command to be processed.
 * \param  len  Length of the command to be processed.
 ******************************************************************************/
uint8_t CalcCRC(uint8_t * buf, uint8_t len);


#endif /* SRC_SENSOR_INTERFACE_CRC8_SAE_J1850_H_ */
