#include "Controller/TLE987x_SPI/TLE987x_SPI.h"


uint32_t TLE987x_SPI_SendReceive(uint8_t* txBuffer)
{
	WDT1_Service();
	
	// TX most significant word (tx_msw) / least significant word (tx_lsw)
	uint16_t tx_msw = (txBuffer[0] << 8) | txBuffer[1]; 
	uint16_t tx_lsw = (txBuffer[2] << 8) | txBuffer[3];
	
	// RX most significant word (rx_msw) / least significant word (rx_lsw)
	uint16_t rx_msw;
	uint16_t rx_lsw;
	
	// Clear any pending receive data
    (void)SSC1->RB.reg;
	
	// Assert CS
	GPIO_SetOutputLevelLow(CS_PIN);
	
	// Write tx_msw and receive rx_msw
	Field_Wrt16(&SSC1->TB.reg, (uint16)SSC1_TB_TB_VALUE_Pos, (uint16)SSC1_TB_TB_VALUE_Msk, tx_msw);
	while (SSC1->CON.bit.BSY == 1u);
	rx_msw = u16_Field_Rd16(&SSC1->RB.reg, (uint16)SSC1_RB_RB_VALUE_Pos, (uint16)SSC1_RB_RB_VALUE_Msk);
	
	// Write tx_lsw and receive rx_lsw
	Field_Wrt16(&SSC1->TB.reg, (uint16)SSC1_TB_TB_VALUE_Pos, (uint16)SSC1_TB_TB_VALUE_Msk, tx_lsw);
	while (SSC1->CON.bit.BSY == 1u);
	rx_lsw = u16_Field_Rd16(&SSC1->RB.reg, (uint16)SSC1_RB_RB_VALUE_Pos, (uint16)SSC1_RB_RB_VALUE_Msk);
	
	// Deassert CS
	GPIO_SetOutputLevelHigh(CS_PIN);
	
	return (uint32_t) (rx_msw << 16) | rx_lsw;
}