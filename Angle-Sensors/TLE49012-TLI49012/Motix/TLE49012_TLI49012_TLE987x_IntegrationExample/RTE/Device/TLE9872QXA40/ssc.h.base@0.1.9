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
/**
 * \file     ssc.h
 *
 * \brief    High-Speed Synchronous Serial Interface low level access library
 *
 * \version  V0.1.9
 * \date     02. Jan 2025
 */

/*******************************************************************************
**                             Author(s) Identity                             **
********************************************************************************
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** DM           Daniel Mysliwitz                                              **
** TA           Thomas Albersinger                                            **
** BG           Blandine Guillot                                              **
** JO           Julia Ott                                                     **
*******************************************************************************/

/*******************************************************************************
**                          Revision Control History                          **
********************************************************************************
** V0.1.0: 2014-05-15, TA:   Initial version                                  **
** V0.1.1: 2015-02-10, DM:   Individual header file added                     **
** V0.1.2: 2015-06-24, DM:   SendWord functions return received data word     **
** V0.1.3: 2017-05-24, DM:   Interrupt APIs added                             **
** V0.1.4: 2017-10-18, DM:   MISRA 2012 compliance, the following PC-Lint     **
**                           rules are globally deactivated:                  **
**                           - Info 793: ANSI/ISO limit of 6 'significant     **
**                             characters in an external identifier           **
**                           - Info 835: A zero has been given as right       **
**                             argument to operator                           **
**                           - Info 845: The left argument to operator '&'    **
**                             is certain to be 0                             **
**                           Replaced macros by INLINE functions              **
**                           Replaced register accesses within functions by   **
**                           function calls                                   **
**                           Replaced __STATIC_INLINE by INLINE               **
** V0.1.5: 2018-11-27, JO:   Doxygen update                                   **
**                           Moved revision history from ssc.c to ssc.h       **
**                           Replaced __STATIC_INLINE by INLINE               **
** V0.1.6: 2020-04-15, BG:   Updated revision history format                  **
** V0.1.7: 2020-07-21, BG:   EP-439: Formatted .h/.c files                    **
** V0.1.8: 2022-02-28, JO:   EP-936: Updated copyright and branding           **
** V0.1.9: 2025-01-02, JO:   EP-1493: Updated license                         **
*******************************************************************************/

#ifndef SSC_H
#define SSC_H

/*******************************************************************************
**                                  Includes                                  **
*******************************************************************************/
#include "tle987x.h"
#include "types.h"
#include "sfr_access.h"

/*******************************************************************************
**                          Global Macro Definitions                          **
*******************************************************************************/
/**\brief SSC1 Manual Baudrate*/
#define SSC1_tBit_us (1.0 / (SSC1_MAN_BAUDRATE / 1000.0))
/**\brief SSC2 Manual Baudrate*/
#define SSC2_tBit_us (1.0 / (SSC2_MAN_BAUDRATE / 1000.0))

/*******************************************************************************
**                     Global Inline Function Definitions                     **
*******************************************************************************/
/** \brief clears transmit interrupt flag for SSC1.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the transmit interrupt for SSC1.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC1_TX_Int_En();
 *   if ((uint8)SCU->IRCON1.bit.TIR == (uint8)1)
 *   {
 *     SSC1_TX_CALLBACK();
 *     SSC1_TX_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC1_TX_Int_Clr(void)
{
  Field_Wrt8(&SCU->IRCON1CLR.reg, (uint8)SCU_IRCON1CLR_TIRC_Pos, (uint8)SCU_IRCON1CLR_TIRC_Msk, 1u);
}

/** \brief clears receive interrupt flag for SSC1.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the receive interrupt for SSC1.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC1_RX_Int_En();
 *   if ((uint8)SCU->IRCON1.bit.RIR == (uint8)1)
 *   {
 *     SSC1_RX_CALLBACK();
 *     SSC1_RX_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC1_RX_Int_Clr(void)
{
  Field_Wrt8(&SCU->IRCON1CLR.reg, (uint8)SCU_IRCON1CLR_RIRC_Pos, (uint8)SCU_IRCON1CLR_RIRC_Msk, 1u);
}

/** \brief clears error interrupt flag for SSC1.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the error interrupt for SSC1.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC1_Err_Int_En();
 *   if ((uint8)SCU->IRCON1.bit.EIR == (uint8)1)
 *   {
 *     SSC1_ERR_CALLBACK();
 *     SSC1_Err_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC1_Err_Int_Clr(void)
{
  Field_Wrt8(&SCU->IRCON1CLR.reg, (uint8)SCU_IRCON1CLR_EIRC_Pos, (uint8)SCU_IRCON1CLR_EIRC_Msk, 1u);
}

/** \brief clears transmit interrupt flag for SSC2.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats thd transmit interrupt for SSC2.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC2_TX_Int_En();
 *   if ((uint8)SCU->IRCON2.bit.TIR == (uint8)1)
 *   {
 *     SSC2_TX_CALLBACK();
 *     SSC2_TX_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC2_TX_Int_Clr(void)
{
  Field_Wrt8(&SCU->IRCON2CLR.reg, (uint8)SCU_IRCON2CLR_TIRC_Pos, (uint8)SCU_IRCON2CLR_TIRC_Msk, 1u);
}

/** \brief clears receive interrupt flag for SSC2.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the receive interrupt for SSC2.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC2_RX_Int_En();
 *   if ((uint8)SCU->IRCON2.bit.RIR == (uint8)1)
 *   {
 *     SSC2_RX_CALLBACK();
 *     SSC2_RX_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC2_RX_Int_Clr(void)
{
  Field_Wrt8(&SCU->IRCON2CLR.reg, (uint8)SCU_IRCON2CLR_RIRC_Pos, (uint8)SCU_IRCON2CLR_RIRC_Msk, 1u);
}

/** \brief clears error interrupt flag for SSC2.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the error interrupt for SSC2.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC2_Err_Int_En();
 *   if ((uint8)SCU->IRCON2.bit.EIR == (uint8)1)
 *   {
 *     SSC2_ERR_CALLBACK();
 *     SSC2_Err_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC2_Err_Int_Clr(void)
{
  Field_Wrt8(&SCU->IRCON2CLR.reg, (uint8)SCU_IRCON2CLR_EIRC_Pos, (uint8)SCU_IRCON2CLR_EIRC_Msk, 1u);
}

/** \brief enables transmit interrupt for SSC1.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats thd transmit interrupt for SSC1.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC1_TX_Int_En();
 *   if ((uint8)SCU->IRCON1.bit.TIR == (uint8)1)
 *   {
 *     SSC1_TX_CALLBACK();
 *     SSC1_TX_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC1_TX_Int_En(void)
{
  Field_Mod8(&SCU->MODIEN1.reg, (uint8)SCU_MODIEN1_TIREN1_Pos, (uint8)SCU_MODIEN1_TIREN1_Msk, 1u);
}

/** \brief disables transmit interrupt for SSC1.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats thd transmit interrupt for SSC1.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC1_TX_Int_En();
 *   if ((uint8)SCU->IRCON1.bit.TIR == (uint8)1)
 *   {
 *     SSC1_TX_CALLBACK();
 *     SSC1_TX_Int_Clr();
 *   }
 *   SSC1_TX_Int_Dis();
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC1_TX_Int_Dis(void)
{
  Field_Mod8(&SCU->MODIEN1.reg, (uint8)SCU_MODIEN1_TIREN1_Pos, (uint8)SCU_MODIEN1_TIREN1_Msk, 0u);
}

/** \brief enables receive interrupt for SSC1.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the receive interrupt for SSC1.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC1_RX_Int_En();
 *   if ((uint8)SCU->IRCON1.bit.RIR == (uint8)1)
 *   {
 *     SSC1_RX_CALLBACK();
 *     SSC1_RX_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC1_RX_Int_En(void)
{
  Field_Mod8(&SCU->MODIEN1.reg, (uint8)SCU_MODIEN1_RIREN1_Pos, (uint8)SCU_MODIEN1_RIREN1_Msk, 1u);
}

/** \brief disables receive interrupt for SSC1.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the receive interrupt for SSC1.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC1_RX_Int_En();
 *   if ((uint8)SCU->IRCON1.bit.RIR == (uint8)1)
 *   {
 *     SSC1_RX_CALLBACK();
 *     SSC1_RX_Int_Clr();
 *   }
 *   SSC1_RX_Int_Dis();
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC1_RX_Int_Dis(void)
{
  Field_Mod8(&SCU->MODIEN1.reg, (uint8)SCU_MODIEN1_RIREN1_Pos, (uint8)SCU_MODIEN1_RIREN1_Msk, 0u);
}

/** \brief enables error interrupt for SSC1.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the error interrupt for SSC1.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC1_Err_Int_En();
 *   if ((uint8)SCU->IRCON1.bit.EIR == (uint8)1)
 *   {
 *     SSC1_ERR_CALLBACK();
 *     SSC1_Err_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC1_Err_Int_En(void)
{
  Field_Mod8(&SCU->MODIEN1.reg, (uint8)SCU_MODIEN1_EIREN1_Pos, (uint8)SCU_MODIEN1_EIREN1_Msk, 1u);
}

/** \brief disables error interrupt for SSC1.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the error interrupt for SSC1.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC1_Err_Int_En();
 *   if ((uint8)SCU->IRCON1.bit.EIR == (uint8)1)
 *   {
 *     SSC1_ERR_CALLBACK();
 *     SSC1_Err_Int_Clr();
 *   }
 *   SSC1_Err_Int_Dis();
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC1_Err_Int_Dis(void)
{
  Field_Mod8(&SCU->MODIEN1.reg, (uint8)SCU_MODIEN1_EIREN1_Pos, (uint8)SCU_MODIEN1_EIREN1_Msk, 0u);
}

/** \brief enables transmit interrupt for SSC2.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats thd transmit interrupt for SSC2.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC2_TX_Int_En();
 *   if ((uint8)SCU->IRCON2.bit.TIR == (uint8)1)
 *   {
 *     SSC2_TX_CALLBACK();
 *     SSC2_TX_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC2_TX_Int_En(void)
{
  Field_Mod8(&SCU->MODIEN2.reg, (uint8)SCU_MODIEN2_TIREN2_Pos, (uint8)SCU_MODIEN2_TIREN2_Msk, 1u);
}

/** \brief disables transmit interrupt for SSC2.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats thd transmit interrupt for SSC2.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC2_TX_Int_En();
 *   if ((uint8)SCU->IRCON2.bit.TIR == (uint8)1)
 *   {
 *     SSC2_TX_CALLBACK();
 *     SSC2_TX_Int_Clr();
 *   }
 *   SSC2_TX_Int_Dis();
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC2_TX_Int_Dis(void)
{
  Field_Mod8(&SCU->MODIEN2.reg, (uint8)SCU_MODIEN2_TIREN2_Pos, (uint8)SCU_MODIEN2_TIREN2_Msk, 0u);
}

/** \brief enables receive interrupt for SSC2.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the receive interrupt for SSC2.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC2_RX_Int_En();
 *   if ((uint8)SCU->IRCON2.bit.RIR == (uint8)1)
 *   {
 *     SSC2_RX_CALLBACK();
 *     SSC2_RX_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC2_RX_Int_En(void)
{
  Field_Mod8(&SCU->MODIEN2.reg, (uint8)SCU_MODIEN2_RIREN2_Pos, (uint8)SCU_MODIEN2_RIREN2_Msk, 1u);
}

/** \brief disables receive interrupt for SSC2.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the receive interrupt for SSC2.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC2_RX_Int_En();
 *   if ((uint8)SCU->IRCON2.bit.RIR == (uint8)1)
 *   {
 *     SSC2_RX_CALLBACK();
 *     SSC2_RX_Int_Clr();
 *   }
 *   SSC2_RX_Int_Dis();
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC2_RX_Int_Dis(void)
{
  Field_Mod8(&SCU->MODIEN2.reg, (uint8)SCU_MODIEN2_RIREN2_Pos, (uint8)SCU_MODIEN2_RIREN2_Msk, 0u);
}

/** \brief enables error interrupt for SSC2.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the error interrupt for SSC2.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC2_Err_Int_En();
 *   if ((uint8)SCU->IRCON2.bit.EIR == (uint8)1)
 *   {
 *     SSC2_ERR_CALLBACK();
 *     SSC2_Err_Int_Clr();
 *   }
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC2_Err_Int_En(void)
{
  Field_Mod8(&SCU->MODIEN2.reg, (uint8)SCU_MODIEN2_EIREN2_Pos, (uint8)SCU_MODIEN2_EIREN2_Msk, 1u);
}

/** \brief disables error interrupt for SSC2.
 *
 * \brief <b>Example</b><br>
 * \brief This example treats the error interrupt for SSC2.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *   SSC2_Err_Int_En();
 *   if ((uint8)SCU->IRCON2.bit.EIR == (uint8)1)
 *   {
 *     SSC2_ERR_CALLBACK();
 *     SSC2_Err_Int_Clr();
 *   }
 *   SSC2_Err_Int_Dis();
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE void SSC2_Err_Int_Dis(void)
{
  Field_Mod8(&SCU->MODIEN2.reg, (uint8)SCU_MODIEN2_EIREN2_Pos, (uint8)SCU_MODIEN2_EIREN2_Msk, 0u);
}

/*******************************************************************************
**                        Global Function Declarations                        **
*******************************************************************************/
/** \brief Initializes the SSC1 module based on the Config Wizard for MOTIX MCU configuration
 *
 * \ingroup ssc_api
 */
void SSC1_Init(void);

/** \brief Initializes the SSC2 module based on the Config Wizard for MOTIX MCU configuration
 *
 * \ingroup ssc_api
 */
void SSC2_Init(void);

/*******************************************************************************
**                     Global Inline Function Declarations                    **
*******************************************************************************/
INLINE uint16 SSC1_SendWord(uint16 DataWord);
INLINE uint16 SSC1_ReadWord(void);
INLINE uint16 SSC2_SendWord(uint16 DataWord);
INLINE uint16 SSC2_ReadWord(void);

/*******************************************************************************
**                     Global Inline Function Definitions                     **
*******************************************************************************/
/** \brief SSC1: Send data word.
 *
 * \param DataWord Data to send
 * \return Received data
 *
 * \brief <b>Example</b><br>
 * \brief This example sends "A" with SSC1 and reads the received data.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *     uint16 data;
 *
 *     data = SSC1_SendWord(0x41);
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE uint16 SSC1_SendWord(uint16 DataWord)
{
  Field_Wrt16(&SSC1->TB.reg, (uint16)SSC1_TB_TB_VALUE_Pos, (uint16)SSC1_TB_TB_VALUE_Msk, DataWord);
  return u16_Field_Rd16(&SSC1->RB.reg, (uint16)SSC1_RB_RB_VALUE_Pos, (uint16)SSC1_RB_RB_VALUE_Msk);
}

/** \brief SSC1: Read data word from receive buffer.
 *
 * \return Received data
 *
 * \brief <b>Example</b><br>
 * \brief This example receives data with SSC1.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *     uint16 data;
 *
 *     data = SSC1_ReadWord();
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE uint16 SSC1_ReadWord(void)
{
  return u16_Field_Rd16(&SSC1->RB.reg, (uint16)SSC1_RB_RB_VALUE_Pos, (uint16)SSC1_RB_RB_VALUE_Msk);
}

/** \brief SSC2: Send data word.
 *
 * \param DataWord Data to send
 * \return Received data
 *
 * \brief <b>Example</b><br>
 * \brief This example sends "A" with SSC2 and reads the received data.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *     uint16 data;
 *
 *     data = SSC2_SendWord(0x41);
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE uint16 SSC2_SendWord(uint16 DataWord)
{
  Field_Wrt16(&SSC2->TB.reg, (uint16)SSC2_TB_TB_VALUE_Pos, (uint16)SSC2_TB_TB_VALUE_Msk, DataWord);
  return u16_Field_Rd16(&SSC2->RB.reg, (uint16)SSC2_RB_RB_VALUE_Pos, (uint16)SSC2_RB_RB_VALUE_Msk);
}

/** \brief SSC2: Read data word from receive buffer.
 *
 * \return Received data
 *
 * \brief <b>Example</b><br>
 * \brief This example receives data with SSC2.
 * ~~~~~~~~~~~~~~~{.c}
 * void Example_Function(void)
 * {
 *     uint16 data;
 *
 *     data = SSC2_ReadWord();
 * }
 * ~~~~~~~~~~~~~~~
 * \ingroup ssc_api
 */
INLINE uint16 SSC2_ReadWord(void)
{
  return u16_Field_Rd16(&SSC2->RB.reg, (uint16)SSC2_RB_RB_VALUE_Pos, (uint16)SSC2_RB_RB_VALUE_Msk);
}

#endif
