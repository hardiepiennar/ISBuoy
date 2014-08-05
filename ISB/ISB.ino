#include "DifferentialPCFanMotors.h"
#include "pinConfig.h"


void setup() {
  /*Setup Motors*/
  setupMotors(LEFT_MOTOR,RIGHT_MOTOR);
}

void loop() {
  /*Motor Wet Long-run Test*/ 
  setLeftThrust(255);
  delay(60000);
  setLeftThrust(125);
  delay(60000);
  setLeftThrust(0);
  setRightThrust(255);
  delay(60000);
  setRightThrust(125);
  delay(60000);
  setLeftThrust(255);
  setRightThrust(255);
  delay(60000);
  setLeftThrust(125);
  setRightThrust(125);
  delay(60000);
  setLeftThrust(0);
  setRightThrust(0);
}
