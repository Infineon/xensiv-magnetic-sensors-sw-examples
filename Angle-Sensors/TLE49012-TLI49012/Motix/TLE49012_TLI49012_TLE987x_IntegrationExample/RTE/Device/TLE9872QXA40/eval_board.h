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
 * \file     eval_board.h
 *
 * \brief    LED assignment
 *
 * \version  V0.1.3
 * \date     02. Jan 2025
 *
 */

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
** V0.1.0: 2020-04-15, BG:   Initial version of revision history              **
** V0.1.1: 2020-07-21, BG:   EP-439: Formatted .h/.c files                    **
** V0.1.2: 2022-02-28, JO:   EP-936: Updated copyright and branding           **
** V0.1.3: 2025-01-02, JO:   EP-1493: Updated license                         **
*******************************************************************************/

#ifndef EVAL_BOARD_H
#define EVAL_BOARD_H

/*******************************************************************************
**                           Global Type Definitions                          **
*******************************************************************************/
/* GPIO LED assignment for TLE987x evaluation board */
#define LED1 (0x01)
#define LED2 (0x02)
#define LED3 (0x03)
#define LED4 (0x12)
#define LED5 (0x10)
#define LED6 (0x13)
#define LED7 (0x14)
#define LED8 (0x04)

#endif
