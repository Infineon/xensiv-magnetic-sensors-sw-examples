/**********************************************************************************************************************
* File Name:   main.c
*
* Description: 		This example code provides a starting point in interfacing the TLx49012 angle sensor with the 
*					TLE987x microcontroller family. The high-speed synchronous serial interface (SSC) peripheral is 
*					used to implement the SPI protocol for sensor configuration and angle readout.
*
* Platforms:		TLE987X EVALB_VQFN
*						- TLE9871QXA20
*						- TLE9872QXA40
*					TLE9879 EVALKIT
*						- TLE9879QXA40
*
* Related Document: See README.md
*
***********************************************************************************************************************
*
* Copyright (c) Infineon Technologies AG
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,are permitted provided that the
* following conditions are met:
*
*   Redistributions of source code must retain the above copyright notice, this list of conditions and the  following
*   disclaimer.
*
*   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
*   following disclaimer in the documentation and/or other materials provided with the distribution.
*
*   Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote
*   products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT  OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**********************************************************************************************************************/


/**
 * @file        TLx49012_TLE987x_Integration_Example
 * @version     1.0.0
 * @date        2026-07-30
 * @description This example code provides a starting point in interfacing the TLx49012 angle sensor with the 
 *				TLE987x microcontroller family. The high-speed synchronous serial interface (SSC) peripheral is 
 *				used to implement the SPI protocol for sensor configuration and angle readout.
 * @changelog
 *   v1.0.0 - Initial release
 */


/*******************************************************************************
* Header Files
*******************************************************************************/
#include "tle_device.h"
#include "eval_board.h"
#include <stdio.h>

#include "Controller/TLE987x_UART/TLE987x_UART.h"
#include "Sensor/TLx49012.h"


/*******************************************************************************
* Global variables
*******************************************************************************/
uint16_t angle_LSB;


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This function provides continuous readout of the TLx49012 angle sensor.
* Peripherals are configured using the PDL-proprietary function, TLE_Init().
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
	// Initialize TLE9871 peripherals
	TLE_Init();
	
	// Initialize TLx49012 sensor
	TLx49012_Init();
	
	while(1)
	{		
		// Service watchdog
		WDT1_Service();
		
		// Read 16-bit angle value
		angle_LSB = TLx49012_GetAngleLSB();
		
		// Send information to terminal 
		TLE987x_UART_SendAngleInfo(angle_LSB);
		
		// ~5.5 ms for UART transmission + 44.5 ms delay = 50 ms readout period (20Hz)
		Delay_us(44500);	
	}
}