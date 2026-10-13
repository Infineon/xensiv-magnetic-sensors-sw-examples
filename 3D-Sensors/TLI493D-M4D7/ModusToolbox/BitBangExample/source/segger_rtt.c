/******************************************************************************
* File Name:   segger_rtt.c
*
* Description: Minimal RTT implementation (see segger_rtt.h).
*
*              Layout of the SEGGER RTT control block (search-able by the
*              J-Link host tools by scanning target RAM for the ASCII tag):
*
*                 char  acID[16]     = "SEGGER RTT" (padded with NUL)
*                 int   MaxNumUp     = 1
*                 int   MaxNumDown   = 1
*                 BUF   aUp[1]       (sName, pBuffer, size, wrOff, rdOff, flags)
*                 BUF   aDown[1]
*
*              Trick: we keep the tag corrupted during early boot ("_EGGER ...")
*              so the host tool can't latch onto a half-initialized control
*              block. Once everything is set up, rtt_init() patches the first
*              character to 'S'.
*
*****************************************************************************/

#include "segger_rtt.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define RTT_FLAGS_NO_BLOCK_SKIP 0U   /* drop new data when buffer full      */

typedef struct
{
    const char *sName;
    char       *pBuffer;
    unsigned    SizeOfBuffer;
    unsigned    WrOff;       /* updated by target            */
    volatile unsigned RdOff; /* updated by host (J-Link)     */
    unsigned    Flags;
} rtt_buf_t;

typedef struct
{
    char       acID[16];
    int        MaxNumUpBuffers;
    int        MaxNumDownBuffers;
    rtt_buf_t  aUp[1];
    rtt_buf_t  aDown[1];
} rtt_cb_t;

/* --- Static buffers ------------------------------------------------------ */
static char up_buf  [RTT_UP_BUFFER_SIZE];
static char down_buf[RTT_DOWN_BUFFER_SIZE];

/* The control block. acID is intentionally mis-spelled until rtt_init().    */
static rtt_cb_t _SEGGER_RTT __attribute__((used)) =
{
    /* Note the leading '_' instead of 'S' — patched in rtt_init().          */
    .acID = { '_','E','G','G','E','R',' ','R','T','T',0,0,0,0,0,0 },
    .MaxNumUpBuffers   = 1,
    .MaxNumDownBuffers = 1,
    .aUp   = { { "Terminal", up_buf,   RTT_UP_BUFFER_SIZE,   0U, 0U, RTT_FLAGS_NO_BLOCK_SKIP } },
    .aDown = { { "Terminal", down_buf, RTT_DOWN_BUFFER_SIZE, 0U, 0U, RTT_FLAGS_NO_BLOCK_SKIP } }
};

/*--------------------------------------------------------------------------*/
void rtt_init(void)
{
    /* Patch the tag last, so the host tools only find a fully-initialized
     * control block in RAM. */
    _SEGGER_RTT.acID[0] = 'S';
}

/*--------------------------------------------------------------------------*/
size_t rtt_write(const void *data, size_t len)
{
    if (data == NULL || len == 0U) { return 0U; }

    rtt_buf_t      *b   = &_SEGGER_RTT.aUp[0];
    const uint8_t  *src = (const uint8_t *)data;
    const unsigned  sz  = b->SizeOfBuffer;
    unsigned        wr  = b->WrOff;
    unsigned        rd  = b->RdOff;
    size_t          written = 0U;

    /* Compute free space (one slot kept empty to disambiguate full/empty). */
    unsigned free_space = (rd > wr)
                          ? (rd - wr - 1U)
                          : (sz - wr + rd - 1U);

    if (len > free_space) { len = free_space; }

    while (len--)
    {
        b->pBuffer[wr] = (char)*src++;
        wr = (wr + 1U) % sz;
        ++written;
    }

    /* Publish updated write offset (single store on M0, naturally atomic). */
    b->WrOff = wr;
    return written;
}

/*--------------------------------------------------------------------------*/
size_t rtt_write_str(const char *s)
{
    return s ? rtt_write(s, strlen(s)) : 0U;
}

/*--------------------------------------------------------------------------*/
int rtt_printf(const char *fmt, ...)
{
    char    line[128];
    va_list ap;

    va_start(ap, fmt);
    int n = vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);

    if (n < 0) { return n; }
    size_t to_write = ((size_t)n < sizeof(line)) ? (size_t)n : sizeof(line) - 1U;
    rtt_write(line, to_write);
    return n;
}
