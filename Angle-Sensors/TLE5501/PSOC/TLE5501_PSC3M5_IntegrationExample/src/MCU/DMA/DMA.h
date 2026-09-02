#ifndef SRC_MCU_ADC_DMA_H_
#define SRC_MCU_ADC_DMA_H_


#include "stdbool.h"
#include "stdint.h"


// ADC DMA completion flag
extern volatile bool ADC_DMA_done_flag;

// ADC DMA completion flag
extern volatile bool ADC_DMA_error_flag;

// The result buffer for ADC pseudo differential channels, contain channel ID
extern uint32_t ADC_result_buffer[];


// Initialize the DMA interrupt and configure target array for data transfer
void PSC3M5_DMA_Init(void);


#endif /* SRC_MCU_ADC_DMA_H_ */
