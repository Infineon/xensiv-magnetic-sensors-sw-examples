#ifndef SRC_MCU_PSC3M5_H_
#define SRC_MCU_PSC3M5_H_


#include "ADC/ADC.h"
#include "DMA/DMA.h"
#include "UART/UART.h"


// Initialize interrupts and start peripherals
void PSC3M5_MCU_Init()
{
	// Enable the UART HAL
	PSC3M5_UART_Init();
	
	// Start the HPPASS peripheral
	PSC3M5_ADC_Init();
	
	// Configure the DMA interrupt and set data transfer path
	PSC3M5_DMA_Init();
}


#endif /* SRC_MCU_PSC3M5_H_ */
