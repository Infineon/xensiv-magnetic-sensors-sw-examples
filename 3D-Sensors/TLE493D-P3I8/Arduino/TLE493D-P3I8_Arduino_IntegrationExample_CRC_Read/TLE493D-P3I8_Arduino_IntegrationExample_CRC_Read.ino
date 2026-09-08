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

/******************************************************************************
 * Place a magnet on top of the sensor to exceed the wake-up threshold!
 ******************************************************************************/

#include "SPI.h"
int CSN = 3;                      // CSN pin
int INT_PIN = 9;                  // INT pin
volatile uint8_t buf[6];          // buffer to store received values
volatile bool interrupt_flag = 0; // interrupt flag indicating an interrupt event

void setup() {
  pinMode(CSN, OUTPUT);           // configure CSN
  digitalWrite(CSN, HIGH);
  pinMode(INT_PIN, INPUT);        // configure INT

  Serial.begin(115200);           // initializing the serial port
  while (!Serial); 

  SPI.begin(); delay(1000);       // initialize the SPI port as master

  // configure sensor
  SPI.beginTransaction(SPISettings (1000000, MSBFIRST, SPI_MODE2)); //1MHz sck freq
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

  // link the interrupt function
  attachInterrupt(digitalPinToInterrupt(INT_PIN), interruptFunction, FALLING);
  delay(10);
}

// below function calculates the CRC-8 value for a given data array and its length
uint8_t crc8 (uint8_t *data, uint8_t length) { 
  uint8_t CRC_POLY = 0x2F;        // polynomial : x8+x5+x3+x2+x+1  
  uint8_t i, bit;
  uint8_t crc = 0x00;             // initially the CRC remainder has to be set with the original seed
  for (i = 0; i < length; i++) {
    crc = crc ^ data[i]; 
    // crc is calculated as the XOR operation from the previous crc and data for each bit position in a 8-bit word
    for (bit = 0; bit < 8; bit++) {
      // if the MSB of the crc is 1 (with the & 0x80 mask we get the MSB of the crc).
      if (crc & 0x80) {
        // crc advances on position (crc is moved left 1 bit: the MSB is deleted since it will be cancelled out with the first one of the generator polynomial and a new bit 
        // from the message is taken as LSB.) 
        crc = crc << 1;       
        // crc is calculated as the XOR operation from the previous crc and the generator polynomial (0x2F). Be aware that here the x8 bit is not taken since 
        // the MSB of the crc already has been deleted in the previous step
        crc = crc ^ CRC_POLY;
      } else { // In case the crc MSB is 0
        // crc advances one position (this step is to ensure that the XOR operation is only 
        // done when the generator polynomial is aligned with a MSB of the message that is 1.
        crc = crc << 1;
      }
    }
  }
  return crc;  // return the crc remainder
}

void interruptFunction() {
  interrupt_flag = 1;
}

void loop() {
  if (interrupt_flag){
    interrupt_flag = 0;
    // readout the last measurement data
    SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE2));
      digitalWrite(CSN, LOW);    
        SPI.transfer(0x84);         // sent read command starting from register 0x04
        Serial.println("bz_msbs, bz_lsbs, t_msbs, t_lsbs, CRC, diag: ");
        for(int i=0; i<6; i++)      // read the bytes holding the data
        {                           // from previous measurement
          buf[i] = SPI.transfer(0x00);
          Serial.print(buf[i], HEX);// stream the raw data
          Serial.print(", ");
        }
      SPI.endTransaction();         // here the MOSI and CLK line are triggered
      delayMicroseconds(5);         // to satisfy t_CSN_lag
    digitalWrite(CSN, HIGH);
    Serial.println(); Serial.println();

    // built 14 bit data 
    int16_t  Z = (int16_t)((buf[0] << 8) | ((buf[1] & 0x3F) << 2)) >> 2;
    uint16_t T = (int16_t)((buf[2] << 8) | ((buf[3] & 0x3F) << 2)) >> 2;
    float   Z_in_mT = float(Z)/118.0;
    float T_in_degC = float(T-4200)/15.2 + 25;

    // send via serial port to be displayed on a terminal 
    Serial.println("Bz in LSB_14, T in LSB, Bz in mT, T in °C: ");
    Serial.print(Z);         Serial.print(",\t");
    Serial.print(T);         Serial.print(",\t");
    // values of Bz(in mT) and Temperature(in Celsius)
    Serial.print(Z_in_mT);   Serial.print("mT,\t");
    Serial.print(T_in_degC); Serial.println("°C");
    Serial.println();

    // display the received CRC message
    Serial.println("Received CRC message in HEX: ");
    Serial.println(buf[4], HEX);   
    Serial.println();

    // display the diag-byte in binary for debugging
    Serial.println("Diagnosis Register value: ");
    for (int i=7; i>=4; i--){Serial.print(buf[5]>>i & 0x01);}
    Serial.print(" ");
    for (int i=3; i>=0; i--){Serial.print(buf[5]>>i & 0x01);} 
    Serial.println(); Serial.println();
    
    // compute the CRC
    uint8_t data[7];  // construct the data over which the crc will be computed
    data[0] = buf[0]; // z_msbs
    data[1] = buf[1]; // z_lsbs
    data[2] = buf[2]; // temp_msbs
    data[3] = buf[3]; // temp_lsbs
    data[4] = buf[5]; // diag reg
    data[5] = 0x32;   // mod1 reg
    data[6] = 0xEE;   // mod2 reg
    uint8_t computedCRC = crc8(data, 7); //compute the crc over the 7 bytes inclucing bz_msb bz_lsb temp_msb temp_lsb diag mod1 mod2
    Serial.println("Computed CRC in HEX: ");
    Serial.println(computedCRC, HEX);  
    Serial.println();
    Serial.println();
    Serial.println();
  }
}
