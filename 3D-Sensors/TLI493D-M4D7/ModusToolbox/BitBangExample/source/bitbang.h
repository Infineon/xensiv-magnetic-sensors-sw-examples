/******************************************************************************
* File Name:   bitbang.h
*
* Description: Software (bit-banged) UART-style serial library for XMC1xxx.
*              One TX pin, one RX pin, asynchronous 8-N-1 framing
*              (start bit low, 8 data bits LSB-first, stop bit high).
*
*              Designed to be portable across XMC projects: pass a context
*              struct describing the pins and baudrate, and call the send /
*              receive functions.
*
* Hardware:    Tested on Kit XMC14 2Go (XMC1404, VQFN40)
*              CPU clock assumed = SystemCoreClock from CMSIS
*
* Limitations: - Blocking (busy-wait) implementation, no IRQ.
*              - Practical max baudrate ~115200 at 48 MHz CPU clock.
*              - No parity, no flow control.
*
*****************************************************************************/

#ifndef BITBANG_H
#define BITBANG_H

#include <stdint.h>
#include <stddef.h>
#include "xmc_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Bit-bang serial port context.
 * Fill all fields, then call bitbang_init().
 */
typedef struct
{
    XMC_GPIO_PORT_t *tx_port;   /**< TX port, or NULL to disable TX        */
    uint8_t          tx_pin;    /**< TX pin number (0..15)                 */
    XMC_GPIO_PORT_t *rx_port;   /**< RX port, or NULL to disable RX        */
    uint8_t          rx_pin;    /**< RX pin number (0..15)                 */
    uint32_t         baudrate;  /**< Baudrate in bits/sec (e.g. 115200)    */

    /* --- internal, computed by bitbang_init() --- */
    uint32_t _loop_iters_per_bit;       /**< full bit time in delay loops  */
    uint32_t _loop_iters_per_half_bit;  /**< half bit time in delay loops  */
} bitbang_t;

/**
 * Configure GPIO pins and pre-compute timing for the given context.
 * Must be called once before any send/recv.
 */
void bitbang_init(bitbang_t *ctx);

/** Transmit a single byte (8-N-1). Blocking. */
void bitbang_send_byte(const bitbang_t *ctx, uint8_t b);

/** Transmit a buffer. Blocking. */
void bitbang_send(const bitbang_t *ctx, const uint8_t *data, size_t len);

/**
 * Wait for a single byte on RX (falling edge = start bit) and return it.
 *
 * @param ctx         Initialized bit-bang context.
 * @param out         Where to store the received byte on success.
 * @param timeout_ms  Max time to wait for the start bit. 0 = no timeout.
 * @return            1 on success, 0 on timeout, -1 on framing error
 *                    (start bit not low at sample point, or stop bit not high).
 */
int bitbang_recv_byte(const bitbang_t *ctx, uint8_t *out, uint32_t timeout_ms);

/**
 * Receive an exact-length back-to-back frame on RX.
 *
 * Tight single-pass loop: after each byte's stop bit it immediately polls
 * for the next start bit, so consecutive bytes that arrive with little or
 * no gap (typical of a sensor reply at 115200 baud) are not lost between
 * function calls.
 *
 * @param ctx                Initialized bit-bang context.
 * @param buf                Destination buffer, must hold @p len bytes.
 * @param len                Number of bytes to receive.
 * @param first_timeout_ms   Max time to wait for the FIRST start bit
 *                           (i.e. sensor response latency). 0 = no timeout.
 * @param inter_timeout_ms   Max time to wait between two consecutive bytes
 *                           inside the frame. Should be a few bit times,
 *                           e.g. 2..5 ms. 0 = no timeout.
 * @return                   Number of bytes actually received (0..len).
 *                           Stops on timeout or framing error.
 */
size_t bitbang_recv_frame(const bitbang_t *ctx,
                          uint8_t *buf, size_t len,
                          uint32_t first_timeout_ms,
                          uint32_t inter_timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* BITBANG_H */
