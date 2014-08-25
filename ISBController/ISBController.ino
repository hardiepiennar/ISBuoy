/*
Code for ISB Controller. The ISB Controller is a user interface 
 for the ISB project. This firmware controls an LCD and rotary 
 encoder to give the user control through menus. Data can then 
 be sent between the ISB system and the controller through a 
 tranceiver 
 
 Thanks to Adafruit for their LCD Library and Tutorials.
 Thanks to 0xPIT for his rotary encoder library
 Thanks to the Arduino libraries used in this project
 */


/*Tranceiver includes*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24Network.h>
#include <RF24.h>
/*LCD Includes*/
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
/*Rotary Encoder Includes*/
#include <ClickEncoder.h>
#include <TimerOne.h>
/*General Includes*/
#include "pins.h"
#include "configuration.h"
#include "gui.h"
#include "communication.h"

/*GUI global variables*/
/*LCD initialisation on software SPI, hardware SPI will be used for the tranceiver*/
Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_CLK,LCD_DIN,LCD_DC, LCD_CS, LCD_RST);
/*Encoder initialisation using defined pins and steps-per-notch*/
ClickEncoder *encoder = new ClickEncoder(ENCODER_DT, ENCODER_CLK, ENCODER_SWITCH,ENCODER_STEPS_PER_NOTCH);

/*RF module global variables*/
RF24 radio(RF_CE, RF_CSN);
// Network uses that radio
RF24Network network(radio);



int heartbeatReqCount = 0; //Currently used to time buoy data requests

bool buoyStatus [COURSE_SIZE]; // Status of buoy, gets updated at constant intervals. -1 indicates no communication.

/*Buoy variables*/
int buoyCompass = 0;
int buoyBat = 0;
long buoyLat = 0;
long buoyLon = 0;



void setup()   {
  Serial.begin(9600);

  /*Initialise rotary encoder used to control the interface
   and screen used to display the interface*/
  initEncoder();
  initScreen(MENU_BUOYS,MENU_BACK); 
  initRF();

  /*Show intro*/
  splashScreen();
  delay(2000);
  
  /*Initialise Buoy Statuses*/
  int i = 0;
  for(i = 0;i < COURSE_SIZE;i++)
    buoyStatus[i] = false;
 
}


void loop() {
  updateNetwork();
  
  
      
  /*Check the status of buoys*/
  if(heartbeatReqCount == 5000)
  {     
    checkBuoy(BUOY_G1_ADDRESS);
    getBuoyCompass(BUOY_G1_ADDRESS);
    requestBattery(BUOY_G1_ADDRESS);
    requestLat(BUOY_G1_ADDRESS);
    requestLon(BUOY_G1_ADDRESS);
 
    /*Update the screen incase some values have changed*/
    updateGUI();

    /*Reset Counter*/
    heartbeatReqCount = 0;
  }

  /*Check controls and handle user input*/
  controlUpdate();

  /*Update LCD*/
  screenUpdate();

  /*Increase counter timing buoy queries*/
  heartbeatReqCount += 1;
}











