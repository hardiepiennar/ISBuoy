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

/*General Includes*/
#include "pins.h"
#include "configuration.h"
#include "gui.h"

/*Include LCD*/
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
/*Rotary Encoder Includes*/
#include <ClickEncoder.h>
#include <TimerOne.h>

/*GUI global variables*/
/*LCD initialisation on software SPI, hardware SPI will be used for the tranceiver*/
Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_CLK,LCD_DIN,LCD_DC, LCD_CS, LCD_RST);

/*Encoder initialisation using defined pins and steps-per-notch*/
ClickEncoder *encoder = new ClickEncoder(ENCODER_DT, ENCODER_CLK, ENCODER_SWITCH,ENCODER_STEPS_PER_NOTCH);


/*Define radio addresses*/
#define CONTROLLER_ADDRESS 0
#define BUOY_G1_ADDRESS 1

/*Define message types*/
#define REQUEST_HEARTBEAT 0
#define REQUEST_BAT 1
#define REQUEST_GPS 2
#define REQUEST_MAG 3
#define REPLY_HEARTBEAT 4
#define RECEIVE_ERROR 200

#define COURSE_SIZE 10


/*RF module variables*/
RF24 radio(RF_CE, RF_CSN);
// Network uses that radio
RF24Network network(radio);

byte payload[8];

short heartbeatReqCount = 0;



int buoyStatus [COURSE_SIZE]; // Status of buoy, gets updated at constant intervals. -1 indicates no communication.

void setup()   {
  Serial.begin(9600);

  /*Initialise rotary encoder used to control the interface
   and screen used to display the interface*/
  initEncoder();
  initScreen(MENU_STATUS,MENU_BACK); 
  initRF();

  /*Show intro*/
  splashScreen();
  delay(2000);
  
  /*Initialise Buoy Statuses*/
  int i = 0;
  for(i = 0;i < COURSE_SIZE;i++)
    buoyStatus[i] = 0;
 
}


void loop() {
  /*Update the mesh network*/
  network.update();
  checkIfPacketsAreAvailable();
  
  /*Check the status of buoys*/
  if(heartbeatReqCount == 0)
  {
    checkBuoy(BUOY_G1_ADDRESS);  
    updateGUI();

  }

  /*Start with main routine*/
  //update all systems
  /*Test peripherals*/


  /*Check controls and handle user input*/
  controlUpdate();

  /*Update LCD*/
  screenUpdate();


  heartbeatReqCount += 1;
}

void checkIfPacketsAreAvailable()
{
  payload[0] = 200;
  if (network.available() )
  {  
    // Read the data payload until we've received everything
    bool done = false;
    
     //Fetch the data payload
     RF24NetworkHeader header;
     network.read(header,&payload,sizeof(payload));
     //done = radio.read(&payload, sizeof(payload) );      
       
    
    //Check the type of message that was received
     if(payload[0] == REPLY_HEARTBEAT)
     {
        //Update alive buoys
        buoyStatus[payload[1]] = 1;
     }     

  }
  
}

/*Checks if a buoy is alive. If the buoy is active, it will reply its battery voltage*/
void checkBuoy(int buoyAddress)
{
   /*Build Payload*/
   payload[0] = REQUEST_HEARTBEAT;
   
   /*Do transmission*/
   RF24NetworkHeader header(/*to node*/ buoyAddress);
   bool sendStatus = network.write(header,&payload,sizeof(payload));

   //radio.stopListening();
   
   //bool sendStatus = radio.write(&payload, sizeof(payload));
   
   /*Check if msg could be sent succesfully, if not buoy is not available*/
   if (!sendStatus)
   {
     buoyStatus[buoyAddress-1] = 0;

   }
     
}


/*Initialise RF module to state where it is connected to the course*/
void initRF()
{    
    radio.begin();
    network.begin(/*channel*/ 90, /*node address*/ CONTROLLER_ADDRESS);    
}








