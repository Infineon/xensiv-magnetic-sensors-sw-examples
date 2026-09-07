/*****************************************************************************
 *
 * Copyright (C) 2024 Infineon Technologies AG (INFINEON). All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/
 
/* ---------------------------------------------------------------- */
/* Very basic sketch to read out data from the TLE493D-P3B6 with    */
/* plain I2C commands in master-controlled mode. The magnetic       */
/* settings are extra short-range range (50 mT).                    */
/* ---------------------------------------------------------------- */

#include <Wire.h>       // default I²C library
#define ADDRESS 0x5D    // for the P3B6
#define LSB2mT 118.0    // Sensitivity for extra short range
#define DELAY 50        // value in ms, change to modify update rate
#define VDD_pin 8       // pin to enable low for VDD_I2C
uint8_t buf[10];        // buffer to store received values


void setup() {
  pinMode(VDD_pin, OUTPUT);         // switch on VDD for sensor
  digitalWrite(VDD_pin, HIGH);  delay(100);
  digitalWrite(VDD_pin, LOW);

  Serial.begin(1000000);             // initializing the serial port
  while (!Serial);

  Wire.begin();                     // init I2C peripheral
  Wire.setClock(400000);
  delay(100);

  // configure sensor
  Wire.beginTransmission(ADDRESS);  // sensor address
  Wire.write(0x0A);                 // set pointer start to write to register 0x0A [mod1]
  Wire.write(0xC6);                 // [mod1] reset value is 0x62
  Wire.write(0x02);                 // [mod2] reset value is 0x00
  Wire.endTransmission();

  Serial.println("Bx, By, Bz, T | first in LSB_14 than in mT/°C, finishing with diagnosis register");
}

void loop() {    
  // readout the last measurement data and trigger new measurement
  Wire.requestFrom(ADDRESS, 10);
  for (uint8_t i = 0; i < 10; i++) {
    buf[i] = Wire.read();
  }

  // built 14 bit data 
  int16_t X = (int16_t)((buf[0] << 8) | ((buf[1] & 0x3F) << 2)) >> 2;
  int16_t Y = (int16_t)((buf[2] << 8) | ((buf[3] & 0x3F) << 2)) >> 2;
  int16_t Z = (int16_t)((buf[4] << 8) | ((buf[5] & 0x3F) << 2)) >> 2;
  uint16_t T = (uint16_t)((buf[6] << 8) | ((buf[7] & 0x3F) << 2)) >> 2;
  float   X_in_mT = float(X)/LSB2mT;
  float   Y_in_mT = float(Y)/LSB2mT;
  float   Z_in_mT = float(Z)/LSB2mT;
  float T_in_degC = float(T-4200)/15.2 + 25;

  /* --------------------------------------------------------- */
  /* enter your application code here                          */
  /* --------------------------------------------------------- */

  // send via serial port to be displayed on a terminal 
  Serial.print(X);         Serial.print("\t");
  Serial.print(Y);         Serial.print("\t");
  Serial.print(Z);         Serial.print("\t");
  Serial.print(T);         Serial.print("\t");
  Serial.print(X_in_mT);   Serial.print("\t");
  Serial.print(Y_in_mT);   Serial.print("\t");
  Serial.print(Z_in_mT);   Serial.print("\t");
  Serial.print(T_in_degC); Serial.print("\t");

  // display the diag-byte in binary for debugging
  for (int i=7; i>=4; i--){Serial.print(buf[9]>>i & 0x01);}
  Serial.print(" ");
  for (int i=3; i>=0; i--){Serial.print(buf[9]>>i & 0x01);} 
  Serial.println();

  delay(DELAY);  
}