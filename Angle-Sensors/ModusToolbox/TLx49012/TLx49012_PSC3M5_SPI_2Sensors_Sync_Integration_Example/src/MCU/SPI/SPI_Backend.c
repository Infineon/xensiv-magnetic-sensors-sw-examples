#include "SPI_Backend.h"
#include <stdint.h>


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


void PSC3M5_SPI_Interrupt(void)
{
    Cy_SCB_SPI_Interrupt(SENSOR_SPI_HW, &SENSOR_SPI_context);
}


void PSC3M5_SPI_Init(void)
{
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

    // Hook interrupt service routine and enable interrupt
    sysSPIstatus = Cy_SysInt_Init(&SENSOR_SPI_IRQ_cfg, &PSC3M5_SPI_Interrupt);
    
    if(sysSPIstatus != CY_SYSINT_SUCCESS)
    {
        CY_ASSERT(0);
    }
     
    // Enable interrupt in NVIC
    NVIC_EnableIRQ(SENSOR_SPI_IRQ);

    // Enable the SPI Master block
    Cy_SCB_SPI_Enable(SENSOR_SPI_HW);
}

uint32_t PSC3M5_SPI_SendReceive(uint8_t *txBuffer, uint8_t slaveSelect)
{
	if(slaveSelect == 0)
	{
	    Cy_SCB_SPI_SetActiveSlaveSelect(SENSOR_SPI_HW, CY_SCB_SPI_SLAVE_SELECT0);
	}
	else
	{
	    Cy_SCB_SPI_SetActiveSlaveSelect(SENSOR_SPI_HW, CY_SCB_SPI_SLAVE_SELECT1);
	}

    // Initiate SPI Master write transaction
    Cy_SCB_SPI_Transfer(SENSOR_SPI_HW, txBuffer, sensor_response, sizeof(txBuffer), &SENSOR_SPI_context);

    // Blocking wait for transfer completion
    while (0UL != (CY_SCB_SPI_TRANSFER_ACTIVE & Cy_SCB_SPI_GetTransferStatus(SENSOR_SPI_HW, &SENSOR_SPI_context)));
    
    // Assemble sensor response in a 32-bit word
    uint32_t response = (sensor_response[0] << 24) | (sensor_response[1] << 16) | (sensor_response[2] << 8) | sensor_response[3]; 
    
    // Return sensor response
    return response;
}

/* [] END OF FILE */


