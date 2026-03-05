
/*
TLE4972 – SICI interface example (Arduino Uno)

PIN_SICI = A0 ->  external 2.2 kΩ pull-up to 3.3 V
PIN_VSENS = 12 -> VSENS / LDO_EN (HIGH = sensor on)

This example:
1) Enters SICI interface (0xABCD password),
2) Disables ISM (write 0x8000 to address 0x25),
3) Reads EEPROM 0x40–0x51 (pipelined),
4) Powers ISM back up,
5) Exits interface mode.

*/

#include <Arduino.h>

// -------- Pins ---------------
#define PIN_SICI A0 // AOUT / SICI line
#define PIN_VSENS 12 // VSENS / LDO_EN

struct BitTiming {
uint16_t t1;
uint16_t t2;
uint16_t t3;
uint16_t tR;
uint16_t t4;
uint16_t t5;
};

// -------- SICI timing --------
static const unsigned TBIT_US = 45; // 40..7500 µs allowed
static const unsigned ENTER_IFEN_US = 300; // keep AOUT low before enabling SICI
static const unsigned DELAY_BETWEEN_CMD = 150; // gap between frames (avoid long-high exit)

static inline void driveLow() { pinMode(PIN_SICI, OUTPUT); digitalWrite(PIN_SICI, LOW); }
static inline void releaseLine() { pinMode(PIN_SICI, INPUT); } // hi-Z, pulled up to 3.3 V
static inline int readLine() { return digitalRead(PIN_SICI); }
static inline void usDelay(unsigned t) { delayMicroseconds(t); }

static inline void sensorOn() { pinMode(PIN_VSENS, OUTPUT); digitalWrite(PIN_VSENS, HIGH); }
static inline void sensorOff() { pinMode(PIN_VSENS, OUTPUT); digitalWrite(PIN_VSENS, LOW); }


static BitTiming g_bit0, g_bit1;

// ---------------- Compute bit timings ------------------

// Key timing paramreters from User manual
// t1  First low pulse duration which is 0.33*TBIT (bit=0) or 0.67*TBIT (bit=1)
// t2  Remaining high portion (TBIT - t1)
// t4  Response window 2*(t1-t2) - minimum 
// t3  Low time before read (inside t4) which is 0.20*t4
// tR  Response duration which is 0.70*t4
// t5  Time between 2 bits which is TBIT-t4

static void initBitTimingOne(BitTiming &b, uint8_t lowPercent)
{
  // t1: low portion of the first phase (percentage of TBIT)
  b.t1 = (uint16_t)(TBIT_US * lowPercent / 100);
  b.t2 = (uint16_t)(TBIT_US - b.t1);

  // t4 = 2 * |t1 − t2|
  uint16_t diff = (b.t1 > b.t2) ? (b.t1 - b.t2) : (b.t2 - b.t1);
  b.t4 = (uint16_t)(2 * diff);

  // datasheet: t3 ~10–30% of t4, tR ~50–80% of t4
  b.t3 = (uint16_t)(b.t4 * 20 / 100); // 20%
  b.tR = (uint16_t)(b.t4 * 70 / 100); // 70%

  // remaining time between bits
  b.t5 = (TBIT_US > b.t4) ? (uint16_t)(TBIT_US - b.t4) : (uint16_t)1;
}

static void initBitTimings()
{
  // from UM table 2: low time sending 0 = 33%, sending 1 = 67%
  initBitTimingOne(g_bit0, 33); // logic '0'
  initBitTimingOne(g_bit1, 67); // logic '1'
}

// Send one bit and sample the sensor’s return bit
// NOTE: AOUT low during read window = logic '1' (so we invert line level)
static uint8_t SICI_SendBit(uint8_t bit)
{
  BitTiming &t = bit ? g_bit1 : g_bit0;

  // Encode the bit (t1 / t2)
  driveLow();
  usDelay(t.t1);
  releaseLine();
  usDelay(t.t2);

  // Response window: short low pulse then device drives line
  driveLow(); 
  usDelay(t.t3);
  releaseLine();

  // Sample mid-tR
  usDelay(t.tR / 2);
  uint8_t line = (uint8_t)(readLine() & 1);
  usDelay(t.tR / 2);

  // Invert: line LOW -> logic 1, line HIGH -> logic 0
  uint8_t rxBit = line ? 0 : 1;

  // Finish t4 and add t5 gap
  uint16_t used = (uint16_t)(t.t3 + t.tR);
  if (t.t4 > used) {
    usDelay(t.t4 - used);
  }
  usDelay(t.t5);

  return rxBit;
}

// Send 16-bit frame (LSB first) and return 16-bit response
static uint16_t SICI_SendCommand(uint16_t cmd)
{
uint16_t rx = 0;

noInterrupts();
for (uint8_t i = 0; i < 16; i++) {
uint8_t txb = (cmd >> i) & 0x01;
uint8_t rxb = SICI_SendBit(txb);

// LSB of response arrives first, we shift existing bits right and insert at MSB
rx = (uint16_t)((rx >> 1) | (rxb ? 0x8000 : 0x0000));
}
interrupts();

usDelay(DELAY_BETWEEN_CMD);
return rx;
}

// Build SICI command word 
static uint16_t SICI_ComputeCommand(bool write, uint8_t addr)
{
  uint16_t cmd = 0;
  if (write) cmd |= (1u << 15); // 1000 0000 0000 0000
  cmd |= (uint16_t)(addr << 4); // (0000 0000 0010 0101) < 4 = 0000 0010 0101 0000 //cmd = 1000 0010 0101 0000
  return cmd; // 0x8YY0
}

// Generic helper: read 16-bit register at given address (internal or EEPROM).
// Two-step method: first read "arms" the address, second returns valid data.
static uint16_t SICI_ReadRegister(uint8_t addr)
{
(void)SICI_SendCommand(SICI_ComputeCommand(false, addr)); // dummy, loads data
uint16_t val = SICI_SendCommand(SICI_ComputeCommand(false, addr)); // read back
return val;
}

static void EnterInterface()
{
  Serial.println("Entering SICI interface...");

  driveLow();

  // Power Cycle to align timings
  sensorOff();
  delay(2);
  sensorOn();

  // AOUT must be forced to GND before the end of this time window in order to enter the SICI interface
  usDelay(ENTER_IFEN_US); 
  
  // Release the line (hi-Z) so the bus goes high via pull-up, as required just before sending the password 
  releaseLine();

  uint16_t tx = 0xABCD;
  uint16_t rx = SICI_SendCommand(tx); // Sending the 16-bit Enter-interface command (send user password (LSB-first))

  Serial.print(F(" TX password = 0x")); Serial.println(tx, HEX);
  Serial.print(F(" RX (while sending password) = 0x")); Serial.println(rx, HEX);
}

static void DisableISM()
{
  Serial.println("Disabling ISM...");

  uint16_t header = SICI_ComputeCommand(true, 0x25);
  uint16_t r0 = SICI_SendCommand(header); // Sending Power down ISM (write command)

  Serial.print(F(" TX header = 0x")); Serial.println(header, HEX);
  Serial.print(F(" RX (during header) = 0x")); Serial.println(r0, HEX);

  uint16_t data = 0x8000;
  uint16_t r1 = SICI_SendCommand(data); //Power down ISM (Set data)
  
  Serial.print(F(" TX data = 0x")); Serial.println(data, HEX);
  Serial.print(F(" RX (during data) = 0x")); Serial.println(r1, HEX);

  Serial.println("ISM disabled.");
}

// Example EEPROM read (0x40..0x51), using pipelined reads as before
static void ReadEEPROM()
{
  Serial.println("Reading EEPROM...");

  uint16_t eep[18]; // buffer for 18 words (0x40 to 0x51)

  // Read command at EEPROM line 0
  (void)SICI_SendCommand(SICI_ComputeCommand(false, 0x40));

  // Read command for Remaining addresses (0x41 to 0x51)
  // Each call sends the next READ and receives the previous address's data
  for (uint8_t i = 0; i < 17; i++) {
    uint8_t nextAddr = 0x41 + i;
    eep[i] = SICI_SendCommand(SICI_ComputeCommand(false, nextAddr));
  }
  eep[17] = SICI_SendCommand(SICI_ComputeCommand(false, 0x00)); // NOP - dummy to receive 0x51 address data

  for (uint8_t i = 0; i < 18; i++) {
    Serial.print("EEPROM[0x");
    Serial.print(0x40 + i, HEX);
    Serial.print("] = 0x");
    Serial.println(eep[i], HEX);
  }

  Serial.println("EEPROM read complete.");
}

static void PowerUpISM()
{
  Serial.println("Powering up ISM...");
  
  uint16_t header = SICI_ComputeCommand(true, 0x25);
  uint16_t r0 = SICI_SendCommand(header); // Power on ISm (write command)
  
  Serial.print(F(" TX header = 0x")); 
  Serial.println(header, HEX);

  Serial.print(F(" RX (during header) = 0x"));
  Serial.println(r0, HEX);

  uint16_t data = 0x0000;
  uint16_t r1 = SICI_SendCommand(data);  // Power on ISM (set data)
  
  Serial.print(F(" TX data = 0x")); Serial.println(data, HEX);
  Serial.print(F(" RX (during data) = 0x")); Serial.println(r1, HEX);

  Serial.println("ISM powered up.");
}

static void ExitInterface()
{
  Serial.println("Exiting interface mode...");

  driveLow();
  delay(5);
  releaseLine();
  delay(50);
  
  Serial.println("Exited test mode.");
}

void setup()
{
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println(F("TLE4972 SICI sequence (Arduino example)"));

  releaseLine();
  sensorOff();
  delay(5);

  initBitTimings(); // precompute timing once

  EnterInterface();
  DisableISM();
  ReadEEPROM(); 
  PowerUpISM();
  ExitInterface();

Serial.println(F("\nAll steps done."));
}

void loop()
{
  //
}
