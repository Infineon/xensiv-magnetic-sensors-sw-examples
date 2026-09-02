/**********************************************************************************************************************
* File Name:   main.c
*
* Description: 		This example code provides a starting point in interfacing the TLE5502 angle sensor with the 
*					TLE987x microcontroller family.
*
* Platforms:		TLE987X EVALB_VQFN
*									- TLE9871QXA20
*									- TLE9872QXA40
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
 * @file        TLE5502_TLE987x_Integration_Example
 * @version     1.0.0
 * @date        2026-07-30
 * @description This example code provides a starting point in interfacing the TLE5502 angle sensor with the 
 *				TLE987x microcontroller family.
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
#include "Controller/TLE987x_GPIO/TLE987x_GPIO.h"
#include "Sensor/TLE5502.h"


/*******************************************************************************
* Global variables
*******************************************************************************/
TLE5502_t TLE5502_data;				/* latest sensor snapshot: 4 channel voltages + angle */


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This function provides continuous readout of the TLE5502 angle sensor.
* Peripherals are configured using the PDL-proprietary function, TLE_Init().
*
*******************************************************************************/
int main(void)
{
	// Initialize TLE987x peripherals (clock, ports, UART, WDT1) as configured in the PDL
	TLE_Init();
	
	// Configure ADC1 sequence for the four differential sin/cos inputs
	ADC_Init_SinCos();
	
	while(1)
	{		
		// Service watchdog
		WDT1_Service();	
		
		// Sample all four channels and decode the mechanical angle
		TLE5502_data = TLE5502_GetAngleAndVoltage();
		
		
		// Send information to terminal (mV -> V for readability)
		TLE987x_UART_SendVoltageAngleInfo(TLE5502_data.SIN_P_mV / 1000.0f,
																			TLE5502_data.SIN_N_mV / 1000.0f,
																			TLE5502_data.COS_P_mV / 1000.0f,
																			TLE5502_data.COS_N_mV / 1000.0f,
																			TLE5502_data.ANGLE_DEG);
				
		// Toggles P0_4 LED once per loop as data indication @ 10Hz
		GPIO_TogglePin(BLINK_PIN);
		
		// ~14.25 ms for data decoding and UART transmission + 35.75 ms delay = 50 ms readout period (20Hz)
		Delay_us(35750);	
	}
}