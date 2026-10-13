/*******************************************************************************
 * File Name: cycfg_pins.h
 *
 * Description:
 * Pin configuration
 * This file was automatically generated and should not be modified.
 * Configurator Backend 3.90.0
 * device-db 4.33.0.9315
 * mtb-xmclib-cat3 4.6.0.5280
 *
 *******************************************************************************
 * Copyright 2026, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#if !defined(CYCFG_PINS_H)
#define CYCFG_PINS_H

#include "cycfg_notices.h"
#include "xmc_gpio.h"

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#define CYBSP_XTAL1_PORT XMC_GPIO_PORT0
#define CYBSP_XTAL1_PORT_NUM 0U
#define CYBSP_XTAL1_PIN 10U
#define CYBSP_XTAL2_PORT XMC_GPIO_PORT0
#define CYBSP_XTAL2_PORT_NUM 0U
#define CYBSP_XTAL2_PIN 11U
#define CYBSP_USER_LED2_PORT XMC_GPIO_PORT0
#define CYBSP_USER_LED2_PORT_NUM 0U
#define CYBSP_USER_LED2_PIN 6U
#define CYBSP_USER_LED_PORT XMC_GPIO_PORT0
#define CYBSP_USER_LED1_PORT CYBSP_USER_LED_PORT
#define CYBSP_USER_LED_PORT_NUM 0U
#define CYBSP_USER_LED1_PORT_NUM CYBSP_USER_LED_PORT_NUM
#define CYBSP_USER_LED_PIN 7U
#define CYBSP_USER_LED1_PIN CYBSP_USER_LED_PIN
#define CYBSP_SWDCK_PORT XMC_GPIO_PORT1
#define CYBSP_SWDCK_PORT_NUM 1U
#define CYBSP_SWDCK_PIN 2U
#define CYBSP_SWDIO_PORT XMC_GPIO_PORT1
#define CYBSP_SWDIO_PORT_NUM 1U
#define CYBSP_SWDIO_PIN 3U
#define CYBSP_DEBUG_UART_RX_PORT XMC_GPIO_PORT2
#define CYBSP_DEBUG_UART_RX_PORT_NUM 2U
#define CYBSP_DEBUG_UART_RX_PIN 1U
#define CYBSP_DEBUG_UART_TX_PORT XMC_GPIO_PORT2
#define CYBSP_DEBUG_UART_TX_PORT_NUM 2U
#define CYBSP_DEBUG_UART_TX_PIN 2U

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* CYCFG_PINS_H */
