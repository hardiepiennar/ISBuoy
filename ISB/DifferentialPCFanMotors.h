/*
Differential PC-Fan Motor Driver

The code here sets up 2 motors, called left (Port) and right (Starboard), on 2 different PWM channels and allows the user to control their speed from 0 to 255.
The main features of this class is that it changes masks the pwm functions, keeps the track of the currently-set motor speed and properly deals with a motor start-up.
This class is intended for 2-wire fans. Therefore, no tach feedback is available and the motors are controlled in an open-loop fasion. Fans should be controlled through a transistor.
*/
#include <Arduino.h>

#ifndef DIFF_PC_FAN_MOTORS
#define DIFF_PC_FAN_MOTORS
void setupMotors(int leftMotor,int rightMotor);
void setLeftThrust(int thrust);
void setRightThrust(int thrust);
int getLeftThrust();
int getRightThrust();
#endif
