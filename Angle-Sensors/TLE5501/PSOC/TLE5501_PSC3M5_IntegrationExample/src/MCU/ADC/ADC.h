#ifndef SRC_MCU_ADC_ADC_H_
#define SRC_MCU_ADC_ADC_H_


// Starts the HPPASS peripheral.
void PSC3M5_ADC_Init(void);

// Starts an ADC conversion and waits for DMA to finish the transfer
void PSC3M5_ADC_TriggerMeasurement(void);


#endif /* SRC_MCU_ADC_ADC_H_ */
