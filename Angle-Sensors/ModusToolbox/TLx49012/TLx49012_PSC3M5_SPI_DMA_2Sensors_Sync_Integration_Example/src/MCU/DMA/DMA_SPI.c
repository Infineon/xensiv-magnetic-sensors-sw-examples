#include <DMA_SPI.h>
#include "cycfg_dmas.h"


/* Interrupt priority for RXDMA */
#define RXDMA_INTERRUPT_PRIORITY (7u)

/* Interrupt priority for TXDMA */
#define TXDMA_INTERRUPT_PRIORITY (7u)

/* variable to check the rx DMA transaction status */
volatile bool rx_dma_done = false;

/* variable to check the tx DMA transcation status */
volatile bool tx_dma_done = false;

/******************************************************************************
* Function Name: configure_tx_dma
*******************************************************************************
*
* Summary:      This function configure the transmit DMA block 
*
* Parameters:   tx_buffer
*
* Return:       (uint32_t) INIT_SUCCESS or INIT_FAILURE
*
******************************************************************************/
uint32_t configure_tx_dma(uint8_t* tx_buffer)
{
     cy_en_dma_status_t dma_init_status;
     const cy_stc_sysint_t intTxDma_cfg =
     {
         .intrSrc      = txDma_IRQ,
         .intrPriority = 7u
     };
     /* Initialize descriptor */
     dma_init_status = Cy_DMA_Descriptor_Init(&txDma_Descriptor_0, &txDma_Descriptor_0_config);
     if (dma_init_status!=CY_DMA_SUCCESS)
     {
         return INIT_FAILURE;
     }

     dma_init_status = Cy_DMA_Channel_Init(txDma_HW, txDma_CHANNEL, &txDma_channelConfig);
     if (dma_init_status!=CY_DMA_SUCCESS)
     {
         return INIT_FAILURE;
     }

     /* Set source and destination for descriptor 1 */
     Cy_DMA_Descriptor_SetSrcAddress(&txDma_Descriptor_0, (uint8_t *)tx_buffer);
     Cy_DMA_Descriptor_SetDstAddress(&txDma_Descriptor_0, (void *)&SENSOR_SPI_HW->TX_FIFO_WR);

     /* Initialize and enable the interrupt from TxDma */
     #if defined(CY_DEVICE_PSC3)
     Cy_SysInt_Init(&intTxDma_cfg,&tx_dma_complete);
     #else
     cyhal_system_set_isr(txDma_IRQ, txDma_IRQ, TXDMA_INTERRUPT_PRIORITY, &tx_dma_complete);
     #endif
     NVIC_EnableIRQ((IRQn_Type)intTxDma_cfg.intrSrc);

      /* Enable DMA interrupt source. */
     Cy_DMA_Channel_SetInterruptMask(txDma_HW, txDma_CHANNEL, CY_DMA_INTR_MASK);
     
     /* Enable DMA block to start descriptor execution process */
     Cy_DMA_Enable(txDma_HW);
     
     return INIT_SUCCESS;
}

/******************************************************************************
* Function Name: tx_dma_complete
*******************************************************************************
*
* Summary:      This function check the tx DMA status
*
* Parameters:   None
*
* Return:       None
*
******************************************************************************/
void tx_dma_complete(void)
{
     /* Check tx DMA status */
     if ((CY_DMA_INTR_CAUSE_COMPLETION    != Cy_DMA_Channel_GetStatus(txDma_HW, txDma_CHANNEL)) &&
         (CY_DMA_INTR_CAUSE_CURR_PTR_NULL != Cy_DMA_Channel_GetStatus(txDma_HW, txDma_CHANNEL)))
     {
         /* DMA error occurred while TX operations */
         CY_ASSERT(0);
     }

     tx_dma_done = true;
     /* Clear tx DMA interrupt */
     Cy_DMA_Channel_ClearInterrupt(txDma_HW, txDma_CHANNEL);
}

/******************************************************************************
* Function Name: configure_rx_dma
*******************************************************************************
*
* Summary:      This function configure the receive DMA block 
*
* Parameters:   rx_buffer
*
* Return:       (uint32_t) INIT_SUCCESS or INIT_FAILURE
*
******************************************************************************/
uint32_t configure_rx_dma(uint8_t* rx_buffer)
{
     cy_en_dma_status_t dma_init_status;
     const cy_stc_sysint_t intRxDma_cfg =
     {
         .intrSrc      = rxDma_IRQ,
         .intrPriority = 7u
     };
     /* Initialize descriptor */
     dma_init_status = Cy_DMA_Descriptor_Init(&rxDma_Descriptor_0, &rxDma_Descriptor_0_config);
     if (dma_init_status!=CY_DMA_SUCCESS)
     {
         return INIT_FAILURE;
     }

     dma_init_status = Cy_DMA_Channel_Init(rxDma_HW, rxDma_CHANNEL, &rxDma_channelConfig);
     if (dma_init_status!=CY_DMA_SUCCESS)
     {
         return INIT_FAILURE;
     }

     /* Set source and destination for descriptor 1 */
     Cy_DMA_Descriptor_SetSrcAddress(&rxDma_Descriptor_0, (void *)&SENSOR_SPI_HW->RX_FIFO_RD);
     Cy_DMA_Descriptor_SetDstAddress(&rxDma_Descriptor_0, (uint8_t *)rx_buffer);

      /* Initialize and enable the interrupt from TxDma */
     #if defined(CY_DEVICE_PSC3)
     Cy_SysInt_Init(&intRxDma_cfg,&rx_dma_complete);
     #else
     cyhal_system_set_isr(rxDma_IRQ, rxDma_IRQ, RXDMA_INTERRUPT_PRIORITY, &rx_dma_complete);
     #endif
     NVIC_EnableIRQ((IRQn_Type)intRxDma_cfg.intrSrc);

      /* Enable DMA interrupt source. */
     Cy_DMA_Channel_SetInterruptMask(rxDma_HW, rxDma_CHANNEL, CY_DMA_INTR_MASK);
     
     /* Enable channel and DMA block to start descriptor execution process */
     Cy_DMA_Channel_Enable(rxDma_HW, rxDma_CHANNEL);
     
     Cy_DMA_Enable(rxDma_HW);
     return INIT_SUCCESS;
}

/******************************************************************************
* Function Name: rx_dma_complete
*******************************************************************************
*
* Summary:      This function check the rx DMA status
*
* Parameters:   None
*
* Return:       None
*
******************************************************************************/
void rx_dma_complete(void)
{
    /* Scenario: Inside the interrupt service routine for block DW0 channel 23: */
    if (CY_DMA_INTR_MASK == Cy_DMA_Channel_GetInterruptStatusMasked(rxDma_HW, rxDma_CHANNEL))
    {
        /* Get the interrupt cause */
        cy_en_dma_intr_cause_t cause = Cy_DMA_Channel_GetStatus(rxDma_HW, rxDma_CHANNEL);
        if (CY_DMA_INTR_CAUSE_COMPLETION != cause)
        {
            /* DMA error occurred while RX operations */
            CY_ASSERT(0);
        }
        else
        {
            rx_dma_done = true;
        }

        /* Clear the interrupt */
        Cy_DMA_Channel_ClearInterrupt(rxDma_HW, rxDma_CHANNEL);
    }
}
