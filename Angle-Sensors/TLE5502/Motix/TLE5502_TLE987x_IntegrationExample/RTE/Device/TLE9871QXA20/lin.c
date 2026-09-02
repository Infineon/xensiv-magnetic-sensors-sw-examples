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
#include "lin.h"
#include "lin_defines.h"

/*******************************************************************************
**                         Global Function Definitions                        **
*******************************************************************************/

void LIN_Init(void)
{
#if (LIN_Configuration_En == 1)
  /* set LIN Trx into Sleep Mode           */
  LIN_Set_Mode(LIN_MODE_SLEEP);

  /* wait until LIN Trx switched           *
   * into Sleep Mode                       */
  while (LIN_Get_Mode() != LIN_GET_MODE_SLEEP)
  {}

  /* write register according to Config Wizard for MOTIX MCU **
  ** settings, incl. target LIN Slope Mode       */
  LIN->CTRL_STS.reg = (uint32) LIN_CTRL_STS;
  SCU->LINST.reg = (uint8) SCU_LINST;
#endif
}
