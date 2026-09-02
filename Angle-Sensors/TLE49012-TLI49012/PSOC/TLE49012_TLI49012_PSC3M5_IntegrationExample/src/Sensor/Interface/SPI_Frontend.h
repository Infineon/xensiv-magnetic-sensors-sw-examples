#ifndef SRC_SENSOR_SPI_FRONTEND_H_
#define SRC_SENSOR_SPI_FRONTEND_H_


#include "stdint.h"
#include "stdbool.h"
#include "src/MCU/SPI/SPI_Backend.h"
#include "src/Sensor/Interface/CRC8_SAE_J1850.h"


/*******************************************************************************
 * Function Name: SPI_WriteInFrame
 ***************************************************************************//**
 * \brief  Sends a write command via SPI with in-frame sensor response.
 *         Command and response are transmitted in the same SPI frame.
 * \param  addr  Write address.
 * \param  data  Data to be written.
 * \return  Full 32-bit sensor response.
 ****************(*************************************************************/
uint32_t TLx49012_SPI_WriteInFrame(uint8_t addr, uint16_t data);


/*******************************************************************************
 * Function Name: SPI_ReadInFrame
 ***************************************************************************//**
 * \brief  Sends a read command via SPI with in-frame sensor response.
 *         Command and response are transmitted in the same SPI frame.
 * \param  addr         Read address.
 * \param  clearStatus  If true, device status register is cleared.
 * \return  Full 32-bit sensor response.
 ******************************************************************************/
uint32_t TLx49012_SPI_ReadInFrame(uint8_t addr, bool clearStatus);


#endif /* SRC_SENSOR_SPI_FRONTEND_H_ */
