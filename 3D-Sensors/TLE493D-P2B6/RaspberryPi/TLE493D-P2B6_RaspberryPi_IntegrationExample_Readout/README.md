# 3D Magnetic Sensor in Linux environment

This code example demonstrates how to interface the 3D magnetic sensor [TLE493D-P2B6](https://www.infineon.com/cms/en/product/sensor/magnetic-sensors/magnetic-position-sensors/3d-magnetics/tle493d-p2b6-a0/) in Linux environment. The sensor uses I2C communication protocol. The 3D magnetic field Bx, By and Bz along with Temperature are read. The user can extend this example to their end-application.

For this example we are using Raspberry Pi.





## Pre-requisites

- Connect the VDD, GND, SCL and SDA of the TLE493D-P2B6-A0 sensor and Raspberry Pi(or any Linux based microcontroller).
- Clone this repository or download the .zip file of this repository. 
- Change directory to *...\xensiv-magnetic-sensors-sw-examples\3D-Sensors\TLE493D-P2B6\RaspberryPi\TLE493D-P2B6_RaspberryPi_IntegrationExample_Readout* folder in terminal.
- In terminal we have to create the executable for which you can enter the following command: **gcc -o executable_name tle493d-p2b6.c**
- Once the execuable is generated, you can run the code in terminal: **./executable_name**




