/*
 * DMA_SPI.h
 *
 *  Created on: 6 apr. 2026
 *      Author: Taciuc
 */

#ifndef SRC_MCU_DMA_DMA_SPI_H_
#define SRC_MCU_DMA_DMA_SPI_H_


#include "stdbool.h"
#include "stdint.h"
#include "cy_pdl.h"
#include "cycfg.h"
#include "cycfg_dmas.h"


/******************************************************************************
 * MACROS                                                        *
******************************************************************************/

/* Initialization status */
#define INIT_SUCCESS            (0)
#define INIT_FAILURE            (1)


/******************************************************************************
 * Function Prototypes                                                        *
******************************************************************************/

uint32_t configure_tx_dma(uint8_t* txBuffer);
void tx_dma_complete(void);
uint32_t configure_rx_dma(uint8_t* rxBuffer);
void rx_dma_complete(void);
void handle_error(void);


/******************************************************************************
 * Extern Variables                                                           *
******************************************************************************/

extern volatile bool tx_dma_done;
extern volatile bool rx_dma_done;



#endif /* SRC_MCU_DMA_DMA_SPI_H_ */
