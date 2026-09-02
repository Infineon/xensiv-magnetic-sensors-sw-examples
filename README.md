<img src="doc/IFX_LOGO_600.png" align="right" width="150" alt="Infineon" />

# XENSIV™ Sensors – Software Examples

This repository contains software examples for the XENSIV™ magnetic sensors.

Examples are organized as **`<Sensor-Family>/<Sensor>/<Platform>/<Example>`**.

<details open>
<summary><b>Overview</b> — 36 examples across 6 sensor families</summary>

| Family | Sensors |
|---|---|
| [3D Magnetic Sensors](#3d-magnetic-sensors) | TLE493D-A2B6, TLE493D-P2B6, TLE493D-P3B6, TLE493D-P3I8, TLE493D-W2B6, TLI493D-W2B6, TLI493D-W2BW |
| [Angle Sensors](#angle-sensors) | TLE49012 / TLI49012, TLE5012, TLE5501, TLE5502 |
| [Current Sensors](#current-sensors) | TLE4972 |
| [Linear Sensors](#linear-sensors) | TLE4998S4 |
| [Pressure Sensors](#pressure-sensors) | KP215F1701, KP467 |
| [Speed Sensors](#speed-sensors) | TLE4922 |

</details>

---

## 3D Magnetic Sensors

<details>
<summary><b>Show 13 examples</b></summary>

| Sensor | Platform | Example | Description |
|---|---|---|---|
| [TLE493D-A2B6](3D-Sensors/TLE493D-A2B6) | Arduino | [Raw readout](3D-Sensors/TLE493D-A2B6/Arduino/Raw_readout/TLE493D-A2B6) | I²C readout on Shield2Go using the Wire library, master-controlled 1-byte-read mode |
| [TLE493D-P2B6](3D-Sensors/TLE493D-P2B6) | Linux | [Raw readout](3D-Sensors/TLE493D-P2B6/Linux) | Bx/By/Bz and temperature over I²C from Linux user space (Raspberry Pi) |
| [TLE493D-P3B6](3D-Sensors/TLE493D-P3B6) | Arduino | [Raw readout](3D-Sensors/TLE493D-P3B6/Arduino/Raw_readout/TLE493D-P3B6) | I²C readout on Shield2Go / 2Go Kit in master-controlled mode |
| [TLE493D-P3B6](3D-Sensors/TLE493D-P3B6) | Arduino | [Bx, Z and T only](3D-Sensors/TLE493D-P3B6/Arduino/Raw_readout/TLE493D-P3B6-W3B6-Bx_Z_and_T_only) | Reduced readout of Bx, Bz and temperature — also applicable to the W3B6 variant |
| [TLE493D-P3I8](3D-Sensors/TLE493D-P3I8) | Arduino | [Raw readout](3D-Sensors/TLE493D-P3I8/Arduino/Raw_readout/TLE493D-P3I8) | SPI readout in the full range setting (±160 mT) |
| [TLE493D-P3I8](3D-Sensors/TLE493D-P3I8) | Arduino | [Wake-up on Z](3D-Sensors/TLE493D-P3I8/Arduino/Raw_readout/TLE493D-P3I8_Wake_Up_on_Z) | Interrupt / wake-up on Z-axis thresholds at 16 Hz update rate |
| [TLE493D-P3I8](3D-Sensors/TLE493D-P3I8) | Arduino | [CRC at read](3D-Sensors/TLE493D-P3I8/Arduino/Raw_readout/TLE493D_P3I8_CRC_at_read) | SPI readout with CRC verification of the received frame |
| [TLE493D-W2B6](3D-Sensors/TLE493D-W2B6) | Arduino | [DrillTriggerV2 add-on](3D-Sensors/TLE493D-W2B6/Arduino/AddOns/DrillTriggerV2) | Drill trigger add-on mounted on the 2Go Kit |
| [TLI493D-W2B6](3D-Sensors/TLI493D-W2B6) | Arduino | [Raw readout](3D-Sensors/TLI493D-W2B6/Arduino/Raw_readout/TLI493D-W2B6) | I²C readout on Shield2Go, master-controlled 1-byte-read mode |
| [TLI493D-W2B6](3D-Sensors/TLI493D-W2B6) | Arduino | [Training template](3D-Sensors/TLI493D-W2B6/Arduino/Training/Template) | Skeleton sketch based on the TLx493D library, used as a training starting point |
| [TLI493D-W2BW](3D-Sensors/TLI493D-W2BW) | Arduino | [Raw readout](3D-Sensors/TLI493D-W2BW/Arduino/Raw_readout/TLI493D-W2BW) | I²C readout on Shield2Go using the Wire library |
| [TLI493D-W2BW](3D-Sensors/TLI493D-W2BW) | Arduino | [JoystickBasic add-on](3D-Sensors/TLI493D-W2BW/Arduino/AddOns/JoystickBasic) | Bx/By/Bz interpreted as joystick input with the Play2Go add-on on XMC 2Go |
| [TLI493D-W2BW](3D-Sensors/TLI493D-W2BW) | Python | [Spindle movement](3D-Sensors/TLI493D-W2BW/Python/Spindle%20Movement) | Spindle position measurement — Arduino firmware plus Python host script |

</details>

---

## Angle Sensors

<details>
<summary><b>Show 16 examples</b></summary>

| Sensor | Platform | Example | Description |
|---|---|---|---|
| [TLE49012 / TLI49012](Angle-Sensors/TLE49012-TLI49012) | Arduino | [SPI in-frame read/write](Angle-Sensors/TLE49012-TLI49012/Arduino/TLE49012_TLI49012_Arduino_IntegrationExample_SPI_InFrameRW) | SPI access where the read data is returned within the same frame |
| [TLE49012 / TLI49012](Angle-Sensors/TLE49012-TLI49012) | Arduino | [SPI next-frame read/write](Angle-Sensors/TLE49012-TLI49012/Arduino/TLE49012_TLI49012_Arduino_IntegrationExample_SPI_NextFrameRW) | SPI access where the read data is returned in the following frame |
| [TLE49012 / TLI49012](Angle-Sensors/TLE49012-TLI49012) | AURIX™ | [TC375 integration](Angle-Sensors/TLE49012-TLI49012/Aurix/TLE49012_TLI49012_TC375_IntegrationExample) | SPI integration with the TC375 AURIX™ MCU |
| [TLE49012 / TLI49012](Angle-Sensors/TLE49012-TLI49012) | MOTIX™ | [TLE987x integration](Angle-Sensors/TLE49012-TLI49012/Motix/TLE49012_TLI49012_TLE987x_IntegrationExample) | SPI integration with the MOTIX™ TLE987x MCU family |
| [TLE49012 / TLI49012](Angle-Sensors/TLE49012-TLI49012) | PSOC™ | [PSC3M5 integration](Angle-Sensors/TLE49012-TLI49012/PSOC/TLE49012_TLI49012_PSC3M5_IntegrationExample) | Single-sensor SPI integration on the PSOC™ Control C3M5 control card |
| [TLE49012 / TLI49012](Angle-Sensors/TLE49012-TLI49012) | PSOC™ | [PSC3M5 two-sensor sync](Angle-Sensors/TLE49012-TLI49012/PSOC/TLE49012_TLI49012_PSC3M5_IntegrationExample_TwoSensorSync) | Synchronized readout of two sensors over SPI |
| [TLE49012 / TLI49012](Angle-Sensors/TLE49012-TLI49012) | PSOC™ | [PSC3M5 two-sensor sync with DMA](Angle-Sensors/TLE49012-TLI49012/PSOC/TLE49012_TLI49012_PSC3M5_IntegrationExample_TwoSensorSync_DMA) | Synchronized two-sensor readout offloaded to DMA |
| [TLE49012 / TLI49012](Angle-Sensors/TLE49012-TLI49012) | Documentation | [Evaluation kit serial commands interface](Angle-Sensors/TLE49012-TLI49012/Documentation/TLE49012_TLI49012_EvaluationKit_SerialCommandsInterface) | Serial command protocol reference for the evaluation kit |
| [TLE5012](Angle-Sensors/TLE5012) | MOTIX™ | [TLE987x integration (SSC)](Angle-Sensors/TLE5012/Motix/TLE5012_TLE987x_IntegrationExample) | TLE5012B steering-angle measurement over SSC 3-wire SPI with TLE9879 |
| [TLE5012](Angle-Sensors/TLE5012) | MOTIX™ | [TLE987x integration (PWM)](Angle-Sensors/TLE5012/Motix/TLE5012_TLE987x_IntegrationExample/PWM) | Angle derived from the PWM duty cycle (6.25 %–93.5 % ↔ 0°–360°) |
| [TLE5501](Angle-Sensors/TLE5501) | PSOC™ | [PSC3M5 integration](Angle-Sensors/TLE5501/PSOC/TLE5501_PSC3M5_IntegrationExample) | TMR angle sensor with the PSOC™ Control C3 MCU |
| [TLE5502](Angle-Sensors/TLE5502) | AURIX™ | [TC297 sync readout, one-time calibration](Angle-Sensors/TLE5502/Aurix/TLE5502_TC297_IntegrationExample_SyncReadout/TC297_CalibOneTime_ParallelConversions) | Parallel VADC conversions across 4 groups, CCU6-triggered, one-time calibration |
| [TLE5502](Angle-Sensors/TLE5502) | AURIX™ | [TC297 sync readout, on-go calibration](Angle-Sensors/TLE5502/Aurix/TLE5502_TC297_IntegrationExample_SyncReadout/TC297_CalibOnGo_Parallel%20Conversions) | Same synchronous readout with continuous on-the-fly calibration |
| [TLE5502](Angle-Sensors/TLE5502) | AURIX™ | [TC334, one-time calibration](Angle-Sensors/TLE5502/Aurix/TLE5502_TC334_IntegrationExample/TLE5502_AurixCalibOneTime) | Readout on KIT_A2G_TC334_LITE with one-time calibration |
| [TLE5502](Angle-Sensors/TLE5502) | AURIX™ | [TC334, on-go calibration](Angle-Sensors/TLE5502/Aurix/TLE5502_TC334_IntegrationExample/TLE5502_AurixCalibOnGo) | Readout on KIT_A2G_TC334_LITE with continuous calibration |
| [TLE5502](Angle-Sensors/TLE5502) | MOTIX™ | [TLE987x integration](Angle-Sensors/TLE5502/Motix/TLE5502_TLE987x_IntegrationExample) | Sine/cosine differential pairs sampled with the on-chip ADC1 |

</details>

---

## Current Sensors

<details>
<summary><b>Show 2 examples</b></summary>

| Sensor | Platform | Example | Description |
|---|---|---|---|
| [TLE4972](Current-Sensors/TLE4972) | Arduino | [SICI interface example](Current-Sensors/TLE4972/Arduino) | Register and EEPROM access over the single-wire SICI interface on the AOUT pin |
| [TLE4972](Current-Sensors/TLE4972) | Python | [Serial commands examples](Current-Sensors/TLE4972/Python/TLE4972_Serial_Commands_Examples) | Programmer serial command interface — DCW calibration and OCD threshold programming |

</details>

---

## Linear Sensors

<details>
<summary><b>Show 1 example</b></summary>

| Sensor | Platform | Example | Description |
|---|---|---|---|
| [TLE4998S4](Linear-Sensors/TLE4998S4) | AURIX™ | [TC277 SENT readout](Linear-Sensors/TLE4998S4/Aurix) | Decoding SENT frames from the sensor on KIT_AURIX_TC277_TRB |

</details>

---

## Pressure Sensors

<details>
<summary><b>Show 2 examples</b></summary>

| Sensor | Platform | Example | Description |
|---|---|---|---|
| [KP215F1701](Pressure-Sensors/KP2151701) | PSOC™ | [PSOC™ 4 analog readout](Pressure-Sensors/KP2151701/PSOC) | MAP sensor sampled with the SAR ADC on CY8CKIT-149, streamed over UART |
| [KP467](Pressure-Sensors/KP467) | PSOC™ | [PSoC™ 6 SPI readout and LPM demo](Pressure-Sensors/KP467/PSOC) | 10/12/14-bit readout plus a low-power monitoring wake-up demonstration |

</details>

---

## Speed Sensors

<details>
<summary><b>Show 2 examples</b></summary>

| Sensor | Platform | Example | Description |
|---|---|---|---|
| [TLE4922](Speed-Sensors/TLE4922) | Arduino | [Tooth wheel speed measurement](Speed-Sensors/TLE4922/Arduino) | Instantaneous speed, average speed and RPM of a rotating wheel |
| [TLE4922](Speed-Sensors/TLE4922) | PSOC™ | [Tooth wheel speed measurement](Speed-Sensors/TLE4922/PSOC) | PWM edge interrupts on PSoC™ 6, with average speed computed when the wheel stops |

</details>

---

> **Note:** the provided example code is not a qualified solution and is provided "as-is".