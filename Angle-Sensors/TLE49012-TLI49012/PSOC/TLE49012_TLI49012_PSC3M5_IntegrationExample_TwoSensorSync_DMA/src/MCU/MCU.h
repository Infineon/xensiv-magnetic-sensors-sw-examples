#ifndef SRC_MCU_MCU_H_
#define SRC_MCU_MCU_H_


#include "SPI/SPI_Backend.h"
#include "UART/UART.h"


void PSC3M5_MCU_Init()
{
	PSC3M5_SPI_Init();
	PSC3M5_UART_Init();
}

#endif /* SRC_MCU_MCU_H_ */
