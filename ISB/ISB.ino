#include "DifferentialPCFanMotors.h"
#include "pinConfig.h"


void setup() {
  setupMotors(LEFT_MOTOR,RIGHT_MOTOR);
}

void loop() {
  setLeftThrust(125);
  delay(5000);
  setLeftThrust(0);
  delay(5000);
  setLeftThrust(254);
  delay(5000);
  setLeftThrust(0);
  delay(5000);
  
}
