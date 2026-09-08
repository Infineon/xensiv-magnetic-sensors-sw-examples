#ifndef SRC_CONTROLLER_TLE987X_SPI_TLE987X_SPI_H_
#define SRC_CONTROLLER_TLE987X_SPI_TLE987X_SPI_H_


#include "tle_device.h"
#include "Controller/TLE987x_GPIO/TLE987x_GPIO.h"
#include "ssc.h"


#define CS_PIN_PORT	&PORT->P1_DATA.reg
#define CS_PIN_Pos	(uint8)PORT_P1_DATA_P0_Pos
#define CS_PIN_Msk	(uint8)PORT_P1_DATA_P0_Msk
#define CS_PIN		CS_PIN_PORT, CS_PIN_Pos, CS_PIN_Msk


uint32_t TLE987x_SPI_SendReceive(uint8_t* txBuffer);


#endif /* SRC_CONTROLLER_TLE987X_SPI_TLE987X_SPI_H_ */