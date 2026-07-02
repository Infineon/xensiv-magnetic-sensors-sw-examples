#include "SPI_Backend.h"
#include "cy_dma.h"
#include "cy_scb_spi.h"
#include <stdint.h>


// DMA buffers
uint8_t tx_buffer[4];
uint8_t rx_buffer[4];

// SPI context, used in SPI High-level protocol
cy_stc_scb_spi_context_t SENSOR_SPI_context;

// Interrupt configuration structure
const cy_stc_sysint_t SENSOR_SPI_IRQ_cfg =
    {
	    .intrSrc      = SENSOR_SPI_IRQ,
	    .intrPriority = SCB_SPI_INTR_PRIORITY
    };

// Array for storing the last sensor response
static uint8_t sensor_response[4] = {};

void PSC3M5_SPI_Init(void)
{
	uint32_t status = 0;
    cy_en_scb_spi_status_t result;
    cy_en_sysint_status_t sysSPIstatus;

    // Configure the SPI block
    result = Cy_SCB_SPI_Init(SENSOR_SPI_HW, &SENSOR_SPI_config, &SENSOR_SPI_context);
    if( result != CY_SCB_SPI_SUCCESS)
    {
        CY_ASSERT(0);
    }
    
    // Set active slave select to line 0
    Cy_SCB_SPI_SetActiveSlaveSelect(SENSOR_SPI_HW, CY_SCB_SPI_SLAVE_SELECT0);

    // Enable the SPI Master block
    Cy_SCB_SPI_Enable(SENSOR_SPI_HW);

	// Configure DMA
	status = configure_rx_dma(rx_buffer);
	if (INIT_FAILURE == status)
    {
        /* NOTE: This function will block the CPU forever */
       CY_ASSERT(0);
    }

	status = configure_tx_dma(tx_buffer);
	if (INIT_FAILURE == status)
    {
        /* NOTE: This function will block the CPU forever */
       CY_ASSERT(0);
    }

}

uint32_t PSC3M5_SPI_SendReceive(uint8_t *txBuffer, uint8_t slaveSelect)
{
	// Switch slave
	if(slaveSelect == 0)
	{
	    Cy_SCB_SPI_SetActiveSlaveSelect(SENSOR_SPI_HW, CY_SCB_SPI_SLAVE_SELECT0);
	}
	else
	{
	    Cy_SCB_SPI_SetActiveSlaveSelect(SENSOR_SPI_HW, CY_SCB_SPI_SLAVE_SELECT1);
	}

	//Copy data to DMA tx_buffer
	memcpy(tx_buffer, txBuffer, sizeof(tx_buffer));

    /* Re enable DMA channel to transfer 4 bytes of data from tx_buffer into SPI TX-FIFO */
    Cy_DMA_Channel_Enable(txDma_HW, txDma_CHANNEL);

    /* Wait for DMA transmit transaction */
    while (false == tx_dma_done) {}
    tx_dma_done = false;

	/* Wait for DMA receive transaction */
	while(false == rx_dma_done){}
	
	/* Assemble received sensor response in a 32-bit word */
	uint32_t response = (rx_buffer[0] << 24) | (rx_buffer[1] << 16) | (rx_buffer[2] << 8) | rx_buffer[3];
	rx_dma_done = false;

	/* Re enable DMA channel to receive 4 bytes of data in rx_buffer from SPI RX-FIFO */
	Cy_DMA_Channel_Enable(rxDma_HW, rxDma_CHANNEL);
    
    /* Return sensor response */
    return response;
}

/* [] END OF FILE */


