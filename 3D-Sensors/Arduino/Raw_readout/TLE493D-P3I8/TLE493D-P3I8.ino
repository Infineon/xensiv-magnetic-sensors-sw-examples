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
/* plain SPI commands in master-controlled mode. The magnetic       */
/* settings are full range (160 mT).                                */
/* ---------------------------------------------------------------- */

#include "SPI.h"
int VDD_EN_LOW = 4;
int CSN = 3;                      // CSN pin
uint8_t buf[10];                  // buffer to store received values


void setup() {
  pinMode(CSN, OUTPUT);           // configure CSN
  digitalWrite(CSN, HIGH);
  pinMode(VDD_EN_LOW, OUTPUT);          
  digitalWrite(VDD_EN_LOW, LOW);

  Serial.begin(1000000);           // initializing the serial port
  while (!Serial);

  SPI.begin();                    // initialize the SPI port as master
  delay(100);  

  // configure sensor
  SPI.beginTransaction(SPISettings (1000000, MSBFIRST, SPI_MODE2));
  digitalWrite(CSN, LOW);      
    SPI.transfer(0x0A);           // write to register 0x0A   
    SPI.transfer(0xEA);           // [mod1] reset value is 0x62
    SPI.transfer(0x00);           // [mod2] reset value is 0x00
  SPI.endTransaction();     
  delayMicroseconds(5);           // to satisfy t_CSN_lag
  digitalWrite(CSN, HIGH);

  Serial.println("Bx, By, Bz, T");
  delay(10);
}

void loop() {    
  // Readout the last measurement data and trigger new measurement
  SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE2));
    digitalWrite(CSN, LOW);   
      SPI.transfer(0x80);         // sent read command starting from 0x00
      for(int i=0; i<0x0A; i++)         
      {
        buf[i] = SPI.transfer(0x00);// read the bytes holding the data from previous measurement
      }
      SPI.endTransaction();       // transaction over
      delayMicroseconds(5);       // to satisfy t_CSN_lag
    digitalWrite(CSN, HIGH); 

  // Built 14 bit data 
  int16_t X = (int16_t)((buf[0] << 8) | ((buf[1] & 0x3F) << 2)) >> 2;
  int16_t Y = (int16_t)((buf[2] << 8) | ((buf[3] & 0x3F) << 2)) >> 2;
  int16_t Z = (int16_t)((buf[4] << 8) | ((buf[5] & 0x3F) << 2)) >> 2;
  uint16_t T = (int16_t)((buf[6] << 8) | ((buf[7] & 0x3F) << 2)) >> 2;
  float   X_in_mT = float(X)/29.5;
  float   Y_in_mT = float(Y)/29.5;
  float   Z_in_mT = float(Z)/29.5;
  float T_in_degC = float(T-4200)/15.2 + 25;

  // send via serial port to be displayed on a terminal 
  Serial.print(X);         Serial.print("\t");
  Serial.print(Y);         Serial.print("\t");
  Serial.print(Z);         Serial.print("\t");
  Serial.print(T);         Serial.print("\t");
  Serial.print(X_in_mT);   Serial.print("\t");
  Serial.print(Y_in_mT);   Serial.print("\t");
  Serial.print(Z_in_mT);   Serial.print("\t");
  Serial.print(T_in_degC); Serial.print("\t");

  /* --------------------------------------------------------- */
  /* enter your application code here                          */
  /* --------------------------------------------------------- */

  // display the diag-byte in binary for debugging
  for (int i=7; i>=4; i--){Serial.print(buf[9]>>i & 0x01);}
  Serial.print(" ");
  for (int i=3; i>=0; i--){Serial.print(buf[9]>>i & 0x01);} 
  Serial.println();

  delay(100);  
}