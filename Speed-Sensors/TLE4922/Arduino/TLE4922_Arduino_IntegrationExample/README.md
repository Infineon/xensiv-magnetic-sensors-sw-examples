# Arduino: Tooth Wheel Speed Measurement

## Overview

The project implements a wheel speed sensor using a Arduino microcontroller. The system calculates the instantaneous speed, average speed, and RPM of a rotating wheel.

## Hardware Requirements

- Arduino Uno
- Hall Effect Sensor TLE4922
- Programming language: C
- Pull-up resistor and Jumpers

## Hardware setup

Use jumper wires to establish a connection between TLE4922 Speed sensor and the Arduino Uno as mentioned below.

1. **VCC** : Connect to the **3.3V** supply on the Arduino kit.
2. **GND** : Connect to the **GND** on the Arduino kit.
3. **Output** : Connect to an input pin on Arduino with a pull-up resistor to 3.3V.

## Design and implementation

This code example demonstrates how to measure the speed of a rotating tooth wheel using the TLE4922 speed sensor and Arduino. The sensor outputs a PWM signal corresponding to the wheel rotation, which is read by the controller. An interrupt is triggered on every rising edge of the PWM signal, allowing the calculation of the instantaneous speed based on the known pitch length of the wheel. Additionally, the system calculates the average speed when the wheel stops, defined as no pulse detected for 2 seconds.


