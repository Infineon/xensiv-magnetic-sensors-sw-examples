#ifndef SRC_MCU_SPI_SPI_BACKEND_H_
#define SRC_MCU_SPI_SPI_BACKEND_H_

#include "cy_pdl.h"
#include "cycfg.h"


/* Assign SPI interrupt priority */
#define SCB_SPI_INTR_PRIORITY  (3U)


void PSC3M5_SPI_Init(void);
uint32_t PSC3M5_SPI_SendReceive(uint8_t *txBuffer, uint8_t slaveSelect);


#endif /* SRC_MCU_SPI_SPI_BACKEND_H_ */
