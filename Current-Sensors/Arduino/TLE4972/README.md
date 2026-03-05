# TLE4972 – SICI interface example (Arduino Uno)

This example demonstrates how to communicate with the Infineon TLE4972 current sensor using the SICI (Serial Inspection and Configuration Interface) through the AOUT pin. The SICI interface allows access to internal registers and EEPROM of the device using a single-wire bidirectional communication protocol.

The Arduino sketch implements the following sequence:
1.	Enter SICI interface mode using the password 0xABCD
2.	Disable the Internal State Machine (ISM)
3.	Read EEPROM addresses 0x40 to 0x51
4.	Re-enable the ISM
5.	Exit the SICI interface mode

The code uses precise microsecond timing to generate the duty-cycle encoded SICI communication protocol and to read back responses from the sensor.

## Hardware Requirements

- Arduino Uno (or any compatible Arduino board)
- Infineon TLE4972 current sensor
- Pull-up resistor: 2.2 kΩ to 3.3 V on the AOUT line 
- Sensor supply controlled through a VSENS/LDO_EN pin

## Hardware Connection

| Signal | Arduino pin | TLE4972 pin / node | Notes |
|---|---:|---|---|
| SICI data | A0 | AOUT |  |
| Pull-up | — | AOUT | Add 2.2 kΩ pull-up to 3.3 V (AOUT is used as a single-wire bus during SICI). |
| Sensor enable | D12 | VSENS / LDO_EN | HIGH = sensor on, LOW = sensor off (per your wiring). |
| Supply | 5V, GND | sensor supply network |

### Important notes:

- The AOUT pin acts as a bidirectional communication line during SICI.
- The Arduino must drive the line LOW or release it (high-impedance).
- Never drive the line HIGH directly because the sensor may also drive it.

## Software Requirements

- Arduino IDE
- No additional libraries are required

The sketch relies on the standard Arduino functions:
- pinMode()
- digitalWrite()
- digitalRead()
- delayMicroseconds()

## Using the code example

1. Upload the sketch to the Arduino board. 
2. Open the Serial Monitor.
3. Set baud rate to 115200.
4. Reset the board or restart the sketch.

The serial output will show:
- Password transmission result
- ISM disable command
- EEPROM contents
- ISM enable command
- Interface exit confirmation

## Related resources

Resources  | Links
-----------|----------------------------------
Data sheet | [Infineon-tle4972-ae35s5-datasheet](https://www.infineon.com/assets/row/public/documents/24/49/infineon-tle4972-ae35s5-datasheet-en.pdf) <br> 
User manual | Viewing the user manuals requires a registered myInfineon account: <br> [Infineon-tle4972-usermanual](https://www.infineon.com/assets/row/public/documents/24/44/infineon-tle4972-usermanual-en.pdf) <br>
Code examples  | [Using ModusToolbox&trade;](https://github.com/Infineon/Code-Examples-for-ModusToolbox-Software) on GitHub <br> [Based on Infineon sensors](https://github.com/Infineon/xensiv-magnetic-sensors-sw-examples) on GitHub
Development kits | Select your kits from the [Evaluation board finder](https://www.infineon.com/cms/en/design-support/finder-selection-tools/product-finder/evaluation-board)

## Other resources

Infineon provides a wealth of data at [www.infineon.com](https://www.infineon.com) to help you select the right device, and quickly and effectively integrate it into your design.


## Document history

Document title: *TLE4972 – SICI interface example (Arduino Uno)*

 Version | Description of change
 ------- | ---------------------
 1.0.0   | New code example

<br>


All referenced product or service names and trademarks are the property of their respective owners.

The Bluetooth&reg; word mark and logos are registered trademarks owned by Bluetooth SIG, Inc., and any use of such marks by Infineon is under license.

PSOC&trade;, formerly known as PSoC&trade;, is a trademark of Infineon Technologies. Any references to PSoC&trade; in this document or others shall be deemed to refer to PSOC&trade;.

---------------------------------------------------------

(c) 2025, Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
This software, associated documentation and materials ("Software") is owned by Infineon Technologies AG or one of its affiliates ("Infineon") and is protected by and subject to worldwide patent protection, worldwide copyright laws, and international treaty provisions. Therefore, you may use this Software only as provided in the license agreement accompanying the software package from which you obtained this Software. If no license agreement applies, then any use, reproduction, modification, translation, or compilation of this Software is prohibited without the express written permission of Infineon.
<br>
Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A SPECIFIC USE/PURPOSE OR MERCHANTABILITY. Infineon reserves the right to make changes to the Software without notice. You are responsible for properly designing, programming, and testing the functionality and safety of your intended application of the Software, as well as complying with any legal requirements related to its use. Infineon does not guarantee that the Software will be free from intrusion, data theft or loss, or other breaches (“Security Breaches”), and Infineon shall have no liability arising out of any Security Breaches. Unless otherwise explicitly approved by Infineon, the Software may not be used in any application where a failure of the Product or any consequences of the use thereof can reasonably be expected to result in personal injury.
