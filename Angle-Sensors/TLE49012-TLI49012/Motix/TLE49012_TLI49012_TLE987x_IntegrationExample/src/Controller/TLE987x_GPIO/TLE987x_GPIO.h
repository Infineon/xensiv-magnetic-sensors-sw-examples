#ifndef SRC_CONTROLLER_TLE987X_GPIO_TLE987X_GPIO_H_
#define SRC_CONTROLLER_TLE987X_GPIO_TLE987X_GPIO_H_


#include "port.h"
#include "port_defines.h"


void GPIO_SetOutputLevelLow(volatile uint8 *port, uint8_t pin, uint8_t msk);
void GPIO_SetOutputLevelHigh(volatile uint8 *port, uint8_t pin, uint8_t msk);
void GPIO_TogglePin(volatile uint8 *port, uint8_t pin, uint8_t msk);


#endif /* SRC_CONTROLLER_TLE987X_GPIO_TLE987X_GPIO_H_ */
