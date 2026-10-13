/******************************************************************************
* File Name:   bitbang.c
*
* Description: Implementation of the software UART (bit-banging) library.
*              See bitbang.h for the public API.
*
* Timing:      Uses a calibrated busy-wait loop (Cortex-M0 core). The cost
*              of one iteration, BITBANG_CYCLES_PER_LOOP_X10 below (CPU
*              cycles x 10), was measured empirically and depends on the
*              optimization level; tweak it if you measure significant skew
*              with a logic analyzer.
*
*****************************************************************************/

#include "bitbang.h"
#include "xmc_scu.h"
#include "cmsis_compiler.h"

/* SystemCoreClock is the standard CMSIS global holding the current
 * CPU clock frequency in Hz. It is defined and kept up-to-date by the
 * XMC startup / system_XMC1400.c code.
 */
extern uint32_t SystemCoreClock;

/* Approximate CPU cycles consumed by one iteration of delay_loop(),
 * multiplied by 10 (so we can express e.g. 9.4 cycles/iter as 94).
 * Calibrated empirically on XMC1404 @ 48 MHz with this exact toolchain
 * and -Og: each iteration consumes ~9.4 CPU cycles.
 *
 * To re-tune: scope the start-bit width on TX. If measured = M us and
 * target at this baud rate is T us, set:
 *     new_x10 = old_x10 * M / T
 */
#ifndef BITBANG_CYCLES_PER_LOOP_X10
#define BITBANG_CYCLES_PER_LOOP_X10   94U
#endif

/*----------------------------------------------------------------------------*/
/* Tight delay loop. Marked noinline so the compiler can't unroll it away.    */
/*----------------------------------------------------------------------------*/
static void __attribute__((noinline)) delay_loop(uint32_t iters)
{
    while (iters--)
    {
        __NOP();
    }
}

/*----------------------------------------------------------------------------*/
void bitbang_init(bitbang_t *ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    /* --- TX pin: idle high (push-pull output) --- */
    if (ctx->tx_port != NULL)
    {
        XMC_GPIO_CONFIG_t tx_cfg =
        {
            .mode         = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
            .output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH
        };
        XMC_GPIO_Init(ctx->tx_port, ctx->tx_pin, &tx_cfg);
        XMC_GPIO_SetOutputHigh(ctx->tx_port, ctx->tx_pin);
    }

    /* --- RX pin: input with pull-up so an unconnected line idles high --- */
    if (ctx->rx_port != NULL)
    {
        XMC_GPIO_CONFIG_t rx_cfg =
        {
            .mode         = XMC_GPIO_MODE_INPUT_PULL_UP
        };
        XMC_GPIO_Init(ctx->rx_port, ctx->rx_pin, &rx_cfg);
    }

    /* --- Compute loop iterations per bit time --- */
    uint32_t cpu_hz = SystemCoreClock;
    if (cpu_hz == 0U || ctx->baudrate == 0U)
    {
        /* Fallback: assume 48 MHz, 9600 baud */
        cpu_hz = 48000000U;
        if (ctx->baudrate == 0U)
        {
            ctx->baudrate = 9600U;
        }
    }

    uint32_t cycles_per_bit = cpu_hz / ctx->baudrate;
    ctx->_loop_iters_per_bit      = (cycles_per_bit * 10U) / BITBANG_CYCLES_PER_LOOP_X10;
    ctx->_loop_iters_per_half_bit = ctx->_loop_iters_per_bit / 2U;
}

/*----------------------------------------------------------------------------*/
void bitbang_send_byte(const bitbang_t *ctx, uint8_t b)
{
    if (ctx == NULL || ctx->tx_port == NULL)
    {
        return;
    }

    XMC_GPIO_PORT_t *p = ctx->tx_port;
    const uint8_t    n = ctx->tx_pin;
    const uint32_t   t = ctx->_loop_iters_per_bit;

    /* Start bit: drive low for one bit time */
    XMC_GPIO_SetOutputLow(p, n);
    delay_loop(t);

    /* 8 data bits, LSB first */
    for (uint8_t i = 0U; i < 8U; ++i)
    {
        if (b & 0x01U)
        {
            XMC_GPIO_SetOutputHigh(p, n);
        }
        else
        {
            XMC_GPIO_SetOutputLow(p, n);
        }
        b >>= 1;
        delay_loop(t);
    }

    /* Stop bit: drive high for one bit time (idle) */
    XMC_GPIO_SetOutputHigh(p, n);
    delay_loop(t);
}

/*----------------------------------------------------------------------------*/
void bitbang_send(const bitbang_t *ctx, const uint8_t *data, size_t len)
{
    if (data == NULL)
    {
        return;
    }
    for (size_t i = 0U; i < len; ++i)
    {
        bitbang_send_byte(ctx, data[i]);
    }
}

/*----------------------------------------------------------------------------*/
int bitbang_recv_byte(const bitbang_t *ctx, uint8_t *out, uint32_t timeout_ms)
{
    if (ctx == NULL || ctx->rx_port == NULL || out == NULL)
    {
        return -1;
    }

    XMC_GPIO_PORT_t *p = ctx->rx_port;
    const uint8_t    n = ctx->rx_pin;
    const uint32_t   t      = ctx->_loop_iters_per_bit;
    const uint32_t   t_half = ctx->_loop_iters_per_half_bit;

    /* --- Wait for falling edge (start bit) with optional timeout ---
     *
     * Two-phase wait: first ensure the line is HIGH (idle), then wait for
     * it to go LOW. Without this, when bytes arrive back-to-back the line
     * may already be in the start bit when we re-enter this function. We
     * would then time the data samples from "now" instead of from the real
     * falling edge and read garbage (typically 0xFF or 0x00 patterns).
     */
    uint32_t budget = 0U;
    if (timeout_ms > 0U)
    {
        uint32_t cpu_hz = SystemCoreClock;
        if (cpu_hz == 0U) cpu_hz = 48000000U;
        budget = (cpu_hz / 1000U) * timeout_ms / (BITBANG_CYCLES_PER_LOOP_X10 / 5U);
    }

    /* Phase 1: wait for line to be high (idle). Use a small fraction of
     * the total timeout so a stuck-low line doesn't eat the whole budget. */
    uint32_t idle_budget = (timeout_ms > 0U) ? (budget / 4U + 1U) : 0U;
    while (XMC_GPIO_GetInput(p, n) == 0U)
    {
        if (timeout_ms > 0U)
        {
            if (idle_budget == 0U) { return 0; }
            --idle_budget;
        }
    }

    /* Phase 2: wait for the falling edge that starts the next byte. */
    while (XMC_GPIO_GetInput(p, n) != 0U)
    {
        if (timeout_ms > 0U)
        {
            if (budget == 0U)
            {
                return 0; /* timeout */
            }
            --budget;
        }
    }

    /* Falling edge detected. Wait half a bit time so subsequent samples
     * land in the middle of each bit cell.
     */
    delay_loop(t_half);

    /* Re-check start bit: must still be low. */
    if (XMC_GPIO_GetInput(p, n) != 0U)
    {
        return -1; /* false start / glitch */
    }

    /* Sample 8 data bits, LSB first. Wait one full bit time before each sample. */
    uint8_t value = 0U;
    for (uint8_t i = 0U; i < 8U; ++i)
    {
        delay_loop(t);
        if (XMC_GPIO_GetInput(p, n) != 0U)
        {
            value |= (uint8_t)(1U << i);
        }
    }

    /* Wait one bit time, then check stop bit (must be high). */
    delay_loop(t);
    if (XMC_GPIO_GetInput(p, n) == 0U)
    {
        return -1; /* framing error */
    }

    *out = value;
    return 1;
}

/*----------------------------------------------------------------------------*/
/* Convert a millisecond timeout into a polling-loop budget that is roughly
 * comparable to the per-bit delay loop iteration count. Same formula as
 * used inside bitbang_recv_byte().
 */
static uint32_t ms_to_poll_budget(uint32_t timeout_ms)
{
    if (timeout_ms == 0U)
    {
        return 0U;
    }
    uint32_t cpu_hz = SystemCoreClock;
    if (cpu_hz == 0U) cpu_hz = 48000000U;
    return (cpu_hz / 1000U) * timeout_ms / (BITBANG_CYCLES_PER_LOOP_X10 / 5U);
}

/*----------------------------------------------------------------------------*/
size_t bitbang_recv_frame(const bitbang_t *ctx,
                          uint8_t *buf, size_t len,
                          uint32_t first_timeout_ms,
                          uint32_t inter_timeout_ms)
{
    if (ctx == NULL || ctx->rx_port == NULL || buf == NULL || len == 0U)
    {
        return 0U;
    }

    XMC_GPIO_PORT_t *p = ctx->rx_port;
    const uint8_t    n = ctx->rx_pin;
    const uint32_t   t      = ctx->_loop_iters_per_bit;
    const uint32_t   t_half = ctx->_loop_iters_per_half_bit;

    /* Pre-compute both budgets ONCE up front. The XMC1404 has no hardware
     * divider; calling ms_to_poll_budget() inside the per-byte loop adds
     * enough cycles to miss the start bit of a back-to-back next byte at
     * 115200 baud (only ~4 us of slack between bytes).
     */
    const uint32_t first_budget_init = ms_to_poll_budget(first_timeout_ms);
    const uint32_t inter_budget_init = ms_to_poll_budget(inter_timeout_ms);

    size_t received = 0U;

    /* --- Byte 0: full "wait for line high, then falling edge" sync ------- */
    {
        uint32_t budget      = first_budget_init;
        uint32_t idle_budget = (first_timeout_ms != 0U) ? (budget / 4U + 1U) : 0U;

        /* Phase 1: line idle high. */
        while (XMC_GPIO_GetInput(p, n) == 0U)
        {
            if (first_timeout_ms != 0U)
            {
                if (idle_budget == 0U) { return 0U; }
                --idle_budget;
            }
        }
        /* Phase 2: falling edge. */
        while (XMC_GPIO_GetInput(p, n) != 0U)
        {
            if (first_timeout_ms != 0U)
            {
                if (budget == 0U) { return 0U; }
                --budget;
            }
        }

        delay_loop(t_half);
        if (XMC_GPIO_GetInput(p, n) != 0U) { return 0U; } /* glitch */

        uint8_t value = 0U;
        for (uint8_t i = 0U; i < 8U; ++i)
        {
            delay_loop(t);
            if (XMC_GPIO_GetInput(p, n) != 0U)
            {
                value |= (uint8_t)(1U << i);
            }
        }
        delay_loop(t);
        if (XMC_GPIO_GetInput(p, n) == 0U) { return 0U; } /* framing */

        buf[received++] = value;
    }

    /* --- Bytes 1..len-1: tight edge-detect for each inter-byte boundary --
     * Don't dead-reckon timing across bytes (loop overhead between the
     * stop-bit sample and the next call site causes drift). Instead, after
     * sampling each stop bit we immediately spin-poll for the next falling
     * edge and re-sync to it. The transmitter is our clock reference.
     *
     * The line state when we arrive here:
     *   - We just sampled the stop bit at MID-cell, so we have ~half a bit
     *     time of stop bit remaining before the next falling edge (for a
     *     truly back-to-back byte).
     *   - The polled "wait for high" / "wait for low" sequence below
     *     handles BOTH back-to-back and small-gap cases without any
     *     fragile timing assumptions.
     */
    for (size_t k = 1U; k < len; ++k)
    {
        uint32_t budget = inter_budget_init;

        /* Phase 1: ensure line is high (stop / idle).
         * If we're already past the next falling edge we'd miss it, but
         * the stop-bit sample we just passed guarantees the line was high
         * a few cycles ago and the next falling edge is still ~half a
         * bit time away — so this normally passes immediately.
         */
        while (XMC_GPIO_GetInput(p, n) == 0U)
        {
            if (inter_timeout_ms != 0U)
            {
                if (budget == 0U) { return received; }
                --budget;
            }
        }

        /* Phase 2: wait for the falling edge of the next start bit. */
        while (XMC_GPIO_GetInput(p, n) != 0U)
        {
            if (inter_timeout_ms != 0U)
            {
                if (budget == 0U) { return received; }
                --budget;
            }
        }

        /* Edge detected: align to mid-start-bit. */
        delay_loop(t_half);
        if (XMC_GPIO_GetInput(p, n) != 0U) { return received; } /* glitch */

        /* Sample 8 data bits, LSB first. */
        uint8_t value = 0U;
        for (uint8_t i = 0U; i < 8U; ++i)
        {
            delay_loop(t);
            if (XMC_GPIO_GetInput(p, n) != 0U)
            {
                value |= (uint8_t)(1U << i);
            }
        }
        /* Stop bit. */
        delay_loop(t);
        if (XMC_GPIO_GetInput(p, n) == 0U) { return received; }

        buf[received++] = value;
    }

    return received;
}
