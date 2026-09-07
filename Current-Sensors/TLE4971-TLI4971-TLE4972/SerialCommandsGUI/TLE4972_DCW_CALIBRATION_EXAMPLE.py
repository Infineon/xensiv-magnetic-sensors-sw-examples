#######################################################################################################################
# Copyright (c) 2026, Infineon Technologies AG
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,are permitted provided that the
# following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this list of conditions and the following
# disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
# disclaimer in the documentation and/or other materials provided with the distribution.
#
# Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote
# products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# To improve the quality of the software, users are encouraged to share modifications, enhancements or bug fixes with
# Infineon Technologies AG.
#######################################################################################################################
#
# Change History
# --------------
#
# 2026-08-06:
#     - Initial
#
#######################################################################################################################

"""TLE4972 Double Code Word (DCW) calibration example with a Tkinter UI.

Implements the "Double Code Word Calibration Flow" from README.md, which maps
the procedure of the TLE4972 user manual (Rev. 1.0.1, ch. 6.4, 6.6.2, 6.6.2.1
and 6.6.2.2) onto the serial commands of the programmer firmware.

WARNING: This procedure REWRITES FACTORY CALIBRATION DATA. The coefficients
o_base, g_base, g_tc_tl, g_tc_tq and g_tc_tt are determined per device on the
production line over the full temperature range; a DCW calibration replaces
them with values derived from a single room-temperature measurement.
The original EEPROM image is archived in Phase 1 and can be written back with
the "Restore backup" button. The EEPROM CRC is never recalculated by the
firmware or the sensor - an image with a wrong CRC drives the OCD outputs to
GND permanently after the next start-up.


REQUIRED USER INPUTS
--------------------
* Target sensitivity [mV/A]   - the sensitivity the calibration aims at.
* Test current ITEST [A]      - measured with a calibrated source, shunt or
                                current probe. Must be >= 10 % of the target
                                full-scale current and low enough to avoid
                                self-heating.
* ADC scale [mV/LSB]          - conversion from the 12-bit readout codes to
                                millivolts. Default 3300/4096; correct it for
                                the analog front-end actually fitted.
* Frames to average           - >= 100 per the user manual (DC measurement).

REQUIREMENTS
------------
* Python 3.10 or newer, pyserial (pip install pyserial), Tkinter.
* A programmer board running the firmware image.
* A current source able to apply ITEST and exactly 0 A on demand. The script
  prompts the operator before every measurement point.

PROCEDURE (button order)
------------------------
    1. Enter firmware mode   bootloader echo (0x09) -> start firmware (0x12)
                             -> firmware version (0x21)
    2. Init MCU              cmdInitMCU (0x22), array configuration 0
    3. Phase 1 - backup      cmdEnterTestMode (0x11), cmdReadEEPROM (0x10),
                             then read Gain_CW (0x20), Offset_CW (0x21) and
                             TINT (0x18) with cmdReadRegister/GetRegisterValue
    4. Phase 2 - gain sweep  Gain_CW = 210 and 570; per point measure VO at
                             0 A and at ITEST -> Sensitivity1, Sensitivity2
                             -> Gain_CW_target by inverse-sensitivity
                             interpolation
    5. Phase 3 - offset sweep Offset_CW = -40 and +40 with Gain_CW_target;
                             measure VO at 0 A -> Offset1, Offset2
                             -> Offset_CW_target by interpolation to null
    6. Phase 4 - coefficients o_base, g_base, g_tc_tl, g_tc_tq, g_tc_tt from
                             the analytic model, cubic fit over -40 / TCAL /
                             100 / 150 degC, then CRC patch
    7. Phase 5 - program     cmdProgramEEPROM (0x26) -> cmdResetSensor (0x13)
                             -> read back -> compare + CRC check
    8. Verify measurement    reset -> mux -> readout, re-measure sensitivity
                             and offset and compare against the targets

MEASUREMENT MODE
----------------
Each sweep point needs a state the normal readout start-up does not produce:
the ISM stays powered down (so the code word is not overwritten) while the
AOUT buffer runs in normal operating mode (so AOUT/VREF are measurable). The
script therefore writes register 0x25 = 0x9000 after setting the code words
and does NOT issue cmdResetSensor before cmdStartReadout - a reset would
discard the code word. Because SICI is disabled until the next chip reset,
every sweep point starts with its own power cycle.

NOTES
-----
* Internal register field widths (user manual ch. 4.1): TINT (0x18) bits 15:0,
  Gain_CW (0x20) bits 10:0 unsigned, Offset_CW (0x21) bits 7:0 sign-magnitude
  (bit 7 = sign, range -127..+127). They match the EEPROM fields they trim:
  g_base is 11 bits and o_base is 8 bits signed. Read-backs are masked to
  these widths and compared with what was written.
* Only words 2, 3, 4, 5 and 8 are modified; all other bits are copied from the
  image read in Phase 1. s_base (word 5) and epk_base (word 8) are preserved.
* A coefficient that overflows its EEPROM field aborts Phase 4 instead of
  being truncated - that indicates a measurement problem.
"""

from __future__ import annotations

import queue
import threading
import time
import tkinter as tk
from datetime import datetime
from pathlib import Path
from tkinter import messagebox, ttk

__version__ = "1.0.0"

try:
    import serial
    from serial.tools import list_ports
except ImportError:  # pragma: no cover - dependency hint for the user
    serial = None
    list_ports = None

# ---------------------------------------------------------------------------
# Protocol constants (README.md - Firmware Mode Protocol)
# ---------------------------------------------------------------------------

BAUDRATE = 1_250_000
START_BYTE = 0xAA

BL_ECHO = 0x09
BL_START_FIRMWARE = 0x12
BL_ACK = 0x20

CMD_READ_EEPROM = 0x10
CMD_ENTER_TEST_MODE = 0x11
CMD_RESET_SENSOR = 0x13
CMD_START_READOUT = 0x14
CMD_STOP_READOUT = 0x15
CMD_READ_REGISTER = 0x17
CMD_GET_REGISTER_VALUE = 0x18
CMD_FW_VERSION = 0x21
CMD_INIT_MCU = 0x22
CMD_PROGRAM_EEPROM = 0x26
CMD_ENABLE_MUX = 0x29
CMD_SET_REGISTER = 0x30

DEFAULT_TIMEOUT_S = 1.0
INIT_MCU_TIMEOUT_S = 30.0
# Without a bootloader answer the device may still be booting, so the first firmware
# query is given a long window before the link is declared dead.
FW_VERSION_TIMEOUT_S = 15.0

# Pacing. The programmer relays every command over a slow interface to the
# sensor, so the host leaves generous idle gaps rather than the bare minimum.
INTER_COMMAND_DELAY_S = 0.05    # idle gap before every command frame
PAYLOAD_GAP_S = 0.02            # gap between a command header and its payload
RESET_SETTLE_S = 0.30           # after cmdResetSensor, before the next command
EEPROM_WRITE_SETTLE_S = 0.50    # after cmdProgramEEPROM, before reading back
REGISTER_SETTLE_S = 0.02        # after cmdSetRegister
# The readout stream and the stop acknowledge race each other, so the host waits for
# the line to fall quiet before it trusts the next response.
STREAM_QUIET_S = 0.05
STREAM_DRAIN_TIMEOUT_S = 2.0
EEPROM_VERIFY_ATTEMPTS = 3
PROGRAM_ATTEMPTS = 3            # burn cycles before giving up (endurance is 100)
BACKUP_DIR = Path(__file__).resolve().parent / "eeprom_backups"

EEPROM_LINE_COUNT = 18
EEPROM_BYTE_COUNT = EEPROM_LINE_COUNT * 2

CRC_POLYNOMIAL = 0x1D
CRC_SEED = 0xAA

# Internal sensor registers (user manual ch. 4.1)
REG_TEMPERATURE = 0x18
REG_GAIN_CW = 0x20
REG_OFFSET_CW = 0x21
REG_SICI_BYPASS = 0x25

SICI_BYPASS_MEASURE = 0x9000  # test_pd_ism = 1, SICI_mode_dis = 1
SICI_ENTRY_ATTEMPTS = 3
REGISTER_WRITE_ATTEMPTS = 3
MEASUREMENT_ENTRY_ATTEMPTS = 3
# SICI liveness is probed by reading TINT and range-checking it. Register 0x25 cannot
# be used for this: it is a write-only test register and does not read back.
SICI_PROBE_MIN_C = -50.0
SICI_PROBE_MAX_C = 160.0

# Implemented field widths (user manual ch. 4.1). Bits above these are not driven,
# so read-backs are masked before comparing.
TINT_MASK = 0xFFFF            # TINT, bits 15:0
GAIN_CW_MASK = 0x07FF         # Gain_CW, bits 10:0, unsigned
OFFSET_CW_SIGN_BIT = 7        # Offset_CW, bits 7:0, sign-magnitude (+-127)
OFFSET_CW_MASK = 0xFF

# Double Code Word sweep points (user manual ch. 6.6.2)
GAIN_CW1 = 210
GAIN_CW2 = 570
OFFSET_CW1 = -40
OFFSET_CW2 = 40

GAIN_NUMERATOR = 15 * 2 ** 13   # 122880, eq. 14/15/21
GAIN_DENOMINATOR = 300
GAIN_CW_OFFSET = 512

FIT_TEMPERATURES_C = (-40.0, None, 100.0, 150.0)  # None = TCAL, filled at runtime

OPMODE_SINGLE_ENDED = 3
OPMODE_NAMES = {0: "SDBID", 1: "FD", 2: "SDUNI", 3: "SE"}
MEASRNG_NAMES = {0x05: "S1", 0x06: "S2", 0x08: "S3", 0x0C: "S4", 0x10: "S5", 0x18: "S6"}

# Bit widths of the calibration fields (README - EEPROM words touched)
FIELD_LIMITS = {
    "g_base": (0, 2047),
    "g_tc_tl": (-2048, 2047),
    "g_tc_tq": (-512, 511),
    "g_tc_tt": (-1024, 1023),
    "o_base": (-128, 127),
}


# ---------------------------------------------------------------------------
# EEPROM CRC (TLE4972 user manual ch. 5.2)
# ---------------------------------------------------------------------------

def crc8(data: bytes) -> int:
    crc = CRC_SEED
    for byte in data:
        crc ^= byte
        for _ in range(8):
            crc = ((crc << 1) ^ CRC_POLYNOMIAL) & 0xFF if crc & 0x80 else (crc << 1) & 0xFF
    return crc ^ 0xFF


def eeprom_crc(words: list[int]) -> int:
    """CRC over line 3..17, then line 0..1, then the high byte of line 2."""
    stream = bytearray()
    for i in range(EEPROM_LINE_COUNT):
        word = words[(i + 3) % EEPROM_LINE_COUNT] & 0xFFFF
        stream.append(word >> 8)
        stream.append(word & 0xFF)
    return crc8(bytes(stream[:-1]))  # low byte of line 2 is the CRC field itself


def eeprom_crc_check(words: list[int]) -> bool:
    return (words[2] & 0xFF) == eeprom_crc(words)


def patch_crc(words: list[int]) -> int:
    crc = eeprom_crc(words)
    words[2] = (words[2] & 0xFF00) | crc
    return crc


def words_to_bytes(words: list[int]) -> bytes:
    out = bytearray()
    for word in words:
        out.append((word >> 8) & 0xFF)
        out.append(word & 0xFF)
    return bytes(out)


def bytes_to_words(data: bytes) -> list[int]:
    return [(data[i] << 8) | data[i + 1] for i in range(0, len(data), 2)]


def hex_dump(data: bytes, per_line: int = 8) -> list[str]:
    return [" ".join(f"{b:02X}" for b in data[i:i + per_line])
            for i in range(0, len(data), per_line)]


def save_backup_file(words: list[int], sensor: int, gain_cw: int,
                     offset_cw: int, t_cal: float) -> Path:
    """Archive the untouched EEPROM image so a device can be recovered later."""
    BACKUP_DIR.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now()
    path = BACKUP_DIR / f"eeprom_sensor{sensor + 1}_{stamp:%Y%m%d_%H%M%S}.txt"
    lines = [
        "# TLE4972 EEPROM backup taken before double code word calibration",
        f"# saved      : {stamp:%Y-%m-%d %H:%M:%S}",
        f"# sensor     : {sensor + 1}",
        f"# CRC        : 0x{words[2] & 0xFF:02X}",
        f"# Gain_CW    : {gain_cw}",
        f"# Offset_CW  : {offset_cw}",
        f"# TCAL       : {t_cal:.2f} C",
        "#",
        "# word  addr  value",
    ]
    lines += [f"  {index:>4}  0x{0x40 + index:02X}  0x{word:04X}"
              for index, word in enumerate(words)]
    lines.append("#")
    lines.append("# raw bytes")
    lines += [f"# {line}" for line in hex_dump(words_to_bytes(words))]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


# ---------------------------------------------------------------------------
# Calibration coefficient access (README - EEPROM words touched)
# ---------------------------------------------------------------------------

def to_signed(value: int, bits: int) -> int:
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


def get_g_base(words: list[int]) -> int:
    return ((words[3] & 0x000F)
            | ((words[4] & 0x003F) << 4)
            | (((words[5] >> 4) & 1) << 10))


def get_g_tc_tl(words: list[int]) -> int:
    return to_signed((words[3] >> 4) & 0x0FFF, 12)


def get_g_tc_tq(words: list[int]) -> int:
    return to_signed((words[4] >> 6) & 0x03FF, 10)


def get_g_tc_tt(words: list[int]) -> int:
    return to_signed((words[5] >> 5) & 0x07FF, 11)


def get_o_base(words: list[int]) -> int:
    return to_signed(words[8] & 0x00FF, 8)


def get_opmode(words: list[int]) -> int:
    return (words[0] >> 5) & 0x03


def check_field(name: str, value: int) -> int:
    low, high = FIELD_LIMITS[name]
    if not low <= value <= high:
        raise ValueError(f"{name} = {value} is outside its EEPROM field range "
                         f"[{low}, {high}] - the measurement is not plausible")
    return value


def pack_coefficients(base: list[int], g_base: int, g_tc_tl: int, g_tc_tq: int,
                      g_tc_tt: int, o_base: int) -> list[int]:
    """Read-modify-write words 3, 4, 5 and 8; s_base and epk_base are kept."""
    words = list(base)
    words[3] = ((g_tc_tl & 0x0FFF) << 4) | (g_base & 0x000F)
    words[4] = ((g_tc_tq & 0x03FF) << 6) | ((g_base >> 4) & 0x003F)
    words[5] = (((g_tc_tt & 0x07FF) << 5)
                | (((g_base >> 10) & 1) << 4)
                | (words[5] & 0x000F))
    words[8] = (words[8] & 0xFF00) | (o_base & 0x00FF)
    return words


def decode_word(index: int, word: int) -> str:
    if index == 0:
        rng = word & 0x1F
        return (f"MEASRNG={rng:#04x} ({MEASRNG_NAMES.get(rng, '?')}), "
                f"OPMODE={(word >> 5) & 3} ({OPMODE_NAMES[(word >> 5) & 3]})")
    if index == 2:
        return f"CRC={word & 0xFF:#04x}"
    if index == 3:
        return f"g_tc_tl={to_signed((word >> 4) & 0xFFF, 12)}, g_base[3:0]={word & 0xF}"
    if index == 4:
        return f"g_tc_tq={to_signed((word >> 6) & 0x3FF, 10)}, g_base[9:4]={word & 0x3F}"
    if index == 5:
        return (f"g_tc_tt={to_signed((word >> 5) & 0x7FF, 11)}, "
                f"g_base[10]={(word >> 4) & 1}, s_base[3:0]={word & 0xF}")
    if index == 8:
        return f"o_base={to_signed(word & 0xFF, 8)}, epk_base={to_signed(word >> 8, 8)}"
    return ""


# ---------------------------------------------------------------------------
# Double Code Word maths (user manual ch. 6.6.2 / 6.6.2.2)
# ---------------------------------------------------------------------------

def temperature_code(celsius: float) -> float:
    return (celsius - 65.0) * 16.0 + 2048.0


def temperature_celsius(code: int) -> float:
    return (code - 2048) / 16.0 + 65.0


def gain_from_cw(code_word: float) -> float:
    return GAIN_NUMERATOR / (GAIN_DENOMINATOR * (code_word + GAIN_CW_OFFSET))


def cw_from_gain(gain: float) -> float:
    return GAIN_NUMERATOR / (GAIN_DENOMINATOR * gain) - GAIN_CW_OFFSET


def interpolate_gain_cw(sens1: float, sens2: float, target: float) -> tuple[float, float]:
    """Linear interpolation on the inverse sensitivity (eq. 11)."""
    denominator = (1.0 / sens2) - (1.0 / sens1)
    if denominator == 0.0:
        raise ValueError("Sensitivity1 and Sensitivity2 are identical - "
                         "the Gain_CW write had no effect (was the ISM reactivated?)")
    ratio = ((1.0 / target) - (1.0 / sens1)) / denominator
    return ratio, GAIN_CW1 + ratio * (GAIN_CW2 - GAIN_CW1)


def interpolate_offset_cw(offset1: float, offset2: float) -> tuple[float, float, float]:
    """Linear interpolation to null offset (eq. 10)."""
    average = (offset1 + offset2) / 2.0
    ratio = (offset2 - offset1) / (OFFSET_CW2 - OFFSET_CW1)
    if ratio == 0.0:
        raise ValueError("Offset1 and Offset2 are identical - "
                         "the Offset_CW write had no effect (was the ISM reactivated?)")
    return average, ratio, -average / ratio


def solve_linear_system(matrix: list[list[float]], rhs: list[float]) -> list[float]:
    """Gaussian elimination with partial pivoting."""
    size = len(rhs)
    augmented = [row[:] + [rhs[i]] for i, row in enumerate(matrix)]
    for column in range(size):
        pivot = max(range(column, size), key=lambda r: abs(augmented[r][column]))
        if abs(augmented[pivot][column]) < 1e-300:
            raise ValueError("singular matrix in the cubic fit")
        augmented[column], augmented[pivot] = augmented[pivot], augmented[column]
        for row in range(column + 1, size):
            factor = augmented[row][column] / augmented[column][column]
            for col in range(column, size + 1):
                augmented[row][col] -= factor * augmented[column][col]
    solution = [0.0] * size
    for row in reversed(range(size)):
        total = augmented[row][size] - sum(augmented[row][c] * solution[c]
                                           for c in range(row + 1, size))
        solution[row] = total / augmented[row][row]
    return solution


def compute_new_coefficients(base: list[int], gain_cw_old: int, offset_cw_old: int,
                             gain_cw_target: int, offset_cw_target: int,
                             t_cal_celsius: float) -> dict:
    """Phase 4 of the README flow (user manual eq. 12 to 24)."""
    g_base_old = get_g_base(base)
    g_tc_tl_old = get_g_tc_tl(base)
    g_tc_tq_old = get_g_tc_tq(base)
    g_tc_tt_old = get_g_tc_tt(base)
    o_base_old = get_o_base(base)

    o_base_new = o_base_old + offset_cw_target - offset_cw_old          # eq. 12

    gain_old_cal = gain_from_cw(gain_cw_old)                            # eq. 14
    gain_new_cal = gain_from_cw(gain_cw_target)                         # eq. 15
    gain_cf = gain_new_cal / gain_old_cal                               # eq. 16

    base_old = float(g_base_old)                                        # eq. 17
    tl_old = g_tc_tl_old / 2 ** 11
    tq_old = g_tc_tq_old / 2 ** 22
    tt_old = g_tc_tt_old / 2 ** 35

    support_c = [t if t is not None else t_cal_celsius for t in FIT_TEMPERATURES_C]
    support_t = [temperature_code(c) for c in support_c]

    targets = []
    for celsius, code in zip(support_c, support_t):
        if celsius == t_cal_celsius:
            targets.append(float(gain_cw_target))  # already known at TCAL
            continue
        cw_old_t = round(base_old + tl_old * code                       # eq. 18
                         + tq_old * code ** 2 + tt_old * code ** 3)
        gain_new_t = gain_from_cw(cw_old_t) * gain_cf                   # eq. 19/20
        targets.append(cw_from_gain(gain_new_t))                        # eq. 21

    matrix = [[1.0, t, t ** 2, t ** 3] for t in support_t]              # eq. 23
    base_new, tl_new, tq_new, tt_new = solve_linear_system(matrix, targets)

    g_base_new = check_field("g_base", round(base_new))                 # eq. 24
    g_tc_tl_new = check_field("g_tc_tl", round(tl_new * 2 ** 11))
    g_tc_tq_new = check_field("g_tc_tq", round(tq_new * 2 ** 22))
    g_tc_tt_new = check_field("g_tc_tt", round(tt_new * 2 ** 35))
    o_base_new = check_field("o_base", o_base_new)

    words = pack_coefficients(base, g_base_new, g_tc_tl_new, g_tc_tq_new,
                              g_tc_tt_new, o_base_new)
    crc = patch_crc(words)

    return {
        "old": {"g_base": g_base_old, "g_tc_tl": g_tc_tl_old, "g_tc_tq": g_tc_tq_old,
                "g_tc_tt": g_tc_tt_old, "o_base": o_base_old},
        "new": {"g_base": g_base_new, "g_tc_tl": g_tc_tl_new, "g_tc_tq": g_tc_tq_new,
                "g_tc_tt": g_tc_tt_new, "o_base": o_base_new},
        "gain_old_cal": gain_old_cal, "gain_new_cal": gain_new_cal, "gain_cf": gain_cf,
        "fit": {"BASE": base_new, "TL": tl_new, "TQ": tq_new, "TT": tt_new},
        "support": list(zip(support_c, support_t, targets)),
        "words": words, "crc": crc,
    }


# ---------------------------------------------------------------------------
# Serial protocol driver
# ---------------------------------------------------------------------------

class ProtocolError(Exception):
    pass


class Programmer:
    """Thin wrapper around the 5-byte command frame protocol."""

    def __init__(self, port: str, log):
        if serial is None:
            raise ProtocolError("pyserial is not installed (pip install pyserial)")
        self._log = log
        self.ser = serial.Serial(port, BAUDRATE, timeout=DEFAULT_TIMEOUT_S, write_timeout=2.0)
        self.ser.reset_input_buffer()

    def close(self) -> None:
        try:
            self.ser.close()
        except Exception:
            pass

    # -- low level ---------------------------------------------------------

    def _write(self, data: bytes, comment: str = "") -> None:
        for line in hex_dump(data):
            self._log(f"TX  {line:<26}{comment}")
            comment = ""
        self.ser.write(data)
        self.ser.flush()

    def _read(self, count: int, comment: str = "", timeout: float | None = None) -> bytes:
        previous = self.ser.timeout
        if timeout is not None:
            self.ser.timeout = timeout
        try:
            data = self.ser.read(count)
        finally:
            self.ser.timeout = previous
        if len(data) != count:
            raise ProtocolError(f"timeout: expected {count} bytes, got {len(data)}")
        for line in hex_dump(data):
            self._log(f"RX  {line:<26}{comment}")
            comment = ""
        return data

    def _drain(self, quiet: float = STREAM_QUIET_S,
               deadline: float = STREAM_DRAIN_TIMEOUT_S) -> int:
        """Discard everything still arriving until the line stays quiet."""
        dropped = 0
        end = time.time() + deadline
        while self.ser.in_waiting and time.time() < end:
            dropped += len(self.ser.read(self.ser.in_waiting))
            time.sleep(quiet)
        return dropped

    # -- bootloader --------------------------------------------------------

    def bootloader_echo(self) -> bool:
        self.ser.reset_input_buffer()
        self._write(bytes([BL_ECHO]), "echo command")
        try:
            return self._read(1, "bootloader acknowledgment")[0] == BL_ACK
        except ProtocolError:
            self._log("--  no bootloader response (device may already run firmware)")
            return False

    def bootloader_start_firmware(self) -> bool:
        self._write(bytes([BL_START_FIRMWARE]), "exit bootloader / start firmware")
        try:
            ok = self._read(1, "acknowledgment (restart follows)")[0] == BL_ACK
        except ProtocolError:
            return False
        time.sleep(0.5)
        self.ser.reset_input_buffer()
        return ok

    # -- firmware ----------------------------------------------------------

    def command(self, cmd: int, sensor: int, payload: bytes = b"",
                n_resp: int = 0, comment: str = "",
                timeout: float | None = None) -> bytes:
        time.sleep(INTER_COMMAND_DELAY_S)
        stale = self._drain()
        if stale:
            self._log(f"--  discarded {stale} stale byte(s) before {comment or 'the command'}")
        header = bytes([START_BYTE, cmd, sensor, len(payload), n_resp])
        self._write(header, comment or f"cmd {cmd:#04x}")
        self._read(2, "acknowledge", timeout)
        if payload:
            time.sleep(PAYLOAD_GAP_S)
            self._write(payload, "payload")
            self._read(2, "final acknowledge", timeout)
        if n_resp:
            return self._read(n_resp, "response payload", timeout)
        return b""

    def fw_version(self, sensor: int, timeout: float | None = None) -> int:
        return self.command(CMD_FW_VERSION, sensor, n_resp=2,
                            comment="cmdFwVersion", timeout=timeout)[1]

    def init_mcu(self, sensor: int, array_config: int = 0) -> None:
        # cmdInitMCU re-initialises all peripherals and only answers when done.
        self.command(CMD_INIT_MCU, sensor, bytes([array_config, 0x00]),
                     comment="cmdInitMCU", timeout=INIT_MCU_TIMEOUT_S)

    def enter_test_mode(self, sensor: int) -> None:
        self.command(CMD_ENTER_TEST_MODE, sensor, comment="cmdEnterTestMode")

    def read_eeprom(self, sensor: int) -> list[int]:
        data = self.command(CMD_READ_EEPROM, sensor, n_resp=EEPROM_BYTE_COUNT,
                            comment="cmdReadEEPROM")
        return bytes_to_words(data)

    def program_eeprom(self, sensor: int, words: list[int]) -> None:
        self.command(CMD_PROGRAM_EEPROM, sensor, words_to_bytes(words),
                     comment="cmdProgramEEPROM")
        time.sleep(EEPROM_WRITE_SETTLE_S)
        self._log(f"--  waited {EEPROM_WRITE_SETTLE_S * 1000:.0f} ms for the EEPROM write")

    def reset_sensor(self, sensor: int) -> None:
        self.command(CMD_RESET_SENSOR, sensor, comment="cmdResetSensor")
        time.sleep(RESET_SETTLE_S)
        self._log(f"--  waited {RESET_SETTLE_S * 1000:.0f} ms for the sensor to boot")

    def read_register(self, sensor: int, address: int) -> int:
        self.command(CMD_READ_REGISTER, sensor, bytes([0x00, address & 0xFF]),
                     comment=f"cmdReadRegister 0x{address:02X}")
        data = self.command(CMD_GET_REGISTER_VALUE, sensor, n_resp=2,
                            comment="cmdGetRegisterValue")
        return (data[0] << 8) | data[1]

    def set_register(self, sensor: int, address: int, value: int) -> None:
        payload = bytes([address & 0xFF, (value >> 8) & 0xFF, value & 0xFF])
        self.command(CMD_SET_REGISTER, sensor, payload,
                     comment=f"cmdSetRegister 0x{address:02X} = 0x{value:04X}")
        time.sleep(REGISTER_SETTLE_S)

    def enable_mux(self, enable: bool, level: int) -> None:
        self.command(CMD_ENABLE_MUX, 0, bytes([1 if enable else 0, level]),
                     comment="cmdEnableMux")

    def start_readout(self, sensor: int) -> None:
        self.command(CMD_START_READOUT, sensor, comment="cmdStartReadout")

    def stop_readout(self, sensor: int) -> None:
        # The acknowledge arrives interleaved with stream frames, so the input
        # buffer is drained instead of parsed.
        self._write(bytes([START_BYTE, CMD_STOP_READOUT, sensor, 0x00, 0x00]),
                    "cmdStopReadout")
        time.sleep(STREAM_QUIET_S)
        dropped = self._drain()
        self._log(f"--  stream stopped, {dropped} byte(s) discarded")

    def read_adc_frame(self) -> bytes | None:
        """Resynchronize on 0xAA and return one checksum-validated 12-byte frame."""
        deadline = time.time() + 2.0
        while time.time() < deadline:
            marker = self.ser.read(1)
            if not marker:
                return None
            if marker[0] != START_BYTE:
                continue
            rest = self.ser.read(11)
            if len(rest) != 11:
                return None
            frame = marker + rest
            if sum(frame[1:11]) & 0xFF == frame[11]:
                return frame
        return None


def frame_aout_vref(frame: bytes, sensor: int) -> tuple[int, int]:
    high = frame[3 + sensor * 3]
    aout = ((high & 0xF0) << 4) | frame[1 + sensor * 3]
    vref = ((high & 0x0F) << 8) | frame[2 + sensor * 3]
    return aout, vref


# ---------------------------------------------------------------------------
# UI
# ---------------------------------------------------------------------------

class App:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title(f"TLE4972 - Double Code Word Calibration Example v{__version__}")
        self.root.geometry("1280x900")

        self.prg: Programmer | None = None
        self.backup: list[int] | None = None      # image archived in Phase 1
        self.modified: list[int] | None = None    # image with the new coefficients
        self.gain_cw_old: int | None = None
        self.offset_cw_old: int | None = None
        self.t_cal: float | None = None
        self.sensitivity: dict[int, float] = {}   # Gain_CW -> mV/A
        self.offsets: dict[int, float] = {}       # Offset_CW -> mV
        self.baseline_sensitivity: float | None = None   # as found, before calibration
        self.baseline_offset: float | None = None
        self.gain_cw_target: int | None = None
        self.offset_cw_target: int | None = None
        self.result: dict | None = None

        self.busy = False
        self.log_queue: queue.Queue[str] = queue.Queue()
        self.ui_queue: queue.Queue = queue.Queue()

        self._build_ui()
        self.refresh_ports()
        self._pump()

    # -- layout ------------------------------------------------------------

    def _build_ui(self) -> None:
        ttk.Label(
            self.root,
            text=("WARNING: this procedure overwrites the factory calibration coefficients and "
                  "requires a recalculated EEPROM CRC. The original image is archived in Phase 1 "
                  "and can be written back with 'Restore backup'."),
            foreground="#a00000", wraplength=1240, justify="left").pack(fill="x", padx=10, pady=(8, 4))

        bar = ttk.LabelFrame(self.root, text="Connection")
        bar.pack(fill="x", padx=10, pady=4)

        ttk.Label(bar, text="Port:").grid(row=0, column=0, padx=(8, 2), pady=6)
        self.port_var = tk.StringVar()
        self.port_box = ttk.Combobox(bar, textvariable=self.port_var, width=38, state="readonly")
        self.port_box.grid(row=0, column=1, padx=2)
        ttk.Button(bar, text="Refresh", command=self.refresh_ports).grid(row=0, column=2, padx=4)

        self.connect_btn = ttk.Button(bar, text="Connect", command=self.toggle_connection)
        self.connect_btn.grid(row=0, column=3, padx=4)

        ttk.Label(bar, text=f"{BAUDRATE:,} 8N1").grid(row=0, column=4, padx=8)

        ttk.Label(bar, text="Sensor:").grid(row=0, column=5, padx=(16, 2))
        self.sensor_var = tk.StringVar(value="1")
        ttk.Combobox(bar, textvariable=self.sensor_var, width=4, state="readonly",
                     values=("1", "2", "3")).grid(row=0, column=6)

        self.status_var = tk.StringVar(value="disconnected")
        ttk.Label(bar, textvariable=self.status_var).grid(row=0, column=7, padx=16, sticky="w")

        params = ttk.LabelFrame(self.root, text="Calibration inputs")
        params.pack(fill="x", padx=10, pady=4)

        self.target_sens_var = tk.StringVar(value="5.000")
        self.itest_var = tk.StringVar(value="50.0")
        self.adc_scale_var = tk.StringVar(value=f"{3300.0 / 4096.0:.6f}")
        self.frames_var = tk.StringVar(value="200")
        fields = (
            ("Target sensitivity [mV/A]:", self.target_sens_var, 12),
            ("Test current ITEST [A]:", self.itest_var, 12),
            ("ADC scale [mV/LSB]:", self.adc_scale_var, 12),
            ("Frames to average:", self.frames_var, 8),
        )
        for col, (text, var, width) in enumerate(fields):
            ttk.Label(params, text=text).grid(row=0, column=col * 2, padx=(8, 2), pady=6, sticky="e")
            ttk.Entry(params, textvariable=var, width=width).grid(row=0, column=col * 2 + 1, padx=(0, 8))

        actions = ttk.LabelFrame(self.root, text="Sequence")
        actions.pack(fill="x", padx=10, pady=4)

        steps = (
            ("1. Enter firmware mode", self.act_enter_firmware),
            ("2. Init MCU (mapping 0)", self.act_init_mcu),
            ("3. Phase 1 - backup", self.act_backup),
            ("4. Phase 2 - gain sweep", self.act_gain_sweep),
            ("5. Phase 3 - offset sweep", self.act_offset_sweep),
            ("6. Phase 4 - coefficients", self.act_compute),
            ("7. Phase 5 - program", self.act_program_verify),
            ("8. Verify measurement", self.act_verify_measurement),
        )
        self.action_buttons = []
        for index, (text, cmd) in enumerate(steps):
            btn = ttk.Button(actions, text=text, command=cmd, width=24)
            btn.grid(row=index // 4, column=index % 4, padx=4, pady=4)
            self.action_buttons.append(btn)

        extra = ttk.Frame(actions)
        extra.grid(row=0, column=4, rowspan=2, padx=(16, 4))
        self.restore_btn = ttk.Button(extra, text="Restore backup", command=self.act_restore, width=18)
        self.restore_btn.pack(pady=2)
        self.action_buttons.append(self.restore_btn)
        ttk.Button(extra, text="Clear log", command=self.clear_log, width=18).pack(pady=2)

        middle = ttk.Frame(self.root)
        middle.pack(fill="both", expand=True, padx=10, pady=4)

        mem = ttk.LabelFrame(middle, text="EEPROM image (18 x 16-bit words)")
        mem.pack(side="left", fill="both", expand=True)

        self.tree = ttk.Treeview(mem, columns=("addr", "word", "value", "new", "decoded"),
                                 show="headings", height=18)
        for name, title, width in (
            ("addr", "Addr", 60), ("word", "Word", 55), ("value", "Backup", 75),
            ("new", "To write", 80), ("decoded", "Decoded", 400),
        ):
            self.tree.heading(name, text=title)
            self.tree.column(name, width=width, anchor="w")
        self.tree.tag_configure("changed", background="#fff2cc")
        self.tree.tag_configure("readonly", foreground="#777777")
        self.tree.pack(side="left", fill="both", expand=True, padx=(6, 0), pady=6)
        ttk.Scrollbar(mem, orient="vertical", command=self.tree.yview).pack(side="left", fill="y", pady=6)

        side = ttk.Frame(middle)
        side.pack(side="left", fill="y", padx=(10, 0))

        results = ttk.LabelFrame(side, text="Calibration results")
        results.pack(fill="both", expand=True)
        self.results_text = tk.Text(results, width=52, height=26, font=("Consolas", 9),
                                    wrap="none", state="disabled")
        self.results_text.pack(fill="both", expand=True, padx=6, pady=6)

        logbox = ttk.LabelFrame(self.root, text="Serial console")
        logbox.pack(fill="both", expand=True, padx=10, pady=(4, 10))
        self.log_text = tk.Text(logbox, height=12, font=("Consolas", 9), wrap="none")
        self.log_text.pack(side="left", fill="both", expand=True, padx=(6, 0), pady=6)
        scroll = ttk.Scrollbar(logbox, orient="vertical", command=self.log_text.yview)
        scroll.pack(side="left", fill="y", pady=6)
        self.log_text.configure(yscrollcommand=scroll.set)

    # -- logging and thread plumbing ---------------------------------------

    def log(self, message: str) -> None:
        self.log_queue.put(message)

    def post(self, callback) -> None:
        """Schedule a UI update from a worker thread (Tk must only be touched here)."""
        self.ui_queue.put(callback)

    def _pump(self) -> None:
        while True:
            try:
                message = self.log_queue.get_nowait()
            except queue.Empty:
                break
            self.log_text.insert("end", message + "\n")
            self.log_text.see("end")
        while True:
            try:
                callback = self.ui_queue.get_nowait()
            except queue.Empty:
                break
            try:
                callback()
            except Exception as exc:
                self.log(f"!! UI update failed: {exc!r}")
        self.root.after(80, self._pump)

    def clear_log(self) -> None:
        self.log_text.delete("1.0", "end")

    def ask_operator(self, title: str, message: str) -> bool:
        """Blocking confirmation for a worker thread; the dialog runs on the main thread."""
        done = threading.Event()
        answer: dict[str, bool] = {}

        def show():
            answer["ok"] = bool(messagebox.askokcancel(title, message))
            done.set()

        self.post(show)
        done.wait()
        return answer.get("ok", False)

    # -- connection --------------------------------------------------------

    def refresh_ports(self) -> None:
        if list_ports is None:
            self.port_box["values"] = ()
            self.log("!! pyserial is not installed - run: pip install pyserial")
            return
        ports = [f"{p.device} - {p.description}" for p in list_ports.comports()]
        self.port_box["values"] = ports
        if ports and not self.port_var.get():
            self.port_var.set(ports[0])
        self.log(f"[host] {len(ports)} serial port(s) found")

    def toggle_connection(self) -> None:
        if self.prg is not None:
            self.prg.close()
            self.prg = None
            self.connect_btn.config(text="Connect")
            self.status_var.set("disconnected")
            self.log("[host] port closed")
            return

        selection = self.port_var.get()
        if not selection:
            messagebox.showwarning("No port", "Select a serial port first.")
            return
        port = selection.split(" - ")[0]
        try:
            self.prg = Programmer(port, self.log)
        except Exception as exc:
            messagebox.showerror("Connection failed", str(exc))
            self.log(f"!! {exc}")
            return
        self.connect_btn.config(text="Disconnect")
        self.status_var.set(f"connected: {port}")
        self.log(f"[host] {port} open @ {BAUDRATE} 8N1, no flow control")

    @property
    def sensor(self) -> int:
        return int(self.sensor_var.get()) - 1

    def _run(self, func) -> None:
        if self.busy:
            return
        if self.prg is None:
            messagebox.showwarning("Not connected", "Connect to a serial port first.")
            return
        self.busy = True
        for btn in self.action_buttons:
            btn.state(["disabled"])

        def worker():
            try:
                func()
            except (ProtocolError, ValueError) as exc:
                self.log(f"!! {exc}")
                text = str(exc)
                self.post(lambda: messagebox.showerror("Calibration aborted", text))
            except Exception as exc:  # keep the UI alive on unexpected failures
                self.log(f"!! {exc!r}")
                text = repr(exc)
                self.post(lambda: messagebox.showerror("Error", text))
            finally:
                self.post(self._done)

        threading.Thread(target=worker, daemon=True).start()

    def _done(self) -> None:
        self.busy = False
        for btn in self.action_buttons:
            btn.state(["!disabled"])

    # -- input parsing -----------------------------------------------------

    def _inputs(self) -> tuple[float, float, float, int]:
        try:
            target = float(self.target_sens_var.get())
            itest = float(self.itest_var.get())
            scale = float(self.adc_scale_var.get())
            frames = int(self.frames_var.get())
        except ValueError as exc:
            raise ValueError(f"invalid calibration input: {exc}") from exc
        if target <= 0.0:
            raise ValueError("target sensitivity must be > 0 mV/A")
        if itest <= 0.0:
            raise ValueError("test current ITEST must be > 0 A")
        if scale <= 0.0:
            raise ValueError("ADC scale must be > 0 mV/LSB")
        if frames < 100:
            raise ValueError("the user manual requires averaging over at least 100 samples")
        return target, itest, scale, frames

    # -- measurement primitives -------------------------------------------

    def _encode_offset_cw(self, value: int) -> int:
        if not -127 <= value <= 127:
            raise ValueError(f"Offset_CW {value} outside the 8 bit sign-magnitude range -127..127")
        magnitude = abs(value) & ((1 << OFFSET_CW_SIGN_BIT) - 1)
        return magnitude | ((1 << OFFSET_CW_SIGN_BIT) if value < 0 else 0)

    def _decode_offset_cw(self, raw: int) -> int:
        magnitude = raw & ((1 << OFFSET_CW_SIGN_BIT) - 1)
        return -magnitude if raw & (1 << OFFSET_CW_SIGN_BIT) else magnitude

    def _enter_test_mode_verified(self) -> None:
        """cmdEnterTestMode, then prove SICI answers by reading a plausible temperature."""
        assert self.prg
        for attempt in range(1, SICI_ENTRY_ATTEMPTS + 1):
            self.prg.enter_test_mode(self.sensor)
            raw = self.prg.read_register(self.sensor, REG_TEMPERATURE)
            celsius = temperature_celsius(raw & TINT_MASK)
            plausible = SICI_PROBE_MIN_C <= celsius <= SICI_PROBE_MAX_C
            self.log(f"[host] SICI check: TINT = 0x{raw:04X} -> {celsius:+.1f} degC"
                     + ("" if plausible else f"  <- implausible, attempt {attempt}"))
            if plausible:
                return
        raise ProtocolError(
            f"SICI is not responding: the temperature register read back 0x{raw:04X}, "
            f"which decodes to {celsius:+.1f} degC. The sensor is not in SICI mode, so "
            "register reads and writes are meaningless. Check the SICI/AOUT connection, "
            "the ground return and that the part was fully power-cycled.")

    def _verify_eeprom(self, target: list[int], label: str) -> tuple[bool, list[int]]:
        """Compare the EEPROM against target, re-reading because single words can garble."""
        assert self.prg
        words: list[int] = []
        for attempt in range(1, EEPROM_VERIFY_ATTEMPTS + 1):
            words = self.prg.read_eeprom(self.sensor)
            differing = [i for i, (a, b) in enumerate(zip(words, target)) if a != b]
            crc_ok = eeprom_crc_check(words)
            self.log(f"[host] {label}: {'MATCH' if not differing else 'MISMATCH'} "
                     f"({EEPROM_LINE_COUNT - len(differing)}/{EEPROM_LINE_COUNT} words), "
                     f"CRC {'VALID' if crc_ok else 'INVALID'}")
            for index in differing:
                self.log(f"[host]   word {index:>2}: wrote 0x{target[index]:04X}, "
                         f"read 0x{words[index]:04X}")
            if not differing and crc_ok:
                return True, words
            if attempt < EEPROM_VERIFY_ATTEMPTS:
                self.log(f"[host] re-reading the EEPROM ({attempt} of "
                         f"{EEPROM_VERIFY_ATTEMPTS} attempts used)")
        return False, words

    def _program_verified(self, target: list[int]) -> bool:
        """Burn the image and confirm it; burnEEPROM() does not verify its own transfers."""
        assert self.prg
        for cycle in range(1, PROGRAM_ATTEMPTS + 1):
            self.log(f"[host] burn cycle {cycle} of at most {PROGRAM_ATTEMPTS}")
            self.prg.program_eeprom(self.sensor, target)
            ok, _ = self._verify_eeprom(target, "pre-reset read-back")
            if ok:
                return True
            if cycle < PROGRAM_ATTEMPTS:
                self.log("[host] a word did not take - re-programming")
        return False

    def _write_code_word(self, address: int, value: int, mask: int, label: str) -> bool:
        """Write a code word and confirm it over SICI, retrying the transfer if it garbles."""
        assert self.prg
        for attempt in range(1, REGISTER_WRITE_ATTEMPTS + 1):
            self.prg.set_register(self.sensor, address, value)
            raw_back = self.prg.read_register(self.sensor, address)
            back = raw_back & mask
            self.log(f"[host] {label} written 0x{value:04X}, read back 0x{back:04X} "
                     f"(raw 0x{raw_back:04X})"
                     + ("" if back == value else f"  <- mismatch, attempt {attempt}"))
            if back == value:
                return True
        return False

    def _enter_measurement_mode(self, gain_cw: int | None = None,
                                offset_cw: int | None = None) -> None:
        """Power cycle, write the code words, then release AOUT with the ISM down."""
        assert self.prg and self.backup is not None
        if gain_cw is not None and not 0 <= gain_cw <= GAIN_CW_MASK:
            raise ValueError(f"Gain_CW {gain_cw} outside the 11 bit field range 0..{GAIN_CW_MASK}")
        offset_raw = None if offset_cw is None else self._encode_offset_cw(offset_cw)

        for attempt in range(1, MEASUREMENT_ENTRY_ATTEMPTS + 1):
            # cmdEnterTestMode power-cycles VDD itself (with OCD/SICI held low first),
            # so no separate cmdResetSensor is needed here.
            self._enter_test_mode_verified()
            written = (gain_cw is None
                       or self._write_code_word(REG_GAIN_CW, gain_cw, GAIN_CW_MASK, "Gain_CW"))
            if written and offset_raw is not None:
                written = self._write_code_word(REG_OFFSET_CW, offset_raw, OFFSET_CW_MASK,
                                                f"Offset_CW ({offset_cw:+d})")
            if written:
                break
            self.log(f"[host] code word setup attempt {attempt} failed, "
                     "power-cycling and re-entering SICI")
        else:
            raise ProtocolError(
                "The code words could not be confirmed over SICI after "
                f"{MEASUREMENT_ENTRY_ATTEMPTS} power cycles. The SICI link is unreliable: "
                "check the AOUT/SICI wiring, the ground return and the supply decoupling.")

        self.prg.set_register(self.sensor, REG_SICI_BYPASS, SICI_BYPASS_MEASURE)
        self.log("[host] SICI disabled, ISM still down, AOUT buffer in normal operating mode")
        time.sleep(0.01)  # >= TISM_SETTLING (100 us)

        single_ended = get_opmode(self.backup) == OPMODE_SINGLE_ENDED
        self.prg.enable_mux(single_ended, 1 if single_ended else 0)
        self.log(f"[host] VREF mux {'enabled (single-ended)' if single_ended else 'disabled (differential)'}")

    def _measure_vo(self, frames: int, scale: float, label: str) -> float:
        """Average VO = V(AOUT) - V(VREF) over `frames` valid readout frames."""
        assert self.prg
        self.prg.start_readout(self.sensor)
        total = 0
        collected = 0
        dropped = 0
        while collected < frames:
            frame = self.prg.read_adc_frame()
            if frame is None:
                dropped += 1
                if dropped > 10:
                    self.prg.stop_readout(self.sensor)
                    raise ProtocolError("no valid ADC frames received - is the sensor streaming?")
                continue
            aout, vref = frame_aout_vref(frame, self.sensor)
            total += aout - vref
            collected += 1
        self.prg.stop_readout(self.sensor)
        average_code = total / collected
        millivolts = average_code * scale
        self.log(f"[host] {label}: {collected} frames, VO = {average_code:+.2f} LSB "
                 f"= {millivolts:+.4f} mV")
        return millivolts

    def _measure_point(self, frames: int, scale: float, itest: float,
                       label: str, with_current: bool) -> float:
        prompt = (f"Apply the test current ITEST = {itest} A to the primary conductor."
                  if with_current else
                  "Set the primary current to exactly 0 A.")
        if not self.ask_operator(f"Operator action - {label}", prompt + "\n\nPress OK to measure."):
            raise ValueError("aborted by the operator")
        return self._measure_vo(frames, scale, label)

    # -- actions -----------------------------------------------------------

    def act_enter_firmware(self) -> None:
        def job():
            assert self.prg
            self.log("--- bootloader handshake ---")
            if self.prg.bootloader_echo():
                self.log("[host] bootloader responding -> link OK")
                if self.prg.bootloader_start_firmware():
                    self.log("[host] firmware mode active")
                version = self.prg.fw_version(self.sensor)
            else:
                self.log("[host] assuming firmware mode is already active")
                self.log(f"[host] waiting up to {FW_VERSION_TIMEOUT_S:.0f} s for cmdFwVersion")
                version = self.prg.fw_version(self.sensor, timeout=FW_VERSION_TIMEOUT_S)
            self.log(f"[host] firmware version {version}")
        self._run(job)

    def act_init_mcu(self) -> None:
        def job():
            assert self.prg
            self.log("--- init MCU, array config 0 (default mapping) ---")
            self.prg.init_mcu(self.sensor, 0)
            self.log("[host] MCU initialized")
        self._run(job)

    def act_backup(self) -> None:
        def job():
            assert self.prg
            self.log("--- Phase 1: back up the device state ---")
            self._enter_test_mode_verified()
            words = self.prg.read_eeprom(self.sensor)

            stored = words[2] & 0xFF
            calculated = eeprom_crc(words)
            self.log("[host] EEPROM image archived (36 bytes):")
            for line in hex_dump(words_to_bytes(words)):
                self.log(f"       {line}")
            self.log(f"[host] CRC stored = 0x{stored:02X}, calculated = 0x{calculated:02X} "
                     f"-> {'VALID' if stored == calculated else 'INVALID'}")
            if stored != calculated:
                raise ValueError("the EEPROM image read from the sensor already has an "
                                 "invalid CRC - refusing to calibrate")

            gain_cw_raw = self.prg.read_register(self.sensor, REG_GAIN_CW)
            offset_raw_full = self.prg.read_register(self.sensor, REG_OFFSET_CW)
            t_int_raw = self.prg.read_register(self.sensor, REG_TEMPERATURE)
            gain_cw = gain_cw_raw & GAIN_CW_MASK
            offset_raw = offset_raw_full & OFFSET_CW_MASK
            offset_cw = self._decode_offset_cw(offset_raw)
            t_int = t_int_raw & TINT_MASK
            t_cal = temperature_celsius(t_int)

            self.log(f"[host] Gain_CW_old   = {gain_cw} (raw 0x{gain_cw_raw:04X}, 11 bit)")
            self.log(f"[host] Offset_CW_old = {offset_cw} (raw 0x{offset_raw_full:04X}, "
                     f"8 bit sign-magnitude)")
            self.log(f"[host] TINT = {t_int} -> TCAL = {t_cal:.2f} C")

            path = save_backup_file(words, self.sensor, gain_cw, offset_cw, t_cal)
            self.log(f"[host] backup written to {path}")

            def update():
                self.backup = words
                self.modified = None
                self.gain_cw_old = gain_cw
                self.offset_cw_old = offset_cw
                self.t_cal = t_cal
                self.sensitivity.clear()
                self.offsets.clear()
                self.baseline_sensitivity = None
                self.baseline_offset = None
                self.gain_cw_target = None
                self.offset_cw_target = None
                self.result = None
                self._refresh_tree()
                self._refresh_results()
            self.post(update)
        self._run(job)

    def act_gain_sweep(self) -> None:
        if self.backup is None:
            messagebox.showwarning("No backup", "Run Phase 1 first.")
            return
        target, itest, scale, frames = self._inputs()

        def job():
            self.log(f"--- Phase 2: gain sweep, Gain_CW = {GAIN_CW1} and {GAIN_CW2} ---")
            # The device still holds the code words it booted with, so this first
            # readout is the as-found performance.
            self.log("[host] baseline readout with the original code words")
            self._enter_measurement_mode()
            base_zero = self._measure_point(frames, scale, itest, "baseline @ 0 A", False)
            base_test = self._measure_point(frames, scale, itest, "baseline @ ITEST", True)
            base_sensitivity = (base_test - base_zero) / itest
            base_error = (base_sensitivity - target) / target * 100.0
            self.log(f"[host] baseline sensitivity = {base_sensitivity:.6f} mV/A "
                     f"(target {target:.6f}, error {base_error:+.3f} %)")
            self.log(f"[host] baseline offset      = {base_zero:+.6f} mV")

            measured: dict[int, float] = {}
            for gain_cw in (GAIN_CW1, GAIN_CW2):
                self.log(f"[host] Gain_CW = {gain_cw}")
                self._enter_measurement_mode(gain_cw=gain_cw)
                vo_zero = self._measure_point(frames, scale, itest,
                                              f"Gain_CW {gain_cw} @ 0 A", False)
                vo_test = self._measure_point(frames, scale, itest,
                                              f"Gain_CW {gain_cw} @ ITEST", True)
                sensitivity = (vo_test - vo_zero) / itest
                measured[gain_cw] = sensitivity
                self.log(f"[host] Sensitivity(Gain_CW={gain_cw}) = {sensitivity:.6f} mV/A")

            ratio, gain_cw_target = interpolate_gain_cw(
                measured[GAIN_CW1], measured[GAIN_CW2], target)
            rounded = round(gain_cw_target)
            self.log(f"[host] Sensitivity_RATIO = {ratio:.6f}")
            self.log(f"[host] Gain_CW_target = {gain_cw_target:.2f} -> {rounded} (0x{rounded:03X})")
            if not 0 <= rounded <= GAIN_CW_MASK:
                raise ValueError(f"Gain_CW_target {rounded} is outside the 11 bit field "
                                 f"range 0..{GAIN_CW_MASK} - check the target sensitivity")

            def update():
                self.sensitivity = measured
                self.baseline_sensitivity = base_sensitivity
                self.baseline_offset = base_zero
                self.gain_cw_target = rounded
                self._refresh_results()
            self.post(update)
        self._run(job)

    def act_offset_sweep(self) -> None:
        if self.gain_cw_target is None:
            messagebox.showwarning("No Gain_CW_target", "Run Phase 2 first.")
            return
        _, itest, scale, frames = self._inputs()
        gain_cw_target = self.gain_cw_target

        def job():
            self.log(f"--- Phase 3: offset sweep, Offset_CW = {OFFSET_CW1} and {OFFSET_CW2} "
                     f"with Gain_CW = {gain_cw_target} ---")
            measured: dict[int, float] = {}
            for offset_cw in (OFFSET_CW1, OFFSET_CW2):
                self.log(f"[host] Offset_CW = {offset_cw}")
                self._enter_measurement_mode(gain_cw=gain_cw_target, offset_cw=offset_cw)
                measured[offset_cw] = self._measure_point(
                    frames, scale, itest, f"Offset_CW {offset_cw} @ 0 A", False)
                self.log(f"[host] Offset(Offset_CW={offset_cw}) = {measured[offset_cw]:+.6f} mV")

            average, ratio, offset_cw_target = interpolate_offset_cw(
                measured[OFFSET_CW1], measured[OFFSET_CW2])
            rounded = round(offset_cw_target)
            self.log(f"[host] Offset_AVG = {average:+.6f} mV, "
                     f"Offset_RATIO = {ratio:.6f} mV/LSB")
            self.log(f"[host] Offset_CW_target = {offset_cw_target:.2f} -> {rounded}")
            if not -127 <= rounded <= 127:
                raise ValueError(f"Offset_CW_target {rounded} is outside the 8 bit field "
                                 f"range -127..127 - check the measurement")

            def update():
                self.offsets = measured
                self.offset_cw_target = rounded
                self._refresh_results()
            self.post(update)
        self._run(job)

    def act_compute(self) -> None:
        if self.busy:
            return
        if (self.backup is None or self.gain_cw_target is None
                or self.offset_cw_target is None or self.gain_cw_old is None
                or self.offset_cw_old is None or self.t_cal is None):
            messagebox.showwarning("Incomplete", "Run Phases 1 to 3 first.")
            return
        try:
            result = compute_new_coefficients(
                self.backup, self.gain_cw_old, self.offset_cw_old,
                self.gain_cw_target, self.offset_cw_target, self.t_cal)
        except ValueError as exc:
            self.log(f"!! {exc}")
            messagebox.showerror("Phase 4 failed", str(exc))
            return

        self.log("--- Phase 4: recalculate the EEPROM coefficients ---")
        self.log(f"[host] Gain_old(TCAL) = {result['gain_old_cal']:.6f}, "
                 f"Gain_new(TCAL) = {result['gain_new_cal']:.6f}, "
                 f"Gain_CF = {result['gain_cf']:.6f}")
        for celsius, code, value in result["support"]:
            self.log(f"[host] Gain_CW_target({celsius:+.1f} C, T={code:.0f}) = {value:.3f}")
        fit = result["fit"]
        self.log(f"[host] BASE_new = {fit['BASE']:.4f}, TL_new = {fit['TL']:.6e}, "
                 f"TQ_new = {fit['TQ']:.6e}, TT_new = {fit['TT']:.6e}")
        for name in ("g_base", "g_tc_tl", "g_tc_tq", "g_tc_tt", "o_base"):
            self.log(f"[host] {name:<9}: {result['old'][name]:>6} -> {result['new'][name]:>6}")
        for index in (3, 4, 5, 8):
            self.log(f"[host] word {index:>2}: 0x{self.backup[index]:04X} -> "
                     f"0x{result['words'][index]:04X}")
        self.log(f"[host] CRC: 0x{self.backup[2] & 0xFF:02X} -> 0x{result['crc']:02X}")
        self.log("[host] image to program (8 bytes per line):")
        for line in hex_dump(words_to_bytes(result["words"])):
            self.log(f"       {line}")

        self.result = result
        self.modified = result["words"]
        self._refresh_tree()
        self._refresh_results()

    def act_program_verify(self) -> None:
        if self.modified is None:
            messagebox.showwarning("Nothing to program", "Run Phase 4 first.")
            return
        if not messagebox.askyesno(
                "Confirm programming",
                f"Overwrite the calibration coefficients of sensor {self.sensor_var.get()}?\n\n"
                f"New CRC: 0x{self.modified[2] & 0xFF:02X}\n\n"
                "This replaces factory calibration data. The EEPROM supports a limited "
                "number of programming cycles (max 100)."):
            return

        target = list(self.modified)

        def job():
            assert self.prg
            self.log("--- Phase 5: program and verify ---")
            # SICI was switched off by the sweeps; cmdEnterTestMode power-cycles the part.
            self._enter_test_mode_verified()
            ok = self._program_verified(target)
            if not ok:
                self.log("[host] PROGRAMMING FAILED - the sensor was NOT reset, so the old "
                         "content is still active and SICI is still available")
                self.post(lambda: messagebox.showerror(
                    "Programming failed",
                    "The EEPROM read-back does not match what was written.\n\n"
                    "The sensor has NOT been reset, so the previous content is still "
                    "active. Use 'Restore backup' or retry before power-cycling it."))
                return

            self.log("[host] content verified, resetting the sensor to activate it")
            self.prg.reset_sensor(self.sensor)
            self._enter_test_mode_verified()
            match, readback = self._verify_eeprom(target, "post-reset read-back")
            crc_ok = eeprom_crc_check(readback)
            self.log(f"[host] CRC stored = 0x{readback[2] & 0xFF:02X}, "
                     f"calculated = 0x{eeprom_crc(readback):02X} "
                     f"-> {'VALID' if crc_ok else 'INVALID'}")
            self.log("[host] CALIBRATION PROGRAMMED" if match and crc_ok
                     else "[host] PROGRAMMING FAILED - restore the backup")

            def update():
                if match and crc_ok:
                    messagebox.showinfo("Success", "Read-back matches and the CRC is valid.\n\n"
                                                   "Run step 8 to confirm sensitivity and offset.")
                else:
                    messagebox.showerror("Verification failed",
                                         "Read-back does not match or the CRC is invalid.\n\n"
                                         "Use 'Restore backup' before power-cycling the part.")
            self.post(update)
        self._run(job)

    def act_verify_measurement(self) -> None:
        if self.backup is None:
            messagebox.showwarning("No backup", "Run Phase 1 first.")
            return
        target, itest, scale, frames = self._inputs()

        def job():
            assert self.prg and self.backup is not None
            self.log("--- verification: measure sensitivity and offset in normal operation ---")
            self.prg.reset_sensor(self.sensor)
            single_ended = get_opmode(self.backup) == OPMODE_SINGLE_ENDED
            self.prg.enable_mux(single_ended, 1 if single_ended else 0)

            vo_zero = self._measure_point(frames, scale, itest, "verify @ 0 A", False)
            vo_test = self._measure_point(frames, scale, itest, "verify @ ITEST", True)
            sensitivity = (vo_test - vo_zero) / itest
            error = (sensitivity - target) / target * 100.0
            self.log(f"[host] measured sensitivity = {sensitivity:.6f} mV/A "
                     f"(target {target:.6f}, error {error:+.3f} %)")
            self.log(f"[host] measured offset      = {vo_zero:+.6f} mV (target 0)")

            base_s = self.baseline_sensitivity
            base_o = self.baseline_offset
            comparison = ""
            if base_s is not None and base_o is not None:
                base_error = (base_s - target) / target * 100.0
                self.log(f"[host] baseline was {base_s:.6f} mV/A (error {base_error:+.3f} %), "
                         f"offset {base_o:+.6f} mV")
                comparison = (
                    f"  vs baseline : {base_s:.6f} -> {sensitivity:.6f} mV/A "
                    f"(error {base_error:+.3f} -> {error:+.3f} %)\n"
                    f"                {base_o:+.6f} -> {vo_zero:+.6f} mV offset\n")

            def update():
                self._append_results(
                    "\nverification\n"
                    f"  sensitivity : {sensitivity:.6f} mV/A (error {error:+.3f} %)\n"
                    f"  offset      : {vo_zero:+.6f} mV\n" + comparison)
            self.post(update)
        self._run(job)

    def act_restore(self) -> None:
        if self.backup is None:
            messagebox.showwarning("No backup", "Run Phase 1 first.")
            return
        if not messagebox.askyesno(
                "Confirm restore",
                f"Write the Phase 1 backup image back to sensor {self.sensor_var.get()}?\n\n"
                f"CRC: 0x{self.backup[2] & 0xFF:02X} (already valid, no recalculation needed)"):
            return
        target = list(self.backup)

        def job():
            assert self.prg
            self.log("--- rollback: reprogram the Phase 1 backup image ---")
            self._enter_test_mode_verified()
            written = self._program_verified(target)
            if not written:
                self.log("[host] RESTORE FAILED - pre-reset read-back mismatch, "
                         "the sensor was NOT reset")
                self.post(lambda: messagebox.showerror(
                    "Restore failed",
                    "The read-back does not match the backup. The sensor has NOT "
                    "been reset - retry the restore."))
                return

            self.prg.reset_sensor(self.sensor)
            self._enter_test_mode_verified()
            ok, _ = self._verify_eeprom(target, "post-reset read-back")
            self.log("[host] ORIGINAL IMAGE RESTORED" if ok else "[host] RESTORE FAILED")
            self.post(lambda: messagebox.showinfo("Restore", "Original image restored.")
                      if ok else messagebox.showerror("Restore failed",
                                                      "The read-back does not match the backup."))
        self._run(job)

    # -- view helpers ------------------------------------------------------

    def _refresh_tree(self) -> None:
        self.tree.delete(*self.tree.get_children())
        if self.backup is None:
            return
        for index, word in enumerate(self.backup):
            new = self.modified[index] if self.modified else None
            changed = new is not None and new != word
            tags = ("changed",) if changed else (("readonly",) if index in (2, 3, 4, 5, 8) else ())
            self.tree.insert("", "end", tags=tags, values=(
                f"0x{0x40 + index:02X}",
                index,
                f"0x{word:04X}",
                f"0x{new:04X}" if new is not None else "",
                decode_word(index, new if new is not None else word),
            ))

    def _refresh_results(self) -> None:
        lines: list[str] = []
        lines.append("device state (Phase 1)")
        lines.append(f"  Gain_CW_old   : {self._fmt(self.gain_cw_old)}")
        lines.append(f"  Offset_CW_old : {self._fmt(self.offset_cw_old)}")
        lines.append(f"  TCAL          : {self._fmt(self.t_cal, '.2f')} C")
        lines.append("")
        lines.append("baseline, as found (Phase 2)")
        lines.append(f"  sensitivity   : {self._fmt(self.baseline_sensitivity, '.6f')} mV/A")
        lines.append(f"  offset        : {self._fmt(self.baseline_offset, '+.6f')} mV")
        lines.append("")
        lines.append("gain sweep (Phase 2)")
        for cw in (GAIN_CW1, GAIN_CW2):
            value = self.sensitivity.get(cw)
            lines.append(f"  S(Gain_CW={cw:>3}) : "
                         f"{'-' if value is None else f'{value:.6f} mV/A'}")
        lines.append(f"  Gain_CW_target: {self._fmt(self.gain_cw_target)}")
        lines.append("")
        lines.append("offset sweep (Phase 3)")
        for cw in (OFFSET_CW1, OFFSET_CW2):
            value = self.offsets.get(cw)
            lines.append(f"  O(Offset_CW={cw:>3}): "
                         f"{'-' if value is None else f'{value:+.6f} mV'}")
        lines.append(f"  Offset_CW_tgt : {self._fmt(self.offset_cw_target)}")
        lines.append("")
        lines.append("coefficients (Phase 4)")
        if self.result is None:
            lines.append("  not calculated yet")
        else:
            lines.append(f"  Gain_CF       : {self.result['gain_cf']:.6f}")
            lines.append(f"  {'field':<10}{'old':>8}{'new':>8}")
            for name in ("g_base", "g_tc_tl", "g_tc_tq", "g_tc_tt", "o_base"):
                lines.append(f"  {name:<10}{self.result['old'][name]:>8}"
                             f"{self.result['new'][name]:>8}")
            lines.append("")
            lines.append(f"  {'word':<10}{'backup':>8}{'new':>8}")
            for index in (2, 3, 4, 5, 8):
                lines.append(f"  {index:<10}  0x{self.backup[index]:04X}  0x"
                             f"{self.result['words'][index]:04X}")
            lines.append(f"  CRC           : 0x{self.backup[2] & 0xFF:02X} -> "
                         f"0x{self.result['crc']:02X}")
        self._set_results("\n".join(lines) + "\n")

    @staticmethod
    def _fmt(value, spec: str = "") -> str:
        return "-" if value is None else format(value, spec)

    def _set_results(self, text: str) -> None:
        self.results_text.configure(state="normal")
        self.results_text.delete("1.0", "end")
        self.results_text.insert("1.0", text)
        self.results_text.configure(state="disabled")

    def _append_results(self, text: str) -> None:
        self.results_text.configure(state="normal")
        self.results_text.insert("end", text)
        self.results_text.configure(state="disabled")

    def on_close(self) -> None:
        if self.prg is not None:
            self.prg.close()
        self.root.destroy()


def main() -> None:
    root = tk.Tk()
    app = App(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()


if __name__ == "__main__":
    main()
