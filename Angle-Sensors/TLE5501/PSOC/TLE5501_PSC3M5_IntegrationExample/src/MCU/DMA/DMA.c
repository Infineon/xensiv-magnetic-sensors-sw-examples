#include "DMA.h"
#include "cycfg_dmas.h"


// Interrupt configuration structure of ADC DMA.
const cy_stc_sysint_t ADC_DMA_intr_config =
{
    .intrSrc = ADC_DMA_IRQ,
    .intrPriority = 0U,
};


volatile bool ADC_DMA_done_flag = false;	// ADC DMA successful flag
volatile bool ADC_DMA_error_flag = false;	// ADC DMA error flag
uint32_t ADC_result_buffer[2] = {0};		// Target array to which DMA will move the data from the HPPASS FIFO after a conversion.


// Check if interrupt cause is a successful read from HPPASS FIFO
void ADC_DMA_IntrHandler(void)
{
	Cy_DMA_Channel_ClearInterrupt(ADC_DMA_HW, ADC_DMA_CHANNEL);
	
    // Check interrupt cause to capture errors
    cy_en_dma_intr_cause_t dma_status = Cy_DMA_Channel_GetStatus(ADC_DMA_HW, ADC_DMA_CHANNEL);
    if (CY_DMA_INTR_CAUSE_COMPLETION == dma_status)
    {
		// DMA transfer ok
        ADC_DMA_done_flag = true;
    }
    /*
    else
    {
        // DMA error occurred
        ADC_DMA_error_flag = true;
    }
    */
}

void PSC3M5_DMA_Init(void)
{
	// Initialize ADC DMA descriptor 0
    if (CY_DMA_SUCCESS != Cy_DMA_Descriptor_Init(&ADC_DMA_Descriptor_0, &ADC_DMA_Descriptor_0_config))
    {
        CY_ASSERT(0);
    }
    
    // Set source and destination address for ADC DMA descriptor 0
    Cy_DMA_Descriptor_SetSrcAddress(&ADC_DMA_Descriptor_0, (uint32_t *) CY_HPPASS_SAR_FIFO_READ_PTR(0));
    Cy_DMA_Descriptor_SetDstAddress(&ADC_DMA_Descriptor_0,  (uint32_t *) ADC_result_buffer);
    
    // ADC DMA channel configuration
    if (CY_DMA_SUCCESS != Cy_DMA_Channel_Init(ADC_DMA_HW, ADC_DMA_CHANNEL, &ADC_DMA_channelConfig))
    {
        CY_ASSERT(0);
    }
    Cy_DMA_Channel_SetDescriptor(ADC_DMA_HW, ADC_DMA_CHANNEL, &ADC_DMA_Descriptor_0);
    
    // Initialize and enable interrupt for ADC DMA
    Cy_DMA_Channel_SetInterruptMask(ADC_DMA_HW, ADC_DMA_CHANNEL, CY_DMA_INTR_MASK);
    Cy_SysInt_Init(&ADC_DMA_intr_config, ADC_DMA_IntrHandler);
    NVIC_EnableIRQ(ADC_DMA_intr_config.intrSrc);
    
    // Enable ADC DMA channel
    ADC_DMA_done_flag = false;
    ADC_DMA_error_flag = false;
    Cy_DMA_Channel_Enable(ADC_DMA_HW, ADC_DMA_CHANNEL);
    
    // Enable DMA
    Cy_DMA_Enable(ADC_DMA_HW);
}




