#include "communication.h"
/*Tranceiver includes*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24Network.h>
#include <RF24.h>

/*RF module variables*/
extern RF24 radio;
//Network uses that radio
extern RF24Network network;


/*Assigns memory for the payload*/
byte payload[8]; 

/*System variables*/
extern byte state;
extern byte buoyCode;

/*GPS storage variables*/
extern long gpsLat;
extern long gpsLon;
extern int battery;
extern bool gpsFix;
extern int compassHeading;

void checkIfPacketsAreAvailable()
{
  /*Handle radio*/
  if(network.available())
  {
    //Serial.println("Received request");

     //Fetch the data payload
    RF24NetworkHeader header;
    network.read(header,&payload,sizeof(payload)); 
    delay(20);
    
    if (payload[0] == REQUEST_HEARTBEAT)
    {      
      Serial.println("R HB"); 
      /*Build heartbeat payload*/
      payload[0] = REPLY_HEARTBEAT;
      payload[1] = buoyCode;
      payload[2] = state;
      
      /*Send off msg*/
      //Serial.println("Sending heartbeat Packet");
      RF24NetworkHeader header(/*to node*/ CONTROLLER_ADDRESS);
      bool sendStatus = network.write(header,&payload,sizeof(payload));   
      delay(20);   
    }   
    else if (payload[0] == REQUEST_MAG)
    {      
      Serial.println("R M"); 
      /*Build heartbeat payload*/
      payload[0] = REPLY_MAG;
      payload[1] = buoyCode;
      payload[2] = state;
      
      /*Copy int variable into array*/      
      const void* buf = &compassHeading;      
      memcpy(&payload[3],buf,sizeof(int));

      /*Send off msg*/
      //Serial.println("Sending MAG Packet");
      RF24NetworkHeader header(/*to node*/ CONTROLLER_ADDRESS);
      bool sendStatus = network.write(header,&payload,sizeof(payload));
 
    } 
    else if (payload[0] == REQUEST_GPS_FIX)
    {    
      Serial.println("R GF");  
      /*Build heartbeat payload*/
      payload[0] = REPLY_GPS_FIX;
      payload[1] = buoyCode;
      payload[2] = state;
      
      /*Copy int variable into array*/
      const void* buf = &battery;      
      memcpy(&payload[3],buf,sizeof(short));
      /*Send off msg*/
      //Serial.println("Sending MAG Packet");
      RF24NetworkHeader header(/*to node*/ CONTROLLER_ADDRESS);
      bool sendStatus = network.write(header,&payload,sizeof(payload));    
    } 
    else if (payload[0] == REQUEST_GPS_LAT)
    {    
      Serial.print("R La"); 
      /*Build heartbeat payload*/
      payload[0] = REPLY_GPS_LAT;
      payload[1] = buoyCode;
      payload[2] = state;
      
      /*Copy int variable into array*/
      const void* buf = &gpsLat;      
      memcpy(&payload[3],buf,sizeof(long));
      /*Send off msg*/
      //Serial.println("Sending MAG Packet");
      RF24NetworkHeader header(/*to node*/ CONTROLLER_ADDRESS);
      bool sendStatus = network.write(header,&payload,sizeof(payload));    
    }
    else if (payload[0] == REQUEST_GPS_LON)
    {    
      Serial.println("R Lo");  
      /*Build heartbeat payload*/
      payload[0] = REPLY_GPS_LON;
      payload[1] = buoyCode;
      payload[2] = state;
      
      /*Copy int variable into array*/
      const void* buf = &gpsLon;      
      memcpy(&payload[3],buf,sizeof(long));
      /*Send off msg*/
      //Serial.println("Sending MAG Packet");
      RF24NetworkHeader header(/*to node*/ CONTROLLER_ADDRESS);
      bool sendStatus = network.write(header,&payload,sizeof(payload));    
    }  
    
    
  }
}

/*Initialise RF module to state where it is connected to the course*/
void initRF()
{    
    radio.begin();
    network.begin(/*channel*/ 90, /*node address*/ BUOY_G1_ADDRESS);
}
