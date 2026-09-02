#ifndef SRC_CONTROLLER_TLE987X_GPIO_TLE987X_GPIO_H_
#define SRC_CONTROLLER_TLE987X_GPIO_TLE987X_GPIO_H_


#include "port.h"
#include "port_defines.h"

/* Heartbeat LED on P0.4 */
#define BLINK_PIN_PORT	&PORT->P0_DATA.reg
#define BLINK_PIN_Pos	(uint8)PORT_P0_DATA_P4_Pos
#define BLINK_PIN_Msk	(uint8)PORT_P0_DATA_P4_Msk
#define BLINK_PIN		BLINK_PIN_PORT, BLINK_PIN_Pos, BLINK_PIN_Msk



void GPIO_SetOutputLevelLow(volatile uint8 *port, uint8_t pin, uint8_t msk);
void GPIO_SetOutputLevelHigh(volatile uint8 *port, uint8_t pin, uint8_t msk);
void GPIO_TogglePin(volatile uint8 *port, uint8_t pin, uint8_t msk);


#endif /* SRC_CONTROLLER_TLE987X_GPIO_TLE987X_GPIO_H_ */
