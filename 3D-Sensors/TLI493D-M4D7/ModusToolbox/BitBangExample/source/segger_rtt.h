/******************************************************************************
* File Name:   segger_rtt.h
*
* Description: Minimal Real-Time Transfer (RTT) implementation, compatible
*              with SEGGER's J-Link host tools (JLinkRTTViewer, JLinkRTTClient,
*              JLinkRTTLogger). The control block lives in RAM and exposes
*              one "up" buffer (target -> host) plus one "down" buffer
*              (host -> target).
*
*              Protocol reference:
*                https://wiki.segger.com/RTT
*
*              This implementation is a clean-room rewrite of the well-known
*              control-block layout. Released under the same Apache-2.0 license
*              as the rest of the project.
*
*****************************************************************************/

#ifndef SEGGER_RTT_H
#define SEGGER_RTT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RTT_UP_BUFFER_SIZE
#define RTT_UP_BUFFER_SIZE   1024U   /* target -> host                      */
#endif

#ifndef RTT_DOWN_BUFFER_SIZE
#define RTT_DOWN_BUFFER_SIZE  16U    /* host -> target (rarely used here)   */
#endif

/** Initialize the RTT control block. Call once at startup. */
void rtt_init(void);

/** Write up to len bytes; non-blocking, drops bytes if buffer is full.
 *  Returns the number of bytes accepted into the buffer. */
size_t rtt_write(const void *data, size_t len);

/** Convenience: write a NUL-terminated string. */
size_t rtt_write_str(const char *s);

/** printf-style formatted write. Limited to a 128-byte line per call. */
int rtt_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif

#endif /* SEGGER_RTT_H */
