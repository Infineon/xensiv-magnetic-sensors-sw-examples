/**
 * @file TLx49012_Arduino_SPI_Next_Frame_Read_Write_Example_Code.ino
 * @date 2026-02-25
 *
 * @cond
 *********************************************************************************************************************
 * TLx49012 SPI NEXT FRAME READ & WRITE EXAMPLE CODE for Arduino v1.0.0
 *
 * Copyright (c) 2026, Infineon Technologies AG
 * All rights reserved.                        
 *                                             
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the 
 * following conditions are met:   
 *                                                                              
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer.                        
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
 * disclaimer in the documentation and/or other materials provided with the distribution.                       
 * 
 * Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote 
 * products derived from this software without specific prior written permission.                                           
 *                                                                              
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE  
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR  
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                  
 *                                                    
 *********************************************************************************************************************
 *
 * Change History
 * --------------
 *
 * 2026-02-25:
 *     - Initial
 * 
 * @endcond 
 *
 */

/**
 * !!The user must ensure the correct connection between the sensor and the arduino development board!!
 * TLx49012 - Supply voltage (VDD) VDD 3.0 – 5.5 V
 *
 * (SENSOR) -> (MCU)
 * VDD      -> 5V/3.3V
 * GND      -> GND
 * IFA      -> CSN
 * IFB      -> SCK
 * IFC      -> MOSI / COPI
 * IFD      -> MISO / CIPO
 * IFE      -> NC
 * VPROG    -> NC
 */

/**
 *  Includes
 */
#include "SPI.h"

/**
 *  Pin Definition
 *  Define only Chip Select. SPI.begin() initializes all the default SPI pins
 */
#define PIN_CSN   10 // SS

/**
 *  Register definition
 */

// CRC BITMAP DISABLE
#define STAT_EN_1_REG	  4
#define CRC_BM_DIS		  0x7E18

// UNLOCK
#define UNLOCK_REG		  120
#define USR_PASS		    0x4711

// PREDICTED ANGLE
#define ADDR_ANGLE_PRED 0x0C

// STATUS / RESET REGISTER
#define STAT_EN_REG     2
#define VAL_SOFT_RESET_VM_DATA 	  0x8E82
#define VAL_SOFT_RESET_NVM_DATA   0x8E81

// USR_CONFIG_7_REG - angle_base + angle_dir
#define USR_CONFIG_7_REG 69

// USER CONFIGURATION AREA
#define USR_CONFIG_START_ADDRESS  63 
#define USR_CONFIG_STOP_ADDRESS   77

// SPI NEXT FRAME COMMANDS
#define CMD_ADDR		  0xFF
#define CMD_DATA_ADDR	0xFD

// SPI NEXT FRAME REGISTER ACCESS TYPE
#define AC_READ_INC		0b00
#define AC_WRITE_CONT	0b01
#define AC_WRITE_INC	0b10

/**
 *  Fast CRC LUT - CRC8_SAE_J1850
 */

#define SPI_SEED        0xFF // CRC seed used for the SPI communication
#define CONFIG_SEED     0xAA // CRC seed used for configurations/bitmap

uint8_t crcTable[256];
uint8_t CalcCRC(uint8_t * buf, uint8_t len, uint8_t seed)
{
  const uint8_t * ptr = buf;
  uint8_t _crc = seed; // seed
  while(len--) _crc = crcTable[_crc ^ *ptr++];
  return ~_crc;
}
void CRCInit(void)
{
  uint8_t _crc;
  for (int i = 0; i < 0x100; i++)
  {
    _crc = i;
    for (uint8_t bit = 0; bit < 8; bit++) _crc = (_crc & 0x80) ? ((_crc << 1) ^ 0x1D) : (_crc << 1); // 0x1D polynomial
    crcTable[i] = _crc;
  }
}

/**
 *  SPI Initialization
 *  MCU Dependent
 *  Initializes: the CS pin to output, SPI peripheral with the default pinout & SPI paramters:
 *  SPI @1MHz, MSB__First, SPI_MODE1 (CPOL - 0; CPHA - 1) 
 */
void SPIInit(void)
{
  pinMode(PIN_CSN, OUTPUT); // Init only CS pin
  
  SPI.begin(); // Inits MISO,MOSI,SCLK
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
}

/**
 *  SPI Backbone
 *  MCU Dependent
 *  Send 4 bytes and receive 4 bytes - return uint32_t (can be changed to return a vector)
 */
uint32_t SpiSendAndReceive(uint8_t* data_temp)
{
  // Create buffer for receiving and sending data
  static uint8_t sentReceivedData[4] = {0,0,0,0};
  
  // Fill buffer with received data
  for(uint8_t i = 0; i < sizeof(sentReceivedData); i++)
  {
    sentReceivedData[i] = data_temp[i];
  }
  
  // Print full MOSI frame
  Serial.print("MOSI Frame: 0x");
  Serial.println((uint32_t) (((uint32_t)sentReceivedData[0] << 24) | ((uint32_t)sentReceivedData[1] << 16) | ((uint32_t)sentReceivedData[2] << 8) | (uint32_t)sentReceivedData[3]), HEX);
  

  digitalWrite(PIN_CSN, LOW);                                 // Assert Chip Select
  SPI.transfer(sentReceivedData, sizeof(sentReceivedData));   // Send data and receive in the same buffer
  digitalWrite(PIN_CSN, HIGH);                                // Deassert Chip Select
  
  // return data in 32 bits - full MISO frame
  return (uint32_t) (((uint32_t)sentReceivedData[0] << 24) | ((uint32_t)sentReceivedData[1] << 16) | ((uint32_t)sentReceivedData[2] << 8) | (uint32_t)sentReceivedData[3]);
}

/**
 *  Next Frame Architecture
 *  MCU Independent
 *  SPI Command Next Frame 
 *  Input: ((14 bit max) address of target register, (2 bit) register access type)
 *  Output: (32 bit) sensor response (data depends on previous communication frame)  
 */
uint32_t SpiCommandNextFrame(uint16_t addr, uint8_t accessType)
{
	static uint8_t data_temp[4] 	= {0,0,0,0};
	static uint8_t  cmd				    = 0b11; 			// RESERVED Access Type

	cmd = accessType & 0x03;

	data_temp[0] = CMD_ADDR;
	data_temp[1] = (uint8_t)( (addr >> 6) & 0xFF ); 	// 14 bit address - take MSB
	data_temp[2] = ((addr & 0x3F) << 2) | cmd;			  // 6 bit LSB from address and cmd 2b
	data_temp[3] = CalcCRC(data_temp, 3, SPI_SEED);

	return SpiSendAndReceive(data_temp);
}

/**
 *  Next Frame Architecture
 *  MCU Independent
 *  SPI Read Next Frame 
 *  Input: (bool) option to clear device status
 *  Output: (32 bit) sensor response (data from the addressed register before clear request)  
 */
uint32_t SpiReadNextFrame(bool clearStatus)
{
	static uint8_t data_temp[4] 	= {0,0,0,0};

	data_temp[0] = CMD_DATA_ADDR;
	data_temp[1] = 0x00;						//Unused
	if(clearStatus)
	{
		data_temp[2] = 0xFF;					// All 1s clear device status bit
	}
	else
	{
		data_temp[2] = 0x00;					// All 0s no clearing
	}
	data_temp[3] = CalcCRC(data_temp, 3, SPI_SEED);		// 1 byte CRC

	return SpiSendAndReceive(data_temp);
}

/**
 *  Next Frame Architecture
 *  MCU Independent
 *  SPI Write Next Frame 
 *  Input: (16 bit) data to be written to the addressed register
 *  Output: (32 bit) sensor response (data from the addressed register before write action)  
 */
uint32_t SpiWriteNextFrame(uint16_t data)
{
	static uint8_t data_temp[4] 	= {0,0,0,0};

	data_temp[0] = CMD_DATA_ADDR;
	data_temp[1] = (data >> 8) & 0xFF;			          // 1 data byte MSB
	data_temp[2] = data & 0xFF;					              // 1 data byte LSB
	data_temp[3] = CalcCRC(data_temp, 3, SPI_SEED);		// 1 byte CRC

	return SpiSendAndReceive(data_temp);
}

/**
 *  Next Frame Architecture
 *  MCU Independent
 *  SPI Read Register Next Frame 
 *  Input: ((14 bit max) address of target register, (bool) option to clear device status)
 *  Output: (32 bit) sensor response (data from the addressed register before clear request)  
 */
uint32_t SpiReadRegNextFrame(uint16_t addr, bool clearStatus)
{
	SpiCommandNextFrame(addr, AC_READ_INC);
	return SpiReadNextFrame(clearStatus);
}

/**
 *  Next Frame Architecture
 *  MCU Independent
 *  SPI Write Register Next Frame 
 *  Input: ((14 bits max) address of target register, (16 bits) data to be written to the addressed register)
 *  Output: (32 bits) sensor response (data from the addressed register before write action)  
 */
uint32_t SpiWriteRegNextFrame(uint16_t addr, uint16_t data)
{
	SpiCommandNextFrame(addr, AC_WRITE_INC);
	return SpiWriteNextFrame(data);
}

 /**
 * Function for calculating the User Configuration block CRC
 * Reads all user bitmap registers with auto increment
 * Input: (8bit) start address of block, (8bit) stop address of block, (bool) option to clear device status
 * Output: (8bit) calculated block CRC
 */
uint32_t CalcUserConfigCRC(uint8_t startAddr, uint8_t stopAddr, bool clearStatus)
{
  if (stopAddr < startAddr)
  {
    return 0; // Empty range
  }

  uint8_t count = (uint8_t)(stopAddr - startAddr + 1);

  uint8_t stream[2 * count - 1]; // Last register contains also the CRC of the block => use only MSB from register 
  uint8_t idx = 0;

  SpiCommandNextFrame(startAddr, AC_READ_INC); // Send next frame read command with auto increment to the first register address, do not care about response

  for (uint8_t i = 0; i < count; i++)
  {
    uint32_t resp = SpiReadNextFrame(clearStatus); // Read registers with auto increment

    uint8_t msb = (uint8_t)((resp >> 16) & 0xFF); // Take data MSB
    uint8_t lsb = (uint8_t)((resp >>  8) & 0xFF); // Take data LSB

    if (i == (uint8_t)(count - 1))
    {
      stream[idx++] = msb; // Last word: only MSB
    }
    else
    {
      // All other words: MSB then LSB
      stream[idx++] = msb;
      stream[idx++] = lsb;
    }
  }

   return CalcCRC(stream, idx, CONFIG_SEED);
}

 /**
 * Function for writing the CRC for the bitmap User Configuration block
 */
void WriteUserConfigCRC()
{
  uint8_t crc = CalcUserConfigCRC(USR_CONFIG_START_ADDRESS, USR_CONFIG_STOP_ADDRESS, true);           // Calculate CRC
  uint16_t crcLine = (uint16_t)((SpiReadRegNextFrame(USR_CONFIG_STOP_ADDRESS, true) >> 8) & 0xFFFF);  // Read CRC line
  crcLine = (uint16_t)((crcLine & 0xFF00) | crc);                                                     // Replace CRC and keep useful data
  SpiWriteRegNextFrame(USR_CONFIG_STOP_ADDRESS, crcLine);                                             // Write new CRC

  // Print data
  Serial.print("                        New CRC: 0x");
  Serial.println(crc, HEX);
}

 /**
 * Function for decoding SPI angle value
 * Input: Unsigned 16 bit raw angle
 * Output: float angle in 360 degrees range
 */
const float LSB2Degree16Bit = 360.0 / 65536;
float GetAngleDeg(uint16_t angleLsb)
{
  return angleLsb * LSB2Degree16Bit;
}

/**
 *  setup
 *  Put your setup code here, to run once:
 *  Initializes Fast CRC, SPI, Serial port @115200 baud rate
 */
void setup()
{
  // Initialize fast CRC LUT
  CRCInit();
  // Initialize SPI
  SPIInit();
  // Start serial communication with 115200 baud rate
  Serial.begin(115200);
  Serial.println("Init Done...");
}


/**
 *  loop
 *  put your main code here, to run repeatedly:
 *  reads angle @ 1 seconds
 */
// Bool used to choose SPI angle decoding in case it is read
bool DECODE_ANGLE = true;

void loop()
{
  // Static frame variables
  static uint32_t   responseFrame;
  static uint8_t    responseStatus;
  static uint16_t   responseData;
  static uint8_t    responseCRC;
  // Frame counter
  static uint32_t   frameCount = 0;

  frameCount++;
  Serial.println("");
  Serial.print("**********NEW FRAME********** nr.");
  Serial.println(frameCount);

  // Read any address w/wo Clear Status
  responseFrame = SpiReadRegNextFrame(ADDR_ANGLE_PRED, true);

  // Decode Frame
  responseStatus  = (responseFrame >> 24) & 0xFF;
  responseData    = (responseFrame >> 8) & 0xFFFF;
  responseCRC     = responseFrame & 0xFF;
  
  // Print data
  Serial.print("MISO Frame: 0x");
  Serial.println(responseFrame, HEX);

  Serial.print("Status: 0x");
  Serial.println(responseStatus, HEX);

  Serial.print("Data: 0x");
  Serial.println(responseData, HEX);

  Serial.print("CRC: 0x");
  Serial.println(responseCRC, HEX);

  // Option to decode angle
  if(DECODE_ANGLE)
  {
    Serial.print("###########ANGLE[deg]: ");
    Serial.println(GetAngleDeg(responseData), 2);
  }


  // #################### Write example ###########################
  static uint32_t temp;

  Serial.println("");
  Serial.println("***WRITE EXAMPLE***");

  // Unlock user registers for write access
  SpiWriteRegNextFrame(UNLOCK_REG, USR_PASS);

  // Disable CRC check for bitmap
  SpiWriteRegNextFrame(STAT_EN_1_REG, CRC_BM_DIS);

  // Write any USER register
  temp = SpiWriteRegNextFrame(USR_CONFIG_7_REG, 0x1001);
  Serial.print("                        Data before write: 0x");
  temp = (temp >> 8) & 0xFFFF; // Display only data
  Serial.println(temp, HEX);

  temp = SpiReadRegNextFrame(USR_CONFIG_7_REG, true);
  Serial.print("                        Data after write: 0x");
  temp = (temp >> 8) & 0xFFFF; // Display only data
  Serial.println(temp, HEX);

  WriteUserConfigCRC(); // Update CRC - Mandatory for restart from VM, volatile data is lost in case of wrong CRC detection

  SpiWriteRegNextFrame(STAT_EN_REG, VAL_SOFT_RESET_NVM_DATA); // Restart from NVM - lose all volatile data
  //SpiWriteRegNextFrame(STAT_EN_REG, VAL_SOFT_RESET_VM_DATA); // Restart from VM - keep all volatile data


  // Readout delay - Can be changed
  delay(1000);
}
