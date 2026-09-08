/*******************************************************************************
* File Name:   main.c
*
* Description: This example code provides a starting point in interfacing the 
*              TLx49012 angle sensors with the KIT_PSC3M5_CC2 evaluation board.
* 			   It exemplifies SPI synchronized angle readouts of two sensors.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2024-2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/**
 * @file        TLx49012_PSC3M5_SPI_2Sensors_Sync_Integration_Example
 * @version     1.0.0
 * @date        2026-04-08
 * @description This example code provides a starting point in interfacing the 
 *              TLx49012 angle sensors with the KIT_PSC3M5_CC2 evaluation board.
 *			    It exemplifies SPI synchronized angle readouts of two sensors. 
 * @changelog
 *   v1.0.0 - Initial release
 */

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_scb_common.h"
#include "cy_syslib.h"
#include "cycfg_peripherals.h"
#include "mtb_hal.h"
#include "cybsp.h"

#include "src/MCU/MCU.h"
#include "src/Sensor/TLx49012.h"
#include <stdio.h>


/*******************************************************************************
* Global variables
*******************************************************************************/
uint16_t g_angleLsb_sensor0;		// Angle value received via SPI.
uint16_t g_angleLsb_sensor1;		// Angle value received via SPI.

AngleSyncRegister g_angleSyncRegister_sensor0; // Register value
AngleSyncRegister g_angleSyncRegister_sensor1; // Register value
    

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This function provides continuous readout of the TLx49012 angle sensor.
* Peripherals are configured using the BSP-proprietary function, cybsp_init().
* Interrupt configuration for SPI is done in the PSC3M5_MCU_Init() function.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    // Initialize the device and board peripherals
    result = cybsp_init();

    // Board init failed. Stop program execution
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

	// Fully initialize peripherals, with SPI interrupt and UART HAL
	PSC3M5_MCU_Init();

	// Initialize CRC LUT and soft-fuse the sensor
	TLx49012_Init();

    // Enable global interrupts
    __enable_irq();
    
    for (;;)
    {
		/*	
		// Send SPI read command and get LSB angle value
		g_angleLsb_sensor0 = TLx49012_GetAngleLSB(SPI_SLAVE0);
		Cy_SysLib_DelayUs(1);
		g_angleLsb_sensor1 = TLx49012_GetAngleLSB(SPI_SLAVE1);
		
		// Print to serial port the angle in both LSB and degrees
		PSC3M5_UART_SendAngleInfo(g_angleLsb_sensor0, SPI_SLAVE0);
		PSC3M5_UART_SendAngleInfo(g_angleLsb_sensor1, SPI_SLAVE1);
		*/
		
		// Send synchronization trigger
		TLx49012_TriggerSyncPin();
		
		// Get sync register values
		g_angleSyncRegister_sensor0 = TLx49012_GetAngleSyncRegister(SPI_SLAVE0);
		Cy_SysLib_DelayUs(1);
		g_angleSyncRegister_sensor1 = TLx49012_GetAngleSyncRegister(SPI_SLAVE1);

		// Check for new valid/fresh data
		if((g_angleSyncRegister_sensor0.bitfieldAccess.readStatus == 0) && (g_angleSyncRegister_sensor1.bitfieldAccess.readStatus == 0))
		{
			// Fresh values - compute
			printf(("*** NEW FRESH VALUES ***\r\n"));
			PSC3M5_UART_SendSyncAngleInfo(g_angleSyncRegister_sensor0.bitfieldAccess.AngleSync,SPI_SLAVE0); 
			PSC3M5_UART_SendSyncAngleInfo(g_angleSyncRegister_sensor1.bitfieldAccess.AngleSync,SPI_SLAVE1); 
		}
		
		// ~20Hz readout
		Cy_SysLib_Delay(50);
    }
}

/* [] END OF FILE */
