#ifndef SRC_MCU_UART_UART_H_
#define SRC_MCU_UART_UART_H_


#include "src/Sensor/TLE5501_0002_defines.h"


// Initializes the UART HAL for serial port communication.
void PSC3M5_UART_Init(void);

// Sends SIN[rad], COS[rad] and ANGLE[deg] to the serial port.
void PSC3M5_UART_SendAngleInfo(TLE5501_t* sensor);


#endif /* SRC_MCU_UART_UART_H_ */
