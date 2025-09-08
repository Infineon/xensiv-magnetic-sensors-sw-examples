/*
 ***********************************************************************************************************************
 *
 * Copyright (c) Infineon Technologies AG
 * All rights reserved.
 *
 * The applicable license agreement can be found at this pack's installation directory in the file
 * license/IFX_SW_Licence_MOTIX_LITIX.txt
 *
 **********************************************************************************************************************/

/*******************************************************************************
**                             Author(s) Identity                             **
********************************************************************************
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** BG           Blandine Guillot                                              **
** JO           Julia Ott                                                     **
*******************************************************************************/

/*******************************************************************************
**                          Revision Control History                          **
********************************************************************************
** V1.0.0: 2020-04-15, BG:   Initial version of revision history              **
** V1.0.1: 2020-07-21, BG:   EP-439: Formatted .h/.c files                    **
** V1.0.2: 2020-08-04, BG:   EP-431: Fixed ARMCC v6 compiler warnings         **
** V1.1.0: 2024-11-06, JO:   EP-1535: Fixed compiler warnings (ARMCC V6.22)   **
** V1.1.1: 2025-01-02, JO:   EP-1493: Updated license                         **
*******************************************************************************/

/*******************************************************************************
**                                  Abstract                                  **
********************************************************************************
** Timer2: Measurement of the PWM (duty cycle/frequency)                      **
********************************************************************************
** Two boards are necessary for this example:                                 **
**  - 1st board: flash this example -> master board                           **
**  - 2nd board: flash the CCU6_PWM_Generation example -> slave board         **
** Connect P1.2 of the master board with one of the PWM output pins of the    **
** slave board.                                                               **
**                                                                            **
** Measuring the low part of the PWM:                                         **
** Timer2 starts counting with a falling edge on P1.2                         **
** Timer2 captures on the rising edge on P1.2 (PWM low phase, capDC)          **
**                                                                            **
** Measuring the period of the PWM:                                           **
** Timer2 still runs from previous falling edge                               **
** Timer2 captures on the falling edge on P1.2 (PWM period, capPer)           **
**                                                                            **
** Always one period is skipped in order to reprogram the edge detection of   **
** the Timer2.                                                                **
**                                                                            **
** Max. PWM period is 1.6ms (625Hz)                                           **
** @ 1kHz PWM frequency (1ms) the min. recognizable DC is 0.3%, max. 99.8%    **
********************************************************************************
**         start          capture   capture                capture            **
**         Timer2         Timer2    Timer2                 Timer2             **
**       -----                --------              ---------                 **
** P1.2       |              |        |            |         |                **
**            :--------------:\       :------------          :------...       **
**            :    capDC     : \      :\                     :    capDC       **
**            :<------------>:  \     : \                    :<-----...       **
**            :                  \    :  \                   :                **
**            :     PWM_Capture() ISR :   \                  :                **
**           (captured on rising edge):    \                 :                **
**            :                       :     \                :                **
**            :    capPer             :      \               :    capPer      **
**            :<--------------------->:       \              :<-----...       **
**                                             \                              **
**                                      PWM_Capture() ISR                     **
**                            (captured on  falling edge)                     **
*******************************************************************************/

#include "tle_device.h"
#include "eval_board.h"
#include <stdio.h>

static float32 capPer, capDC,capON;
static uint16 bDC_Per;
static float32 tOn,angDeg;
	

void PWM_Capture(void);
void calculate_Angle(void);
void display_Angle(void);


int main(void)
{
  /* Initialization of hardware modules based on Config Wizard configuration */
  TLE_Init();
  bDC_Per = 0;
  
  while (1)
  {
		calculate_Angle();
		display_Angle();
    (void)WDT1_Service();
	}
}

void calculate_Angle(void)
{
	tOn = (float32)capON/capPer;
	tOn = tOn * 100;
	angDeg = (((tOn - 6.25) * (((float32)1/244) * ((float32)360 / (87.5/244)))));
	
}

void display_Angle(void)
{
	printf("*************\n");
	printf("The Current measured angle is: %f", angDeg);
	printf("\n*************\n");
}

/* Callback function for Timer2 */
void PWM_Capture(void)
{
  /* Timer2 in run mode */
  if (TIMER21->T2CON.bit.TR2 == 1u)
  {
    if (bDC_Per == 0u)
    {
      /* Get the duty cycle */
      capDC = TIMER21_Get_Capture();
      /* Next capture on falling edge */
      TIMER21->T2MOD.bit.EDGESEL = 0u;
      bDC_Per = 1u;
    }
    else
    {
      /* Get the PWM period */
      capPer = TIMER21_Get_Capture();
			capON = capPer - capDC;
      /* Next capture on rising edge */
      TIMER21->T2MOD.bit.EDGESEL = 1u;
      bDC_Per = 0u;
      /* Stop Timer2 */
      TIMER21_Stop();
      /* Reset Timer2 */
      TIMER21_Clear_Count();
    }
  }
}


