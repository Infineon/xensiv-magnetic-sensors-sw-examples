#include "TLx493D_inc.hpp"
using namespace ifx::tlx493d;

#define EXAMPLE_DEFINE 10

/** W2B6 evaluation board */
const uint8_t POWER_PIN = LED2;
TLx493D_W2B6 dut(Wire, TLx493D_IIC_ADDR_A0_e);
/** Declaration of the sensor object. */

void setup() {
  // put your setup code here, to run once:
  Serial.begin(1000000);
  delay(3000);
  dut.setPowerPin(POWER_PIN, OUTPUT, INPUT, HIGH, LOW, 1000, 250000);
  dut.begin();
  dut.setPowerMode(TLx493D_LOW_POWER_MODE_e);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // read data from sensor
  double t, x, y, z;
  dut.getMagneticFieldAndTemperature(&x, &y, &z, &t);

  // calculations
  double br = sqrt(x*x+y*y+z*z);
  double angle = atan2(y, x) * 180.0/PI;
  
  /* Example for if-else structure. Don't copy blindly, it's NOT THE SOLUTION.
  if(angle > 180.0){
    Serial.println("Pressed");
  }
  else{

  }
  */
  
  // Print data to the PC console
  Serial.print(br);
  Serial.print("\t"); //Tabulator space
  Serial.print(angle);
  Serial.println(); //New line
}


