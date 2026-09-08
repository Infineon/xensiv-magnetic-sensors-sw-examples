/*
 ***********************************************************************************************************************
 *
 * Copyright (c) 2015, Infineon Technologies AG
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

/*******************************************************************************
**                             Author(s) Identity                             **
********************************************************************************
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** BG           Blandine Guillot                                              **
*******************************************************************************/

/*******************************************************************************
**                          Revision Control History                          **
********************************************************************************
** V1.0.0: 2020-04-15, BG:   Initial version of revision history              **
** V1.0.1: 2020-07-21, BG:   EP-439: Formatted .h/.c files                    **
*******************************************************************************/

/*******************************************************************************
**                                  Abstract                                  **
********************************************************************************
** SSC1: Send one byte as SPI master                                          **
********************************************************************************
** SSC1 is set up to operate at 1Mbaud.                                       **
** SSC1 sends one byte at P0.3 (CLK), P0.2 (MTSR). Check that the data are    **
** received on P2.4 (MRST).                                                   **
*******************************************************************************/



/*************Code for Getting angle value from TLE5012B*************/

#include "tle_device.h"
#include <stdio.h>

void configSensor(void);
void readSensorAngle(void);
void calculateAngle(void);
void turnDirectionDetection(void);

uint16_t angval;
uint16_t angVal_cpy;
signed int value;
float ang_deg;
float max_angle = 360;
float max_15_bit_value = 32768;

int main(void)
{
  /* Initialization of hardware modules based on Config Wizard configuration */
  TLE_Init();
	
	/* Basic Sensor Configurations*/
	configSensor();  
  
  for (;;)
  {
    (void)WDT1_Service();
		readSensorAngle();
		calculateAngle();
		turnDirectionDetection();
		 
  }
}

/********Sensor Configuration************/

void configSensor(void)                 
{
	//Enable prediction, disable autocal
	PORT_P10_Output_Low_Set();
	SSC1_SendWord(0x5080);
	SSC1_SendWord(0x0804);
	PORT_P10_Output_High_Set();
	for(int i=0;i<1000;i++);
	
	//Read 0x0D (for IIF-Hysteresis option)
	PORT_P10_Output_Low_Set();
	SSC1_SendWord(0x80D0);
	uint16_t reg = SSC1_SendWord(0xFFFF);
	PORT_P10_Output_High_Set();
	for(int i=0;i<1000;i++);
	
	//Write Hysteresis option in 0x0D to 0x00 (0°)
	reg &= 0xFFFC;
	PORT_P10_Output_Low_Set();
	SSC1_SendWord(0x50D0);
	SSC1_SendWord(reg);
	PORT_P10_Output_High_Set();
	for(int i=0;i<1000;i++);
	
	//Enable prediction, enable autocal
	PORT_P10_Output_Low_Set();
	SSC1_SendWord(0x5080);
	SSC1_SendWord(0x0805);
	PORT_P10_Output_High_Set();
	for(int i=0;i<1000;i++);
	
	//read STAT to clear S_FUSE
	PORT_P10_Output_Low_Set();
	SSC1_SendWord(0x8000);
	SSC1_SendWord(0xFFFF);
	PORT_P10_Output_High_Set();
}

void readSensorAngle(void)
{
	
	PORT_P10_Output_Low_Set();
	SSC1_SendWord(0x8020);
	angval = SSC1_SendWord(0xFFFF);
	PORT_P10_Output_High_Set();
		
}

void calculateAngle(void)
{
	uint16_t i;

	angVal_cpy = (angval & 0x7FFF); 
	
	value = (-1 * (angVal_cpy & 0x4000));
	
	for(i = 13; i > 0; i--)
	{
		value = value | (angVal_cpy & (1 << i));
	}
	ang_deg = (max_angle / max_15_bit_value) * value;
}

void turnDirectionDetection(void)
{
	if (ang_deg < 0 )
	{
		ang_deg = ang_deg * -1;
    printf("\n***********Turning Left**********\n");
		printf("Turn Angle:%0.2f\r\n",ang_deg);
	}
  else if(ang_deg > 0)
  {
    printf("\n***********Turning Right**********\n");
		printf("Turn Angle:%0.2f\r\n",ang_deg);
  }		
	
}


