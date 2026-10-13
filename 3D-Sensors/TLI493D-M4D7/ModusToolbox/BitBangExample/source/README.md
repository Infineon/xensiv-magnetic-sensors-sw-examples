# BitBang — Software UART Library for XMC1400

A small, reusable C library that implements a software (bit-banged)
asynchronous serial port for Infineon XMC1xxx microcontrollers, plus an
application for the **XMC1400 2Go** board that programs and reads out a
daisy chain of sensors and prints the result over SEGGER RTT.

## What's in here

| File                      | Purpose                                                        |
| ------------------------- | -------------------------------------------------------------- |
| `bitbang.h` / `bitbang.c` | Bit-banged UART library (drop-in for any XMC project).         |
| `segger_rtt.h` / `.c`     | Minimal SEGGER RTT driver (`rtt_init`, `rtt_write_str`, `rtt_printf`). |
| `main.c`                  | Sensor start-up programming and read-out loop.                 |
| `Makefile`                | ModusToolbox build file (target: `APP_KIT_XMC14_2GO`).         |
| `bsps/`, `libs/`, `deps/` | ModusToolbox BSP & dependency wiring.                          |

## Application behaviour

1. **Start-up programming (once):**
   1. wait 2 ms
   2. send `0x00`
   3. wait 1 ms
   4. send `0x4F`
   5. wait 2 ms
2. **Read-out loop (repeats forever):**
   1. send `0x48` (read-out-all command, `SENSOR_CMD[]` in `main.c`)
   2. receive `NUM_SENSORS x BYTES_PER_SENSOR` = 4 x 5 = 20 bytes
      (first byte within `RX_TIMEOUT_MS` = 5 ms, then at most
      `RX_INTER_BYTE_MS` = 2 ms between bytes)
   3. print the bytes to RTT, one line per sensor frame (`F1`..`F4`)
   4. pause roughly `TX_INTERVAL_LOOPS` busy-wait iterations

Serial settings: **115200 baud, 8-N-1** (`BB_BAUDRATE`).

## Wiring (XMC1400 2Go)

| Function             | MCU pin          | Note                                   |
| -------------------- | ---------------- | -------------------------------------- |
| Bit-bang TX          | **P2.11**        | -> sensor 1 RX                         |
| Bit-bang RX          | **P2.10**        | <- sensor 4 TX                         |
| OK / heartbeat LED   | P0.7 (USER_LED1) | Toggles every loop iteration           |
| Fail LED             | P0.6 (USER_LED2) | On if the last response was incomplete or timed out |

The sensors form a daisy chain: sensor N TX -> sensor N+1 RX.
**GND must be shared** with all sensors. Supply the sensors with 3.3 V (or
their own supply with common GND).

## RTT output

Open `JLinkRTTViewer` (or `JLinkRTTClient`) while the board is connected via
J-Link. Example output:

```
=== BitBang Sensor Probe ===
Baudrate: 115200, 4 sensors x 5 bytes = 20 response bytes
Programming sensor...

[#1] TX: 48
       F1: xx xx xx xx xx
       F2: xx xx xx xx xx
       F3: xx xx xx xx xx
       F4: xx xx xx xx xx
```

If no bytes arrive, `RX: (no bytes / timeout)` is printed; a partial
response is followed by `incomplete: n/20 bytes`.

## Using the library in your own project

```c
#include "bitbang.h"

bitbang_t bb = {
    .tx_port  = XMC_GPIO_PORT2,
    .tx_pin   = 11U,
    .rx_port  = XMC_GPIO_PORT2,
    .rx_pin   = 10U,
    .baudrate = 115200U,
};

bitbang_init(&bb);
bitbang_send(&bb, (const uint8_t *)"Hello\r\n", 7);

uint8_t b;
if (bitbang_recv_byte(&bb, &b, 100U /*ms*/) == 1) {
    /* got byte */
}

/* Back-to-back frame (no bytes lost between calls) */
uint8_t buf[20];
size_t n = bitbang_recv_frame(&bb, buf, sizeof(buf),
                              5U /*first byte, ms*/, 2U /*inter-byte, ms*/);
```

The library has no global state — you can have multiple independent
`bitbang_t` instances on different pin pairs, as long as you don't try
to use them concurrently from interrupts (the implementation is
blocking and not re-entrant).

### Frame format

- 1 start bit (low)
- 8 data bits, **LSB first**
- 1 stop bit (high)
- No parity, no flow control (8-N-1)

## Build / Flash

Requires ModusToolbox (`make`) and the SEGGER J-Link software. Run from this
directory:

```sh
make build     # build only
make program   # build (if needed) and flash via J-Link
```

On Windows make sure the ModusToolbox `modus-shell\bin` directory and the
SEGGER J-Link directory are on `PATH`, with J-Link **before** any JDK `bin`
directory (the JDK also ships a `jlink.exe`).

> The project path must not contain spaces (ModusToolbox `mtbquery` cannot
> handle them). The project lives two directory levels above `mtb_shared`,
> so the `Makefile` sets `CY_GETLIBS_SHARED_PATH=../../` and `libs/mtb.mk`
> uses `../../mtb_shared/...` search paths. If `make getlibs` fails to clone
> `TARGET_APP_KIT_XMC14_2GO` from GitHub, ignore that — the BSP is already
> provided locally under `bsps/`. After `make getlibs`, check that the
> versions in `libs/mtb.mk` match the folders in `mtb_shared`.

## Tuning bit-bang timing

The delay loop in `bitbang.c` is calibrated by the macro
`BITBANG_CYCLES_PER_LOOP_X10` (CPU cycles per loop iteration x 10, default
`94`). If you scope the TX pin and see that the bit time is too short or too
long, adjust this value (or override it via `DEFINES=BITBANG_CYCLES_PER_LOOP_X10=...`
in the `Makefile`).

For best accuracy at higher baudrates (>= 57600), measure with a logic
analyzer and tune the macro per optimization level.

The start-up delays in `main.c` use a polled SysTick timer and are
independent of this calibration.
