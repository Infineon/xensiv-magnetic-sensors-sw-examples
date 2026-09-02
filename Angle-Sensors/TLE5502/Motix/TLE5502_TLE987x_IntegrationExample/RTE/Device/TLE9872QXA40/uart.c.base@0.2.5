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
#include "uart.h"
#include "wdt1.h"
#include "sfr_access.h"
#include "scu_defines.h"
#if ((UART1_STD_EN == 1) || (UART2_STD_EN == 1))
  #include <stdio.h>
#endif

/*******************************************************************************
**                             External CallBacks                             **
*******************************************************************************/
static TUart uart1;
static TUart uart2;

/*******************************************************************************
**                         Global Function Definitions                        **
*******************************************************************************/

void UART1_Init(void)
{
#if (UART1_Configuration_En == 1)
  UART1->SCON.reg = (uint8) UART1_SCON;
  SCU->BCON1.reg = (uint8) 0;
  SCU->BGH1.reg = (uint8) (UART1_BRVAL >> 3u);
  SCU->BGL1.reg = (uint8) (((UART1_BRVAL & 7u) << 5u) | UART1_FD);
  SCU->BCON1.reg = (uint8) SCU_BCON1;
  uart1.clock = UART1_CLK;
#endif
}

void UART1_BaudRate_Set(uint32 baudrate)
{
  uint16 brval_u11q5;

  if ((baudrate >= (uint16)1221) && (baudrate <= (uint32)1250000))
  {
    brval_u11q5 = (uint16)(((((uint32)uart1.clock * (uint32)1E6) << 1u) / baudrate));
    UART1_BaudRateGen_Dis();
    UART1_BaudRate_Value_Set(brval_u11q5);
    UART1_BaudRateGen_En();
  }
}

void UART2_Init(void)
{
#if (UART2_Configuration_En == 1)
  UART2->SCON.reg = (uint8) UART2_SCON;
  SCU->BCON2.reg = (uint8) 0;
  SCU->BGH2.reg = (uint8) (UART2_BRVAL >> 3u);
  SCU->BGL2.reg = (uint8) (((UART2_BRVAL & 7u) << 5u) | UART2_FD);
  SCU->BCON2.reg = (uint8) SCU_BCON2;
  uart2.clock = UART2_CLK;
#endif
}

void UART2_BaudRate_Set(uint32 baudrate)
{
  uint16 brval_u11q5;

  if ((baudrate >= (uint16)1221) && (baudrate <= (uint32)1250000))
  {
    brval_u11q5 = (uint16)(((((uint32)uart2.clock * (uint32)1E6) << 1u) / baudrate));
    UART2_BaudRateGen_Dis();
    UART2_BaudRate_Value_Set(brval_u11q5);
    UART2_BaudRateGen_En();
  }
}

#if ((UART1_STD_EN == 1) && (UART2_STD_EN == 0))
/** \brief Sends a character via UART1.
 *
 * \param Char Character to send
 * \return Sent character
 *
 * \ingroup uart_api
 */
sint32 stdout_putchar(sint32 Char)
{
  UART1_Send_Byte((uint8)Char);

  if (Char == (sint32)'\n')
  {
    /* line feed: */
    WDT1_SOW_Service(1);

    while (UART1_isByteTransmitted() == false)
    {
      /* Execute wait function until character is transmitted */
    }

    (void)WDT1_Service();
    /* Clear TI bit and Send additional carriage return */
    UART1_Send_Byte((uint8)'\r');
  }

  while (UART1_isByteTransmitted() == false)
  {
    /* Execute wait function until character is transmitted */
    (void)WDT1_Service();
  }

  return Char;
} /* End of stdout_putchar */


/** \brief Receives a character via UART1.
 *
 * \return Received character
 *
 * \ingroup uart_api
 */
sint32 stdin_getchar(void)
{
  while (UART1_isByteReceived() == false)
  {
    /* Execute wait function until character is received */
    (void)WDT1_Service();
  }

  /* Clear RI bit and get character */
  return UART1_Get_Byte();
} /* End of stdin_getchar */

void ttywrch(int ch)
{
  (void)stdout_putchar(ch);
}
#endif /* UART1_STD_EN == 1 */

#if ((UART1_STD_EN == 0) && (UART2_STD_EN == 1))
/** \brief Sends a character via UART2.
 *
 * \param Char Character to send
 * \return Sent character
 *
 * \ingroup uart_api
 */
sint32 stdout_putchar(sint32 Char)
{
  UART2_Send_Byte((uint8)Char);

  if (Char == (sint32)'\n')
  {
    /* line feed: */
    WDT1_SOW_Service(1);

    while (UART2_isByteTransmitted() == false)
    {
      /* Execute wait function until character is transmitted */
    }

    (void)WDT1_Service();
    /* Clear TI bit and Send additional carriage return */
    UART2_Send_Byte((uint8)'\r');
  }

  while (UART2_isByteTransmitted() == false)
  {
    /* Execute wait function until character is transmitted */
    (void)WDT1_Service();
  }

  return Char;
} /* End of stdout_putchar */


/** \brief Receives a character via UART2.
 *
 * \return Received character
 *
 * \ingroup uart_api
 */
sint32 stdin_getchar(void)
{
  while (UART2_isByteReceived() == false)
  {
    /* Execute wait function until character is received */
    (void)WDT1_Service();
  }

  /* Clear RI bit and get character */
  return UART2_Get_Byte();
} /* End of stdin_getchar */

void ttywrch(int ch)
{
  (void)stdout_putchar(ch);
}
#endif /* UART2_STD_EN == 1 */

#if ((UART1_STD_EN == 1) && (UART2_STD_EN == 1))
  #error only one UART can be selected as STDIN/STDOUT device at a time
#endif
