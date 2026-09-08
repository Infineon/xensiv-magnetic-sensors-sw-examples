/*****************************************************************************
 *
 * Copyright (C) 2023 Infineon Technologies AG (INFINEON). All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/
 
/* ---------------------------------------------------------------- */
/* Very basic sketch to read out data from the TLI493D-W2B6 sensor  */
/* with the master controlled 1-byte-read mode triggering ADC on    */
/* read after 0x05, with clock stretching                           */
/* ---------------------------------------------------------------- */

#include <Wire.h>                 // default I²C library

#define ADDRESS 0x35              // for the A0 derivate

#define DELAY 100                 // value in ms, change to modify update rate

void setup() {
  pinMode(15, OUTPUT);            // the power pin
  digitalWrite(15, LOW); 
  delay(1000);
        
  // Init I2C peripheral
  pinMode(10, INPUT_PULLUP);      // to make sure the XMC's I2C does not break due to the      
  pinMode(11, INPUT_PULLUP);      // Interrupt triggered by the sensor, in it's default configuration.
  Wire.begin();
  Wire.setClock(400000);

  // Configure sensor
  Wire.beginTransmission(ADDRESS); // Sensor address
  Wire.write(0x10);                // Register address
  Wire.write(0x21);                
  Wire.write(0x15);                
  digitalWrite(15, HIGH);          //power on the sensor directly before config transmission, to not get stuck due to interrupt
  delayMicroseconds(10);
  Wire.endTransmission();
  
  // Init serial port for communication to the PC
  Serial.begin(115200);
  while (!Serial); delay(1000);
  Serial.println("Bx, By, Bz, T, debug");
}

void loop() {
  // Readout the first 7 data bytes
  uint8_t buf[7];
  Wire.requestFrom(ADDRESS, 7);
  for (uint8_t i = 0; i < 7; i++) {
    buf[i] = Wire.read();
  }

  // Built 12 bit data 
  int16_t X = (int16_t)((buf[0] << 8) | (buf[4] & 0xF0)) >> 4;
  int16_t Y = (int16_t)((buf[1] << 8) | ((buf[4] & 0x0F) << 4)) >> 4;
  int16_t Z = (int16_t)((buf[2] << 8) | ((buf[5] & 0x0F) << 4)) >> 4;
  uint16_t T = (buf[3] << 4) | (buf[5] >> 4);

  /* --------------------------------------------------------- */
  /* Enter your application code here*/
  /* --------------------------------------------------------- */
  
  // Send via serial port to be displayed on a terminal 
  Serial.print(X);  Serial.print("\t");
  Serial.print(Y);  Serial.print("\t");
  Serial.print(Z);  Serial.print("\t");
  Serial.print(T);  Serial.print("\t");
  for (int16_t i = 7; i>-1; i--){
    uint8_t plot = buf[6] >> i;
    Serial.print(plot & 0x01);
    if (i == 4){ Serial.print(" "); }
  }
  Serial.println();

  delay(DELAY);
}
