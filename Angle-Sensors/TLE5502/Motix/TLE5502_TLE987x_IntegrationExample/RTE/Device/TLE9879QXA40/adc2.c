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
**                                  Includes                                  **
*******************************************************************************/
#include "adc2.h"

/*******************************************************************************
**                         Global Function Definitions                        **
*******************************************************************************/

void ADC2_Init(void)
{
#if ((ADC2_CTRL2 & 1u) != 0u)
  ADC2->CTRL1.reg = (uint32) ADC2_CTRL1;
  ADC2->CTRL2.reg = (uint32) ADC2_CTRL2;
  ADC2->CTRL4.reg = (uint32) ADC2_CTRL4;
  ADC2->CTRL_STS.reg = (uint32)(ADC2_CTRL_STS | 1u);
  ADC2->TH0_3_LOWER.reg = (uint32) ADC2_TH0_3_LOWER;
  ADC2->TH0_3_UPPER.reg = (uint32) ADC2_TH0_3_UPPER;
  ADC2->TH4_5_LOWER.reg = (uint32) ADC2_TH4_5_LOWER;
  ADC2->TH4_5_UPPER.reg = (uint32) ADC2_TH4_5_UPPER;
  ADC2->TH6_9_LOWER.reg = (uint32) ADC2_TH6_9_LOWER;
  /*ADC2->TH6_9_UPPER is protected and cannot be changed in user mode*/
  ADC2->FILT_LO_CTRL.reg = ADC2_FILT_LO_CTRL;
  ADC2->FILT_UP_CTRL.reg = ADC2_FILT_UP_CTRL;
  ADC2->FILTCOEFF0_5.reg = (uint32) ADC2_FILTCOEFF0_5;
#if (ADC2_XML_VERSION >= 10300)
  ADC2->CNT0_3_LOWER.reg = (uint32) ADC2_CNT0_3_LOWER;
  ADC2->CNT0_3_UPPER.reg = (uint32) ADC2_CNT0_3_UPPER;
  ADC2->CNT4_5_LOWER.reg = (uint32) ADC2_CNT4_5_LOWER;
  ADC2->CNT4_5_UPPER.reg = (uint32) ADC2_CNT4_5_UPPER;
  ADC2->MMODE0_5.reg = (uint32) ADC2_MMODE0_5;
#endif
#endif
}
