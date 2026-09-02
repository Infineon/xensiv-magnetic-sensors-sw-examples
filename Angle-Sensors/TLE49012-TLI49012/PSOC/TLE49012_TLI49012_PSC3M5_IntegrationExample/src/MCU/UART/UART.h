#ifndef SRC_MCU_UART_UART_H_
#define SRC_MCU_UART_UART_H_


#include "stdint.h"


// Initializes the UART HAL for serial port communication.
void PSC3M5_UART_Init(void);

// Sends SIN[rad], COS[rad] and ANGLE[deg] to the serial port.
void PSC3M5_UART_SendAngleInfo(uint16_t angle);


#endif /* SRC_MCU_UART_UART_H_ */
