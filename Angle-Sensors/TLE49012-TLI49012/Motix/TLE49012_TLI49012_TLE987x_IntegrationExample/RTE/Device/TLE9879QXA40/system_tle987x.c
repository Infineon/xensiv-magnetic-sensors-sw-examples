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
#include "tle987x.h"
#include "bsl_defines.h"
#include "scu.h"
#include "wdt1.h"
#include "tle_variants.h"
#include "RTE_Components.h"
#include "system_tle987x.h"

/*----------------------------------------------------------------------------
  Define BSL parameter
 *----------------------------------------------------------------------------*/
#if defined (__CC_ARM) || defined(__ARMCC_VERSION)
  /* ARMCC v5: __CC_ARM, ARMCC v6: __ARMCC_VERSION */
  #if (CONFIGWIZARD == 1)
    #if (NAC_NAD_EN == 1)
      const uint32 p_NACNAD __attribute__((section(sNADStart), used)) = (uint32)NAD_NAC;
    #endif
  #else /* (CONFIGWIZARD == 2) */
    #if (BSL_NAC_NAD_EN == 1u)
      const uint32 p_NACNAD __attribute__((section(sNADStart), used)) = (uint32)BSL_NAD_NAC;
    #endif
  #endif
#elif defined(__IAR_SYSTEMS_ICC__)
  /* IAR Compiler */
  #if (CONFIGWIZARD == 1)
    #if (NAC_NAD_EN == 1)
      __root const uint32 p_NACNAD @ NACStart = (uint32)NAD_NAC;
    #endif
  #else /* (CONFIGWIZARD == 2) */
    #if (BSL_NAC_NAD_EN == 1u)
      __root const uint32 p_NACNAD @ NACStart = (uint32)BSL_NAD_NAC;
    #endif
  #endif
#else
  #error Unsupported compiler!
#endif

void SystemInit(void)
{
  /* Vector table relocation */
  CPU->VTOR.reg   = ProgFlashStart;
  SCU_ClkInit();
  SysTick_Init();
  WDT1_Init();
  /* erratas */
  /* clear IE flags */
  TIMER2->T2CON1.reg = 0;
  TIMER21->T2CON1.reg = 0;
}
