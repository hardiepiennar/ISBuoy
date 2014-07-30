/*
Differential PC-Fan Motor Driver

The code here sets up 2 motors, called left (Port) and right (Starboard), on 2 different PWM channels and allows the user to control their speed from 0 to 255.
The main features of this class is that it  masks the pwm functions and keeps the track of the currently-set motor speed.
This class is intended for 2-wire fans. Therefore, no tach feedback is available and the motors are controlled in an open-loop fasion. Fans should be controlled through a transistor.
*/

#include "DifferentialPCFanMotors.h"

/*Motor definitions*/
int leftMotorPin = -1;//Pin used for left motor          
int leftMotorCurrentSpeed = 0; //The last speed the motor was set to
int leftMotorStartupTimer = 0;//The last speed the motor was set to
int rightMotorPin = -1; 
int rightMotorCurrentSpeed = 0; 
int rightMotorStartupTimer = 0; 


/*This routine sets up the motor-driving pins*/
void setupMotors(int leftMotor,int rightMotor)  { 
  //Set motor pin variables
  leftMotorPin = leftMotor;
  rightMotorPin = rightMotor;
  
  //Set the motor pins as outputs
  pinMode(leftMotorPin, OUTPUT);
  pinMode(rightMotorPin, OUTPUT);
} 

/*These routines are used to change the speed of each motor using a value between 0 and 255
thrust - motor thrust from 0 to a 255*/
void setLeftThrust(int thrust)
{
  leftMotorCurrentSpeed = thrust; //Save current speed value for this motor
  analogWrite(leftMotorPin, leftMotorCurrentSpeed); //Set motor speed 
}
void setRightThrust(int thrust)
{
  rightMotorCurrentSpeed = thrust; //Save current speed value for this motor
  analogWrite(rightMotorPin, rightMotorCurrentSpeed); //Set motor speed
}

/*Call functions to check last set motor speed*/
int getLeftThrust()
{
  return leftMotorCurrentSpeed; //Save current speed value for this motor
}
int getRightThrust()
{
  return rightMotorCurrentSpeed; //Save current speed value for this motor
}

