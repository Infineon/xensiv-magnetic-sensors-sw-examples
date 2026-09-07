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
/* Very basic sketch to read out data from the TLE493D-P3I8 with    */
/* plain SPI commands with the Wake Up / Interrupt functionality    */
/* on Z only if Z >= 63 LSB14 or Z <= -129 LSB14 at the update      */
/* frequency of 16 Hz.                                              */
/* ---------------------------------------------------------------- */

#include "SPI.h"
int CSN = 3;                      // CSN pin
int INT_PIN = 9;                  // INT pin
int VDD_EN_LOW = 4;
volatile uint8_t buf[6];          // buffer to store received values

void setup() {
  pinMode(CSN, OUTPUT);           // configure CSN
  digitalWrite(CSN, HIGH);
  pinMode(INT_PIN, INPUT);        // configure INT
  pinMode(VDD_EN_LOW, OUTPUT);          
  digitalWrite(VDD_EN_LOW, LOW);

  Serial.begin(1000000);           // initializing the serial port
  while (!Serial); 

  SPI.begin(); delay(100);        // initialize the SPI port as master

  // configure sensor
  SPI.beginTransaction(SPISettings (1000000, MSBFIRST, SPI_MODE2));
    digitalWrite(CSN, LOW);
      SPI.transfer(0x0A);         // set pointer start to write to register 0x0A [mod1]
      SPI.transfer(0x32);         // [mod1] reset value is 0x62
      SPI.transfer(0xEE);         // [mod2] reset value is 0x00
      SPI.transfer(0x7F);         // [wu_xh_msbs] reset value is 0x7F
      SPI.transfer(0x80);         // [wu_xl_msbs] reset value is 0x80
      SPI.transfer(0x7F);         // [wu_yh_msbs] reset value is 0x7F
      SPI.transfer(0x80);         // [wu_yl_msbs] reset value is 0x80
      SPI.transfer(0x00);         // [wu_zh_msbs] reset value is 0x7F
      SPI.transfer(0xFE);         // [wu_zl_msbs] reset value is 0x80
    SPI.endTransaction();         // here the MOSI and CLK line are triggered
    delayMicroseconds(5);         // to satisfy t_CSN_lag
  digitalWrite(CSN, HIGH);
  Serial.println("Bz in LSB_14, T in LSB, Bz in mT, T in °C, diag");

  // link the interrupt function
  attachInterrupt(digitalPinToInterrupt(INT_PIN), interruptFunction, FALLING);
  delay(10);
}

void interruptFunction() {
  // readout the last measurement data
  SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE2));
    digitalWrite(CSN, LOW);    
      SPI.transfer(0x84);         // sent read command starting from register 0x04
      for(int i=0; i<6; i++)      // read the bytes holding the data
      {                           // from previous measurement
        buf[i] = SPI.transfer(0x00);
      }
    SPI.endTransaction();         // here the MOSI and CLK line are triggered
    delayMicroseconds(5);         // to satisfy t_CSN_lag
  digitalWrite(CSN, HIGH);

  // built 14 bit data 
  int16_t  Z = (int16_t)((buf[0] << 8) | ((buf[1] & 0x3F) << 2)) >> 2;
  uint16_t T = (int16_t)((buf[2] << 8) | ((buf[3] & 0x3F) << 2)) >> 2;
  float   Z_in_mT = float(Z)/118.0;
  float T_in_degC = float(T-4200)/15.2 + 25;

  // send via serial port to be displayed on a terminal 
  Serial.print(Z);         Serial.print("\t");
  Serial.print(T);         Serial.print("\t");
  Serial.print(Z_in_mT);   Serial.print("\t");
  Serial.print(T_in_degC); Serial.print("\t");

  /* --------------------------------------------------------- */
  /* enter your application code here                          */
  /* --------------------------------------------------------- */

  // display the diag-byte in binary for debugging
  for (int i=7; i>=4; i--){Serial.print(buf[5]>>i & 0x01);}
  Serial.print(" ");
  for (int i=3; i>=0; i--){Serial.print(buf[5]>>i & 0x01);} 
  Serial.println();
}

void loop() {
  delay(10);
}
