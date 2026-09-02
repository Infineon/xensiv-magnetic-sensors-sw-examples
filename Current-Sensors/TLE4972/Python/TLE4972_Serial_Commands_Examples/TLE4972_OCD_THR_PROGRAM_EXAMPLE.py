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

"""TLE4972 OCD threshold programming example with a Tkinter UI.

Implements the "Complete Programming Example" from README.md:
port scan -> connect -> firmware mode -> init MCU -> test mode -> read EEPROM ->
modify OCD1/OCD2 thresholds -> recalculate CRC -> program -> power cycle ->
read back and verify.

WARNING: The sensor EEPROM CRC is never recalculated by the firmware or the
sensor. Programming an image with a wrong CRC leaves the part in a permanent
fault state (OCD outputs driven to GND) after its next start-up.


REQUIREMENTS
------------
* Python 3.10 or newer (uses ``X | None`` annotations).
* pyserial:  pip install pyserial
* Tkinter (part of the standard Windows/macOS Python installers).
* A programmer board running the *firmware* image, not only the bootloader.
  The firmware itself must be flashed with the Infineon evaluation kit; this
  script only switches an already-flashed board from bootloader to firmware.

USAGE
-----
    python TLE4972_OCD_THR_PROGRAM_EXAMPLE.py

1.  Plug in the board, press *Refresh*, pick the COM port and press *Connect*.
    The port opens at 1 250 000 baud, 8N1, no flow control.
2.  Select the sensor socket (1..3). The UI sends the zero-based index on the
    wire; the labels are one-based, as in the user manual.
3.  Run the numbered buttons in order:
      1. Enter firmware mode  - bootloader echo (0x09), start firmware (0x12),
                                then read the firmware version (0x21).
      2. Init MCU             - cmdInitMCU (0x22) with array configuration 0
                                (default zero mapping). This re-initialises all
                                peripherals and can take several seconds, hence
                                the extended INIT_MCU_TIMEOUT_S.
      3. Test mode + read     - cmdEnterTestMode (0x11) puts the sensor into
                                SICI test mode (ISM powered down), then
                                cmdReadEEPROM (0x10) returns 36 bytes = 18 words.
      4. Apply + recalc CRC   - writes the OCD1/OCD2 spinbox values into word 1,
                                recalculates the CRC and patches word 2. Nothing
                                is sent to the board in this step.
      5. Program + verify     - cmdProgramEEPROM (0x26), cmdResetSensor (0x13),
                                re-enter test mode, read back and compare.
4.  The console pane mirrors every frame as TX/RX hex, 8 bytes per line, so the
    output can be compared one-to-one with the transcripts in README.md.

MEMORY VIEW
-----------
The table lists all 18 EEPROM words with their decoded fields. Rows that differ
between the read image and the image to be written are highlighted; word 2 is
greyed because its low byte is the CRC and is managed by this script.
Only the OCD1/OCD2 thresholds are editable - every other bit is copied
unchanged from the image that was read, so an unread sensor can never be
programmed with fabricated content.

CRC
---
CRC-8 SAE-J1850: polynomial 0x1D, seed 0xAA, final inversion 0xFF. The byte
stream runs over lines 3..17, then 0..1, then the high byte of line 2; the low
byte of line 2 is the CRC field itself and is excluded. This matches
``eepromCrcCalc()`` in src/sici/eeprom_crc.c and the Python reference in
tools/tle4972_crc.py, and was verified against both example XML memory maps.

SAFETY NOTES
------------
* The EEPROM supports a limited number of programming cycles (max 100). Step 5
  asks for confirmation and refuses to run if nothing actually changed.
* cmdResetSensor (0x13) cycles the shared 3.3 V rail, so it power-cycles all
  three sockets regardless of the selected sensor.
* Programming always uses the image that was read back from the device, so a
  failed or partial read aborts the sequence instead of writing garbage.

TROUBLESHOOTING
---------------
* "timeout: expected N bytes, got 0" - the board is in the wrong mode (run
  step 1 again), the wrong COM port is selected, or another application still
  holds the port open.
* Step 2 timing out - increase INIT_MCU_TIMEOUT_S.
* Read-back mismatch - do NOT power-cycle and walk away; re-read the EEPROM and
  reprogram a valid image, otherwise the part stays in the CRC fault state.
"""

from __future__ import annotations

import queue
import threading
import time
import tkinter as tk
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
CMD_FW_VERSION = 0x21
CMD_INIT_MCU = 0x22
CMD_PROGRAM_EEPROM = 0x26

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
EEPROM_VERIFY_ATTEMPTS = 3      # re-reads before a mismatch counts as a bad write
PROGRAM_ATTEMPTS = 3            # burn cycles before giving up (endurance is 100)

EEPROM_LINE_COUNT = 18
EEPROM_BYTE_COUNT = EEPROM_LINE_COUNT * 2

CRC_POLYNOMIAL = 0x1D
CRC_SEED = 0xAA

MEASRNG_NAMES = {0x05: "S1", 0x06: "S2", 0x08: "S3", 0x0C: "S4", 0x10: "S5", 0x18: "S6"}
OPMODE_NAMES = {0: "SDBID", 1: "FD", 2: "SDUNI", 3: "SE"}
VREFEXT_NAMES = {0: "1.65 V", 2: "1.5 V", 3: "1.8 V"}
HYST_NAMES = {0: "0*FS", 1: "0.0625*FS", 2: "0.125*FS", 3: "0.25*FS"}


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


# ---------------------------------------------------------------------------
# EEPROM field access
# ---------------------------------------------------------------------------

def get_ocd1_threshold(words: list[int]) -> int:
    return (words[1] >> 2) & 0x3F


def get_ocd2_threshold(words: list[int]) -> int:
    return (words[1] >> 10) & 0x3F


def set_ocd1_threshold(words: list[int], value: int) -> None:
    words[1] = (words[1] & ~(0x3F << 2)) | ((value & 0x3F) << 2)


def set_ocd2_threshold(words: list[int], value: int) -> None:
    words[1] = (words[1] & ~(0x3F << 10)) | ((value & 0x3F) << 10)


def get_opmode(words: list[int]) -> int:
    return (words[0] >> 5) & 0x03


def decode_word(index: int, word: int) -> str:
    if index == 0:
        rng = word & 0x1F
        return (f"MEASRNG={rng:#04x} ({MEASRNG_NAMES.get(rng, '?')}), "
                f"OPMODE={(word >> 5) & 3} ({OPMODE_NAMES[(word >> 5) & 3]}), "
                f"OCD1DEGL={(word >> 7) & 7}, OCD2DEGL={(word >> 10) & 0xF}, "
                f"OCD1EN={(word >> 14) & 1}, OCD2EN={(word >> 15) & 1}")
    if index == 1:
        return (f"OCD1HYST={word & 3} ({HYST_NAMES[word & 3]}), "
                f"OCD1THRSH={(word >> 2) & 0x3F}, "
                f"OCD2HYST={(word >> 8) & 3} ({HYST_NAMES[(word >> 8) & 3]}), "
                f"OCD2THRSH={(word >> 10) & 0x3F}")
    if index == 2:
        vref = (word >> 8) & 7
        return (f"CRC={word & 0xFF:#04x}, VREFEXT={vref} ({VREFEXT_NAMES.get(vref, '?')}), "
                f"PolInv={(word >> 11) & 1}, QV1V5SD={(word >> 12) & 1}, "
                f"OCD1FONLY={(word >> 13) & 1}, RatioGain={(word >> 14) & 1}, "
                f"RatioOff={(word >> 15) & 1}")
    return "calibration coefficients - do not modify"


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

    def _drain(self, quiet: float = 0.05, deadline: float = 2.0) -> int:
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


# ---------------------------------------------------------------------------
# UI
# ---------------------------------------------------------------------------

class App:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title(f"TLE4972 - OCD Threshold Programming Example v{__version__}")
        self.root.geometry("1180x820")

        self.prg: Programmer | None = None
        self.image: list[int] | None = None      # image read from the sensor
        self.modified: list[int] | None = None   # image with pending changes
        self.busy = False
        self.log_queue: queue.Queue[str] = queue.Queue()
        self.ui_queue: queue.Queue = queue.Queue()

        self._build_ui()
        self.refresh_ports()
        self._pump()

    # -- layout ------------------------------------------------------------

    def _build_ui(self) -> None:
        warning = ttk.Label(
            self.root,
            text=("WARNING: the EEPROM CRC is never recalculated by the firmware or the sensor. "
                  "Programming an image with a wrong CRC drives the OCD outputs to GND permanently."),
            foreground="#a00000", wraplength=1140, justify="left")
        warning.pack(fill="x", padx=10, pady=(8, 4))

        # connection bar
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

        # actions
        actions = ttk.LabelFrame(self.root, text="Sequence")
        actions.pack(fill="x", padx=10, pady=4)

        steps = [
            ("1. Enter firmware mode", self.act_enter_firmware),
            ("2. Init MCU (mapping 0)", self.act_init_mcu),
            ("3. Test mode + read EEPROM", self.act_read_eeprom),
            ("4. Apply + recalc CRC", self.act_apply_changes),
            ("5. Program + verify", self.act_program_verify),
        ]
        self.action_buttons = []
        for col, (text, cmd) in enumerate(steps):
            btn = ttk.Button(actions, text=text, command=cmd, width=24)
            btn.grid(row=0, column=col, padx=4, pady=6)
            self.action_buttons.append(btn)

        ttk.Button(actions, text="Clear log", command=self.clear_log,
                   width=24).grid(row=1, column=0, padx=4, pady=(0, 6))

        # main split
        middle = ttk.Frame(self.root)
        middle.pack(fill="both", expand=True, padx=10, pady=4)

        mem = ttk.LabelFrame(middle, text="EEPROM image (18 x 16-bit words)")
        mem.pack(side="left", fill="both", expand=True)

        columns = ("addr", "word", "value", "new", "decoded")
        self.tree = ttk.Treeview(mem, columns=columns, show="headings", height=18)
        for name, title, width in (
            ("addr", "Addr", 60), ("word", "Word", 55),
            ("value", "Read", 70), ("new", "To write", 80), ("decoded", "Decoded", 470),
        ):
            self.tree.heading(name, text=title)
            self.tree.column(name, width=width, anchor="w")
        self.tree.tag_configure("changed", background="#fff2cc")
        self.tree.tag_configure("readonly", foreground="#777777")
        self.tree.pack(side="left", fill="both", expand=True, padx=(6, 0), pady=6)
        ttk.Scrollbar(mem, orient="vertical", command=self.tree.yview).pack(side="left", fill="y", pady=6)

        side = ttk.Frame(middle)
        side.pack(side="left", fill="y", padx=(10, 0))

        edit = ttk.LabelFrame(side, text="OCD thresholds (6-bit, 0..63)")
        edit.pack(fill="x")

        ttk.Label(edit, text="OCD1 threshold:").grid(row=0, column=0, sticky="w", padx=8, pady=(8, 2))
        self.ocd1_var = tk.IntVar(value=0)
        ttk.Spinbox(edit, from_=0, to=63, textvariable=self.ocd1_var,
                    width=8).grid(row=0, column=1, padx=8, pady=(8, 2))
        self.ocd1_cur = ttk.Label(edit, text="read: -")
        self.ocd1_cur.grid(row=1, column=0, columnspan=2, sticky="w", padx=8)

        ttk.Label(edit, text="OCD2 threshold:").grid(row=2, column=0, sticky="w", padx=8, pady=(8, 2))
        self.ocd2_var = tk.IntVar(value=0)
        ttk.Spinbox(edit, from_=0, to=63, textvariable=self.ocd2_var,
                    width=8).grid(row=2, column=1, padx=8, pady=(8, 2))
        self.ocd2_cur = ttk.Label(edit, text="read: -")
        self.ocd2_cur.grid(row=3, column=0, columnspan=2, sticky="w", padx=8, pady=(0, 8))

        crc = ttk.LabelFrame(side, text="CRC status")
        crc.pack(fill="x", pady=(10, 0))
        self.crc_read_var = tk.StringVar(value="read image     : -")
        self.crc_calc_var = tk.StringVar(value="calculated     : -")
        self.crc_new_var = tk.StringVar(value="image to write : -")
        self.crc_verify_var = tk.StringVar(value="verification   : -")
        for var in (self.crc_read_var, self.crc_calc_var, self.crc_new_var, self.crc_verify_var):
            ttk.Label(crc, textvariable=var, font=("Consolas", 9)).pack(anchor="w", padx=8, pady=2)

        info = ttk.LabelFrame(side, text="Device")
        info.pack(fill="x", pady=(10, 0))
        self.info_var = tk.StringVar(value="firmware : -\noutput   : -\nrange    : -")
        ttk.Label(info, textvariable=self.info_var, font=("Consolas", 9),
                  justify="left").pack(anchor="w", padx=8, pady=6)

        # log
        logbox = ttk.LabelFrame(self.root, text="Serial console")
        logbox.pack(fill="both", expand=True, padx=10, pady=(4, 10))
        self.log_text = tk.Text(logbox, height=14, font=("Consolas", 9), wrap="none")
        self.log_text.pack(side="left", fill="both", expand=True, padx=(6, 0), pady=6)
        scroll = ttk.Scrollbar(logbox, orient="vertical", command=self.log_text.yview)
        scroll.pack(side="left", fill="y", pady=6)
        self.log_text.configure(yscrollcommand=scroll.set)

    # -- logging -----------------------------------------------------------

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
        self.log(f"[host] opening {port} @ {BAUDRATE} 8N1, no flow control")
        self.log("[host] port open")

    # -- worker plumbing ---------------------------------------------------

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
            except ProtocolError as exc:
                self.log(f"!! protocol error: {exc}")
                text = str(exc)
                self.post(lambda: messagebox.showerror("Protocol error", text))
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

    # -- actions -----------------------------------------------------------

    def act_enter_firmware(self) -> None:
        def job():
            assert self.prg
            self.log("--- step 2/3: bootloader handshake ---")
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
            self.post(lambda: self._set_info(firmware=str(version)))
        self._run(job)

    def act_init_mcu(self) -> None:
        def job():
            assert self.prg
            self.log("--- step 4: init MCU, array config 0 (default mapping) ---")
            self.prg.init_mcu(self.sensor, 0)
            self.log("[host] MCU initialized, array config 0 (default mapping)")
        self._run(job)

    def act_read_eeprom(self) -> None:
        def job():
            assert self.prg
            self.log("--- steps 5+6: test mode and EEPROM read ---")
            self.prg.enter_test_mode(self.sensor)
            self.log(f"[host] sensor {self.sensor + 1} in SICI test mode, ISM powered down")
            words = self.prg.read_eeprom(self.sensor)
            self.post(lambda: self._load_image(words))
        self._run(job)

    def act_apply_changes(self) -> None:
        if self.busy:
            return
        if self.image is None:
            messagebox.showwarning("No image", "Read the EEPROM first.")
            return
        try:
            ocd1 = int(self.ocd1_var.get())
            ocd2 = int(self.ocd2_var.get())
        except tk.TclError:
            messagebox.showerror("Invalid value", "Thresholds must be integers 0..63.")
            return
        if not (0 <= ocd1 <= 63 and 0 <= ocd2 <= 63):
            messagebox.showerror("Invalid value", "Thresholds must be in range 0..63.")
            return

        words = list(self.image)
        old_word1 = words[1]
        set_ocd1_threshold(words, ocd1)
        set_ocd2_threshold(words, ocd2)
        old_crc = words[2] & 0xFF
        new_crc = patch_crc(words)

        self.log("--- steps 7+8: modify thresholds and recalculate CRC ---")
        self.log(f"[host] OCD1THRSH {get_ocd1_threshold(self.image)} -> {ocd1}, "
                 f"OCD2THRSH {get_ocd2_threshold(self.image)} -> {ocd2}")
        self.log(f"[host] word 1: 0x{old_word1:04X} -> 0x{words[1]:04X}")
        self.log(f"[host] CRC old = 0x{old_crc:02X}, CRC new = 0x{new_crc:02X}")
        self.log(f"[host] patching word 2: 0x{self.image[2]:04X} -> 0x{words[2]:04X}")
        self.log(f"[host] self-check: eeprom_crc_check() = {int(eeprom_crc_check(words))}")
        self.log("[host] image to program (8 bytes per line):")
        for line in hex_dump(words_to_bytes(words)):
            self.log(f"       {line}")

        self.modified = words
        self.crc_new_var.set(f"image to write : 0x{new_crc:02X}")
        self._refresh_tree()

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

    def act_program_verify(self) -> None:
        if self.modified is None:
            messagebox.showwarning("Nothing to program", "Apply the changes first.")
            return
        if self.modified == self.image:
            messagebox.showinfo("No change", "The modified image is identical to the read image.")
            return
        if not messagebox.askyesno(
                "Confirm programming",
                f"Program sensor {self.sensor_var.get()} EEPROM?\n\n"
                f"New CRC: 0x{self.modified[2] & 0xFF:02X}\n\n"
                "The EEPROM supports a limited number of programming cycles (max 100)."):
            return

        target = list(self.modified)

        def job():
            assert self.prg
            written_ok = False
            for cycle in range(1, PROGRAM_ATTEMPTS + 1):
                self.log(f"--- step 9: program EEPROM (burn cycle {cycle}) ---")
                self.prg.program_eeprom(self.sensor, target)
                self.log("[host] EEPROM programming sequence complete")

                self.log("--- step 10: verify the written content before resetting ---")
                written_ok, _ = self._verify_eeprom(target, "pre-reset read-back")
                if written_ok:
                    break
                if cycle < PROGRAM_ATTEMPTS:
                    self.log("[host] burnEEPROM() does not verify its own SICI transfers, "
                             "so a garbled word stays wrong - re-programming")
            if not written_ok:
                self.log("[host] PROGRAMMING FAILED - the sensor was NOT reset, so the old "
                         "content is still active")
                self.post(lambda: messagebox.showerror(
                    "Programming failed",
                    "The EEPROM read-back does not match what was written.\n\n"
                    "The sensor has NOT been reset, so the previous content is still "
                    "active. Reprogram before power-cycling it."))
                return

            self.log("--- step 11: power-cycle sensor ---")
            self.prg.reset_sensor(self.sensor)
            self.log("[host] sensor restarted")

            self.log("--- step 12: read back and verify ---")
            self.prg.enter_test_mode(self.sensor)
            match, readback = self._verify_eeprom(target, "post-reset read-back")
            crc_ok = eeprom_crc_check(readback)
            stored = readback[2] & 0xFF
            calculated = eeprom_crc(readback)

            self.log(f"[host] CRC stored = 0x{stored:02X}, CRC calculated = 0x{calculated:02X} "
                     f"-> {'VALID' if crc_ok else 'INVALID'}")
            self.log(f"[host] word 1 = 0x{readback[1]:04X} -> OCD1THRSH = "
                     f"{get_ocd1_threshold(readback)}, OCD2THRSH = {get_ocd2_threshold(readback)}")
            self.log("[host] PROGRAMMING SUCCESSFUL" if match and crc_ok
                     else "[host] PROGRAMMING FAILED")

            def update():
                self.crc_verify_var.set(
                    f"verification   : 0x{stored:02X} {'OK' if match and crc_ok else 'FAIL'}")
                self._load_image(readback)
                if match and crc_ok:
                    messagebox.showinfo("Success", "Read-back matches and the CRC is valid.")
                else:
                    messagebox.showerror("Verification failed",
                                         "Read-back does not match or the CRC is invalid.")
            self.post(update)
        self._run(job)

    # -- view helpers ------------------------------------------------------

    def _load_image(self, words: list[int]) -> None:
        self.image = list(words)
        self.modified = None
        stored = words[2] & 0xFF
        calculated = eeprom_crc(words)

        self.log("[host] EEPROM read OK (36 bytes)")
        for line in hex_dump(words_to_bytes(words)):
            self.log(f"       {line}")
        self.log(f"[host] CRC stored = 0x{stored:02X}, CRC calculated = 0x{calculated:02X} "
                 f"-> {'VALID' if stored == calculated else 'INVALID'}")

        self.ocd1_var.set(get_ocd1_threshold(words))
        self.ocd2_var.set(get_ocd2_threshold(words))
        self.ocd1_cur.config(text=f"read: {get_ocd1_threshold(words)}")
        self.ocd2_cur.config(text=f"read: {get_ocd2_threshold(words)}")
        self.crc_read_var.set(f"read image     : 0x{stored:02X}")
        self.crc_calc_var.set(f"calculated     : 0x{calculated:02X} "
                              f"{'OK' if stored == calculated else 'MISMATCH'}")
        self.crc_new_var.set("image to write : -")
        self._set_info(output=OPMODE_NAMES[get_opmode(words)],
                       measrng=MEASRNG_NAMES.get(words[0] & 0x1F, "?"))
        self._refresh_tree()

    def _set_info(self, firmware: str | None = None, output: str | None = None,
                  measrng: str | None = None) -> None:
        lines = dict(line.split(":", 1) for line in self.info_var.get().split("\n"))
        current = {k.strip(): v.strip() for k, v in lines.items()}
        if firmware is not None:
            current["firmware"] = firmware
        if output is not None:
            current["output"] = output
        if measrng is not None:
            current["range"] = measrng
        self.info_var.set("\n".join(f"{k:<9}: {v}" for k, v in current.items()))

    def _refresh_tree(self) -> None:
        self.tree.delete(*self.tree.get_children())
        if self.image is None:
            return
        for index, word in enumerate(self.image):
            new = self.modified[index] if self.modified else None
            changed = new is not None and new != word
            tags = ("changed",) if changed else (("readonly",) if index >= 3 else ())
            self.tree.insert("", "end", tags=tags, values=(
                f"0x{0x40 + index:02X}",
                index,
                f"0x{word:04X}",
                f"0x{new:04X}" if new is not None else "",
                decode_word(index, new if new is not None else word),
            ))

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
