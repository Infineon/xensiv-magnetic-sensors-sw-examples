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
#include "tle_device.h"

/*******************************************************************************
**                         Global Function Definitions                        **
*******************************************************************************/
void TLE_Init(void)
{
#ifdef RTE_DEVICE_SDK_SCU
  SCU_Init();
#endif
#ifdef RTE_DEVICE_SDK_PMU
  PMU_Init();
#endif
#ifdef RTE_DEVICE_SDK_ADC1
  ADC1_Init();
#endif
#ifdef RTE_DEVICE_SDK_ADC2
  ADC2_Init();
#endif
#ifdef RTE_DEVICE_SDK_ADC34
  SDADC_Init();
#endif
#ifdef RTE_DEVICE_SDK_BDRV
  BDRV_Init();
#endif
#ifdef RTE_DEVICE_SDK_CCU6
  CCU6_Init();
#endif
#ifdef RTE_DEVICE_SDK_CSA
  CSA_Init();
#endif
#ifdef RTE_DEVICE_SDK_GPT12E
  GPT12E_Init();
#endif
#ifdef RTE_DEVICE_SDK_LIN
  LIN_Init();
#endif
#ifdef RTE_DEVICE_SDK_MON
  MON_Init();
#endif
#ifdef RTE_DEVICE_SDK_SSC
  SSC1_Init();
  SSC2_Init();
#endif
#ifdef RTE_DEVICE_SDK_TIMER2X
  TIMER2_Init();
  TIMER21_Init();
#endif
#ifdef RTE_DEVICE_SDK_TIMER3
  TIMER3_Init();
#endif
#ifdef RTE_DEVICE_SDK_UART
  UART1_Init();
  UART2_Init();
#endif
#ifdef RTE_DEVICE_SDK_DMA
  DMA_Init();
#endif
#ifdef RTE_DEVICE_SDK_PORT
  PORT_Init();
#endif
#ifdef RTE_DEVICE_SDK_INT
  INT_Init();
#endif
}
