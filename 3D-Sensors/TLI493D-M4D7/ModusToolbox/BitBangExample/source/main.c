/******************************************************************************
* File Name:   main.c
*
* Description: At start-up programs the sensors once (PROG_CMD_1, delay,
*              PROG_CMD_2, delay). Then, in a loop, sends SENSOR_CMD (0x48,
*              read-out all) on the bit-bang TX (P2.11) into a daisy chain of
*              NUM_SENSORS sensors and reads back
*              NUM_SENSORS x BYTES_PER_SENSOR bytes on the bit-bang RX (P2.10).
*              The response is printed to the SEGGER RTT terminal so it can be
*              viewed on the PC via JLinkRTTViewer / JLinkRTTClient.
*
*              Edit SENSOR_CMD[] below to change what is transmitted.
*              Each loop iteration is followed by a busy-wait of
*              TX_INTERVAL_LOOPS iterations.
*
* LEDs:        P0.7 (USER_LED1) -- heartbeat, toggles every loop iteration
*              P0.6 (USER_LED2) -- ON if the last response timed out / errored
*
* Wiring:      MCU P2.11 -> sensor 1 RX
*              sensor N TX -> sensor N+1 RX   (daisy chain)
*              MCU P2.10 <- sensor 4 TX
*              GND       <- shared with all sensors (REQUIRED)
*              Sensor VCC at 3.3 V (or own supply with shared GND)
*
* Hardware:    Kit XMC14 2Go (XMC1404, VQFN40)
*
*****************************************************************************/

#include "cybsp.h"
#include "cy_utils.h"
#include "xmc_gpio.h"
#include "xmc_scu.h"

#include "bitbang.h"
#include "segger_rtt.h"

#include <stdint.h>
#include <stddef.h>

/*---------------------------------------------------------------------------*/
/* Pin definitions                                                           */
/*---------------------------------------------------------------------------*/

#define BB_TX_PORT      XMC_GPIO_PORT2
#define BB_TX_PIN       11U

#define BB_RX_PORT      XMC_GPIO_PORT2
#define BB_RX_PIN       10U

#define LED_OK_PORT     XMC_GPIO_PORT0
#define LED_OK_PIN      7U      /* USER_LED1 */
#define LED_FAIL_PORT   XMC_GPIO_PORT0
#define LED_FAIL_PIN    6U      /* USER_LED2 */

/*---------------------------------------------------------------------------*/
/* Application parameters                                                    */
/*---------------------------------------------------------------------------*/

#define BB_BAUDRATE         115200U
#define RX_TIMEOUT_MS       5U       /* wait for FIRST response byte       */
#define RX_INTER_BYTE_MS    2U       /* gap allowed between response bytes */
#define NUM_SENSORS         4U       /* sensors in the daisy chain         */
#define BYTES_PER_SENSOR    5U       /* response bytes per sensor          */
#define RESPONSE_LEN        (NUM_SENSORS * BYTES_PER_SENSOR)
#define TX_INTERVAL_LOOPS   1500000U /* crude inter-iteration delay        */

/* One-time sensor programming at start-up */
#define PROG_CMD_1          0x00U
#define PROG_DELAY_1_MS     1U
#define PROG_CMD_2          0x4FU
#define PROG_DELAY_2_MS     2U

/* >>> EDIT THIS to change what the MCU sends to the sensor <<<            */
static const uint8_t SENSOR_CMD[] = { 0x48U };   /* readout all command = 0x48    */

/*---------------------------------------------------------------------------*/
/* LEDs - LED1 toggles every loop iteration, LED2 ON if the last response    */
/* was incomplete or timed out                                               */
/*---------------------------------------------------------------------------*/

static void leds_init(void)
{
    XMC_GPIO_CONFIG_t led_cfg =
    {
        .mode         = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
        .output_level = XMC_GPIO_OUTPUT_LEVEL_LOW
    };
    XMC_GPIO_Init(LED_OK_PORT,   LED_OK_PIN,   &led_cfg);
    XMC_GPIO_Init(LED_FAIL_PORT, LED_FAIL_PIN, &led_cfg);
}

static void busy_delay(volatile uint32_t n)
{
    while (n--) { __NOP(); }
}

/* Polled SysTick (no interrupt): accurate millisecond delay. */
static void delay_ms(uint32_t ms)
{
    SysTick->LOAD = (SystemCoreClock / 1000U) - 1U;
    SysTick->VAL  = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (ms--)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U) { }
    }

    SysTick->CTRL = 0U;
}

/*---------------------------------------------------------------------------*/
/* main                                                                      */
/*---------------------------------------------------------------------------*/

int main(void)
{
    cy_rslt_t result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    leds_init();
    rtt_init();

    rtt_write_str("\r\n=== BitBang Sensor Probe ===\r\n");
    rtt_printf("Baudrate: %u, %u sensors x %u bytes = %u response bytes\r\n",
               (unsigned)BB_BAUDRATE, (unsigned)NUM_SENSORS,
               (unsigned)BYTES_PER_SENSOR, (unsigned)RESPONSE_LEN);

    bitbang_t bb =
    {
        .tx_port  = BB_TX_PORT,
        .tx_pin   = BB_TX_PIN,
        .rx_port  = BB_RX_PORT,
        .rx_pin   = BB_RX_PIN,
        .baudrate = BB_BAUDRATE
    };
    bitbang_init(&bb);

    /* --- Start-up: program the sensor once ------------------------------ */
    rtt_write_str("Programming sensor...\r\n");
    delay_ms(PROG_DELAY_2_MS);
    bitbang_send_byte(&bb, PROG_CMD_1);
    delay_ms(PROG_DELAY_1_MS);
    bitbang_send_byte(&bb, PROG_CMD_2);
    delay_ms(PROG_DELAY_2_MS);

    uint32_t iter = 0U;

    for (;;)
    {
        ++iter;
        XMC_GPIO_ToggleOutput(LED_OK_PORT, LED_OK_PIN);

        /* --- Send command to the sensor ---------------------------------- */
        rtt_printf("\r\n[#%lu] TX: ", (unsigned long)iter);
        for (size_t i = 0U; i < sizeof(SENSOR_CMD); ++i)
        {
            rtt_printf("%02X ", SENSOR_CMD[i]);
        }
        rtt_write_str("\r\n");

        bitbang_send(&bb, SENSOR_CMD, sizeof(SENSOR_CMD));

        /* --- Read RESPONSE_LEN bytes back --------------------------------
         * Use the single-call frame receiver so that consecutive bytes
         * arriving back-to-back at 115200 baud are not lost between
         * bitbang_recv_byte() invocations.
         */
        uint8_t  rx[RESPONSE_LEN];
        uint8_t  rx_count = (uint8_t)bitbang_recv_frame(&bb, rx, RESPONSE_LEN,
                                                       RX_TIMEOUT_MS,
                                                       RX_INTER_BYTE_MS);
        int      err      = (rx_count != RESPONSE_LEN) ? 1 : 0;

        /* --- Print whatever we got, one line per sensor frame ------------ */
        if (rx_count == 0U)
        {
            rtt_write_str("       RX: (no bytes / timeout)\r\n");
        }
        else
        {
            /* F1..F4 = frames in arrival order on P2.10 */
            for (uint8_t i = 0U; i < rx_count; ++i)
            {
                if ((i % BYTES_PER_SENSOR) == 0U)
                {
                    rtt_printf("       F%u: ", (unsigned)(i / BYTES_PER_SENSOR + 1U));
                }
                rtt_printf("%02X ", rx[i]);
                if ((((i + 1U) % BYTES_PER_SENSOR) == 0U) || ((i + 1U) == rx_count))
                {
                    rtt_write_str("\r\n");
                }
            }
            if (rx_count != RESPONSE_LEN)
            {
                rtt_printf("       incomplete: %u/%u bytes\r\n",
                           (unsigned)rx_count, (unsigned)RESPONSE_LEN);
            }
        }

        /* --- Status LED -------------------------------------------------- */
        if (err || rx_count != RESPONSE_LEN)
        {
            XMC_GPIO_SetOutputHigh(LED_FAIL_PORT, LED_FAIL_PIN);
        }
        else
        {
            XMC_GPIO_SetOutputLow(LED_FAIL_PORT, LED_FAIL_PIN);
        }

        busy_delay(TX_INTERVAL_LOOPS);
    }
}
