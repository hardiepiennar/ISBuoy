#include "communication.h"
#include "configuration.h"
#include "pins.h"

/*Tranceiver includes*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24Network.h>
#include <RF24.h>

/*RF module global variables*/
extern RF24 radio;
// Network uses that radio
extern RF24Network network;

/*Global Buoy variables*/
extern bool buoyStatus [COURSE_SIZE];
extern int buoyCompass;
extern int buoyBat;
extern long buoyLat;
extern long buoyLon;


/*Circular messaging queue. A send pointer points to the
packet that should be sent next. A populate pointer points
to the next entry to be filled*/
byte sendQueue[MAX_QUEUE_SIZE][PAYLOAD_SIZE];
byte queueSendPointer = 0;
byte queuePopulatePointer = 0;
bool waitingOnReply = false;
int timeout = 255;

/*HAndles alll the routine communications tasks,
updates the network, checks for messages and 
tries to send messages*/
void updateNetwork()
{
  /*Update the mesh network*/
  network.update();
  
  /*Check for new packets*/
  checkIfPacketsAreAvailable();
  
  /*Handle message sending queue.
  If a message is sent, this routine waits for a reply.
  If the reply takes to long, the next message will be sent*/
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
       
        waitingOnReply = true;
        timeout = 5000;
    }
    else if(timeout < 0)
    {
        waitingOnReply = false;
        timeout = 255;
        Serial.println("Timeout");
        buoyStatus[BUOY_G1_ADDRESS-1] = false;
    }
    else
    {
      timeout -=1;
    }
    
  }
}

/*Polls the network to see if new packets are available,
If a new packet is available, the packet is handled and 
the received message flag is lifted*/
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
        memcpy(&buoyBat,&payloadRec[3],sizeof(int));     

     }
     else if(payloadRec[0] == REPLY_GPS_LAT)
     {     
        Serial.println("R La");
        memcpy(&buoyLon,&payloadRec[3],sizeof(long)); 
         

     }
     else if(payloadRec[0] == REPLY_GPS_LON)
     {     
        Serial.println("R Lo");
        memcpy(&buoyLat,&payloadRec[3],sizeof(long));     

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
void checkBuoy(int buoyAddress)
{
   byte* payload = sendQueue[queuePopulatePointer];
  
   /*Build Payload*/
   Serial.println("S HB");
   payload[0] = REQUEST_HEARTBEAT;
   payload[1] = buoyAddress;
     
   queuePopulatePointer++;
   if(queuePopulatePointer == MAX_QUEUE_SIZE)
      queuePopulatePointer = 0;
}

/*Checks if a buoy is alive. If the buoy is active, it will reply its battery voltage*/
void getBuoyCompass(int buoyAddress)
{
   byte* payload = sendQueue[queuePopulatePointer];
  
   /*Build Payload*/
   Serial.println("S M");
   payload[0] = REQUEST_MAG;
   payload[1] = buoyAddress;
   
    queuePopulatePointer++;
    if(queuePopulatePointer == MAX_QUEUE_SIZE)
      queuePopulatePointer = 0;
}

void requestBattery(int buoyAddress)
{
    byte* payload = sendQueue[queuePopulatePointer];
  
    /*Build payload*/
    Serial.println("S GF");
    payload[0] = REQUEST_GPS_FIX;
    payload[1] = buoyAddress;
    
    queuePopulatePointer++;
    if(queuePopulatePointer == MAX_QUEUE_SIZE)
      queuePopulatePointer = 0;
}

void requestLat(int buoyAddress)
{
    byte* payload = sendQueue[queuePopulatePointer];
  
    /*Build payload*/
    Serial.println("S La");
    payload[0] = REQUEST_GPS_LAT;
    payload[1] = buoyAddress;
    
    queuePopulatePointer++;
    if(queuePopulatePointer == MAX_QUEUE_SIZE)
      queuePopulatePointer = 0;
}

void requestLon(int buoyAddress)
{
    byte* payload = sendQueue[queuePopulatePointer];
  
    /*Build payload*/
    Serial.println("S Lo");
    payload[0] = REQUEST_GPS_LON;
    payload[1] = buoyAddress;
    
    queuePopulatePointer++;
    if(queuePopulatePointer == MAX_QUEUE_SIZE)
      queuePopulatePointer = 0;
}


/*Initialise RF module to state where it is connected to the course*/
void initRF()
{    
    radio.begin();
    network.begin(/*channel*/ 90, /*node address*/ CONTROLLER_ADDRESS);    
}

