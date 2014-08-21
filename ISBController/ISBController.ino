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

/*Radio General Settings*/
#define PAYLOAD_SIZE 8
#define MAX_QUEUE_SIZE 10

/*Define radio addresses*/
#define CONTROLLER_ADDRESS 0
#define BUOY_G1_ADDRESS 1

/*Define message types*/
#define REQUEST_HEARTBEAT 0
#define REQUEST_BAT 1
#define REQUEST_GPS 2
#define REQUEST_MAG 3
#define REQUEST_GPS_FIX 4
#define REQUEST_GPS_LAT 5
#define REQUEST_GPS_LON 6
#define REPLY_HEARTBEAT 7
#define REPLY_MAG 8
#define REPLY_GPS_FIX 9
#define REPLY_GPS_LAT 10
#define REPLY_GPS_LON 11
#define RECEIVE_ERROR 200

/*RF module variables*/
RF24 radio(RF_CE, RF_CSN);
// Network uses that radio
RF24Network network(radio);

//Handle event when queue is full
//byte payload[PAYLOAD_SIZE];
byte sendQueue[MAX_QUEUE_SIZE][PAYLOAD_SIZE];
byte queueSendPointer = 0;
byte queuePopulatePointer = 0;

int heartbeatReqCount = 0;

bool buoyStatus [COURSE_SIZE]; // Status of buoy, gets updated at constant intervals. -1 indicates no communication.
int buoyCompass = 0;
byte buoyNoSat = 0;
long buoyLat = 0;
long buoyLon = 0;

bool waitingOnReply = false;
byte hbTurnCount = 200;
byte magTurnCount = 200;
int timeout = 255;

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
  /*Update the mesh network*/
  network.update();
  checkIfPacketsAreAvailable();
  
  if( queueSendPointer != queuePopulatePointer)
  {
    if(!waitingOnReply )
    {
      
       /*Do transmission*/
       Serial.print(" Node ");
       Serial.print(sendQueue[queueSendPointer][1]);
       Serial.print(" Send Pointer ");
       Serial.print(queueSendPointer);
       Serial.print(" Pop Pointer ");
       Serial.println(queuePopulatePointer);
       
       RF24NetworkHeader header(/*to node*/ sendQueue[queueSendPointer][1]);        
       bool sendStatus = network.write(header,&sendQueue[queueSendPointer],sizeof(sendQueue[queueSendPointer]));
       queueSendPointer++;
       if(queueSendPointer == MAX_QUEUE_SIZE)
         queueSendPointer = 0;
        /*if(magTurnCount <= hbTurnCount)
        {
          getBuoyCompass(BUOY_G1_ADDRESS);
          magTurnCount = 200;
          
        }
        else if(hbTurnCount < magTurnCount)
        {
          checkBuoy(BUOY_G1_ADDRESS);
          requestNoOfSats(BUOY_G1_ADDRESS);
          hbTurnCount = 200;
        } 
        
        if(magTurnCount > 0) magTurnCount -= 1;
        if(hbTurnCount > 0) hbTurnCount -= 1;*/
        
        waitingOnReply = true;
        timeout = 5000;
    }
    else if(timeout < 0)
    {
        waitingOnReply = false;
        timeout = 255;
        Serial.println("Timeout");
    }
    else
      timeout -=1;
    
  }
      
  /*Check the status of buoys*/
  if(heartbeatReqCount == 5000)
  {     
    checkBuoy(BUOY_G1_ADDRESS,sendQueue[queuePopulatePointer++]);
    if(queuePopulatePointer == MAX_QUEUE_SIZE)
     queuePopulatePointer = 0;
    getBuoyCompass(BUOY_G1_ADDRESS,sendQueue[queuePopulatePointer++]);
    if(queuePopulatePointer == MAX_QUEUE_SIZE)
     queuePopulatePointer = 0;
    requestNoOfSats(BUOY_G1_ADDRESS,sendQueue[queuePopulatePointer++]);
    if(queuePopulatePointer == MAX_QUEUE_SIZE)
     queuePopulatePointer = 0;
    
    updateGUI();
    //if(buoyStatus[0]) Serial.println("Active"); else Serial.println("Inactive");
    heartbeatReqCount = 0;
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
  byte payloadRec[PAYLOAD_SIZE];
  while(network.available() )  
  {  
     
     //Fetch the data payload
     RF24NetworkHeader header;
     network.read(header,&payloadRec,sizeof(payloadRec));
    
    //Check the type of message that was received
     if(payloadRec[0] == REPLY_HEARTBEAT)
     {
        Serial.println("R HB");
        //Update alive buoys
        buoyStatus[payloadRec[1]] = true;
     }    
     else if(payloadRec[0] == REPLY_MAG)
     {     
        Serial.println("R M");
        memcpy(&buoyCompass,&payloadRec[3],sizeof(int));        
     }    
     else if(payloadRec[0] == REPLY_GPS_FIX)
     {     
        Serial.println("R GF");
        memcpy(&buoyNoSat,&payloadRec[3],sizeof(byte));     

     }    
     else
     {
        Serial.print("Unknown Packet: "); 
        Serial.println(payloadRec[0]);
     }
     waitingOnReply = false;
  }
  
}

/*Checks if a buoy is alive. If the buoy is active, it will reply its battery voltage*/
void checkBuoy(int buoyAddress, byte* payload)
{
   /*Build Payload*/
   Serial.println("S HB");
   payload[0] = REQUEST_HEARTBEAT;
   payload[1] = buoyAddress;
     
   /*Check if msg could be sent succesfully, if not buoy is not available*/
   //if (!sendStatus)
   //{
     //Serial.println("Failed");
     //buoyStatus[buoyAddress-1] = false;

   //}    
   
}

/*Checks if a buoy is alive. If the buoy is active, it will reply its battery voltage*/
void getBuoyCompass(int buoyAddress, byte* payload)
{
   /*Build Payload*/
   Serial.println("S M");
   payload[0] = REQUEST_MAG;
   payload[1] = buoyAddress;
   
}

void requestNoOfSats(int buoyAddress, byte* payload)
{
      /*Build payload*/
      Serial.println("S GF");
      payload[0] = REQUEST_GPS_FIX;
      payload[1] = buoyAddress;
      /*Send off msg*/
      //RF24NetworkHeader header(/*to node*/ buoyAddress);
      //bool sendStatus = network.write(header,&payload,sizeof(payload));   
}


/*Initialise RF module to state where it is connected to the course*/
void initRF()
{    
    radio.begin();
    network.begin(/*channel*/ 90, /*node address*/ CONTROLLER_ADDRESS);    
}








